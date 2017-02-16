# Moebius/Windwalker readme

These two action/fighting games with RPG-ish elements (and a map) are different in how they show their maps.

## Moebius is straight forward: 4 64x64 maps, uncompressed.

## Windwalker is trickier. The maps are compressed as follows:

* byte X with 0x80 means no compression for the next (X + 3 - x80) bytes. Do not put this byte to the map.
* if compression is off, read byte 1 and send byte 2 to the map (byte 1) times.

The information is on disk 3 of the kracked version of windwalker (in GZip).

It is also in MAPS.GLB in the PC version of the game.
