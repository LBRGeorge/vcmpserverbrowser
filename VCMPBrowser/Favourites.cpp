#include "Favourites.h"
#include "Servers.h"

extern TCHAR g_szAppDataDir[ MAX_PATH ];

void CFavourites::ParseData( unsigned char* szBuf )
{
	unsigned int uiServers = UFT_GetUInt( &szBuf );

	char szIP[ 64 ] = { 0 };
	unsigned short usPort = 0;
	CServers* pServer = NULL;

	for ( unsigned int ui = 0; ui < uiServers; ui++ )
	{
		UFT_GetString( &szBuf, szIP );
		usPort = UFT_GetUShort( &szBuf );

		pServer = CServerManager::New();
		if ( pServer )
		{
			pServer->SetServerIP( szIP );
			pServer->SetServerPort( usPort );

			pServer->SetServerName( "(Waiting for server data...)" );

			pServer->SetType( E_ST_FAV );

			pServer->Query();

			//pServer->UpdateList();
			PostMessage( CMainWindow::GetMain(), WM_UPDATESERVERLIST, 0, (LPARAM)pServer );
		}
	}
}

void CFavourites::ParsePasswordData( unsigned char* szBuf )
{
	unsigned int uiServers = UFT_GetUChar( &szBuf );

	char szIP[ 64 ] = { 0 };
	unsigned short usPort = 0;
	char szPass[ 64 ] = { 0 };
	CServers* pServer = NULL;

	for ( unsigned int ui = 0; ui < uiServers; ui++ )
	{
		UFT_GetString( &szBuf, szIP );
		usPort = UFT_GetUShort( &szBuf );
		UFT_GetString( &szBuf, szPass );

		pServer = CServerManager::Find( szIP, usPort );
		if ( pServer )
		{
			pServer->SetServerPass( szPass );
		}
	}
}

void CFavourites::Load( void )
{
	TCHAR szFile[ 384 ] = { NULL };
	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "/VCMP/Favourites.uft" ) );

	FILE* file = fopen( szFile, "r" );
	if ( file )
	{
		UFT_ParseFromFile( file, ParseData );

		fclose( file );
	}

	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "/VCMP/Passwords.uft" ) );

	file = fopen( szFile, "r" );
	if ( file )
	{
		UFT_ParseFromFile( file, ParsePasswordData );

		fclose( file );
	}
}

void CFavourites::Write( void )
{
	TCHAR szFile[ 384 ] = { NULL };
	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "/VCMP/Favourites.uft" ) );

	unsigned int uiFavServers = CServerManager::CountType( E_ST_FAV );

	UFT_File_Type* pFile = UFT_BeginFile( szFile );

	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "/VCMP/Passwords.uft" ) );

	UFT_File_Type* pPassFile = UFT_BeginFile( szFile );

	unsigned int uiPasswordedServers = 0;
	size_t uiFilesOffset = 0;

	if ( pFile )
	{
		UFT_SetUInt( pFile, uiFavServers );
		if ( pPassFile )
		{
			uiFilesOffset = UFT_AddVarUChar( pPassFile, uiPasswordedServers );
		}

		for ( size_t ui = 0; ui < CServerManager::Count(); ui++ )
		{
			CServers* p = CServerManager::Find( ui );
			if ( p )
			{
				if ( p->GetType() & E_ST_FAV )
				{
					UFT_SetString( pFile, p->GetServerIP() );
					UFT_SetUShort( pFile, p->GetServerPort() );

					if ( ( pPassFile ) && ( p->GetServerPass() ) )
					{
						UFT_SetString( pPassFile, p->GetServerIP() );
						UFT_SetUShort( pPassFile, p->GetServerPort() );
						UFT_SetString( pPassFile, p->GetServerPass() );

						uiPasswordedServers++;
					}
				}
			}
		}

		UFT_EndFile( pFile );
		if ( pPassFile )
		{
			UFT_SetVarUChar( pPassFile, uiFilesOffset, uiPasswordedServers );
			UFT_EndFile( pPassFile );
		}
	}
}