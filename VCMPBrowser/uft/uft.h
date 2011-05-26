#pragma once

#include <stddef.h>     /* size_t */
#include <stdio.h>
#include "uft_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*UFT_Parse_Callback)( unsigned char* szBuf );

typedef struct UFT_File_Type
{
	FILE* file;
	unsigned char* buffer;
	unsigned char* startBuf;
	size_t allocatedBytes;
} UFT_File_Type;

typedef struct UFT_Parse_Return
{
	E_UFT_PARSE_ERROR eError;
	unsigned int uiVersion;
	unsigned int uiType;
} UFT_Parse_Return;

unsigned char UFT_GetUChar( unsigned char** szBuf );
unsigned short UFT_GetUShort( unsigned char** szBuf );
unsigned int UFT_GetUInt( unsigned char** szBuf );
unsigned long long UFT_GetUInt64( unsigned char** szBuf );
char UFT_GetChar( unsigned char** szBuf );
short UFT_GetShort( unsigned char** szBuf );
int UFT_GetInt( unsigned char** szBuf );
float UFT_GetFloat( unsigned char** szBuf );
void UFT_GetString( unsigned char** szBuf, char* szRet );

// Setting Stuff
void UFT_SetUChar( UFT_File_Type* file, unsigned char uc );
void UFT_SetUShort( UFT_File_Type* file, unsigned short us );
void UFT_SetUInt( UFT_File_Type* file, unsigned int ui );
void UFT_SetUInt64( UFT_File_Type* file, unsigned long long ull );
void UFT_SetChar( UFT_File_Type* file, char uc );
void UFT_SetShort( UFT_File_Type* file, short us );
void UFT_SetInt( UFT_File_Type* file, int ui );
void UFT_SetInt64( UFT_File_Type* file, long long ull );

void UFT_SetStringA( UFT_File_Type* file, const char* sz );
void UFT_SetStringW( UFT_File_Type* file, const wchar_t* sz );

#ifdef _UNICODE
#define UFT_SetString UFT_SetStringW
#else
#define UFT_SetString UFT_SetStringA
#endif

// Variable stuff
size_t UFT_AddVarUChar( UFT_File_Type* file, unsigned char uc );
size_t UFT_AddVarUInt( UFT_File_Type* file, unsigned int ui );

void UFT_SetVarUChar( UFT_File_Type* file, size_t offset, unsigned char uc );
void UFT_SetVarUInt( UFT_File_Type* file, size_t offset, unsigned int ui );

unsigned char* UFT_CreateHeader( unsigned char* szBuf );
unsigned char* UFT_CreateHeaderReverse( unsigned char* szBuf );

const UFT_Parse_Return* UFT_ParseFromFile( FILE* file, void* callback );
const UFT_Parse_Return* UFT_ParseFromBuffer( const char* szBuffer, size_t iBufLen, void* callback );

UFT_File_Type* UFT_BeginFile( const char* szFileName );
E_UFT_FILE_ERROR UFT_EndFile( UFT_File_Type* file );

#ifdef __cplusplus
}
#endif
