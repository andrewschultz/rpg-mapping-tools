void HelpBombOut();
void helpMinorFeatures();
void AHSHelp();
void NMRHelp();

void HelpBombOut()
{
	printf("Flag -? for this help command.\n\
Flag -?? for minor features.\n\
Flag -0 so bin file puts 0s for known.\n\
Flag -a shows options for AHS files and how to write them.n\
Flag -af fills unknown squares with LCD of their hex values.n\
Flag -b specifies blank icon.\n\
Flag -c to set default blank color, in hexadecimal.\n\
Flag -d to get rid of *.png.(bak/000) files.\n\
Flag -eb to erase binary files, -ei to erase and ignore, -ib to ignore binary.\n\
Flag -db to give debug text (xtr files, etc.).\n\
Flag -h to output html file of the output graphic's palettes.\n\
Flag -n to turn off default header.\n\
Flag -i to show icon debugging.\n\
Flag -io to create icon BMP. (optional # for filler/border color).\n\
Flag -nh to show NMR help.\n\
Flag -p to postprocess to png. -pa also erases png.bak/000 files.\n\
Flag -r to regenerate the base BMP file.\n\
Flag -R to reverse when IDing unused icons (default is top to bottom).\n\
Flag -s to show used/unused icon stats at the end.\n\
Flag -S to print out sort warning for icon files.\n\
Flag -t(xx) to flag transparency and specify the color. Black=default.\n\
Flag -u to debug squares with no icon.\n\
Flag -uf to debug squares with no icon, only showing the first.\n\
Flag -x to add extra modifications to the base BMP files, -xn to add -xtr, and -x0 to disable XTR file/add -nox.\n\
Flag -# gives stats, -#n is unused only, -#u is used only\n");

	exit(0);
}

void helpMinorFeatures()
{
	printf("Flag -ch asks for color help\n\
Flag -xy signifies mapping for Xyphus, which has a half-icon shifted jagged map\n\
Flag -i(s/h/w) forces icon width/height, mostly used for testing\n\
Flag -ir to ignore redefinition of icons (only if this is expected).\n");

    exit(0);
}

void AHSHelp()
{
	printf("# means a comment.\n\
h15 sets height to 15. (default 8, max 16)\n\
w13 sets width to 13. (default 8, max 16)\n\
hw/wh/s9 sets height and width to 9. (default 8, max 16)\n\
0x0a=3 sets icon 0x0a to 3 (green).\n\
0x0fc0e copies icon 0x0e to 0x0f.\n\
0x12h03 makes 0x12 a horizontal flip of 03.\n\
0x12h03 makes 0x12 a vertical flip of 03.\n\
\n\
0x0ax34 alternates 3/4 in checkerboard pattern, starting with 3.\n\
0x0a/35 puts 3 in the upper left, 5 in lower right, diagonal. Extra / means 3 along the center.\n\
0x0a\\35 puts 3 in the upper right, 5 in lower left, diagonal. Extra \\ means 3 along the center.\n\
\n\
0x12l06 rotates 06 90 degrees left.\n\
0x12r06 rotates 06 90 degrees right.\n\
0x12H34 horizonally clips an icon by 3, with color 4 trim.\n\
0x12V34 vertically clips an icon by 3, with color 4 trim.\n\
0x12S34 switches 2 colors in an icon, in this case, 3 and 4.\n\
L00 = start LCD 0-9 at number given (l means don't overwrite icons that are there).\n\
\n\
\'#\' detects the character instead of, say, 0x0a.\n\
> runs something from the command line.\n\
\n\
; ends the file.\n");

	exit(0);
}

void NMRHelp()
{
	printf("NMR Help\n\
A quick and dirty guide to NMR files. #'s are comments, which are not allowed, yet.\n\
my-in.bmp\n\
my-in.pix #doesn't need the same prefix\n\
x-init,y-init,x-final,y-final,BINFILE,BMPFILE\n\
Additional lines make more maps\n\
. #flushes the buffer\n\
GT #optional command line stuff\n\
semicolon #ends input\n");

	printf("NMR Help\n");

	exit(0);
}

