//Standing Stones mapping program
//copyright 2013-2014 by Andrew Schultz
//with minor readability fixes in 2017
//
//I make no claims as to overall readability
//this simply reads in the 16x16 mess and finds where the doors and special squares are

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

short LR[16][16];
short UD[16][16];

short bmpAry[272][272];

void doStairs(FILE * X, short ud, short myColor, short myX, short myY);

void printLRUD();
void initBMP();
void setDefaults();
void redoBMP();
void writeBMP();

FILE * I;

void main(int argc, char * argv[])
{
	FILE * F = fopen("stst.txt", "r");
	FILE * G;

	char inFile[50] = "stasto1.dsk";

	char fileLine[60];
	char fileStr[40];

	short i, j, fileCount = 0, temp;

	short doors[16][16];
	long myOffset = 0;

	fileLine[0] = 0;

	if (!F)
	{
		printf("Need stst.txt to read map offsets. NOTE: they're just x3000, then increasing by x800.");
		return;
	}

	if (argc > 1) //Not very sophisticated, but we don't have a lot of options here
		strcpy(inFile, argv[1]);

	G = fopen(inFile, "rb");

	if (!G)
	{
		printf("You will need standing_stones_1.dsk to run this program--or more precisely, to rename it to stasto1.dsk.\nYou can also specify a valid DSK file on the command line.");
		return;
	}

	while (fgets(fileLine, 30, F))
	{
		printf("%s", fileLine);
		myOffset = strtol(fileLine, NULL, 16);
		fseek(G, myOffset, 0);

		for (j=0; j < 16; j++)
		{
			for (i=0; i < 16; i++)
			{
				doors[i][j] = fgetc(G); //first comes the 16x16 array of doors
				//printf("%02x ", doors[i][j]);
			}
			//printf("\n");
		}

		for (j=0; j < 16; j++)
		{
			for (i=0; i < 16; i++)
			{
				LR[i][j] = (doors[i][j] & 0xc) + ((doors[(i+15)%16][j] & 0xc0) >> 6);
				UD[i][j] = (doors[i][(j+15)%16] & 0x3) + ((doors[i][j] & 0x30) >> 2);
			}
		}

		//printLRUD();
		fileCount++;
		sprintf(fileStr, "stones-%d.bmp", fileCount);
		I = fopen(fileStr, "wb");
		initBMP();
		fseek(G, myOffset + 0x600, 0); // now we move to the location on disk that tracks the stairs. Up, down, both.
		for (j=0; j < 16; j++)
			for (i=0; i < 16; i++)
			{
				temp = fgetc(G);
				if (temp == 0xa0)
					doStairs(G, 1, 3, i, j);
				if (temp == 0xa1)
					doStairs(G, 0, 3, i, j);
				if (temp == 0xb0)
					doStairs(G, 1, 2, i, j);
			}
		redoBMP();
		writeBMP();
		fclose(I);
	}
	fclose(F);
}

void doStairs(FILE * X, short ud, short myColor, short myX, short myY)
{

	short i;

	if ((myX >= 16) || (myY >= 16))
		return;

	printf("Stairs %d at %x %x.\n", ud, myX, myY);

	for (i=4; i < 12; i++)
		bmpAry[15+myX*16][myY*16+i+8] = bmpAry[16+myX*16][myY*16+i+8] = myColor;
	if (ud == 0)
		bmpAry[14+myX*16][myY*16+13] = bmpAry[17+myX*16][myY*16+13] =
			bmpAry[13+myX*16][myY*16+14] = bmpAry[18+myX*16][myY*16+14] = myColor;
	else
		bmpAry[14+myX*16][myY*16+18] = bmpAry[17+myX*16][myY*16+18] =
			bmpAry[13+myX*16][myY*16+17] = bmpAry[18+myX*16][myY*16+17] = myColor;
}

void initBMP()
{
	short i, j;
	FILE * H = fopen("272.bmp", "rb");

	if (H == NULL)
	{
		printf ("Need 272.bmp.\n");
		return;
	}
	for (i=0; i < 272; i++)
		for (j=0; j < 272; j++)
			bmpAry[i][j] = 1;

	for (i=0; i < 0x436; i++)
	{
		fputc(fgetc(H), I);
	}
	fclose(H);
}

//this is where we dump each door. It's the main part of the program. There are a lot of magic numbers here. Sorry about that.
void redoBMP()
{
	short i, j, k;

	for (j=0; j <= 16; j++)
	{
		for (i=0; i <= 16; i++)
		{
			if (j < 16)
			{
			switch(LR[i%16][j%16]) // this is a bit convoluted. 0 = open, 1 = door, 2 = secret door, 3 = wall left 2bytes = square to left/up, right 2bytes = square to right/down
			{ // so 0, 5, 10, 15 are the most popular, but then the cases where you have 1 way travel or even door/secret door etc. need to be addressed
			case 0:
				break;

			case 5: // 1 = door
				for (k=0; k < 16; k++)
					bmpAry[7+i*16][j*16+8+k] = bmpAry[8+i*16][j*16+8+k] = 0;
				bmpAry[6+i*16][j*16+15] = bmpAry[9+i*16][j*16+15] =
					bmpAry[6+i*16][j*16+16] = bmpAry[9+i*16][j*16+16] = 0;
				break;

			case 0xa: // 2 on both sides, so, a secret door
				for (k=0; k < 16; k++)
					bmpAry[7+i*16][j*16+8+k] = bmpAry[8+i*16][j*16+8+k] = 0;
				for (k=6; k < 10; k++)
					bmpAry[5+i*16][j*16+8+k] = bmpAry[10+i*16][j*16+8+k] = 0;
				bmpAry[6+i*16][j*16+17] = bmpAry[9+i*16][j*16+14] = 0;
				break;

			case 15: // a straight up wall
				for (k=0; k < 16; k++)
					bmpAry[7+i*16][j*16+8+k] = bmpAry[8+i*16][j*16+8+k] = 0;
				break;

			case 1:
			case 2: //we don't differentiate between door and secret door when drawing 1-way. We don't have the resolution for it.
				for (k=5; k < 11; k++)
						bmpAry[7+i*16][j*16+8+k] = 0;
				bmpAry[8+i*16][j*16+13] = bmpAry[i*16+8][j*16+18] = 0;
				break;

			case 4:
			case 8: //this is like 1-2 except reversing the drawing
				for (k=5; k < 11; k++)
						bmpAry[8+i*16][j*16+8+k] = 0;
				bmpAry[7+i*16][j*16+13] = bmpAry[i*16+7][j*16+18] = 0;
				break;

			case 0x7:
			case 0xb: //(12) vs 3 -- draw  an extra wall
				for (k=0; k < 16; k++)
					bmpAry[7+i*16][j*16+8+k] = bmpAry[8+i*16][j*16+8+k] = 0;
			case 0x3: // 0 vs 3
				bmpAry[6+i*16][j*16+13] = bmpAry[5+i*16][j*16+17] =
					bmpAry[5+i*16][j*16+14] = bmpAry[6+i*16][j*16+18] = 0;

				for (k=4; k < 12; k++)
					bmpAry[k+i*16][j*16+15] = bmpAry[k+i*16][j*16+16] = 0;
				break;

			case 0xd: // 3 vs (12)
			case 0xe:
				for (k=0; k < 16; k++)
					bmpAry[7+i*16][j*16+8+k] = bmpAry[8+i*16][j*16+8+k] = 0;
			case 0xc: // 3 vs 0
				bmpAry[9+i*16][j*16+13] = bmpAry[10+i*16][j*16+17] =
					bmpAry[10+i*16][j*16+14] = bmpAry[9+i*16][j*16+18] = 0;

				for (k=4; k < 12; k++)
					bmpAry[k+i*16][j*16+15] = bmpAry[k+i*16][j*16+16] = 0;
				break;

			case 6: //2 vs 1/1 vs 2 means secret door on one side but door on other, we show a knob on one side
			case 9:
				for (k=6; k < 10; k++)
					if (LR[i%16][j%16] == 9)
						bmpAry[i*16+5][j*16+8+k] = 0;
					else
						bmpAry[i*16+10][j*16+8+k] = 0;

			default:
				printf("LR %x at %x %x not defined.\n", LR[i][j], i, j);
				break;
			}
			}
			if (i < 16)
			{
			switch(UD[i%16][j%16])
			{
			case 0:
				break;

			case 1:
			case 2:
				for (k=5; k < 11; k++)
					bmpAry[i*16+8+k][7+j*16] = 0;
				bmpAry[i*16+13][8+j*16] = bmpAry[i*16+18][8+j*16] = 0;
				break;

			case 4:
			case 8:
				for (k=5; k < 11; k++)
					bmpAry[i*16+8+k][8+j*16] = 0;
				bmpAry[i*16+13][7+j*16] = bmpAry[i*16+18][7+j*16] = 0;
				break;

			case 15:
				for (k=0; k < 16; k++)
					bmpAry[i*16+8+k][7+j*16] = bmpAry[i*16+8+k][8+j*16] = 0;
				break;

			case 0xa:
				for (k=0; k < 16; k++)
					bmpAry[i*16+8+k][7+j*16] = bmpAry[i*16+8+k][8+j*16] = 0;
				for (k=6; k < 10; k++)
					bmpAry[i*16+8+k][5+j*16] = bmpAry[i*16+8+k][10+j*16] = 0;
				bmpAry[i*16+14][6+j*16] = bmpAry[i*16+17][9+j*16] = 0;
				break;

			case 6:
				for (k=6; k < 10; k++)
					bmpAry[i*16+8+k][5+j*16] = 0;
			case 9:
				if (UD[i%16][j%16] == 9)
				{
					for (k=6; k < 10; k++)
						bmpAry[i*16+8+k][10+j*16] = 0;
				}
			case 5:
				for (k=0; k < 16; k++)
					bmpAry[i*16+8+k][7+j*16] = bmpAry[i*16+8+k][8+j*16] = 0;
				bmpAry[i*16+15][j*16+6] = bmpAry[i*16+15][j*16+9] =
					bmpAry[i*16+16][j*16+6] = bmpAry[i*16+16][j*16+9] = 0;
				break;

			case 0x7:
			case 0xb:
				for (k=0; k < 16; k++)
					bmpAry[i*16+8+k][7+j*16] = bmpAry[i*16+8+k][8+j*16] = 0;
			case 0x3:

				for (k=4; k < 12; k++)
					bmpAry[i*16+15][j*16+k] = bmpAry[i*16+16][j*16+k] = 0;

				bmpAry[i*16+13][j*16+6] = bmpAry[i*16+18][j*16+6] =
				bmpAry[i*16+14][j*16+5] = bmpAry[i*16+17][j*16+5] = 0;
				break;

			case 0xd:
			case 0xe:
				for (k=0; k < 16; k++)
					bmpAry[i*16+8+k][7+j*16] = bmpAry[i*16+8+k][8+j*16] = 0;
			case 0xc:
				for (k=4; k < 12; k++)
					bmpAry[i*16+15][j*16+k] = bmpAry[i*16+16][j*16+k] = 0;

				bmpAry[i*16+13][j*16+9] = bmpAry[i*16+18][j*16+9] =
				bmpAry[i*16+14][j*16+10] = bmpAry[i*16+17][j*16+10] = 0;
				break;

			default:
				printf("UD %x at %x %x not defined.\n", UD[i][j], i, j);
				break;
			}
			}
		}
	}
}

void writeBMP()
{
	short i, j;
	for (j=0; j < 272; j++)
		for (i=0; i < 272; i++)
			fputc(bmpAry[i][271-j], I);
	fclose(I);
}

//Debugging function to print out what the doors look like,  UL and DR.
void printLRUD()
{
	short i, j;

	printf("LR:\n");

	for (j=0; j < 16; j++)
	{
		for (i=0; i < 16; i++)
			printf(" %x", LR[i][j]);
		printf("\n");
	}

	printf("UD:\n");

	for (j=0; j < 16; j++)
	{
		for (i=0; i < 16; i++)
			printf(" %x", UD[i][j]);
		printf("\n");
	}
}