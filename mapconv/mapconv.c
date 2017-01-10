/*Mapconv.c
Takes a bitmap file and a text file on the side and processes them.
Search for HelpBombOut for the parameters it takes in.

In order to process a bitmap successfully, we need the following files:

1. an NMR (new map routine) file which specifies
  1a. the bitmap it wishes to read from
  1b. the PIX file(text based icon file) it wishes to read from
  1c. width of an icon
  1d. height of an icon
  1e. each line of the form [Xi,Yi,Xf,Yf,x.bin,x.bmp]
    1e1. x.bin is a binary dump of the icon numbers, as the bitmap is upside down
    1e2. x.bmp is the name of the output bmp file that shows the detailed map.
  1f. period followed by semicolon. Later this will be reworked to allow multiple bitmaps/PIX files.
  1g. You can also have > to specify a command to run.
2. an optional XTR file with the same base name as the NMR file. The XTR file has the following format:
  > =10
  > =16 base 10/16
  > t- turns transparency off(default)
  > t+ turns it on
  > aa,bb,cc aa,bb base(above) cc(base 16) drop something in
  > # is a comment
  > ; ends it

Future options: set all unused to zero
let 1 icon equal another
let 1 icon = another but with colors switched somehow
1 icon = another rotated or flipped.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUM_ICONS 256

#define HEIGHT_FOUND 1
#define WIDTH_FOUND 2

#define BLACK 0
#define WHITE 1
#define TRANSPARENCYCOLOR BLACK

#define VALID 0
#define THEEND 1
#define INVALID 2

#define BLANKICON 0
#define BLANKCOLOR 8

#define MAXICONSIZE 16

#define MAXSTRING 100

short NewPIXFile;

#define HEADERSIZE 0x436
#define ADJ_HEADER_SIZE 0x40

#define MAPCONV_NO_HEADER_FLAG 1
#define MAPCONV_USE_EGA_HEADER 2
#define MAPCONV_XTRA_AMENDMENTS 4
#define MAPCONV_DEBUG_UNKNOWN_SQUARES 8
#define MAPCONV_USE_TRANSPARENCY 16
#define MAPCONV_BIN_FLAG_KNOWN 32
#define MAPCONV_SORT_PIX 64
#define MAPCONV_DEBUG_ONLY_FIRST 128
#define MAPCONV_SHOW_END_STATS 256
#define MAPCONV_BOTTOMTOP 512
#define MAPCONV_DEBUG_ICONS 1024
#define MAPCONV_PNG_POST 2048
#define MAPCONV_DELETE_PNG_ARTIFACTS 4096

#define NMR_READ_SUCCESS 0
#define NMR_READ_NOFILE 1
#define NMR_READ_PIXCORRUPT 2
#define NMR_READ_NOBMPFILE 3

long MAPCONV_STATUS = 0;

char TheHdr[ADJ_HEADER_SIZE] = {
	0x00, 0x00, 0x00, 0x00,
	(char)0xff, (char)0xff, (char)0xff, 0x00,
	0x00, 0x00, (char)0xff, 0x00,
	0x00, (char)0x0ff, 0x00, 0x00,
	(char)0xff, 0x00, 0x00, 0x00,
	0x00, (char)0x80, (char)0xff, 0x00,
	(char)0xff, 0x00, (char)0xff, 0x00,
	0x00, (char)0xff, (char)0xff, 0x00,
	(char)0x80, (char)0x80, (char)0x80, 0x00,
	(char)0x80, (char)0x80, (char)0xff, 0x00,
	(char)0x80, (char)0x00, (char)0x00, 0x00,
	(char)0xff, (char)0xff, (char)0x80, 0x00,
	(char)0x8c, (char)0xb4, (char)0xd2, 0x00,
	(char)0x13, (char)0x45, (char)0x8b, 0x00,
	(char)0x00, (char)0xff, (char)0x80, 0x00,
	(char)0x00, (char)0x80, (char)0x00, 0x00
};
//0=black 1=white 2=red 3=green 4=blue 5=orange 6=purple 7=yellow
//8=grey 9=pink 10=DkBlue 11=LtBlue 12=LtBrown 13=Brown 14=LtGrn 15=DkGreen

char EgaHdr[ADJ_HEADER_SIZE] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, (char)0xaa,
	0x00, (char)0xaa, 0x00,
	0x00, (char)0xaa, (char)0xaa,
	(char)0xaa, 0x00, 0x00,
	(char)0xaa, 0x00, (char)0xaa,
	(char)0xaa, 0x55, 0x00,
	(char)0xaa, (char)0xaa, (char)0xaa,
	0x55, 0x55, 0x55,
	0x55, 0x55, (char)0xff,
	0x55, (char)0xff, 0x55,
	(char)0xff, 0x55, 0x55,
	0x55, (char)0xff, (char)0xff,
	(char)0xff, 0x55, (char)0xff,
	(char)0xff, (char)0xff, 0x55,
	(char)0xff, (char)0xff, (char)0xff,
};

typedef struct
{
	short printHTMLFile;

	short BlankIcon;
	short BlankColor;
	short TransparencyColor;

	char BmpStr[MAXSTRING];
	char PixStr[MAXSTRING];

	char XtrStr[MAXSTRING];

	char BinStr[MAXSTRING];
	char OutStr[MAXSTRING];

	short TheWidth;
	short TheHeight;

	short Xi;
	short Yi;
	short Xf;
	short Yf;
	
	short ary[640][640];
	short transpary[640][640];

	short Icons[256][16][16];
	short IconUsed[256];

	short IsNewPIX;

	short LastIconViewed;

}
DomesdayBook;

DomesdayBook BmpHandler;

int ReadInIcons(char [MAXSTRING]);
short NMRRead(char [MAXSTRING]);
void AHSHelp();
void putlong(long, FILE *);
void OneIcon(int, char [MAXSTRING], FILE *);
int LatestNumber(FILE *);
int CharToNum(int);
void ReadFromBmp();
void WriteToBmp();
void ModifyArray();
void PrintOutUnused();
void printGrid();

void HelpBombOut();

long InMapH = 256;
long InMapW = 256;

short lineInFile = 0;

short debug = 0;

main(int argc, char * argv[])
{
	char myFile[50];
	short CurComd = 1;

	myFile[0] = 0;

	BmpHandler.BlankColor = BLANKCOLOR;
	BmpHandler.BlankIcon = BLANKICON;
	BmpHandler.TransparencyColor = TRANSPARENCYCOLOR;
	BmpHandler.LastIconViewed = -1;

	BmpHandler.printHTMLFile = 0;

	if (argc < 2)
/*	{
		printf("No arguments, giving help briefing.\n");
		HelpBombOut();
		return 0;
	}
*/
	{
		//MAPCONV_STATUS |= MAPCONV_XTRA_AMENDMENTS;
		//MAPCONV_STATUS |= MAPCONV_USE_TRANSPARENCY;
		printf("Trying default file dmdd.nmr.\n");
		NMRRead("dmdd.nmr");
		return 0;
	}
	for (CurComd = 1; CurComd < argc; CurComd++)
	{
		//printf("%d %s\n", CurComd, argv[CurComd]);
		if (argv[CurComd][0] == '-')
		{
			switch(argv[CurComd][1])
			{
			case '0':
				MAPCONV_STATUS |= MAPCONV_BIN_FLAG_KNOWN;
				printf(".bin file exports known as 0's\n");
				break;

			case 'a':
				AHSHelp();
				return 0;

			case 'b':
				BmpHandler.BlankIcon = (short)strtol(argv[CurComd+1], NULL, 16);
				printf("Blank icon is %x.\n", BmpHandler.BlankIcon);
				CurComd++;
				break;

			case 'c':
				BmpHandler.BlankColor = (short)strtol(argv[CurComd+1], NULL, 16);
				printf("Use default color %x for blank icons.\n", BmpHandler.BlankColor);
				CurComd++;
				break;

			case 'd': // delete intermediary files
				if (argv[CurComd][2] == 'b')
				{
					debug = 1;
					break;
				}
				system("erase *.png.bak");
				system("erase *.png.0*");
				break;

			case 'e':
				CurComd++;
				MAPCONV_STATUS |= MAPCONV_USE_EGA_HEADER;
				printf("Using EGA header.\n");
				break;

			case 'h':
				BmpHandler.printHTMLFile = 1;
				break;

			case 'i':
				MAPCONV_STATUS |= MAPCONV_DEBUG_ICONS;
				printf("Debugging icons.\n");
				break;

			case 'n':
				MAPCONV_STATUS |= MAPCONV_NO_HEADER_FLAG;
				printf("Don't use built in header.\n");
				break;

			case 'p':
				MAPCONV_STATUS |= MAPCONV_PNG_POST;
				printf("Convert generated BMPs to PNG, erasing old file if there.\n");
				if (argv[CurComd][2] == 'a')
				{
					printf("Erasing *.png.bak/001 etc as well.\n");
					MAPCONV_STATUS |= MAPCONV_DELETE_PNG_ARTIFACTS;
				}
				break;

			case 'S':
				MAPCONV_STATUS |= MAPCONV_SORT_PIX;
				printf("Warning you if PIX file is not sorted.\n");
				break;

			case 's':
				MAPCONV_STATUS |= MAPCONV_SHOW_END_STATS;
				//printf("Showing end stats.\n");
				break;

			case 't':
				MAPCONV_STATUS |= MAPCONV_USE_TRANSPARENCY;
				printf("Use transparency.\n");
				if (argv[CurComd][2])
				{
					BmpHandler.TransparencyColor = (short) strtol(argv[CurComd], NULL, 16);
					printf("Transparency color %02x.\n", BmpHandler.TransparencyColor);
				}
				break;

			case 'u':
				MAPCONV_STATUS |= MAPCONV_DEBUG_UNKNOWN_SQUARES;
				if (argv[CurComd][2] == 'f')
					MAPCONV_STATUS |= MAPCONV_DEBUG_ONLY_FIRST;
				break;

			case 'x':
				MAPCONV_STATUS |= MAPCONV_XTRA_AMENDMENTS;
				printf("This run will look for the .xtr file to amend the BMPs.\n");
				break;

			default:
				printf("%s is not a known option, bailing out.\n", argv[CurComd]);

			case '?':
				HelpBombOut();
				return 0;

			}
		}
		else
		{
			if (myFile[0] > 0)
			{
				printf("Already picked out a file to read.\n");
				return 0;
			}
			strcpy(myFile, argv[CurComd]);
		}
	}
	if (myFile[0] == 0)
	{
		printf("You need to input a file, not just flags.\n");
		HelpBombOut();
		return 0;
	}

	if (NMRRead(myFile) != NMR_READ_SUCCESS)
	{
		printf("The NMR file didn't seem to read successfully.\n");
		return 0;
	}

	if (BmpHandler.printHTMLFile > 0)
	{
		char paletteLine[200];
		short q;
		FILE * HT = fopen("palette.htm", "w");

		paletteLine[0] = 0;
		printf("Started writing palette.\n");
		fputs("<html><body>\n<table>\n", HT);
		for (q=0; q < 16; q++)
		{
			sprintf(paletteLine, "<tr><td bgcolor=%02x%02x%02x>Color # %x</td></tr>\n",
				TheHdr[4*q+2] & 0xff, TheHdr[4*q+1] & 0xff, TheHdr[4*q+0] & 0xff, q);
			fputs(paletteLine, HT);
		}
		fputs("</table>\n</body>\n</html>\n", HT);
		printf("Finished writing palette.\n");
	}

	return 0;
}

void HelpBombOut()
{
	printf("Flag -? for this help command.\n\
Flag -0 so bin file puts 0s for known.\n\
Flag -a shows options for AHS files and how to write them.n\
Flag -b specifies blank icon.\n\
Flag -c to set default blank color, in hexadecimal.\n\
Flag -d to get rid of *.png.(bak/000) files.\n\
Flag -db to give debug text (xtr files, etc.).\n\
Flag -h to output html file of the output graphic's palettes.\n\
Flag -n to turn off default header.\n\
Flag -i to show icon debugging.\n\
Flag -nh to show NMR help.\n\
Flag -p to postprocess to png. -pa also erases png.bak/000 files.\n\
Flag -r to reverse when IDing unused icons (default is top to bottom).\n\
Flag -s to show used/unused icon stats at the end.\n\
Flag -S to print out sort warning for icon files.\n\
Flag -t(xx) to flag transparency and specify the color. Black=default.\n\
Flag -u to debug squares with no icon.\n\
Flag -uf to debug squares with no icon, only showing the first.\n\
Flag -v to reVerse the default top-down process of -uf.\n\
Flag -x to add extra modifications to the base BMP files.\n");

}

void AHSHelp()
{
	printf("# means a comment.\n\
h15 sets height to 15. (default 8, max 16)\n\
w13 sets width to 13. (default 8, max 16)\n\
hw/wh/s9 sets height and width to 9. (default 8, max 16)\n\
0x0a=3 sets icon 0x0a to 3 (green).\n\
0x0fc0e copies icon 0x0e to 0x0f.\n\
0x12h03 makes 0x12 a horizontal flip of 03.\n\
0x12h03 makes 0x12 a vertical flip of 03.\n\
\n\
0x0ax34 alternates 3/4 in checkerboard pattern, starting with 3.\n\
0x0a/35 puts 3 in the upper left, 5 in lower right, diagonal. Extra / means 3 along the center.\n\
0x0a\\35 puts 3 in the upper right, 5 in lower left, diagonal. Extra \\ means 3 along the center.\n\
\n\
0x12l06 rotates 06 90 degrees left.\n\
0x12r06 rotates 06 90 degrees right.\n\
0x12H34 horizonally clips an icon by 3, with color 4 trim.\n\
0x12V34 vertically clips an icon by 3, with color 4 trim.\n\
0x12S34 switches 2 colors in an icon, in this case, 3 and 4.\n\
\n\
\'#\' detects the character instead of, say, 0x0a.\n\
> runs something from the command line.\n\
\n\
; ends the file.\n");
}

void NMRHelp()
{
	printf("NMR Help\n\
A quick and dirty guide to NMR files. #'s are comments, which are not allowed, yet.\n\
my-in.bmp\n\
my-in.pix #doesn't need the same prefix\n\
x-init,y-init,x-final,y-final,BINFILE,BMPFILE\n\
Additional lines make more maps\n\
. #flushes the buffer\n\
GT #optional command line stuff\n\
semicolon #ends input\n");

	printf("NMR Help\n");
}

short NMRRead(char FileStr[MAXSTRING])
{
	FILE * G, * F = fopen(FileStr, "r");

	char BufStr[MAXSTRING];
	char * BufStr2;
	char * BufStr3;

	short BufStrCount;
	short ch;

	short i, j;

	if (F == NULL)
	{
		for (i=0; i < (short)strlen(FileStr); i++)
			if (FileStr[i] == '.')
			{
				printf("Empty NMR file.\n");
				return NMR_READ_NOFILE;
			}
		//okay, let's tack on an NMR extension if the file had no extension
		strcpy(BufStr, FileStr);
		strcat(BufStr, ".nmr");
		F = fopen(BufStr, "r");
		if (!F)
		{
			printf("Couldn't find %s or %s. Returning.\n");
			return NMR_READ_NOFILE;
		}
		else
		{
			printf("Tacking on extension.\n");
		}
	}

	for (i=0; i < 256; i++)
		BmpHandler.IconUsed[i] = 0;
	//set all icons as unused

	fgets(BmpHandler.BmpStr, MAXSTRING, F);
	BmpHandler.BmpStr[strlen(BmpHandler.BmpStr)-1] = 0;

	G = fopen(BmpHandler.BmpStr, "rb");

	if (G == NULL)
	{
		printf("Empty BMP file.\n");
		return NMR_READ_NOBMPFILE;
	}

	for (i=0;  i < 0x436;  i++)
	{
		switch (i)
		{
		case 0x12:
			InMapH = fgetc(G);
			break;
		case 0x13:
			InMapH += fgetc(G) * 256;
			break;
		case 0x16:
			InMapW = fgetc(G);
			break;
		case 0x17:
			InMapW += fgetc(G) * 256;
			break;
		default:
			fgetc(G);
			break;
		}
	}

	for (j=0;  j < InMapH;  j++)
		for (i=0;  i < InMapW;  i++)
		{
			BmpHandler.ary[i][InMapH-j-1] = fgetc(G);
			BmpHandler.transpary[i][j] = 0;
		}

	while(1)
	{
	fgets(BmpHandler.PixStr, MAXSTRING, F);

	if (BmpHandler.PixStr[strlen(BmpHandler.PixStr)-1] == '\n')
		BmpHandler.PixStr[strlen(BmpHandler.PixStr)-1] = 0;
	NewPIXFile = 1;

	if (ReadInIcons(BmpHandler.PixStr) == INVALID)
	{
		printf("%s PIX file seems corrupt, possibly missing.\n", BmpHandler.PixStr);
		return NMR_READ_PIXCORRUPT;
	}

	if (BmpHandler.TheWidth == 0)
	{
		fgets(BufStr, MAXSTRING, F);
		BmpHandler.TheWidth = (short)strtol(BufStr, &BufStr2, 10);
	}

	if (BmpHandler.TheHeight == 0)
	{
	fgets(BufStr, MAXSTRING, F);
	BmpHandler.TheHeight = (short)strtol(BufStr, &BufStr2, 10);
	}

//      else printf("Icon read successful.\n");

	ReadFromBmp();

	if (MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS)
		ModifyArray();

	while (1)
	{
		if (fgets(BufStr, MAXSTRING, F) == NULL)
			break;

		ch = BufStr[0];

		if ((BufStr[0] == ';') || (BufStr[0] == ':') || (BufStr[0] == '.'))
			break;

		if (BufStr[0] == '#')
			continue;

        printf("String-read: %s", BufStr);

		BufStrCount = 0;

		BmpHandler.Xi = (short)strtol(BufStr+BufStrCount, &BufStr2, 10);

		while (BufStr[BufStrCount] != ',')
			BufStrCount++;
		BufStrCount++;

		BmpHandler.Yi = (short)strtol(BufStr+BufStrCount, &BufStr2, 10);

		while (BufStr[BufStrCount] != ',')
			BufStrCount++;
		BufStrCount++;

		BmpHandler.Xf = (short)strtol(BufStr+BufStrCount, &BufStr2, 10);

		while (BufStr[BufStrCount] != ',')
			BufStrCount++;
		BufStrCount++;

		BmpHandler.Yf = (short)strtol(BufStr+BufStrCount, &BufStr2, 10);

		while (BufStr[BufStrCount] != ',')
			BufStrCount++;
		BufStrCount++;

		BufStr3 = BufStr + BufStrCount;

		while(BufStr[BufStrCount] != ',')
			BufStrCount++;

		BufStr[BufStrCount] = 0;

		strcpy(BmpHandler.BinStr,BufStr3);
		strcpy(BmpHandler.OutStr,BufStr+BufStrCount+1);
		BmpHandler.OutStr[strlen(BmpHandler.OutStr)-1] = 0;

		//Here, we process one.  If there is a new PIX file we read it in otherwise we just read from the BMP.
		//if bin-string = "X" then don't print a BIN file.

		WriteToBmp();

		NewPIXFile = 0;

	}

	if (MAPCONV_STATUS & MAPCONV_DELETE_PNG_ARTIFACTS)
	{
		system("erase *.png.bak");
		system("erase *.png.0*");
	}

	if ((ch == '.') || (ch == EOF))
		return NMR_READ_SUCCESS;

	fgetc(F);

	}
}

int ReadInIcons(char yzzy[MAXSTRING])
{
	FILE * F = fopen(yzzy, "r");
	short hwdef = 0;
	short temp = 0;
	char buffer[200];
	int i1, i2, i3;
	for (i1 = 0;  i1 < NUM_ICONS;  i1++)
		for (i2 = 0;  i2 < MAXICONSIZE;  i2++)
			for (i3 = 0;  i3 < MAXICONSIZE;  i3++)
                                BmpHandler.Icons[i1][i3][i2] = BmpHandler.BlankColor;

	BmpHandler.TheHeight = 8;
	BmpHandler.TheWidth = 8;

	if (F == NULL)
	{
		printf("%s is a null file to read icons !\n", yzzy);
		return INVALID;
	}

	while(1)
	{
		fgets(buffer, 40, F);
		lineInFile++;
		//printf("Reading %s", buffer);

		temp = (short) strtol(buffer+1, NULL, 10);
		if (temp > MAXICONSIZE)
			temp = MAXICONSIZE;

		switch(buffer[0])
		{
			case 'h':
				if (hwdef & HEIGHT_FOUND)
				{
					printf("H redefined with line %s", buffer);
					return -1;
				}
				if (buffer[1] == 'w')
				{
					if (hwdef & WIDTH_FOUND)
					{
						printf("W redefined with line %s", buffer);
						return -1;
					}
					temp = (short) strtol(buffer+2, NULL, 10);
					if (temp > MAXICONSIZE)
						temp = MAXICONSIZE;
					BmpHandler.TheHeight = BmpHandler.TheWidth = temp;
				}
				else
					BmpHandler.TheHeight = temp;
				break;

			case 'w':
				if (hwdef & WIDTH_FOUND)
				{
					printf("W redefined with line %s", buffer);
					return -1;
				}
				if (buffer[1] == 'h')
				{
					if (hwdef & HEIGHT_FOUND)
					{
						printf("H redefined with line %s", buffer);
						return -1;
					}
					temp = (short) strtol(buffer+2, NULL, 10);
					if (temp > MAXICONSIZE)
						temp = MAXICONSIZE;
					BmpHandler.TheHeight = BmpHandler.TheWidth = temp;
				}
				else
					BmpHandler.TheWidth = temp;
				break;

			case 's':
				if (hwdef)
				{
					printf("H/W redefined with line %s", buffer);
					return -1;
				}
				temp = (short) strtol(buffer+2, NULL, 10);
				if (temp > MAXICONSIZE)
					temp = MAXICONSIZE;
				BmpHandler.TheHeight = BmpHandler.TheWidth = temp;
				break;

			case '\n':	//blank line
				printf("Warning, blank line %d.\n", lineInFile);
				break;

			case '#':	//comment
				break;

			case '0': // hexadecimal representation
				if (buffer[1] != 'x')
				{
					printf("Bad hexadecimal at %d.\n", lineInFile);
					return INVALID;
				}
				OneIcon(CharToNum(buffer[2])*16+CharToNum(buffer[3]), buffer+4, F);
				//printf("Icon %d printed.\n", v);
				break;

			case '\'': // character representation i.e. 'a'
				if (buffer[1] != '\'')
				{
					printf("Bad hexadecimal at %d.\n", lineInFile);
					return INVALID;
				}
				OneIcon(strtol(buffer+3, NULL, 16), buffer+3, F);
				break;

			case '>': //command line
				system(buffer+1);
				break;

			case ';':
				return THEEND;

			default:
				printf("Invalid command line %d: %s", lineInFile, buffer);
				return INVALID;
		}
	}
	return VALID;

}

void ReadFromBmp()
{
	FILE * F1 = fopen(BmpHandler.BmpStr, "rb");
	int i, j;
	for (i=0; i < 0x436; i++)
		fgetc(F1);

	for (i=0;  i < InMapH;  i++)
		for (j=0;  j < InMapW;  j++)
			BmpHandler.ary[j][InMapH-i-1] = fgetc(F1);
	fclose(F1);
}

void WriteToBmp()
{
	long temp;
	FILE * F1 = fopen(BmpHandler.BmpStr, "rb");
	FILE * F2;
	FILE * F3 = fopen(BmpHandler.OutStr, "wb");
	int i, j, i2, j2, j3, count;

	
	for (i = 0;  i < HEADERSIZE;  i++)
	{
        switch(i)
        {
			case 0x12:
				temp = (long)(BmpHandler.Xf-BmpHandler.Xi)*(long)BmpHandler.TheWidth;
				putlong(temp, F3);
				i += 3;
				fgetc(F1);
				fgetc(F1);
				fgetc(F1);
				fgetc(F1);
				break;
			case 0x16:
				putlong((long)(BmpHandler.Yf-BmpHandler.Yi)*(long)BmpHandler.TheHeight, F3);
				i += 3;
				fgetc(F1);
				fgetc(F1);
				fgetc(F1);
				fgetc(F1);
				break;
			case 0x36:
				if (MAPCONV_STATUS & MAPCONV_NO_HEADER_FLAG)
					fputc(fgetc(F1), F3);
				else if (MAPCONV_STATUS & MAPCONV_USE_EGA_HEADER)
				{
					for (j=0; j < ADJ_HEADER_SIZE; j++)
					{
						fgetc(F1);
						fputc(EgaHdr[j], F3);
					}
				}
				else
				{
				for (j=0;  j < ADJ_HEADER_SIZE;  j++)
				{
					count = fgetc(F1);
					fputc(TheHdr[j], F3);
				}
				i += (ADJ_HEADER_SIZE - 1);
				}
				break;
            default:
                fputc(fgetc(F1), F3);
                break;
         }
 	}

	if (BmpHandler.BinStr[0] != 0)
	{
		F2 = fopen(BmpHandler.BinStr, "wb");

		if (debug)
			printf("Writing binary file to %s.\n", BmpHandler.BinStr);

		if (MAPCONV_STATUS & MAPCONV_BIN_FLAG_KNOWN)
		{
			for (i = BmpHandler.Yi;  i < BmpHandler.Yf;  i++)
				for (j = BmpHandler.Xi;  j < BmpHandler.Xf;  j++)
					if (BmpHandler.IconUsed[BmpHandler.ary[j][i]])
						fputc(0, F2);
					else
						fputc(BmpHandler.ary[j][i], F2);
		}
		else
		{
			for (i = BmpHandler.Yi;  i < BmpHandler.Yf;  i++)
				for (j = BmpHandler.Xi;  j < BmpHandler.Xf;  j++)
					fputc(BmpHandler.ary[j][i], F2);
		}
		fclose(F2);
	}
	else
	{
		if (debug)
			printf("Not writing a binary file.\n");
	}

	for (j = BmpHandler.Yi;  j < BmpHandler.Yf;  j++)
		for (j2 = 0;  j2 < BmpHandler.TheHeight;  j2++)
		{
			for (i = BmpHandler.Xi;  i < BmpHandler.Xf;  i++)
				for (i2 = 0;  i2 < BmpHandler.TheWidth;  i2++)
				{
					short xyz=BmpHandler.Icons[BmpHandler.transpary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]][i2][BmpHandler.TheHeight-j2-1];
					if ((BmpHandler.transpary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]) && (xyz != BmpHandler.TransparencyColor))
						fputc((char)xyz, F3);
					else
						fputc((char)BmpHandler.Icons[BmpHandler.ary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]][i2][BmpHandler.TheHeight-j2-1], F3);
				}
                  if (temp % 4)
                    for (j3=(temp%4);  j3<4;  j3++)
			    fputc(0, F3);
		}
	fclose(F1);
	fclose(F3);

	if (MAPCONV_STATUS & MAPCONV_PNG_POST)
	{
		char pngString[80];
		char myCmd[100];

		short len;

		FILE *fp;

		strcpy(pngString, BmpHandler.OutStr);
		len = strlen(pngString);
		pngString[len-3] = 'p';
		pngString[len-2] = 'n';
		pngString[len-1] = 'g';

		fp = fopen (pngString, "r");
		if (fp!=NULL)
		{
		fclose (fp);
		sprintf(myCmd, "erase %s", pngString);
		system(myCmd);
		}
		sprintf(myCmd, "bmp2png -9 %s", BmpHandler.OutStr);
		system(myCmd);
	}

	if (MAPCONV_STATUS & MAPCONV_DEBUG_UNKNOWN_SQUARES)
		PrintOutUnused();
}

void PrintOutUnused()
{
	long j, i, k;
	short used[256] = {0};
	short usedAtAll[256] = {0};
	long totalUnused = 0;
	long totalUsed = 0;
	short unusedBaseIcons = 0;
	short usedBaseIcons = 0;

	printf("Roll call for unused icons:\n");

	printGrid();

	for (j = BmpHandler.Yi; j < BmpHandler.Yf; j++)
	{
		k=j;
		if (MAPCONV_STATUS & MAPCONV_BOTTOMTOP)
		{
			k = BmpHandler.Yf + BmpHandler.Yi - k - 1;
		}
		for (i = BmpHandler.Xi; i < BmpHandler.Xf; i++)
		{ //printf("%2x", BmpHandler.ary[i][j]);
			if (!BmpHandler.IconUsed[BmpHandler.ary[i][k]])
			{
				if ((MAPCONV_STATUS & MAPCONV_DEBUG_ONLY_FIRST) && (used[BmpHandler.ary[i][k]]))
				{
				}
				else
					printf("%02x %02x Unused icon %x also %d %d.\n", i, k, BmpHandler.ary[i][k], i-BmpHandler.Xi, k-BmpHandler.Yi);
				used[BmpHandler.ary[i][k]]++;
				if (BmpHandler.IconUsed[BmpHandler.ary[i][k]] == 0)
					unusedBaseIcons++;
				BmpHandler.IconUsed[BmpHandler.ary[i][k]] = 2;
				totalUnused++;
			}
			else
			{
				if (usedAtAll[BmpHandler.ary[i][k]] == 0)
				{
					usedAtAll[BmpHandler.ary[i][k]] = 1;
					usedBaseIcons++;
				}
				if (BmpHandler.IconUsed[BmpHandler.ary[i][k]] == 2)
					totalUnused++;
				else
					totalUsed++;
			}
		}
//		printf("\n");
	}
	if (MAPCONV_STATUS & MAPCONV_SHOW_END_STATS)
	{
		float q = ((float)(totalUsed*100))/(totalUsed+totalUnused);
	    printf("Total stats:\n");
		printf("%d of %d usable icons in final map, for %f percent.\n", totalUsed, totalUsed+totalUnused, q);
		printf("%d unused based icons, %d used.\n", unusedBaseIcons, usedBaseIcons);
	}
	if (totalUnused > 0)
		printGrid();
}

void printGrid()
{
	short i;

	for (i=0; i < 256; i++)
	{
		printf("%d", BmpHandler.IconUsed[i]);
		if (i%16 == 15)
			printf("\n");
	}
}

void ModifyArray()
{
	short u = strlen(BmpHandler.BmpStr);

	FILE * F;

	long xc, yc, nv, i, j, myBase = 10;
	long xi=0, yi=0, x2=0, y2=0;
	long count;
	
	char buffer[200];
	char * SecondString;
	short XtrTransparencyRead = 0;
	short everBase = 0;

	strcpy(BmpHandler.XtrStr, BmpHandler.BmpStr);

	BmpHandler.XtrStr[u-3] = 'x';
	BmpHandler.XtrStr[u-2] = 't';
	BmpHandler.XtrStr[u-1] = 'r';

	F = fopen(BmpHandler.XtrStr, "r");

	if (F == NULL)
	{
		printf("No such xtr file as %s.\n", BmpHandler.XtrStr);
		return;
	}
	else
		printf("Reading %s.\n", BmpHandler.XtrStr);

	while (fgets(buffer, 200, F))
	{
		switch(buffer[0])
		{
		case ';':
			goto evercheck;
			break;

		case '=':
			myBase = strtol(buffer+1, &SecondString, 10);
			everBase = 1;
			break;

		case 't':
			if (buffer[1] == '-')
				XtrTransparencyRead = 0;
			else
				XtrTransparencyRead = 1;
			break;

		case 'r':
			{
				long x1, y1, x2, y2, defColor;

				x1=strtol(buffer+1, &SecondString, myBase);
				y1=strtol(buffer+4, &SecondString, myBase);
				x2=strtol(buffer+7, &SecondString, myBase);
				y2=strtol(buffer+10, &SecondString, myBase);
				defColor=strtol(buffer+13, &SecondString, myBase);

				for (j=0; j < y2; j++)
					for (i=0; i < x2; i++)
						BmpHandler.ary[x1+i][y1+j] = (short)defColor;
				break;
			}
		case '~':
			//swap bytes
		case '>':
			{	//copy from one set of bytes to another
				long x1, y1, x2, y2, x3, y3;
				short temp;
				x1=strtol(buffer+1, &SecondString, myBase);
				y1=strtol(buffer+4, &SecondString, myBase);
				x2=strtol(buffer+7, &SecondString, myBase);
				y2=strtol(buffer+10, &SecondString, myBase);
				x3=strtol(buffer+13, &SecondString, myBase);
				y3=strtol(buffer+16, &SecondString, myBase);

				if (buffer[0] == '>')
				{
					for (j=0; j < y3; j++)
						for (i=0; i < x3; i++)
						{
							BmpHandler.ary[x2+i][y2+j] = BmpHandler.ary[x1+i][y1+j];
							BmpHandler.transpary[x2+i][y2+j] = BmpHandler.transpary[x1+i][y1+j];
						}
				}
				else
				{
					for (j=0; j < y3; j++)
						for (i=0; i < x3; i++)
						{
							temp = BmpHandler.ary[i+xi][j+yi];
							BmpHandler.ary[i+xi][j+yi] = BmpHandler.ary[i+x2][j+y2];
							BmpHandler.ary[i+x2][j+y2] = temp;

							temp = BmpHandler.transpary[i+xi][j+yi];
							BmpHandler.transpary[i+xi][j+yi] = BmpHandler.transpary[i+x2][j+y2];
							BmpHandler.transpary[i+x2][j+y2] = temp;
						}
				}
				break;
			}

		case 'x':
			xi = strtol(buffer+1, &SecondString, myBase);
			break;
		case 'y':
			yi = strtol(buffer+1, &SecondString, myBase);
			break;

		case '#':
			break;

		case 'i':
			if (buffer[1] != '\n')
				count = strtol(buffer+1, NULL, 10);
			else count = 1;
			yc-= count;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
				BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			else
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
			break;

		case 'j':
			if (buffer[1] != '\n')
				count = strtol(buffer+1, NULL, 10);
			else count = 1;
			xc-= count;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
				BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			else
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
			break;

		case 'k':
			if (buffer[1] != '\n')
				count = strtol(buffer+1, NULL, 10);
			else count = 1;
			xc+= count;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
				BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			else
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
			break;

		case 'm':
			if (buffer[1] != '\n')
				count = strtol(buffer+1, NULL, 10);
			else count = 1;
			yc+= count;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
				BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			else
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			xc = strtol(buffer, &SecondString, myBase);
			count = 0;
			while (buffer[count] && (buffer[count] != ','))
				count++;
			if (!buffer[count]) { printf("XTR file string too short: %s", buffer); break; }

			yc = strtol(buffer+count+1, &SecondString, myBase);

			count++;

			while (buffer[count] && (buffer[count] != ',') && (buffer[count] != '='))
				count++;
			if (!buffer[count]) { printf("XTR file string too short: %s", buffer); break; }

			if (buffer[count] == '=')
				nv = (char)buffer[count+1] & 0xff;	//ie =T, character stuff
			else
				nv = strtol(buffer+count+1, &SecondString, 16);

			if (debug > 0)
			{
				if (debug == 2)
					printf("xc %d xi %d yc %d yi %d, base %d\n", xc, xi, yc, yi, myBase);
				printf("Array[%d][%d]=%d with char %d from string %s", xc+xi, yc+yi, nv, count, buffer);
			}

//			BmpHandler.ary[xc][255-yc] = nv;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
				BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			else
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
			break;

		default:
			printf("Xtr file has faulty line:\n%s", buffer);
			break;
		}

		if (buffer[0] == ';')
			break;

	}

evercheck:
	fclose(F);
	if (everBase == 0)
	{
		printf("Note, you never defined a base with =##, so I assumed base 10.\n");
	}
	return;
}

void putlong(long x, FILE * F)
{
	int i;
	for (i=0;  i < 4;  i++)
		fputc((x>>(i*8))&0xff, F);
}

void OneIcon(int q, char myBuf[MAXSTRING], FILE * F)
{
	int i,j;
	short tst, tst2;
	char buffer[MAXSTRING];
	short startLoc = 1;

	if (MAPCONV_STATUS & MAPCONV_DEBUG_ICONS)
		printf("Icon-read %x:%c\n", q, myBuf[0] == '\n' ? '.' : myBuf[0]);

	if (MAPCONV_STATUS & MAPCONV_SORT_PIX)
		if (q < BmpHandler.LastIconViewed)
			printf("PIX file not sorted. Icon %x is after %x.\n", q, BmpHandler.LastIconViewed);

	BmpHandler.LastIconViewed = q;

	BmpHandler.IconUsed[q] = 1;

	tst = (short) strtol(myBuf+1, NULL, 16);

	switch(myBuf[0])
	{
	case ' ': //ok, if they don't see the space, pretend it's a return, for now
	case '\n':
		break;

	case '=':	//changes one icon to all one color.
		for (j=0;  j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][i][j] = tst;
		return;

	case '/': // upper left is color #1, double slash means diagonal is too
		if (myBuf[startLoc] == '/')
			startLoc++;
		tst = CharToNum(myBuf[startLoc]);
		tst2 = CharToNum(myBuf[startLoc+1]);
		if (tst == '/')
			startLoc++;
		for (j=0;  j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				if (i + j < BmpHandler.TheHeight - 1)
					BmpHandler.Icons[q][i][j] = tst;
				else if (i + j > BmpHandler.TheHeight - 1)
					BmpHandler.Icons[q][i][j] = tst2;
				else if (startLoc == 2)
					BmpHandler.Icons[q][i][j] = tst;
				else
					BmpHandler.Icons[q][i][j] = tst2;
		return;

	case '\\': // upper right is color #1, double slash means diagonal is too
		if (myBuf[startLoc] == '\\')
			startLoc++;
		tst = CharToNum(myBuf[startLoc]);
		tst2 = CharToNum(myBuf[startLoc+1]);
		for (j=0;  j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				if (i > j)
					BmpHandler.Icons[q][i][j] = tst;
				else if (i < j)
					BmpHandler.Icons[q][i][j] = tst2;
				else if (startLoc == 2)
					BmpHandler.Icons[q][i][j] = tst;
				else
					BmpHandler.Icons[q][i][j] = tst2;
		return;

	case 'c': //copies one icon to another
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][i][j] = BmpHandler.Icons[tst][i][j];
		return;

	case 'e': // 180 degree rotation
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.TheWidth-1-i][BmpHandler.TheHeight-1-j] = BmpHandler.Icons[tst][i][j];
		return;

	case 'f': // flip across the SW/NE diagonal
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.TheWidth-1-j][BmpHandler.TheHeight-1-i] = BmpHandler.Icons[tst][i][j];
		return;

	case 'F': // flip across the SE/NW diagonal
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][j][i] = BmpHandler.Icons[tst][i][j];
		return;

	case 'h': //copies one icon to another, flipped horizontally
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.TheWidth-1-i][j] = BmpHandler.Icons[tst][i][j];
		return;

	case 'l': //rotate 90 degrees left
		if (BmpHandler.TheHeight != BmpHandler.TheWidth)
		{
			printf ("Need height = width to rotate.\n");
			return;
		}
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][j][BmpHandler.TheHeight-1-i] = BmpHandler.Icons[tst][i][j];
		return;

	case 'r': //rotate 90 degrees right
		if (BmpHandler.TheHeight != BmpHandler.TheWidth)
		{
			printf ("Need height = width to rotate.\n");
			return;
		}
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.TheWidth-1-j][i] = BmpHandler.Icons[tst][i][j];
		return;

	case 'v': //copies one icon to another, flipped vertically
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				BmpHandler.Icons[q][i][BmpHandler.TheHeight-1-j] = BmpHandler.Icons[tst][i][j];
		return;

	case 'x': //alternating checkerboard colors
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		fgetc(F);
		for (j=0;  j < BmpHandler.TheHeight; j++)
			for (i=0;  i < BmpHandler.TheWidth;  i++)
				if ((i+j)%2)
					BmpHandler.Icons[q][i][j] = tst2;
				else
					BmpHandler.Icons[q][i][j] = tst;
		return;

	case 'H': //horizontal trimming, with color after
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0; i < tst2; i++)
				BmpHandler.Icons[q][i][j] = BmpHandler.Icons[q][BmpHandler.TheWidth-i-1][j] = tst;
		return;

	case 'V': //vertical trimming, with color after
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		for (j=0; j < tst2; j++)
			for (i=0; i < BmpHandler.TheWidth; i++)
				BmpHandler.Icons[q][i][j] = BmpHandler.Icons[q][i][BmpHandler.TheHeight-j-1] = tst;
		return;

	case 'S': //switch 2 colors. Actually, you can switch with a null color with no problem.
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		for (j=0; j < BmpHandler.TheHeight; j++)
			for (i=0; i < BmpHandler.TheWidth; i++)
				if ((BmpHandler.Icons[q][i][j] == tst) || (BmpHandler.Icons[q][i][j] == tst2))
					BmpHandler.Icons[q][i][j] = tst + tst2 - BmpHandler.Icons[q][i][j];
		return;

	default:
		printf("Unknown command for icon %x: %s", q, myBuf);
		return;
	}
	for (j=0;  j < BmpHandler.TheHeight; j++)
	{
		lineInFile++;
		fgets(buffer, 40, F);
		for (i=0;  i < BmpHandler.TheWidth;  i++)
                {
                        BmpHandler.Icons[q][i][j] = CharToNum(buffer[i]);
                        if ((buffer[i] > '9') || (buffer[i] < '0'))
						  if ((buffer[i] > 'Z') || (buffer[i] < 'A'))
							if ((buffer[i] > 'z') || (buffer[i] < 'a'))
							{
                              printf("Reading icon 0x%x(%d), bogus char 0x%x at %d,%d in icon, line %d.\n",
								  q, q, buffer[i], i, j, lineInFile);
							  if ( buffer[i] == 0xa)
								  printf("Likely early carriage return.\n");
							}
                }
	}
}

int LatestNumber(FILE * F)
{
        int jjj;
        jjj = (16*CharToNum(fgetc(F)) + CharToNum(fgetc(F)));
        return jjj;
}

int CharToNum(int z)
{
	if ((z >= 'a') && (z <= 'z'))
		return (z-'a'+10);
	if ((z >= 'A') && (z <= 'Z'))
		return (z-'A'+10);
	return (z - '0');
}

