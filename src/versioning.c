/*
 ============================================================================
 Name        : versioning.c
 Author      : Tommy Lin
 Version     :
 Copyright   : Copyright (c) 2014 ENE Technology, Inc.
 Description : Versioning firmware file
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <windows.h>



#define BUFFERSIZE						(64 * 1024)

#define DEFAULT_VERSION_OFFSET			0x410


int read_file(char *file_name, char *buf, PDWORD len)
{
	HANDLE	hFile;
	OVERLAPPED	ol = {0};


	hFile = CreateFile(file_name,
					   GENERIC_READ,
					   FILE_SHARE_READ,
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return EXIT_FAILURE;

	if (FALSE == ReadFile(hFile, buf, BUFFERSIZE, len, &ol)) {
		printf("ReadFile failed!()");
		CloseHandle(hFile);
		return EXIT_FAILURE;
	}

	printf("Open \"%s\" success(%lu bytes)!!!\n", file_name, *len);

	CloseHandle(hFile);

	return EXIT_SUCCESS;
}


void print_argv(int argc, char *argv[])
{
	int i;

	for (i = 0; i < argc; i++) {
		printf("parameter %d = %s\n", i, argv[i]);
	}
}

/* to find the pattern of -X.YY */
BOOL is_file_versioned(char *source)
{
	char *pch = {0};

	pch = strchr(source, '-');

	if (pch == NULL)
		/* Can't find '-' in source string */
		return FALSE;

	if (!isdigit(pch[1]))
		/* -X.YY: X is not a digit number. */
		return FALSE;

	if (pch[2] != '.')
		/* -X.YY: index [2] is dot '.' */
		return FALSE;

	if (!isdigit(pch[3]))
		/* -X.YY: Y is not a digit number. */
		return FALSE;

	return TRUE;
}

int main(int argc, char *argv[]) {

	DWORD dwBytesReturned;
	char ReadBuffer[BUFFERSIZE] = {0};
	char version[8] = {0};
	char cmd[1024] = {0};
	char filename[64] = {0};
	int version_offset;
	int index;

#define __print_argument__
#ifdef __print_argument__
	print_argv(argc, argv);
#endif

	if (is_file_versioned(argv[1])) {
		printf("File %s already versioned!!!\n", argv[1]);
		return EXIT_SUCCESS;
	}


	if (read_file(argv[1], ReadBuffer, &dwBytesReturned))
		return EXIT_FAILURE;

	if (argc >= 3)
		version_offset = strtol(argv[2], NULL, 10);
	else
		version_offset = DEFAULT_VERSION_OFFSET;

	version[0] = '-';
	strcpy(version + 1, ReadBuffer + version_offset);
	printf("Firmware version: %s @ %d(0x%X)\n", version + 1, version_offset, version_offset);


	index = strlen(argv[1]) - 4;
	strncpy(filename, argv[1], index);
	strncpy(filename + index, version, strlen(version));
	strncpy(filename + index + strlen(version), argv[1] + index, 4);

	sprintf(cmd, "cp -v %s %s", argv[1], filename);
	system(cmd);

	return EXIT_SUCCESS;
}
