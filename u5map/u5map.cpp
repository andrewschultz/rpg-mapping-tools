// u5map.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN  

#include <ddraw.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include <commdlg.h>

#include <stdlib.h>

#include <shellapi.h>

#include "u5map.h"

#define ICONSIZE 32

//#defines for Windows
#define WINDOW_CLASS_NAME "WINCLASS1"

// functions
void ReadTheDungeons();
void PaintDunMap();
void PaintRoomMap();
short curDirValid();
void adjustSecretCheckmarks();

//globals
long curRoom = 0;
long curDungeon = 0;
long curLevel = 0;

long resetRoomA = 0;

short wrapHalf = 0;

short dunTile[8][8][8][8];
short roomBase[11][11][16][8];

HWND hwnd;

HDC roomdc;
HDC leveldc;
HDC level2dc;

HDC localhdc;

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
	short temp;

	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wparam))
		{
			//DUNGEON MENU ITEMS

		case ID_DUNGEON_DECEIT:
		case ID_DUNGEON_DESPISE:
		case ID_DUNGEON_DESTARD:
		case ID_DUNGEON_WRONG:
		case ID_DUNGEON_COVETOUS:
		case ID_DUNGEON_SHAME:
		case ID_DUNGEON_HYTHLOTH:
		case ID_DUNGEON_DOOM:
			if (curDungeon != LOWORD(wparam) - ID_DUNGEON_DECEIT)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon = LOWORD(wparam) - ID_DUNGEON_DECEIT;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				PaintRoomMap();
			}
			break;

		case ID_DUNGEON_PREV:
			if (curDungeon > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon--;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				PaintRoomMap();
			}
			break;

		case ID_DUNGEON_NEXT:
			if (curDungeon < 7)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon++;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				PaintRoomMap();
			}
			break;

			//NAVIGATION MENU ITEMS

		case ID_NAV_1:
		case ID_NAV_2:
		case ID_NAV_3:
		case ID_NAV_4:
		case ID_NAV_5:
		case ID_NAV_6:
		case ID_NAV_7:
		case ID_NAV_8:
			if (curLevel != LOWORD(wparam)-ID_NAV_1)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_UNCHECKED);
				curLevel = LOWORD(wparam) - ID_NAV_1;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_CHECKED);
				PaintDunMap();
			}
			break;

		case ID_NAV_A:
		case ID_NAV_B:
		case ID_NAV_C:
		case ID_NAV_D:
		case ID_NAV_E:
		case ID_NAV_F:
		case ID_NAV_G:
		case ID_NAV_H:
		case ID_NAV_I:
		case ID_NAV_J:
		case ID_NAV_K:
		case ID_NAV_L:
		case ID_NAV_M:
		case ID_NAV_N:
		case ID_NAV_O:
		case ID_NAV_P:
			curRoom = LOWORD(wparam) - ID_NAV_A;
			PaintDunMap();
			break;

		case ID_RESET_ROOM_A:
			resetRoomA = !resetRoomA;
			if (resetRoomA)
				CheckMenuItem( GetMenu(hwnd), ID_RESET_ROOM_A, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_RESET_ROOM_A, MF_UNCHECKED);
			break;

		case ID_NAV_UP:
			if (curLevel > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_UNCHECKED);
				curLevel--;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_CHECKED);
				PaintDunMap();
			}
			break;

		case ID_NAV_DOWN:
			if (curLevel < 7)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_UNCHECKED);
				curLevel++;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_CHECKED);
				PaintDunMap();
			}
			break;

		case ID_NAV_PREVRM:
			if (curRoom > 0)
				curRoom--;
				PaintRoomMap();
			break;

		case ID_NAV_NEXTRM:
			if (curRoom < 15)
			{
				curRoom++;
				PaintRoomMap();
			}
			break;

			//OPTIONS MENU ITEMS

		case ID_OPTIONS_WRAPHALF:
			wrapHalf = !wrapHalf;
			if (wrapHalf)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_UNCHECKED);
			break;

			//ABOUT MENU ITEMS

		case ID_ABOUT_BASICS:
			MessageBox(hwnd, "This Ultima V mapper goes through all the dungeons and dungeon rooms.", "About", MB_OK);
			break;

		case ID_ABOUT_THANKS:
			MessageBox(hwnd, "The Ultima V Codex was a big help.\nSo was GitHub.", "About", MB_OK);
			break;

		case ID_ABOUT_REPO:
			ShellExecute(hwnd, "open", "https://github.com/andrewschultz/rpg-mapping-tools/",
				NULL, NULL, SW_SHOWNORMAL);
			break;

		case ID_ABOUT_REPO_U5:
			ShellExecute(hwnd, "open", "https://github.com/andrewschultz/rpg-mapping-tools/tree/master/u5map",
				NULL, NULL, SW_SHOWNORMAL);
			break;

		}
	case WM_PAINT:
		PaintDunMap();
		PaintRoomMap();

	}

	return (DefWindowProc(hwnd, msg, wparam, lparam));

}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
WNDCLASS winclass;	// this will hold the class we create

HACCEL hAccelTable;
MSG		 msg;		// generic message

winclass.style			= CS_DBLCLKS | CS_OWNDC | 
                          CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hInstance;
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
							  "Ultima 5 Dungeon Simulator",	     // title
							  WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					 		  0,0,	   // x,y
							  20*ICONSIZE, 16*ICONSIZE, // width, height
							  NULL,	   // handle to parent 
							  NULL,	   // handle to menu
							  hInstance,// instance
							  NULL)))	// creation parms
	return(0);

	localhdc = GetDC(hwnd);

	HBITMAP roombmp = (HBITMAP)LoadImage(hInstance,"room.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	HBITMAP levelbmp = (HBITMAP)LoadImage(hInstance,"level.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	HBITMAP level2bmp = (HBITMAP)LoadImage(hInstance,"level2.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	roomdc = CreateCompatibleDC(localhdc);
	leveldc = CreateCompatibleDC(localhdc);
	level2dc = CreateCompatibleDC(localhdc);

	HBITMAP oldroom = (HBITMAP)SelectObject(roomdc, roombmp);
	HBITMAP oldlevel = (HBITMAP)SelectObject(leveldc, levelbmp);
	HBITMAP oldlevel2 = (HBITMAP)SelectObject(level2dc, level2bmp);

	ReadTheDungeons();
	PaintDunMap();
	PaintRoomMap();

    hAccelTable = LoadAccelerators(hInstance, "MYACCEL");

	CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);

	if (resetRoomA)
		CheckMenuItem( GetMenu(hwnd), ID_RESET_ROOM_A, MF_CHECKED);

	while (1)
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
    
	}
	return 0;
}

void PaintDunMap()
{
	int i = 0;
	int j = 0;

	if (wrapHalf)
	{
		for (j=0; j < 8; j++)
		{
			for (i=0; i < 8; i++)
			{
				BitBlt(localhdc, i*16, j*16, 16, 16, level2dc,
					16*(dunTile[i][j][curLevel][curDungeon] % 0x10), 16*(dunTile[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16, 16, 16, level2dc,
					16*(dunTile[i][j][curLevel][curDungeon] % 0x10), 16*(dunTile[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16, j*16+128, 16, 16, level2dc,
					16*(dunTile[i][j][curLevel][curDungeon] % 0x10), 16*(dunTile[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16+128, 16, 16, level2dc,
					16*(dunTile[i][j][curLevel][curDungeon] % 0x10), 16*(dunTile[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
			}
		}
		return;
	}

	for (j=0; j < 8; j++)
	{
		for (i=0; i < 8; i++)
			BitBlt(localhdc, i*32, j*32, 32, 32, leveldc,
				32*(dunTile[i][j][curLevel][curDungeon] % 0x10), 32*(dunTile[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
	}
}

void PaintRoomMap()
{
	short i, j;
	short tempIcon[11][11];

	if (curDungeon == 1)
	{
		Rectangle(localhdc, 288, 0, 640, 352);
		return;
	}
	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			tempIcon[i][j] = roomBase[i][j][curRoom][curDungeon];

	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			BitBlt(localhdc, i*32+288, j*32, 32, 32, roomdc,
				32*(tempIcon[i][j] % 0x20), 32*(tempIcon[i][j] / 0x20), SRCCOPY);
}

void ReadTheDungeons()
{
	short tempRoom[32][11];

	short i, j, k, l;

	FILE * F = fopen("DUNGEON.DAT", "rb");

	for (l=0; l < 8; l++)
		for (k=0; k < 8; k++)
			for (j=0; j < 8; j++)
				for (i=0; i < 8; i++)
					dunTile[i][j][k][l] = fgetc(F);

	fclose(F);

	F = fopen("DUNGEON.CBT", "rb");

	for (l=0; l < 8; l++)
	{
		if (l == 1)
			l++;
		for (k=0; k < 16; k++)
		{
			for (j=0; j < 11; j++)
				for (i=0; i < 32; i++)
					tempRoom[i][j] = fgetc(F);
			for (j=0; j < 11; j++)
				for (i=0; i <11; i++)
					roomBase[i][j][k][l] = tempRoom[i][j];
		}
	}
}

short curDirValid()
{
	return 0;
}

void adjustSecretCheckmarks()
{
}