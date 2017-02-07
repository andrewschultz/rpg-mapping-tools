#include <stdio.h>
#include <string.h>

short ary[256][16][16];
short icons[100][100];

int main(int argc, char * argv[])
{
	short ch;
	short i, j, k, i2, j2;
	short width, height;
	short bmpHeight;
	short bmpWidth;
	short count = 2;

	char picFile[200];
	char mapFile[200];
	char outFile[200];
	FILE * F;
	FILE * G;
	FILE * H;
	FILE * I;
	if (argc < 2)
	{
		printf("Need 2+ args.\n");
		return 0;
	}

	strcpy(picFile, argv[1]);

	F = fopen(picFile, "rb");
	if (F == NULL)
	{
		printf("%s not found as pic file.\n", picFile);
		return 0;
	}

	fseek(F, 0x100, SEEK_CUR);
	for (k=0; k < 256; k++)
	{
		fgetc(F);
		fgetc(F);
		for (j=0; j < 16; j++)
			for (i=0; i < 8; i++)
			{
				ch = fgetc(F);
				ary[k][i*2][j] = ch & 0xf;
				ary[k][i*2+1][j] = (ch >> 4) & 0xf;
			}
	}

	fclose(F);

	while(count < argc)
	{
	strcpy(mapFile, argv[count]);
	G = fopen(mapFile, "rb");
	if (G == NULL)
	{
		printf("%s not found as MCM file.\n", picFile);
		count++;
		continue;
	}
	printf("Reading %s\n", mapFile);
	width = fgetc(G);
	height = fgetc(G);

	for (j=0; j < height; j++)
		for (i=0; i < width; i++)
			icons[i][height-j-1] = fgetc(G) & 0xff;

	strcpy(outFile, mapFile);
	outFile[strlen(outFile)-3] = 'b';
	outFile[strlen(outFile)-2] = 'm';
	outFile[strlen(outFile)-1] = 'p';

	H = fopen(outFile, "wb");
	I = fopen("c:\\coding\\games\\vga.bmp", "rb");

	bmpWidth = width * 16;
	bmpHeight = height * 16;

	for (i=0; i < 0x436; i++)
	{
		switch(i)
		{
		case 0x12:
			for (j=0; j < 4; j++)
			{
				fputc((bmpWidth >> (8*j)) & 0xff, H);
			}
			fgetc(I);
			break;
		case 0x16:
			for (j=0; j < 4; j++)
			{
				fputc((bmpHeight >> (8*j)) & 0xff, H);
			}
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x17:
		case 0x18:
		case 0x19:
			fgetc(I);
			break;
		default:
			fputc(fgetc(I), H);
			break;
		}
	}
	for (j=0; j < height; j++)
		for (j2 = 0; j2 < 16; j2++)
			for (i=0; i < width; i++)
				for (i2 = 0; i2 < 16; i2++)
				{
					fputc(ary[icons[i][j]][i2][15-j2], H);
				}
	fclose(G);
	fclose(H);
	fclose(I);
	count++;
	}
	return 0;
}