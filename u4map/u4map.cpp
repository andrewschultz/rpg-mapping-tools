// u4map.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN  

#include <ddraw.h>
#include <windowsx.h>
#include <stdio.h>
#include <math.h>
#include <commdlg.h>

#include <stdlib.h>

#include <shellapi.h>

#include "u4map.h"

//#defines for Windows plus function calls
#define WINDOW_CLASS_NAME "WINCLASS1"

void ReadTheDungeons();
void PaintDunMap();
void PaintRoomMap();
void readDun(char x[20], int q);
short curDirValid();
void adjustSecretCheckmarks();
void adjHeader();
void DungeonReset();
void doRoomCheck();
short toMonster(short icon);
void rlsDebug(char x[50]);

//#defines for in-app use
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

#define DECEIT 0
#define DESPISE 1
#define DESTARD 2
#define WRONG 3
#define COVETOUS 4
#define SHAME 5
#define HYTHLOTH 6
#define ABYSS 7

#define MIMIC 0xac

#define MONSTERS 52

//###################globals

HWND hwnd;

HDC roomdc;
HDC leveldc;

HDC localhdc;

long curDungeon = 0;
long curLevel = 0;
long curRoom = 0;
long showMonsters = 0;
long showParty = 0;
long altIcon = 0;
long showSpoilers = 0;
long mainLabel = 0;
short roomTextSummary = 0;

short curDir = 0; //?? what about when things are turned off

short hideMimic = 0;

short wrapHalf = 0;

short resetRoomA = 0;
short resetLevel0 = 0;

short syncLevelToRoom = 0;
short restrictRoom = 0;

short mainDun[8][8][8][8] = {0};

short roomMap[11][11][64][8] = {0};

short roomMonster[16][64][8][8] = {0};

short monsterType[16][64][8] = {0};
short monsterX[16][64][8] = {0};
short monsterY[16][64][8] = {0};

short partyX[4][8][64][8] = {0};
short partyY[4][8][64][8] = {0};

short changeByte[16][64][8] = {0};

short roomLev[64][8] = {0};

short showPath[4] = {0};

char dunName[8][9] = { "Deceit", "Despise", "Destard", "Wrong", "Covetous", "Shame", "Hythloth", "Abyss"};

char monsterName[52][13] = { "Mage", "Bard", "Fighter", "Druid", "Tinker", "Paladin", "Ranger", "Shepherd",
	"Guard", "Merchant", "Bard(2)", "Jester", "Beggar", "Child", "Bull", "Lord British",
	"Pirate ship", "Pirate ship", "Nixie", "Squid", "Serpent", "Seahorse", "Whirlpool", "Twister",
	"Rat", "Bat", "Spider", "Ghost", 
	"Slime", "Troll", "Gremlin", "Mimic",
	"Reaper", "Insects", "Gazer", "Phantom",
	"Orc", "Skeleton", "Rogue", "Python",
	"Ettin", "Headless", "Cyclops", "Wisp",
	"Mage", "Liche", "Lava Lizard", "Zorn",
	"Daemon", "Hydra", "Dragon", "Balron"
};

//Mage down to shepherd. You start as a mage, for simplicity.
short slotIcon[8] = { 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e };
short slotShow[8] = {1, 1, 1, 1, 1, 1, 1, 1};

long mouseDownX, mouseDownY;

#define ICONSIZE 32

#define PARTY_NONE 0
#define PARTY_NORTH 1
#define PARTY_EAST 2
#define PARTY_SOUTH 3
#define PARTY_WEST 4

LRESULT CALLBACK WindowProc(HWND hwnd, 
						    UINT msg, 
                            WPARAM wparam, 
                            LPARAM lparam)
{
	short temp, temp2, i;

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
		case ID_DUNGEON_ABYSS:
			if (curDungeon != LOWORD(wparam) - ID_DUNGEON_DECEIT)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon = LOWORD(wparam) - ID_DUNGEON_DECEIT;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				DungeonReset();
			}
			break;

		case ID_DUNGEON_PREV:
			if (curDungeon > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon--;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				DungeonReset();
			}
			break;

		case ID_DUNGEON_NEXT:
			if (curDungeon < ABYSS)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon++;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				DungeonReset();
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

		case ID_NAV_SHIFT_A:
		case ID_NAV_SHIFT_B:
		case ID_NAV_SHIFT_C:
		case ID_NAV_SHIFT_D:
		case ID_NAV_SHIFT_E:
		case ID_NAV_SHIFT_F:
		case ID_NAV_SHIFT_G:
		case ID_NAV_SHIFT_H:
		case ID_NAV_SHIFT_I:
		case ID_NAV_SHIFT_J:
		case ID_NAV_SHIFT_K:
		case ID_NAV_SHIFT_L:
		case ID_NAV_SHIFT_M:
		case ID_NAV_SHIFT_N:
		case ID_NAV_SHIFT_O:
		case ID_NAV_SHIFT_P:
		case ID_NAV_ALT_A:
		case ID_NAV_ALT_B:
		case ID_NAV_ALT_C:
		case ID_NAV_ALT_D:
		case ID_NAV_ALT_E:
		case ID_NAV_ALT_F:
		case ID_NAV_ALT_G:
		case ID_NAV_ALT_H:
		case ID_NAV_ALT_I:
		case ID_NAV_ALT_J:
		case ID_NAV_ALT_K:
		case ID_NAV_ALT_L:
		case ID_NAV_ALT_M:
		case ID_NAV_ALT_N:
		case ID_NAV_ALT_O:
		case ID_NAV_ALT_P:
		case ID_NAV_ALT_SHIFT_A:
		case ID_NAV_ALT_SHIFT_B:
		case ID_NAV_ALT_SHIFT_C:
		case ID_NAV_ALT_SHIFT_D:
		case ID_NAV_ALT_SHIFT_E:
		case ID_NAV_ALT_SHIFT_F:
		case ID_NAV_ALT_SHIFT_G:
		case ID_NAV_ALT_SHIFT_H:
		case ID_NAV_ALT_SHIFT_I:
		case ID_NAV_ALT_SHIFT_J:
		case ID_NAV_ALT_SHIFT_K:
		case ID_NAV_ALT_SHIFT_L:
		case ID_NAV_ALT_SHIFT_M:
		case ID_NAV_ALT_SHIFT_N:
		case ID_NAV_ALT_SHIFT_O:
		case ID_NAV_ALT_SHIFT_P:
			if (curDungeon == ABYSS)
			{
				curRoom = LOWORD(wparam) - ID_NAV_A;
				doRoomCheck();
			}
			break;

		case ID_NAV_ABYSS_UP2:
			if ((curDungeon == ABYSS) && (curRoom >= 16))
			{
				curRoom -= 16;
				doRoomCheck();
			}
			break;

		case ID_NAV_ABYSS_DOWN2:
			if ((curDungeon == ABYSS) && (curRoom <= 47))
			{
				curRoom += 16;
				doRoomCheck();
			}
			break;

		case ID_OPTIONS_RESET_ROOM_A:
			resetRoomA = !resetRoomA;
			if (resetRoomA)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_A, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_A, MF_UNCHECKED);
			break;

		case ID_OPTIONS_RESET_LEVEL_0:
			resetLevel0 = !resetLevel0;
			if (resetLevel0)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_0, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_0, MF_UNCHECKED);
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
			if (curLevel < ABYSS)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_UNCHECKED);
				curLevel++;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLevel, MF_CHECKED);
				PaintDunMap();
			}
			break;

		case ID_NAV_PREVRM:
			if (curRoom > 0)
				if (restrictRoom)
					if (curLevel != roomLev[curRoom-1][curDungeon])
						break;
				curRoom--;
				doRoomCheck();
			break;

		case ID_NAV_NEXTRM:
			if (curRoom < 15)
			{
				if (restrictRoom)
					if (curLevel != roomLev[curRoom+1][curDungeon])
						break;
				curRoom++;
				doRoomCheck();
			}
			else if ((curRoom < 63) && (curDungeon == ABYSS))
			{
				if (restrictRoom)
					if (curLevel != roomLev[curRoom-1][curDungeon])
						break;
				curRoom++; //Abyss has four times the rooms of the other dungeons.
				doRoomCheck();
			}
			break;

			//OPTIONS MENU ITEMS

		case ID_OPTIONS_SPOIL:
			showSpoilers = !showSpoilers;
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

		case ID_OPTIONS_TEXTSUMMARY:
			roomTextSummary = !roomTextSummary;
			if (roomTextSummary)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_TEXTSUMMARY, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_TEXTSUMMARY, MF_UNCHECKED);
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
			if (showParty)
				curDir = showParty - 1;
			PaintRoomMap();
			break;

		case ID_OPTIONS_ALT_ICONS:
			altIcon = !altIcon;
			if (altIcon)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ALT_ICONS, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_ALT_ICONS, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_OPTIONS_REVEAL_ALL_SECRET:
			showPath[0] = showPath[1] = showPath[2] = showPath[3] = 1;
			adjustSecretCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_HIDE_ALL_SECRET:
			showPath[0] = showPath[1] = showPath[2] = showPath[3] = 0;
			adjustSecretCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_SHOW_1ST_SECRET:
		case ID_OPTIONS_SHOW_2ND_SECRET:
		case ID_OPTIONS_SHOW_3RD_SECRET:
		case ID_OPTIONS_SHOW_4TH_SECRET:
			temp = LOWORD(wparam)-ID_OPTIONS_SHOW_1ST_SECRET;
			showPath[temp] = !showPath[temp];
			for (i=0; i < 4; i++)
				if (changeByte[4*i+1][curRoom][curDungeon] == changeByte[4*temp+1][curRoom][curDungeon])
					showPath[i] = showPath[temp];
			adjustSecretCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_WRAPHALF:
			wrapHalf = !wrapHalf;
			if (wrapHalf)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_UNCHECKED);
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
			
			//MINOR/SILLY OPTIONS
		case ID_MINOR_SWAP_2:
		case ID_MINOR_SWAP_3:
		case ID_MINOR_SWAP_4:
		case ID_MINOR_SWAP_5:
		case ID_MINOR_SWAP_6:
		case ID_MINOR_SWAP_7:
		case ID_MINOR_SWAP_8:
			temp = slotIcon[0]; //switch player 1's class with player (SWAP#). ID_MINOR_SWAP_1
			slotIcon[0] = slotIcon[LOWORD(wparam)-ID_MINOR_SWAP_1];
			slotIcon[LOWORD(wparam)-ID_MINOR_SWAP_1] = temp;
			break;
			
		case ID_MINOR_HIDE_2:
		case ID_MINOR_HIDE_3:
		case ID_MINOR_HIDE_4:
		case ID_MINOR_HIDE_5:
		case ID_MINOR_HIDE_6:
		case ID_MINOR_HIDE_7:
		case ID_MINOR_HIDE_8:
			temp = LOWORD(wparam)-ID_MINOR_HIDE_1;
			slotShow[temp] = !slotShow[temp];
			temp2 = 0;
			for (i=1; i < 7; i++)
				temp2 += slotShow[i];
			if (slotShow[temp])
			{
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_UNCHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_UNCHECKED);
				if (temp2 == 7)
					CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_CHECKED);
				else
					CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_UNCHECKED);
			}
			else
			{
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_UNCHECKED);
				if (temp2 == 0)
					CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_CHECKED);
				else
					CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_UNCHECKED);
			}
			PaintRoomMap();
			break;

		case ID_MINOR_HIDE_ALL:
			for (temp=1; temp <= 7; temp++)
			{
				slotShow[temp] = 0;
				CheckMenuItem( GetMenu(hwnd), temp + ID_MINOR_HIDE_1, MF_CHECKED);
			}
			CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_CHECKED);
			CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_MINOR_HIDE_NONE:
			for (temp=1; temp <= 7; temp++)
			{
				slotShow[temp] = 1;
				CheckMenuItem( GetMenu(hwnd), temp + ID_MINOR_HIDE_1, MF_UNCHECKED);
			}
			CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_ALL, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_CHECKED);
			PaintRoomMap();
			break;

			//ABOUT MENU ITEMS

		case ID_ABOUT_BASICS:
			MessageBox(hwnd, "Ultima IV Dungeon Browser\n\
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

		case ID_ABOUT_REPO_U4:
			ShellExecute(hwnd, "open", "https://github.com/andrewschultz/rpg-mapping-tools/tree/master/u5map",
				NULL, NULL, SW_SHOWNORMAL);
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

				if (showParty)
					if ((mouseDownX >= 18) && (mouseDownX < 40) && (mouseDownY < 22))
						if ((mouseUpX >= 18) && (mouseUpX < 40) && (mouseUpY < 22))
						{
							short froms = 0, tos = 0;
							mouseDownX -= 18;
							mouseDownX /= 2;
							mouseDownY /= 2;
							mouseUpX -= 18;
							mouseUpX /= 2;
							mouseUpY /= 2;

							for (temp = 0; temp < 8; temp++)
							{
								if (partyX[curDir][temp][curRoom][curLevel] == mouseDownX)
									if (partyY[curDir][temp][curRoom][curLevel] == mouseDownY)
										froms = temp;
								if (partyX[curDir][temp][curRoom][curLevel] == mouseUpX)
									if (partyY[curDir][temp][curRoom][curLevel] == mouseUpY)
										tos = temp;
							}

							if (froms != tos)
							{
								temp = slotIcon[froms];
								slotIcon[froms] = slotIcon[tos];
								slotIcon[tos] = temp;
							}
							doRoomCheck();
							break;

						}

				if ((mouseDownX < 16) && (mouseDownY < 16))
				{
				if (wrapHalf)
				{//this allows us to move from 1 quadrant to another
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
				if (mouseDownX == mouseUpX)
					if (mouseDownY == mouseUpY)
					{
						temp = mainDun[mouseDownX][mouseDownY][curLevel][curDungeon];
						if ((temp >= 0xd0) && (temp <= 0xdf))
						{
							curRoom = temp & 0xf;
							doRoomCheck();
						}
						if ((temp % 16 >.8) && (temp <= 0x7f))
						{
							curRoom = (temp & 0x7) + 8 * (temp / 0x10);
							doRoomCheck();
						}
				}
				}
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
							  "Ultima 4 Dungeon Simulator",	     // title
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

	roomdc = CreateCompatibleDC(localhdc);
	leveldc = CreateCompatibleDC(localhdc);

	HBITMAP oldroom = (HBITMAP)SelectObject(roomdc, roombmp);
	HBITMAP oldlevel = (HBITMAP)SelectObject(leveldc, levelbmp);

	ReadTheDungeons();
	DungeonReset();

    hAccelTable = LoadAccelerators(hInstance, "MYACCEL");

	CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_ALL_SECRET, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE, MF_UNCHECKED);

	CheckMenuItem( GetMenu(hwnd), ID_MINOR_HIDE_NONE, MF_CHECKED);

	if (resetRoomA)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_A, MF_CHECKED);

	if (resetLevel0)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_0, MF_CHECKED);

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

void readDun(char x[20], int q)
{
	short i, j, k, temp, temp2;
	int numDun = 16;
	FILE * F = fopen(x, "rb");

	if (q == ABYSS)
		numDun = 64;

	for (k=0; k < 8; k++)
		for (j=0; j < 8; j++)
			for (i=0; i < 8; i++)
			{
				temp = fgetc(F); //?? 0xd(0-9) & 0xf0 == 
				temp2 = temp;
				if ((temp >= 0xd0) && (temp <= 0xdf))
				{
					if (q == ABYSS) //in abyss, add 16 for each (level/2)
					{
						temp2 = temp % 0x10;
						if (k >= 2)
						{
							temp %= 0x10;
							if (temp >= 8)
								temp += 8;
							temp += 8;
							temp += (0x20 * (k/2));
							temp2 += 16 * (k / 2);
						}
					}

					roomLev[temp2][q] = k;
				}
				mainDun[i][j][k][q] = temp;
			}

	for (k=0; k < numDun; k++)
	{
		//This determines the special squares that change if you step on them
		for (i=0; i < 16; i++)
			changeByte[i][k][q] = fgetc(F);

		//Monster type, X and Y coords.
		for (i=0; i < 16; i++)
			monsterType[i][k][q] = fgetc(F);
		for (i=0; i < 16; i++)
			monsterX[i][k][q] = fgetc(F);
		for (i=0; i < 16; i++)
			monsterY[i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partyX[0][i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyY[0][i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partyX[1][i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyY[1][i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partyX[2][i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyY[2][i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partyX[3][i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyY[3][i][k][q] = fgetc(F);

		for (j=0; j < 11; j++)
			for (i=0; i < 11; i++)
				roomMap[i][j][k][q] = fgetc(F);
		fseek(F, 0x7, SEEK_CUR);
	}
	fclose(F);

}

void ReadTheDungeons()
{
	readDun("DECEIT.DNG", 0);
	readDun("DESPISE.DNG", 1);
	readDun("DESTARD.DNG", 2);
	readDun("WRONG.DNG", 3);
	readDun("COVETOUS.DNG", 4);
	readDun("SHAME.DNG", 5);
	readDun("HYTHLOTH.DNG", 6);
	readDun("ABYSS.DNG", ABYSS);
}

void PaintDunMap()
{
	int i, j;
	short temp;

	adjHeader();

	if (wrapHalf)
	{
		for (j=0; j < 8; j++)
			for (i=0; i < 8; i++)
			{
				temp = mainDun[i][j][curLevel][curDungeon];
				if (((temp >= 0xd0) && (temp <= 0xdf)) && (!mainLabel))
					temp = 0xfd; // rooms 1-16
				if (((temp % 16 >= 0x8) && (temp <= 0x7f)) && (!mainLabel))
					temp = 0xfd; // rooms 17-64
				BitBlt(localhdc, i*16, j*16, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16, j*16+128, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16+128, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
			}
		return;
	}

	for (j=0; j < 8; j++)
	{
		for (i=0; i < 8; i++)
		{
			temp = mainDun[i][j][curLevel][curDungeon];
			if (((temp >= 0xd0) && (temp <= 0xdf)) && (!mainLabel))
				temp = 0xfd;
			StretchBlt(localhdc, i*32, j*32, 32, 32, leveldc,
				16*(temp % 0x10), 16*(temp / 0x10), 16, 16, SRCCOPY);
		}
	}

}

void PaintRoomMap()
{
	int i, j, k, temp, temp2;
	short thisIcon[11][11] = {0};
	short changedYet[11][11] = {0};

	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			thisIcon[i][j] = roomMap[i][j][curRoom][curDungeon];

	if (showMonsters)
	{
		for (i=0; i < 16; i++)
			if (monsterType[i][curRoom][curDungeon])
			{
				if (monsterX[i][curRoom][curDungeon] > 0xa)
					k=0;
				if (monsterY[i][curRoom][curDungeon] > 0xa)
					k=0;
				thisIcon[monsterX[i][curRoom][curDungeon]][monsterY[i][curRoom][curDungeon]] = monsterType[i][curRoom][curDungeon];
				if ((!hideMimic) && (monsterType[i][curRoom][curDungeon] == MIMIC))
					thisIcon[monsterX[i][curRoom][curDungeon]][monsterY[i][curRoom][curDungeon]]++;
			}
	}

	// "smart" party placement
	if (showParty == 5)
	{
		for (i=NORTH; i <= WEST; i++)
		{
			if (partyX[i][0][curRoom][curDungeon] + partyY[i][0][curRoom][curDungeon])
			{
				for (j=0; j < 8; j++)
					if (slotShow[j])
						thisIcon[partyX[i][j][curRoom][curDungeon]][partyY[i][j][curRoom][curDungeon]] = slotIcon[j];
				curDir = i;
				break;
			}
		}
		if (i == 4)
			MessageBox(hwnd, "Choosing a viable direction did not work! Please note the room and dungeon and report it at u4map's github site.", "Probable bug", MB_OK);
	}
	else
	//Do we show where the party is? Can they enter the room from this direction? If so, display all 8 characters.

	if (showParty && curDirValid())
	{
		for (i=0; i < 8; i++)
		{
			switch (showParty)
			{
			case PARTY_NORTH:
				if (slotShow[i])
					thisIcon[partyX[0][i][curRoom][curDungeon]][partyY[0][i][curRoom][curDungeon]] = slotIcon[i];
				break;
			case PARTY_SOUTH:
				if (slotShow[i])
					thisIcon[partyX[2][i][curRoom][curDungeon]][partyY[2][i][curRoom][curDungeon]] = slotIcon[i];
				break;
			case PARTY_EAST:
				if (slotShow[i])
					thisIcon[partyX[1][i][curRoom][curDungeon]][partyY[1][i][curRoom][curDungeon]] = slotIcon[i];
				break;
			case PARTY_WEST:
				if (slotShow[i])
					thisIcon[partyX[3][i][curRoom][curDungeon]][partyY[3][i][curRoom][curDungeon]] = slotIcon[i];
				break;
			}
		}
	}

	//How much of the spoiler paths do we show?

	for (i=0; i < 4; i++)
		if (showPath[i])
		{
			temp = changeByte[4*i][curRoom][curDungeon];
			if (changeByte[4*i+2][curRoom][curDungeon])
			{
				thisIcon[changeByte[4*i+2][curRoom][curDungeon] >> 4][changeByte[4*i+2][curRoom][curDungeon] & 0xf] = temp;
			}
			if (changeByte[4*i+3][curRoom][curDungeon])
			{
				thisIcon[changeByte[4*i+3][curRoom][curDungeon] >> 4][changeByte[4*i+3][curRoom][curDungeon] & 0xf] = temp;
			}
		}

	for (i=0; i < 4; i++)
		if (changeByte[4*i][curRoom][curDungeon])
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_ENABLED);
		else
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_GRAYED);

	//Okay, print everything out

	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			BitBlt(localhdc, i*32+288, j*32, 32, 32, roomdc,
				32*(thisIcon[i][j] % 0x10), 32*(thisIcon[i][j] / 0x10), SRCCOPY);

	//After deciding which icon to show, we now decide if we want the transparent spoilers of where to walk and what it reveals

	if (showSpoilers)
	{
		for (i=0; i < 0x10; i += 4)
			if (changeByte[i+1][curRoom][curDungeon])
			{
				temp = changeByte[i+1][curRoom][curDungeon] >> 4;
				temp2 = changeByte[i+1][curRoom][curDungeon] & 0xf;
				changedYet[temp][temp2] = 1;
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					208, 240, 16, 16, 0x000000);

			}
		for (i=0; i < 0x10; i += 4)
		{
			if (changeByte[i+2][curRoom][curDungeon])
			{
				temp = changeByte[i+1][curRoom][curDungeon] >> 4;
				temp2 = changeByte[i+1][curRoom][curDungeon] & 0xf;
				// ?? we need to figure a way to print stuff out to see what spoils what, or doesn't. It depends on changeByte.
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					448 + 32 * changedYet[temp][temp2], 480, 32, 32, 0x000000);

			}
			if (changeByte[i+3][curRoom][curDungeon])
			{
				temp = changeByte[i+1][curRoom][curDungeon] >> 4;
				temp2 = changeByte[i+1][curRoom][curDungeon] & 0xf;
				// ?? we need to figure a way to print stuff out to see what spoils what, or doesn't. It depends on changeByte.
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					480 + 32 * changedYet[temp][temp2], 480, 32, 32, 0x000000);

			}
		}
		
	}

	if (syncLevelToRoom)
	{
		if (curLevel != roomLev[curRoom][curDungeon])
		{
			curLevel = roomLev[curRoom][curDungeon];
			PaintDunMap();
		}
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
	for (i=0; i < 16; i++)
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
	ReleaseDC(hwnd, hdc);
	}


	adjHeader();
}

short curDirValid()
{
	short runTotal = 0;
	short i;

	for (i=0; i < 8; i++)
	{
		 if (showParty==PARTY_NORTH)
			 runTotal += partyX[0][i][curRoom][curDungeon] + partyY[0][i][curRoom][curDungeon];
		 if (showParty==PARTY_SOUTH)
			 runTotal += partyX[2][i][curRoom][curDungeon] + partyY[2][i][curRoom][curDungeon];
		 if (showParty==PARTY_EAST)
			 runTotal += partyX[1][i][curRoom][curDungeon] + partyY[1][i][curRoom][curDungeon];
		 if (showParty==PARTY_WEST)
			 runTotal += partyX[3][i][curRoom][curDungeon] + partyY[3][i][curRoom][curDungeon];
	}
	return (runTotal != 0);
}

void adjustSecretCheckmarks()
{
	short i;
	short temp = 0;
	for (i=0; i < 4; i++)
		temp += showPath[i];

	if (temp == 4)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_REVEAL_ALL_SECRET, MF_CHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_REVEAL_ALL_SECRET, MF_UNCHECKED);

	if (temp == 0)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_ALL_SECRET, MF_CHECKED);
	else
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_ALL_SECRET, MF_UNCHECKED);

	for (i=0; i < 4; i++)
		if (showPath[i] && changeByte[4*i+1][curRoom][curDungeon])
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_CHECKED);
		else
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_UNCHECKED);
}

void adjHeader()
{
	char buffer[100];

	sprintf(buffer, "Ultima IV Dungeon Browser: %s, level %d, room %d (L%d)", dunName[curDungeon], curLevel + 1,
		curRoom + 1, roomLev[curRoom][curDungeon]+1);
	SetWindowText(hwnd, buffer);
}

void DungeonReset()
{
	PaintDunMap();
	if (resetRoomA)
		curRoom = 0;
	if (resetLevel0)
		curLevel = 0;
	if ((curRoom > 15) && (curDungeon != ABYSS))
		curRoom = 15;	//From far down the abyss, room 16+ is not valid, so let's default to the altar room

	//(dis)able Abyss up/down
	EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_UP2, MF_GRAYED);
	if (curDungeon == ABYSS)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_GRAYED);

	doRoomCheck();
}

void doRoomCheck()
{
	if ((curDungeon == ABYSS) && (curRoom >= 16))
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_UP2, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_UP2, MF_GRAYED);

	if ((curDungeon == ABYSS) && (curRoom <= 47))
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_GRAYED);

	if (curRoom > 0)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_GRAYED);

	if (curRoom < 15)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_ENABLED);
	else if ((curRoom < 63) && (curDungeon == ABYSS))
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_GRAYED);

	adjustSecretCheckmarks();
	PaintRoomMap();
}

short toMonster(short icon)
{
	if ((icon >= 0x20) && (icon <= 0x2f))
	{ //class enemies, 2 icons per
		return (icon - 0x20) / 2;
	}
	if ((icon >= 0x50) && (icon <= 0x5f))
	{ //townfolk, including Lord British, 2 icons per
		return 8 + (icon - 0x50) / 2;
	}
	if ((icon >= 0x80) && (icon <= 0x8f))
	{ //sea monsters, 2 icons per
		return 16 + (icon - 0x80) / 2;
	}
	if ((icon >= 0x90) && (icon <= 0xff))
	{ //other monsters, 4 icons per
		return 24 + (icon - 0x90) / 4;
	}
	return -1; //No monster icon found
}

void rlsDebug(char x[50])
{
	FILE * F = fopen("debug.txt", "a");

	fputs(x, F);
	fputs("\n", F);
	fclose(F);
}