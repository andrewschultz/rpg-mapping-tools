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

//#defines for Windows
#define WINDOW_CLASS_NAME "WINCLASS1"

// functions
void ReadTheDungeons();
void PaintDunMap();
void PaintRoomMap();
short curDirValid();
void adjustSecretCheckmarks();
void checkPrevNextRoom();
void checkPrevNextDun();
void checkPrevNextLvl();
void adjHeader();
short toMonster(short icon);
void doRoomCheck();

//local pound-defines
#define MIMIC 0xa8

#define DECEIT 0
#define DESPISE 1
#define DESTARD 2
#define WRONG 3
#define COVETOUS 4
#define SHAME 5
#define HYTHLOTH 6
#define DOOM 7

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

#define ICONSIZE 32

//globals
long curRoom = 0;
long curDungeon = 0;
long curLevel = 0;

long resetRoomA = 0;
long resetLvl1 = 0;

short syncLevelToRoom = 0;
short restrictRoom = 0;

short wrapHalf = 0;
short hideMimic = 0;

long mouseDownX, mouseDownY;

short mainDun[8][8][8][8];
short roomBase[11][11][16][8];

short roomLev[16][8];

short monsterType[21][16][8];
short monsterX[21][16][8];
short monsterY[21][16][8];

short partyX[4][6][16][8];
short partyY[4][6][16][8];

short roomTextSummary = 0;
short showMonsters = 0;
short showSpoilers = 0;
short showParty = 0;
short newPushed = 0;
short showPushed = 0;

short curDir = 0;

short whatTo[8][16][8];
short fromSquareX[8][16][8];
short fromSquareY[8][16][8];
short toSquare1X[8][16][8];
short toSquare1Y[8][16][8];
short toSquare2X[8][16][8];
short toSquare2Y[8][16][8];

short partyArray[6] = { 0x14c, 0x144, 0x148, 0x140, 0x140, 0x140 };

char dunName[8][9] = { "Deceit", "Despise", "Destard", "Wrong", "Covetous", "Shame", "Hythloth", "Doom"};

#define MONSTERS 46
char monsterName[MONSTERS][11] = { "Seahorse", "Squid", "Serpent", "Shark", "Rat", "Bat", "Spider", "Ghost", "Slime", "Gremlin", "Mimic", "Reaper", "Gazer", "TREASURE", "Gargoyle", "Insects", "Orc", "Skeleton", "Snake", "Ettin", "Headless", "Wisp", "Mongbat", "Dragon", "Sand Trap", "Troll", "FIELD", "Whorl", "Balrog", "Corpser", "Rotworm", "Shadowlord" }; 

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
				if (resetLvl1)
					curLevel = 0;
				PaintRoomMap();
				checkPrevNextDun();
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
				if (resetLvl1)
					curLevel = 0;
				PaintRoomMap();
				checkPrevNextDun();
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
				if (resetLvl1)
					curLevel = 0;
				PaintRoomMap();
				checkPrevNextDun();
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
				checkPrevNextDun();
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
			checkPrevNextRoom();
			break;

		case ID_OPTIONS_ROOM_A:
			resetRoomA = !resetRoomA;
			if (resetRoomA)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ROOM_A, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ROOM_A, MF_UNCHECKED);
			break;

		case ID_OPTIONS_LVL_1:
			resetLvl1 = !resetLvl1;
			if (resetLvl1)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_LVL_1, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_LVL_1, MF_UNCHECKED);
			break;

		case ID_NAV_UP:
			if (curLevel > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_UNCHECKED);
				curLevel--;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_CHECKED);
				PaintDunMap();
				checkPrevNextLvl();
			}
			break;

		case ID_NAV_DOWN:
			if (curLevel < 7)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_UNCHECKED);
				curLevel++;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_CHECKED);
				PaintDunMap();
				checkPrevNextLvl();
			}
			break;

		case ID_NAV_PREVRM:
			if (curRoom > 0)
				if (restrictRoom)
					if (curLevel != roomLev[curRoom-1][curDungeon])
						break;
				curRoom--;
				PaintRoomMap();
				checkPrevNextRoom();
			break;

		case ID_NAV_NEXTRM:
			if (curRoom < 15)
			{
				if (restrictRoom)
					if (curLevel != roomLev[curRoom+1][curDungeon])
						break;
				curRoom++;
				PaintRoomMap();
				checkPrevNextRoom();
			}
			break;

			//OPTIONS MENU ITEMS

		case ID_OPTIONS_WRAPHALF:
			wrapHalf = !wrapHalf;
			if (wrapHalf)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_UNCHECKED);
			PaintDunMap();
			break;

		case ID_OPTIONS_TEXTSUMMARY:
			roomTextSummary = !roomTextSummary;
			if (roomTextSummary)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_TEXTSUMMARY, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_TEXTSUMMARY, MF_UNCHECKED);
			break;

		case ID_OPTIONS_MONSTERS:
			showMonsters = !showMonsters;
			if (showMonsters)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MONSTERS, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MONSTERS, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_OPTIONS_HIDE_MIMIC:
			hideMimic = !hideMimic;
			if (hideMimic)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_MIMIC, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_MIMIC, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_OPTIONS_SHOW_SPOILERS:
			showSpoilers = !showSpoilers;
			if (showSpoilers)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_SPOILERS, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_SPOILERS, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_OPTIONS_PARTY_NONE:
		case ID_OPTIONS_PARTY_NORTH:
		case ID_OPTIONS_PARTY_EAST:
		case ID_OPTIONS_PARTY_SOUTH:
		case ID_OPTIONS_PARTY_WEST:
		case ID_OPTIONS_PARTY_FIRSTVIABLE:
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE + showParty, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
			showParty = LOWORD(wparam) - ID_OPTIONS_PARTY_NONE;
			PaintRoomMap();
			break;

		case ID_OPTIONS_SYNC_LEVEL_TO_ROOM:
			syncLevelToRoom = !syncLevelToRoom;
			if (syncLevelToRoom)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SYNC_LEVEL_TO_ROOM, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SYNC_LEVEL_TO_ROOM, MF_UNCHECKED);
			break;

		case ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL:
			restrictRoom = !restrictRoom;
			if (restrictRoom)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL, MF_UNCHECKED);
			break;

		case ID_OPTIONS_PSGS_0:
		case ID_OPTIONS_PSGS_1:
		case ID_OPTIONS_PSGS_2:
		case ID_OPTIONS_PSGS_3:
		case ID_OPTIONS_PSGS_4:
		case ID_OPTIONS_PSGS_5:
		case ID_OPTIONS_PSGS_6:
		case ID_OPTIONS_PSGS_7:
		case ID_OPTIONS_PSGS_8:
			showPushed = LOWORD(wparam) - ID_OPTIONS_PSGS_0;
			PaintRoomMap();
			break;

		case ID_OPTIONS_SHOW_PUSHED:
			newPushed = !newPushed;
			showPushed = 8 * newPushed;
			if (showPushed)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_PUSHED, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_PUSHED, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_OPTIONS_NOGUY_2:
		case ID_OPTIONS_NOGUY_3:
		case ID_OPTIONS_NOGUY_4:
		case ID_OPTIONS_NOGUY_5:
		case ID_OPTIONS_NOGUY_6:
		case ID_OPTIONS_MAGE_2:
		case ID_OPTIONS_MAGE_3:
		case ID_OPTIONS_MAGE_4:
		case ID_OPTIONS_MAGE_5:
		case ID_OPTIONS_MAGE_6:
		case ID_OPTIONS_BARD_2:
		case ID_OPTIONS_BARD_3:
		case ID_OPTIONS_BARD_4:
		case ID_OPTIONS_BARD_5:
		case ID_OPTIONS_BARD_6:
		case ID_OPTIONS_FIGHTER_2:
		case ID_OPTIONS_FIGHTER_3:
		case ID_OPTIONS_FIGHTER_4:
		case ID_OPTIONS_FIGHTER_5:
		case ID_OPTIONS_FIGHTER_6:
			temp = LOWORD(wparam) - ID_OPTIONS_NOGUY_2;
			if (temp / 5) //ignore this if there is no guy;
				partyArray[temp % 5 + 1] = 316 + 4 * (temp / 5);
			else
				partyArray[temp % 5 + 1] = 0;
			PaintRoomMap();
			if (temp % 5 == 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MAGE_2, MF_UNCHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_FIGHTER_2, MF_UNCHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_BARD_2, MF_UNCHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_NOGUY_2, MF_UNCHECKED);
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
			}
			break;
			//ABOUT MENU ITEMS

		case ID_OPTIONS_FBMDOC:
			MessageBox(hwnd, "F8-F12 are players 2-6.\nCtrl=fighter, shift=bard, alt=mage none=no player.\n\
You can't change the Avatar, because.\nYou also can't change friends to the Avatar, because.",
				"Changing fellow PCs", MB_OK);
			break;

		case ID_ABOUT_BASICS:
			MessageBox(hwnd, "Ultima V Dungeon Browser\n\
This application lets the player browse all the dungeons and rooms.\n\
It features options and accelerators to bypass the usual traps and grinds.\n\
Your party is customizable for fun, too.\n\
Bugs? schultz.andrew@sbcglobal.net", "About", MB_OK);
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

		case ID_ABOUT_README:
			ShellExecute(hwnd, "open", "readme.txt", NULL, NULL, SW_SHOWNORMAL);
			break;

		}
		case WM_LBUTTONDOWN:
			mouseDownX = LOWORD(lparam)/16;
			mouseDownY = HIWORD(lparam)/16;
			break;

		case WM_LBUTTONUP:
			{
				long mouseUpX = LOWORD(lparam)/16;
				long mouseUpY = HIWORD(lparam)/16;

				if ((mouseUpX != mouseDownX) || (mouseUpY != mouseDownY))
					break;

				if ((mouseUpX < 16) && (mouseUpY < 16))
				{
					if (wrapHalf)
					{
						mouseDownX %= 8;
						mouseDownY %= 8;
						mouseUpX %= 8;
						mouseUpY %= 8;
					}
					else
					{
						mouseDownX /= 2;
						mouseDownY /= 2;
						mouseUpX /= 2;
						mouseUpY /= 2;
					}
					temp = mainDun[mouseDownX][mouseDownY][curLevel][curDungeon];
					if ((temp >= 0xf0) && (temp <= 0xff))
					{
						curRoom = temp & 0xf;
						doRoomCheck();
					}
				}
				if ((mouseUpX >= 18) && (mouseUpX < 40) && (mouseUpY < 22) && (showParty))
				{
					short i;
					mouseUpX /= 2;
					mouseUpX -= 9;
					mouseUpY /= 2;
					for (i=1; i < 6; i++)
						if ((partyX[curDir][i][curRoom][curDungeon] == mouseUpX) && (partyY[curDir][i][curRoom][curDungeon] == mouseUpY))
						{
							partyArray[i] += 4;
							if (partyArray[i] == 0x14c)
								partyArray[i] = 0x140;
						}
					doRoomCheck();
				}
			}
			break;

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
							  WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU,
					 		  0,0,	   // x,y
							  20*ICONSIZE, 16*ICONSIZE, // width, height. We want space for text at the bottom to allow room description.
							  NULL,	   // handle to parent 
							  NULL,	   // handle to menu
							  hInstance,// instance
							  NULL)))	// creation parms
	return(0);

	localhdc = GetDC(hwnd);

	HBITMAP roombmp = (HBITMAP)LoadImage(hInstance,"room.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	HBITMAP levelbmp = (HBITMAP)LoadImage(hInstance,"level.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	roomdc = CreateCompatibleDC(localhdc);
	leveldc = CreateCompatibleDC(localhdc);

	HBITMAP oldroom = (HBITMAP)SelectObject(roomdc, roombmp);
	HBITMAP oldlevel = (HBITMAP)SelectObject(leveldc, levelbmp);

	ReadTheDungeons();
	PaintDunMap();
	PaintRoomMap();

    hAccelTable = LoadAccelerators(hInstance, "MYACCEL");

	CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE, MF_UNCHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_BARD_2, MF_CHECKED);

	if (resetRoomA)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ROOM_A, MF_CHECKED);

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
				BitBlt(localhdc, i*16, j*16, 16, 16, leveldc,
					16*(mainDun[i][j][curLevel][curDungeon] % 0x10), 16*(mainDun[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16, 16, 16, leveldc,
					16*(mainDun[i][j][curLevel][curDungeon] % 0x10), 16*(mainDun[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16, j*16+128, 16, 16, leveldc,
					16*(mainDun[i][j][curLevel][curDungeon] % 0x10), 16*(mainDun[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16+128, 16, 16, leveldc,
					16*(mainDun[i][j][curLevel][curDungeon] % 0x10), 16*(mainDun[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
			}
		}
		return;
	}

	for (j=0; j < 8; j++)
	{
		for (i=0; i < 8; i++)
			StretchBlt(localhdc, i*32, j*32, 32, 32, leveldc,
				16*(mainDun[i][j][curLevel][curDungeon] % 0x10), 16*(mainDun[i][j][curLevel][curDungeon] / 0x10), 16, 16, SRCCOPY);
	}
	adjHeader();
}

void PaintRoomMap()
{
	short i, j, temp;
	short tempIcon[11][11];
	short checkAry[11][11] = {0};

	if (curDungeon == DESPISE)
	{
	HDC hdc = GetDC(hwnd);
	HBRUSH hbrush=CreateSolidBrush(RGB(128,128,128));
	RECT rect;

	rect.top=0;
	rect.bottom=11 * ICONSIZE; //yeah I can do math it's 352 but
	rect.left=288;
	rect.right=288 + 11 * ICONSIZE;

	FillRect(hdc, &rect, hbrush);

	DeleteObject(hbrush);
		return;
	}
	//first the base icons
	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			tempIcon[i][j] = roomBase[i][j][curRoom][curDungeon];
			if ((!hideMimic) && (roomBase[i][j][curRoom][curDungeon] == MIMIC)) //Show the mimic
				tempIcon[i][j] += 1;

	//now show monsters
	if (showMonsters)
	{
		for (i=0; i < 21; i++)
			if (monsterType[i][curRoom][curDungeon])
				tempIcon[monsterX[i][curRoom][curDungeon]][monsterY[i][curRoom][curDungeon]] = monsterType[i][curRoom][curDungeon] + 256;
	}


	//"smart" placement
	if (showParty == 5)
	{
		for (i=NORTH; i <= WEST; i++)
		{
			if (partyX[i][0][curRoom][curDungeon] + partyY[i][0][curRoom][curDungeon])
			{
				for (j=0; j < 6; j++)
					if (partyArray[j])
						tempIcon[partyX[i][j][curRoom][curDungeon]][partyY[i][j][curRoom][curDungeon]] = partyArray[j];
				curDir = i;
				break;
			}
		}
		if (i == 4)
			MessageBox(hwnd, "Choosing a viable direction did not work! Please note the room and dungeon and report it at u4map's github site.", "Probable bug", MB_OK);
	}
	else if (showParty > 0)
		curDir = showParty - 1;

	switch (showParty)
	{
	case NORTH + 1:
	case SOUTH + 1:
	case EAST + 1:
	case WEST + 1:
		if (partyX[showParty-1][0][curRoom][curDungeon] + partyY[showParty-1][0][curRoom][curDungeon])
			for (i=0; i < 6; i++)
				if (partyArray[i])
					tempIcon[partyX[showParty-1][i][curRoom][curDungeon]][partyY[showParty-1][i][curRoom][curDungeon]] = partyArray[i];
		break;

	case 0:
	case 5: //already done above
	default:
		break;
	}

	for (j=0; j < 11; j++) //Put out the main squares here.
		for (i=0; i < 11; i++)
			BitBlt(localhdc, i*32+288, j*32, 32, 32, roomdc,
				32*(tempIcon[i][j] % 0x20), 32*(tempIcon[i][j] / 0x20), SRCCOPY);

	if (showPushed)
	{
		for (i=0; i < showPushed; i++)
			if (whatTo[i][curRoom][curDungeon])
			{
				BitBlt(localhdc, toSquare1X[i][curRoom][curDungeon]*32+288, toSquare1Y[i][curRoom][curDungeon]*32,
					32, 32, roomdc,
					32*(whatTo[i][curRoom][curDungeon] % 0x20), 32*(whatTo[i][curRoom][curDungeon] / 0x20), SRCCOPY);
				BitBlt(localhdc, toSquare2X[i][curRoom][curDungeon]*32+288, toSquare2Y[i][curRoom][curDungeon]*32,
					32, 32, roomdc,
					32*(whatTo[i][curRoom][curDungeon] % 0x20), 32*(whatTo[i][curRoom][curDungeon] / 0x20), SRCCOPY);
			}
	}

	if (showSpoilers)
	{
		for (i=0; i < 8; i++) //the "from square"
			if (whatTo[i][curRoom][curDungeon])
			{
				TransparentBlt(localhdc, 288+32*fromSquareX[i][curRoom][curDungeon], 32*fromSquareY[i][curRoom][curDungeon],
					32, 32, leveldc, 208, 224, 16, 16, 0x000000);
				checkAry[fromSquareX[i][curRoom][curDungeon]][fromSquareY[i][curRoom][curDungeon]] = 1;
			}

		for (i=0; i < 8; i++) //the "to squares"
			if (whatTo[i][curRoom][curDungeon])
				TransparentBlt(localhdc, 288+32*toSquare1X[i][curRoom][curDungeon], 32*toSquare1Y[i][curRoom][curDungeon],
					32, 32, leveldc, 448 + 32 * checkAry[toSquare1X[i][curRoom][curDungeon]][toSquare1Y[i][curRoom][curDungeon]],
					448, 32, 32, 0x000000);
		//The horrid looking + 32 * expression is to say, if we already have a yellow square, put an orange inside.
		//Otherwise, put an orange on the outside.
		for (i=0; i < 8; i++)
			if (whatTo[i][curRoom][curDungeon])
				TransparentBlt(localhdc, 288+32*toSquare2X[i][curRoom][curDungeon], 32*toSquare2Y[i][curRoom][curDungeon],
					32, 32, leveldc, 448 + 32 * checkAry[toSquare2X[i][curRoom][curDungeon]][toSquare2Y[i][curRoom][curDungeon]],
					448, 32, 32, 0x000000);
	}
	
	//now text list of monsters
	if (roomTextSummary)
	{
	short monInRoom[MONSTERS] = {0};
	char roomString[300];
	char buffer[100];
	RECT rc;
	HDC hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);

	rc.left = 288;
	rc.top = 360;

	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 0));

	roomString[0] = 0;
	for (i=0; i < 21; i++)
	{
		temp = monsterType[i][curRoom][curDungeon];
		if (toMonster(temp) != -1)
			monInRoom[toMonster(temp)]++;
	}
	for (i=0; i < MONSTERS; i++)
		if (monInRoom[i])
		{
			sprintf(buffer, "%d %s", monInRoom[i], monsterName[i]);
			if (monInRoom[i] > 1)
				strcat(buffer, "s");
			strcat(buffer, "\n");
			strcat(roomString, buffer);
		}
	if (strlen(roomString))
		DrawText(hdc, roomString, strlen(roomString), &rc, DT_LEFT | DT_TOP);
	}

	for (i=0; i < 8; i++)
	{
		if (!whatTo[i][curRoom][curDungeon])
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_GRAYED);
		else if (i == 7) //ok, last must work if available
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_ENABLED);
		else if ((fromSquareX[i][curRoom][curDungeon] != fromSquareX[i+1][curRoom][curDungeon]) ||
			(fromSquareY[i][curRoom][curDungeon] != fromSquareY[i+1][curRoom][curDungeon]))
		//make sure the next push is a different square than this.
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_ENABLED);
		else //oops, same as previous, grey it
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_GRAYED);
	}

	if (syncLevelToRoom)
	{
		if (curLevel != roomLev[curRoom][curDungeon])
		{
			curLevel = roomLev[curRoom][curDungeon];
			PaintDunMap();
		}
	}
	adjHeader();
}

void ReadTheDungeons()
{
	short tempRoom[32][11];

	short i, j, k, l, temp;

	FILE * F = fopen("DUNGEON.DAT", "rb");

	for (l=0; l < 8; l++)
		for (k=0; k < 8; k++)
			for (j=0; j < 8; j++)
				for (i=0; i < 8; i++)
				{
					temp = fgetc(F);
					if ((temp <= 0xff) && (temp >= 0xf0))
						roomLev[temp-0xf0][l] = k;
					mainDun[i][j][k][l] = temp;
				}

	fclose(F);

	F = fopen("DUNGEON.CBT", "rb");

	for (l=0; l < 8; l++)
	{
		if (l == 1)
			l++;
		for (k=0; k < 16; k++)
		{
			//easier for my sanity to read in the rectangle then sort from there
			for (j=0; j < 11; j++)
				for (i=0; i < 32; i++)
					tempRoom[i][j] = fgetc(F);

			//first, the room basics
			for (j=0; j < 11; j++)
				for (i=0; i <11; i++)
					roomBase[i][j][k][l] = tempRoom[i][j];

			//now handle which squares change which squares to what
			for (i=0; i < 8; i++)
			{
				whatTo[i][k][l] = tempRoom[i+11][0];
				fromSquareX[i][k][l] = tempRoom[i+11][8];
				fromSquareY[i][k][l] = tempRoom[i+19][8];
				toSquare1X[i][k][l] = tempRoom[i+11][9];
				toSquare1Y[i][k][l] = tempRoom[i+19][9];
				toSquare2X[i][k][l] = tempRoom[i+11][10];
				toSquare2Y[i][k][l] = tempRoom[i+19][10];
			}

			//now who starts where
			for (i=0; i < 6; i++)
			{
				partyX[1][i][k][l] = tempRoom[i+11][2];
				partyY[1][i][k][l] = tempRoom[i+17][2];
				partyX[3][i][k][l] = tempRoom[i+11][1]; //U5 is counterclockwise but I copied lots of U4 code
				partyY[3][i][k][l] = tempRoom[i+17][2]; //so this makes it clockwise
				partyX[2][i][k][l] = tempRoom[i+11][3];
				partyY[2][i][k][l] = tempRoom[i+17][3];
				partyX[0][i][k][l] = tempRoom[i+11][4];
				partyY[0][i][k][l] = tempRoom[i+17][4];
			}

			//now the monsters
			for (i=0; i < 21; i++)
			{
				monsterType[i][k][l] = tempRoom[i+11][5];
				monsterX[i][k][l] = tempRoom[i+11][6];
				monsterY[i][k][l] = tempRoom[i+11][7];
			}
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

void checkPrevNextDun()
{
	if (curDungeon == 7)
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_NEXT, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_NEXT, MF_ENABLED);
	if (curDungeon == 0)
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_PREV, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_PREV, MF_ENABLED);
	checkPrevNextRoom(); //if room is reset to A, we need to grey out the "previous room" option
	checkPrevNextLvl(); //if level is reset to 0, we need to grey out the "previous level" option
}

void checkPrevNextLvl()
{
	if (curLevel == 7)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_DOWN, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_DOWN, MF_ENABLED);
	if (curLevel == 0)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_UP, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_UP, MF_ENABLED);
}


void checkPrevNextRoom()
{
	if (curRoom == 15)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_ENABLED);
	if (curRoom == 0)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_ENABLED);
	showPushed = newPushed * 8;
}

void adjHeader()
{
	char buffer[100];
	char buffer2[20];
	sprintf(buffer, "Ultima V Dungeon Surfer: %s, level %d", dunName[curDungeon], curLevel + 1);
	if (curDungeon == DESPISE)
		strcat(buffer, " (no rooms)");
	else
	{
		sprintf(buffer2, " room %d (L%d)", curRoom+1, roomLev[curRoom][curDungeon]+1);
		strcat(buffer, buffer2);
	}
	SetWindowText(hwnd, buffer);
}

void doRoomCheck()
{
	checkPrevNextLvl();
	checkPrevNextRoom();
	PaintRoomMap();
}

short toMonster(short icon)
{
	//This is probably not perfect. One room had kids etc. in it. So I'd need another case for that.
	if ((icon >= 0x80) && (icon <= 0xff))
	{
		return (icon - 0x80) / 4;
	}
	return -1;
}