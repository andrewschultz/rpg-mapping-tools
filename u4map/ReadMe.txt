Ultima IV Dungeon Surfer

Originally conceived circa 2005, restarted 2015 (5/2)

This Windows executable allows the user to page through all eight dungeons in Ultima 5 and all the rooms. Each dungeon has 16, except the Abyss, which has 64.

The accelerators and options are hopefully described well enough in the menus.

None of the binaries from Ultima V are included here, but *.DUN (each dungeon name) has the dungeon map data. SHAPES.EGA has the pixel data.

Thanks to the Codex of Ultimate Wisdom for miscellaneous info that helped make exploring easier:

http://ultima.wikia.com/wiki/Ultima_IV:_Quest_of_the_Avatar

You will probably want the DSW file to build the app, if you wish to build it, though you can also just add msimg32.lib, which is needed for TransparentBlt.

At the moment, you definitely need all the BMP files to build it. And you need the BMP files to run it, too.

You do not need the u4d.* maps. They are only there to show you how I extracted the dungeon data. They are used by mapconv and mapcpy.

