//u4map.h: the list of constants for menu items.
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
#define ID_DUNGEON_ABYSS 108
#define ID_DUNGEON_INFO	109

#define ID_DUNGEON_PREV 120
#define ID_DUNGEON_NEXT 121

#define ID_NAV_UP 201
#define ID_NAV_DOWN 202
#define ID_NAV_PREVRM 203
#define ID_NAV_NEXTRM	204

//Ultima IV specific options, since the Abyss is bigger than Doom
#define ID_NAV_ABYSS_UP2	205
#define ID_NAV_ABYSS_DOWN2	206

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

//This whole chunk is U4-specific, since it involves picking out a specific Abyss room.
//The user may not use too many of these, but they are there.
#define ID_NAV_SHIFT_A	237
#define ID_NAV_SHIFT_B	238
#define ID_NAV_SHIFT_C	239
#define ID_NAV_SHIFT_D	240
#define ID_NAV_SHIFT_E	241
#define ID_NAV_SHIFT_F	242
#define ID_NAV_SHIFT_G	243
#define ID_NAV_SHIFT_H	244
#define ID_NAV_SHIFT_I	245
#define ID_NAV_SHIFT_J	246
#define ID_NAV_SHIFT_K	247
#define ID_NAV_SHIFT_L	248
#define ID_NAV_SHIFT_M	249
#define ID_NAV_SHIFT_N	250
#define ID_NAV_SHIFT_O	251
#define ID_NAV_SHIFT_P	252

#define ID_NAV_CTRL_A	253
#define ID_NAV_CTRL_B	254
#define ID_NAV_CTRL_C	255
#define ID_NAV_CTRL_D	256
#define ID_NAV_CTRL_E	257
#define ID_NAV_CTRL_F	258
#define ID_NAV_CTRL_G	259
#define ID_NAV_CTRL_H	260
#define ID_NAV_CTRL_I	261
#define ID_NAV_CTRL_J	262
#define ID_NAV_CTRL_K	263
#define ID_NAV_CTRL_L	264
#define ID_NAV_CTRL_M	265
#define ID_NAV_CTRL_N	266
#define ID_NAV_CTRL_O	267
#define ID_NAV_CTRL_P	268

#define ID_NAV_CTRL_SHIFT_A	269
#define ID_NAV_CTRL_SHIFT_B	270
#define ID_NAV_CTRL_SHIFT_C	271
#define ID_NAV_CTRL_SHIFT_D	272
#define ID_NAV_CTRL_SHIFT_E	273
#define ID_NAV_CTRL_SHIFT_F	274
#define ID_NAV_CTRL_SHIFT_G	275
#define ID_NAV_CTRL_SHIFT_H	276
#define ID_NAV_CTRL_SHIFT_I	277
#define ID_NAV_CTRL_SHIFT_J	278
#define ID_NAV_CTRL_SHIFT_K	279
#define ID_NAV_CTRL_SHIFT_L	280
#define ID_NAV_CTRL_SHIFT_M	281
#define ID_NAV_CTRL_SHIFT_N	282
#define ID_NAV_CTRL_SHIFT_O	283
#define ID_NAV_CTRL_SHIFT_P	284

#define ID_NAV_INSTR	285

#define ID_OPTIONS_MONSTERS	301
#define ID_OPTIONS_HIDE_MIMIC 302
#define ID_OPTIONS_OUTLINE_SPOILER_SQUARES 303
#define ID_OPTIONS_OUTLINE_CHANGED_SQUARES	304

//Oh, to be able to call this 411!
#define ID_OPTIONS_TEXTSUMMARY	311

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
#define ID_OPTIONS_SHOW_THIS_ROOM	362
#define ID_OPTIONS_HIDE_THIS_ROOM	363
#define ID_OPTIONS_SHOW_1ST_SECRET	364
#define ID_OPTIONS_SHOW_2ND_SECRET	365
#define ID_OPTIONS_SHOW_3RD_SECRET	366
#define ID_OPTIONS_SHOW_4TH_SECRET	367

//Game specific options being able to swap characters at will
#define ID_MINOR_HIDE_ALL	401
#define ID_MINOR_HIDE_NONE	402

#define ID_MINOR_SWAP_1	411 //not really valid but there for reference
#define ID_MINOR_SWAP_2	412
#define ID_MINOR_SWAP_3	413
#define ID_MINOR_SWAP_4	414
#define ID_MINOR_SWAP_5	415
#define ID_MINOR_SWAP_6	416
#define ID_MINOR_SWAP_7	417
#define ID_MINOR_SWAP_8	418

#define ID_MINOR_HIDE_1	421 //not really valid but there for reference
#define ID_MINOR_HIDE_2	422
#define ID_MINOR_HIDE_3	423
#define ID_MINOR_HIDE_4	424
#define ID_MINOR_HIDE_5	425
#define ID_MINOR_HIDE_6	426
#define ID_MINOR_HIDE_7	427
#define ID_MINOR_HIDE_8	428

#define ID_MINOR_ALT_ICONS	431

//ABOUT menu
#define ID_ABOUT_BASICS	501
#define ID_ABOUT_THANKS	502
#define ID_ABOUT_REPO	503
#define ID_ABOUT_REPO_THISAPP	504
#define ID_ABOUT_README	505

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

#define VK_OEM_PLUS	0xbb
#define VK_MINUS	0xbd
#define VK_OEM_BACKSLASH	0xdc
