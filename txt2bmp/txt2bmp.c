#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void main(int argc, char * argv[])
{
	short map[256][256] = {0};
	short colors[256] = {0};

	short bmpHeight = 256;
	short bmpWidth = 256;

	FILE * F1;
	FILE * F2;
	FILE * F3;

	char buffer[256];
	char myfi[256];
	char my256[256] = "c:\\coding\\games\\256.bmp";

	int i = 0;
	int j;

	if (argc == 1)
	{
		strcpy(myfi, "wil.txt");
		//printf("Need to specify a file!\n");
		//return;
	}
	else
		strcpy(myfi, argv[1]);

	for (i=0; i < 256; i++)
		colors[i] = 8;

	F2 = fopen(myfi, "r");

	if (F2 == NULL)
	{
		printf("No file %s, bailing.\n", argv[0]);
		return;
	}

	i = 0;

	while (fgets(buffer, 200, F2))
	{
		if (buffer[0] == 0)
			continue;
		if ((buffer[0] == ';') && (strlen(buffer) < 5))
			break;
		if (buffer[0] == '#')
			continue;
		if (buffer[0] == 'h')
			if (buffer[1] == '=')
			{
				bmpHeight = (short)strtol(buffer+2, NULL, 10);
				continue;
			}
		if (buffer[0] == 'w')
			if (buffer[1] == '=')
			{
				bmpWidth = (short)strtol(buffer+2, NULL, 10);
				continue;
			}

		for (j=0; j < (short)(strlen(buffer)-1); j++)
			map[j][i] = (char)buffer[j];
		i++;
	}

	F1 = fopen(my256, "rb");

	if (F1 == NULL) { printf("No file %s, bailing.\n", my256); }
	F3 = fopen("c:\\coding\\games\\txt2bmp\\out.bmp", "wb");

	for (i=0; i < 0x436; i++) { fputc(fgetc(F1), F3); }

	for (j=0; j < 256; j++)
		for (i=0; i < 256; i++)
			fputc(map[i][255-j], F3);

	fclose(F1);
	fclose(F2);
	fclose(F3);

}