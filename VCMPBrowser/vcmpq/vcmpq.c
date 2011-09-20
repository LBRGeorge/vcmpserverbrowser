#include <memory.h>
#include <time.h>
#include <malloc.h>
#include <string.h>

#include "vcmpq.h"

void memand( void* pDest, const void* pSource, int size) 
{
	unsigned char* pDest2 = (unsigned char*)pDest;
	const unsigned char* pSource2 = (const unsigned char*)pSource;

	for ( ; size; --size)
		*pDest2++ &= *pSource2++;
}

unsigned char VCMPQ_GetUChar( const unsigned char** szBuf )
{
	return * (*szBuf)++;
}

unsigned short VCMPQ_GetUShort( const unsigned char** szBuf )
{
	unsigned short usValue = * (const unsigned short*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

unsigned int VCMPQ_GetUInt( const unsigned char** szBuf )
{
	unsigned int uiValue = * (const unsigned int*)*szBuf;
	*szBuf += sizeof( uiValue );
	return uiValue;
}

unsigned long long VCMPQ_GetUInt64( const unsigned char** szBuf )
{
	unsigned long long ullValue = * (const unsigned long long*)*szBuf;
	*szBuf += sizeof( ullValue );
	return ullValue;
}

char VCMPQ_GetChar( const unsigned char** szBuf )
{
	return * (*szBuf)++;
}

short VCMPQ_GetShort( const unsigned char** szBuf )
{
	short usValue = * (const short*)*szBuf;
	*szBuf += sizeof( usValue );
	return usValue;
}

int VCMPQ_GetInt( const unsigned char** szBuf )
{
	int uiValue = * (const int*)*szBuf;
	*szBuf += sizeof( uiValue );
	return uiValue;
}

float VCMPQ_GetFloat( const unsigned char** szBuf )
{
	float fValue = * (const float*)*szBuf;
	*szBuf += sizeof( fValue );
	return fValue;
}

void VCMPQ_GetString( const unsigned char** szBuf, char* szRet )
{
	unsigned int uiLen = VCMPQ_GetUChar( szBuf );

	memcpy( szRet, (*szBuf), uiLen );
	*szBuf += uiLen;
}

void VCMPQ_SetUChar( vcmpq_parse_info* pPI, unsigned char ucValue )
{
	*pPI->szSndBufEnd++ = ucValue;
}

void VCMPQ_SetUShort( vcmpq_parse_info* pPI, unsigned short usValue )
{
	*(unsigned short*)pPI->szSndBufEnd = usValue;
	pPI->szSndBufEnd += sizeof( usValue );
}

void VCMPQ_SetUInt( vcmpq_parse_info* pPI, unsigned int uiValue )
{
	*(unsigned int*)pPI->szSndBufEnd = uiValue;
	pPI->szSndBufEnd += sizeof( uiValue );
}

void VCMPQ_SetUInt64( vcmpq_parse_info* pPI, unsigned long long ullValue )
{
	*(unsigned long long*)pPI->szSndBufEnd = ullValue;
	pPI->szSndBufEnd += sizeof( ullValue );
}

void VCMPQ_SetChar( vcmpq_parse_info* pPI, char ucValue )
{
	*pPI->szSndBufEnd++ = ucValue;
}

void VCMPQ_SetShort( vcmpq_parse_info* pPI, short usValue )
{
	*(short*)pPI->szSndBufEnd = usValue;
	pPI->szSndBufEnd += sizeof( usValue );
}

void VCMPQ_SetInt( vcmpq_parse_info* pPI, int uiValue )
{
	*(int*)pPI->szSndBufEnd = uiValue;
	pPI->szSndBufEnd += sizeof( uiValue );
}

void VCMPQ_SetFloat( vcmpq_parse_info* pPI, float fValue )
{
	*(float*)pPI->szSndBufEnd = fValue;
	pPI->szSndBufEnd += sizeof( fValue );
}

void VCMPQ_SetString( vcmpq_parse_info* pPI, const unsigned char* szAddBuf )
{
	size_t uiLen = strlen( szAddBuf ) + 1;

	if ( uiLen > VCMPQ_MAX_STRING_LEN ) return;

	if ( pPI->szSndBufEnd + uiLen >= pPI->szSndBufMax )
	{
		pPI->szSndBufEnd = pPI->szSndBufMax;
		return;
	}

	VCMPQ_SetUChar( pPI, (unsigned char)uiLen );

	memcpy( pPI->szSndBufEnd, szAddBuf, uiLen );
	pPI->szSndBufEnd += uiLen;
}

void VCMPQ_CreateQuery( vcmpq_request pRequest, unsigned char* szBuf )
{
	*(unsigned int*)szBuf = * (unsigned int*)"VCMP";
	szBuf += 4;

	*(unsigned int*)szBuf = pRequest.ulIP; // IP
	szBuf += 4;

	*(unsigned short*)szBuf = pRequest.usPort; // Port
	szBuf += 2;

	// Header Type
	*szBuf++ = (unsigned char)pRequest.eRequestType;
}