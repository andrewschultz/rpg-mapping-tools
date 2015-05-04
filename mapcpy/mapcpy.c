#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXW 640
#define MAXH 640

typedef enum { false, true } bool;

short isYellow(short x);
bool viable (int a1, int a2, int a3);

main(int argc, char * argv[])
{
	short replace[256] = { -1 };
	short addSpace = 0;
	FILE * F, * G = NULL, * H, * I;
	long keepGoing = 1;
	unsigned int count = 0;
	long toSet = 0, temp = 0, curLine = 0;
	long myDefaultOffset = 0;
	long myAbsX = 0, myAbsY = 0;
	long myLastX = 0, myLastY = 0, myX = 0, myY = 0, myH = 0, myW = 0, myOffset = 0, myXMin = 0, myXMax = 32, setTransparent = 0, transpColor = 0,
		myYModOffset=0, myXModOffset=0, outH=256, outW=256, doFringe = 0, gegege=0;
	long i, j, i2, j2;
	short qflags = 15, quartering = 0;

	short myary[MAXW][MAXW], ch;

	char buffer[200];

	if (argc == 1)
		F = fopen("mapcpy.txt", "r");
	else
		F = fopen(argv[1], "r");

	if (F == NULL)
	{
		printf("no such file\n");
		return 0;
	}

	for (j=0; j < MAXH; j++)
		for (i=0; i < MAXW; i++)
			myary[i][j] = 0;

	for (i=0; i < 256; i++)
		replace[i] = -1;

	while ((fgets(buffer, 200, F) != NULL) && (keepGoing))
	{
		curLine++;
		switch(buffer[0])
		{
		case '=':
			count = 0;
			if (buffer[1] == '=')
			{
				for (i=0; i < 256; i++)
					replace[i] = -1;
				break;
			}
			toSet = strtol(buffer+count+1, NULL, 16);
			count += 3;

			while ((count < strlen(buffer)) && (buffer[count] == '='))
			{
				temp = strtol(buffer+count+1, NULL, 16);
				replace[temp] = (short) (toSet & 0xff);
				count += 3;
			}
			break;

		//cover everything with a certain color
		case 'D':
			temp = strtol(buffer+1, NULL, 16);
			for (j=0; j < MAXH; j++)
				for (i=0; i < MAXW; i++)
					myary[i][j] = (short) temp;
			break;

		case '\n':
		case '#':
			break;

		//absolute offsets
		case 'a':
			if (buffer[1] == 'x')
				myAbsX = strtol(buffer+2, NULL, 10);
			if (buffer[1] == 'y')
				myAbsY = strtol(buffer+2, NULL, 10);
			break;

		case 'F':
			doFringe = 1;
			break;

		case '0':
			addSpace = 0;
			break;

		case '1':
			addSpace = 1;
			break;

		case 'c':
		case 'C':
			if ((buffer[1] == 'r') || (buffer[1] == 'R'))
			{
				myX = myLastX + myW;
				myY = myLastY;
				if (addSpace == 1)
					myX++;
				break;
				printf("Invalid command--cr is the only one.");
			}
			break;

		case 't':
			if (buffer[1] == 'c')
			{
				transpColor = strtol(buffer+2, NULL, 16);
				break;
			}
			if (buffer[1] == '-')
				setTransparent = 0;
			else
				setTransparent = 1;
			break;

		//x/y move you to absolute coordinates myX+myAbsX
		case 'x':
			myX = strtol(buffer+1, NULL, 10);
			myLastX = myX;
			break;

		case 'y':
			myY = strtol(buffer+1, NULL, 10);
			myLastY = myY;
			break;

		case 'd':
			if (buffer[1] == 'x')
			{
				if (buffer[2] == '-')
				{
					myX -= strtol(buffer+3, NULL, 10);
				}
				else
					myX += strtol(buffer+2, NULL, 10);
				//printf("Adding %d to x, now %d\n", strtol(buffer+2, NULL, 10), myX);
				break;
			}
			if (buffer[1] == 'y')
			{
				if (buffer[2] == '-')
				{
					myY -= strtol(buffer+3, NULL, 10);
				}
				else
					myY += strtol(buffer+2, NULL, 10);
				//printf("Adding %d to y, now %d\n", strtol(buffer+2, NULL, 10), myY);
				break;
			}
			printf("Error at line %d. d(xy) is proper usage for moving.\n", curLine);
			break;

		//+/-/^/v move you around relatively, as opposed to absolute
		case '+':
			myXModOffset = strtol(buffer+1, NULL, 10);
			break;

		case '-':
			myXModOffset = myW - strtol(buffer+1, NULL, 10);
			break;

		case 'v':
			myYModOffset = strtol(buffer+1, NULL, 10);
			break;

		case '^':
			myYModOffset = myH - strtol(buffer+1, NULL, 10);
			break;

		case 'W':
			outW = strtol(buffer+1, NULL, 10);
			break;

		case 'H':
			outH = strtol(buffer+1, NULL, 10);
			break;

		case 'h':
			myH = strtol(buffer+1, NULL, 10);
			break;

		case 'w':
			myW = strtol(buffer+1, NULL, 10);
			break;

		case 'm':
			myXMin = strtol(buffer+1, NULL, 10);
			break;

		case 'M':
			myXMax = strtol(buffer+1, NULL, 10);
			break;

		case 'q':
			if (buffer[1] == '-')
			{
				quartering = 0;
				qflags = 15;
				break;
			}
			if ((buffer[1] == 'n') || (buffer[1] == 'N'))
			{
				qflags |= 1; qflags &= 0xfc;
			}
			if ((buffer[1] == 's') || (buffer[1] == 'S'))
			{
				qflags |= 4; qflags &= 0xff;
			}
			if ((buffer[2] == 'e') || (buffer[2] == 'E'))
			{
				qflags |= 2; qflags &= 0xfe;
			}
			if ((buffer[2] == 'w') || (buffer[2] == 'W'))
			{
				qflags |= 8; qflags &= 0xf8;
			}
			if ((qflags != 12) && (qflags != 9) && (qflags != 6) && (qflags != 3))
			{
				printf ("Bailing. Bad qflags value of %d at line %d given %s\n", qflags, curLine, buffer);
			}
			break;

		case ';':
			keepGoing = 0;
			break;

		case 'r':
			if (G)
				fclose(G);
			buffer[strlen(buffer)-1] = 0;
			//debug below
			//printf("Reading %s\n", buffer+1);
			G = fopen(buffer+1, "rb");
			if (G == NULL)
			{
				printf("oops file %s does not exist bailing at line %d\n", buffer+1, curLine);
				return 0;
			}
			break;

		case 'g': // this is specific to Gegege No Kitaro
			gegege=1;
			break;

		case '>': // this outputs to a file
			buffer[strlen(buffer)-1] = 0;
			printf("Writing %s\n", buffer+1);
			I = fopen(buffer+1, "wb");
			H = fopen("256.bmp", "rb");
			if (H == NULL)
			{
				printf("No 256.bmp in current directory. Trying c:\\coding\\games.\n");
				H = fopen("c:\\coding\\games\\256.bmp", "rb");
				if (H == NULL)
				{
					printf("Need 256.bmp. I can't read it. Bailing.\n");
					return 0;
				}
			}
			for (i=0; i < 0x436; i++)
			{
				switch(i)
				{
				case 0x12:
					fputc(outW & 0xff, I);
					fgetc(H);
					break;

				case 0x13:
					fputc((outW>>8) & 0xff, I);
					fgetc(H);
					break;

				case 0x16:
					fputc(outH & 0xff, I);
					fgetc(H);
					break;

				case 0x17:
					fputc((outH>>8) & 0xff, I);
					fgetc(H);
					break;

				default:
					fputc(fgetc(H), I);
					break;
				}
			}
			fclose(H);

			printf("outH outW %d %d\n", outH, outW);
			if (gegege == 1) //VERY VERY BAD HACK.
			{
			for (j=1; j < outH-1; j++)
				for (i=0; i < outW-1; i++)
					if (myary[i][j] == 0x03)
						if (isYellow(myary[i][j-1]) || isYellow(myary[i][j+1]) || isYellow(myary[i+1][j]) || isYellow(myary[i-1][j]))
						if ((myary[i][j-1] >> 4) && (myary[i][j+1] >> 4) && (myary[i+1][j] >> 4) && (myary[i-1][j] >> 4))
							myary[i][j] = 0x4e;
			}

			printf("outH outW %d %d\n", outH, outW);
			for (j=0; j < outH; j++)
				for (i=0; i < outW; i++)
					fputc(myary[i][outH-1-j], I);
			fclose(I);
			printf("outH outW %d %d\n", outH, outW);
			break;

		case 'O':	//default offset, for later. Break 'o' into a function
			myDefaultOffset = strtol(buffer+1, NULL, 16);
			break;

		case 'o':
			myOffset = strtol(buffer+1, NULL, 16);
			fseek(G, myOffset, SEEK_SET);

			for (j=0; j < myH; j++)
				for (i=0; i < myW; i++)
				{
					i2 = (i+myXModOffset) % myW;
					j2 = (j+myYModOffset) % myH;
					ch = fgetc(G) & 0xff;
					if (replace[ch] != -1)
						ch = replace[ch];
					if ((setTransparent) && (ch == transpColor))
						continue;
					if ((i2 >= myXMin) && (i2 < myXMax))
					{
						if (viable(i, j, qflags))
						{
							myary[myX+i2][myY+j2] = ch;
						}
					}
					//not perfect. What if we have fringe AND shift-offset? Oh well.
					if ((i == 0) && (doFringe))
						myary[myX+myW][myY+j2] = myary[myX][myY+j2];
				}
			if (doFringe)
				for (i=0; i < myW+1; i++)
					myary[myX+i][myY+myH] = myary[myX+i][myY];
			myY += myH;
			myXModOffset = 0;
			myYModOffset = 0;
			myXMin = 0;
			myXMax = myW;
			if (addSpace == 1)
				myY++;
			break;

		default:
			printf("Unknown line: %s", buffer);
			break;

		}
	}
	if (G) fclose(G);
	return 0;

}

short isYellow(short x)
{
	switch(x)
	{
	default:
		return 0;
		break;

	case 0x4a:
	case 0x4e:
	case 0x4f:
	case 0x50:
	case 0x56:
	case 0x57:
		return 1;
		break;
	}
	return 0;
}

bool viable (int a1, int a2, int a3)
{
	if (a3 == 15) { return true; }
	if ((a3 == 12) && (a1 % 2 == 0) && (a2 % 2 == 1)) { return true; }
	if ((a3 == 9) && (a1 % 2 == 0) && (a2 % 2 == 0)) { return true; }
	if ((a3 == 6) && (a1 % 2 == 1) && (a2 % 2 == 1)) { return true; }
	if ((a3 == 3) && (a1 % 2 == 0) && (a2 % 2 == 1)) { return true; }
	return 1;
}