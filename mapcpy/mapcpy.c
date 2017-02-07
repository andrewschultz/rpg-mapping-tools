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

#define MAXW 640
#define MAXH 640

typedef enum { false, true } bool;

short isYellow(short x); // very limited function
bool viable (int a1, int a2, int a3); //very limited function

void usage();
void mapCpyFmt();

main(int argc, char * argv[])
{
	short replace[256] = { -1 };
	short addSpace = 0;

	short needToProc = 0;
	short needToBMP = 0;
	short commentBlock = 0;

	FILE * F, * G = NULL, * H, * I;
	long keepGoing = 1;
	unsigned int count = 1;
	short goDirection = 1;
	long toSet = 0, temp = 0, curLine = 0;
	long myDefaultOffset = 0;
	long myAbsX = 0, myAbsY = 0;
	long myLastX = 0, myLastY = 0, myX = 0, myY = 0, myH = 0, myW = 0, myOffset = 0, myXMin = 0, myXMax = 32, setTransparent = 0, transpColor = 0,
		myYModOffset=0, myXModOffset=0, outH=256, outW=256, doFringe = 0, gegege=0;
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

	short myary[MAXW][MAXW], ch;
	char myExt[10] = "";

	char myFile[200];
	char myLine[200];
	char * buffer;

	short gotFile = 0;
	short debug = 0;

	while ((short)count < argc)
	{
		if (argv[count][0] == '-')
		{
			switch(argv[count][1])
			{
			case '?':
				usage();
				return 0;

			case 'd':
			case 'D':
				debug = 1;
				count++;
				break;

			case 'l':
			case 'L':
				launch = 1;
				count++;
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
		printf("no such file\n");
		return 0;
	}

	count = 0;

	for (j=0; j < MAXH; j++)
		for (i=0; i < MAXW; i++)
			myary[i][j] = 0;

	for (i=0; i < 256; i++)
		replace[i] = -1;

	while ((fgets(myLine, 200, F) != NULL) && (keepGoing))
	{
		curLine++;
		if (commentBlock)
		{
			if ((myLine[1] == '-') && (myLine[0] == '#'))
				commentBlock = 0;
			else
				continue;
		}
		buffer = strtok(myLine, "\t");
		do
		{
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

		case '#':
			if (buffer[1] == '+')
			{
				commentBlock = (short)curLine;
				break;
			}

			if (buffer[1] == '!')
				printf("%s", buffer);
		case '\n':
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

		case '0':
			addSpace = 0;
			break;

		case '1':
			addSpace = 1;
			break;

		case '*':
			for (i=0; i < MAXW; i++)
				for (j=0; j < MAXW; j++)
					myary[i][j] = 0;
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

		case 'y':
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

		case 'd': // this changes the starting point to draw the rectangle
			if (buffer[1] == 'x')
			{
				if (buffer[2] == '-')
				{
					if (myX < strtol(buffer+3, NULL, 10))
					{
						printf("WARNING dx command went to -x, %d back %d at line %d\n", myX, strtol(buffer+3, NULL, 10), curLine);
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
						printf("WARNING dy command went to -y, %d back %d at line %d\n", myY, strtol(buffer+3, NULL, 10), curLine);
						break;
					}
					myY -= strtol(buffer+3, NULL, 10);
				}
				else
					myY += strtol(buffer+2, NULL, 10);
				//printf("Adding %d to y, now %d\n", strtol(buffer+2, NULL, 10), myY);
				break;
			}
			printf("Error at line %d. d(xy) is proper usage for moving.\n", curLine);
			break;

		//+/-/^/v rotate the start-x in the rectangle. For moving around, use dx/dy
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
			myXMax = myW;
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

		case 'R':	//force vertical or horizontal
			if ((buffer[1] == 'v') || (buffer[1] == 'V'))
			{
				if (forceH)
				{
					printf("Already forcing horizontal line %d.\n", curLine);
					break;
				}
				if (forceV)
				{
					printf("Already forcing vertical line %d.\n", curLine);
					break;
				}
				forceV = 1;
			}
			if ((buffer[1] == 'h') || (buffer[1] == 'H'))
			{
				if (forceH)
				{
					printf("Already forcing horizontal line %d.\n", curLine);
					break;
				}
				if (forceV)
				{
					printf("Already forcing vertical line %d.\n", curLine);
					break;
				}
			}
			break;

		case 'r':
			if (G)
				fclose(G);
			if (buffer[strlen(buffer)-1] == '\n')
				 buffer[strlen(buffer)-1] = 0;
			if (needToProc)
			{
				printf("WARNING: %s (line %d) occurs before processing the last 'r' line.\n", buffer, curLine);
			}
			//debug below
			//printf("Reading %s\n", buffer+1);
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
					printf("oops file %s does not exist bailing at line %d\n", buffer+1, curLine);
					return 0;
				}
			}
			needToProc = 1;
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
					printf("Warning line %d doesn't really change as vertical was already %c.\n", curLine, buffer[1]);
			}
			break;

		case 'e':
			if (buffer[1] == '=')
			{
				buffer[strlen(buffer)-1] = 0;
				strcpy(myExt, buffer+2);
			}
			break;
		
		case 'f': // switches which direction a partial-save will go
			goDirection = 0 - goDirection;
			break;

		case 'g': // this is specific to Gegege No Kitaro
			if (buffer[1] == 'e')
				gegege=1;
			break;

		case 'L':
			myLastY = myY;
			myLastX = myX;
			break;

		case 'u': // "up and over"
			if (vertical)
				myLastX = myLastX + strtol(buffer+1, NULL, 10);
			else
				myLastY = myLastY + strtol(buffer+1, NULL, 10);
			myY = myLastY;
			myX = myLastX;
			break;

		case '>': // this outputs to a file
			localLaunch = launch;
			buffer[strlen(buffer)-1] = 0;
			if (needToProc)
			{
				printf("WARNING: %s (line %d) occurs before processing the last 'r' line.\n", buffer, curLine);
			}
			if (buffer[1] == '+')
			{
				localLaunch = 1;
				I = fopen(buffer+2, "wb");
			}
			else if (buffer[1] == '-')
			{
				localLaunch = 0;
				I = fopen(buffer+2, "wb");
			}
			else
				I = fopen(buffer+1, "wb");
			if (I == NULL)
			{
				printf("Couldn't read %s.\n", buffer+1+launch);
				break;
			}

			needToBMP = 0;
			printf("Writing %s\n", buffer+1+launch);
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
					if (myary[i][j] == 0x03)
						if (isYellow(myary[i][j-1]) || isYellow(myary[i][j+1]) || isYellow(myary[i+1][j]) || isYellow(myary[i-1][j]))
						if ((myary[i][j-1] >> 4) && (myary[i][j+1] >> 4) && (myary[i+1][j] >> 4) && (myary[i-1][j] >> 4))
							myary[i][j] = 0x4e;
			}

			temp = ((outW+3)>>2)<<2;
			for (j=0; j < outH; j++)
			{
				for (i=0; i < outW; i++)
					fputc(myary[i][outH-1-j], I);
				if (outW % 4)
					for (i=outW; i < temp; i++)
						fputc(0, I);
			}

			fclose(I);
			if (localLaunch)
			{
				char cmdbuf[100] = "mspaint ";
				strcat(cmdbuf, buffer+2);
				system(cmdbuf);
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
			if (buffer[1] == 'd')
			{
				myDefaultOffset = strtol(buffer+2, NULL, 16);
				break;
			}
			if (debug)
			{
				printf("Rect from %d %d to %d %d\n", myX, myY, myX + myW, myY + myH);
			}
			if (myX + myXMax > MAXW)
			{
				printf("Oops line %d went too far right.\n", curLine);
				return 0;
			}
			if (myY + myH > MAXH)
			{
				printf("Oops line %d went too far down.\n", curLine);
				return 0;
			}

			needToProc = 0;
			if (!needToBMP)
				needToBMP = (short)curLine;

			myOffset = strtol(buffer+1, NULL, 16);
			if ((buffer[0] == 'o') && (myOffset < 0x230))
				myOffset *= 0x100;

			fseek(G, myOffset, SEEK_SET);

			for (j=0; j < myH; j++)
				for (i=0; i < myW; i++)
				{
					i2 = (i+myXModOffset) % myW;
					j2 = (j*goDirection+myYModOffset) % myH;
					ch = fgetc(G) & 0xff;
					if (replace[ch] != -1)
						ch = replace[ch];
					if ((setTransparent) && (ch == transpColor))
						continue;
					if ((i2 >= myXMin) && (i2 < myXMax))
					{
						if (viable(i, j, qflags))
						{
							if (overlapOK || (myary[myX+i2][myY+j2] == 0))
								myary[myX+i2][myY+j2] = ch;
						}
					}
					//not perfect. What if we have fringe AND shift-offset? Oh well.
					if ((i == 0) && (doFringe))
						if (overlapOK || (myary[myX+myW][myY+j2] == 0))
							myary[myX+myW][myY+j2] = myary[myX][myY+j2];
				}
			if (doFringe)
				for (i=0; i < myW+1; i++)
					if (overlapOK || (myary[myX+i][myY+myH] == 0))
						myary[myX+i][myY+myH] = myary[myX+i][myY];
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
				myY += myH;
			else
				myX += myW;
			myXModOffset = 0;
			myYModOffset = 0;
			myXMin = 0;
			myXMax = myW;
			if (addSpace == 1)
				myY++;
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

			needToProc = 0;
			if (!needToBMP)
				needToBMP = (short)curLine;

			{
				char buf2[100];
				char * token;
				char seps[] = ",";
				strcpy(buf2, buffer);
				token = strtok(buf2, seps);
				token = strtok(NULL, seps);
 				if (token)
					sscanf(token, "%x", &sectorTemp);
			}
			myOffset = strtol(buffer+1, NULL, 16);
			if ((buffer[0] == 's') && ( myOffset < 0x230))
				myOffset *= 0x100;
			fseek(G, myOffset, SEEK_SET);
			{
				long thisSector = sectorSize;
				if (sectorTemp)
				{
					thisSector = sectorTemp;
					sectorTemp = 0;
				}
				for (i=0; i < thisSector; i++)
				{
					myary[myX + curX][myY] = fgetc(G) & 0xff;
					curX++;
					if (curX == myW)
					{
						curX = 0;
						myY++;
					}
				}
			}
			break;

		default:
			printf("Unknown line: %s", buffer);
			break;

		}
		} while (buffer=strtok(NULL, "\t"));
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
Flag -d prints out debug text.\n\
Flag -l forces launch of generated BMP unless locally specified with >- or >+.\n\
Flag -u prints out how to write a file for mapcpy to read.\n");
}

void mapCpyFmt()
{
	printf("The basics You need W#, H# and w (for width of input) and x#/y# tells where to start\n\
O/o goes to an offset (O doesn't auto multiply small #s by 0x100 hex)\n\
S/s reads a sector (256 bytes unless a comma separated value after)\n\
Semicolon ends it\n\
dy/dx adjusts where you are\n\
Flag -d prints out debug text.\n\
Flag -u prints out how to write a file for mapcpy to read.\n\
https://github.com/andrewschultz/rpg-mapping-tools/tree/master/samples contains examples\n\
    especially magic-candle, phantasie and 2400\n");
}
