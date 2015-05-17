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
void adjustRoomCheckmarks();
void checkTheParty();
short thisLevelWraps();
short openSpace(short x);
void initMenu();

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
long curDun = 0;
long curLev = 0;

long resetRoomA = 0;
long resetLvl1 = 0;

short syncLevelToRoom = 0;
short restrictRoom = 0;

short wrapType = 2;
short mainLabel = 1;
short hideMimic = 0;

long mouseDownX, mouseDownY;

short mainDun[8][8][8][8];
short roomBase[11][11][16][8];

long roomLev[16][8];

short monsterType[21][16][8];
short monsterX[21][16][8];
short monsterY[21][16][8];

short partyX[4][6][16][8];
short partyY[4][6][16][8];

#define DUNTRACKING 14
#define CHEST 0
#define HEAL_FOUNTAIN 1
#define POISON_FOUNTAIN 2
#define HURT_FOUNTAIN 3
#define VISIBLE_PIT 4
#define HIDDEN_PIT 5
#define BOMB_TRAP 6
#define POISON_FIELD 7
#define SLEEP_FIELD 8
#define LIGHTNING_FIELD 9
#define FIRE_FIELD 10
#define WRITING 11
#define CAVEIN 12
#define SECRET_DOOR 13

char plu[2][2] = { "", "s" };

short dunSpoil[14][8] = {0};
short dunIconVal[14] = {0x41, 0x51, 0x52, 0x53, 0x60, 0x61, 0x62, 0x80, 0x81, 0x82, 0x83, 0xb1, 0xc0, 0xd0};

char fountStr[3][7] = { "Heal", "Poison", "Hurt" };
char fieldStr[4][10] = { "Sleep", "Poison", "Fire", "Lightning" };
char pitStr[2][4] = { "Vis", "Hid" };

short dunTextSummary = 0;
short roomTextSummary = 0;
short showMonsters = 0;
short showSpoilers = 0;
short showParty = 0;
short newPushed = 0;
short showPushed[8] = {0};

short curDir = 0;

short whatTo[8][16][8];
short fromSquareX[8][16][8];
short fromSquareY[8][16][8];
short toSquare1X[8][16][8];
short toSquare1Y[8][16][8];
short toSquare2X[8][16][8];
short toSquare2Y[8][16][8];

short levSecret[8] = {0};
short levChest[8] = {0};

short levelWraps[8][8] = {0};

short partyArray[6] = { 0x14c, 0x144, 0x148, 0x140, 0x140, 0x140 };

char dunName[8][9] = { "Deceit", "Despise", "Destard", "Wrong", "Covetous", "Shame", "Hythloth", "Doom"};

#define MONSTERS 32
char monsterName[MONSTERS][13] = {
	"Sea Horse", "Squid", "Serpent", "Shark", "Giant Rat", "Bat", "Giant Spider", "Ghost",
	"Slime", "Gremlin", "Mimic", "Reaper", "Gazer", "TREASURE", "Gargoyle", "Insect Swarm",
	"Orc", "Skeleton", "Python", "Ettin", "Headless", "Wisp", "Daemon", "Dragon",
	"Sand Trap", "Troll", "FIELD", "WHIRLPOOL", "Mongbat", "Corpser", "Rot Worm", "Shadow Lord"
};  //technically treasure/field/whirlpool aren't monsters but I'd have to muck with array counts otherwise

short msgStart[8] = { 0, 1, -1, 2, 3, 7, 10, -1 };
char msgs[11][25] = { "BOTTOMLESS PIT", "THE MAZE OF LOST SOULS", "THE PRISON WRONG", "THE CRYPT", "UPPER CRYPTS", "LOWER CRYPTS", "DEBTORS ALLY", "DEEP", "DEEPER", "DEEPEST", "MOTHER LODE MAZE" };

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
			if (curDun != LOWORD(wparam) - ID_DUNGEON_DECEIT)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_UNCHECKED);
				curDun = LOWORD(wparam) - ID_DUNGEON_DECEIT;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				if (resetLvl1)
					curLev = 0;
				PaintRoomMap();
				checkPrevNextDun();
			}
			break;

		case ID_DUNGEON_PREV:
			if (curDun > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_UNCHECKED);
				curDun--;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				if (resetLvl1)
					curLev = 0;
				PaintRoomMap();
				checkPrevNextDun();
			}
			break;

		case ID_DUNGEON_NEXT:
			if (curDun < 7)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_UNCHECKED);
				curDun++;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				if (resetLvl1)
					curLev = 0;
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
			if (curLev != LOWORD(wparam)-ID_NAV_1)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
				curLev = LOWORD(wparam) - ID_NAV_1;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_CHECKED);
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

		case ID_OPTIONS_RESET_ROOM_1:
			resetRoomA = !resetRoomA;
			if (resetRoomA)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_1, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_1, MF_UNCHECKED);
			break;

		case ID_OPTIONS_RESET_LEVEL_1:
			resetLvl1 = !resetLvl1;
			if (resetLvl1)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_1, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_1, MF_UNCHECKED);
			break;

		case ID_NAV_UP:
			if (curLev > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
				curLev--;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_CHECKED);
				PaintDunMap();
				checkPrevNextLvl();
			}
			break;

		case ID_NAV_DOWN:
			if (curLev < 7)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
				curLev++;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_CHECKED);
				PaintDunMap();
				checkPrevNextLvl();
			}
			break;

		case ID_NAV_PREVRM:
			if (curRoom > 0)
				if (restrictRoom)
					if (curLev != roomLev[curRoom-1][curDun])
						break;
				curRoom--;
				PaintRoomMap();
				checkPrevNextRoom();
			break;

		case ID_NAV_NEXTRM:
			if (curRoom < 15)
			{
				if (restrictRoom)
					if (curLev != roomLev[curRoom+1][curDun])
						break;
				curRoom++;
				PaintRoomMap();
				checkPrevNextRoom();
			}
			break;

			//OPTIONS MENU ITEMS

		case ID_OPTIONS_SIZE_HALF:
		case ID_OPTIONS_SIZE_FULL:
		case ID_OPTIONS_SIZE_AUTO:
		case ID_OPTIONS_SIZE_BORDERED:
			wrapType = LOWORD(wparam) - ID_OPTIONS_SIZE_HALF;
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SIZE_HALF, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SIZE_FULL, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SIZE_AUTO, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
			PaintDunMap();
			break;

		case ID_OPTIONS_MAINMAP_LABEL:
			mainLabel = !mainLabel;
			if (mainLabel)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MAINMAP_LABEL, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MAINMAP_LABEL, MF_UNCHECKED);
			PaintDunMap();
			break;

		case ID_OPTIONS_DUNTEXTSUMMARY:
			dunTextSummary = !dunTextSummary;
			if (dunTextSummary)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_DUNTEXTSUMMARY, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_DUNTEXTSUMMARY, MF_UNCHECKED);
			PaintDunMap();
			break;

		case ID_OPTIONS_ROOMTEXTSUMMARY:
			roomTextSummary = !roomTextSummary;
			if (roomTextSummary)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ROOMTEXTSUMMARY, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ROOMTEXTSUMMARY, MF_UNCHECKED);
			PaintRoomMap();
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

		case ID_OPTIONS_OUTLINE_SPOILER_SQUARES:
			showSpoilers = !showSpoilers;
			if (showSpoilers)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_SPOILER_SQUARES, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_SPOILER_SQUARES, MF_UNCHECKED);
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

		case ID_OPTIONS_PSGS_1:
		case ID_OPTIONS_PSGS_2:
		case ID_OPTIONS_PSGS_3:
		case ID_OPTIONS_PSGS_4:
		case ID_OPTIONS_PSGS_5:
		case ID_OPTIONS_PSGS_6:
		case ID_OPTIONS_PSGS_7:
		case ID_OPTIONS_PSGS_8:
			temp = LOWORD(wparam) - ID_OPTIONS_PSGS_1;
			showPushed[temp] = 1 - showPushed[temp];
			{
				short i;
				for (i = 0; i < 8; i++)
					if (fromSquareX[temp][curRoom][curDun] == fromSquareX[i][curRoom][curDun])
						if (fromSquareY[temp][curRoom][curDun] == fromSquareY[i][curRoom][curDun])
							showPushed[i] = showPushed[temp];
			}
			adjustRoomCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_OUTLINE_CHANGED_SQUARES:
			newPushed = !newPushed;
			for (temp = 0; temp < 8; temp++)
				showPushed[temp] = newPushed;
			if (newPushed)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_CHANGED_SQUARES, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_CHANGED_SQUARES, MF_UNCHECKED);
			adjustRoomCheckmarks();
			PaintRoomMap();
			break;

		case ID_MINOR_NOGUY_2:
		case ID_MINOR_NOGUY_3:
		case ID_MINOR_NOGUY_4:
		case ID_MINOR_NOGUY_5:
		case ID_MINOR_NOGUY_6:
		case ID_MINOR_MAGE_2:
		case ID_MINOR_MAGE_3:
		case ID_MINOR_MAGE_4:
		case ID_MINOR_MAGE_5:
		case ID_MINOR_MAGE_6:
		case ID_MINOR_BARD_2:
		case ID_MINOR_BARD_3:
		case ID_MINOR_BARD_4:
		case ID_MINOR_BARD_5:
		case ID_MINOR_BARD_6:
		case ID_MINOR_FIGHTER_2:
		case ID_MINOR_FIGHTER_3:
		case ID_MINOR_FIGHTER_4:
		case ID_MINOR_FIGHTER_5:
		case ID_MINOR_FIGHTER_6:
			temp = LOWORD(wparam) - ID_MINOR_NOGUY_2;
			if (temp / 10) //This is kind of a bad hack. The IDs in u5map.h are 411-415, 421-415, etc.
				partyArray[temp % 10 + 1] = 0x13c + 4 * (temp / 10);
			else //It's one of the "Noguy" settings
				partyArray[temp % 10 + 1] = 0;
			checkTheParty();
			PaintRoomMap();
			break;
			//ABOUT MENU ITEMS

		case ID_ABOUT_BASICS:
			MessageBox(hwnd, "Ultima V Dungeon Surfer\n\
This application lets the player browse all the dungeons and rooms.\n\
It features options and accelerators to bypass the usual traps and grinds.\n\
Your party is customizable for fun, too.\n\
Bugs? schultz.andrew@sbcglobal.net", "Basics", MB_OK);
			break;

		case ID_ABOUT_THANKS:
			MessageBox(hwnd, "tk421.net was useful to direct me to obscure locations.\nThe Ultima V Codex was a big help.\nSo was GitHub.\nDosBox helped run U4 efficiently in a window.", "Thanks/References", MB_OK);
			break;

		case ID_ABOUT_REPO:
			ShellExecute(hwnd, "open", "https://github.com/andrewschultz/rpg-mapping-tools/",
				NULL, NULL, SW_SHOWNORMAL);
			break;

		case ID_ABOUT_REPO_THISAPP:
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
					if (thisLevelWraps())
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
					temp = mainDun[mouseDownX][mouseDownY][curLev][curDun];
					if ((temp >= 0xf0) && (temp <= 0xff))
					{
						curRoom = temp & 0xf;
						doRoomCheck();
					}

					if ((temp >= 0xb1) && (temp <= 0xb4)) //message
					{
						short temp2 = temp - 0xb1 + msgStart[curDun];
						MessageBox(hwnd, msgs[temp2], "Wall writing", MB_OK);

					}
				}
				if ((mouseUpX >= 18) && (mouseUpX < 40) && (mouseUpY < 22) && (showParty))
				{
					short i;
					mouseUpX /= 2;
					mouseUpX -= 9;
					mouseUpY /= 2;
					for (i=1; i < 6; i++)
						if ((partyX[curDir][i][curRoom][curDun] == mouseUpX) && (partyY[curDir][i][curRoom][curDun] == mouseUpY))
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

	initMenu();

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
	short temp = 0, temp2 = 0;
	RECT rc;
	HDC hdc = GetDC(hwnd);

	GetClientRect(hwnd, &rc);

	rc.left = 0;
	rc.top = 288;
	rc.right = 288;
	rc.bottom = 640;

	{//I suppose we could cheat here and StretchBlt Icon #0
		HBRUSH hbrush=CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &rc, hbrush);
		DeleteObject(hbrush);
		ReleaseDC(hwnd, hdc);
	}

	if (wrapType == 3)
	{
		for (j=0; j < 16; j++)
			for (i=0; i < 16; i++)
			{
				if ((i < 3) || (j < 3) || (j > 12) || (i > 12))
					temp = 1;
				else
				{
					temp = mainDun[(i+4)%8][(j+4)%8][curLev][curDun]; //clean this code up, as it's around twice
					if (!mainLabel)
						if ((temp >= 0xf0) && (temp <= 0xff))
							temp = 0xed;
				}
				BitBlt(localhdc, i*16, j*16, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
			}
		adjHeader();
		return;
	}

	for (j=0; j < 8; j++)
	{
		for (i=0; i < 8; i++)
		{
			temp = mainDun[i][j][curLev][curDun];

			if (!mainLabel)
				if ((temp >= 0xf0) && (temp <= 0xff))
					temp = 0xed;
			if (thisLevelWraps())
			{
				BitBlt(localhdc, i*16, j*16, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16, j*16+128, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16+128, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
			}
			else
				StretchBlt(localhdc, i*32, j*32, 32, 32, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), 16, 16, SRCCOPY);
		}
	}

	if (dunTextSummary)
	{
		char buffer[300] = "";
		char buffer2[100];
		short temp = 0;

		SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 0));

		sprintf(buffer, "%d chest%s, %d writings%s, %d secret door%s, %d cave-in%s.\n",
			dunSpoil[CHEST][curDun], plu[dunSpoil[CHEST][curDun]>1],
			dunSpoil[WRITING][curDun], plu[dunSpoil[WRITING][curDun]>1],
			dunSpoil[CAVEIN][curDun], plu[dunSpoil[CAVEIN][curDun]>1],
			dunSpoil[SECRET_DOOR][curDun], plu[dunSpoil[SECRET_DOOR][curDun]>1]
			);

		temp = dunSpoil[VISIBLE_PIT][curDun] + dunSpoil[HIDDEN_PIT][curDun];
		if (temp)
		{
			sprintf(buffer2, "%d pits%s: %d visible, %d invisible. ",
				temp, plu[temp>1], dunSpoil[VISIBLE_PIT][curDun], dunSpoil[HIDDEN_PIT][curDun]);
			strcat(buffer, buffer2);
		}
		else
			strcat(buffer, "No pits. ");

		if (dunSpoil[BOMB_TRAP][curDun])
		{
			sprintf(buffer2, "%d bomb trap%s.\n",
				dunSpoil[BOMB_TRAP][curDun], plu[dunSpoil[BOMB_TRAP][curDun]>1]);
			strcat(buffer, buffer2);
		}
		else
			strcat(buffer, "No bomb traps.\n");

		temp = 0;
		for (i=HEAL_FOUNTAIN; i <= HURT_FOUNTAIN; i++)
			temp += dunSpoil[i][curDun];

		if (temp == 0)
			strcat(buffer, "No fountains.\n");
		else
		{
			strcat(buffer, "Fountains:");
			for (i=HEAL_FOUNTAIN; i <= HURT_FOUNTAIN; i++)
				if (dunSpoil[i][curDun])
				{
					sprintf(buffer2, " %d %s", dunSpoil[i][curDun], fountStr[i-HEAL_FOUNTAIN]);
					strcat(buffer, buffer2);
				}
			strcat(buffer, "\n");
		}


		temp = 0;
		for (i=POISON_FIELD; i <= FIRE_FIELD; i++)
			temp += dunSpoil[i][curDun];

		if (temp == 0)
			strcat(buffer, "No fields.\n");
		else
		{
			strcat(buffer, "Fields:");
			for (i=POISON_FIELD; i <= FIRE_FIELD; i++)
				if (dunSpoil[i][curDun])
				{
					sprintf(buffer2, " %d %s%s", dunSpoil[i][curDun], fieldStr[i-POISON_FIELD], plu[dunSpoil[i][curDun]>1]);
					strcat(buffer, buffer2);
				}
			strcat(buffer, "\n");
		}
		DrawText(hdc, buffer, strlen(buffer), &rc, DT_LEFT | DT_TOP);


	}

	ReleaseDC(hwnd, hdc);

	adjHeader();
}

void PaintRoomMap()
{
	short i, j, temp, temp2;
	short tempIcon[11][11];
	short checkAry[11][11] = {0};

	RECT rc;
	HDC hdc = GetDC(hwnd);

	if (curDun == DESPISE)
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
			tempIcon[i][j] = roomBase[i][j][curRoom][curDun];
			if ((!hideMimic) && (roomBase[i][j][curRoom][curDun] == MIMIC)) //Show the mimic
				tempIcon[i][j] += 1;

	//now show monsters
	if (showMonsters)
	{
		for (i=0; i < 21; i++)
			if (monsterType[i][curRoom][curDun])
				tempIcon[monsterX[i][curRoom][curDun]][monsterY[i][curRoom][curDun]] = monsterType[i][curRoom][curDun] + 256;
	}


	//"smart" placement
	if (showParty == 5)
	{
		for (i=NORTH; i <= WEST; i++)
		{
			if (partyX[i][0][curRoom][curDun] + partyY[i][0][curRoom][curDun])
			{
				for (j=0; j < 6; j++)
					if (partyArray[j])
						tempIcon[partyX[i][j][curRoom][curDun]][partyY[i][j][curRoom][curDun]] = partyArray[j];
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
		if (partyX[showParty-1][0][curRoom][curDun] + partyY[showParty-1][0][curRoom][curDun])
			for (i=0; i < 6; i++)
				if (partyArray[i])
					tempIcon[partyX[showParty-1][i][curRoom][curDun]][partyY[showParty-1][i][curRoom][curDun]] = partyArray[i];
		break;

	case 0:
	case 5: //already done above
	default:
		break;
	}

	for (j=0; j < 11; j++) //Put out the main squares here.
		for (i=0; i < 11; i++)
		{
			if ((i == 5) && (j == 3))
			{ temp = 5; }
			BitBlt(localhdc, i*32+288, j*32, 32, 32, roomdc,
				32*(tempIcon[i][j] % 0x20), 32*(tempIcon[i][j] / 0x20), SRCCOPY);
		}

	for (i=0; i < 8; i++)
		if (showPushed[i] && whatTo[i][curRoom][curDun])
		{
			BitBlt(localhdc, toSquare1X[i][curRoom][curDun]*32+288, toSquare1Y[i][curRoom][curDun]*32,
				32, 32, roomdc,
				32*(whatTo[i][curRoom][curDun] % 0x20), 32*(whatTo[i][curRoom][curDun] / 0x20), SRCCOPY);
			BitBlt(localhdc, toSquare2X[i][curRoom][curDun]*32+288, toSquare2Y[i][curRoom][curDun]*32,
				32, 32, roomdc,
				32*(whatTo[i][curRoom][curDun] % 0x20), 32*(whatTo[i][curRoom][curDun] / 0x20), SRCCOPY);
		}

	if (showSpoilers)
	{
		for (i=0; i < 8; i++) //the "from square"
			if (whatTo[i][curRoom][curDun])
			{
				TransparentBlt(localhdc, 288+32*fromSquareX[i][curRoom][curDun], 32*fromSquareY[i][curRoom][curDun],
					32, 32, leveldc, 208, 224, 16, 16, 0x000000);
				checkAry[fromSquareX[i][curRoom][curDun]][fromSquareY[i][curRoom][curDun]] = 1;
			}

		for (i=0; i < 8; i++) //the "to squares"
			if (whatTo[i][curRoom][curDun])
				TransparentBlt(localhdc, 288+32*toSquare1X[i][curRoom][curDun], 32*toSquare1Y[i][curRoom][curDun],
					32, 32, leveldc, 448 + 32 * checkAry[toSquare1X[i][curRoom][curDun]][toSquare1Y[i][curRoom][curDun]],
					448, 32, 32, 0x000000);
		//The horrid looking + 32 * expression is to say, if we already have a yellow square, put an orange inside.
		//Otherwise, put an orange on the outside.
		for (i=0; i < 8; i++)
			if (whatTo[i][curRoom][curDun])
				TransparentBlt(localhdc, 288+32*toSquare2X[i][curRoom][curDun], 32*toSquare2Y[i][curRoom][curDun],
					32, 32, leveldc, 448 + 32 * checkAry[toSquare2X[i][curRoom][curDun]][toSquare2Y[i][curRoom][curDun]],
					448, 32, 32, 0x000000);
	}
	
	//now text list of monsters

	{//I suppose we could cheat here and StretchBlt Icon #0
		HBRUSH hbrush=CreateSolidBrush(RGB(0, 0, 0));

		rc.left = 288;
		rc.top = 352;
		rc.bottom = 560;
		rc.right = 640;

		FillRect(hdc, &rc, hbrush);
		DeleteObject(hbrush);
		ReleaseDC(hwnd, hdc);
	}
	if (roomTextSummary)
	{
		short foundSecret = 0;
		short needComma = 0;
		short monInRoom[MONSTERS] = {0};
		char roomString[300];
		char buffer[100];

		GetClientRect(hwnd, &rc);

		rc.left = 288;
		rc.top = 352;

		SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 0));

		roomString[0] = 0;
		for (i=0; i < 21; i++) // first we get the monsters
		{
			temp = monsterType[i][curRoom][curDun];
			if (toMonster(temp) != -1)
				monInRoom[toMonster(temp)]++;
		}
		for (i=0; i < MONSTERS; i++)
			if (monInRoom[i])
			{
				if (needComma)
					strcat(roomString, ", ");
				else
					needComma = 1;
				sprintf(buffer, "%d %s", monInRoom[i], monsterName[i]);
				if (monInRoom[i] > 1)
					strcat(buffer, "s");
				strcat(roomString, buffer);
			}

		if (needComma)
			strcat(roomString, ". ");
		else
			strcat(roomString, "No monsters. ");

		temp = temp2 = 0;
		for (i=0; i < 21; i++)
		{
			if (monsterType[i][curRoom][curDun] == 0x101)
				temp++;
			if ((monsterType[i][curRoom][curDun] >= 0x102) && (monsterType[i][curRoom][curDun] <= 0x10f))
				temp2++;

		}

		if (temp + temp2)
			sprintf(buffer, "%d treasure: %d%s chest, %d misc\n", temp+temp2, temp, plu[temp!=1], temp2);
		else
			sprintf(buffer, "No chests or misc items.\n");

		strcat(roomString, buffer);

		temp = 0;
		for (i=0; i < 8; i++)
			if (fromSquareX[i][curRoom][curDun] + fromSquareY[i][curRoom][curDun])
			{
				foundSecret = 1;
				temp2 = fromSquareX[i][curRoom][curDun] + 16*fromSquareY[i][curRoom][curDun];
				if (temp2 != temp)
				{
					if (temp)
						strcat(roomString, "\n");
					sprintf(buffer, "(%d, %d):", fromSquareX[i][curRoom][curDun], fromSquareY[i][curRoom][curDun]);
					strcat(roomString, buffer);
				}
				sprintf(buffer, " (%d, %d)", toSquare1X[i][curRoom][curDun], toSquare1Y[i][curRoom][curDun]);
				strcat(roomString, buffer);
				if (toSquare2X[i][curRoom][curDun] + toSquare2Y[i][curRoom][curDun])
					if (toSquare2X[i][curRoom][curDun] + 16 * toSquare2Y[i][curRoom][curDun] !=
						toSquare1X[i][curRoom][curDun] + 16 * toSquare1Y[i][curRoom][curDun])
					{
						sprintf(buffer, " (%d, %d)", toSquare2X[i][curRoom][curDun], toSquare2Y[i][curRoom][curDun]);
						strcat(roomString, buffer);
					}
				temp = temp2;
			}

		if (temp)
			strcat(roomString, "\n");

		if (!foundSecret)
			strcat(roomString, "No secret squares to push.\n");

		if (strlen(roomString))
			DrawText(hdc, roomString, strlen(roomString), &rc, DT_LEFT | DT_TOP | DT_WORDBREAK);
	}

	for (i=0; i < 8; i++)
	{
		if (!whatTo[i][curRoom][curDun])
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_GRAYED);
		else
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_ENABLED);
	}

	if (syncLevelToRoom)
	{
		if (curLev != roomLev[curRoom][curDun])
		{
			curLev = roomLev[curRoom][curDun];
			PaintDunMap();
		}
	}
	adjHeader();

	adjustRoomCheckmarks();
}

//This lumps together stuff like similar walls
short iconRedir (int x)
{
	if ((x >= 0xb2) && (x <= 0xb4))
		return 0xb1;
	if ((x == 0x68) || (x == 0x69))
		return x - 8;
	return x;
}

void spoilDungeon(short thisDun)
{
	char buffer[400] = "";
	char buffer2[100] = "";
	short i, temp = 0;
	short needComma = 0;
	
	RECT rc;
	HDC hdc = GetDC(hwnd);
	
	GetClientRect(hwnd, &rc);

	rc.left = 0;
	rc.right = 8 * ICONSIZE;
	rc.top = 288;
	rc.bottom = 480;

	if (dunTextSummary == 0)
	{//I suppose we could cheat here and StretchBlt Icon #0
		HBRUSH hbrush=CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &rc, hbrush);
		DeleteObject(hbrush);
		ReleaseDC(hwnd, hdc);
		return;
	}
	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 0));

	sprintf(buffer, "%s info: %d chests, %d secret doors, %d writing, %d cave-ins\n", dunName[thisDun], dunSpoil[CHEST][thisDun],
		dunSpoil[SECRET_DOOR][thisDun], dunSpoil[WRITING][thisDun], dunSpoil[CAVEIN][thisDun]);
	
	temp = 0;
	for (i=POISON_FIELD; i <= FIRE_FIELD; i++)
		temp += dunSpoil[i][thisDun];
	
	if (temp)
	{
		strcat(buffer, "Fields: ");
		for (i=POISON_FIELD; i <= FIRE_FIELD; i++)
			if (dunSpoil[i][thisDun])
				sprintf(buffer, " %d %s", dunSpoil[i][thisDun], fieldStr[i - POISON_FIELD]);
		strcat(buffer, ".\n");
	}
	else
		strcat(buffer, "No magic fields.\n");
	
	temp = dunSpoil[VISIBLE_PIT][thisDun] + dunSpoil[HIDDEN_PIT][thisDun];
	
	if (temp)
	{
		strcat(buffer, "Pits: ");
		for (i=VISIBLE_PIT; i <= HIDDEN_PIT; i++)
			if (dunSpoil[i][thisDun])
				sprintf(buffer2, " %d %s", dunSpoil[i][thisDun], pitStr[i - VISIBLE_PIT]);
		strcat(buffer, buffer2);
	}
	else
		strcat(buffer, "No pits.\n");
	
	for (i=HEAL_FOUNTAIN; i < HURT_FOUNTAIN; i++)
		temp += dunSpoil[i][thisDun];
	
	if (temp)
	{
		strcat(buffer, "Fountains: ");
		for (i=HEAL_FOUNTAIN; i <= HURT_FOUNTAIN; i++)
			if (dunSpoil[i][thisDun])
				sprintf(buffer2, " %d %s", dunSpoil[i][thisDun], fountStr[i - HEAL_FOUNTAIN]);
		strcat(buffer, buffer2);
	}
	else
		strcat(buffer, "No fountains.\n");

	DrawText(hdc, buffer, strlen(buffer), &rc, DT_LEFT | DT_TOP);
	ReleaseDC(hwnd, hdc);
}

void ReadTheDungeons()
{
	short tempRoom[32][11];

	short i, j, k, l, q, temp;

	FILE * F = fopen("DUNGEON.DAT", "rb");

	if (F == NULL)
	{
		MessageBox(hwnd, "You are missing Dungeon.dat. The dungeon surfer needs it to work.", "Oops!", MB_OK);
		return;
	}

	for (l=0; l < 8; l++)
		for (k=0; k < 8; k++)
		{
			levelWraps[k][l] = 0;
			for (j=0; j < 8; j++)
				for (i=0; i < 8; i++)
				{
					temp = fgetc(F);

					for (q=0; q < DUNTRACKING; q++)
					{
						if (iconRedir(temp) == dunIconVal[q])
							dunSpoil[q][l]++;
					}

					if (openSpace(temp))
					{
						if ((i == 0) || (i == 7))
							levelWraps[k][l] = 1;
						if ((j == 0) || (j == 7))
							levelWraps[k][l] = 1;
					}
					if ((temp <= 0xff) && (temp >= 0xf0))
					{
						if (!roomLev[temp-0xf0][l])
							roomLev[temp-0xf0][l]= k;
						else if (roomLev[temp-0xf0][l] % 16 != k)
						{
							roomLev[temp-0xf0][l] <<= 4;
							roomLev[temp-0xf0][l] += k;
						}
					}
					mainDun[i][j][k][l] = temp;
				}
		}

	fclose(F);

	F = fopen("DUNGEON.CBT", "rb");

	if (!F)
	{
		MessageBox(hwnd, "You are missing Dungeon.CBT, a necessary file from Ultima V. The Surfer will not work without it.",
			"Oh no!", MB_OK);
		return;
	}

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
				partyX[NORTH][i][k][l] = tempRoom[i+11][4];
				partyY[NORTH][i][k][l] = tempRoom[i+17][4];
				partyX[EAST][i][k][l] = tempRoom[i+11][1];
				partyY[EAST][i][k][l] = tempRoom[i+17][1];
				partyX[SOUTH][i][k][l] = tempRoom[i+11][3];
				partyY[SOUTH][i][k][l] = tempRoom[i+17][3];
				partyX[WEST][i][k][l] = tempRoom[i+11][2];
				partyY[WEST][i][k][l] = tempRoom[i+17][2];
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
	if (curDun == 7)
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_NEXT, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_NEXT, MF_ENABLED);
	if (curDun == 0)
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_PREV, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_DUNGEON_PREV, MF_ENABLED);
	checkPrevNextRoom(); //if room is reset to A, we need to grey out the "previous room" option
	checkPrevNextLvl(); //if level is reset to 0, we need to grey out the "previous level" option
}

void checkPrevNextLvl()
{
	if (curLev == 7)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_DOWN, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_DOWN, MF_ENABLED);
	if (curLev == 0)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_UP, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_UP, MF_ENABLED);
}


void checkPrevNextRoom()
{
	short i;

	if (curRoom == 15)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_ENABLED);
	if (curRoom == 0)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_GRAYED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_ENABLED);
	for (i=0; i < 8; i++)
		showPushed[i] = newPushed;
}

void adjHeader()
{
	char buffer[100];
	char buffer2[30];
	char buffer3[30];

	sprintf(buffer, "Ultima V Dungeon Surfer: %s, level %d ", dunName[curDun], curLev + 1);
	if (curDun == DESPISE)
		strcat(buffer, " (no rooms)");
	else
	{
		if (roomLev[curRoom][curDun] <= 8)
		{
			sprintf(buffer2, "room %d (L%d)", curRoom+1, roomLev[curRoom][curDun]+1);
		}
		else
		{
			long oddTemp = roomLev[curRoom][curDun];
			short i;

			buffer2[0] = 0;

			for (i=7; i >= 0; i--)
				if (oddTemp >> (4*i))
					if (!buffer2[0])
						sprintf(buffer2, "room %d (%d", curRoom+1, oddTemp >> (4*i));
					else
					{
						sprintf(buffer3, ", %d", (oddTemp>>(4*i)) & 0xf);
						strcat(buffer2, buffer3);
					}

			strcat(buffer2, ")");
		}
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

void adjustRoomCheckmarks()
{
	short i;

	for (i=0; i < 8; i++)
	{
		if (showPushed[i] && (fromSquareX[i][curRoom][curDun] + fromSquareY[i][curRoom][curDun]))
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_CHECKED);
		else
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PSGS_1 + i, MF_UNCHECKED);
	}

}

void checkTheParty()
{
	short i;
	short temp;

	short foundAnyone = 0;
	short missedAnyone = 0;

	for (i=1; i < 6; i++)
	{
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_NOGUY_2 + i - 1, MF_UNCHECKED);  //Vacant
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_NOGUY_2 + i + 9, MF_UNCHECKED);  //Mage
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_NOGUY_2 + i + 19, MF_UNCHECKED); //Bard
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_NOGUY_2 + i + 29, MF_UNCHECKED); //Fighter

		if (!partyArray[i])
			temp = 0;
		else
			temp = ((partyArray[i] - 0x13c) / 4) * 10;

		temp += ID_MINOR_NOGUY_2 + i - 1;

		CheckMenuItem( GetMenu(hwnd), temp, MF_CHECKED);

		if (partyArray[i] == 0)
			missedAnyone = 1;
		if (partyArray[i] != 0)
			foundAnyone = 1;
	}

	if (missedAnyone)
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_UNCHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_CHECKED);

	if (foundAnyone)
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_UNCHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_CHECKED);
}

short thisLevelWraps()
{
	if (wrapType == 0)
		return 1;
	if (wrapType == 1)
		return 0;
	if (levelWraps[curRoom][curDun])
		return 1;
	return 0;
}

short openSpace(short x)
{
	if (x == 0xb0)
		return 0;
	if ((x <= 0xc3) && (x >= 0xc0))
		return 0;
	return 1;
}

void initMenu()
{
	CheckMenuItem( GetMenu(hwnd), wrapType + ID_OPTIONS_SIZE_HALF, MF_CHECKED);

	CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE, MF_UNCHECKED);

	checkTheParty();
	if (mainLabel)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MAINMAP_LABEL, MF_CHECKED);

	if (resetRoomA)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_1, MF_CHECKED);

}