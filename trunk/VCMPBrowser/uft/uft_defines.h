#pragma once

#define _UFT_VER 1

#define UFT_STACK_ALLOCATION_SIZE 128
#define UFT_MAX_STRING_LEN 250

typedef enum E_UFT_PARSE_ERROR
{
	E_UPE_NONE,
	E_UPE_HEADER_TOO_SMALL,
	E_UPE_NO_MEMORY,
	E_UPE_NOT_UFT,
	E_UPE_WRONG_VER,
	E_UPE_UNSUPPORTED
} E_UFT_PARSE_ERROR;

typedef enum E_UFT_FILE_ERROR
{
	E_UFE_NONE,
	E_UFE_INVALIDPTR,
	E_UFE_INVALIDFILE,
} E_UFT_FILE_ERROR;

// The enum below contains byteflags to use
typedef enum E_UFT_FILE_TYPE
{
	E_UFT_UNUSED = 0x00, // Version 1 used this
	E_UFT_PLAINTEXT = 0x01,

	E_UFT_ENCODED = 0x02,
	E_UFT_ARCHIVE = 0x04,
	E_UFT_COMPRESSED = 0x08
} E_UFT_FILE_TYPE;
