//editor.cpp
//copyright 2008-2017 by Andrew Schultz
//this program edits a wall based map and allows for
//  icons inside squares
//  tracking squares done
//  1 way doors
#define WIN32_LEAN_AND_MEAN

#include <ddraw.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include <commdlg.h>
#include <io.h>

#include <stdlib.h>

#include "Shlwapi.h"

#include "editor.h"

#define ICONWIDTH 16
#define ICONHEIGHT 16
#define MAXICONSWIDE 35
#define MAXICONSHIGH 35
#define MAXFILENAME 256

#define WALLICONSTART 38

#define GRIDLINE 8
#define MAXWALLICON 10
#define PIXELS_IN 4
#define BEENTHERE 0xd3

#define LEFTWALL LRWallArray[xCurrent][yCurrent]
#define RIGHTWALL LRWallArray[xCurrent+1][yCurrent]
#define UPWALL UDWallArray[xCurrent][yCurrent]
#define DOWNWALL UDWallArray[xCurrent][yCurrent+1]

#define BLACK RGB(0,0,0)
#define RED RGB(255,0,0)
#define WHITE RGB(255,255,255)

#define XVAL(a,b) (SquareIconArray[a][b]%16)*16
#define YVAL(a,b) (SquareIconArray[a][b]/16)*16
// FUNCTION DEFS
void DoOpenFile(HWND hwnd);
void DoCreateFile(HWND hwnd);
void DrawPointers(HWND hwnd, COLORREF myColor);
void DrawPointerRectangle(HWND hwnd, long xOffset, long yOffset, COLORREF myColor);
void ReadBinaryMap(HWND hwnd, char x[MAXFILENAME]);
void ReloadTheMap(HWND hwnd);
void showClipContents(HWND hwnd);
void drawMyIcons(HWND hwnd);
void changeBarText(HWND hwnd);
void regularTextOut(char x[100], HWND hwnd);
void noteNewShiftBox(HWND hwnd);

void CreateNewMapfile(long, long);
void SaveMapfile();
void flipStuff (HWND hwnd, int xi, int yi, int xf, int yf);
void SaveBitmapFile(HWND hwnd, short trim);
void parseCmdLine();

char textToShift[200]; // receives name of item to delete.
void setInitialChecks(HWND hwnd);
void switchIconPointer(HWND hwnd, short q);
void shiftTheMap(HWND hwnd, short x, short y);
void checkShiftExpand(HWND hwnd, short dl, short dr, short du, short dd);

BOOL CALLBACK ShiftDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK HexDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);

// DEFINES ////////////////////////////////////////////////

// defines for windows
#define WINDOW_CLASS_NAME "WINCLASS1"

// MACROS /////////////////////////////////////////////////

#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// GLOBALS ////////////////////////////////////////////////
HWND hwnd = NULL; // save the window handle
HDC icondc, walldc;

short UDWallArray[MAXICONSWIDE+1][MAXICONSHIGH+1];
short LRWallArray[MAXICONSWIDE+1][MAXICONSHIGH+1];
short SquareIconArray[MAXICONSWIDE][MAXICONSHIGH];
short TranspIconArray[MAXICONSWIDE][MAXICONSHIGH];
short CutUDWallArray[MAXICONSWIDE+1][MAXICONSHIGH+1];
short CutLRWallArray[MAXICONSWIDE+1][MAXICONSHIGH+1];
short CutSquareIconArray[MAXICONSWIDE][MAXICONSHIGH];

short ControlKeyDown = 0;
long MouseDownX, MouseDownY;
long xCurrent = 0, yCurrent = 0, iconNumber = 0, massCutPasteIcon;
long notPeriod = 0;
long WallIconNumber = 2;	//default to doors. Can add/delete walls with (shift) WASZ
long myW = MAXICONSWIDE, myH = MAXICONSHIGH;
char CurrentFileName[200] = "";

long cutHeight=0, cutWidth = 0;
long zeroBottom = 1;
short myEmpty = 0, myTransparency = 0, showPointerRectangle = 1;
long showDebug = 1;

short cutPasteEdges = 1;
short shiftLeft = 0, shiftRight = 31, shiftUp = 0, shiftDown = 31;

long bufferL=0,bufferR=0,bufferU=0,bufferD=0;
long wrapType = ID_OTHER_NOWRAP;

long workNotSaved = 0;

short prevDefArray[10] = {
	0x3f, 0x26, 0x56, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0x3f
};

char iconFileName[200] = "icons.bmp";
char wallFileName[200] = "walls.bmp";
char startFileName[200] = "";

long shotcount=0;

// FUNCTIONS //////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hwnd,
						    UINT msg,
                            WPARAM wparam,
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT		ps;		// used in WM_PAINT
HDC				hdc;	// handle to a device context

// what is the message
switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
		case ID_FILE_CREATE_NEW:
			DoCreateFile(hwnd);
			break;

		case ID_FILE_OPEN:
			DoOpenFile(hwnd);
			ReloadTheMap(hwnd);
			break;

		case ID_FILE_SAVE_BITMAP:
			ReloadTheMap(hwnd);
			SaveBitmapFile(hwnd, 0);
			break;

		case ID_FILE_SAVE_BITMAP_TRIM:
			ReloadTheMap(hwnd);
			SaveBitmapFile(hwnd, 1);
			break;

		case ID_FILE_SAVE_MAPFILE:
			SaveMapfile();
			break;

		case ID_EDIT_RELOAD_ALL:
		case ID_EDIT_RELOAD:
			{
				ReloadTheMap(hwnd);
				drawMyIcons(hwnd);
			}
				break;

		case ID_EDIT_WIPE:
			{
				long i, j;
				for (i=0; i < 35; i++)
					for (j=0; j < 35; j++)
						if (SquareIconArray[i][j] == BEENTHERE)
							SquareIconArray[i][j] = 0;
				ReloadTheMap(hwnd);
			}
			break;

		case ID_EDIT_FLIP_STAIRS:

			{
				long i, j;
				for (i=0; i < 35; i++)
					for (j=0; j < 35; j++)
					{
						switch(SquareIconArray[i][j])
						{
						case 0x26:
						case 0x56:
							SquareIconArray[i][j] = 0x7c - SquareIconArray[i][j];
							break;	//yes cutesy code I don't care

						default:
							break;
						}
					}
				ReloadTheMap(hwnd);
			}
			break;

		case ID_EDIT_FLIP_TELEPORT:
			{
				long iNum = iconNumber >> 4;
				if ((iNum == 4) || (iNum == 6))
				{
					DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(0,0,0));
					iconNumber ^= 0x20;
					DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
				}
			}
			break;

		case ID_EDIT_COPY:
		case ID_EDIT_CUT:
			{
				long i, j;

				if (bufferL == -1)
					break;

				cutHeight = bufferU-bufferD+1;
				cutWidth = bufferR-bufferL+1;

				if (bufferL > bufferR)
				{
					MessageBox(hwnd, "left-right buffer is wrong", "oops", MB_OK);
					break;
				}

				if (bufferU > bufferD)
				{
					MessageBox(hwnd, "up-down buffer is wrong", "oops", MB_OK);
					break;
				}

				for (j=bufferU; j <= bufferD; j++)
					for (i=bufferL; i <= bufferR; i++)
						CutSquareIconArray[i-bufferL][j-bufferU] = SquareIconArray[i][j];

				for (j=bufferU; j <= bufferD+1; j++)
					for (i=bufferL; i <= bufferR; i++)
						CutUDWallArray[i-bufferL][j-bufferU] = UDWallArray[i][j];

				for (j=bufferU; j <= bufferD; j++)
					for (i=bufferL; i <= bufferR+1; i++)
						CutLRWallArray[i-bufferL][j-bufferU] = LRWallArray[i][j];

				if (LOWORD(wparam) == ID_EDIT_CUT)
				{
					short edgeTrim = 1 - cutPasteEdges;
					for (j=bufferU; j <= bufferD; j++)
						for (i=bufferL; i <= bufferR; i++)
							SquareIconArray[i][j] = 0;

					for (j=bufferU + edgeTrim; j <= bufferD + 1 - edgeTrim; j++)
						for (i=bufferL + edgeTrim; i <= bufferR - edgeTrim; i++)
							UDWallArray[i][j] = 0;

					for (j=bufferU + edgeTrim; j <= bufferD - edgeTrim; j++)
						for (i=bufferL + edgeTrim; i <= bufferR + 1 - edgeTrim; i++)
							LRWallArray[i][j] = 0;

					workNotSaved = 1;
				}
				ReloadTheMap(hwnd);
			}
			break;

		case ID_EDIT_PASTE:
			if (bufferL == -1)
				break;
			{
				short edgeTrim = 1 - cutPasteEdges;
				long i, j;

				for (j=bufferU; j <= bufferD; j++)
					for (i=bufferL; i <= bufferR; i++)
						if ((!myTransparency) || (CutSquareIconArray[i+xCurrent-bufferL][j+yCurrent-bufferU]))
							SquareIconArray[i+xCurrent-bufferL][j+yCurrent-bufferU] = CutSquareIconArray[i-bufferL][j-bufferU];

				for (j=bufferU + edgeTrim; j <= bufferD+1 - edgeTrim; j++)
					for (i=bufferL + edgeTrim; i <= bufferR - edgeTrim; i++)
						if ((!myTransparency) || (CutUDWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU]))
							UDWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU] = CutUDWallArray[i-bufferL][j-bufferU];

				for (j=bufferU + edgeTrim; j <= bufferD - edgeTrim; j++)
					for (i=bufferL + edgeTrim; i <= bufferR + 1 - edgeTrim; i++)
						if ((!myTransparency) || (CutLRWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU]))
							LRWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU] = CutLRWallArray[i-bufferL][j-bufferU];
			}
			workNotSaved = 1;
			ReloadTheMap(hwnd);
			break;

		case ID_EDIT_CUT_PASTE_EDGES:
			cutPasteEdges = !cutPasteEdges;
			if (cutPasteEdges)
				CheckMenuItem( GetMenu(hwnd), ID_EDIT_CUT_PASTE_EDGES, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_EDIT_CUT_PASTE_EDGES, MF_UNCHECKED);
			break;

		case ID_EDIT_CLEARALL:
			{
				short i, j;
				for (j=0; j < myH; j++)
					for (i=0; i < myW; i++)
						UDWallArray[i][j] = LRWallArray[i][j] = SquareIconArray[i][j] = 0;
				workNotSaved = 1;
			}
			break;

		case ID_EDIT_SEECLIP:
			showClipContents(hwnd);
			break;

		case ID_EDIT_TRANSPARENCY:
			myTransparency ^= 1;
			if (myTransparency)
				CheckMenuItem( GetMenu(hwnd), ID_EDIT_TRANSPARENCY, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_EDIT_TRANSPARENCY, MF_UNCHECKED);
			break;


		case ID_EDIT_CHOOSEMASSCPICON:
			massCutPasteIcon = iconNumber;
			break;

		case ID_EDIT_ICONMASSCP:
			{
				long i, j;
				for (j=0; j < MAXICONSHIGH; j++)
					for (i=0; i < MAXICONSWIDE; i++)
						if (SquareIconArray[i][j] == massCutPasteIcon)
							SquareIconArray[i][j] = (short)iconNumber;
			}
			ReloadTheMap(hwnd);
			break;

		case ID_EDIT_FILL_GRIDLINES:
			{
			long i, j;
			for (j=0; j < MAXICONSHIGH; j++)
				for (i=0; i < MAXICONSWIDE; i++)
				{
					if ((UDWallArray[i][j] == 0) && (i < MAXICONSWIDE-1))
						UDWallArray[i][j] = GRIDLINE;
					if ((LRWallArray[i][j] == 0) && (j < MAXICONSHIGH-1))
						LRWallArray[i][j] = GRIDLINE;
				}
			}
			ReloadTheMap(hwnd);
			break;

		case ID_EDIT_EMPTY_GRIDLINES:
			{
			long i, j;
			for (j=0; j < MAXICONSHIGH; j++)
				for (i=0; i < MAXICONSWIDE; i++)
				{
					if (UDWallArray[i][j] == GRIDLINE)
						UDWallArray[i][j] = 0;
					if (LRWallArray[i][j] == GRIDLINE)
						LRWallArray[i][j] = 0;
				}
			}
			ReloadTheMap(hwnd);
			break;

		case ID_EDIT_MOVE_TO_ICON:
			//GotoDlgCtrl(GetDlgItem(ID_TEXTBOX_ITEM_TO_JUMP));
			DialogBox(NULL, MAKEINTRESOURCE(IDD_HEXDIALOG), hwnd, reinterpret_cast<DLGPROC>(HexDlgProc));
			if (textToShift[0])
			{
				long x = strtol(textToShift, NULL, 16);
				if (x >= 0x100)
				{
					MessageBox(NULL, "Must be between 0 and 0xff.", "Bad input", MB_OK);
					break;
				}
				switchIconPointer(hwnd, (short)x);
			}
			break;

		case ID_SHIFT_LEFT:
			shiftTheMap(hwnd,-1,0);
			break;

		case ID_SHIFT_RIGHT:
			shiftTheMap(hwnd,1,0);
			break;

		case ID_SHIFT_UP:
			shiftTheMap(hwnd,0,-1);
			break;

		case ID_SHIFT_DOWN:
			shiftTheMap(hwnd,0,1);
			break;

		case ID_SHIFT_DIALOG:
			DialogBox(NULL, MAKEINTRESOURCE(IDD_SHIFTDIALOG), hwnd, reinterpret_cast<DLGPROC>(ShiftDlgProc));
			noteNewShiftBox(hwnd);
			break;

		case ID_SHIFT_CHOOSE_3232:
			shiftLeft = shiftUp = 0;
			shiftRight = shiftDown = 31;
			noteNewShiftBox(hwnd);
			break;

		case ID_SHIFT_CHOOSE_UL:
		case ID_SHIFT_CHOOSE_UR:
		case ID_SHIFT_CHOOSE_DL:
		case ID_SHIFT_CHOOSE_DR:
			shiftLeft = 17 * ((wparam - ID_SHIFT_CHOOSE_UL) & 1);
			shiftUp = 17 * (((wparam - ID_SHIFT_CHOOSE_UL) & 2) >> 1);
			shiftRight = shiftLeft + 15;
			shiftDown = shiftUp + 15;
			noteNewShiftBox(hwnd);
			break;

		case ID_SHIFT_EXPAND_UPLEFT:
			checkShiftExpand(hwnd,-1,0,-1,0);
			break;

		case ID_SHIFT_EXPAND_UP:
			checkShiftExpand(hwnd,0,0,-1,0);
			break;

		case ID_SHIFT_EXPAND_UPRIGHT:
			checkShiftExpand(hwnd,0,1,-1,0);
			break;

		case ID_SHIFT_EXPAND_LEFT:
			checkShiftExpand(hwnd,-1,0,0,0);
			break;

		case ID_SHIFT_EXPAND_RIGHT:
			checkShiftExpand(hwnd,0,1,0,0);
			break;

		case ID_SHIFT_EXPAND_DOWNLEFT:
			checkShiftExpand(hwnd,-1,0,0,1);
			break;

		case ID_SHIFT_EXPAND_DOWN:
			checkShiftExpand(hwnd,0,0,0,1);
			break;

		case ID_SHIFT_EXPAND_DOWNRIGHT:
			checkShiftExpand(hwnd,0,1,0,1);
			break;

		case ID_SHIFT_CUT_UPLEFT:
			checkShiftExpand(hwnd,1,0,1,0);
			break;

		case ID_SHIFT_CUT_UP:
			checkShiftExpand(hwnd,0,0,1,0);
			break;

		case ID_SHIFT_CUT_UPRIGHT:
			checkShiftExpand(hwnd,0,-1,1,0);
			break;

		case ID_SHIFT_CUT_LEFT:
			checkShiftExpand(hwnd,1,0,0,0);
			break;

		case ID_SHIFT_CUT_RIGHT:
			checkShiftExpand(hwnd,0,-1,0,0);
			break;

		case ID_SHIFT_CUT_DOWNLEFT:
			checkShiftExpand(hwnd,1,0,0,-1);
			break;

		case ID_SHIFT_CUT_DOWN:
			checkShiftExpand(hwnd,0,0,0,-1);
			break;

		case ID_SHIFT_CUT_DOWNRIGHT:
			checkShiftExpand(hwnd,0,-1,0,-1);
			break;

		case ID_OTHER_NOWRAP:
		case ID_OTHER_16WRAP:
		case ID_OTHER_32WRAP:
		case ID_OTHER_CUSTWRAP:
			CheckMenuItem( GetMenu(hwnd), wrapType, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), wparam, MF_CHECKED);
			wrapType = wparam;
			break;

		case ID_OTHER_SHOW_POINTER_RECTANGLE:
			showPointerRectangle ^= 1;
			if (showPointerRectangle)
				CheckMenuItem( GetMenu(hwnd), ID_OTHER_SHOW_POINTER_RECTANGLE, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OTHER_SHOW_POINTER_RECTANGLE, MF_UNCHECKED);
			break;

		case ID_DEBUG_TOGGLE:
			showDebug ^= 1;
			if (showDebug)
				CheckMenuItem( GetMenu(hwnd), ID_DEBUG_TOGGLE, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_DEBUG_TOGGLE, MF_UNCHECKED);
			break;

		case ID_MOVE_CENTER:
			xCurrent = 16;
			yCurrent = 16;
			DrawPointerRectangle(hwnd, 17, 0, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 0, 17, RGB(255,0,0));
			break;

		case ID_MOVE_UL:
			xCurrent = 0;
			yCurrent = 0;
			DrawPointerRectangle(hwnd, 1, 0, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 0, 1, RGB(255,0,0));
			break;

		case ID_MOVE_DR:
			xCurrent = 32;
			yCurrent = 32;
			DrawPointerRectangle(hwnd, 32, 0, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 0, 32, RGB(255,0,0));
			break;

		case ID_MOVE_ALLUP:
			yCurrent = 0;
			DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 0, 1, RGB(255,0,0));
			break;

		case ID_MOVE_ALLDOWN:
			yCurrent = 32;
			DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 0, 33, RGB(255,0,0));
			break;

		case ID_MOVE_ALLLEFT:
			xCurrent = 0;
			DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 1, 0, RGB(255,0,0));
			break;

		case ID_MOVE_ALLRIGHT:
			xCurrent = 32;
			DrawPointerRectangle(hwnd, 33, 0, RGB(255,0,0));
			DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,0,0));
			break;

		case ID_MOVE_PREV16:
			yCurrent += 17;
		case ID_MOVE_NEXT16:
			xCurrent += 17;
			if (xCurrent > 33)
			{
				xCurrent -= 34;
				yCurrent += 17;
			}
			yCurrent %= 34;
			DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,0,0));
			DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,0,0));
			break;

		case ID_VERIFY_16WRAP:
			{
				char buffer[500]="";
				long i, count=0;

				for (i=0; i < 33; i++)
				{
					if (UDWallArray[i][0] != UDWallArray[i][16])
					{
						if (count < 4)
							sprintf(buffer, "%s%x: %x,0 mismatch %x,16\n", buffer, ++count, i, i);
					}
					if (UDWallArray[i][17] != UDWallArray[i][33])
					{
						if (count < 4)
							sprintf(buffer, "%s%x: %x,17 mismatch %x,33\n", buffer, ++count, i, i);
					}
					if (LRWallArray[0][i] != LRWallArray[16][i])
					{
						if (count < 4)
							sprintf(buffer, "%s%x: 0,%x mismatch 16,%d\n", buffer, ++count, i, i);
					}
					if (LRWallArray[17][i] != LRWallArray[33][i])
					{
						if (count < 4)
							sprintf(buffer, "%s%x: 17,%x mismatch 33,%d\n", buffer, ++count, i, i);
					}
				}
				if (count)
					MessageBox(hwnd, buffer, "Mismatches", MB_OK);
				else
					MessageBox(hwnd, "pasta fazoo", "No mismatches", MB_OK);
			}
			break;

		case ID_VERIFY_32WRAP:
			{
				char buffer[500]="";
				long i;
				long count = 0;

				for (i=0; i < 32; i++)
				{
					if (UDWallArray[i][0] != UDWallArray[i][32])
					{
						if (count < 4)
							sprintf(buffer, "%s%x: %d,0 mismatch %x,32\n", buffer, ++count, i, i);
					}
					if (LRWallArray[0][i] != LRWallArray[32][i])
					{
						if (count < 4)
							sprintf(buffer, "%s%x: 0,%d mismatch 32,%x\n", buffer, ++count, i, i);
					}
				}
				if (count)
					MessageBox(hwnd, buffer, "Mismatches", MB_OK);
				else
					MessageBox(hwnd, "woohoo", "No mismatches", MB_OK);
			}
			break;

		case ID_VERIFY_TWO1616:
		case ID_VERIFY_FULL3232:
		case ID_VERIFY_TRIMMED:
			myEmpty = 32;
			if (LOWORD(wparam) == ID_VERIFY_TWO1616)
				myEmpty = 16;
			{
				long accounted=0;
				long firstX=-1, firstY=-1, i, j, lastX, lastY, count=0;
				long xi=0,yi=0,xf=32,yf=32;

				if (LOWORD(wparam) == ID_VERIFY_TRIMMED)
				{
					xi=yi=35;
					xf=yf=-1;
					for (j=0; j < 34; j++)
						for (i=0; i < 34; i++)
							if (SquareIconArray[i][j])
							{
								if (i > xf)
									xf=i;
								if (i < xi)
									xi=i;
								if (j < yi)
									yi=j;
								if (j > yf)
									yf=j;
							}
					if (xi == 35)
					{
						MessageBox(hwnd, "No visited icons to trim.", "Oops", MB_OK);
						break;
					}
				}

				for (j=yi; j <= yf; j++)
					for (i=xi; i <= xf; i++)
					{
						if (SquareIconArray[i][j] == 0)
						{
							if ((i != myEmpty) && (j != myEmpty))
							{
								count++;
								if (firstX == -1)
								{
									firstX = i;
									firstY = j;
								}
								lastX = i;
								lastY = j;
							}
						}
						else
							accounted++;
					}

				if (count)
				{
					char mybuf[200];
					float myPct = (float)count /((1+xf-xi)*(1+yf-yi));
					myPct = 100 * (1 - myPct);
					sprintf(mybuf, "%d squares visited\n%d squares unvisited\nFirst %02x,%02x\nLast %02x,%02x\n%.02f pct done",
						accounted, count, firstX, firstY, lastX, lastY, myPct);
					MessageBox(hwnd, mybuf, "Still some to go", MB_OK);
				} else MessageBox(hwnd, "Hooray!", "No blank squares left.", MB_OK);

			}

			break;

		case ID_FLIP_12:
			flipStuff(hwnd,0,0,17,0);
			break;

		case ID_FLIP_13:
			flipStuff(hwnd,0,0,0,17);
			break;

		case ID_FLIP_14:
			flipStuff(hwnd,0,0,17,17);
			break;

		case ID_FLIP_23:
			flipStuff(hwnd,17,0,0,17);
			break;

		case ID_FLIP_24:
			flipStuff(hwnd,17,0,17,17);
			break;

		case ID_FLIP_34:
			flipStuff(hwnd,0,17,17,17);
			break;

		case ID_HELP_ABOUT:
			MessageBox(hwnd, "This program was conceived as a way to print out maps for games I couldn\'t"
				" binary edit easily.\n"
				"It prints out walls and items and may offer text commentaries one day.\n\n"
				"Copyright 2008-2017 by Andrew Schultz, but if any code helps you, steal it!",
					"About", MB_OK);
			break;

		case ID_HELP_DOCS:
			MessageBox(hwnd,
				"IJKM places walls, ctrl- means 1-way, shift deletes, ctrl-shift=other way\n"
				"WASZ places fancier walls\n\nctrl-shift changes them, shift-(0-9) makes new default wall\n"
				"G=get current icon, H=jump to that icon in hex\n"
				"F2=save map file, (ctrl)-F3 = save (trimmed) BMP\n"
				, "Docs", MB_OK);
			break;

		case ID_HELP_THANKS:
			MessageBox(hwnd, "CodeGuru.com\n"
				"Andre LaMothe for Windows for Dummies\n"
				"Probably some of his code and comments still creeped in here\n"
				"Microsoft\'s online help\n"
				"http://www.gametutorials.com\n"
				"http://www.stackoverflow.com\n"
				"http://github.com for helping me organize things\n",
					"Thanks", MB_OK);
			break;
		}

	case WM_CREATE:
        {
				ReloadTheMap(hwnd);
				drawMyIcons(hwnd);
		return(0);
		} break;

	case WM_PAINT:
		{
		// simply validate the window
				ReloadTheMap(hwnd);
				drawMyIcons(hwnd);
		hdc = BeginPaint(hwnd,&ps);
		EndPaint(hwnd,&ps);
		return(0);
   		} break;

	case WM_KEYUP:
		switch(wparam)
		{
		case VK_CONTROL:
			if (0)
			{
				char buffer[40];

				sprintf(buffer, "Ctrl key up %d %d      ", KEY_DOWN(VK_CONTROL), KEY_UP(VK_CONTROL));
				// print out the key on the screen
				regularTextOut(buffer, hwnd);
				break;
			}
		}
		break;

	case WM_RBUTTONDOWN:
		if (0)
		{
			HDC hdc=GetDC(hwnd);

			TransparentBlt(hdc, LOWORD(lparam), HIWORD(lparam), 80, 32, icondc, 0, 0, 80, 32, RGB(255,0,0));
			ReleaseDC(hwnd, hdc);
		}
		break;

	case WM_LBUTTONDOWN:
		MouseDownX = LOWORD(lparam)/16;
		MouseDownY = HIWORD(lparam)/16;
		break;

	case WM_LBUTTONUP:
		{
			char buffer[50];
			long MouseDownX2 = LOWORD(lparam)/16;
			long MouseDownY2 = HIWORD(lparam)/16;



		//No drag and drop at this time.
		if ((MouseDownX != MouseDownX2) || (MouseDownY != MouseDownY2))
		{
			bufferL = MouseDownX;
			bufferR = MouseDownX2;
			if (MouseDownX > MouseDownX2)
			{
				bufferL = MouseDownX2;
				bufferR = MouseDownX;
			}

			bufferU = MouseDownY;
			bufferD = MouseDownY2;
			if (MouseDownY > MouseDownY2)
			{
				bufferU = MouseDownY2;
				bufferD = MouseDownY;
			}
			bufferL--;
			bufferR--;
			bufferU--;
			bufferD--;

			if ((bufferL < 0) || (bufferU < 0))
				break;

			sprintf(buffer, "Cut/paste data:\r\n%02x,%02x to %02x,%02x.", bufferL, bufferU, bufferR+1, bufferD+1);
			hdc = GetDC(hwnd);

			// set the colors
			SetTextColor(hdc, RGB(0,255,0));
			SetBkColor(hdc,RGB(0,0,255));
			SetBkMode(hdc,OPAQUE);

			// output the message
			{
				RECT rect;
				rect.top=480;
				rect.bottom=560;
				rect.left=800;
				rect.right=896;

				DrawText(hdc,buffer,strlen(buffer),&rect,DT_TOP|DT_LEFT);
			}

			// release dc
			ReleaseDC(hwnd,hdc);

			break;
		}
		//bufferL=bufferR=bufferU=bufferD=-1;

//Mouse select new icons
		if ((MouseDownX >= WALLICONSTART) && (MouseDownX < WALLICONSTART + 16))
			if ((MouseDownY >= 10) && (MouseDownY <= 25))
			{
				DrawPointerRectangle(hwnd, WALLICONSTART - 1, 10 + (iconNumber >> 4), BLACK);
				DrawPointerRectangle(hwnd, WALLICONSTART + (iconNumber & 0xf), 9, BLACK);
				iconNumber = (MouseDownX - WALLICONSTART) + ((MouseDownY - 10) << 4);
				DrawPointerRectangle(hwnd, WALLICONSTART - 1, MouseDownY, RGB(255,0,0));
				DrawPointerRectangle(hwnd, MouseDownX, 9, RGB(255,0,0));
				break;
			}

		if ((MouseDownX <= 35) && (MouseDownY <= 35))
			if ((MouseDownX >= 1) && (MouseDownY >= 1))
			{
				RECT rect;
				long yc2;

				char buffer[100];
			//rubbish below
			//sprintf(buffer, "X=%d Y=%d icon=%d", MouseDownX, MouseDownY, iconNumber);
			//MessageBox(hwnd, buffer, buffer, MB_OK);
				DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(0,0,0));
				DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(0,0,0));
				xCurrent = MouseDownX-1;
				yCurrent = MouseDownY-1;
				if (KEY_DOWN(VK_CONTROL))
					SquareIconArray[MouseDownX-1][MouseDownY-1] = (short)iconNumber;
				ReloadTheMap(hwnd);
				DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,0,0));
				DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,0,0));

				yc2 = yCurrent;

				if (zeroBottom) yc2 = 31 - yc2;

				sprintf(buffer, "%02d,%02d decimal\r\n%02x,%02x hex", xCurrent, yc2, xCurrent, yc2);
				hdc = GetDC(hwnd);

				// set the colors
				SetTextColor(hdc, RGB(0,255,0));
				SetBkColor(hdc,RGB(0,0,0));
				SetBkMode(hdc,OPAQUE);

				// output the message
				rect.top=432;
				rect.bottom=520;
				rect.left=576;
				rect.right=800;

				DrawText(hdc,buffer,strlen(buffer),&rect,DT_TOP|DT_LEFT);

				// release dc
				ReleaseDC(hwnd,hdc);

				break;
			}

			//Mouse select new walls
		if ((MouseDownX >= WALLICONSTART) && (MouseDownX < WALLICONSTART + 10))
			if ((MouseDownY >= 2) && (MouseDownY <= 3))
			{
				DrawPointerRectangle(hwnd, WallIconNumber + WALLICONSTART, 1, BLACK);
				WallIconNumber = MouseDownX - WALLICONSTART;
				DrawPointerRectangle(hwnd, WallIconNumber + WALLICONSTART, 1, RED);
				break;
			}

				sprintf(buffer, "Unknown click area %d %d",MouseDownX, MouseDownY);
				hdc = GetDC(hwnd);

				// set the colors
				SetTextColor(hdc, RGB(0,255,0));
				SetBkColor(hdc,RGB(0,0,0));
				SetBkMode(hdc,OPAQUE);

				// output the message
				TextOut(hdc,608,480,buffer,strlen(buffer));

				// release dc
				ReleaseDC(hwnd,hdc);

		break;
		}


    case WM_KEYDOWN: // this message contains the virtual code
			switch(wparam)
			{
				break;

			case 0xc0: //backquote
				{
					char buffer[50];
					sprintf(buffer, "Dec X=%02d Y=%02d\nHex X=%02x Y=%02x", xCurrent, yCurrent, xCurrent, yCurrent);
					MessageBox(hwnd, buffer, "Where am I?", MB_OK);
				}
				break;

			case VK_DELETE:
				workNotSaved = 1;
				if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_SHIFT))
					break;
				if (KEY_DOWN(VK_CONTROL))
				{
					TranspIconArray[xCurrent][yCurrent] = 0;
				}
				else if (KEY_DOWN(VK_SHIFT))
				{
					UDWallArray[xCurrent][yCurrent] =
					LRWallArray[xCurrent][yCurrent] =
					UDWallArray[xCurrent][(yCurrent+1) % MAXICONSHIGH] =
					LRWallArray[(xCurrent+1) % MAXICONSWIDE][yCurrent] = 0;
				}
				else
					SquareIconArray[xCurrent][yCurrent] = 0;
				ReloadTheMap(hwnd);
				break;


			case VK_LEFT:
			case VK_RIGHT:
				if (KEY_DOWN(VK_SHIFT) && KEY_DOWN(VK_CONTROL))
				{
					if (wparam == VK_RIGHT)
						switchIconPointer(hwnd, iconNumber | 0xf);
					else
						switchIconPointer(hwnd, iconNumber & 0xf0);
					break;
				}
				if (KEY_DOWN(VK_SHIFT))
				{
					DrawPointerRectangle(hwnd, MAXICONSWIDE + 3 + WallIconNumber, 1, RGB(0,0,0));
					switch(wparam)
					{
					case VK_LEFT:
						WallIconNumber+= MAXWALLICON - 1;
						break;
					case VK_RIGHT:
						WallIconNumber+= 1;
						break;
					}
					WallIconNumber %= MAXWALLICON;
					DrawPointerRectangle(hwnd, MAXICONSWIDE + 3 + WallIconNumber, 1, RGB(255,0,0));
					break;
				}
			case VK_UP:
			case VK_DOWN:
				if (KEY_DOWN(VK_SHIFT) && KEY_DOWN(VK_CONTROL))
				{
					if (wparam == VK_DOWN)
						switchIconPointer(hwnd, iconNumber | 0xf0);
					else
						switchIconPointer(hwnd, iconNumber & 0x0f);
					break;
				}
				// L-U-R-D.
				{
					char buffer[100];
					RECT rect;
					long yc2;


					if (KEY_DOWN(VK_CONTROL))
					{
						DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(0,0,0));
						DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(0,0,0));

						switch(wparam)
						{
						case VK_LEFT:
							iconNumber += 255;
							break;
						case VK_RIGHT:
							iconNumber += 1;
							break;
						case VK_UP:
							iconNumber += 240;
							break;
						case VK_DOWN:
							iconNumber += 16;
							break;
						}

						iconNumber %= 256;

						DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
						DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(255,0,0));

						sprintf(buffer, "Icon %02x ",iconNumber);
						regularTextOut(buffer, hwnd);
					}
					else
					{
						//rectangle down the side & along top
						DrawPointers(hwnd, RGB(255,255,255));

						switch(wparam)
						{
						case VK_LEFT:
							xCurrent--;
							if (xCurrent & 0xf00)
								xCurrent += MAXICONSWIDE - 1;
							break;

						case VK_RIGHT:
							xCurrent++;
							if (xCurrent == MAXICONSWIDE - 1)
								xCurrent = 0;
							break;

						case VK_UP:
							yCurrent--;
							if (yCurrent & 0xf00)
								yCurrent += MAXICONSHIGH - 1;
							break;

						case VK_DOWN:
							yCurrent++;
							if (yCurrent == MAXICONSHIGH - 1)
								yCurrent = 0;
							break;

						}

						DrawPointers(hwnd, RGB(255,0,0));

						yc2 = yCurrent;
						if (zeroBottom) yc2 = 31 - yc2;
						sprintf(buffer, "%02d,%02d D\r\n%02x,%02x H", xCurrent, yc2, xCurrent, yc2);
						hdc = GetDC(hwnd);

						// set the colors
						SetTextColor(hdc, RGB(0,255,0));
						SetBkColor(hdc,RGB(0,0,0));
						SetBkMode(hdc,OPAQUE);

						// output the message
						rect.top=32;
						rect.bottom=72;
						rect.left=800;
						rect.right=900;

						DrawText(hdc,buffer,strlen(buffer),&rect,DT_TOP|DT_LEFT);

						// release dc
						ReleaseDC(hwnd,hdc);

					}

				}
				break;

				/*
					TransparentBlt(hdc, xCurrent*16+8, yCurrent*16+16, 16, 16, walldc,
						16*(LEFTWALL%16), 16*(LEFTWALL/16), 16, 16, RGB(255,0,0));
					TransparentBlt(hdc, xCurrent*16+24, yCurrent*16+16, 16, 16, walldc,
						16*(RIGHTWALL%16), 16*(RIGHTWALL/16), 16, 16, RGB(255,0,0));
					TransparentBlt(hdc, xCurrent*16+16, yCurrent*16+8, 16, 16, walldc,
						16*(UPWALL%16), 16*(UPWALL/16), 16, 16, RGB(255,0,0));
					TransparentBlt(hdc, xCurrent*16+16, yCurrent*16+24, 16, 16, walldc,
						16*(DOWNWALL%16), 16*(DOWNWALL/16), 16, 16, RGB(255,0,0));
				*/

			case VK_W:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_SHIFT))
						UDWallArray[xCurrent][yCurrent] = (UDWallArray[xCurrent][yCurrent] + 1) % 8;
					else if (KEY_DOWN(VK_CONTROL))
						UDWallArray[xCurrent][yCurrent] = 0;
					else
						UDWallArray[xCurrent][yCurrent] = (short)WallIconNumber;

					regularTextOut(buffer, hwnd);
					ReloadTheMap(hwnd);
					workNotSaved = 1;

					break;
				}

			case VK_A:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_SHIFT))
						LRWallArray[xCurrent][yCurrent] = (LRWallArray[xCurrent][yCurrent] + 1) % 8;
					else if (KEY_DOWN(VK_CONTROL))
						LRWallArray[xCurrent][yCurrent] = 0;
					else
						LRWallArray[xCurrent][yCurrent] = (short)WallIconNumber;

					regularTextOut(buffer, hwnd);
					ReloadTheMap(hwnd);
					workNotSaved = 1;

					break;
				}
				break;

			case VK_S:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_SHIFT))
						LRWallArray[xCurrent+1][yCurrent] = (LRWallArray[xCurrent+1][yCurrent] + 1) % 8;
					else if (KEY_DOWN(VK_CONTROL))
						LRWallArray[xCurrent+1][yCurrent] = 0;
					else
						LRWallArray[xCurrent+1][yCurrent] = (short)WallIconNumber;

					regularTextOut(buffer, hwnd);
					ReloadTheMap(hwnd);
					workNotSaved = 1;

					break;
				}
				break;

			case VK_Z:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL) && KEY_DOWN(VK_SHIFT) != 0	)
						UDWallArray[xCurrent][yCurrent+1] = (UDWallArray[xCurrent][yCurrent+1] + 1) % 8;
					else if (KEY_DOWN(VK_CONTROL))
						UDWallArray[xCurrent][yCurrent+1] = 0;
					else
						UDWallArray[xCurrent][yCurrent+1] = (short)WallIconNumber;

					regularTextOut(buffer, hwnd);
					ReloadTheMap(hwnd);
					workNotSaved = 1;

					break;
				}

			case VK_I:
				workNotSaved = 1;
				if (KEY_DOWN(VK_CONTROL))
					if (KEY_DOWN(VK_SHIFT))
						UDWallArray[xCurrent][yCurrent] = 4;
					else
						UDWallArray[xCurrent][yCurrent] = 5;
				else if (KEY_DOWN(VK_SHIFT))
					UDWallArray[xCurrent][yCurrent] = 0;
				else
					UDWallArray[xCurrent][yCurrent] = 1;
				ReloadTheMap(hwnd);
				break;

			case VK_J:
				workNotSaved = 1;
				if (KEY_DOWN(VK_CONTROL))
					if (KEY_DOWN(VK_SHIFT))
						LRWallArray[xCurrent][yCurrent] = 4;
					else
						LRWallArray[xCurrent][yCurrent] = 5;
				else if (KEY_DOWN(VK_SHIFT))
					LRWallArray[xCurrent][yCurrent] = 0;
				else
					LRWallArray[xCurrent][yCurrent] = 1;
				ReloadTheMap(hwnd);
				break;

			case VK_K:
				workNotSaved = 1;
				if (KEY_DOWN(VK_CONTROL))
					if (KEY_DOWN(VK_SHIFT))
						LRWallArray[xCurrent+1][yCurrent] = 4;
					else
						LRWallArray[xCurrent+1][yCurrent] = 5;
				else if (KEY_DOWN(VK_SHIFT))
					LRWallArray[xCurrent+1][yCurrent] = 0;
				else
					LRWallArray[xCurrent+1][yCurrent] = 1;
				ReloadTheMap(hwnd);
				break;

			case VK_M:
				workNotSaved = 1;
				if (KEY_DOWN(VK_CONTROL))
					if (KEY_DOWN(VK_SHIFT))
						UDWallArray[xCurrent][yCurrent+1] = 4;
					else
						UDWallArray[xCurrent][yCurrent+1] = 5;
				else if (KEY_DOWN(VK_SHIFT))
					UDWallArray[xCurrent][yCurrent+1] = 0;
				else
					UDWallArray[xCurrent][yCurrent+1] = 1;
				ReloadTheMap(hwnd);
				break;

			case VK_G:
				if ((SquareIconArray[xCurrent][yCurrent]) && (SquareIconArray[xCurrent][yCurrent] != BEENTHERE))
					switchIconPointer(hwnd, SquareIconArray[xCurrent][yCurrent]);
				break;

			case 0xbc: // comma
				if (iconNumber == BEENTHERE)
				{
					switchIconPointer(hwnd, (short)notPeriod);
				}
				else
				{
					notPeriod = iconNumber;
				}
				break;

			case 0xbe:	//period
				if (SquareIconArray[xCurrent][yCurrent] == 0)
				{
					SquareIconArray[xCurrent][yCurrent] = BEENTHERE;
					ReloadTheMap(hwnd);
					break;
				}
				else
				{
					char buffer[100];
					sprintf(buffer, "Already hit %02d,%02d", xCurrent, yCurrent);
					regularTextOut(buffer, hwnd);
				}
				break;

			case 0xba:	//semicolon
				if (KEY_DOWN(VK_SHIFT))
					UDWallArray[xCurrent][yCurrent+1] = 5;
				else
					UDWallArray[xCurrent][yCurrent+1] = 4;
				ReloadTheMap(hwnd);
				break;

			case VK_BACK:
				if (KEY_DOWN(VK_CONTROL))
				{
					//Clear all walls
					//Redraw center icon

				}
				//need to redraw walls! Aigh. Better yet, merge with VK_SPACE.
				//BitBlt(hdc, xCurrent*16+16, yCurrent*16+16, 16, 16, icondc, 0, 0, SRCCOPY);
				break;

			case VK_SPACE:
				{
					char buffer[100];

					sprintf(buffer, "Icon %d at %d %d        ", iconNumber, xCurrent, yCurrent);
					workNotSaved = 1;
 					if (KEY_DOWN(VK_CONTROL))
						TranspIconArray[xCurrent][yCurrent] = (short)iconNumber;
					else
						SquareIconArray[xCurrent][yCurrent] = (short)iconNumber;

					//Write over original icon, draw out walls
					//regularTextOut(buffer, hwnd);

					ReloadTheMap(hwnd);

					break;
				}

			case VK_0:
			case VK_1:
			case VK_2:
			case VK_3:
			case VK_4:
			case VK_5:
			case VK_6:
			case VK_7:
			case VK_8:
			case VK_9:
				if (KEY_DOWN(VK_SHIFT))
				{
					DrawPointerRectangle(hwnd, MAXICONSWIDE + 3 + WallIconNumber, 1, RGB(0,0,0));
					DrawPointerRectangle(hwnd, MAXICONSWIDE + 3 + wparam-VK_0, 1, RGB(255,0,0));
					WallIconNumber = wparam-VK_0;
					break;
				}
				//no ctrl, alt or shift
				SquareIconArray[xCurrent][yCurrent] = (short)prevDefArray[wparam-VK_0];
				workNotSaved = 1;
				ReloadTheMap(hwnd);
				break;
			default:
			{
				long q = 1;
			}
		break;

			}

			return(0);
			break;

    case WM_MOUSEMOVE: // whenever the mouse moves this is sent
         {
        // extract x,y and buttons
        int mouse_x = (int)LOWORD(lparam);
        int mouse_y = (int)HIWORD(lparam);
        int buttons = (int)wparam;

        return(0);
        } break;

	case WM_CLOSE:
		if (workNotSaved)
		{
			long x = MessageBox(NULL, "Do you wish to exit without saving? If so, hit OK. If not, hit Cancel.", "Save Warning", MB_OKCANCEL);
			if (x != 1)
				return(0);
		}
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		{
		// kill the application
		PostQuitMessage(0);
		return(0);
		} break;

	default: break;

    } // end switch

// process any messages that we didn't take care of
return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

// WINMAIN ////////////////////////////////////////////////
int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{

WNDCLASS winclass;	// this will hold the class we create
//HWND	 hwnd;		// generic window handle
MSG		 msg;		// generic message

HACCEL hAccelTable;

HBITMAP iconbmp;
HBITMAP wallbmp;

HBITMAP oldicon;
HBITMAP oldwall;

HDC localhdc;

// first fill in the window class stucture
winclass.style			= CS_DBLCLKS | CS_OWNDC |
                          CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hinstance;
winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
winclass.hCursor		= LoadCursor(NULL, IDC_ARROW);
winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= TEXT("MYMENU");
winclass.lpszClassName	= WINDOW_CLASS_NAME;

// register the window class
if (!RegisterClass(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, // class
						  "GUI Map Drawer",	     // title
						  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					 	  0,0,	   // x,y
						  16*(MAXICONSWIDE+24),16*(MAXICONSHIGH+4), // width, height
						  NULL,	   // handle to parent
						  NULL,	   // handle to menu
						  hinstance,// instance
						  NULL)))	// creation parms
	return(0);

	// save the window handle in a global
	hwnd = hwnd;

	changeBarText(hwnd);

    hAccelTable = LoadAccelerators(hinstance, "MYACCEL");
// enter main event loop

	parseCmdLine();

	//basic layout of the GUI here
	iconbmp = (HBITMAP)LoadImage(hinstance, iconFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	wallbmp = (HBITMAP)LoadImage(hinstance, wallFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	localhdc = GetDC(hwnd);

	icondc = CreateCompatibleDC(localhdc);
	walldc = CreateCompatibleDC(localhdc);

	oldicon = (HBITMAP)SelectObject(icondc, iconbmp);
	oldwall = (HBITMAP)SelectObject(walldc, wallbmp);

	drawMyIcons(hwnd);

	SetBkColor(localhdc,RGB(255,255,255));
	Rectangle(localhdc, 0,0,576,576);

	setInitialChecks(hwnd);

	if (startFileName[0])
	{
		ReadBinaryMap(hwnd, startFileName);
	}

while(1)
	{
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
		// test if this is a quit
        if (msg.message == WM_QUIT)
           break;

        if (!TranslateAccelerator(hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		} // end if

    // main game processing goes here

	} // end while

	SelectObject(icondc,oldicon);
	SelectObject(walldc,oldwall);
	DeleteObject(wallbmp);
	DeleteObject(iconbmp);
	ReleaseDC(hwnd, localhdc);
	DeleteDC(icondc);
	DeleteDC(walldc);

// return to Windows like this
return(msg.wParam);

} // end WinMain

VOID DoOpenFile(HWND hwnd)
{

OPENFILENAME  locOFN;
char        locFilterSpec[128] = TEXT("MAP and TXT files\0*.map\0");
char        locFileName[MAXFILENAME];
char        locFileTitle[MAXFILENAME];

  locFileName[0] = 0;
  locFileTitle[0] = 0;


  locOFN.lStructSize = sizeof(OPENFILENAME);
  locOFN.hwndOwner = hwnd;
  locOFN.hInstance = NULL;
  locOFN.lpstrFilter = locFilterSpec;
  locOFN.lpstrCustomFilter = NULL;
  locOFN.nMaxCustFilter = 0L;
  locOFN.nFilterIndex = 0L;
  locOFN.lpstrFile = locFileName;
  locOFN.nMaxFile = MAXFILENAME;
  locOFN.lpstrFileTitle = locFileTitle;
  locOFN.nMaxFileTitle = MAXFILENAME;
  locOFN.lpstrInitialDir = NULL;
  locOFN.lpstrTitle = TEXT("Open...");

  locOFN.Flags = OFN_FILEMUSTEXIST;

  locOFN.lpstrDefExt = NULL;
  locOFN.lCustData = 0;
  locOFN.lpfnHook = NULL;
  locOFN.lpTemplateName = NULL;

if (workNotSaved)
{
	long x = MessageBox(NULL, "Do you wish to open a new file without saving? If so, hit OK. If not, hit Cancel.", "Save Warning", MB_OKCANCEL);
	if (x != 1)
		return;
}

  if (GetOpenFileName(&locOFN) == TRUE)
  {
	  long i;
	  for (i=0; locFileName[i]; i++);
	  i--;
	  if (locFileName[i] == 'p')
	  {
		  ReadBinaryMap(hwnd, locFileName);
		  strcpy(CurrentFileName, locFileName);
		  changeBarText(hwnd);
		  workNotSaved = 0;
	  }
  }
}

VOID DoCreateFile(HWND hwnd)
{

OPENFILENAME  locOFN;
char        locFilterSpec[128] = TEXT("MAP and TXT files\0*.map\0");
char        locFileName[MAXFILENAME];
char        locFileTitle[MAXFILENAME];

  locFileName[0] = 0;
  locFileTitle[0] = 0;


  locOFN.lStructSize = sizeof(OPENFILENAME);
  locOFN.hwndOwner = hwnd;
  locOFN.hInstance = NULL;
  locOFN.lpstrFilter = locFilterSpec;
  locOFN.lpstrCustomFilter = NULL;
  locOFN.nMaxCustFilter = 0L;
  locOFN.nFilterIndex = 0L;
  locOFN.lpstrFile = locFileName;
  locOFN.nMaxFile = MAXFILENAME;
  locOFN.lpstrFileTitle = locFileTitle;
  locOFN.nMaxFileTitle = MAXFILENAME;
  locOFN.lpstrInitialDir = NULL;
  locOFN.lpstrTitle = TEXT("Open...");

  locOFN.Flags = 0;

  locOFN.lpstrDefExt = NULL;
  locOFN.lCustData = 0;
  locOFN.lpfnHook = NULL;
  locOFN.lpTemplateName = NULL;

if (workNotSaved)
{
	long x = MessageBox(NULL, "Do you wish to open a new file without saving? If so, hit OK. If not, hit Cancel.", "Save Warning", MB_OKCANCEL);
	if (x != 1)
		return;
}

  if (GetOpenFileName(&locOFN) == TRUE)
  {
	  long acc;
	  long q = strlen(locFileName);
	  if ((strlen(locFileName) < 4) ||
		  !((locFileName[q-1] == 'p') && (locFileName[q-2] == 'a') && (locFileName[q-3] == 'm') && (locFileName[q-4] == '.')))
		  strcat(locFileName, ".map");

	  acc = _access(locFileName, 0);
	  if (acc == 0)
	  {
		  MessageBox(NULL, "The file already exists. Use ctrl-o to open.", "File exists", MB_OK);
		  return;
	  }

	  strcpy(CurrentFileName, locFileName);
	  changeBarText(hwnd);
	  workNotSaved = 0;
  }
}

///////////////////////////////////////////////////////////

void ReadBinaryMap(HWND hwnd, char x[MAXFILENAME])
{
	long j, i;
	short haveTransp;

	FILE * F = fopen(x, "rb");
	if (F == NULL)
	{
		MessageBox(hwnd, "Uh oh", x, MB_OK);
		return;
	}

	//initialize
	for (j=0; j < MAXICONSHIGH; j++)
		for (i=0; i < MAXICONSWIDE; i++)
			SquareIconArray[i][j] = 0;

	for (j=0; j < MAXICONSHIGH; j++)
		for (i=0; i < MAXICONSWIDE; i++)
			TranspIconArray[i][j] = 0;

	for (j=0; j < MAXICONSHIGH+1; j++)
		for (i=0; i < MAXICONSWIDE+1; i++)
			UDWallArray[i][j] = LRWallArray[i][j] = 0;

	myW = fgetc(F);
	myH = fgetc(F);

	if (myW > 35 || myH > 35)
	{
		char x[200];

		sprintf(x, "%d(%02x) x %d(%02x) dimensions are outside 35x35 bounds, rounding down.", myW, myW, myH, myH);
		MessageBox(hwnd, x, "dimension problems", MB_OK);
		if (myW > 35)
			myW = 35;
		if (myW > 35)
			myW = 35;
	}

	for (j=0; j < myH; j++)
	{
		fgetc(F);
		for (i=0; i < myW; i++)
		{
			UDWallArray[i][j] = fgetc(F);
			fgetc(F);
		}
		for (i=0; i < myW; i++)
		{
			LRWallArray[i][j] = fgetc(F);
			SquareIconArray[i][j] = fgetc(F);
		}
		LRWallArray[i][j] = fgetc(F);
	}
	//1 extra time, for the bottom, at least for now
	fgetc(F);
	for (i=0; i < myW; i++)
	{
		UDWallArray[i][j] = fgetc(F);
		fgetc(F);
	}
	fclose(F);

	haveTransp = fgetc(F);
	if ((haveTransp == EOF) || (!haveTransp))
		return;

	for (j=0; j < myH; j++)
		for (i=0; i < myW; i++)
			TranspIconArray[i][j] = fgetc(F);

	myW = 35;
	myH = 35;

	ReloadTheMap(hwnd);

}

void CreateNewMapfile(long x, long y)
{
	FILE * F;
	long i, j;

	F = fopen("new.map", "wb");

	fputc(x, F);
	fputc(y, F);

	myW = x;
	myH = y;

	for (j=0; j < 2*y+1; j++)
		for (i=0; i < 2*x+1; i++)
			fputc(0, F);

	fclose(F);
}

void SaveMapfile()
{
	FILE * F;
	long i, j;
	long gotTrans = 0;
	long q;
	char MsgBoxMsg[200] = "Blank file, save to blank.map?\n\nCurrent directory is ";
	char buffer2[200];

	GetCurrentDirectory(200, buffer2);
	strcat(MsgBoxMsg, buffer2);

	if (CurrentFileName[0] == 0)
	{
		q = MessageBox(hwnd, MsgBoxMsg, "Blank file", MB_OKCANCEL);
		if (q == 1)
		{
			long acc;
			acc = _access("blank.map", 0);
			if (acc == 0)
			{
				q = MessageBox(hwnd, "Blank.map exists, overwrite? (you can move it and resave)", "Blank.map nag", MB_OKCANCEL);
			}
			if (q == 2)
				return;
			strcpy(CurrentFileName, "blank.map");
			changeBarText(hwnd);
		}
		else
			return;
	}

	F = fopen(CurrentFileName, "wb");

	fputc(myW, F);
	fputc(myH, F);

	for (j=0; j < myH; j++)
	{
		for (i=0; i < myW; i++)
		{
			fputc(0, F);
			fputc(UDWallArray[i][j], F);
		}
		fputc(0, F);

		for (i=0; i < myW; i++)
		{
			fputc(LRWallArray[i][j], F);
			fputc(SquareIconArray[i][j], F);
		}
		fputc(LRWallArray[i][j], F);
	}
	for (i=0; i < myW; i++) // final row
	{
		fputc(0, F);
		fputc(UDWallArray[i][j], F);
	}
	fputc(0, F);

	for (j=0; j < myH; j++)
		for (i=0; i < myW; i++)
			if (TranspIconArray[i][j])
				gotTrans = 1;

	if (gotTrans)
	{
		fputc(1, F);
		for (j=0; j < myH; j++)
			for (i=0; i < myW; i++)
				fputc(TranspIconArray[i][j], F);
	}

	fclose(F);
	workNotSaved = 0;
}

void DrawPointers(HWND hwnd, COLORREF myColor)
{
	if (showPointerRectangle)
	{
	DrawPointerRectangle(hwnd, xCurrent+1, 0, myColor);
	DrawPointerRectangle(hwnd, 0, yCurrent+1, myColor);
	DrawPointerRectangle(hwnd, xCurrent+1, 35, myColor);
	DrawPointerRectangle(hwnd, 35, yCurrent+1, myColor);
	}
}

void DrawPointerRectangle(HWND hwnd, long xOffset, long yOffset, COLORREF myColor)
{
	HDC hdc = GetDC(hwnd);
	HBRUSH hbrush = CreateSolidBrush(myColor);
	RECT rect;

	rect.top=yOffset*ICONHEIGHT + PIXELS_IN;
	rect.bottom=(yOffset+1)*ICONHEIGHT - PIXELS_IN;
	rect.left=xOffset*ICONWIDTH + PIXELS_IN;
	rect.right=(xOffset+1)*ICONWIDTH - PIXELS_IN;

	FillRect(hdc, &rect, hbrush);

	ReleaseDC(hwnd, hdc);

	DeleteObject(hbrush);

}
/* Bitmap saving

                    locDrawPage.hOutputDC = locCDC;
                    locDrawPage.hFormatDC = locDC;

                    SendMessage(hwnd,SCCVW_DRAWPAGE,0,(LPARAM)(PSCCVWDRAWPAGE41)&locDrawPage);

                    SelectObject(locCDC,locOldBitmapHnd);

                    locDIBHnd = DibFromBitmap (locBitmapHnd, BI_RGB, 4, locDrawPage.hPalette);
                    WriteDIB ("C:\\PAGE_4.BMP",locDIBHnd);

                    if (locDrawPage.hPalette)
                        DeleteObject(locDrawPage.hPalette);

                    GlobalFree(locDIBHnd);

*/

void ReloadTheMap(HWND hwnd)
{
	long i, j;
	HDC hdc = GetDC(hwnd);
	HBRUSH hbrush=CreateSolidBrush(RGB(255,255,255));
	RECT rect;

	rect.top=0;
	rect.bottom=ICONHEIGHT*(MAXICONSHIGH+1);
	rect.left=0;
	rect.right=ICONWIDTH*(MAXICONSWIDE+1);

	FillRect(hdc, &rect, hbrush);

	DeleteObject(hbrush);

	for (j=0; j <= MAXICONSHIGH-1; j++)
	{
		for (i=0; i <= MAXICONSWIDE-1; i++)
		{
			if (SquareIconArray[i][j])
				BitBlt(hdc, i*16+16, j*16+16, 16, 16, icondc, XVAL(i,j), YVAL(i,j), SRCCOPY);
			if (TranspIconArray[i][j])
				TransparentBlt(hdc, i*16+16, j*16+16, 16, 16, icondc,
					(TranspIconArray[i][j]%16)*16,
					(TranspIconArray[i][j]/16)*16,
					16, 16, RGB(255,255,255));
		}
	}

	if (showPointerRectangle)
	{
		DrawPointers(hwnd, RGB(255,0,0));
	}
	for (j=0; j < MAXICONSHIGH+1; j++)
		for (i=0; i < MAXICONSWIDE+1; i++)
		{
			TransparentBlt(hdc, 8+i*16, 16+j*16, 16, 16, walldc,
				LRWallArray[i][j]*16, 16, 16, 16, RGB(255,255,255));
			TransparentBlt(hdc, 24+i*16, 16+j*16, 16, 16, walldc,
				LRWallArray[i+1][j]*16, 16, 16, 16, RGB(255,255,255));
			TransparentBlt(hdc, 16+i*16, 8+j*16, 16, 16, walldc,
				UDWallArray[i][j]*16, 0, 16, 16, RGB(255,255,255));
			TransparentBlt(hdc, 16+i*16, 24+j*16, 16, 16, walldc,
				UDWallArray[i][j+1]*16, 0, 16, 16, RGB(255,255,255));
		}

	DrawPointerRectangle(hwnd, MAXICONSWIDE + 3 + WallIconNumber, 1, RGB(255,0,0));

	DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
	DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(255,0,0));

	ReleaseDC(hwnd, hdc);
	noteNewShiftBox(hwnd);
}

void showClipContents(HWND hwnd)
{
	long i, j;
	HDC hdc = GetDC(hwnd);
	HBRUSH hbrush=CreateSolidBrush(RGB(255,255,255));
	RECT rect;

	rect.top=0;
	rect.bottom=ICONHEIGHT*(MAXICONSHIGH+1);
	rect.left=0;
	rect.right=ICONWIDTH*(MAXICONSWIDE+1);

	FillRect(hdc, &rect, hbrush);

	DeleteObject(hbrush);

	for (j=0; j <= MAXICONSHIGH-1; j++)
	{
		for (i=0; i <= MAXICONSWIDE-1; i++)
			BitBlt(hdc, i*16+16, j*16+16, 16, 16, icondc,
				(CutSquareIconArray[i][j]%16)*16, (CutSquareIconArray[i][j]/16)*16, SRCCOPY);
	}

	if (showPointerRectangle)
	{
		DrawPointers(hwnd, RGB(255,0,0));
	}
	for (j=0; j < MAXICONSHIGH+1; j++)
		for (i=0; i < MAXICONSWIDE+1; i++)
		{
			TransparentBlt(hdc, 8+i*16, 16+j*16, 16, 16, walldc,
				CutLRWallArray[i][j]*16, 16, 16, 16, RGB(255,255,255));
			TransparentBlt(hdc, 24+i*16, 16+j*16, 16, 16, walldc,
				CutLRWallArray[i+1][j]*16, 16, 16, 16, RGB(255,255,255));
			TransparentBlt(hdc, 16+i*16, 8+j*16, 16, 16, walldc,
				CutUDWallArray[i][j]*16, 0, 16, 16, RGB(255,255,255));
			TransparentBlt(hdc, 16+i*16, 24+j*16, 16, 16, walldc,
				CutUDWallArray[i][j+1]*16, 0, 16, 16, RGB(255,255,255));
		}

	DrawPointerRectangle(hwnd, MAXICONSWIDE + 3 + WallIconNumber, 1, RGB(255,0,0));

	DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
	DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(255,0,0));

	ReleaseDC(hwnd, hdc);

	MessageBox(hwnd, "Push OK to see the map again.", "Push OK", MB_OK);
}

void flipStuff (HWND hwnd, int xi, int yi, int xf, int yf)
{
	int i, j;
	short temp;

	for (i=0; i < 16; i++)
		for (j=0; j < 16; j++)
		{
			temp = SquareIconArray[i+xi][j+yi];
			SquareIconArray[i+xi][j+yi] = SquareIconArray[i+xf][j+yf];
			SquareIconArray[i+xf][j+yf] = temp;
		}
	for (i=0; i < 17; i++)
		for (j=0; j < 17; j++)
		{
			temp = UDWallArray[i+xi][j+yi];
			UDWallArray[i+xi][j+yi] = UDWallArray[i+xf][j+yf];
			UDWallArray[i+xf][j+yf] = temp;
			temp = LRWallArray[i+xi][j+yi];
			LRWallArray[i+xi][j+yi] = LRWallArray[i+xf][j+yf];
			LRWallArray[i+xf][j+yf] = temp;
		}
	ReloadTheMap(hwnd);
}

void SaveBitmapFile(HWND hwnd, short trim)
{
	HDC hWinDC;
	HDC hDC;
	HBITMAP oldBM, tBM;
	RECT rPage;
	BITMAPINFO bmi;
	LPBYTE pBytes;
	BITMAPFILEHEADER bmfh;
	DWORD numBytes;
	HANDLE fh;
	DWORD bytesWritten;
	TCHAR string[40];

	TCHAR buf[80];

	long xmin = 0, ymin = 0, xmax = 34, ymax = 34;

	char buf2[200];

	strcpy(buf2, CurrentFileName);
	buf2[strlen(buf2)-3] = 'b';
	buf2[strlen(buf2)-2] = 'm';

	if (!IsWindow(hwnd))	/* should never happen, but just in case... */
		return;

	if (trim)
	{
		short i, j, t;
		char buffer[100];
		xmin = ymin = 35;
		xmax = ymax = -1;
		for (j=0; j < MAXICONSHIGH - 1; j++)
;			for (i=0; i < MAXICONSWIDE - 1; i++)
			{
				t = SquareIconArray[i][j] | TranspIconArray[i][j];
				if (t)
				{
					if (i < xmin)
						xmin = i;
					if (i > xmax)
						xmax = i;
					if (j < ymin)
						ymin = j;
					if (j > ymax)
						ymax = j;
				}
			}
;		for (j=0; j < MAXICONSHIGH - 1; j++)
			for (i=0; i < MAXICONSHIGH - 1; i++)
			{
				t = UDWallArray[i][j];
				if (t)
				{
					if (i < xmin)
						xmin = i;
					if (i > xmax)
						xmax = i;
					if (j < ymin)
						ymin = j;
					if (j > ymax)
						ymax = j - 1;
				}
			}
		for (j=0; j < MAXICONSHIGH - 1; j++)
			for (i=0; i < MAXICONSHIGH - 1; i++)
			{
				t = LRWallArray[i][j];
				if (t)
				{
					if (i < xmin)
						xmin = i;
					if (i > xmax)
						xmax = i - 1;
					if (j < ymin)
						ymin = j;
					if (j > ymax)
						ymax = j;
				}
			}
		sprintf(buffer, "New dimensions (%02d,%02d) to (%02d,%02d)", xmin, ymin, xmax, ymax);
		MessageBox(hwnd, buffer, "Save to", MB_OK);
	}

	ReloadTheMap(hwnd);

	hWinDC = GetDC(hwnd);
	hDC = CreateCompatibleDC(hWinDC);

	wsprintf(buf,TEXT(buf2),shotcount);
	shotcount++;

	GetClientRect(hwnd,&rPage);
	rPage.left = 8 + 16 * xmin;
	rPage.right = 8 + 16 * xmax;
	rPage.top = 8 + 16 * ymin;
	rPage.bottom = 8 + 16 * ymax;

	numBytes = 16 * 16 * (xmax - xmin) * (ymax - ymin) * 3;

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 16 * (xmax - xmin);
	bmi.bmiHeader.biHeight = 16 * (ymax - ymin);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = numBytes;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
//	*(DWORD *)&bmi.bmiColors[0] = 0;
	tBM = CreateDIBSection(hDC,&bmi,DIB_RGB_COLORS,(void **) &pBytes,NULL,0);
	if (!tBM)
	{
		wsprintf(string,TEXT("Error %d creating DIB section"),GetLastError());
		MessageBox(NULL,string,NULL,MB_OK);
		DeleteDC(hDC);
		ReleaseDC(hwnd,hWinDC);
		return;
	}

	oldBM = (HBITMAP) SelectObject(hDC,tBM);
	BitBlt(hDC,0,0,16 * (xmax - xmin),16 * (ymax - ymin),hWinDC,8,8,SRCCOPY);

	bmfh.bfType = 0x4d42;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + numBytes;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	fh = CreateFile(buf,
					GENERIC_READ | GENERIC_WRITE,
					0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if (fh == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL,TEXT("Error creating output file"),NULL,MB_OK);
	}
	else
	{
		WriteFile(fh,&bmfh,sizeof(BITMAPFILEHEADER),&bytesWritten,NULL);
		WriteFile(fh,&bmi,sizeof(BITMAPINFOHEADER),&bytesWritten,NULL);
		WriteFile(fh,pBytes,numBytes,&bytesWritten,NULL);
		CloseHandle(fh);
	}

	DeleteObject(SelectObject(hDC,oldBM));
	DeleteDC(hDC);
	ReleaseDC(hwnd,hWinDC);
}

void parseCmdLine()
{
	short count = 1;
	short gotLocFile = 0;

	while (count < __argc)
	{
		if (__argv[count][0] == '-')
		{
			short lc = __argv[count][1] | 0x20;
			switch(lc)
			{
			case 'i':
				if (__argv[count][2] == ':')
					strcpy(iconFileName, __argv[count+1] + 2);
				else
				{
					strcpy(iconFileName, __argv[count+1]);
					count++;
				}
				break;

			case 'w':
				if (__argv[count][2] == ':')
					strcpy(wallFileName, __argv[count+1] + 2);
				else
				{
					strcpy(wallFileName, __argv[count+1]);
					count++;
				}
				break;

			default:
				printf("WARNING: Unknown flag %s at argument %d.\n", __argv[count], count);
				break;
			}
		}
		else
		{
			if (gotLocFile)
			{
				printf("WARNING: argument %d tries to define a second file.", count);
			}
			else
			{
				gotLocFile = 1;
				strcpy(startFileName, __argv[count]);
			}
		}
		count++;
	}
}

void drawMyIcons(HWND hwnd)
{
	HDC      localhdc = GetDC(hwnd);
	char buffer[50] = "Wall Icons";
	BitBlt(localhdc,16*(3+MAXICONSWIDE),32,160,32,walldc,0,0,SRCCOPY);

	BitBlt(localhdc,16*(3+MAXICONSWIDE),160,256,256,icondc,0,0,SRCCOPY);

	SetTextColor(localhdc, RGB(0,255,0));
	SetBkColor(localhdc,RGB(0,0,0));
	SetBkMode(localhdc,OPAQUE);

				// output the message
	TextOut(localhdc,600,0,buffer,strlen(buffer));

	sprintf(buffer, "Ground Icons");

	TextOut(localhdc,600,128,buffer,strlen(buffer));
	ReleaseDC(hwnd,localhdc);

}

void changeBarText(HWND hwnd)
{
	char guiTitle[100] = "GUI Map Drawer - ";
	if (CurrentFileName[0] == 0)
		strcat(guiTitle, "new file");
	else
		strcat(guiTitle, PathFindFileName(CurrentFileName));
	SetWindowText(hwnd, guiTitle);
}

void regularTextOut(char x[100], HWND hwnd)
{
	HDC hdc;
	if (!showDebug)
		return;

	hdc = GetDC(hwnd);

	// set the colors
	SetTextColor(hdc, RGB(0,255,0));
	SetBkColor(hdc,RGB(0,0,0));
	SetBkMode(hdc,OPAQUE);

	// output the message
	TextOut(hdc,600,440,x,strlen(x));

	// release dc
	ReleaseDC(hwnd,hdc);

	ReloadTheMap(hwnd);
}

BOOL CALLBACK HexDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_INITDIALOG:
			return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    GetDlgItemText(hwndDlg, ID_TEXTBOX_ITEM_TO_JUMP, textToShift, 80);
                    EndDialog(hwndDlg, wParam);
					return TRUE;

                    // Fall through.

                case IDCANCEL:
					textToShift[0] = 0;
                    EndDialog(hwndDlg, wParam);
                    return TRUE;
            }
    }
    return FALSE;
}

BOOL CALLBACK ShiftDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_INITDIALOG:
			{
				char temp[5];
				SetDlgItemText(hwndDlg, ID_TEXTBOX_LEFT_SHIFT, itoa(shiftLeft, temp, 10));
				SetDlgItemText(hwndDlg, ID_TEXTBOX_RIGHT_SHIFT, itoa(shiftRight, temp, 10));
				SetDlgItemText(hwndDlg, ID_TEXTBOX_UP_SHIFT, itoa(shiftUp, temp, 10));
				SetDlgItemText(hwndDlg, ID_TEXTBOX_DOWN_SHIFT, itoa(shiftDown, temp, 10));
			}
			return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
					{
						char temp[80];
						short templ, tempr, tempu, tempd;
						GetDlgItemText(hwndDlg, ID_TEXTBOX_LEFT_SHIFT, temp, 80);
						templ = (short)strtol(temp, NULL, 10);
						GetDlgItemText(hwndDlg, ID_TEXTBOX_RIGHT_SHIFT, temp, 80);
						tempr = (short)strtol(temp, NULL, 10);
						GetDlgItemText(hwndDlg, ID_TEXTBOX_UP_SHIFT, temp, 80);
						tempu = (short)strtol(temp, NULL, 10);
						GetDlgItemText(hwndDlg, ID_TEXTBOX_DOWN_SHIFT, temp, 80);
						tempd = (short)strtol(temp, NULL, 10);

						if (tempd <= tempu)
						{
							MessageBox(hwndDlg, "down must be > up.", "Error", MB_OK);
							return TRUE;
						}
						if (tempr <= templ)
						{
							MessageBox(hwndDlg, "left must be > right.", "Error", MB_OK);
							return TRUE;
						}
						if ((templ < 0) || (tempu < 0))
						{
							MessageBox(hwndDlg, "No negative values.", "Error", MB_OK);
							return TRUE;
						}
						if ((tempd > 34) || (tempr > 34))
						{
							MessageBox(hwndDlg, "Down/right boundaries must be below 35.", "Error", MB_OK);
							return TRUE;
						}

						shiftLeft = templ;
						shiftRight = tempr;
						shiftUp = tempu;
						shiftDown = tempd;

						EndDialog(hwndDlg, wParam);
					}
					return TRUE;

                    // Fall through.

                case IDCANCEL:
					textToShift[0] = 0;
                    EndDialog(hwndDlg, wParam);
                    return TRUE;
            }
    }
    return FALSE;
}

void setInitialChecks(HWND hwnd)
{
	if (showDebug)
		CheckMenuItem( GetMenu(hwnd), ID_DEBUG_TOGGLE, MF_CHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_DEBUG_TOGGLE, MF_UNCHECKED);

	CheckMenuItem( GetMenu(hwnd), wrapType, MF_CHECKED);

	if (showPointerRectangle)
		CheckMenuItem( GetMenu(hwnd), ID_OTHER_SHOW_POINTER_RECTANGLE, MF_CHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_OTHER_SHOW_POINTER_RECTANGLE, MF_UNCHECKED);

	if (cutPasteEdges)
		CheckMenuItem( GetMenu(hwnd), ID_EDIT_CUT_PASTE_EDGES, MF_CHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_EDIT_CUT_PASTE_EDGES, MF_UNCHECKED);

}

void switchIconPointer(HWND hwnd, short q)
{
	DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(0,0,0));
	DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(0,0,0));
	iconNumber = q;
	DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
	DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(255,0,0));
}

void shiftTheMap(HWND hwnd, short x, short y)
{
	short i, j, k;
	short tempArray[40][40];
	char buffer[200];
	if (x && y)
	{
		MessageBox(hwnd, "Whoah! That's too much to move at once, for now.", "Programmer too lazy error", MB_OK);
		return;
	}

	if (!x && !y)
	{
		MessageBox(hwnd, "Nothing moved. This should not have happened.", "Bug. Let me know how", MB_OK);
		return;
	}

	if (x)
	{
		for (i=shiftUp; i <= shiftDown + 1; i++)
		{
			if (LRWallArray[shiftLeft][i] != LRWallArray[shiftRight+1][i])
			{
				sprintf(buffer, "Mismatch at %d, %d to %d, %d.\n", shiftLeft, i, shiftRight, i);
				MessageBox(hwnd, buffer, "Shifting would be corrupt", MB_OK);
				return;
			}
		}

		for (j = shiftUp; j <= shiftDown; j++) //first we move the icons
			for (i = shiftLeft; i <= shiftRight; i++)
			{
				k = i + x;
				if (k < shiftLeft)
					k += shiftRight - shiftLeft + 1;
				if (k > shiftRight)
					k -= shiftRight - shiftLeft + 1;
				tempArray[k][j] = SquareIconArray[i][j];
			}

		for (i = shiftUp; i <= shiftDown; i++)
			tempArray[i][shiftRight+1] = tempArray[i][shiftLeft];

		for (j = shiftUp; j <= shiftDown; j++)
			for (i = shiftLeft; i <= shiftRight; i++)
				SquareIconArray[i][j] = tempArray[i][j];

		for (j = shiftUp; j <= shiftDown; j++) //then we move the LR walls
			for (i = shiftLeft; i <= shiftRight; i++)
			{
				k = i + x;
				if (k < shiftLeft)
					k += shiftRight - shiftLeft + 1;
				if (k > shiftRight)
					k -= shiftRight - shiftLeft + 1;
				tempArray[k][j] = LRWallArray[i][j];
			}
		for (i = shiftUp; i <= shiftDown; i++) //edge cases
			tempArray[shiftRight+1][i] = tempArray[shiftLeft][i];

		for (j = shiftUp; j <= shiftDown; j++) //copy over
			for (i = shiftLeft; i <= shiftRight+1; i++)
				LRWallArray[i][j] = tempArray[i][j];

		for (j = shiftUp; j <= shiftDown; j++) //then we move the UD walls
			for (i = shiftLeft; i <= shiftRight; i++)
			{
				k = i + x;
				if (k < shiftUp)
					k += shiftDown - shiftUp + 1;
				if (k > shiftDown)
					k -= shiftDown - shiftUp + 1;
				tempArray[k][j] = UDWallArray[i][j];
			}
		for (i = shiftLeft; i <= shiftRight; i++) //edge cases
			tempArray[i][shiftDown+1] = tempArray[i][shiftUp];

		for (j = shiftUp; j <= shiftDown+1; j++) //copy over
			for (i = shiftLeft; i <= shiftRight; i++)
				UDWallArray[i][j] = tempArray[i][j];
	}

	if (y)
	{
		for (i=shiftLeft; i <= shiftRight + 1; i++)
		{
			if (UDWallArray[i][shiftUp] != UDWallArray[i][shiftDown+1])
			{
				sprintf(buffer, "Mismatch at %d, %d to %d, %d: wall type %d vs %d.\n", i, shiftUp, i, shiftDown,
					LRWallArray[i][shiftUp], LRWallArray[i][shiftDown+1]);
				MessageBox(hwnd, buffer, "Shifting would be corrupt", MB_OK);
				return;
			}
		}

		for (j = shiftUp; j <= shiftDown; j++) //first we move the icons
			for (i = shiftLeft; i <= shiftRight; i++)
			{
				k = j + y;
				if (k < shiftUp)
					k += shiftDown - shiftUp + 1;
				if (k > shiftDown)
					k -= shiftDown - shiftUp + 1;
				tempArray[i][k] = SquareIconArray[i][j];
			}
		for (j = shiftUp; j <= shiftDown; j++)
			for (i = shiftLeft; i <= shiftRight; i++)
				SquareIconArray[i][j] = tempArray[i][j];

		for (j = shiftUp; j <= shiftDown; j++) //then we move the LR walls
			for (i = shiftLeft; i <= shiftRight; i++)
			{
				k = j + y;
				if (k < shiftUp)
					k += shiftDown - shiftUp + 1;
				if (k > shiftDown)
					k -= shiftDown - shiftUp + 1;
				tempArray[i][k] = LRWallArray[i][j];
			}
		for (i = shiftUp; i <= shiftDown; i++) //edge cases
			tempArray[shiftRight+1][i] = tempArray[shiftLeft][i];

		for (j = shiftUp; j <= shiftDown; j++) //copy over
			for (i = shiftLeft; i <= shiftRight+1; i++)
				LRWallArray[i][j] = tempArray[i][j];

		for (j = shiftUp; j <= shiftDown; j++) //then we move the UD walls
			for (i = shiftLeft; i <= shiftRight; i++)
			{
				k = j + y;
				if (k < shiftUp)
					k += shiftDown - shiftUp + 1;
				if (k > shiftDown)
					k -= shiftDown - shiftUp + 1;
				tempArray[i][k] = UDWallArray[i][j];
			}
		for (i = shiftLeft; i <= shiftRight; i++) //edge cases
			tempArray[i][shiftDown+1] = tempArray[i][shiftUp];

		for (j = shiftUp; j <= shiftDown+1; j++) //copy over
			for (i = shiftLeft; i <= shiftRight; i++)
				UDWallArray[i][j] = tempArray[i][j];
	}
	ReloadTheMap(hwnd);
}

void checkShiftExpand(HWND hwnd, short dl, short dr, short du, short dd)
{
	if (shiftLeft + dl < 0)
	{
		MessageBox(hwnd, "Too far left!", "Can't shift", MB_OK);
		return;
	}
	if (shiftLeft + dl >= shiftRight)
	{
		MessageBox(hwnd, "Can't collapse that far!", "Can't shift", MB_OK);
		return;
	}
	if (shiftRight + dr >= MAXICONSWIDE-1 )
	{
		MessageBox(hwnd, "Too far right!", "Can't shift", MB_OK);
		return;
	}
	if (shiftLeft >= shiftRight + dr)
	{
		MessageBox(hwnd, "Can't collapse that far!", "Can't shift", MB_OK);
		return;
	}
	if (shiftUp + du < 0)
	{
		MessageBox(hwnd, "Too far up!", "Can't shift", MB_OK);
		return;
	}
	if (shiftUp + du >= shiftDown)
	{
		MessageBox(hwnd, "Can't collapse that far!", "Can't shift", MB_OK);
		return;
	}
	if (shiftDown + dd >= MAXICONSWIDE-1)
	{
		MessageBox(hwnd, "Too far down!", "Can't shift", MB_OK);
		return;
	}
	if (shiftUp >=  shiftDown + dd)
	{
		MessageBox(hwnd, "Can't collapse that far!", "Can't shift", MB_OK);
		return;
	}
	shiftLeft += dl;
	shiftRight += dr;
	shiftUp += du;
	shiftDown += dd;
	noteNewShiftBox(hwnd);
}

void noteNewShiftBox(HWND hwnd)
{
	char buffer[200];
	HDC hdc = GetDC(hwnd);

	sprintf(buffer, "Shift area: %d,%d to %d,%d      ", shiftLeft, shiftUp, shiftRight, shiftDown);

	// set the colors
	SetTextColor(hdc, RGB(0,255,0));
	SetBkColor(hdc,RGB(0,0,0));
	SetBkMode(hdc,OPAQUE);

	// output the message
	TextOut(hdc,640,520,buffer,strlen(buffer));

	// release dc
	ReleaseDC(hwnd,hdc);
}