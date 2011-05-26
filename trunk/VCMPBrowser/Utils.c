#include "Utils.h"
#include <direct.h>
#include "errno.h"

void str_cpyA( char* szDest, const char* szSource, unsigned int size )
{
	if ( szSource )
	{
		while ( ( --size ) && ( *szSource ) )
		{
			*szDest++ = *szSource++;
		}
	}

	*szDest = '\0';
}

void str_cpyW( wchar_t* szDest, const wchar_t* szSource, unsigned int size )
{
	if ( szSource )
	{
		while ( ( --size ) && ( *szSource ) )
		{
			*szDest++ = *szSource++;
		}
	}

	*szDest = '\0';
}

void str_catA( char* szDest, const char* szSource, size_t size )
{
	if ( szSource )
	{
		while ( ( --size ) && ( *szDest ) )
		{
			*szDest++;
		}

		while ( ( --size ) && ( *szSource ) )
		{
			*szDest++ = *szSource++;
		}
	}

	*szDest = '\0';
}

void str_catW( wchar_t* szDest, const wchar_t* szSource, size_t size )
{
	if ( szSource )
	{
		while ( ( --size ) && ( *szDest ) )
		{
			*szDest++;
		}

		while ( ( --size ) && ( *szSource ) )
		{
			*szDest++ = *szSource++;
		}
	}

	*szDest = '\0';
}

int str_equalA( const char* s1, const char* s2 )
{
	while ( *s1 == *s2++ )
	{
		if ( !*s1++ ) return 1;
	}

	return 0;
}

int str_equalW( const wchar_t* s1, const wchar_t* s2 )
{
	while ( *s1 == *s2++ )
	{
		if ( !*s1++ ) return 1;
	}

	return 0;
}

int str_prefixA( const char* s1, const char* s2, size_t len )
{
	while ( *s1 == *s2++ )
	{
		if ( ( !--len ) || ( !*s1++ ) ) return 1;
	}

	return 0;
}

int str_prefixW( const wchar_t* s1, const wchar_t* s2, size_t len )
{
	while ( *s1 == *s2++ )
	{
		if ( ( --len ) || ( !*s1++ ) ) return 1;
	}

	return 0;
}

int is_numA( const char* sz )
{
	const char *s=sz;

	if (!*s) return 0;
	for(;;)
	{
		if (!*s) break; if ( *s < '0' || *s > '9' ) return 0; ++s;
		if (!*s) break; if ( *s < '0' || *s > '9' ) return 0; ++s;
		if (!*s) break; if ( *s < '0' || *s > '9' ) return 0; ++s;
		if (!*s) break; if ( *s < '0' || *s > '9' ) return 0; ++s;
	}
	
	return 1;
}


void CreateDirectoryFromPath( char* szPath )
{
	int index = 0;
	int res = 0;

	if (szPath[index])
	{
		index = 1;
		while ( szPath[ index ] )
		{
			if ( szPath[ index ] == '/' || szPath[ index ] == '\\')
			{
				szPath[ index ] = 0;
	
				res = _mkdir( szPath );

				if (res<0 && errno!=EEXIST && errno!=EACCES)
				{
					return;
				}
	
				szPath[ index ] = '/';
			}
	
			index++;
		}
	}
}