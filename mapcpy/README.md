# OVERVIEW

Mapcpy reads in data from a binary disk image or set of images and places it into a blank BMP file. You can restrict the height and width and also move around the initial x- and y-positions. The sample files show how to use the language, so to speak, but they need more details.

# BUILDING

You should not need anything special to build mapcpy.c.

# THINGS TO FIX

Mapcpy.c currently accesses c:\coding\games\256.bmp to create a blank BMP, which is my base directory for tinkering with games. This is not ideal, as it should be more flexible.

While I usually use *.txt as an extention, .mcp may be clearer so the user knows they're dealing with a MAPCPY file.

# EXAMPLES

Examples of mapcpy together with mapconv in action are in the U4 and U5 folders. These require binary accessory files I won't include in the repositories, because you can find them on the Internet, especially U4.