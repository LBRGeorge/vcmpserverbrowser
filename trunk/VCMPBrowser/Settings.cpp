#include "Settings.h"

TCHAR CSettings::m_szName[ 32 ] = { _TEXT( "UnknownPlayer" ) };
TCHAR CSettings::m_szGTAVCPath[ MAX_PATH ] = { 0 };

extern TCHAR g_szAppDataDir[MAX_PATH];

void CSettings::LoadSettings( void )
{
	HKEY hKey;
	char szDir[ MAX_PATH ] = { 0 };
	DWORD dwBufLen = sizeof( szDir );

	// Load the stuff from the registry key. This should be set by the installer
	if( RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\VCMP", NULL, KEY_READ, &hKey ) == ERROR_SUCCESS)
	{
		DWORD dwType = REG_SZ;
		if ( !*m_szGTAVCPath )
		{
			if ( RegQueryValueEx(hKey,"gtavc_exe",NULL,&dwType,(LPBYTE)szDir, &dwBufLen )  == ERROR_SUCCESS)
			{
				if ( *szDir )
					str_cpyA( m_szGTAVCPath, szDir, sizeof( m_szGTAVCPath ) );
			}
		}

		if ( RegQueryValueEx(hKey,"PlayerName",NULL,&dwType,(LPBYTE)szDir, &dwBufLen ) == ERROR_SUCCESS)
		{
			if ( *szDir )
				str_cpyA( m_szName, szDir, sizeof( m_szName ) );
		}

		RegCloseKey( hKey );
	}
}

void CSettings::WriteSettings( void )
{
	HKEY hKey;
	char szDir[ MAX_PATH ] = { 0 };

	if( RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\VCMP", NULL, 0, 0, KEY_WRITE, 0, &hKey, 0 ) == ERROR_SUCCESS)
	{

		if ( *m_szGTAVCPath )
		{
			RegSetValueEx(hKey,"gtavc_exe",NULL,REG_SZ,(LPBYTE)&m_szGTAVCPath, strlen(m_szGTAVCPath) );
		}

		if ( *m_szName )
		{
			RegSetValueEx(hKey,"PlayerName",NULL,REG_SZ,(LPBYTE)&m_szName, strlen(m_szName) );
		}

		RegCloseKey( hKey );
	}
}
