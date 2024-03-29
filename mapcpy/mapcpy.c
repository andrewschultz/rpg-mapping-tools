////////////////////////////////////////////////////////////
//mapcpy.c
//
//copyright 2008(?)-2017 Andrew Schultz, well okay take what you want
//but I'm still happy I made this
//
//copies binary information to the actual map
//this is used for nintendo save states where the map is cut up, or just for regular Apple disk images or PC binary files
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>

#define MAXW 640
#define MAXH 640

#define CLIPMAX 256

#define STRAIGHT_BYTES	0
#define NIB_HIGH_FIRST	1
#define NIB_LOW_FIRST	2
#define BIT_HIGH_FIRST	3
#define BIT_LOW_FIRST	4

typedef enum { false, true } bool;

short isYellow(short x); // very limited function
bool viable (int a1, int a2, int a3); //very limited function

void usage();
void mapCpyFmt();
void readBrackets(char * x);

char lineTab[20];
long myXMin = 0, myXMax = 0, myYMin = 0, myYMax = 0, myW, myH;

main(int argc, char * argv[])
{
	short replace[256] = { -1 };
	short addSpace = 0;

	short needToProc = 0;
	short needToBMP = 0;
	short commentBlock = 0;

	short cutBuffer[CLIPMAX][CLIPMAX]; // for future using a cut buffer but probably not strictly necessary. Maybe limit it to 256?

	short commodore = 0;

	FILE * F = NULL, * G = NULL, * H, * I;
	long keepGoing = 1;
	unsigned int count = 1;
	short goDirection = 1;
	long toSet = 0, temp = 0, temp2 = 0;

	long curLine = 0, tabIndex = 0;

	long myDefaultOffset = 0;
	long myAbsX = 0, myAbsY = 0;
	long myLastX = 0, myLastY = 0, myX = 0, myY = 0, myOffset = 0, setTransparent = 0, transpColor = 0,
		myYModOffset=0, myXModOffset=0, outHi=0, outWi=0, outH=256, outW=256, doFringe = 0, gegege=0;
	long myAnchorX = 0, myAnchorY = 0;
	long i, j, i2, j2;
	short qflags = 15, quartering = 0;

	long sectorSize = 256;
	long sectorTemp = 0;
	long curX = 0;
	short launch = 0, localLaunch = 0;
	short overlapOK = 1;
	short vertical = 1;

	short forceH = 0;
	short forceV = 0;

	short bailOnWarn = 0;

	short persist = 0;

	short readType = STRAIGHT_BYTES;

	short myAry[MAXW][MAXW], ch;
	char myExt[10] = "";

	char myFile[200] = "";
	char myLine[500];

	char * buffer;

	short gotFile = 0;
	short debug = 0;
	short trackOverlap = 0;
	short blockOverlap = 0;

	short postProcess = 0;
	short deleteBeforePostProcess = 0;

	short sectorStart = 0;

	short hJump = 0;
	short vJump = 0;

	while ((short)count < argc)
	{
		if (debug)
			printf("Argument %d: %s\n", count, argv[count]);

		if (argv[count][0] == '-')
		{
			switch(argv[count][1])
			{
			case '?':
				usage();
				return 0;

			case 'b':
			case 'B':
				bailOnWarn = 1;
				count++;
				break;

			case 'd':
			case 'D':
				if (argv[count][2])
					debug = (short)argv[count][2] - (short)'0';
				else
					debug = 1;
				if ((debug < 0) || (debug > 9))
				{
					printf("Unknown debug depth set (need 0-9), assuming 1\n");
					debug  = 1;
				}
				count++;
				break;

			case 'l':
			case 'L':
				launch = 1;
				count++;
				break;

			case 'o':
			case 'O':
				trackOverlap = 1;
				if (argv[count][2] | 0x20 == 'b')
					blockOverlap = 1;
				count++;
				break;

			case 'p':
			case 'P':
				postProcess = 1;
				if ((argv[count][1] == 'd') || (argv[count][1] == 'D'))
					deleteBeforePostProcess = 1;
				break;

			case 'u':
			case 'U':
				mapCpyFmt();
				return 0;

			default:
				printf("Unknown flag %s.\n\n");
				usage();
				return 0;
			}

		}
		else
		{
			if (gotFile)
			{
				printf("Already specified a file on the command line. Bailing.\n");
				return 0;
			}
			strcpy(myFile, argv[count]);
			count++;
		}
	}

	if (myFile[0] == 0)
	{
		printf("You must specify a file on the command line. There is no default.\n");
		return 0;
	}

	F = fopen(myFile, "r");

	if (F == NULL)
	{
		printf("No such file\n");
		return 0;
	}

	count = 0;

	for (j=0; j < MAXH; j++)
		for (i=0; i < MAXW; i++)
			myAry[i][j] = 0;

	for (i=0; i < 256; i++)
		replace[i] = -1;

	while ((fgets(myLine, 500, F) != NULL) && (keepGoing))
	{
		curLine++;
		if (debug)
			printf("%s: %s", lineTab, myLine);

		if (myLine[0] == '#')
		{
			if (commentBlock && (myLine[1] == '-'))
				commentBlock = 0;

			if (myLine[1] == '+')
			{
				commentBlock = (short)curLine;
			}

			if (myLine[1] == '!')
			{
				printf("%s", myLine);
			}

			continue;
		}

		buffer = strtok(myLine, "\t");
		tabIndex = 1;

		do
		{

		sprintf(lineTab, "%d-%d", curLine, tabIndex);

		if (buffer[0] == ' ')
		{
			printf("Line %s starts with a space. Removing all leading spaces.\n", lineTab);
			while (buffer[0] == ' ')
				buffer++;
		}
		if (buffer[0] && (buffer[strlen(buffer)-1] == ' '))
		{
			short count = strlen(buffer)-1;
			while ((count > 0) && (buffer[count-1] == ' '))
				count--;

			printf("Line %s ends with a space. Removing all trailing spaces.\n", lineTab);
			buffer[count] = 0;
		}

		switch(buffer[0])
		{
		case 0:
			printf("WARNING blank entry at line %s.\n", lineTab);
			break;

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

		case '\n':
			break;

		case '0':
			if (buffer[1])
			{
				printf("WARNING: you may've put an 0 where you meant an o at line %s.\n", lineTab);
			}
			printf("WARNING (%s): probably deprecated command 0. Use & instead.", lineTab);
			addSpace = 0;
			break;

		case '1':
			if (buffer[1])
			{
				printf("WARNING (%s): you may've put a 1 where you meant something else at line %s.\n", lineTab);
			}
			printf("WARNING: probably deprecated command 1. Use & instead.", lineTab);
			addSpace = 1;
			break;

		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			printf("WARNING: No commands start with numbers, except for adding/subtracting a space.");
			break;

		case '{': // 4 coordinates giving the local rectangle of what to print
		case '[':
		case '(':
			if ((buffer[1] == '}') || (buffer[1] == ']') || (buffer[1] == ')'))
			{
				myXMin = 0;
				myXMax = myW;
				myYMin = 0;
				myYMax = 0;
				break;
			}
			readBrackets(buffer+1);
			break;

		case ';':
			keepGoing = 0;
			break;

		case '$':
			{
				short index = 0;
				long lastComma = 0;
				long coords[10];
				short commas = 0;
				long inCommands = 1;

				long xi, yi, xClip, yClip, xTo, yTo;
				for (i=0; i < (short)strlen(buffer); i++)
				{
					coords[commas] = i;
					if (buffer[i] == ',')
					{
						commas++;
						if (commas == 5)
							continue;
						lastComma = i;
						switch(commas)
						{
						case 0:
							xi = strtol(buffer+i+1, NULL, 10);
							break;

						case 1:
							yi = strtol(buffer+i+1, NULL, 10);
							break;

						case 2:
							xClip = strtol(buffer+i+1, NULL, 10);
							if (xClip < 0)
							{
								printf("%s: xClip value must be positive.\n", lineTab);
								goto cutpasteend;
							}
							if (xClip > CLIPMAX)
							{
								printf("%s: xClip value must be below %d.\n", lineTab, CLIPMAX);
								goto cutpasteend;
							}
							break;

						case 3:
							yClip = strtol(buffer+i+1, NULL, 10);
							if (yClip < 0)
							{
								printf("%s: yClip value must be positive.\n", lineTab);
								goto cutpasteend;
							}
							if (yClip > CLIPMAX)
							{
								printf("%s: yClip value must be below %d.\n", lineTab, CLIPMAX);
								goto cutpasteend;
							}
							break;

						case 4:
							xTo = strtol(buffer+i+1, NULL, 10);
							break;

						case 5:
							yTo = strtol(buffer+i+1, NULL, 10);
							break;
						}
					}
				}

				if ((commas != 3) && (commas != 5))
				{
					printf("WARNING %s doesn't have right number of commas. You need 3 or 5.");
					break;
				}

				if (commas == 5)
				{
					for (j=0; j < yClip; j++)
						for (i=0; i < xClip; i++)
						{
							cutBuffer[i][j] = myAry[xi+i][yi+j];
							myAry[xi+i][yi+j] = 0;
						}
					for (j=0; j < yClip; j++)
						for (i=0; i < xClip; i++)
							myAry[xTo+i][yTo+j] = cutBuffer[i][j];
					break;
				}

				for (i=lastComma; i < (short)strlen(buffer); i++)
				{
					if (buffer[i] == ':')
					{
						inCommands=1;
						continue;
					}
					if (inCommands)
					{
						switch(buffer[i])
						{

						case 'f': // flip = v + h
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
									cutBuffer[i][j] = myAry[xi+xClip-1-i][yi+yClip-1-j];
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						case 'v':
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
									cutBuffer[i][j] = myAry[xi+i][yi+yClip-1-j];
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						case 'h':
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
									cutBuffer[i][j] = myAry[xi+xClip-1-i][yi+j];
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						case 'r':
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
								{
									cutBuffer[xClip-j-1][i] = myAry[xi+i][yi+j];
									myAry[xi+i][yi+j] = 0;
								}
							for (j=0; j < xClip; j++)
								for (i=0; i < yClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						case 'l':
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
								{
									cutBuffer[j][yClip-i-1] = myAry[xi+i][yi+j];
									myAry[xi+i][yi+j] = 0;
								}
							for (j=0; j < xClip; j++)
								for (i=0; i < yClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						case '\\':
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
								{
									cutBuffer[j][i] = myAry[xi+i][yi+j];
								}
							for (j=0; j < xClip; j++)
								for (i=0; i < yClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						case '/':
							for (j=0; j < yClip; j++)
								for (i=0; i < xClip; i++)
								{
									cutBuffer[xClip-i-1][yClip-j-1] = myAry[xi+i][yi+j];
								}
							for (j=0; j < xClip; j++)
								for (i=0; i < yClip; i++)
									myAry[xi+i][yi+j] = cutBuffer[i][j];
							break;

						}
					}
				}
			}
				cutpasteend:
			break;

		case '+': //+/-/^/v rotate the start-x in the rectangle. For moving around, use dx/dy
			myXModOffset = strtol(buffer+1, NULL, 10);
			break;

		case '-': //+/-/^/v rotate the start-x in the rectangle. For moving around, use dx/dy
			myXModOffset = myW - strtol(buffer+1, NULL, 10);
			break;

		case '^': //+/-/^/v rotate the start-x in the rectangle. For moving around, use dx/dy
			myYModOffset = myH - strtol(buffer+1, NULL, 10);
			break;

		case '&': // changes hjump and/or vjump, which is the default to jump after reading in a rectangle
			if ((buffer[1] == 'h') || (buffer[1] == 'H'))
				hJump = (short)strtol(buffer+2, NULL, 10);
			else if ((buffer[1] == 'v') || (buffer[1] == 'V'))
				vJump = (short)strtol(buffer+2, NULL, 10);
			else
				hJump = vJump = (short)strtol(buffer+1, NULL, 10);
			break;

		case '*': // changes everything to a specific color, default is black
			// if there are 4 commas then it is only a range of coordinates
			{
				short defaultSquare = 0;
				short clearR = MAXW;
				short clearL = 0;
				short clearU = 0;
				short clearD = MAXH;
				short commas = 0;

				if (buffer[1] == '=')
				{
					defaultSquare = (short)strtol(buffer+2, NULL, 16);
					if ((defaultSquare > 255) || (defaultSquare < 0))
					{
						printf("Invalid value for default color on line %s. Choosing 0.\n", lineTab);
						defaultSquare = 0;
					}
				}

				for (i=0; i < (short)strlen(buffer); i++)
					if (buffer[i] == ',')
						commas++;

				switch (commas)
				{
				case 0: //default, turn everything a certain color
					break;
				case 4:
					commas = 0;
					for (i=0; i < (short)strlen(buffer); i++)
					{
						if (buffer[i] == ',')
						{
							switch(commas)
							{
							case 0:
								clearL = (short)strtol(buffer+i+1, NULL, 16);
								break;

							case 1:
								clearU = (short)strtol(buffer+i+1, NULL, 16);
								break;

							case 2:
								clearR = (short)strtol(buffer+i+1, NULL, 16);
								break;

							case 3:
								clearD = (short)strtol(buffer+i+1, NULL, 16);
								break;

							default:
								printf("Oops, bad error.\n");
							}
							commas++;
						}
					}
					if ((clearL < 0) || (clearU < 0))
					{
						printf("Negative crop value at line %s.\n", lineTab);
						break;
					}

					if ((clearL >= clearR) || (clearU >= clearD))
					{
						printf("Invalid crop region at line %s.\n", lineTab);
						break;
					}

					break;

				default:
					printf("Line %s needs either 0 or 4 commas for *-fill, even if you're using the default color.\n", lineTab);
					clearR = -1;
					break;
				}

				for (j=clearU; j < clearD; j++)
					for (i=clearL; i < clearR; i++)
						myAry[i][j] = defaultSquare;
			}
			break;

		case '<': //reset current-x if we are reading sectors that don't match up with rows
			//not clear how this is different than CR but I am keeping for backwards compatibility
			if (curX > 0)
			{
				short temp = 256 - (short)curX;
				printf("Moving left by %d/%x at %s: ", curX, curX, lineTab);
				if ((temp > 0) || (myW < 256))
				{
					printf("possible adjustment(s)");
					while (temp > 0)
					{
						printf(" %d", temp);
						temp -= (short)myW;
					}
				}
				else
					printf("no suggestions");
				printf("\n");
			}
			curX = 0;
			break;

		case '>': // this outputs to a file
			localLaunch = launch;
			temp = 0;
			buffer[strlen(buffer)-1] = 0;
			if (needToProc)
			{
				printf("WARNING: %s (line %s) occurs before processing the last 'r' line.\n", buffer, lineTab);
				needToProc = 0;
			}
			if (buffer[1] == '+')
			{
				localLaunch = 1;
				I = fopen(buffer+2, "wb");
				temp = 1;
			}
			else if (buffer[1] == '-')
			{
				localLaunch = 0;
				I = fopen(buffer+2, "wb");
				temp = 1;
			}
			else
				I = fopen(buffer+1, "wb");
			if (I == NULL)
			{
				printf("Couldn't read %s.\n", buffer+1);
				break;
			}

			needToBMP = 0;
			printf("Writing %s\n", buffer+1+temp);
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

			//printf("outH outW %d %d\n", outH, outW);
			if (gegege == 1) //VERY VERY BAD HACK.
			{
			for (j=1; j < outH-1; j++)
				for (i=0; i < outW-1; i++)
					if (myAry[i][j] == 0x03)
						if (isYellow(myAry[i][j-1]) || isYellow(myAry[i][j+1]) || isYellow(myAry[i+1][j]) || isYellow(myAry[i-1][j]))
						if ((myAry[i][j-1] >> 4) && (myAry[i][j+1] >> 4) && (myAry[i+1][j] >> 4) && (myAry[i-1][j] >> 4))
							myAry[i][j] = 0x4e;
			}

			temp2 = ((outW-outWi) % 4) ? 4 - ((outW - outWi) % 4) : 0;
			for (j=outHi; j < outH; j++)
			{
				for (i=outWi; i < outW; i++)
					fputc(myAry[i][outH-1-j], I);
				if (i)
					for (i=0; i < temp2; i++)
						fputc(0, I);
			}

			fclose(I);

			if (postProcess)
			{
				char cmdbuf[100] = "bmp2png -9 ";
				if (deleteBeforePostProcess)
				{
					char cmdbuf2[100] = "erase ";
					char pngName[100] = "";
					long bl = strlen(pngName);

					strcpy(pngName, buffer+i+1);

					pngName[bl-3] = 'p';
					pngName[bl-2] = 'n';
					pngName[bl-1] = 'g';

					if (_access(pngName, 0) != 0)
					{
						strcat(cmdbuf2, pngName);
						system(cmdbuf2);
					}

				}
				strcat(cmdbuf, buffer+i+1);
				system(cmdbuf);
			}
			if (localLaunch)
			{
				char cmdbuf[100] = "mspaint ";
				strcat(cmdbuf, buffer+temp+1);
				system(cmdbuf);
			}
			break;

		case '\\':
			{
				short comcount=0;
				short diagX = 0;
				short diagY = 0;
				short diagL = 0;
				short temp;
				diagX = (short)strtol(buffer+i+1, NULL, 10);

				for (i=1; i < (short) strlen(buffer); i++)
				{
					if (buffer[i] == ',')
					{
						if (comcount == 0)
							diagY = (short)strtol(buffer+i+1, NULL, 10);
						if (comcount == 1)
							diagL = (short)strtol(buffer+i+1, NULL, 10);
						comcount++;
					}
				}
				if (diagL == 0)
					break;
				for (j=0; j < diagL; j++)
					for (i=j+1; i < diagL; i++)
					{
						temp = myAry[diagX+i][diagY+j];
						myAry[diagX+i][diagY+j] = myAry[diagX+j][diagY+i];
						myAry[diagX+j][diagY+i] = temp;
					}
			}
			break;

		//absolute offsets
		case 'a':
			if (buffer[1] == 'x')
				myAbsX = strtol(buffer+2, NULL, 10);
			if (buffer[1] == 'y')
				myAbsY = strtol(buffer+2, NULL, 10);
			break;

		case 'A': //anchor
			if (buffer[1] == 'x')
			{
				myX = myLastX = myAnchorX = strtol(buffer+2, NULL, 10);
				break;
			}
			if (buffer[1] == 'y')
			{
				myY = myLastY = myAnchorY = strtol(buffer+2, NULL, 10);
				break;
			}
			break;

		case 'b':
		case 'B':
			if ((buffer[1] == 'h') || (buffer[1] == 'H'))
				readType = BIT_HIGH_FIRST;
			else if ((buffer[1] == 'l') || (buffer[1] == 'L'))
				readType = BIT_LOW_FIRST;
			else if ((buffer[1] == '0') || (buffer[1] == '-') || (buffer[1] == 0))
				readType = STRAIGHT_BYTES;
			else
				printf("WARNING line %s: BH or BL (bit/high or low first) are the only two options for B.\
-/0/no byte clears it.\n", lineTab);
			break;

		case 'c': //in the case of Xyphus maps I used the Commodore
		case 'C': //it may have 2 bytes at the start of a 256-block so we need to ignore those
				  //however without C64 we do a carriage return resetting the X to 0 and moving down
				  //this doesn't appear in final files but is good for debugging when we need to jigsaw things together
			if ((buffer[1] == '6') && (buffer[2] == '4'))
			{
				commodore = (buffer[3] != '-');
				break;
			}

			if ((buffer[1] == 'r') || (buffer[1] == 'R'))
			{
				myX = myLastX + myW;
				myY = myLastY;
				break;
				printf("Invalid command--cr is the only one.");
			}
			break;

		case 'd': // this changes the starting point to draw the rectangle
			if (buffer[1] == 'x')
			{
				if (buffer[2] == '-')
				{
					if (myX < strtol(buffer+3, NULL, 10))
					{
						printf("WARNING dx command went to -x, %d back %d at line %s\n",
							myX, strtol(buffer+3, NULL, 10), lineTab);
						break;
					}
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
					if (myY < strtol(buffer+3, NULL, 10))
					{
						printf("WARNING dy command went to -y, %d back %d at line %s\n",
							myY, strtol(buffer+3, NULL, 10), lineTab);
						break;
					}
					myY -= strtol(buffer+3, NULL, 10);
				}
				else
					myY += strtol(buffer+2, NULL, 10);
				//printf("Adding %d to y, now %d\n", strtol(buffer+2, NULL, 10), myY);
				break;
			}
			printf("Error at line %s. d(xy) is proper usage for moving.\n", lineTab);
			break;

		case 'e':
		case 'E':
			if (buffer[1] == '=')
			{
				if (buffer[strlen(buffer)-1] == '\n')
					buffer[strlen(buffer)-1] = 0;
				strcpy(myExt, buffer+2);
			}
			break;

		case 'F': // FROM anchor
			if (buffer[1] == 'x')
			{
				myLastX = myX = myAnchorX + strtol(buffer+2, NULL, 10);
				break;
			}
			if (buffer[1] == 'y')
			{
				myLastY = myY = myAnchorY + strtol(buffer+2, NULL, 10);
				break;
			}
			doFringe = 1;
			break;

		case 'f': // switches which direction a partial-save will go
			goDirection = 0 - goDirection;
			break;

		case 'g': // this is specific to Gegege No Kitaro
			if (buffer[1] == 'e')
				gegege=1;
			break;

		case 'H':
			if (buffer[1] == '-')
				outHi = strtol(buffer+2, NULL, 10);
			else
				outH = strtol(buffer+1, NULL, 10);
			break;

		case 'h':
			myH = strtol(buffer+1, NULL, 10);
			break;

		case 'L':
			myLastY = myY;
			myLastX = myX;
			break;

		case 'm': //The m's are deprecated as they were a bit clumsy, but they're contained here for backwards compatibility for now
			printf("WARNING: %s has deprecated m-command. Use {} instead with 0's for unused min/max, {} to reset.\n", lineTab);
			if (buffer[1] == '{')
			{
				readBrackets(buffer+2);
				break;
			}
			if (buffer[1] == '-')
			{
				myXMin = 0;
				myXMax = myW;
				if (buffer[2] == '-')
				{
					myYMin = 0;
					myYMax = 0;
				}
				break;
			}
			if ((buffer[1] == 'y') || (buffer[1] == 'Y'))
			{
				myYMin = strtol(buffer+2, NULL, 10);
				if (myYMin < 0)
					myYMin = myH + myYMin;
				if (buffer[1] == '-')
				{
					myYMin = 0;
					myYMax = myH;
				if (myYMin < 0)
					myYMin = 0;
				}
				break;
			}
			myXMin = strtol(buffer+1, NULL, 10);
			if (myXMin < 0)
				myXMin = myW + myXMin;
			if (myXMin < 0)
				myXMin = 0;
			break;

		case 'M': //capital m was xMax or, with y, yMax
			printf("WARNING: %s has deprecated m-command. Use {} instead with 0's for unused min/max, {} to reset.\n", lineTab);
			if (buffer[1] == '{')
			{
				readBrackets(buffer+2);
				break;
			}
			if ((buffer[1] == 'y') || (buffer[1] == 'Y'))
			{
				myYMax = strtol(buffer+2, NULL, 10);
				if (myYMax < 0)
					myYMax = myH + myYMin;
				if (buffer[2] == '-')
				{
					myYMin = 0;
					myYMax = 0;
					if (buffer[3] == '-')
					{
						myXMin = 0;
						myXMax = myW;
					}
				if (myYMax < 0)
					myYMax = 0;
					break;
				}
			}
			myXMax = strtol(buffer+1, NULL, 10);
			if (myXMax < 0)
				myXMax = myW + myXMax;
			if (buffer[1] == '-')
			{
				myXMin = 0;
				myXMax = myW;
			}
			if ((myXMax > myW) || (myXMax == 0))
				myXMax = myW;
			break;

		case 'n':
		case 'N':
			if ((buffer[1] == 'h') || (buffer[1] == 'H'))
				readType = NIB_HIGH_FIRST;
			else if ((buffer[1] == 'l') || (buffer[1] == 'L'))
				readType = NIB_LOW_FIRST;
			else if ((buffer[1] == '0') || (buffer[1] == '-') || (buffer[1] == 0))
				readType = STRAIGHT_BYTES;
			else
				printf("WARNING line %s: NH or NL (nibble/high or low first) are the only two options for N.\
-/0/no byte clears it.\n", lineTab);
			break;

		case 'p': // persist tells you whether {}/m rectangles of what to output should persist or disappear after reading next rectangle
			if ((buffer[1] == '-') || (buffer[1] == '0'))
				persist = 0;
			else if ((buffer[1] == '+') || (buffer[1] == 0) || (buffer[1] == '\n'))
				persist = 1;
			else
				printf("Unknown persist prompt %c at %s, need -/0/+/(empty).\n", buffer[1], lineTab);
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
				printf ("Bailing. Bad qflags value of %d at line %s given %s\n", qflags, lineTab, buffer);
			}
			break;

		case 'O':	//capital o = force the offset if it's under 0x230
		case 'o':   //otherwise just offset in file
			if (buffer[1] == '-')
			{
				overlapOK = 0;
				break;
			}
			if (buffer[1] == '+')
			{
				overlapOK = 1;
				break;
			}
			if (buffer[1] == '=') //used to use d but that conflicts with hex
			{
				myDefaultOffset = strtol(buffer+2, NULL, 16);
				break;
			}

			if (G == NULL)
			{
				printf("Warning: line %s, tried to read offset without file open: %s", lineTab, myLine);
				if (bailOnWarn)
					return 0;
				break;
			}

			needToProc = 0;
			if (!needToBMP)
				needToBMP = (short)curLine;

			myOffset = strtol(buffer+1, NULL, 16);
			if ((buffer[0] == 'o') && (myOffset < 0x230))
				myOffset *= 0x100;

fromr:
			if (debug)
			{
				printf("Rect from %d %d to %d %d\n", myX, myY, myX + myW, myY + myH);
			}
			if (myX + myXMax > MAXW)
			{
				printf("Oops line %s went too far right.\n", lineTab);
				return 0;
			}
			if (myY + myH > MAXH)
			{
				printf("Oops line %s went too far down.\n", lineTab);
				return 0;
			}

			fseek(G, myOffset, SEEK_SET);

			for (j=0; j < myH; j++)
				for (i=0; i < myW; i++)
				{
					i2 = (i+myXModOffset) % myW;
					j2 = (j*goDirection+myYModOffset) % myH;

					ch = fgetc(G) & 0xff;
					if (j < myYMin)
						continue;
					if (myYMax && (j >= myYMax))
						continue;

					if (replace[ch] != -1)
						ch = replace[ch];
					if ((setTransparent) && (ch == transpColor))
						continue;
					if ((i2 >= myXMin) && (i2 < myXMax))
					{
						if (viable(i, j, qflags))
						{
							if (trackOverlap && !overlapOK)
								if (ch != myAry[myX+i2][myY+j2])
								{
									printf("Overlap at %s, (%d, %d) %02x vs %02x.\n", buffer, myX+i2, myY+j2);
									if (blockOverlap)
										continue;
								}
							switch(readType)
							{
							case NIB_HIGH_FIRST:
								myAry[myX+i2][myY+j2] = (ch >> 4);
								myAry[myX+i2+1][myY+j2] = (ch & 0xf);
								i++;
								break;

							case NIB_LOW_FIRST:
								myAry[myX+i2][myY+j2] = (ch & 0xf);
								myAry[myX+i2+1][myY+j2] = (ch >> 4);
								i++;
								break;

							default:
								myAry[myX+i2][myY+j2] = ch;
								break;
							}
						}
					}
					//not perfect. What if we have fringe AND shift-offset? Oh well.
					if ((i == 0) && (doFringe))
						if (overlapOK || (myAry[myX+myW][myY+j2] == 0))
							myAry[myX+myW][myY+j2] = myAry[myX][myY+j2];
				}
			if (doFringe)
				for (i=0; i < myW+1; i++)
					if (overlapOK || (myAry[myX+i][myY+myH] == 0))
					{
						if (trackOverlap && !overlapOK)
							if (ch != myAry[myX+i][myY+myH])
							{
								printf("Overlap at %s, (%d, %d) %02x vs %02x.\n", buffer, myX+i, myY+myH);
								if (blockOverlap)
									continue;
							}
							myAry[myX+i][myY+myH] = ch;
					}

			if (forceV)
			{
				myY += myH;
				forceV = 0;
			}
			else if (forceH)
			{
				myX += myW;
				forceH = 0;
			}
			else if (vertical)
				myY += myH + vJump;
			else
				myX += myW + hJump;
			myXModOffset = 0;
			myYModOffset = 0;

			if (!persist)
			{
			myXMin = 0;
			myXMax = myW;
			}
			if (addSpace == 1)
				myY++;

			break;

		case 'R':	//force vertical or horizontal
			if ((buffer[1] == 'v') || (buffer[1] == 'V'))
			{
				if (forceH)
				{
					printf("Already forcing horizontal line %s.\n", lineTab);
					break;
				}
				if (forceV)
				{
					printf("Already forcing vertical line %s.\n", lineTab);
					break;
				}
				forceV = 1;
			}
			if ((buffer[1] == 'h') || (buffer[1] == 'H'))
			{
				if (forceH)
				{
					printf("Already forcing horizontal line %s.\n", lineTab);
					break;
				}
				if (forceV)
				{
					printf("Already forcing vertical line %s.\n", lineTab);
					break;
				}
			}
			break;

		case 'r': // read in a file
			{
				short gotoJump = 0;
				if (G)
					fclose(G);
				if (buffer[strlen(buffer)-1] == '\n')
					 buffer[strlen(buffer)-1] = 0;
				if (buffer[strlen(buffer)-1] == '/')
				{
					buffer[strlen(buffer)-1] = 0;
					myOffset = myDefaultOffset;
					gotoJump = 1;
				}

				if (needToProc)
				{
					printf("WARNING: %s (line %s) occurs before processing the last 'r' line.\n", buffer, lineTab);
					needToProc = 0;
				}
				if (debug)
					printf("Reading %s, also extension %s\n", buffer+1, myExt);
				G = fopen(buffer+1, "rb");
				if (G == NULL)
				{
					char altFile[200];
					if (myExt[0])
					{
						strcpy(altFile, buffer+1);
						strcat(altFile, myExt);
						G = fopen(altFile, "rb");
						if (G == NULL)
						{
							altFile[strlen(altFile)-strlen(myExt)] = 0;
							strcat(altFile, ".");
							strcat(altFile, myExt);
							G = fopen(altFile, "rb");
						}
					}
					if (G == NULL)
					{
						printf("Oops file %s does not exist. Bailing at line %s.\n", buffer+1, lineTab);
						return 0;
					}
				}
				if (gotoJump)
					 goto fromr;
				else
					needToProc = 1;
			}
			break;

		case 'S': //sector read, usually for Apple and 256. Capital = force no 00's if <x230
		case 's': // ",(#)" means only read that many(hex) bytes
			if (buffer[1] == 't')
			{
				sectorTemp = strtol(buffer+2, NULL, 16);
				break;
			}
			if (buffer[1] == 'r')
			{
				if (curX)
				{
					curX = 0;
					myY++;
				}
				break;
			}

			if (G == NULL)
			{
				printf("Warning: line %s, tried to read sector without file open: %s\n", lineTab, buffer);
				if (bailOnWarn)
					return 0;
				break;
			}

			if (myX + myW > MAXW)
			{
				printf("Uh oh horizontal overrun at %s, bailing.", lineTab);
				return 0;
			}

			needToProc = 0;
			if (!needToBMP)
				needToBMP = (short)curLine;

			{
				for (i=0; i < (short)strlen(buffer); i++) // note this is a very bad, weird bug, and it doesn't work long term as strtok itself is a bit flaky
					if (buffer[i] == ',')
					{
						sectorTemp = (short)strtol(buffer+i+1, NULL, 16);
						break;
					}
			}
			myOffset = strtol(buffer+1, NULL, 16);
			if ((buffer[0] == 's') && ( myOffset < 0x230))
				myOffset *= 0x100;

			if ((commodore) && !(myOffset%0x100))
				myOffset += 2;

			sectorStart = myOffset % 0x100;

			fseek(G, myOffset, SEEK_SET);
			{
				long thisSector = sectorSize;
				if (sectorTemp)
				{
					thisSector = sectorTemp;
					sectorTemp = 0;
				}
				for (i=sectorStart; i < thisSector; i++)
				{
					switch(readType)
					{
					case BIT_HIGH_FIRST:
						ch = fgetc(G) & 0xff;
						for (i = 0; i <= 7; i++)
							myAry[myX+curX+i][myY] = (ch >> (7 - i)) & 1;
						curX += 8;
						break;

					case BIT_LOW_FIRST:
						ch = fgetc(G) & 0xff;
						for (i = 0; i <= 7; i++)
							myAry[myX+curX+i][myY] = (ch >> i) & 1;
						curX += 8;
						break;

					case NIB_HIGH_FIRST:
						ch = fgetc(G) & 0xff;
						myAry[myX+curX][myY] = ch >> 4;
						myAry[myX+curX+1][myY] = ch & 0xf;
						curX += 2;
						break;

					case NIB_LOW_FIRST:
						ch = fgetc(G) & 0xff;
						myAry[myX+curX][myY] = ch & 0xf;
						myAry[myX+curX+1][myY] = ch >> 4;
						curX += 2;
						break;

					default:
						myAry[myX + curX][myY] = fgetc(G) & 0xff;
						curX++;
						break;
					}
					if (curX >= myW)
					{
						curX = 0;
						myY++;
						if (myY >= MAXH)
						{
							printf("Uh oh vertical overrun at %s, bailing.", lineTab);
							return 0;
						}
					}
				}
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

		case 'u': // "up and over"
			if (vertical)
				myLastX = myLastX + strtol(buffer+1, NULL, 10);
			else
				myLastY = myLastY + strtol(buffer+1, NULL, 10);
			myY = myLastY;
			myX = myLastX;
			break;

		case 'V':
			{
				short wasVert = vertical;
				if (buffer[1] == '+')
					vertical = 1;
				else if (buffer[1] == '-')
					vertical = 0;
				else
				{
					printf("V needs to have + or -.");
					break;
				}
				if (vertical == wasVert)
					printf("Warning line %s doesn't really change anything as vertical was already %c.\n",
						lineTab, buffer[1]);
			}
			break;

		case 'v':
			myYModOffset = strtol(buffer+1, NULL, 10);
			break;

		case 'W':
			if (buffer[1] == '-')
				outWi = strtol(buffer+2, NULL, 10);
			else
				outW = strtol(buffer+1, NULL, 10);
			break;

		case 'w':
			myW = strtol(buffer+1, NULL, 10);
			myXMax = myW;
			break;

		case 'x': //x/y move you to absolute coordinates myX+myAbsX
			if (buffer[1] == 'd')
			{
				myX = strtol(buffer+2, NULL, 10) + myX;
				myLastX = myX;
			}
			else
			{
			myX = strtol(buffer+1, NULL, 10);
			myLastX = myX;
			}
			break;

		case 'y': //x/y move you to absolute coordinates myX+myAbsX
			if (buffer[1] == 'd')
			{
				myY += strtol(buffer+2, NULL, 10);
				myLastY = myY;
			}
			else
			{
			myY = strtol(buffer+1, NULL, 10);
			myLastY = myY;
			}
			break;

		case '#': //commented line after tabl
			break;

		default:
			printf("Unknown command for line %s: %s\n", lineTab, buffer);
			if (bailOnWarn)
				return 0;
			break;

		}
		tabIndex++;
		}
		while (buffer=strtok(NULL, "\t"));
	}
	if (needToBMP)
	{
		printf("You didn't flush some data to a BMP, starting at line %d.\n", needToBMP);
	}
	if (commentBlock)
	{
		printf("Runaway comment block at line %d.\n", commentBlock);
	}
	if (G) fclose(G);
	return 0;

}

void readBrackets(char * x)
{
	int i;
	short commas = 0;
	short nextInt = 1;

	for (i=0; i < (int)strlen(x); i++)
		commas += (x[i] == ',');

	if (commas != 3)
	{
		printf("Line/tab %s needs 3 commas for m{}. You have %d.\n", lineTab, commas);
	}

	commas = 0;
	for (i=0; i < (int)strlen(x); i++)
	{
		if (nextInt)
		{
			nextInt = 0;
			commas++;
			switch (commas)
			{
			case 1:
				myXMin = strtol(x+i, NULL, 10);
				if (myXMin < 0)
					myXMin += myW;
				if (myXMin < 0)
					myXMin = 0;
				break;
			case 2:
				myYMin = strtol(x+i, NULL, 10);
				if (myYMin < 0)
					myYMin += myH;
				if (myYMin < 0)
					myYMin = 0;
				break;
			case 3:
				myXMax = strtol(x+i, NULL, 10);
				if (myXMax < 0)
					myXMax += myW;
				if ((myXMax < 0) || (myXMax > myW))
					myXMax = myW;
				break;
			case 4:
				myYMax = strtol(x+i, NULL, 10);
				if (myYMax < 0)
					myYMax += myH;
				if (myYMax < 0)
					myYMax = 0;
				i = strlen(x); //cheating to save a bit of time
				break;
			default:
				printf("%s: UH OH. This really should not have happened in m{} %d commas found.\n", lineTab, commas);
				break;
			}
		}
		if (x[i] == ',')
			nextInt = 1;
	}
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

//I'm  not sure what this does since everything returns true. It is probably a hack.
bool viable (int a1, int a2, int a3)
{
	if (a3 == 15) { return true; }
	if ((a3 == 12) && (a1 % 2 == 0) && (a2 % 2 == 1)) { return true; }
	if ((a3 == 9) && (a1 % 2 == 0) && (a2 % 2 == 0)) { return true; }
	if ((a3 == 6) && (a1 % 2 == 1) && (a2 % 2 == 1)) { return true; }
	if ((a3 == 3) && (a1 % 2 == 0) && (a2 % 2 == 1)) { return true; }
	return 1;
}

void usage()
{
	printf("Flag -? for this help command.\n\
Flag -b bails on any warning.\n\
Flag -d prints out debug text.\n\
Flag -l forces launch of generated BMP unless locally specified with >- or >+.\n\
Flag -o flags overlap, -ob blocks it\n\
Flag -u prints out how to write a file for mapcpy to read.\n");
}

void mapCpyFmt()
{
	printf("The basics:\n\
You need W#, H# and w (for width of input) and x#/y# tells where to start\n\
E= gives default file name extension\n\
rFILENAME reads file (slash after means read default # of chars)\n\
O/o goes to an offset (O doesn't auto multiply small #s by 0x100 hex), O= specifies file default offset\n\
S/s reads a sector (256 bytes unless a comma separated value after)\n\
Semicolon ends it\n\
dy/dx adjusts where you are e.g. dy+2, dx-2\n\
u18 moves you down/right (depending on if v+ (default) or v- is set)\n\
$ does copy/paste (xi, yi, xclip, yclip, optional xTo, yTo) : (rlf)\n\
> writes to output\n\
* clears to the given number\n\
&v-8 sets vertical adjust -8 after reading a rectangle, &h+8 horizontal +8, &4 both +4\n\
# comments things out\n\
You may use tabs to separate commands\n\
https://github.com/andrewschultz/rpg-mapping-tools/tree/master/samples contains examples\n\
    especially magic-candle, phantasie and 2400\n");
}

//features:
//postprocess flag (bmp2png)