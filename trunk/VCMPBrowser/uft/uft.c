#include "uft.h"
#include <malloc.h>
#include <string.h>


//typedef void * (__stdcall *UTF_Parse_Callback)( unsigned char* szBuf );

UFT_Parse_Return pReturn;

void UFT_CheckBuffer( UFT_File_Type* file, const size_t sizeToAdd )
{
	size_t bufferLen = (size_t)( file->buffer - file->startBuf );
	if ( ( bufferLen + sizeToAdd ) >= file->allocatedBytes )
	{
		// Increase the buffer!
		unsigned char* szNewBuf = (unsigned char*)realloc( file->startBuf, file->allocatedBytes + (sizeToAdd + UFT_STACK_ALLOCATION_SIZE) );

		if ( !szNewBuf )
			return; // ERROR!

		/*memcpy( szNewBuf, file->startBuf, bufferLen );

		free( file->startBuf );*/

		file->startBuf = szNewBuf;
		file->buffer = (szNewBuf += bufferLen);

		// We allocate the ammount of data we need plus the stack size to be safe!
		file->allocatedBytes += (sizeToAdd + UFT_STACK_ALLOCATION_SIZE);
	}
}

unsigned char UFT_GetUChar( unsigned char** szBuf )
{
	return * (*szBuf)++;
}

unsigned short UFT_GetUShort( unsigned char** szBuf )
{
	unsigned short usValue = * (unsigned short*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

unsigned int UFT_GetUInt( unsigned char** szBuf )
{
	unsigned int usValue = * (unsigned int*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

unsigned long long UFT_GetUInt64( unsigned char** szBuf )
{
	unsigned long long usValue = * (unsigned long long*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

char UFT_GetChar( unsigned char** szBuf )
{
	return * (*szBuf)++;
}

short UFT_GetShort( unsigned char** szBuf )
{
	short usValue = * (short*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

int UFT_GetInt( unsigned char** szBuf )
{
	int usValue = * (int*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

float UFT_GetFloat( unsigned char** szBuf )
{
	float fValue = * (const float*)*szBuf;
	*szBuf += sizeof( fValue );
	return fValue;
}

void UFT_GetString( unsigned char** szBuf, char* szRet )
{
	unsigned short usLen = UFT_GetUShort( szBuf );

	memcpy( szRet, (*szBuf), usLen );
	*szBuf += usLen;
}

void UFT_SetUChar( UFT_File_Type* file, unsigned char uc )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( uc ) );

	*file->buffer++ = uc;
}

void UFT_SetUShort( UFT_File_Type* file, unsigned short us )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( us ) );

	*(unsigned short*)file->buffer = us;
	file->buffer += sizeof( us );
}

void UFT_SetUInt( UFT_File_Type* file, unsigned int ui )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( ui ) );

	*(unsigned int*)file->buffer = ui;
	file->buffer += sizeof( ui );
}

void UFT_SetUInt64( UFT_File_Type* file, unsigned long long ull )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( ull ) );

	*(unsigned long long*)file->buffer = ull;
	file->buffer += sizeof( ull );
}

void UFT_SetChar( UFT_File_Type* file, char uc )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( uc ) );

	*file->buffer++ = uc;
}

void UFT_SetShort( UFT_File_Type* file, short us )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( us ) );

	*(short*)file->buffer = us;
	file->buffer += sizeof( us );
}

void UFT_SetInt( UFT_File_Type* file, int ui )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( ui ) );

	*(int*)file->buffer = ui;
	file->buffer += sizeof( ui );
}

void UFT_SetInt64( UFT_File_Type* file, long long ull )
{
	if ( !file )
		return;

	UFT_CheckBuffer( file, sizeof( ull ) );

	*(long long*)file->buffer = ull;
	file->buffer += sizeof( ull );
}

void UFT_SetStringA( UFT_File_Type* file, const char* sz )
{
	size_t uiLen = strlen( sz ) + 1;

	if ( uiLen > UFT_MAX_STRING_LEN ) 
		return;

	UFT_CheckBuffer( file, sizeof( unsigned short ) + uiLen );

	UFT_SetUShort( file, uiLen );

	memcpy( file->buffer, sz, uiLen );
	file->buffer += uiLen;
}

void UFT_SetStringW( UFT_File_Type* file, const wchar_t* sz )
{
	size_t uiLen = wcslen( sz ) + 1;

	if ( uiLen > UFT_MAX_STRING_LEN ) 
		return;

	UFT_CheckBuffer( file, sizeof( unsigned short ) + uiLen );

	UFT_SetUShort( file, uiLen );

	memcpy( file->buffer, sz, uiLen );
	file->buffer += uiLen;
}

size_t UFT_AddVarUChar( UFT_File_Type* file, unsigned char uc )
{
	size_t offset;

	if ( !file )
		return 0;

	UFT_CheckBuffer( file, sizeof( uc ) );

	*(unsigned char*)file->buffer = uc;
	offset = (size_t)( file->buffer - file->startBuf );
	file->buffer++;

	return offset;
}

size_t UFT_AddVarUInt( UFT_File_Type* file, unsigned int ui )
{
	size_t offset;

	if ( !file )
		return 0;

	UFT_CheckBuffer( file, sizeof( ui ) );

	*(unsigned int*)file->buffer = ui;
	offset = (size_t)( file->buffer - file->startBuf );
	file->buffer += sizeof( ui );

	return offset;
}

void UFT_SetVarUChar( UFT_File_Type* file, size_t offset, unsigned char uc )
{
	*(unsigned char*)( file->startBuf + offset ) = uc;
}

void UFT_SetVarUInt( UFT_File_Type* file, size_t offset, unsigned int ui )
{
	*(unsigned int*)( file->startBuf + offset ) = ui;
}

unsigned char* UFT_CreateHeader( unsigned char* szBuf )
{
	*(unsigned short*)szBuf = 0xFFFF;
	szBuf += 2;

	*(unsigned int*)szBuf = * (unsigned int*)"UFT\0";
	szBuf += 4;

	// Protocol Version
	*szBuf++ = _UFT_VER;

	// File Type
	*szBuf++ = E_UFT_PLAINTEXT;

	return szBuf;
}

unsigned char* UFT_CreateHeaderReverse( unsigned char* szBuf )
{
	// Protocol Version
	*--szBuf = E_UFT_PLAINTEXT;

	*--szBuf = _UFT_VER;

	szBuf -= 4;
	*(unsigned int*)szBuf = * (unsigned int*)"UFT\0";

	szBuf -= 2;
	*(unsigned short*)szBuf = 0xFFFF;

	return szBuf;
}

const UFT_Parse_Return* UFT_ParseFromFile( FILE* file, void* callback )
{
	long lFileLen = 0;
	unsigned char *szFileBuf = 0, *szOrigBuf = 0;
	UFT_Parse_Callback pCallback;

	// This gets the file length
	fseek( file, 0L, SEEK_END );  /* Position to end of file */
	lFileLen = ftell( file );     /* Get file length */
	rewind( file );               /* Back to start of file */

	pReturn.eError = E_UPE_NONE;

	if ( lFileLen < 6 )
	{
		pReturn.eError = E_UPE_HEADER_TOO_SMALL;
		return &pReturn; // Header too small
	}

	szFileBuf = (unsigned char*)calloc( lFileLen + 1, sizeof( char ) );

	szOrigBuf = szFileBuf;

	if ( !szFileBuf )
	{
		pReturn.eError = E_UPE_NO_MEMORY;
		return &pReturn; // No Memory!
	}

	fread( szFileBuf, lFileLen, 1, file );

	if ( *(unsigned short*)szFileBuf != 0xFFFF )
	{
		pReturn.eError = E_UPE_NOT_UFT;
		return &pReturn; // Not a UFT file
	}

	if ( *(unsigned int*)( szFileBuf + 2 ) != *(unsigned int*)"UFT\0" )
	{
		pReturn.eError = E_UPE_NOT_UFT;
		return &pReturn; // Not a UFT file
	}

	szFileBuf += 6;

	pReturn.uiVersion = *szFileBuf++;
	// Version Mismatch
	if ( pReturn.uiVersion != _UFT_VER )
	{
		pReturn.eError = E_UPE_WRONG_VER;
		return &pReturn; // Wrong Version
	}

	pReturn.uiType = *szFileBuf++;
	// Type Mismatch
	if ( ( pReturn.uiType != E_UFT_UNUSED ) && ( pReturn.uiType != E_UFT_PLAINTEXT ) )
	{
		pReturn.eError = E_UPE_UNSUPPORTED;
		return &pReturn; // Unsupported Type
	}

	pCallback = (UFT_Parse_Callback)callback;
	pCallback( szFileBuf );

	free( szOrigBuf );

	return &pReturn;
}

const UFT_Parse_Return* UFT_ParseFromBuffer( const char* szBuffer, size_t iBufLen, void* callback )
{
	long lFileLen = 0;
	unsigned char *szFileBuf = 0, *szOrigBuf = 0;
	UFT_Parse_Callback pCallback;

	pReturn.eError = E_UPE_NONE;

	if ( iBufLen < 6 )
	{
		pReturn.eError = E_UPE_HEADER_TOO_SMALL;
		return &pReturn; // File is smaller than the UFT header
	}

	if ( *(unsigned short*)szBuffer != 0xFFFF )
	{
		pReturn.eError = E_UPE_NOT_UFT;
		return &pReturn; // File is not a UFT file
	}

	if ( *(unsigned int*)( szBuffer + 2 ) != *(unsigned int*)"UFT\0" )
	{
		pReturn.eError = E_UPE_NOT_UFT;
		return &pReturn; // File is not a UFT file
	}

	szBuffer += 6;

	pReturn.uiVersion = *szBuffer++;
	// Version Mismatch
	if ( pReturn.uiVersion != _UFT_VER )
	{
		pReturn.eError = E_UPE_WRONG_VER;
		return &pReturn; // Wrong Version
	}

	pReturn.uiType = *szBuffer++;
	// Type Mismatch
	if ( ( pReturn.uiType != E_UFT_UNUSED ) && ( pReturn.uiType != E_UFT_PLAINTEXT ) )
	{
		pReturn.eError = E_UPE_UNSUPPORTED;
		return &pReturn; // Unsupported Type
	}

	pCallback = (UFT_Parse_Callback)callback;
	pCallback( szBuffer );

	return &pReturn;
}

UFT_File_Type* UFT_BeginFile( const char* szFileName )
{
#if UFT_STACK_ALLOCATION_SIZE <= 7
#error UFT_STACK_ALLOCATION_SIZE Buffer is set too low (Minumum: 7)
#endif

	UFT_File_Type *pFile;
	FILE* file;

	pFile = (UFT_File_Type*)calloc( sizeof( UFT_File_Type ), sizeof( UFT_File_Type* ) );
	
	if ( !pFile )
		return 0;

	file = fopen( szFileName, "wb+" );
	if ( !file )
		return 0;

	pFile->file = file;
	pFile->buffer = (unsigned char*)calloc( UFT_STACK_ALLOCATION_SIZE, sizeof( unsigned char ) );
	// Since buffer will get moved around etc, we save a pointer to the start of the buffer
	pFile->startBuf = pFile->buffer;
	pFile->allocatedBytes = UFT_STACK_ALLOCATION_SIZE;

	if ( !pFile->buffer )
	{
		// Memory fuckup!
		free( pFile );
		return 0;
	}

	// Add the header
	pFile->buffer = UFT_CreateHeader( pFile->buffer );
	//pFile->bufferLen = 7;

	return pFile;
}

E_UFT_FILE_ERROR UFT_EndFile( UFT_File_Type* file )
{
	size_t bufferLen = 0;
	if ( !file )
		return E_UFE_INVALIDPTR;

	if ( !file->file )
		return E_UFE_INVALIDFILE;

	bufferLen = (size_t)( file->buffer - file->startBuf );
	// Write the buffer
	fwrite( file->startBuf, bufferLen, 1, file->file );

	// Close the opened file
	fclose( file->file );

	file->file = 0;
	file->buffer = 0;

	// Free the buffer
	free( file->startBuf );
	file->startBuf = 0;

	free( file );
	file = 0;

	return E_UFE_NONE;
}
