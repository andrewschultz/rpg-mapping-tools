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

//#defines for in-app use
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

//###################globals

HWND hwnd;

HDC roomdc;
HDC leveldc;
HDC level2dc;

HDC localhdc;

long curDungeon = 0;
long curLevel = 0;
long curRoom = 0;
long showMonsters = 0;
long showParty = 0;
long altIcon = 0;
long showSpoilers = 0;
long mainLabel = 0;

short hideMimic = 0;

short wrapHalf = 0;

short resetRoomA = 0;
short resetLevel0 = 0;

short mainDun[8][8][8][8] = {0};

short roomMap[11][11][32][8] = {0};

short roomMonster[16][64][8][8] = {0};

short monsterType[16][64][8] = {0};
short monsterX[16][64][8] = {0};
short monsterY[16][64][8] = {0};

short partyEastX[8][64][8] = {0};
short partyEastY[8][64][8] = {0};

short partyNorthX[8][64][8] = {0};
short partyNorthY[8][64][8] = {0};

short partySouthX[8][64][8] = {0};
short partySouthY[8][64][8] = {0};

short partyWestX[8][64][8] = {0};
short partyWestY[8][64][8] = {0};

short changeByte[16][64][8] = {0};

short showPath[4] = {0};

char dunName[8][9] = { "Deceit", "Despise", "Destard", "Wrong", "Covetous", "Shame", "Hythloth", "Abyss"};

//Mage down to shepherd. You start as a mage, for simplicity.
short slotIcon[8] = { 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e };
short slotShow[8] = {1, 1, 1, 1, 1, 1, 1, 1};

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
		case ID_DUNGEON_ABYSS:
			if (curDungeon != LOWORD(wparam) - ID_DUNGEON_DECEIT)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_UNCHECKED);
				curDungeon = LOWORD(wparam) - ID_DUNGEON_DECEIT;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDungeon, MF_CHECKED);
				PaintDunMap();
				if (resetRoomA)
					curRoom = 0;
				if (resetLevel0)
					curLevel = 0;
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
				if (resetLevel0)
					curLevel = 0;
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
				if (resetLevel0)
					curLevel = 0;
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

		case ID_RESET_LEVEL_0:
			resetLevel0 = !resetLevel0;
			if (resetRoomA)
				CheckMenuItem( GetMenu(hwnd), ID_RESET_LEVEL_0, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_RESET_LEVEL_0, MF_UNCHECKED);
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
			else if ((curRoom < 63) && (curDungeon == 7))
			{
				curRoom++; //Abyss has four times the rooms of the other dungeons.
				PaintRoomMap();
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

		case ID_OPTIONS_HIDEMIMIC:
			hideMimic = !hideMimic;
			if (hideMimic)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDEMIMIC, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDEMIMIC, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_OPTIONS_PARTY_NONE:
		case ID_OPTIONS_PARTY_NORTH:
		case ID_OPTIONS_PARTY_EAST:
		case ID_OPTIONS_PARTY_SOUTH:
		case ID_OPTIONS_PARTY_WEST:
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE + showParty, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
			showParty = LOWORD(wparam) - ID_OPTIONS_PARTY_NONE;
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
			PaintRoomMap();
			break;

		case ID_OPTIONS_HIDE_ALL_SECRET:
			showPath[0] = showPath[1] = showPath[2] = showPath[3] = 0;
			PaintRoomMap();
			break;

		case ID_OPTIONS_SHOW_1ST_SECRET:
		case ID_OPTIONS_SHOW_2ND_SECRET:
		case ID_OPTIONS_SHOW_3RD_SECRET:
		case ID_OPTIONS_SHOW_4TH_SECRET:
			temp = LOWORD(wparam)-ID_OPTIONS_SHOW_1ST_SECRET;
			showPath[temp] = !showPath[temp];
			adjustSecretCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_WRAPHALF:
			wrapHalf = !wrapHalf;
			if (wrapHalf)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_WRAPHALF, MF_UNCHECKED);
			break;
			
		case ID_OPTIONS_MAINMAP_LABEL:
			mainLabel = !mainLabel;
			if (mainLabel)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MAINMAP_LABEL, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_MAINMAP_LABEL, MF_UNCHECKED);
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
			if (slotShow[temp])
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_UNCHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
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

		case ID_ABOUT_REPO_U4:
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
	HBITMAP level2bmp = (HBITMAP)LoadImage(hInstance,"level2.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

	roomdc = CreateCompatibleDC(localhdc);
	leveldc = CreateCompatibleDC(localhdc);
	level2dc = CreateCompatibleDC(localhdc);

	HBITMAP oldroom = (HBITMAP)SelectObject(roomdc, roombmp);
	HBITMAP oldlevel = (HBITMAP)SelectObject(leveldc, levelbmp);
	HBITMAP oldlevel2 = (HBITMAP)SelectObject(level2dc, level2bmp);

	ReadTheDungeons();
	PaintDunMap();

    hAccelTable = LoadAccelerators(hInstance, "MYACCEL");

	CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_ALL_SECRET, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE, MF_UNCHECKED);

	if (resetRoomA)
		CheckMenuItem( GetMenu(hwnd), ID_RESET_ROOM_A, MF_CHECKED);

	if (resetLevel0)
		CheckMenuItem( GetMenu(hwnd), ID_RESET_LEVEL_0, MF_CHECKED);

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
	short i, j, k;
	int numDun = 16;
	FILE * F = fopen(x, "rb");

	if (q == 7)
		numDun = 32;

	for (k=0; k < 8; k++)
		for (j=0; j < 8; j++)
			for (i=0; i < 8; i++)
				mainDun[i][j][k][q] = fgetc(F);
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
			partyNorthX[i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyNorthY[i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partyEastX[i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyEastY[i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partySouthX[i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partySouthY[i][k][q] = fgetc(F);

		for (i=0; i < 8; i++)
			partyWestX[i][k][q] = fgetc(F);
		for (i=0; i < 8; i++)
			partyWestY[i][k][q] = fgetc(F);

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
	readDun("ABYSS.DNG", 7);
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
				if ((temp & 0xf0 == 0xf0) && (!mainLabel))
					temp = 0xef;
				BitBlt(localhdc, i*16, j*16, 16, 16, level2dc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16, 16, 16, level2dc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16, j*16+128, 16, 16, level2dc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
				BitBlt(localhdc, i*16+128, j*16+128, 16, 16, level2dc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
			}
		return;
	}

	for (j=0; j < 8; j++)
	{
		for (i=0; i < 8; i++)
		{
			temp = mainDun[i][j][curLevel][curDungeon];
			if ((temp & 0xf0 == 0xf0) && (!mainLabel))
				temp = 0xef;
			BitBlt(localhdc, i*32, j*32, 32, 32, leveldc,
				32*(mainDun[i][j][curLevel][curDungeon] % 0x10), 32*(mainDun[i][j][curLevel][curDungeon] / 0x10), SRCCOPY);
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
				if ((!hideMimic) && (monsterType[i][curRoom][curDungeon] == 0xac))
					thisIcon[monsterX[i][curRoom][curDungeon]][monsterY[i][curRoom][curDungeon]] = 0xad;
			}
	}

	//Do we show where the party is? Can they enter the room from this direction? If so, display all 8 characters.

	if (showParty && curDirValid())
	{
		for (i=0; i < 8; i++)
		{
			switch (showParty)
			{
			case PARTY_NORTH:
				if (slotShow[i])
					thisIcon[partyNorthX[i][curRoom][curDungeon]][partyNorthY[i][curRoom][curDungeon]] = slotIcon[i];
				break;
			case PARTY_SOUTH:
				if (slotShow[i])
					thisIcon[partySouthX[i][curRoom][curDungeon]][partySouthY[i][curRoom][curDungeon]] = slotIcon[i];
				break;
			case PARTY_EAST:
				if (slotShow[i])
					thisIcon[partyEastX[i][curRoom][curDungeon]][partyEastY[i][curRoom][curDungeon]] = slotIcon[i];
				break;
			case PARTY_WEST:
				if (slotShow[i])
					thisIcon[partyWestX[i][curRoom][curDungeon]][partyWestY[i][curRoom][curDungeon]] = slotIcon[i];
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
					i*8+384, 384, 32, 32, 0xffffff);

			}
		for (i=0; i < 0x10; i += 4)
			if (changeByte[i+2][curRoom][curDungeon])
			{
				temp = changeByte[i+1][curRoom][curDungeon] >> 4;
				temp2 = changeByte[i+1][curRoom][curDungeon] & 0xf;
				// ?? we need to figure a way to print stuff out to see what spoils what, or doesn't. It depends on changeByte.
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					i*8+384, 448, 32, 32, 0xffffff);

			}
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
			 runTotal += partyNorthX[i][curRoom][curDungeon] + partyNorthY[i][curRoom][curDungeon];
		 if (showParty==PARTY_SOUTH)
			 runTotal += partySouthX[i][curRoom][curDungeon] + partySouthY[i][curRoom][curDungeon];
		 if (showParty==PARTY_EAST)
			 runTotal += partyEastX[i][curRoom][curDungeon] + partyEastY[i][curRoom][curDungeon];
		 if (showParty==PARTY_WEST)
			 runTotal += partyWestX[i][curRoom][curDungeon] + partyWestY[i][curRoom][curDungeon];
	}
	return (runTotal != 0);
}

void adjustSecretCheckmarks()
{
	short i;
	short temp = 0;
	for (i=0; i < 4; i++)
		temp += showPath[i];

	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_REVEAL_ALL_SECRET, MF_UNCHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_ALL_SECRET, MF_UNCHECKED);

	if (temp == 4)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_REVEAL_ALL_SECRET, MF_CHECKED);
		return;
	if (temp == 0)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_HIDE_ALL_SECRET, MF_CHECKED);
		return;

	for (i=0; i < 4; i++)
		if (showPath[i])
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET+i, MF_CHECKED);
		else
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET+i, MF_UNCHECKED);
}

void adjHeader()
{
	char buffer[100];
	long fudgeFactor = 0;

	if (curDungeon == 7)	//new 16 rooms every 2 Abyss levels
		fudgeFactor += 16 * (curLevel / 2);

	sprintf(buffer, "Ultima IV Dungeon Surfer: %s, level, %d, room %d", dunName[curDungeon], curLevel, curRoom + fudgeFactor);
	SetWindowText(hwnd, buffer);
}