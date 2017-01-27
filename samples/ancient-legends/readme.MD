# Ancient Legends readme

Ancient Legends is a game created in 2016 by Seth Sternberg of Eight Bit Weapon for Apple II or Apple II emulators. I am grateful to have been a part of this. I tested the game. It was presented at 2016 at KansasFest, a gathering of Apple II fans.

It is a small-sized game that shows off the main features of the [the Outlaw Editor](https://github.com/badvision/lawless-legends/). The overworld is 2-d, and the underworld is 3-d. While the main project is Lawless Legends, AL was created just to show a full robust game could be made and even fit on two Apple IIe disks.

I'll avoid too much subjective commentary on the game because I'm biased, but I liked the game more than enough to write maps for it and to establish this sub-repository to show you how things work. I didn't just write maps--I tweaked the map editor and added a lot of features I always meant to, and ones I didn't realize would be handy ten years ago.

So this small subdirectory has the raw materials for converting the data from Ancient Legends into a bitmap. Here are the instructions:

* pull the XML file from [the Outlaw Editor repository sample files](https://github.com/badvision/lawless-legends/tree/master/Example%20Content/Ancient%20Legends)
* copy all the files here to a local directory
* copy over 256.bmp from this directory, or just save a 256x256 bitmap in MSPaint
* mapconv -x -t al.nmr
* if you have the bmp2png.exe utility, you can add the -p flag to convert everything to PNG.

Note that these directions may not be perfect, so let me know if this is wrong.