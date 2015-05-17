//u5map.h: the list of constants for menu items.
//
//This is organized so that u4map roughly matches u5map.h. Game-specific variables are labeled as such.
//

#define ID_DUNGEON_DECEIT 101
#define ID_DUNGEON_DESPISE 102
#define ID_DUNGEON_DESTARD 103
#define ID_DUNGEON_WRONG 104
#define ID_DUNGEON_COVETOUS 105
#define ID_DUNGEON_SHAME 106
#define ID_DUNGEON_HYTHLOTH 107
#define ID_DUNGEON_DOOM 108
#define ID_DUNGEON_INFO 109

#define ID_DUNGEON_PREV 120
#define ID_DUNGEON_NEXT 121

#define ID_NAV_UP 201
#define ID_NAV_DOWN 202
#define ID_NAV_PREVRM 203
#define ID_NAV_NEXTRM	204

#define ID_NAV_1	211
#define ID_NAV_2	212
#define ID_NAV_3	213
#define ID_NAV_4	214
#define ID_NAV_5	215
#define ID_NAV_6	216
#define ID_NAV_7	217
#define ID_NAV_8	218

#define ID_NAV_NULL	220
#define ID_NAV_A	221
#define ID_NAV_B	222
#define ID_NAV_C	223
#define ID_NAV_D	224
#define ID_NAV_E	225
#define ID_NAV_F	226
#define ID_NAV_G	227
#define ID_NAV_H	228
#define ID_NAV_I	229
#define ID_NAV_J	230
#define ID_NAV_K	231
#define ID_NAV_L	232
#define ID_NAV_M	233
#define ID_NAV_N	234
#define ID_NAV_O	235
#define ID_NAV_P	236

#define ID_OPTIONS_MONSTERS 301
#define ID_OPTIONS_HIDE_MIMIC 302
#define ID_OPTIONS_OUTLINE_SPOILER_SQUARES 303
#define ID_OPTIONS_OUTLINE_CHANGED_SQUARES	304

//Oh, to be able to call this 411!
#define ID_OPTIONS_DUNTEXTSUMMARY	311
#define ID_OPTIONS_ROOMTEXTSUMMARY	312

#define ID_OPTIONS_PARTY_NONE	321
#define ID_OPTIONS_PARTY_NORTH	322
#define ID_OPTIONS_PARTY_EAST	323
#define ID_OPTIONS_PARTY_SOUTH	324
#define ID_OPTIONS_PARTY_WEST	325
#define ID_OPTIONS_PARTY_FIRSTVIABLE	326

#define ID_OPTIONS_SIZE_HALF	341
#define ID_OPTIONS_SIZE_FULL	342
#define ID_OPTIONS_SIZE_AUTO	343
#define ID_OPTIONS_SIZE_BORDERED	344
#define ID_OPTIONS_MAINMAP_LABEL	345
#define ID_OPTIONS_RESET_ROOM_1	346
#define ID_OPTIONS_RESET_LEVEL_1	347
#define ID_OPTIONS_SYNC_LEVEL_TO_ROOM	348
#define ID_OPTIONS_RESTRICT_ROOM_TO_CURRENT_LEVEL	349

#define ID_OPTIONS_DEFAULT_SHOW_SECRET_PSG 361
//U4 has 4 possible, U5 has 8. In either case, we want to be able to toggle any
#define ID_OPTIONS_PSGS_1	362
#define ID_OPTIONS_PSGS_2	363
#define ID_OPTIONS_PSGS_3	364
#define ID_OPTIONS_PSGS_4	365
#define ID_OPTIONS_PSGS_5	366
#define ID_OPTIONS_PSGS_6	367
#define ID_OPTIONS_PSGS_7	368
#define ID_OPTIONS_PSGS_8	369

//Game specific options being able to swap characters at will
#define ID_MINOR_HIDE_ALL	401
#define ID_MINOR_HIDE_NONE	402

#define ID_MINOR_NOGUY_2	411
#define ID_MINOR_NOGUY_3	412
#define ID_MINOR_NOGUY_4	413
#define ID_MINOR_NOGUY_5	414
#define ID_MINOR_NOGUY_6	415
#define ID_MINOR_MAGE_2	421
#define ID_MINOR_MAGE_3	422
#define ID_MINOR_MAGE_4	423
#define ID_MINOR_MAGE_5	424
#define ID_MINOR_MAGE_6	425
#define ID_MINOR_BARD_2	431
#define ID_MINOR_BARD_3	432
#define ID_MINOR_BARD_4	433
#define ID_MINOR_BARD_5	434
#define ID_MINOR_BARD_6	435
#define ID_MINOR_FIGHTER_2	441
#define ID_MINOR_FIGHTER_3	442
#define ID_MINOR_FIGHTER_4	443
#define ID_MINOR_FIGHTER_5	444
#define ID_MINOR_FIGHTER_6	445

//ABOUT menu
#define ID_ABOUT_BASICS	501
#define ID_ABOUT_THANKS	502
#define ID_ABOUT_REPO	503
#define ID_ABOUT_REPO_THISAPP	504
#define ID_ABOUT_README_REPO	505
#define ID_ABOUT_README_LOCAL	506

//Just defining VK_ stuff so there are fewer magic numbers
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39

#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4a
#define VK_K 0x4b
#define VK_L 0x4c
#define VK_M 0x4d
#define VK_N 0x4e
#define VK_O 0x4f
#define VK_P 0x50

#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS	0xbb
#endif
#define VK_MINUS	0xbd
#define VK_OEM_BACKSLASH	0xdc
