# Shard of Spring/Demon's Winter readme

## Shard of Spring

You may notice that the dungeons seem lumped around in small 5x5 bits. That is because the game shows you only part of an area if there are stairs ahead, etc.

Putting the maps together can make things not quite line up by a square or two, so I left things as they are.

The Apple disk image used instead of the PC version, which threw in extra/alternating bytes in the world map, so I went with the Apple.

However, for the outside I didn't realize that the PC version was 206 wide, but rotated 90 degrees clockwise. (WRLDMAP.BIN) For the inside, I had trouble finding things.

## Demon's Winter

THe compression in Demon's Winter is simple but clever.

If a byte X read in is has the high bit (e.g. 80 to ff) then (X & 7f) is the icon to write, and the next bit says how many times. 00 means 256.

The end result is a bunch of maps rotated 90 degrees clockwise. The dw.pl perl script creates them.

The other *.MAP files are more straightforward.

I still don't have an easy way to rotate a map to read in.
