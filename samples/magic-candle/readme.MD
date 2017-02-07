*MAGIC CANDLE SERIES MAP UTILITIES*

This is a sample subdirectory for Magic Candle series maps.

The original Magic Candle is not included here because its format is different from Magic Candle 2, Magic Candle 3 and Bloodstone.

You need to install DOSBox to extract the Bloodstone files.

**mcmap.c**

mcmap.c should allow you to see what each 42x42 chunk would look like on a full screen. It takes in an icon/tile file (MCT) then any number of map (MCM) files.

The tile set E17.MCT corresponds to M*E17.MCM, and so forth. It's a pretty simple format.

* start at 0x100
** 08 10 = width height
** each nibble is  the VGA color, low byte on the left

**mcbmp.pl**

This organizes the MCT file with its respective MCM files.