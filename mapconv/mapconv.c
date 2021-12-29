 /*Mapconv.c
Takes a bitmap file and a text file on the side and processes them.
Search for HelpBombOut for the parameters it takes in.
AHSHelp gives parameters for an AHS file.

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
let 1 icon = another but with colors switched somehow

Want to check next we can change AHS or BMP in the middle of an NMR file.
Maybe have option to start with a certain line or end with it as well.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>

#include "mapconv-help.h"

#define NUM_ICONS 256

#define HEIGHT_FOUND 1
#define WIDTH_FOUND 2

#define BLACK 0
#define WHITE 1
#define GREY 8
#define TRANSPARENCYCOLOR BLACK

#define VALID 0
#define THEEND 1
#define INVALID 2

#define BLANKICON 0
#define BLANKCOLOR 8

#define TRANSPARENT_LCD 2

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
#define MAPCONV_BOTTOM_TOP 512
#define MAPCONV_DEBUG_ICONS 1024
#define MAPCONV_PNG_POST 2048
#define MAPCONV_DELETE_PNG_ARTIFACTS 4096
#define MAPCONV_IGNORE_BINS 8192
#define MAPCONV_REGENERATE_BASE_FILE 16384
#define MAPCONV_XTRA_AMENDMENTS_ALT_NAME 32768
#define MAPCONV_NOTE_NO_XTR 65536
#define MAPCONV_SHOW_FREQ_STATS_USED 131072
#define MAPCONV_SHOW_FREQ_STATS_UNUSED 262144
#define MAPCONV_SHOW_FREQ_STATS 393216
#define MAPCONV_DEBUG_SHOW_EDGE_UNDEF 524288

#define NMR_READ_SUCCESS 0
#define NMR_READ_NOFILE 1
#define NMR_READ_PIXCORRUPT 2
#define NMR_READ_NOBMPFILE 3
#define NMR_READ_OLDVERSION 4
#define NMR_READ_BAD_LINE 5
#define NMR_READ_BAD_MACRO 6

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

//the LCDs are from 0 to F
//B is a tricky one since it could be equivalent to 6 or 8.
//
//   1
// 2  4
//   8
//16  32
//  64
//
//128 = line down the middle
//diagonals are not implemented yet but...
//256 = DR, 512 = DL

short LCDs[36] = {
	119, 128, 93, 109, //0-3
	46, 107, 123, 37, //4-7
	127, 47, 63, 122, //8-b
	83, 124, 91, 27, // c-f
	47, 62, 193, 100, //g-j
	0, 18, 182, 55, //k-n
	119, 31, 47, 19, //o-r
	107, 129, 118, 0, //s-v
	0, 768, 110, 577, //w-z
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

	short IconWidth;
	short IconHeight;

	int Xi;
	int Yi;
	int Xf;
	int Yf;

	short ary[640][640];
	short checkAry[640][640];
	short transpary[640][640];

	short Icons[257][16][16]; // the extra icon space is for rotating an icon about itself
	short IconUsed[256];
	short IconDefined[256];
	short IconIsZigzag[256];

	short freqAry[256];

	short IsNewPIX;

	short LastIconViewed;

	//for XTR files
	long startIconBase;
	long iconNumBase;
	long mainXtrBase;

	int cutUp;
	int cutDown;
	int cutLeft;
	int cutRight;
	char cutColor;
	short cutNext;

	long unknownLCDColor;

}
DomesdayBook;

DomesdayBook BmpHandler;

//Function Declaractions
int ReadInIcons(char [MAXSTRING], short reset);
short NMRRead(char [MAXSTRING]);
void putlong(long, FILE *);
void OneIcon(int, char [MAXSTRING], FILE *);
short otherIcon(char x);

void unknownToLCD();
void LCDize(short whichNum, short whichIcon, short allowPrevDefined, short xi, short yi, short dx, short dy, short clearBefore);
int LatestNumber(FILE *);
void ReadRawData();
int CharToNum(int);
void ReadFromBmp();
void WriteIconsToBmp();
void WriteToBmp();
void ModifyArray(char [MAXSTRING]);
void PrintOutUnused();
void runFreqStats();
int comp (const void * elem1, const void * elem2);
void printGrid();
void snip();

//Globals with defaults
short InMapH = 256;
short InMapW = 256;

short lineInFile = 0;

short debug = 0;

short lcd_fill = 1;
short lcd_blank = 0;

short curBase = 16;

short foundExtra = 0;
short fillUnknownWithLCD;

short transpUnder = 0;
short xyphus = 0;

short printIconBmp = 0;
short iconOutBorderColor = GREY;

main(int argc, char * argv[])
{
	char myFile[50];
	short CurComd = 1;
	short i;

	myFile[0] = 0;

	memset(&BmpHandler, 0, sizeof(BmpHandler));

	BmpHandler.BlankColor = BLANKCOLOR;
	BmpHandler.BlankIcon = BLANKICON;
	BmpHandler.TransparencyColor = TRANSPARENCYCOLOR;
	BmpHandler.LastIconViewed = -1;

	BmpHandler.cutNext = 0;

	BmpHandler.printHTMLFile = 0;

	BmpHandler.iconNumBase = 16;
	BmpHandler.startIconBase = 10;
	BmpHandler.mainXtrBase = 10;

	BmpHandler.IconHeight = BmpHandler.IconWidth = 0;

	BmpHandler.unknownLCDColor = 0x808080;

	for (i=0; i < 256; i++)
		BmpHandler.IconDefined[i] = 0;

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
				if (argv[CurComd][2] == 'u')
				{
					MAPCONV_STATUS |= MAPCONV_DEBUG_SHOW_EDGE_UNDEF;
					break;
				}

				if (argv[CurComd][2] == 'f')
				{
					fillUnknownWithLCD = 1;
					if (argv[CurComd][3] == '=')
						BmpHandler.unknownLCDColor = strtol(argv[CurComd]+3, NULL, 16);
					break;
				}
				AHSHelp();
				return 0;

			case 'b':
				BmpHandler.BlankIcon = (short)strtol(argv[CurComd+1], NULL, 16);
				printf("Blank icon is %x.\n", BmpHandler.BlankIcon);
				break;

			case 'c':
				if (argv[CurComd][2] == 'h')
				{
					printf("0=black 1=white 2=red 3=green 4=blue 5=orange 6=purple 7=yellow\n");
					printf("8=grey 9=pink 10=DkBlue 11=LtBlue 12=LtBrown 13=Brown 14=LtGrn 15=DkGreen\n");
					return 0;
				}
				BmpHandler.BlankColor = (short)strtol(argv[CurComd+1], NULL, 16);
				printf("Use default color %x for blank icons.\n", BmpHandler.BlankColor);
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
				if (argv[CurComd][2] == 'b')
				{
					printf("Erasing .bin files.\n");
					system("erase *.bin");
					break;
				}
				if (argv[CurComd][2] == 'i')
				{
					printf("Erasing .bin files, ignoring creation.\n");
					MAPCONV_STATUS |= MAPCONV_IGNORE_BINS;
					system("erase *.bin");
					break;
				}
				MAPCONV_STATUS |= MAPCONV_USE_EGA_HEADER;
				printf("Using EGA header.\n");
				break;

			case 'h':
				BmpHandler.printHTMLFile = 1;
				break;

			case 'i':

				if (argv[CurComd][2] == 'o')
				{
					printIconBmp = 1;
					if (argv[CurComd][3])
						iconOutBorderColor = strtol(argv[CurComd+3], NULL, 16) && 0xff;
					break;
				}

				if (argv[CurComd][2] == 's')
				{
					if (argc > CurComd)
						BmpHandler.IconHeight = BmpHandler.IconWidth = (short)strtol(argv[CurComd+1], NULL, 10);
					else
						printf("-i(s/h/w) requires a numerical argument after.");
					CurComd++;
					break;
				}

				if (argv[CurComd][2] == 'h')
				{
					if (argc > CurComd)
						BmpHandler.IconHeight = (short)strtol(argv[CurComd+1], NULL, 10);
					else
						printf("-i(s/h/w) requires a numerical argument after.");
					CurComd++;
					break;
				}

				if (argv[CurComd][2] == 'w')
				{
					if (argc > CurComd)
						BmpHandler.IconWidth = (short)strtol(argv[CurComd+1], NULL, 10);
					else
						printf("-i(s/h/w) requires a numerical argument after.");
					CurComd++;
					break;
				}

				if (argv[CurComd][2] == 'b')
				{
					MAPCONV_STATUS |= MAPCONV_IGNORE_BINS;
					break;
				}

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

			case 'r':
				MAPCONV_STATUS |= MAPCONV_REGENERATE_BASE_FILE;
				break;

			case 'R':
				MAPCONV_STATUS |= MAPCONV_BOTTOM_TOP;
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
				if (argv[CurComd][2] == 'y')
				{
					xyphus = 1;
					break;
				}
				if (argv[CurComd][2] == '0')
				{
					MAPCONV_STATUS |= MAPCONV_NOTE_NO_XTR;
					break;
				}
				MAPCONV_STATUS |= MAPCONV_XTRA_AMENDMENTS;
				printf("This run will look for the .xtr file to amend the BMPs.\n");
				if (argv[CurComd][2] == 'n')
				{
					MAPCONV_STATUS |= MAPCONV_XTRA_AMENDMENTS_ALT_NAME;
					printf("Also, a.bmp (for example) will now be a-xtr.bmp.\n");
				}
				break;

			case '#':
				if (argv[CurComd][2] == 0)
					MAPCONV_STATUS |= MAPCONV_SHOW_FREQ_STATS;
				else if (argv[CurComd][2] == 'n')
					MAPCONV_STATUS |= MAPCONV_SHOW_FREQ_STATS_USED;
				else if (argv[CurComd][2] == 'u')
					MAPCONV_STATUS |= MAPCONV_SHOW_FREQ_STATS_UNUSED;
				else
					HelpBombOut();
				break;

			default:
				printf("%s is not a known option, bailing out.\n", argv[CurComd]);

			case '?':
				if (argv[CurComd][2] == '?')
					helpMinorFeatures();
				else
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

	if (!foundExtra && (MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS))
		printf("MAPCONV WARNING: You ran the -x option but the NMR file didn't reference a proper XTR file.\n");

	if (foundExtra && !(MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS))
		printf("MAPCONV WARNING: The NMR file referenced an extra file but you didn't run the -x option.\n");

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

short NMRRead(char FileStr[MAXSTRING])
{
	FILE * G, * F = fopen(FileStr, "r");

	char BufStr[MAXSTRING];
	char * BufStr2;
	char bufLower;
	char wh_help[100];

	short thisLine = 0;
	long buflen = 0;

	short i;

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
			printf("Couldn't find %s or %s. Returning.\n", FileStr, BufStr);
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

	while (1)
	{
		thisLine++;
		fgets(BufStr, MAXSTRING, F);
		buflen = strlen(BufStr);

		if (BufStr[0] == '#')
			continue;
		if (BufStr[0] == ';')
			break;
		if (!BufStr[0])
			break;

		if (BufStr[0] == '\n')
			printf("MAPCONV WARNING: line %d is blank.\n", thisLine);

		if (BufStr[0] == '.')
		{
			printf("MAPCONV WARNING: period (.) on line %d is a relic of old NMR versions. Skipping.\n", thisLine);
			continue;
		}

		if (BufStr[1] != '=')
		{
			printf("MAPCONV WARNING: new NMR routines need (char)= for each line. Change line %d=%s\n", thisLine, BufStr);
			return NMR_READ_OLDVERSION;
		}

		snip(BufStr);

		bufLower = BufStr[0] | 0x20;
		switch(bufLower)
		{

		case 'a':
			if (BufStr[1] == '=')
				BufStr2 = BufStr + 2;
			else
			{
				strcpy(BufStr2, FileStr);
				BufStr2[strlen(BufStr2)-4] = 0;
			}
			strcpy(BmpHandler.PixStr, BufStr2);
			strcat(BmpHandler.PixStr, ".ahs");
			if(_access(BmpHandler.PixStr, 0) == 0)
			{
				strcpy(BmpHandler.PixStr, BufStr2);
				strcat(BmpHandler.PixStr, ".pix");
				if(_access(BmpHandler.PixStr, 0) == 0)
				{
					printf("Could not find %s.pix/.ahs file, bailing.", BufStr);
					return NMR_READ_BAD_MACRO;
				}
			}
			strcpy(BmpHandler.BmpStr, BufStr2);
			strcat(BmpHandler.BmpStr, ".bmp");
			ReadRawData();

			if (ReadInIcons(BmpHandler.PixStr, 1) == INVALID)
			{
				printf("%s PIX file seems corrupt, possibly missing.\n", BmpHandler.PixStr);
				return NMR_READ_PIXCORRUPT;
			}

			strcpy(BmpHandler.XtrStr, BufStr);
			strcat(BmpHandler.XtrStr, ".xtr");

			if (MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS)
				ModifyArray(BmpHandler.XtrStr);

		case 'b':
			{
				int temp;
				char * token;
				char seps[] = ",";
				token = strtok(BufStr+2, seps);

				sscanf(token, "%d", &BmpHandler.cutLeft);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &BmpHandler.cutUp);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &BmpHandler.cutRight);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &BmpHandler.cutDown);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &temp);
				BmpHandler.cutColor = (char)temp;

				if (token == NULL)
				{
					printf("You need five arguments to block out a rectangle.\n");
					break;
				}
				if (BmpHandler.cutDown - BmpHandler.cutUp > 0x80)
				{
					printf("Cut area too high.\n");
					break;
				}
				if (BmpHandler.cutRight - BmpHandler.cutLeft > 0x80)
				{
					printf("Cut area too wide.\n");
					break;
				}
				BmpHandler.cutNext = 1;
			}
			break;

		case 'c': //run a command
			if (MAPCONV_REGENERATE_BASE_FILE)
				system(BufStr+2);
			else
				printf("MAPCONV NOTE: Skipping %s since regenerate_base_file is set to false.", BufStr + 2);
			break;

		case 'h':
			if (BmpHandler.IconWidth != 0)
				printf("Line %d redefines width.\n", thisLine);
			BmpHandler.IconWidth = (short)strtol(BufStr, &BufStr2, 10);
			break;

		case 'i': //read in an icons file
			if (strcmp(BufStr+buflen-4, "ahs") && strcmp(BufStr+buflen-4, "pix"))
				printf("MAPCONV WARNING: wrong extension for icons file.\n");
			strcpy(BmpHandler.PixStr, BufStr + 2);
			snip(BmpHandler.PixStr);

			if (ReadInIcons(BmpHandler.PixStr, 1) == INVALID)
			{
				printf("MAPCONV ERROR: %s PIX file seems corrupt, possibly missing.\n", BmpHandler.PixStr);
				return NMR_READ_PIXCORRUPT;
			}
			break;

        case 'd':
		case 'o': //output a BMP file
			{
				char * token;
				char seps[] = ",";
				token = strtok(BufStr+2, seps);

				sscanf(token, "%d", &BmpHandler.Xi);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &BmpHandler.Yi);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &BmpHandler.Xf);
				token = strtok(NULL, seps);

				sscanf(token, "%d", &BmpHandler.Yf);
				token = strtok(NULL, seps);

				sscanf(token, "%s", &BmpHandler.OutStr);
				token = strtok(NULL, seps);

				if (bufLower == 'd')
                {
                    BmpHandler.Yf += BmpHandler.Yi;
                    BmpHandler.Xf += BmpHandler.Xi;
                }

                strcpy(wh_help, bufLower == 'd' ? " You may wish to switch d= to o=." : "");

                if (BmpHandler.Xi > InMapW)
                    printf("MAPCONV WARNING: %s x-min of %d more than width of map it is read from, %d.%s\n", BmpHandler.OutStr, BmpHandler.Xi, InMapW, wh_help);
                else if (BmpHandler.Xf > InMapW)
                    printf("MAPCONV WARNING: %s x-max of %d more than width of map it is read from, %d.%s\n", BmpHandler.OutStr, BmpHandler.Xf, InMapW, wh_help);

                if (BmpHandler.Yi > InMapH)
                    printf("MAPCONV WARNING: %s y-min of %d more than height of map it is read from, %d.%s\n", BmpHandler.OutStr, BmpHandler.Yi, InMapH, wh_help);
                else if (BmpHandler.Yf > InMapH)
                    printf("MAPCONV WARNING: %s y-max of %d more than height of map it is read from, %d.%s\n", BmpHandler.OutStr, BmpHandler.Yf, InMapH, wh_help);

				BmpHandler.BinStr[0] = (char)0x00;

				if (token)
				{
					sscanf(token, "%s", &BmpHandler.BinStr);
					snip(BmpHandler.BinStr);
				}
				else
					snip(BmpHandler.BinStr);
				WriteToBmp();
				if (printIconBmp)
					WriteIconsToBmp();

				NewPIXFile = 0;

			}
			break;

		case 'r': //read in a raw-data file
			if (strcmp(BufStr+buflen-4, "bmp"))
				printf("WARNING: wrong extension for BMP file.\n");
			strcpy(BmpHandler.BmpStr, BufStr + 2);
			ReadRawData();
			break;

		case 'x': //read in an XTR file
			if (strcmp(BufStr+buflen-4, "xtr"))
				printf("WARNING: wrong extension for XTR file.\n");
			strcpy(BmpHandler.XtrStr, BufStr + 2);
			snip(BmpHandler.XtrStr);

			if (MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS)
				ModifyArray(BmpHandler.XtrStr);
			break;

		case 'w':
			if (BmpHandler.IconHeight != 0)
				printf("Line %d redefines width.\n", thisLine);
			BmpHandler.IconHeight = (short)strtol(BufStr, &BufStr2, 10);
			break;

		default:
			printf("%c not recognized at line %d.\n", BufStr[0], thisLine);
			return NMR_READ_BAD_LINE;
		}
	}

	G = fopen(BmpHandler.BmpStr, "rb");

	if (G == NULL)
	{
		printf("Empty BMP read-in file.\n");
		return NMR_READ_NOBMPFILE;
	}
	fclose(G);

	if (MAPCONV_STATUS & MAPCONV_DELETE_PNG_ARTIFACTS)
	{
		system("erase *.png.bak");
		system("erase *.png.0*");
	}

	return NMR_READ_SUCCESS;

}

int ReadInIcons(char yzzy[MAXSTRING], short reset)
{
	FILE * F = fopen(yzzy, "r");
	short hwdef = 0;
	short temp = 0;
	char buffer[200];
	short i1, i2, i3;
	short blankWarn = 0;
	short keepGoing = 1;
	lineInFile = 0;

	if (foundExtra)
	{
		printf("WARNING: i= is after x= in the NMR file. This may cause false errors to be thrown.\n");
	}

	if (reset == 1)
	{
		for (i1 = 0;  i1 < NUM_ICONS;  i1++)
		{
			BmpHandler.IconDefined[i1] = 0;
			BmpHandler.IconUsed[i1] = 0;
			for (i2 = 0;  i2 < MAXICONSIZE;  i2++)
				for (i3 = 0;  i3 < MAXICONSIZE;  i3++)
					BmpHandler.Icons[i1][i3][i2] = BmpHandler.BlankColor;
		}
	}

	if (F == NULL)
	{
		printf("%s is a null file to read icons !\n", yzzy);
		return INVALID;
	}

	while(keepGoing)
	{
		if (fgets(buffer, 100, F) == NULL)
			break;

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
					BmpHandler.IconHeight = BmpHandler.IconWidth = temp;
				}
				else
					BmpHandler.IconHeight = temp;
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
					BmpHandler.IconHeight = BmpHandler.IconWidth = temp;
				}
				else
					BmpHandler.IconWidth = temp;
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
				BmpHandler.IconHeight = BmpHandler.IconWidth = temp;
				break;

			case 'l':
			case 'L':
				if ((buffer[1] == 'c') || (buffer[1] == 'C'))
				{
					temp = (short) strtol(buffer+2, NULL, 16);
					lcd_fill = temp % 16;
					lcd_blank = (temp & 0xff) / 16;
					break;
				}
				temp = (short) strtol(buffer+1, NULL, 16);
				for (i1 = 0; i1 < 10; i1++)
					LCDize((short)i1, (short)(temp + i1), (short)(buffer[0] == 'L'),
						(short)((BmpHandler.IconWidth + 1) / 4), (short) 1, //Xi Yi
						(short)((BmpHandler.IconWidth/4) + ((BmpHandler.IconWidth+1)/4)),
						(short)((BmpHandler.IconWidth / 2) - 1), //dY
						(short) 1);
				break;

			case '\n':	//blank line
				blankWarn++;
				printf("Warning, blank line %d.\n", lineInFile);
				if (blankWarn == 10)
				{
					printf("Too many blank lines, bailing.\n");
					return INVALID;
				}
				break;

			case '#':	//comment
				break;

			case '0': // hexadecimal representation
				if (buffer[1] == 'd')
				{
					//printf("Icon %d decimal printed.\n", v);
					curBase = 10;
					OneIcon(CharToNum(buffer[2])*10+CharToNum(buffer[3]), buffer+4, F);
				}
				else if ((buffer[1] == 'x') || (buffer[1] == 'X'))
				{
					if (buffer[1] == 'X')
						printf("WARNING: line %d has capital x. This will not affect output.\n", lineInFile);
					//printf("Icon %x hex printed.\n", v);
					curBase = 16;
					OneIcon(CharToNum(buffer[2])*16+CharToNum(buffer[3]), buffer+4, F);
				}
				else
				{
					if (strlen(buffer) > 6)
						printf("Possible extra icon row at line %d.\n", lineInFile);
					else
						printf("Bad hexadecimal at %d.\n", lineInFile);
					return INVALID;
				}
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
				keepGoing = 0;
				break;

			default:
				printf("Invalid command line %d: %s", lineInFile, buffer);
				return INVALID;
		}
	}
	if (fillUnknownWithLCD)
		unknownToLCD();

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

void WriteIconsToBmp()
{
	char IconOutFile[200];
	FILE * F1;
	FILE * F0 = fopen(BmpHandler.BmpStr, "rb");

	short i, j, i2, j2;

	short temp = 0;

	printf("%s\n", BmpHandler.BmpStr);

	strcpy(IconOutFile, BmpHandler.OutStr);

	IconOutFile[strlen(IconOutFile)-4] = 0;
	strcat(IconOutFile, "-ico.bmp");

	printf("Writing icon file %s.\n", IconOutFile);

	F1 = fopen(IconOutFile, "wb");

	for (i=0; i < 0x436; i++)
	{
		switch(i)
		{
		case 0x12:
			fgetc(F0);
			temp = (BmpHandler.IconWidth + 2) * 16;
			fputc(temp & 0xff, F1);
			fputc((temp >> 8) & 0xff, F1);
			break;

		case 0x16:
			fgetc(F0);
			temp = (BmpHandler.IconHeight + 2) * 16;
			fputc(temp & 0xff, F1);
			fputc((temp >> 8) & 0xff, F1);
			break;

		case 0x13:
		case 0x17:
			fgetc(F0);
			break;

			case 0x36:
				if (MAPCONV_STATUS & MAPCONV_NO_HEADER_FLAG)
					fputc(fgetc(F0), F1);
				else if (MAPCONV_STATUS & MAPCONV_USE_EGA_HEADER)
				{
					for (j=0; j < ADJ_HEADER_SIZE; j++)
					{
						fgetc(F0);
						fputc(EgaHdr[j], F1);
					}
				}
				else
				{
				for (j=0;  j < ADJ_HEADER_SIZE;  j++)
				{
					fputc(TheHdr[j], F1);
				}
				i += (ADJ_HEADER_SIZE - 1);
				}
				break;

		default:
			fputc(fgetc(F0), F1);
			break;
		}
	}

	for (j=15; j >= 0; j--)
	{
		for (i2 = 0; i2 < (BmpHandler.IconWidth+2) * 16; i2++)
			fputc(8, F1);

		for (j2 = BmpHandler.IconHeight - 1; j2 >= 0; j2--)
		{
			for (i=0; i < 16; i++)
			{
				fputc(8, F1);
				for (i2=0; i2 < BmpHandler.IconWidth; i2++)
					fputc(BmpHandler.Icons[(i & 0xf) + (j << 4)][i2][j2], F1);
				fputc(8, F1);
			}
		}

		for (i2 = 0; i2 < (BmpHandler.IconWidth+2) * 16; i2++)
			fputc(8, F1);
	}

	fclose(F0);
	fclose(F1);
}

void WriteToBmp()
{
	long temp;
	FILE * F1 = fopen(BmpHandler.BmpStr, "rb");
	FILE * F2;
	FILE * F3;
	char outStr[MAXSTRING];
	short len = strlen(BmpHandler.OutStr);
	short xyphusJump = 0;
	long bmpWidth = 0, bmpHeight = 0;

	int i, j, i2, j2, j3, count;

	strcpy(outStr, BmpHandler.OutStr);

	if (MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS_ALT_NAME)
	{
		if (len < 4)
		{
			printf("The BMP output file name is too short.\n");
			return;
		}

		outStr[len-4] = 0;
		strcat(outStr, "-xtr.bmp");

		if (debug)
			printf("%s is new out file\n", outStr);
	}
	else if ((MAPCONV_STATUS & MAPCONV_NOTE_NO_XTR) && !(MAPCONV_STATUS & MAPCONV_XTRA_AMENDMENTS))
	{
		if (len < 4)
		{
			printf("The BMP output file name is too short.\n");
			return;
		}

		outStr[len-4] = 0;
		strcat(outStr, "-nox.bmp");

		if (debug)
			printf("%s is new out file\n", outStr);
	}
	F3 = fopen(outStr, "wb");

	for (i = 0;  i < HEADERSIZE;  i++)
	{
        switch(i)
        {
			case 0x12:
				bmpWidth = (long)(BmpHandler.Xf-BmpHandler.Xi)*(long)BmpHandler.IconWidth;
				putlong(bmpWidth, F3);
				i += 3;
				fgetc(F1);
				fgetc(F1);
				fgetc(F1);
				fgetc(F1);
				break;

			case 0x16:
				bmpHeight = (long)(BmpHandler.Yf-BmpHandler.Yi)*(long)BmpHandler.IconHeight;
				putlong(bmpHeight, F3);
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
		for (j2 = 0;  j2 < BmpHandler.IconHeight;  j2++)
		{
			if (xyphus && (j & 1))
			{
				xyphusJump = 1;
				for (i2=0; i2 < BmpHandler.IconWidth / 2; i2++)
					putc(0, F3);
			}

			for (i = BmpHandler.Xi;  i < BmpHandler.Xf;  i++)
				for (i2 = 0;  i2 < BmpHandler.IconWidth;  i2++)
				{
					short xyz=BmpHandler.Icons[BmpHandler.transpary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]][i2][BmpHandler.IconHeight-j2-1];
					if (BmpHandler.IconIsZigzag[BmpHandler.ary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]])
                    {
                        fputc(BmpHandler.Icons[BmpHandler.ary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]][(i2+j2+(i*BmpHandler.IconWidth)+(j*BmpHandler.IconHeight)) & 1][0], F3);
                        continue;
                    }
					if (BmpHandler.cutNext)
						if ((i < BmpHandler.cutRight) && (i >= BmpHandler.cutLeft)
							&& (j >= BmpHandler.cutUp) && (j < BmpHandler.cutDown))
						{
							fputc((char)BmpHandler.cutColor, F3);
							continue;
						}
					if ((BmpHandler.transpary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]) && (xyz != BmpHandler.TransparencyColor))
						fputc((char)xyz, F3);
					else
				 		fputc((char)BmpHandler.Icons[BmpHandler.ary[i][BmpHandler.Yf+BmpHandler.Yi-j-1]][i2][BmpHandler.IconHeight-j2-1], F3);
				}

            if (bmpWidth % 4) // byte padding if BMP Width is not divisible by 4
                for (j3=(bmpWidth % 4);  j3<4;  j3++)
                    fputc(0, F3);

			if (xyphusJump)
				for (i2=0; i2 < (BmpHandler.IconWidth+1) / 2; i2++)
					fputc(0, F3);
		}
	fclose(F1);
	fclose(F3);

	BmpHandler.cutNext = 0;

	if (MAPCONV_STATUS & MAPCONV_PNG_POST)
	{
		char pngString[80];
		char myCmd[120];

		short len;

		FILE *fp;

		strcpy(pngString, outStr);
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
		sprintf(myCmd, "bmp2png -9 %s", outStr);
		system(myCmd);
	}

	if (MAPCONV_STATUS & MAPCONV_DEBUG_UNKNOWN_SQUARES)
		PrintOutUnused();
}

void PrintOutUnused()
{
	int j, i, k;
	short used[256] = {0};
	short usedAtAll[256] = {0};
	long totalUnused = 0;
	long totalUsed = 0;
	short unusedBaseIcons = 0;
	short usedBaseIcons = 0;

	short leftmostX[256];
	short leftmostY[256];

	short upmostX[256];
	short upmostY[256];

	short rightmostX[256];
	short rightmostY[256];

	short downmostX[256];
	short downmostY[256];

	short temp;

	for (i=0; i < 256; i++)
		leftmostX[i] = leftmostY[i] = rightmostX[i] = rightmostY[i] = upmostX[i] = upmostY[i] = downmostX[i] = downmostY[i] = -1;

	printf("Roll call for unused icons:\n");

	printGrid();

	for (i=0; i < 256; i++)
		BmpHandler.freqAry[i] = 0;

	for (j = BmpHandler.Yi; j < BmpHandler.Yf; j++)
	{
		k = j;

		if (MAPCONV_STATUS & MAPCONV_BOTTOM_TOP)
		{
			k = BmpHandler.Yf + BmpHandler.Yi - k - 1;
		}
		for (i = BmpHandler.Xi; i < BmpHandler.Xf; i++)
		{ //printf("%2x", BmpHandler.ary[i][j]);
			temp = BmpHandler.ary[i][k]; // this is used so we don't have to type in the current icon a bunch of times

			BmpHandler.freqAry[temp]++;
			if (BmpHandler.IconUsed[temp] != 1)
			{
				if ((leftmostX[temp] == -1) || (i < leftmostX[temp]))
				{
					leftmostX[temp] = (short) i;
					leftmostY[temp] = (short) j;
				}

				if (i > rightmostX[temp])
				{
					rightmostX[temp] = (short) i;
					rightmostY[temp] = (short) j;
				}

				if ((upmostX[temp] == -1) || (j < upmostY[temp]))
				{
					upmostX[temp] = (short) i;
					upmostY[temp] = (short) j;
				}

				if (j > downmostY[temp])
				{
					downmostX[temp] = (short) i;
					downmostY[temp] = (short) j;
				}

			}

			if (!BmpHandler.IconUsed[temp])
			{
				if ((MAPCONV_STATUS & MAPCONV_DEBUG_ONLY_FIRST) && (used[temp]))
				{
				}
				else
					printf("%02x %02x (hex coord) Unused icon %x(hex) also %d %d (decimal coord).\n", i, k, temp, i-BmpHandler.Xi, k-BmpHandler.Yi);
				used[BmpHandler.ary[i][k]]++;
				if (BmpHandler.IconUsed[temp] == 2)
					unusedBaseIcons++;
				BmpHandler.IconUsed[temp] = 2;
				totalUnused++;
			}
			else
			{
				if (usedAtAll[temp] == 0)
				{
					usedAtAll[temp] = 1;
					usedBaseIcons++;
				}
				if (BmpHandler.IconUsed[temp] == 2)
					totalUnused++;
				else
					totalUsed++;
			}
		}
//		printf("\n");
	}
	if (MAPCONV_STATUS & MAPCONV_DEBUG_SHOW_EDGE_UNDEF)
		for (i=0; i < 256; i++)
			if (leftmostX[i] != -1)
				printf("%02x: leftmost=%02x,%02x rightmost=%02x,%02x upmost=%02x,%02x downmost=%02x,%02x\n", i,
					leftmostX[i], leftmostY[i], rightmostX[i], rightmostY[i],
					upmostX[i], upmostY[i], downmostX[i], downmostY[i]);

	if (MAPCONV_STATUS & MAPCONV_SHOW_FREQ_STATS)
	{
		runFreqStats();
	}
	if (MAPCONV_STATUS & MAPCONV_SHOW_END_STATS)
	{
		float q = ((float)(totalUsed*100))/(totalUsed+totalUnused);
	    printf("Total stats:\n");
		printf("%d of %d usable icons in final map, for %f percent.\n", totalUsed, totalUsed+totalUnused, q);
		printf("%d icons not defined in icon file, %d sent to the output BMP.\n", unusedBaseIcons, usedBaseIcons);
	}
	if (totalUnused > 0)
		printGrid();
}

void runFreqStats()
{
	short idx[256];
	short i;
	short printedAnything = 0;

	for (i=0; i < 256; i++)
		idx[i] = i;

    qsort (idx, sizeof(idx)/sizeof(*idx), sizeof(*idx), comp);


    for (i = 0 ; i < 256 ; i++)
		if (BmpHandler.freqAry[idx[i]])
			if (((MAPCONV_STATUS & MAPCONV_SHOW_FREQ_STATS_UNUSED) && BmpHandler.IconUsed[idx[i]] != 1) ||
				((MAPCONV_STATUS & MAPCONV_SHOW_FREQ_STATS_USED) && BmpHandler.IconUsed[idx[i]] == 1))
			{
				if (!printedAnything)
				{
					printf("FREQUENCY STATS\n");
					printedAnything = 1;
				}
		        printf ("%02x (%02d): %d\n", idx[i], idx[i], BmpHandler.freqAry[idx[i]]);
			}

	if (!printedAnything)
		printf("No frequency stats to print.\n");
}

int comp (const void * elem1, const void * elem2)
{
    short f = *((short*)elem1);
    short s = *((short*)elem2);
    if (BmpHandler.freqAry[f] < BmpHandler.freqAry[s]) return  1;
    if (BmpHandler.freqAry[f] > BmpHandler.freqAry[s]) return -1;
    return 0;
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

void ModifyArray(char XtrStr[MAXSTRING])
{
	FILE * F;

	int xc, yc, nv, i, j, myBase = 10;
	int xi=0, yi=0;
	long count;
	short lineNum = 0;
	short temp = 0;
	short XtrErr[256] = {0};

	short transparencyWarnYet = 0;

	char buffer[200];
	char * SecondString;
	short XtrTransparencyRead = 0;
	short everBase = 0;

/*	if (BmpHandler.XtrStr[0] == 0)
	{
		strcpy(BmpHandler.XtrStr, BmpHandler.BmpStr);

		BmpHandler.XtrStr[u-3] = 'x';
		BmpHandler.XtrStr[u-2] = 't';
		BmpHandler.XtrStr[u-1] = 'r';

		printf("Using default bmp -> xtr string, %s.\n", BmpHandler.XtrStr);
	}*/

	F = fopen(XtrStr, "r");

	if (F == NULL)
	{
		printf("No such xtr file as %s.\n", XtrStr);
		return;
	}
	else
		printf("Reading %s.\n", XtrStr);

	foundExtra = 1;

	while (fgets(buffer, 200, F))
	{
		lineNum++;
		switch(buffer[0])
		{
		case ';':
			goto evercheck;
			break;

		case '=':
			BmpHandler.iconNumBase = strtol(buffer+1, &SecondString, 10);
			BmpHandler.startIconBase = BmpHandler.mainXtrBase = BmpHandler.iconNumBase;
			everBase = 1;
			break;

		case 'i': // base for icon number
			if (buffer[1] == '=')
			{
				everBase = 1;
				BmpHandler.iconNumBase = strtol(buffer+2, &SecondString, 10);
				break;
			}
			if (buffer[1] != '\n')
				count = strtol(buffer+1, NULL, 10);
			else
				count = 1;

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

		case 'm': // base for main icon output
			if (buffer[1] == '=')
			{
				everBase = 1;
				BmpHandler.mainXtrBase = strtol(buffer+2, &SecondString, 10);
				break;
			}
			if (buffer[1] != '\n')
				count = strtol(buffer+1, NULL, 10);
			else count = 1;
			yc+= count;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
				BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			else
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
			break;

		case 'n': // base for main icon output
			if (buffer[1] == '=')
				BmpHandler.mainXtrBase = strtol(buffer+2, &SecondString, 10);
			else
				printf("Line %d has bad i= for main icon number output base.\n", lineNum);
			break;

		case 'r':
			{
				long x1, y1, x2, y2, defColor;

				if (BmpHandler.startIconBase != BmpHandler.startIconBase)
					printf("MAPCONV WARNING line %d has startIconBase != mainXtrBase.\n", lineNum);

				x1=strtol(buffer+1, &SecondString, BmpHandler.startIconBase);
				y1=strtol(buffer+4, &SecondString, BmpHandler.startIconBase);
				x2=strtol(buffer+7, &SecondString, BmpHandler.startIconBase);
				y2=strtol(buffer+10, &SecondString, BmpHandler.startIconBase);
				defColor=strtol(buffer+13, &SecondString, BmpHandler.startIconBase);
				if (defColor > 256)
				{
					printf("MAPCONV WARNING: Skipping icon value of %2x on line %d, can't be over x100/256.\n", defColor, lineNum);
				}

				for (j=0; j < y2; j++)
					for (i=0; i < x2; i++)
						BmpHandler.ary[x1+i][y1+j] = (short)defColor;
				break;
			}

		case 's': // base for where to start eg moving x=/y=
			if (buffer[1] == '=')
			{
				everBase = 1;
				BmpHandler.startIconBase = strtol(buffer+1, &SecondString, 10);
			}
			else
				printf("Line %d has bad s= for start coordinates.\n", lineNum);
			break;

		case 't':
			if ((!transparencyWarnYet) && !(MAPCONV_USE_TRANSPARENCY & MAPCONV_STATUS))
			{
				transparencyWarnYet = 1;
				printf("MAPCONV WARNING line %d has a transparency toggle but you aren't running the -t# (0-f) option.\n", lineNum);
			}

			temp = 1;
			if (buffer[temp] == 'u')
			{
				XtrTransparencyRead = transpUnder = (buffer[2] == '+');
				temp++;
			}
			else
				transpUnder = 0;

			if (buffer[temp] == '-')
            {
                if (debug)
                    printf("MAPCONV NOTES: turning transparency off at line %d.\n", lineNum);
				XtrTransparencyRead = 0;
            }
			else if (buffer[temp] == '+')
            {
                if (debug)
                    printf("MAPCONV NOTES: turning transparency on at line %d.\n", lineNum);
				XtrTransparencyRead = 1;
            }
			else if (buffer[1] != 'u')
				printf("%s = bad transparency line at %d. Need t(u)(+/-).\n", buffer, lineNum);
			break;

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
			xi = strtol(buffer+1, &SecondString, BmpHandler.startIconBase);
			break;

		case 'y':
			yi = strtol(buffer+1, &SecondString, BmpHandler.startIconBase);
			break;

		case '#':
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
		case 'f': //read one xtr value
			xc = strtol(buffer, &SecondString, BmpHandler.mainXtrBase);
			count = 0;
			while (buffer[count] && (buffer[count] != ','))
				count++;
			if (!buffer[count]) { printf("MAPCONV WARNING XTR file location/tile line has 0 of 2 commas: %s", buffer); break; }

			yc = strtol(buffer+count+1, &SecondString, BmpHandler.mainXtrBase);

			count++;

			while (buffer[count] && (buffer[count] != ',') && (buffer[count] != '='))
				count++;
			if (!buffer[count]) { printf("MAPCONV WARNING XTR file location/tile line has 1 of 2 commas: %s", buffer); break; }

			if (buffer[count] == '=')
				nv = (char)buffer[count+1] & 0xff;	//ie =T, character stuff
			else
				nv = strtol(buffer+count+1, &SecondString, BmpHandler.iconNumBase);

			if (debug > 0)
			{
				if (debug == 2)
					printf("xc %d xi %d yc %d yi %d, base %d\n", xc, xi, yc, yi, myBase);
				printf("Array[%d][%d]=%d with char %ld from string %s", xc+xi, yc+yi, nv, count, buffer);
			}

			if (!BmpHandler.IconDefined[nv])
				if (!XtrErr[nv])
				{
					XtrErr[nv] = 1;
					printf("Line %d has greyed out icon %02x.\n", lineNum, nv);
				}

//			BmpHandler.ary[xc][255-yc] = nv;
			if (XtrTransparencyRead && (MAPCONV_STATUS & MAPCONV_USE_TRANSPARENCY))
			{
				if (BmpHandler.transpary[xc+xi][yc+yi])
					printf("MAPCONV WARNING: possible redefined xtr/transp %d,%d(%02x,%03x hex)\n", xc+xi, yc+yi, xc+xi, yc+yi);
				if (transpUnder)
				{
					BmpHandler.transpary[xc+xi][yc+yi] = BmpHandler.ary[xc+xi][yc+yi];
					BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
				}
				else
					BmpHandler.transpary[xc+xi][yc+yi] = (short)nv;
			}
			else
			{
				if (BmpHandler.checkAry[xc+xi][yc+yi])
					printf("MAPCONV WARNING: possible redefined xtr %d,%d(%02x,%03x hex)\n", xc+xi, yc+yi, xc+xi, yc+yi);
				BmpHandler.ary[xc+xi][yc+yi] = (short)nv;
				BmpHandler.checkAry[xc+xi][yc+yi] = 1;
			}
			if (debug)
				printf("%2x %2x = %2x %2x + %2x %2x\n", xc+xi, yc+yi, xc, yc, xi, yi);
			break;

		default:
			printf("MAPCONV WARNING Xtr file has %s line #%d:\n%s", strlen(buffer) ? "faulty" : "blank", lineNum, buffer);
			break;
		}

		if (buffer[0] == ';')
			break;

	}

evercheck:
	fclose(F);
	if (everBase == 0)
	{
		printf("Note, you never defined a base with (smi)=##, so I assumed base 10.\n");
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
	short processFurther = 0;
	char buffer[MAXSTRING];
	short startLoc = 1;
	short overwriteCheck = 1; // this is on by default but may get reset

	if (MAPCONV_STATUS & MAPCONV_DEBUG_ICONS)
		printf("Icon-read %x, %c\n", q, myBuf[0] == '\n' ? '.' : myBuf[0]);

	if (MAPCONV_STATUS & MAPCONV_SORT_PIX)
		if (q < BmpHandler.LastIconViewed)
			printf("PIX file not sorted. Icon %x is after %x.\n", q, BmpHandler.LastIconViewed);

	BmpHandler.LastIconViewed = q;

	tst = (short) strtol(myBuf+1, NULL, curBase);

	// have an icon interact on itself. Usually not necessary but this lets you combine, say, checkerboard with an outline.
	if ((q == tst) && otherIcon(myBuf[0]))
	{
		for (j=0;  j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[256][i][j] = BmpHandler.Icons[q][i][j];
		tst = 256;
		if (BmpHandler.IconDefined[q] == 0)
			printf("Warning, operating reflexively on icon %02x hex but it's undefined.");
	}

	switch(myBuf[0])
	{
	case ' ': //ok, if they don't see the space, pretend it's a return, for now
	case '\n':
		processFurther = 1;
		break;

	case '=':	//changes one icon to all one color.
		for (j=0;  j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][i][j] = tst;
		break;

	case '/': // upper left is color #1, double slash means diagonal is too
		if (myBuf[startLoc] == '/')
			startLoc++;
		tst = CharToNum(myBuf[startLoc]);
		tst2 = CharToNum(myBuf[startLoc+1]);
		if (tst == '/')
			startLoc++;
		for (j=0;  j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				if (i + j < BmpHandler.IconHeight - 1)
					BmpHandler.Icons[q][i][j] = tst;
				else if (i + j > BmpHandler.IconHeight - 1)
					BmpHandler.Icons[q][i][j] = tst2;
				else if (startLoc == 2)
					BmpHandler.Icons[q][i][j] = tst;
				else
					BmpHandler.Icons[q][i][j] = tst2;
		break;

	case '\\': // upper right is color #1, double slash means diagonal is too
		if (myBuf[startLoc] == '\\')
			startLoc++;
		tst = CharToNum(myBuf[startLoc]);
		tst2 = CharToNum(myBuf[startLoc+1]);
		for (j=0;  j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				if (i > j)
					BmpHandler.Icons[q][i][j] = tst;
				else if (i < j)
					BmpHandler.Icons[q][i][j] = tst2;
				else if (startLoc == 2)
					BmpHandler.Icons[q][i][j] = tst;
				else
					BmpHandler.Icons[q][i][j] = tst2;
		break;

	case 'c': //copies one icon to another
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: copied-from icon 0x%x is not defined.\n", tst);
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][i][j] = BmpHandler.Icons[tst][i][j];
		break;

	case 'e': // 180 degree rotation
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: rotated-from icon 0x%x is not defined.\n", tst);
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.IconWidth-1-i][BmpHandler.IconHeight-1-j] = BmpHandler.Icons[tst][i][j];
		break;

	case 'f': // flip across the SW/NE diagonal
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: flipped-from icon 0x%x is not defined.\n", tst);
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.IconWidth-1-j][BmpHandler.IconHeight-1-i] = BmpHandler.Icons[tst][i][j];
		break;

	case 'F': // flip across the SE/NW diagonal
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: flipped-from icon 0x%x is not defined.\n", tst);
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][j][i] = BmpHandler.Icons[tst][i][j];
		break;

	case 'h': //copies one icon to another, flipped horizontally
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: flipped-from icon 0x%x is not defined.\n", tst);
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.IconWidth-1-i][j] = BmpHandler.Icons[tst][i][j];
		break;

	case 'H': //horizontal trimming, with color after
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0; i < tst2; i++)
				BmpHandler.Icons[q][i][j] = BmpHandler.Icons[q][BmpHandler.IconWidth-i-1][j] = tst;
		break;

	case 'l': //rotate 90 degrees left
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: rotated-from icon 0x%x is not defined.\n", tst);
			return;
		}
		if (BmpHandler.IconHeight != BmpHandler.IconWidth)
		{
			printf ("MAPCONV ERROR: Need height = width to rotate.\n");
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][j][BmpHandler.IconHeight-1-i] = BmpHandler.Icons[tst][i][j];
		break;

	case 'L': // lcd digits
		LCDize(tst, (short)q, 0,
			(short)((BmpHandler.IconWidth + 1) / 4), (short) 1, //Xi Yi
			(short)((BmpHandler.IconWidth/4) + ((BmpHandler.IconWidth+1)/4)),
			(short)((BmpHandler.IconWidth / 2) - 1), //dY
			(short) 1);
		break;

	case 'O': //this puts a ring around a square, border color first then fill
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][j][BmpHandler.IconHeight-1-i] =
				(i == 0 || j == 0 || i == BmpHandler.IconWidth-1 || j == BmpHandler.IconHeight-1) ? tst>>4 : tst & 0xf;
		break;

	case 'p': // palette replacement eg change reds to purple, 0x34p2~6 (switch) or 0x34p2-6 (only 2 to 6)
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[3]);
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
			{
				if (BmpHandler.Icons[q][j][i] == tst)
					BmpHandler.Icons[q][j][i] = tst2;
				else if ((BmpHandler.Icons[q][j][i] == tst2) && (myBuf[2] == '~'))
					BmpHandler.Icons[q][j][i] = tst;
			}
		overwriteCheck = -1;
		break;

	case 'r': //rotate 90 degrees right
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: rotated-from icon 0x%x is not defined.\n", tst);
			return;
		}
		if (BmpHandler.IconHeight != BmpHandler.IconWidth)
		{
			printf ("MAPCONV ERROR: Need height = width to rotate.\n");
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][BmpHandler.IconWidth-1-j][i] = BmpHandler.Icons[tst][i][j];
		break;

	case 'S': //switch 2 colors. Actually, you can switch with a null color with no problem.
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0; i < BmpHandler.IconWidth; i++)
				if ((BmpHandler.Icons[q][i][j] == tst) || (BmpHandler.Icons[q][i][j] == tst2))
					BmpHandler.Icons[q][i][j] = tst + tst2 - BmpHandler.Icons[q][i][j];
		overwriteCheck = -1;
		break;

	case 'v': //copies one icon to another, flipped vertically
		if (BmpHandler.IconDefined[tst] == 0)
		{
			printf("MAPCONV WARNING: flipped-from icon 0x%x is not defined.\n", tst);
			return;
		}
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][i][BmpHandler.IconHeight-1-j] = BmpHandler.Icons[tst][i][j];
		break;

	case 'V': //vertical trimming, with color after
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		for (j=0; j < tst2; j++)
			for (i=0; i < BmpHandler.IconWidth; i++)
				BmpHandler.Icons[q][i][j] = BmpHandler.Icons[q][i][BmpHandler.IconHeight-j-1] = tst;
		break;

	case 'x': //alternating checkerboard colors, "dumb"
		tst = CharToNum(myBuf[1]);
		tst2 = CharToNum(myBuf[2]);
		//printf("Checkerboard! %d %d\n", tst, tst2);
		for (j=0;  j < BmpHandler.IconHeight; j++)
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				if ((i+j)%2)
					BmpHandler.Icons[q][i][j] = tst2;
				else
					BmpHandler.Icons[q][i][j] = tst;
		break;

    case '%': // alternating checkerboard colors but for odd width or height
        BmpHandler.Icons[q][0][0] = CharToNum(myBuf[1]);
        BmpHandler.Icons[q][1][0] = CharToNum(myBuf[2]);
        BmpHandler.IconIsZigzag[q] = 1;
        break;

	default:
		printf("Unknown command for icon %x: %s", q, myBuf);
		return;
	}
	if ((overwriteCheck == 1) && (BmpHandler.IconDefined[q] == 1))
	{
		printf("Warning, icon %x hex is maybe being overwritten.\n", q);
	}
	if ((overwriteCheck == -1) && (BmpHandler.IconDefined[q] == 0))
	{
		printf("MAPCONV WARNING: tried to do something with blank icon %x hex.\n", q);
	}

	BmpHandler.IconDefined[q] = 1;
	BmpHandler.IconUsed[q] = 1;

	if (!processFurther)
		return;

	for (j=0;  j < BmpHandler.IconHeight; j++)
	{
		short buflen;
		BmpHandler.IconUsed[q] = 1;
		lineInFile++;
		fgets(buffer, 40, F);
		buflen = strlen(buffer);

		if (buffer[0] == '*')
			for (i=0;  i < BmpHandler.IconWidth;  i++)
				BmpHandler.Icons[q][i][j] = CharToNum(buffer[1]);
			if (buflen < BmpHandler.IconWidth+1)
				printf("MAPCONV WARNING: line %d has only %d length.\n", lineInFile, strlen(buffer));
			if (buflen > BmpHandler.IconWidth+1)
				printf("MAPCONV WARNING: line %d runs over with %d length.\n", lineInFile, strlen(buffer));

		for (i=0;  i < BmpHandler.IconWidth;  i++)
		{
            BmpHandler.Icons[q][i][j] = CharToNum(buffer[i]);
            if ((buffer[i] > '9') || (buffer[i] < '0'))
			  if ((buffer[i] > 'Z') || (buffer[i] < 'A'))
				if ((buffer[i] > 'z') || (buffer[i] < 'a'))
				{
                  printf("Reading icon 0x%x(%d), bogus char 0x%x at %d,%d in icon, line %d.\n",
					  q, q, buffer[i], i, j, lineInFile);
				  if ( buffer[i] == 0xa)
					  break;
				}
        }
		if (buffer[i] == '\\')
			break;
	}
}

short otherIcon(char x)
{
	switch(x)
	{
	case 'c':
	case 'e':
	case 'f':
	case 'F':
	case 'h':
	case 'l':
	case 'r':
	case 'v':
		return 1;
	default:
		return 0;
	}
}

void unknownToLCD()
{
	long outlineColor = BmpHandler.unknownLCDColor ^ 0x808080;
	short i;
	short wd = (BmpHandler.IconWidth-4) >> 1;
	short hd = (BmpHandler.IconHeight-1) >> 1;

	if (BmpHandler.IconWidth != BmpHandler.IconHeight)
	{
		printf("Sorry, can't do anything when width != height. Or, rather, it's too tricky.\n");
		return;
	}

	if ((BmpHandler.IconWidth < 8) || (BmpHandler.IconHeight < 8))
	{
		printf("Sorry, dimensions of %dx%d is too narrow to LCDize unknown icons. I'll need 8x8 at least.\n",
			BmpHandler.IconWidth, BmpHandler.IconHeight);
		return;
	}

	for (i=0; i < 256; i++)
	{
		if (!BmpHandler.IconDefined[i])
		{
			//printf("Adding %02x.\n", i);
			LCDize((short)(i >> 4), i, 0, 1, 1, wd, (short)((BmpHandler.IconHeight)/2-1), 1);
			LCDize((short)(i & 0xf), i, TRANSPARENT_LCD,
				(short)(wd+3), 1,	//xi, yi
				wd, (short)((BmpHandler.IconHeight)/2-1), //dx, dy
				1);
		}
	}
}

void LCDize(short whichNum, short whichIcon, short allowPrevDefined, short xi, short yi, short dx, short dy, short clearBefore)
{
	short lcdbin = LCDs[whichNum];
	short i, j;

	if ((allowPrevDefined == 0) && (BmpHandler.IconDefined[whichIcon] != 0))
	{
		printf("Icon %x already used, so I am not reassigning it.\n", whichIcon);
		return;
	}

	if (allowPrevDefined != TRANSPARENT_LCD)
		for (j=0; j < BmpHandler.IconHeight; j++)
			for (i=0; i < BmpHandler.IconWidth; i++)
				BmpHandler.Icons[whichIcon][i][j] = lcd_blank;

	if ((BmpHandler.IconDefined[whichIcon] != 0) && (MAPCONV_STATUS & MAPCONV_DEBUG_ICONS))
	{
		printf("MAPCONV WARNING: icon %x already used, overwriting.\n", whichIcon);
	}

	if (dx < 2)
	{
		printf("MAPCONV WARNING: map width is too narrow.\n");
		return;
	}

	if (lcdbin & 1)
		for (i = xi; i <= dx+xi; i++)
			BmpHandler.Icons[whichIcon][i][1] = lcd_fill;

	if (lcdbin & 2)
		for (i = yi; i <= yi+dy; i++)
			BmpHandler.Icons[whichIcon][xi][i] = lcd_fill;

	if (lcdbin & 4)
		for (i = yi; i <= yi+dy; i++)
			BmpHandler.Icons[whichIcon][xi+dx][i] = lcd_fill;

	if (lcdbin & 8)
		for (i = xi; i <= xi+dx; i++)
			BmpHandler.Icons[whichIcon][i][yi+dy] = lcd_fill;

	if (lcdbin & 16)
		for (i = yi+dy; i <= yi+dy*2; i++)
			BmpHandler.Icons[whichIcon][xi][i] = lcd_fill;

	if (lcdbin & 32)
		for (i = yi+dy; i <= yi+dy*2; i++)
			BmpHandler.Icons[whichIcon][xi+dx][i] = lcd_fill;

	if (lcdbin & 64)
		for (i = xi; i <= dx+xi; i++)
			BmpHandler.Icons[whichIcon][i][yi+dy*2] = lcd_fill;

	if (lcdbin & 128)
		for (i = 1; i <= yi+dy*2; i++)
			BmpHandler.Icons[whichIcon][xi+dx/2][i] = lcd_fill;
}

int LatestNumber(FILE * F)
{
        int jjj;
        jjj = (16*CharToNum(fgetc(F)) + CharToNum(fgetc(F)));
        return jjj;
}

void ReadRawData()
{
	short i, j, l;
	FILE * G;

	if (!BmpHandler.XtrStr[0])
	{
		printf("Assigning XTR file to the raw BMP string.\n");
		strcpy(BmpHandler.XtrStr, BmpHandler.BmpStr);
		l = strlen(BmpHandler.XtrStr);
		BmpHandler.XtrStr[l-3] = 'x';
		BmpHandler.XtrStr[l-2] = 't';
		BmpHandler.XtrStr[l-1] = 'r';
	}

	G = fopen(BmpHandler.BmpStr, "rb");

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

	fclose(G);
}
void snip(char x[MAXSTRING])
{
	short j = strlen(x);

	if (!j)
		return;

	if (x[j-1] == '\n')
		x[j-1] = 0;
}

int CharToNum(int z)
{
	if ((z >= 'a') && (z <= 'z'))
		return (z-'a'+10);
	if ((z >= 'A') && (z <= 'Z'))
		return (z-'A'+10);
	return (z - '0');
}
