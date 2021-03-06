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
void initMenuCheck();
void rlsDebug(char x[50]);
void checkWrapHalf();
void tryGoingUp();
short wrapHalf();
void checkAbyssRoom();
short findLevRoom();

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

#define DUNTRACKING 14
#define CHEST 0
#define STATBALL 1
#define SECRETDOOR 2
#define POISONFIELD 3
#define LIGHTNINGFIELD 4
#define FIREFIELD 5
#define SLEEPFIELD 6
#define WINDS 7
#define TRAPS 8
#define PLAIN_FOUNTAIN 9
#define HEAL_FOUNTAIN 10
#define BADHP_FOUNTAIN 11
#define EQUAL_FOUNTAIN 12
#define POISON_FOUNTAIN 13

#define MIMIC 0xac

#define MONSTERS 52

#define WRAP_HALF 0
#define WRAP_FULL 1
#define WRAP_IF_THERE 2
#define WRAP_CENTERED 3

short centerPadding = 1;

//###################globals

HWND hwnd;

HDC roomdc;
HDC leveldc;

HDC localhdc;

long curDun = 0;
long curLev = 0;
long curRoom = 0;
long showMonsters = 0;
long showParty = 0;
short altIcon = 0;
short showSpoilers = 0;
short showChanged = 0;
long mainLabel = 0;
short showAltarRooms = 1;
short dunTextSummary = 0;
short roomTextSummary = 0;
short lastAbyssRoom = -1;
short rememberAbyssRoom = 1;

short dungeonDump = 0;
FILE * F;

short noFileWarnYet = 0;
short noRoomOnLevelWarn = 0;

char plu[2][2] = { "", "s" };

char fountStr[5][7] = { "Plain", "Heal", "Bad HP", "Equal", "Poison" };
char fieldStr[4][10] = { "Poison", "Lightning", "Fire", "Sleep" };

char txtAry[256];
long txtflag[256] = {0};

char altarRoom[3][55] = {
	"TRUTH 1,1: N to Deceit, E to Shame, W to Wrong",
	"LOVE 3,3: N to Despise, E to Wrong, W to Covetous",
	"COURAGE 7,7: N to Destard, E to Covetous, W to Shame" };

//so we don't have to read dungeon spoilers in every time. See DUNTRACKING for what it maps to.
short dunSpoil[14][8] = {0};
short dunIconVal[14] = {0x40, 0x70, 0xe0, 0xa0, 0xa1, 0xa2, 0xa3, 0x80, 0x81, 0x90, 0x91, 0x92, 0x93, 0x94};

short dunStat[8] = { 1, 2, 4, 5, 6, 3, 7, 0 };

short curDir = 0; //?? what about when things are turned off

short hideMimic = 0;

short wrapType = WRAP_HALF;

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
short showTripsquare[4] = {0};

short levelWraps[64][8] = {0};

short stoneLoc[8] = {0};
short dunBall[8] = {0};

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

short monsterExp[52] = { 12, 7, 7, 10, 9, 4, 3, 9,
13, 9, 7, 9, 13, 10, 11, 255,
16, 16, 5, 9, 9, 9, 16, 16,
4, 4, 5, 6,
4, 7, 4, 13,
16, 4, 16, 9,
6, 4, 6, 4,
8, 5, 9, 5,
12, 13, 7, 16,
8, 14, 15, 16 };

//Mage down to shepherd. You start as a mage, for simplicity.
short slotIcon[8] = { 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e };
short slotShow[8] = {1, 1, 1, 1, 1, 1, 1, 1};

long mouseDownX, mouseDownY;
long mouseUpX, mouseUpY;

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
			if (curDun != LOWORD(wparam) - ID_DUNGEON_DECEIT)
			{
				checkAbyssRoom();
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_UNCHECKED);
				curDun = LOWORD(wparam) - ID_DUNGEON_DECEIT;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_CHECKED);
				DungeonReset();
			}
			break;

		case ID_DUNGEON_PREV:
			if (curDun > 0)
			{
				checkAbyssRoom();
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_UNCHECKED);
				curDun--;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_CHECKED);
				DungeonReset();
			}
			break;

		case ID_DUNGEON_NEXT:
			if (curDun < ABYSS)
			{
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_UNCHECKED);
				curDun++;
				CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT + curDun, MF_CHECKED);
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
			if (curLev != LOWORD(wparam)-ID_NAV_1)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
				curLev = LOWORD(wparam) - ID_NAV_1;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_CHECKED);
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
			CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_UNCHECKED);
			curRoom = LOWORD(wparam) - ID_NAV_A;
			CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
			PaintRoomMap();
			doRoomCheck();
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
		case ID_NAV_CTRL_A:
		case ID_NAV_CTRL_B:
		case ID_NAV_CTRL_C:
		case ID_NAV_CTRL_D:
		case ID_NAV_CTRL_E:
		case ID_NAV_CTRL_F:
		case ID_NAV_CTRL_G:
		case ID_NAV_CTRL_H:
		case ID_NAV_CTRL_I:
		case ID_NAV_CTRL_J:
		case ID_NAV_CTRL_K:
		case ID_NAV_CTRL_L:
		case ID_NAV_CTRL_M:
		case ID_NAV_CTRL_N:
		case ID_NAV_CTRL_O:
		case ID_NAV_CTRL_P:
		case ID_NAV_CTRL_SHIFT_A:
		case ID_NAV_CTRL_SHIFT_B:
		case ID_NAV_CTRL_SHIFT_C:
		case ID_NAV_CTRL_SHIFT_D:
		case ID_NAV_CTRL_SHIFT_E:
		case ID_NAV_CTRL_SHIFT_F:
		case ID_NAV_CTRL_SHIFT_G:
		case ID_NAV_CTRL_SHIFT_H:
		case ID_NAV_CTRL_SHIFT_I:
		case ID_NAV_CTRL_SHIFT_J:
		case ID_NAV_CTRL_SHIFT_K:
		case ID_NAV_CTRL_SHIFT_L:
		case ID_NAV_CTRL_SHIFT_M:
		case ID_NAV_CTRL_SHIFT_N:
		case ID_NAV_CTRL_SHIFT_O:
		case ID_NAV_CTRL_SHIFT_P:
			if (curDun == ABYSS)
			{
				curRoom = LOWORD(wparam) - ID_NAV_A;
				doRoomCheck();
			}
			break;

		case ID_NAV_ABYSS_UP2:
			if ((curDun == ABYSS) && (curRoom >= 16))
			{
				curRoom -= 16;
				doRoomCheck();
			}
			break;

		case ID_NAV_ABYSS_DOWN2:
			if ((curDun == ABYSS) && (curRoom <= 47))
			{
				curRoom += 16;
				doRoomCheck();
			}
			break;

		case ID_OPTIONS_RESET_ROOM_1:
			resetRoomA = !resetRoomA;
			if (resetRoomA)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_1, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_1, MF_UNCHECKED);
			break;

		case ID_OPTIONS_RESET_LEVEL_1:
			resetLevel0 = !resetLevel0;
			if (resetLevel0)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_1, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_1, MF_UNCHECKED);
			break;

		case ID_OPTIONS_SYNC_LEVEL_TO_ROOM:
			syncLevelToRoom = !syncLevelToRoom;
			if (syncLevelToRoom)
			{
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SYNC_LEVEL_TO_ROOM, MF_CHECKED);
				curLev = roomLev[curRoom][curDun];
				PaintDunMap();
			}
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SYNC_LEVEL_TO_ROOM, MF_UNCHECKED);
			break;

		case ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL:
			restrictRoom = !restrictRoom;
			if (restrictRoom)
			{
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL, MF_CHECKED);
				if (curLev != roomLev[curRoom][curDun])
					findLevRoom();
			}
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL, MF_UNCHECKED);
			break;

		case ID_NAV_UP:
			if (curLev > 0)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
				curLev--;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_CHECKED);
				PaintDunMap();
			}
			break;

		case ID_NAV_DOWN:
			if (curLev < 7)
			{
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
				curLev++;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_CHECKED);
				PaintDunMap();
			}
			break;

		case ID_NAV_PREVRM:
			if (curRoom > 0)
			{
				if (restrictRoom)
					if (curLev != roomLev[curRoom-1][curDun])
						break;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_UNCHECKED);
				curRoom--;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_CHECKED);
				doRoomCheck();
			}
			break;

		case ID_NAV_NEXTRM:
			if (curRoom < 15)
			{
				if (restrictRoom)
					if (curLev != roomLev[curRoom+1][curDun])
						break;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_UNCHECKED);
				curRoom++;
				CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_CHECKED);
				doRoomCheck();
			}
			else if ((curRoom < 63) && (curDun == ABYSS))
			{
				if (restrictRoom)
					if (curLev != roomLev[curRoom-1][curDun])
						break;
				curRoom++; //Abyss has four times the rooms of the other dungeons.
				doRoomCheck();
			}
			break;

			//OPTIONS MENU ITEMS

		case ID_OPTIONS_OUTLINE_SPOILER_SQUARES:
			showSpoilers = !showSpoilers;
			if (showSpoilers)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_SPOILER_SQUARES, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_SPOILER_SQUARES, MF_UNCHECKED);
			adjustSecretCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_OUTLINE_CHANGED_SQUARES:
			showChanged = !showChanged;
			if (showChanged)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_CHANGED_SQUARES, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_OUTLINE_CHANGED_SQUARES, MF_UNCHECKED);
			adjustSecretCheckmarks();
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
				curDir = (short)showParty - 1;
			PaintRoomMap();
			break;

		case ID_OPTIONS_SHOW_1ST_SECRET:
		case ID_OPTIONS_SHOW_2ND_SECRET:
		case ID_OPTIONS_SHOW_3RD_SECRET:
		case ID_OPTIONS_SHOW_4TH_SECRET:
			temp = LOWORD(wparam)-ID_OPTIONS_SHOW_1ST_SECRET;
			showTripsquare[temp] = !showTripsquare[temp];
			for (i=0; i < 4; i++)
				if (changeByte[4*i+1][curRoom][curDun] == changeByte[4*temp+1][curRoom][curDun])
					showTripsquare[i] = showTripsquare[temp];
			adjustSecretCheckmarks();
			PaintRoomMap();
			break;

		case ID_OPTIONS_SIZE_BORDERED:
			if (wrapType == 3)
			{
				centerPadding++;
				if (centerPadding == 4)
					centerPadding = 1;
				PaintDunMap();
				break;
			}

		case ID_OPTIONS_SIZE_HALF:
		case ID_OPTIONS_SIZE_FULL:
		case ID_OPTIONS_SIZE_AUTO:
			wrapType = LOWORD(wparam) - ID_OPTIONS_SIZE_HALF;
			for (temp = ID_OPTIONS_SIZE_HALF; temp < ID_OPTIONS_SIZE_AUTO; temp++)
				if (temp == LOWORD(wparam))
					CheckMenuItem( GetMenu(hwnd), temp, MF_CHECKED);
				else
					CheckMenuItem( GetMenu(hwnd), temp, MF_UNCHECKED);
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

		case ID_OPTIONS_SHOW_ALTAR_ROOMS:
			showAltarRooms = !showAltarRooms;
			if (showAltarRooms)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_ALTAR_ROOMS, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_ALTAR_ROOMS, MF_UNCHECKED);
			break;

		case ID_OPTIONS_REMEMBER_ABYSS_ROOM:
			rememberAbyssRoom = !rememberAbyssRoom;
			if (!rememberAbyssRoom)
				lastAbyssRoom = -1;
			if (rememberAbyssRoom)
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_REMEMBER_ABYSS_ROOM, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_REMEMBER_ABYSS_ROOM, MF_UNCHECKED);
			break;

		case ID_NAV_DUNGEONDUMP:
			short j;
			F = fopen("c:\\coding\\games\\u4map\\u4di.txt", "r");
			for (i=0; i < 16; i++)
			{
				char buffer[30];
				fgets(buffer, 30, F);
				for (j=0; j < 16; j++)
					txtAry[j+16*i] = buffer[j];
			}
			fclose(F);
			F = fopen("c:\\coding\\games\\u4map\\u4dump.txt", "w");
			{
				short rts = roomTextSummary;
				roomTextSummary = 1;
				long oldRoom = curRoom;
				long oldDun = curDun;
				short mymax = 16;
				short i;
				short j;
				short k;
				short l;
				memset(&txtflag, 0, sizeof(txtflag));
				dungeonDump = 1;
				for (i=0; i < 8; i++)
				{
					if (i == 7) { mymax = 64; }
					for (j=0; j < mymax; j++)
					{
						curDun = i; curRoom = j;
						char buffer[200];
						fputs("\n\n", F);
						sprintf(buffer, "%s room %d, level %d\n\n", dunName[i], j+1, roomLev[j][i] + 1);
						fputs(buffer, F);
						for (k=0; k < 11; k++)
						{
							fputs("+ ", F);
							for (l=0; l < 11; l++)
							{
								fputc(txtAry[roomMap[l][k][j][i]], F);
								if (txtAry[roomMap[l][k][j][i]] == '?')
									if (txtflag[roomMap[l][k][j][i]] == 0)
										txtflag[roomMap[l][k][j][i]] = 1000000 * i + 10000 * j + 100 * k + l;
							}
							fputs("\n", F);
						}
						PaintRoomMap();
					}
				}
				dungeonDump = 0;
				curRoom = oldRoom;
				curDun = oldDun;
				roomTextSummary = rts;
				for (i=0; i < 256; i++)
					if (txtflag[i] > 0)
					{
						char buffer[50];
						sprintf(buffer, "Undef %x at %s room %d %d, %d\n", i, dunName[(txtflag[i] / 1000000) % 100],
							(txtflag[i] / 10000) % 100 + 1, txtflag[i] % 100, (txtflag[i] / 100) % 100);
						fputs(buffer, F);
					}
			}
			fclose(F);
			break;

			//MINOR/SILLY OPTIONS
		case ID_COSMETIC_SWAP_2:
		case ID_COSMETIC_SWAP_3:
		case ID_COSMETIC_SWAP_4:
		case ID_COSMETIC_SWAP_5:
		case ID_COSMETIC_SWAP_6:
		case ID_COSMETIC_SWAP_7:
		case ID_COSMETIC_SWAP_8:
			temp = slotIcon[0]; //switch player 1's class with player (SWAP#). ID_COSMETIC_SWAP_1
			slotIcon[0] = slotIcon[LOWORD(wparam)-ID_COSMETIC_SWAP_1];
			slotIcon[LOWORD(wparam)-ID_COSMETIC_SWAP_1] = temp;
			PaintRoomMap();
			break;

		case ID_COSMETIC_HIDE_2:
		case ID_COSMETIC_HIDE_3:
		case ID_COSMETIC_HIDE_4:
		case ID_COSMETIC_HIDE_5:
		case ID_COSMETIC_HIDE_6:
		case ID_COSMETIC_HIDE_7:
		case ID_COSMETIC_HIDE_8:
			temp = LOWORD(wparam)-ID_COSMETIC_HIDE_1;
			slotShow[temp] = !slotShow[temp];
			temp2 = 0;
			for (i=1; i < 7; i++)
				temp2 += slotShow[i];
			if (slotShow[temp])
			{
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_UNCHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_NONE, MF_UNCHECKED);
				if (temp2 == 7)
					CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_ALL, MF_CHECKED);
				else
					CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_ALL, MF_UNCHECKED);
			}
			else
			{
				CheckMenuItem( GetMenu(hwnd), LOWORD(wparam), MF_CHECKED);
				CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_ALL, MF_UNCHECKED);
				if (temp2 == 0)
					CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_NONE, MF_CHECKED);
				else
					CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_NONE, MF_UNCHECKED);
			}
			PaintRoomMap();
			break;

		case ID_COSMETIC_HIDE_ALL:
			for (temp=1; temp <= 7; temp++)
			{
				slotShow[temp] = 0;
				CheckMenuItem( GetMenu(hwnd), temp + ID_COSMETIC_HIDE_1, MF_CHECKED);
			}
			CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_ALL, MF_CHECKED);
			CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_NONE, MF_UNCHECKED);
			PaintRoomMap();
			break;

		case ID_COSMETIC_HIDE_NONE:
			for (temp=1; temp <= 7; temp++)
			{
				slotShow[temp] = 1;
				CheckMenuItem( GetMenu(hwnd), temp + ID_COSMETIC_HIDE_1, MF_UNCHECKED);
			}
			CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_ALL, MF_UNCHECKED);
			CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_HIDE_NONE, MF_CHECKED);
			PaintRoomMap();
			break;

		case ID_COSMETIC_ALT_ICONS:
			altIcon = !altIcon;
			if (altIcon)
				CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_ALT_ICONS, MF_CHECKED);
			else
				CheckMenuItem( GetMenu(hwnd), ID_COSMETIC_ALT_ICONS, MF_UNCHECKED);
			PaintRoomMap();
			break;

			//ABOUT MENU ITEMS

		case ID_ABOUT_BASICS:
			MessageBox(hwnd, "Ultima IV Dungeon Surfer\n\
This application lets the player browse all the dungeons and rooms.\n\
It features options and accelerators to bypass the usual traps and grinds.\n\
Your party is customizable for fun, too.\n\
Bugs? schultz.andrew@sbcglobal.net", "About U4 Dungeon Surfer", MB_OK);
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

		case ID_ABOUT_README_REPO:
			ShellExecute(hwnd, "open", "https://raw.githubusercontent.com/andrewschultz/rpg-mapping-tools/master/u4map/ReadMe.txt",
				NULL, NULL, SW_SHOWNORMAL);
			break;

		case ID_ABOUT_README_LOCAL:
			ShellExecute(hwnd, "open", "readme.txt", NULL, NULL, SW_SHOWNORMAL);
			break;

		}

		case WM_RBUTTONDOWN:
			mouseDownX = LOWORD(lparam)/16;
			mouseDownY = HIWORD(lparam)/16;
			checkWrapHalf();

			temp = mainDun[mouseDownX][mouseDownY][curLev][curDun];
			if (temp == 0x30)
			{
				if (!syncLevelToRoom)
				{
				curLev++;
				PaintDunMap();
				}
			}
			break;

		case WM_LBUTTONDOWN:
			mouseDownX = LOWORD(lparam)/16;
			mouseDownY = HIWORD(lparam)/16;
			break;

		case WM_LBUTTONUP:
			{
				mouseUpX = LOWORD(lparam)/16;
				mouseUpY = HIWORD(lparam)/16;

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
								if (partyX[curDir][temp][curRoom][curLev] == mouseDownX)
									if (partyY[curDir][temp][curRoom][curLev] == mouseDownY)
										froms = temp;
								if (partyX[curDir][temp][curRoom][curLev] == mouseUpX)
									if (partyY[curDir][temp][curRoom][curLev] == mouseUpY)
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
					checkWrapHalf();
				if (mouseDownX == mouseUpX)
					if (mouseDownY == mouseUpY)
					{
						char buffer[300];

						temp = mainDun[mouseDownX][mouseDownY][curLev][curDun];

						if (temp == 0x10 || temp == 0x30)
							tryGoingUp();

						if (temp == 0x20)
							if (!syncLevelToRoom)
							{
							curLev++;
							PaintDunMap();
							}

						if ((temp <= 0xa3) && (temp >= 0xa0))
						{
							sprintf(buffer, "%s field", fieldStr[temp - 0xa0]);
							MessageBox(hwnd, buffer, "Info", MB_OK);
						}

						if (temp == 0x8e)
							MessageBox(hwnd, "Pit", "Info", MB_OK);

						if (temp == 0x80)
							MessageBox(hwnd, "Winds", "Info", MB_OK);

						if (temp == 0x70)
							MessageBox(hwnd, "Orb", "Info", MB_OK);

						if (temp == 0xa0)
							MessageBox(hwnd, "Altar", "Info", MB_OK);

						if ((temp <= 0x94) && (temp >= 0x90))
						{
							sprintf(buffer, "%s fountain", fountStr[temp - 0x90]);
							MessageBox(hwnd, buffer, "Info", MB_OK);
						}


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
							  WS_VISIBLE | WS_MINIMIZEBOX | WS_SYSMENU,
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

	initMenuCheck();

	if (resetRoomA)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_ROOM_1, MF_CHECKED);

	if (resetLevel0)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_RESET_LEVEL_1, MF_CHECKED);

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
	rc.top = 288;
	rc.right = 288;
	rc.bottom = 640;

	{//I suppose we could cheat here and StretchBlt Icon #0
		HBRUSH hbrush=CreateSolidBrush(RGB(0, 0, 0));
		FillRect(hdc, &rc, hbrush);
		DeleteObject(hbrush);
		ReleaseDC(hwnd, hdc);
	}

	if (dunTextSummary == 0)
		return;
	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 0));

	//sprintf(buffer, "1");
	sprintf(buffer, "%s info: %d chest%s, %d stat ball%s, %d secret door%s\n", dunName[thisDun],
		dunSpoil[CHEST][thisDun], plu[dunSpoil[CHEST][thisDun] != 1],
		dunSpoil[STATBALL][thisDun], plu[dunSpoil[STATBALL][thisDun] != 1],
		dunSpoil[SECRETDOOR][thisDun], plu[dunSpoil[SECRETDOOR][thisDun] != 1]
		);

	if (dunSpoil[STATBALL][thisDun] > 0)
	{
		strcat(buffer, "Stat ball bonuses:");
		if (dunStat[thisDun] & 4)
			strcat(buffer, " +5 Str");
		if (dunStat[thisDun] & 2)
			strcat(buffer, " +5 Dex");
		if (dunStat[thisDun] & 1)
			strcat(buffer, " +5 Int");
		strcat(buffer, "\n");
	}

	if (thisDun == HYTHLOTH)
		strcat(buffer, "Stone missing from ");
	else if (thisDun == ABYSS)
		strcat(buffer, "Codex at ");
	else
		strcat(buffer, "Stone at ");

	sprintf(buffer2, "%d, %d level %d\n", stoneLoc[thisDun] & 0xf, (stoneLoc[thisDun]>>4) & 0xf, (stoneLoc[thisDun]>>8) + 1);
	strcat(buffer, buffer2);

	for (i=POISONFIELD; i <= SLEEPFIELD; i++)
		temp += dunSpoil[i][thisDun];

	if (temp)
	{
		strcat(buffer, "Fields: ");
		for (i=POISONFIELD; i <= SLEEPFIELD; i++)
			if (dunSpoil[i][thisDun])
			{
				sprintf(buffer2, " %d %s", dunSpoil[i][thisDun], fieldStr[i - POISONFIELD]);
				strcat(buffer, buffer2);
			}
		strcat(buffer, ".\n");
	}
	else
		strcat(buffer, "No magic fields.\n");

	temp = 0;

	for (i=PLAIN_FOUNTAIN; i <= POISON_FOUNTAIN; i++)
		temp += dunSpoil[i][thisDun];

	if (temp)
	{
		strcat(buffer, "Fountains: ");
		for (i=PLAIN_FOUNTAIN; i <= POISON_FOUNTAIN; i++)
			if (dunSpoil[i][thisDun])
				sprintf(buffer2, " %d %s", dunSpoil[i][thisDun], fountStr[i - PLAIN_FOUNTAIN]);
		strcat(buffer, buffer2);
	}
	else
		strcat(buffer, "No fountains.\n");

	DrawText(hdc, buffer, strlen(buffer), &rc, DT_LEFT | DT_TOP);
	ReleaseDC(hwnd, hdc);

	return;
}

void readDun(char x[20], int q)
{
	short i, j, k, l, temp, temp2;
	int numDun = 16;
	FILE * F = fopen(x, "rb");

	if (F == NULL)
	{
		char buffer[200];

		if (!noFileWarnYet)
		{
		sprintf(buffer, "The program was not able to read %s! It probably won't run successfully. You may need to copy the Ultima 4 .DNG files over.", x);
		MessageBox(hwnd, buffer, "Unreadable .DNG file", MB_OK);
		noFileWarnYet = 1;
		}
		return;
	}

	if (q == ABYSS)
		numDun = 64;

	for (k=0; k < 8; k++)
		for (j=0; j < 8; j++)
			for (i=0; i < 8; i++)
			{
				temp = fgetc(F); //?? 0xd(0-9) & 0xf0 ==
				temp2 = temp;

				//for when we want to spoil dungeons so we don't have to keep reading it in
				for (l = 0; l < DUNTRACKING; l++)
					if (temp == dunIconVal[l])
						dunSpoil[l][q]++;

				if (temp == 0xb0)
					stoneLoc[q] = (k<<8)+(j<<4)+i;

				if ((i == 0) || (j == 0) || (i == 7) || (j == 7))
					if (temp != 0xf0)
						levelWraps[k][q] = 1;

				if ((temp >= 0xd0) && (temp <= 0xdf))
				{
					if (q == ABYSS) //in abyss, add 16 for each (level/2)
					{
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
					else
						temp2 = temp % 0x10;

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
	short tempPadding = centerPadding;
	short offset = 0;

	if (wrapType == WRAP_CENTERED)
		offset = 4;

	if (wrapType == WRAP_HALF)
		tempPadding = 0;

	if (wrapHalf())
	{
		for (j=0; j < 16; j++)
			for (i=0; i < 16; i++)
			{
				if ((i < tempPadding) || (j < tempPadding) || (j > 15-tempPadding) || (i > 15-tempPadding))
					temp = 1;
				else
				{
					temp = mainDun[(i+offset)%8][(j+offset)%8][curLev][curDun];
					if ((temp == 0xdf) && (curDun != 7) && (showAltarRooms))
					{
						temp = i / 3 + 0xed;
					}
					if (((temp >= 0xd0) && (temp <= 0xdf)) && (!mainLabel))
						temp = 0xfd; // rooms 1-16

					if (((temp % 16 >= 0x8) && (temp <= 0x7f)) && (!mainLabel))
						temp = 0xfd; // rooms 17-64

				}
				BitBlt(localhdc, i*16, j*16, 16, 16, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), SRCCOPY);
			}
	}
	else
	{
		for (j=0; j < 8; j++)
			for (i=0; i < 8; i++)
			{
				temp = mainDun[i][j][curLev][curDun];

				if ((temp == 0xdf) && (curDun != 7) && (showAltarRooms))
				{
					temp = i / 3 + 0xed;
				}

				if (((temp >= 0xd0) && (temp <= 0xdf)) && (!mainLabel))
					temp = 0xfd; // rooms 1-16
				if (((temp % 16 >= 0x8) && (temp <= 0x7f)) && (!mainLabel))
					temp = 0xfd; // rooms 17-64
				StretchBlt(localhdc, i*32, j*32, 32, 32, leveldc,
					16*(temp % 0x10), 16*(temp / 0x10), 16, 16, SRCCOPY);
			}
	}

	adjHeader();

	if (curLev == 0)
			EnableMenuItem( GetMenu(hwnd), ID_NAV_UP, MF_GRAYED);
	else
			EnableMenuItem( GetMenu(hwnd), ID_NAV_UP, MF_ENABLED);

	if (curLev == 7)
			EnableMenuItem( GetMenu(hwnd), ID_NAV_DOWN, MF_GRAYED);
	else
			EnableMenuItem( GetMenu(hwnd), ID_NAV_DOWN, MF_ENABLED);

	//need to re-sort the room if it gets jumbled
	if (restrictRoom)
		findLevRoom();

	//well not really if the spoil flag isn't set. But we need it, to wipe out the text that was there
	spoilDungeon((short)curDun);
}

void PaintRoomMap()
{
	int i, j, k, temp = 0, temp2 = 0;
	short thisIcon[11][11] = {0};
	short changedYet[11][11] = {0};
	RECT rc;
	HDC hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);

	rc.left = 288;
	rc.top = 360;
	rc.right = 640;
	rc.bottom = 512;

	if (restrictRoom)
		if (curLev != roomLev[curRoom][curDun])
			return;

	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			thisIcon[i][j] = roomMap[i][j][curRoom][curDun];

	if (showMonsters)
	{
		for (i=0; i < 16; i++)
			if (monsterType[i][curRoom][curDun])
			{
				if (monsterX[i][curRoom][curDun] > 0xa)
					k=0;
				if (monsterY[i][curRoom][curDun] > 0xa)
					k=0;
				thisIcon[monsterX[i][curRoom][curDun]][monsterY[i][curRoom][curDun]] = monsterType[i][curRoom][curDun];
				if ((!hideMimic) && (monsterType[i][curRoom][curDun] == MIMIC))
					thisIcon[monsterX[i][curRoom][curDun]][monsterY[i][curRoom][curDun]]++;
			}
	}

	// "smart" party placement
	if (showParty == 5)
	{
		for (i=NORTH; i <= WEST; i++)
		{
			if (partyX[i][0][curRoom][curDun] + partyY[i][0][curRoom][curDun])
			{
				for (j=0; j < 8; j++)
					if (slotShow[j])
						thisIcon[partyX[i][j][curRoom][curDun]][partyY[i][j][curRoom][curDun]] = slotIcon[j] + altIcon;
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
					thisIcon[partyX[0][i][curRoom][curDun]][partyY[0][i][curRoom][curDun]] = slotIcon[i] + altIcon;
				break;
			case PARTY_SOUTH:
				if (slotShow[i])
					thisIcon[partyX[2][i][curRoom][curDun]][partyY[2][i][curRoom][curDun]] = slotIcon[i] + altIcon;
				break;
			case PARTY_EAST:
				if (slotShow[i])
					thisIcon[partyX[1][i][curRoom][curDun]][partyY[1][i][curRoom][curDun]] = slotIcon[i] + altIcon;
				break;
			case PARTY_WEST:
				if (slotShow[i])
					thisIcon[partyX[3][i][curRoom][curDun]][partyY[3][i][curRoom][curDun]] = slotIcon[i] + altIcon;
				break;
			}
		}
	}

	//How much of the spoiler paths do we show?

	for (i=0; i < 4; i++)
		if (showTripsquare[i])
		{
			temp = changeByte[4*i][curRoom][curDun];
			if (changeByte[4*i+2][curRoom][curDun])
			{
				thisIcon[changeByte[4*i+2][curRoom][curDun] >> 4][changeByte[4*i+2][curRoom][curDun] & 0xf] = temp;
			}
			if (changeByte[4*i+3][curRoom][curDun])
			{
				thisIcon[changeByte[4*i+3][curRoom][curDun] >> 4][changeByte[4*i+3][curRoom][curDun] & 0xf] = temp;
			}
		}

	for (i=0; i < 4; i++)
		if (changeByte[4*i][curRoom][curDun])
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_ENABLED);
		else
			EnableMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_GRAYED);

	//Okay, print everything out

if (!dungeonDump)
{
	for (j=0; j < 11; j++)
		for (i=0; i < 11; i++)
			BitBlt(localhdc, i*32+288, j*32, 32, 32, roomdc,
				32*(thisIcon[i][j] % 0x10), 32*(thisIcon[i][j] / 0x10), SRCCOPY);

	//After deciding which icon to show, we now decide if we want the transparent spoilers of where to walk and what it reveals

	if (showSpoilers)
	{
		for (i=0; i < 0x10; i += 4)
			if (changeByte[i+1][curRoom][curDun])
			{
				temp = changeByte[i+1][curRoom][curDun] >> 4;
				temp2 = changeByte[i+1][curRoom][curDun] & 0xf;
				changedYet[temp][temp2] = 1;
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					208, 240, 16, 16, 0x000000);

			}
		for (i=0; i < 0x10; i += 4)
		{
			if (changeByte[i+2][curRoom][curDun])
			{
				temp = changeByte[i+1][curRoom][curDun] >> 4;
				temp2 = changeByte[i+1][curRoom][curDun] & 0xf;
				// ?? we need to figure a way to print stuff out to see what spoils what, or doesn't. It depends on changeByte.
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					448 + 32 * changedYet[temp][temp2], 480, 32, 32, 0x000000);

			}
			if (changeByte[i+3][curRoom][curDun])
			{
				temp = changeByte[i+1][curRoom][curDun] >> 4;
				temp2 = changeByte[i+1][curRoom][curDun] & 0xf;
				// ?? we need to figure a way to print stuff out to see what spoils what, or doesn't. It depends on changeByte.
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					480 + 32 * changedYet[temp][temp2], 480, 32, 32, 0x000000);

			}
		}

	}

	if (showChanged)
	{
		for (i=0; i < 0x10; i += 4)
		{
			if (changeByte[i+2][curRoom][curDun])
			{
				temp = changeByte[i+2][curRoom][curDun] >> 4;
				temp2 = changeByte[i+2][curRoom][curDun] & 0xf;
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					224 + 16 * changedYet[temp][temp2], 240, 16, 16, 0x000000);
			}
			if (changeByte[i+3][curRoom][curDun])
			{
				temp = changeByte[i+3][curRoom][curDun] >> 4;
				temp2 = changeByte[i+3][curRoom][curDun] & 0xf;
				TransparentBlt(localhdc, 288+32*temp, 32*temp2, 32, 32, leveldc,
					224 + 16 * changedYet[temp][temp2], 240, 16, 16, 0x000000);
			}
		}
	}

	if (syncLevelToRoom)
	{
		if (curLev != roomLev[curRoom][curDun])
		{
			curLev = roomLev[curRoom][curDun];
			PaintDunMap();
		}
	}

	{
	HBRUSH hbrush=CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdc, &rc, hbrush);
	DeleteObject(hbrush);
	ReleaseDC(hwnd, hdc);
	}
}
	//now text list of monsters
	if (roomTextSummary)
	{
		short roomExp = 0;
		short foundSecret = 0;
		short needComma = 0;
		short monInRoom[MONSTERS] = {0};
		char roomString[300];
		char buffer[100];
		char dirs[5] = "NESW";

		SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 0));

		for (i=NORTH; i <= WEST; i++)
		{
			if (partyX[i][0][curRoom][curDun] + partyY[i][0][curRoom][curDun])
				temp++;
		}
		sprintf(roomString, "Exit%s: ", plu[temp!=1]);

		for (i=NORTH; i <= WEST; i++)
		{
			if (partyX[i][0][curRoom][curDun] + partyY[i][0][curRoom][curDun])
			{
				temp2 = strlen(roomString);
				roomString[temp2] = dirs[i];
				roomString[temp2+1] = 0;
			}
		}
		strcat(roomString, ".\n");

		temp2 = temp = 0;

		for (i=0; i < 16; i++)
		{
			temp = monsterType[i][curRoom][curDun];
			if (toMonster(temp) != -1)
				monInRoom[toMonster(temp)]++;
		}
		for (i=0; i < MONSTERS; i++)
			if (monInRoom[i])
			{
				roomExp += monInRoom[i] * monsterExp[i];
				sprintf(buffer, "%d %s", monInRoom[i], monsterName[i]);
				if (needComma == 0)
					needComma = 1;
				else
					strcat(roomString, ", ");
				strcat(roomString, buffer);
			}
		temp = 0;
		for (j=0; j < 11; j++)
			for (i=0; i < 11; i++)
				if (roomMap[i][j][curRoom][curDun] == 0x3c)
					temp++;

		if (roomExp)
		{
			sprintf(buffer, " (%d exp)", roomExp);
			strcat(roomString, buffer);
		}

		if (temp)
		{
			sprintf(buffer, "\n%d chest%s, %d gold expected.\n", temp, plu[(temp > 1)], (99*temp) / 2);
			strcat(roomString, buffer);
		}
		else
			strcat(roomString, "\nNo chests.\n");

		temp = 0;
		for (i=0; i < 4; i+= 1)
		{
			if (changeByte[4*i+1][curRoom][curDun])
			{
				foundSecret = 1;
				temp2 = changeByte[4*i+1][curRoom][curDun];
				if (temp2 != temp)
				{
					if (temp)
						strcat(roomString, "\n");
					sprintf(buffer, "%d, %d:", temp2%0x10, temp2>>4);
					strcat(roomString, buffer);
					temp = temp2;
				}
				if (changeByte[4*i+2][curRoom][curDun])
				{
					temp2 = changeByte[4*i+2][curRoom][curDun];
					sprintf(buffer, " (%d,%d)", temp2%0x10, temp2>>4);
					strcat(roomString, buffer);
				}
				if (changeByte[4*i+3][curRoom][curDun])
				{
					temp2 = changeByte[4*i+3][curRoom][curDun];
					sprintf(buffer, " (%d,%d)", temp2%0x10, temp2>>4);
					strcat(roomString, buffer);
				}

			}
		}
		if (!foundSecret)
			strcat(roomString, "Nothing to reveal in this room.\n");

		if ((curRoom == 15) && (curDun != ABYSS))
		{
			char hyth[20] = ", S to Hythloth.\n";
			if (mainDun[1][1][7][curDun] == 0xdf)
			{
				strcat(roomString, altarRoom[0]);
				strcat(roomString, hyth);
			}
			if (mainDun[3][3][7][curDun] == 0xdf)
			{
				strcat(roomString, altarRoom[1]);
				strcat(roomString, hyth);
			}
			if (mainDun[7][7][7][curDun] == 0xdf)
			{
				strcat(roomString, altarRoom[2]);
				strcat(roomString, hyth);
			}
		}

		if (strlen(roomString))
			if (dungeonDump)
			{
				fputs(roomString, F);
				if (foundSecret)
					fputs("\n", F);
			}
			else
				DrawText(hdc, roomString, strlen(roomString), &rc, DT_LEFT | DT_TOP | DT_WORDBREAK);
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
			 runTotal += partyX[0][i][curRoom][curDun] + partyY[0][i][curRoom][curDun];
		 if (showParty==PARTY_SOUTH)
			 runTotal += partyX[2][i][curRoom][curDun] + partyY[2][i][curRoom][curDun];
		 if (showParty==PARTY_EAST)
			 runTotal += partyX[1][i][curRoom][curDun] + partyY[1][i][curRoom][curDun];
		 if (showParty==PARTY_WEST)
			 runTotal += partyX[3][i][curRoom][curDun] + partyY[3][i][curRoom][curDun];
	}
	return (runTotal != 0);
}

void adjustSecretCheckmarks()
{
	short i;
	short temp = 0;
	for (i=0; i < 4; i++)
		temp += showTripsquare[i];

	for (i=0; i < 4; i++)
		if (showTripsquare[i] && changeByte[4*i+1][curRoom][curDun])
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_CHECKED);
		else
			CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_1ST_SECRET + i, MF_UNCHECKED);
}

void adjHeader()
{
	char buffer[100];

	sprintf(buffer, "Ultima IV Dungeon Browser: %s, level %d, room %d (L%d)", dunName[curDun], curLev + 1,
		curRoom + 1, roomLev[curRoom][curDun]+1);
	SetWindowText(hwnd, buffer);
}

void DungeonReset()
{
	PaintDunMap();

	if (resetRoomA)
	{
		CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_UNCHECKED);
		CheckMenuItem( GetMenu(hwnd), ID_NAV_A, MF_CHECKED);
		curRoom = 0;
	}
	if (resetLevel0)
	{
		CheckMenuItem( GetMenu(hwnd), ID_NAV_1 + curLev, MF_UNCHECKED);
		CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);
		curLev = 0;
	}

	if ((curDun == ABYSS) && (rememberAbyssRoom) && (!resetRoomA) && (lastAbyssRoom != -1))
	{
		CheckMenuItem( GetMenu(hwnd), ID_NAV_A + curRoom, MF_UNCHECKED);
		CheckMenuItem( GetMenu(hwnd), ID_NAV_A + (curRoom%16), MF_CHECKED);
		curRoom = lastAbyssRoom;
	}

	if ((curRoom > 15) && (curDun != ABYSS))
		curRoom = 15;	//From far down the abyss, room 16+ is not valid, so let's default to the altar room

	//(dis)able Abyss up/down
	EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_UP2, MF_GRAYED);
	if (curDun == ABYSS)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_GRAYED);

	doRoomCheck();
}

//this function is for moving about rooms. It should not be called when internally tweaking a room.
void doRoomCheck()
{
	short i;

	if ((curDun == ABYSS) && (curRoom >= 16))
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_UP2, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_UP2, MF_GRAYED);

	if ((curDun == ABYSS) && (curRoom <= 47))
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_ABYSS_DOWN2, MF_GRAYED);

	if (curRoom > 0)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_PREVRM, MF_GRAYED);

	if (curRoom < 15)
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_ENABLED);
	else if ((curRoom < 63) && (curDun == ABYSS))
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_ENABLED);
	else
		EnableMenuItem( GetMenu(hwnd), ID_NAV_NEXTRM, MF_GRAYED);

	for (i=0; i < 4; i++)
		showTripsquare[i] = showSpoilers;

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

void initMenuCheck()
{
	CheckMenuItem( GetMenu(hwnd), ID_DUNGEON_DECEIT, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_NAV_1, MF_CHECKED);
	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE, MF_UNCHECKED);
	EnableMenuItem( GetMenu(hwnd), ID_NAV_INSTR, MF_GRAYED);

	CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_PARTY_NONE, MF_CHECKED);

	if (showAltarRooms)
		CheckMenuItem( GetMenu(hwnd), ID_OPTIONS_SHOW_ALTAR_ROOMS, MF_CHECKED);
}

short wrapHalf()
{
	if ((wrapType == WRAP_HALF) || (wrapType == WRAP_CENTERED))
		return 1;
	if (wrapType == WRAP_FULL)
		return 0;
	if (levelWraps[curRoom][curDun]) //wrapType is WRAP_IF_THERE
		return 1;
	else
		return 0;
}

void checkWrapHalf()
{
	if (wrapHalf())
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
}

void tryGoingUp()
{
	if (curLev == 0)
		MessageBox(hwnd, "That would lead back to Brittania!", "There is no escape!", MB_OK);
	else if (!syncLevelToRoom)
	{
		curLev--;
		PaintDunMap();
	}
}

void rlsDebug(char x[50])
{
	FILE * F = fopen("debug.txt", "a");

	fputs(x, F);
	fputs("\n", F);
	fclose(F);
}

void checkAbyssRoom()
{
	if ((curDun == 7) && (rememberAbyssRoom))
		lastAbyssRoom = (short) curRoom;

}

short findLevRoom()
{
	short temp;

	for (temp=0; temp < 16; temp++)
		if (curLev == roomLev[temp][curDun])
		{
			curRoom = temp;
			PaintRoomMap();
			return temp;
		}
	if (!noRoomOnLevelWarn)
	{
		noRoomOnLevelWarn = 1;
		MessageBox(hwnd, "There are no rooms on this level. So the room panel will be greyed until you get to a level with a room.", "Small warning.", MB_OK);
	}
	//The room: Grey it out!
	StretchBlt(localhdc, 288, 0, 352, 352, leveldc, 16, 0, 16, 16, SRCCOPY);
	//The comments: Black them out!
	StretchBlt(localhdc, 288, 352, 352, 176, leveldc, 0, 0, 16, 16, SRCCOPY);
	return -1;
}