#include <stdio.h>

short ico[256][256];

main(int ARGC, char * ARGV[])
{
	int i, j, k, l;
	short ch;

	FILE * F = fopen("shapes.ega", "rb");
	FILE * G = fopen("shapes.bmp", "wb");
	FILE * H = fopen("256.bmp", "rb");

	for (i=0; i < 0x436; i++)
		fputc(fgetc(H), G);

	for (l=0; l < 16; l++)
		for (k=0; k < 16; k++)
			for (j=0; j < 16; j++)
				for (i=0; i < 8; i++)
				{
					ch = fgetc(F);
					ico[2*i+k*16+1][j+l*16] = ch & 0xf;
					ico[2*i+k*16][j+l*16] = ch >> 4;
					//printf("%d, %d = %x, %x, %x\n", j+l*16, 2*i+k*16, ch, ch & 0xf, ch >> 4);
				}
	for (k=0; k < 256; k++)
		for (j=0; j < 256; j++)
		{
			fputc(ico[j][255-k], G);
		}
	fclose(F);
	fclose(G);
	fclose(H);
}