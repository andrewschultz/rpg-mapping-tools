Ultima V Dungeon Surfer

Originally conceived circa 2005, restarted 2015 (5/3)

This Windows executable allows the user to page through all eight dungeons in Ultima 5 and all 112 rooms, 16 per dungeon. Well, except for Despise, which has no rooms.

The accelerators and options are hopefully described well enough in the menus.

None of the binaries from Ultima V are included here, but DUNGEON.DAT and DUNGEON.CBT are the ones to use. TILES.16 has the pixel data, which is LZW-compressed. Thanks to the Codex of Ultimate Wisdom for all this information:

http://ultima.wikia.com/wiki/Ultima_V:_Warriors_of_Destiny

You will probably want the DSW file to build the app, if you wish to build it, though you can also just add msimg32.lib, which is needed for TransparentBlt.

At the moment, you definitely need all the BMP files to build it. And you need the BMP files to run it, too.

You do not need the u5d.* maps. They are only there to show you how I extracted the dungeon data.
