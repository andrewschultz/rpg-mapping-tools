Ultima V Dungeon Surfer

Originally conceived circa 2005, restarted 2015 (5/3)

This Windows executable allows the user to page through all eight dungeons in Ultima 5 and all 112 rooms, 16 per dungeon. Well, except for Despise, which has no rooms.

None of the binaries from Ultima V are included here, but DUNGEON.DAT and DUNGEON.CBT are the ones to use. TILES.16 has the pixel data, which is LZW-compressed. Thanks to the Codex of Ultimate Wisdom for all this information:

http://ultima.wikia.com/wiki/Ultima_V:_Warriors_of_Destiny

==============================================

Here is a brief list of accelerators used by both the Ultima IV and V surfers. You can use the pull-down menu, but it is hopefully a bit more intuitive to use the keys.

1-8 = choose dungeon 1-8, 1=Deceit, 8=Doom. Page up/down pages through dungeons.

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

Ultima V surfer specific options

You may hide the party with F4. F7 shows everyone. Each supporting player may be toggled with F8 through F12. Control makes the player a fighter, shift a bard, and alt a mage. The player must be the avatar.

==============================================

You will probably want the DSW file to build the app, if you wish to build it, though you can also just add msimg32.lib, which is needed for TransparentBlt.

At the moment, you definitely need all the BMP files to build it. And you need the BMP files to run it, too.

You do not need the u5d.* maps. They are only there to show you how I extracted the dungeon data.


