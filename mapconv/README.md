# OVERVIEW

Mapconv reads in an NMR (new map routine) file.

The NMR references a BMP, usually created with MAPCONV, which says what icons are where. There will also be an icons file (*.PIX/AHS) and a possible XTR file, for extra additions. For instance, if one specific icon is wrong, or you want to use a transparent icon, you can add that.

# STANDARD USAGE

mapconv -u mymap.nmr

```
mymap.bmp
mymap.ahs
0,0,20,20,raw.bin,town_1_map.bmp
20,0,40,20,raw.bin,town_2_map.bmp
0,20,20,40,raw.bin,town_3_map.bmp
20,20,40,40,raw.bin,town_4_map.bmp
.
;
```

What happens here is that the upper left 20x20 square is sent to town map 1, the upper right to map 2, the lower left to 3, the lower right to 4.

# BUILDING

Mapconv should be buildable without much problem in Visual C. I haven't used much code that's Windows specific.

The colors are of my own choosing. They seem to work for me to bring the effects I want.

One big thing to remember here is that any PIX files created aren't 100% accurate. Often they just want to give a feel for the terrain. So a lot of the PIX shortcuts help with that.

Examples of mapcpy together with mapconv in action are in the U4 and U5 folders. These require binary accessory files I won't include in the repositories, because you can find them on the Internet, especially U4.

