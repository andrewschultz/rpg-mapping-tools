Ultima IV Dungeon Surfer

Originally conceived circa 2005, restarted 2015 (5/2)

This Windows executable allows the user to page through all eight dungeons in Ultima 5 and all the rooms. Each dungeon has 16, except the Abyss, which has 64.

The accelerators and options are hopefully described well enough in the menus.

None of the binaries from Ultima V are included here, but *.DUN (each dungeon name) has the dungeon map data. SHAPES.EGA has the pixel data.

Thanks to the Codex of Ultimate Wisdom for miscellaneous info that helped make exploring easier:

http://ultima.wikia.com/wiki/Ultima_IV:_Quest_of_the_Avatar

==============================================

Here is a brief list of accelerators used by both the Ultima IV and V surfers. You can use the pull-down menu, but it is hopefully a bit more intuitive to use the keys.

1-8 = choose dungeon 1-8, 1=Deceit, 8=Abyss. Page up/down pages through dungeons.

ctrl-1-8 = choose floor 1-8 on the current dungeon. Up and down arrows are also permitted.

A-P navigates rooms 1-16. You can use left and right arrows, or you can click on the room in the map.

Show Monsters shows the monsters. Hide Mimic hides the mimic--by default, it looks a bit different from the chest.

F5 identifies all trigger squares, and F6 shows all squares they trigger.

9 shows a textual dungeon summary below the dungeon map. 0 shows a textual room summary below the room map.

Alt-(arrows) shows the party starting on that side, if available. Alt-end hides the party, and alt-home shows them in the first available position, north then clockwise.

- shows the dungeon map at half-size, copied four times, so you can see the wrapping. = gives a more close-up view. \ switches to centered view, where you can view 14x14, 12x12 or 10x10 wrap. Pushing \ when in centered view shrinks the view, or sets 10x10 to 14x14.

Label Rooms in Main Map chooses between plain boxes and having a number (1-16) inside.

Reset to room/level 1 on dungeon change are options that reset the user if they wish.

Sync level to room and restrict room to current level are there so you know you are not wandering too far about. For instance, you're prevented from going to level 8 while on the first room.

The about menu contains miscellaneous documentation including:
- basic information popup
- thanks popup
- links to the main GitHub repository, the subdirectory for this game, and the URL for this readme
- this readme locally, if it's been copied there

==============================================

Ultima IV surfer specific options

ctrl- = +32 rooms in the Abyss, shift- = +16.

ctrl-up/down = up or down 2 levels in the abyss (change the room by 16)

Show Altar room changes #16 to Truth, Love or Courage.

Remember last abyss room lets you remember if you were in, say, room 44 in the Abyss. You won't have to page back.

Cosmetic operations include flipping and hiding party members. Since you need one of each party, the app assumes your party classes go from the top virtue to the bottom.

Shift-(2-8) allows flipping. If you want to switch player X and Y, simple hit shift-X then shift-Y. Or vice versa. Cycling through can be achieved with shift-2, 3, 4, 5, 6, 7, 8. This can be done even with one of the players, or both, or the party, hidden.

Ctrl-(2-8) allows you to toggle hiding. Your leader can never be hidden. Since your friends may die in the course of battle, it's reasonable to have a gap in position #2.

Show alt player icons (alt-space) is a silly option that shows the alternate icons in the player animations. Holding down alt-space will allow you to see the animations at much faster than game speeds.

==============================================

You will probably want the DSW file to build the app, if you wish to build it, though you can also just add msimg32.lib, which is needed for TransparentBlt.

At the moment, you definitely need all the BMP files to build it. And you need the BMP files to run it, too.

You do not need the u4d.* maps. They are only there to show you how I extracted the dungeon data. They are used by mapconv and mapcpy. Also, u4shapes.c is not useful for the main executable. It only builds the shape-map.