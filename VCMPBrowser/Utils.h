#pragma once

#include <tchar.h>

#ifdef __cplusplus
	extern "C" {
#endif

void str_cpyA( char* szDest, const char* szSource, unsigned int size );
void str_cpyW( wchar_t* szDest, const wchar_t* szSource, unsigned int size );
void str_catA( char* szDest, const char* szSource, unsigned int size );
void str_catW( wchar_t* szDest, const wchar_t* szSource, unsigned int size );
int str_equalA( const char* s1, const char* s2 );
int str_equalW( const wchar_t* s1, const wchar_t* s2 );
int str_prefixA( const char* s1, const char* s2, size_t len );
int str_prefixW( const wchar_t* s1, const wchar_t* s2, size_t len );
int is_numA( const char* sz );
int is_numW( const wchar_t* sz );

void CreateDirectoryFromPath( char* szPath );

#ifdef _UNICODE
#define str_cpy str_cpyW
#define str_equal str_equalW
#define str_prefix str_prefixW
#define is_num is_numW
#else
#define str_cpy str_cpyA
#define str_equal str_equalA
#define str_prefix str_prefixA
#define is_num is_numA
#endif

#ifdef __cplusplus
}
#endif