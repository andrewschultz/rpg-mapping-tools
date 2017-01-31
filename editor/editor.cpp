//editor.cpp
//copyright ??? by Andrew Schultz
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
void DrawPointerRectangle(HWND hwnd, long xOffset, long yOffset, COLORREF myColor);
void ReadBinaryMap(HWND hwnd, char x[MAXFILENAME]);
void ReloadTheMap(HWND hwnd);
void drawMyIcons(HWND hwnd);
void CreateNewMapfile(long, long);
void SaveMapfile(void);
void flipStuff (HWND hwnd, int xi, int yi, int xf, int yf);
void SaveBitmapFile(HWND hwnd);
void parseCmdLine(LPSTR cmdLine, HWND hwnd);


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
short CutUDWallArray[MAXICONSWIDE+1][MAXICONSHIGH+1];
short CutLRWallArray[MAXICONSWIDE+1][MAXICONSHIGH+1];
short CutSquareIconArray[MAXICONSWIDE][MAXICONSHIGH];

short ControlKeyDown = 0;
long MouseDownX, MouseDownY;
long xCurrent = 0, yCurrent = 0, iconNumber = 0, massCutPasteIcon;
long notPeriod = 0;
long WallIconNumber = 2;	//default to doors. Can add/delete walls with (shift) WASZ
long myW = MAXICONSWIDE, myH = MAXICONSHIGH;
char CurrentFileName[200];

long cutHeight=0, cutWidth = 0;
long zeroBottom = 1;
short myEmpty = 0, myTransparency = 0, showPointerRectangle = 1;

long bufferL=0,bufferR=0,bufferU=0,bufferD=0;
long wrapType = ID_OTHER_NOWRAP;

long workNotSaved;

short prevDefArray[10] = {
	0x3f, 0x26, 0x56, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0x3f
};

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
			CreateNewMapfile(35, 35);
			break;

		case ID_FILE_OPEN:
			DoOpenFile(hwnd);
			ReloadTheMap(hwnd);
			break;

		case ID_FILE_SAVE_BITMAP:
			ReloadTheMap(hwnd);
			SaveBitmapFile(hwnd);
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
				cutWidth = bufferL-bufferR+1;

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
					for (j=bufferU; j <= bufferD; j++)
						for (i=bufferL; i <= bufferR; i++)
							SquareIconArray[i][j] = 0;

					for (j=bufferU; j <= bufferD+1; j++)
						for (i=bufferL; i <= bufferR; i++)
							UDWallArray[i][j] = 0;

					for (j=bufferU; j <= bufferD; j++)
						for (i=bufferL; i <= bufferR+1; i++)
							LRWallArray[i][j] = 0;
				}
				ReloadTheMap(hwnd);
			}
			break;

		case ID_EDIT_PASTE:
			if (bufferL == -1)
				break;
			{
				long i, j;

				for (j=bufferU; j <= bufferD; j++)
					for (i=bufferL; i <= bufferR; i++)
						if ((!myTransparency) || (CutSquareIconArray[i+xCurrent-bufferL][j+yCurrent-bufferU]))
							SquareIconArray[i+xCurrent-bufferL][j+yCurrent-bufferU] = CutSquareIconArray[i-bufferL][j-bufferU];

				for (j=bufferU; j <= bufferD+1; j++)
					for (i=bufferL; i <= bufferR; i++)
						if ((!myTransparency) || (CutUDWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU]))
							UDWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU] = CutUDWallArray[i-bufferL][j-bufferU];

				for (j=bufferU; j <= bufferD; j++)
					for (i=bufferL; i <= bufferR+1; i++)
						if ((!myTransparency) || (CutLRWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU]))
							LRWallArray[i+xCurrent-bufferL][j+yCurrent-bufferU] = CutLRWallArray[i-bufferL][j-bufferU];
			}
			ReloadTheMap(hwnd);
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
			myEmpty = 32;
			if (LOWORD(wparam) == ID_VERIFY_TWO1616)
				myEmpty = 16;
			{
				long firstX=-1, firstY=-1, i, j, lastX, lastY, count=0;

				for (j=0; j < 33; j++)
					for (i=0; i < 33; i++)
					{
						if (SquareIconArray[i][j] == 0)
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

				if (count)
				{
					char mybuf[200];
					float myPct = ((1024-(float)count) * 100) / 1024;
					sprintf(mybuf, "%d squares unvisited\nFirst %02x,%02x\nLast %02x,%02x\n%.02f pct done", count, firstX, firstY, lastX, lastY, myPct);
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
				"It prints out walls and items and may offer text commentaries one day.", "About", MB_OK);
			break;

		case ID_HELP_DOCS:
			MessageBox(hwnd,
				"[] not implemented yet\nArrow keys move left side focus\nCtrl arrow keys move between icons\n\
WASZ puts in walls, Shift-WASZ removes\n0-9 selects predefined icon\nshift 0-9 selects wall type 0-9\n\
IJKM places special walls\n[home/end left-right, ctrl = up/down]\nspace places icons\n\
delete deletes icons, shift-del deletes walls\n", "Docs", MB_OK);
			break;

		case ID_HELP_THANKS:
			MessageBox(hwnd, "CodeGuru.com\n"
				"Andre LaMothe for Windows for Dummies\n"
				"Microsoft\'s online help\n"
				"http://www.gametutorials.com\n"
				"Probably some of his code and comments still creeped in here\n", "Thanks", MB_OK);
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
				hdc = GetDC(hwnd);
        
				// set the colors 
				SetTextColor(hdc, RGB(0,255,0));
				SetBkColor(hdc,RGB(0,0,0));
				SetBkMode(hdc,OPAQUE);

				// output the message
				TextOut(hdc,600,440,buffer,strlen(buffer));

				// release dc
				ReleaseDC(hwnd,hdc);
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

				sprintf(buffer, "%02x,%02x to %02x,%02x.", bufferL, bufferU, bufferR+1, bufferD+1);
				hdc = GetDC(hwnd);
        
				// set the colors 
				SetTextColor(hdc, RGB(0,255,0));
				SetBkColor(hdc,RGB(0,0,255));
				SetBkMode(hdc,OPAQUE);

				// output the message
				TextOut(hdc,608,480,buffer,strlen(buffer));

				// release dc
				ReleaseDC(hwnd,hdc);

			break;
		}
		//bufferL=bufferR=bufferU=bufferD=-1;

//Mouse select new icons
		if ((MouseDownX >= 38) && (MouseDownX <= 53))
			if ((MouseDownY >= 10) && (MouseDownY <= 25))
			{
				DrawPointerRectangle(hwnd, 37, 10 + (iconNumber >> 4), BLACK);
				DrawPointerRectangle(hwnd, 38 + (iconNumber & 0xf), 9, BLACK);
				iconNumber = (MouseDownX - 38) + ((MouseDownY - 10) << 4);
				DrawPointerRectangle(hwnd, 37, MouseDownY, RGB(255,0,0));
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

				sprintf(buffer, "%02d,%02d D\r\n%02x,%02x H", xCurrent, yc2, xCurrent, yc2);
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
		if ((MouseDownX >= 36) && (MouseDownX <= 45))
			if ((MouseDownY >= 2) && (MouseDownY <= 3))
			{
				DrawPointerRectangle(hwnd, WallIconNumber, 1, BLACK);
				WallIconNumber = MouseDownX - 36;
				DrawPointerRectangle(hwnd, WallIconNumber, 1, RED);
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
				if (KEY_DOWN(VK_SHIFT))
				{
					UDWallArray[xCurrent][yCurrent] = 
					LRWallArray[xCurrent][yCurrent] = 
					UDWallArray[xCurrent][(yCurrent+1) % MAXICONSHIGH] = 
					LRWallArray[(xCurrent+1) % MAXICONSWIDE][yCurrent] = 0;
					ReloadTheMap(hwnd);
				}
				else
					SquareIconArray[xCurrent][yCurrent] = 0;
					ReloadTheMap(hwnd);
				break;


			case VK_LEFT:
			case VK_RIGHT:
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
						hdc = GetDC(hwnd);
    
						// set the colors 
						SetTextColor(hdc, RGB(0,255,0));
						SetBkColor(hdc,RGB(0,0,0));
						SetBkMode(hdc,OPAQUE);

						// output the message
						TextOut(hdc,600,440,buffer,strlen(buffer));

						// release dc
						ReleaseDC(hwnd,hdc);
					}
					else
					{
						//rectangle down the side & along top
						DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,255,255));
						DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,255,255));

						switch(wparam)
						{
						case VK_LEFT:
							xCurrent--;
							if (xCurrent & 0xf00)
								xCurrent += MAXICONSWIDE;
							break;

						case VK_RIGHT:
							xCurrent++;
							if (xCurrent == MAXICONSWIDE)
								xCurrent = 0;
							break;

						case VK_UP:
							yCurrent--;
							if (yCurrent & 0xf00)
								yCurrent += MAXICONSHIGH;
							break;

						case VK_DOWN:
							yCurrent++;
							if (yCurrent == MAXICONSHIGH)
								yCurrent = 0;
							break;

						}

						DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,0,0));
						DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,0,0));

				yc2 = yCurrent;
				if (zeroBottom) yc2 = 31 - yc2;
				sprintf(buffer, "%02d,%02d D\r\n%02x,%02x H", xCurrent, yc2, xCurrent, yc2);
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

					if (KEY_DOWN(VK_CONTROL))
						UDWallArray[xCurrent][yCurrent] = 0;
					else
						UDWallArray[xCurrent][yCurrent] = (short)WallIconNumber;

					hdc = GetDC(hwnd);
        
					// set the colors 
					SetTextColor(hdc, RGB(0,255,0));
					SetBkColor(hdc,RGB(0,0,0));
					SetBkMode(hdc,OPAQUE);

					// output the message
					TextOut(hdc,600,440,buffer,strlen(buffer));

					// release dc
					ReleaseDC(hwnd,hdc);

					ReloadTheMap(hwnd);

					break;
				}

			case VK_A:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL))
						LRWallArray[xCurrent][yCurrent] = 0;
					else
						LRWallArray[xCurrent][yCurrent] = (short)WallIconNumber;

					hdc = GetDC(hwnd);
        
					// set the colors 
					SetTextColor(hdc, RGB(0,255,0));
					SetBkColor(hdc,RGB(0,0,0));
					SetBkMode(hdc,OPAQUE);

					// output the message
					TextOut(hdc,600,440,buffer,strlen(buffer));

					// release dc
					ReleaseDC(hwnd,hdc);

					ReloadTheMap(hwnd);

					break;
				}
				break;

			case VK_S:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL))
						LRWallArray[xCurrent+1][yCurrent] = 0;
					else
						LRWallArray[xCurrent+1][yCurrent] = (short)WallIconNumber;

					hdc = GetDC(hwnd);
        
					// set the colors 
					SetTextColor(hdc, RGB(0,255,0));
					SetBkColor(hdc,RGB(0,0,0));
					SetBkMode(hdc,OPAQUE);

					// output the message
					TextOut(hdc,600,440,buffer,strlen(buffer));

					// release dc
					ReleaseDC(hwnd,hdc);

					ReloadTheMap(hwnd);

					break;
				}
				break;

			case VK_Z:
				{
					char buffer[100];

					sprintf(buffer, "Lay piece down. %d",wparam);

					if (KEY_DOWN(VK_CONTROL))
						UDWallArray[xCurrent][yCurrent+1] = 0;
					else
						UDWallArray[xCurrent][yCurrent+1] = (short)WallIconNumber;

					hdc = GetDC(hwnd);
        
					// set the colors 
					SetTextColor(hdc, RGB(0,255,0));
					SetBkColor(hdc,RGB(0,0,0));
					SetBkMode(hdc,OPAQUE);

					// output the message
					TextOut(hdc,600,440,buffer,strlen(buffer));

					// release dc
					ReleaseDC(hwnd,hdc);

					ReloadTheMap(hwnd);

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

			case 0xbc: // comma
				DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(0,0,0));
				DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(0,0,0));
				if (iconNumber == BEENTHERE)
				{
					iconNumber = notPeriod;
				}
				else
				{
					notPeriod = iconNumber;
				}
				DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
				DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(255,0,0));
				drawMyIcons(hwnd);
				break;

			case 0xbe:	//period
				DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(0,0,0));
				DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(0,0,0));
;				if (iconNumber == BEENTHERE)
					iconNumber = 0;
				else
				{
					notPeriod = iconNumber;
					iconNumber = BEENTHERE;
				}
				DrawPointerRectangle(hwnd, MAXICONSWIDE+2, 10+(iconNumber/16), RGB(255,0,0));
				DrawPointerRectangle(hwnd, MAXICONSWIDE+3+(iconNumber%16), 9, RGB(255,0,0));
				drawMyIcons(hwnd);
				break;

			case VK_C:
				{
					short i, j;
					for (j=0; j < myH; j++)
						for (i=0; i < myW; i++)
							UDWallArray[i][j] = LRWallArray[i][j] = SquareIconArray[i][j] = 0;
				}
				break;

			case 0xba:	//semicolon
				if (KEY_DOWN(VK_SHIFT))
					UDWallArray[xCurrent][yCurrent+1] = 5;
				else
					UDWallArray[xCurrent][yCurrent+1] = 4;
				ReloadTheMap(hwnd);
				break;

			case VK_G:
				iconNumber = (long) SquareIconArray[xCurrent][yCurrent];
				break;



			case VK_BACK:
				if (VK_CONTROL)
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
					SquareIconArray[xCurrent][yCurrent] = (short)iconNumber;

					//Write over original icon, draw out walls
					/*
					hdc = GetDC(hwnd);
        
					// set the colors 
					SetTextColor(hdc, RGB(0,255,0));
					SetBkColor(hdc,RGB(0,0,0));
					SetBkMode(hdc,OPAQUE);

					// output the message
					TextOut(hdc,600,440,buffer,strlen(buffer));

					// release dc
					ReleaseDC(hwnd,hdc);
					*/

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

	case WM_DESTROY:
		{
		// kill the application
			if (workNotSaved)
			{
				long x = MessageBox(NULL, "Do you wish to exit without saving? If so, hit OK. If not, hit Cancel.", "Save Warning", MB_OKCANCEL);
			}
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

    hAccelTable = LoadAccelerators(hinstance, "MYACCEL");
// enter main event loop

{
	HBITMAP iconbmp = (HBITMAP)LoadImage(hinstance,"icons.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	HBITMAP wallbmp = (HBITMAP)LoadImage(hinstance,"walls.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	HDC      localhdc = GetDC(hwnd);

icondc = CreateCompatibleDC(localhdc);
walldc = CreateCompatibleDC(localhdc);

	HBITMAP oldicon = (HBITMAP)SelectObject(icondc, iconbmp);
	HBITMAP oldwall = (HBITMAP)SelectObject(walldc, wallbmp);

	drawMyIcons(hwnd);

	SetBkColor(localhdc,RGB(255,255,255));
	Rectangle(localhdc, 0,0,560,560);

	parseCmdLine(lpcmdline, hwnd);

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
}
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
  
#ifdef WINCE
  locOFN.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_LONGNAMES;
#else
  locOFN.Flags = OFN_FILEMUSTEXIST;
#endif
  locOFN.lpstrDefExt = NULL;
  locOFN.lCustData = 0;
  locOFN.lpfnHook = NULL;
  locOFN.lpTemplateName = NULL;

  if (GetOpenFileName(&locOFN) == TRUE)
  {
	  long i;
	  for (i=0; locFileName[i]; i++);
	  i--;
	  if (locFileName[i] == 'p')
	  {
		  char guiTitle[200] = "GUI Map Editor - ";
		  ReadBinaryMap(hwnd, locFileName);
		  strcpy(CurrentFileName, locFileName);
		  strcat(guiTitle, PathFindFileName(CurrentFileName));
		  SetWindowText(hwnd, guiTitle);
	  }
  }
}

///////////////////////////////////////////////////////////

void ReadBinaryMap(HWND hwnd, char x[MAXFILENAME])
{
	long j, i;

	FILE * F = fopen(x, "rb");
	if (F == NULL)
	{
		MessageBox(hwnd, "Uh oh", "Uh oh", MB_OK);
		return;
	}

	//initialize
	for (j=0; j < MAXICONSHIGH; j++)
		for (i=0; i < MAXICONSWIDE; i++)
			SquareIconArray[i][j] = 0;

	for (j=0; j < MAXICONSHIGH+1; j++)
		for (i=0; i < MAXICONSWIDE+1; i++)
			UDWallArray[i][j] = LRWallArray[i][j] = 0;

	myW = fgetc(F);
	myH = fgetc(F);

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
	long q;
	
	if (CurrentFileName[0] == 0)
		q = MessageBox(hwnd, "Blank file, save to blank.map?", "Blank file", MB_OKCANCEL);
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
		}
		else
			return;

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
	for (i=0; i < myW; i++)
	{
		fputc(0, F);
		fputc(UDWallArray[i][j], F);
	}
	fputc(0, F);

	fclose(F);
	workNotSaved = 0;
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
	rect.bottom=ICONHEIGHT*MAXICONSHIGH;
	rect.left=0;
	rect.right=ICONWIDTH*MAXICONSWIDE;

	FillRect(hdc, &rect, hbrush);

	DeleteObject(hbrush);

	for (j=0; j < MAXICONSHIGH-1; j++)
	{
		for (i=0; i < MAXICONSWIDE-1; i++)
			BitBlt(hdc, i*16+16, j*16+16, 16, 16, icondc, XVAL(i,j), YVAL(i,j), SRCCOPY);
	}

	if (showPointerRectangle)
	{
		DrawPointerRectangle(hwnd, xCurrent+1, 0, RGB(255,0,0));
		DrawPointerRectangle(hwnd, 0, yCurrent+1, RGB(255,0,0));
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

	ReleaseDC(hwnd, hdc);
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

void SaveBitmapFile(HWND hwnd)
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

	char buf2[200];

	strcpy(buf2, CurrentFileName);
	buf2[strlen(buf2)-3] = 'b';
	buf2[strlen(buf2)-2] = 'm';
	

	if (!IsWindow(hwnd))	/* should never happen, but just in case... */
		return;

	ReloadTheMap(hwnd);

	hWinDC = GetDC(hwnd);
	hDC = CreateCompatibleDC(hWinDC);

	wsprintf(buf,TEXT(buf2),shotcount);
	shotcount++;

	GetClientRect(hwnd,&rPage);
	rPage.left = rPage.top = 8;
	rPage.bottom = rPage.right = 552;

	numBytes = 544 * 544 * 3;

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 544;
	bmi.bmiHeader.biHeight = 544;
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
	BitBlt(hDC,0,0,544,544,hWinDC,8,8,SRCCOPY);

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

void parseCmdLine(LPSTR lpcmdline, HWND hwnd)
{
	short numSpaces = 0;
	long i;
	char buffer[200] = "";

	for (i=0; lpcmdline[i] !=0; i++)
		numSpaces+= (lpcmdline[i] == ' ');

	if ((numSpaces == 0) && (i > 1))
	{
		ReadBinaryMap(hwnd, lpcmdline);
		strcpy(CurrentFileName, lpcmdline);
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