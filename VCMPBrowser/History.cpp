#include "History.h"
#include "Servers.h"

#ifndef _NO_HISTORY

extern TCHAR g_szAppDataDir[ MAX_PATH ];

void CHistory::ParseData( unsigned char* szBuf )
{
	unsigned int uiServers = UFT_GetUInt( &szBuf );

	char szIP[ 64 ] = { 0 };
	unsigned short usPort = 0;
	unsigned int uiLastPlayed = 0;
	CServers* pServer = NULL;

	for ( unsigned int ui = 0; ui < uiServers; ui++ )
	{
		UFT_GetString( &szBuf, szIP );
		usPort = UFT_GetUShort( &szBuf );
		uiLastPlayed = UFT_GetUInt( &szBuf );

		pServer = CServerManager::Find( szIP, usPort );
		if ( !pServer )
		{
			pServer = CServerManager::New();
			if ( pServer )
			{
				pServer->SetServerIP( szIP );
				pServer->SetServerPort( usPort );

				pServer->SetServerName( "(Waiting for server data...)" );

				pServer->SetType( E_ST_HISTORY );

				pServer->Query();				
			}
		}

		if ( pServer )
		{
			CHistoryServer* pHist =	CHistoryServerManager::New( pServer );
			if ( pHist )
			{
				pHist->SetLastPlayed( uiLastPlayed );
				pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_HISTORY) );

				PostMessage( CMainWindow::GetMain(), WM_UPDATEHISTORYSERVERLIST, 0, (LPARAM)pHist );
			}
		}
	}
}

void CHistory::Load( void )
{
	TCHAR szFile[ 384 ] = { NULL };
	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "\\VCMP\\History.uft" ) );

	FILE* file = fopen( szFile, "r" );
	if ( file )
	{
		UFT_ParseFromFile( file, ParseData );

		fclose( file );
	}
}

void CHistory::Write( void )
{
	TCHAR szFile[ 384 ] = { NULL };
	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "\\VCMP\\History.uft" ) );

	unsigned int uiHistoryServers = CHistoryServerManager::Count();

	UFT_File_Type* pFile = UFT_BeginFile( szFile );

	if ( pFile )
	{
		UFT_SetUInt( pFile, uiHistoryServers );

		CHistoryServer* pHist = 0;
		CServers* pServer = 0;
		for ( size_t ui = 0; ui < CHistoryServerManager::Count(); ui++ )
		{
			pHist = CHistoryServerManager::Find( ui );
			if ( pHist )
			{
				pServer = pHist->GetServer();
				if ( pServer )
				{
					UFT_SetString( pFile, pServer->GetServerIP() );
					UFT_SetUShort( pFile, pServer->GetServerPort() );
					UFT_SetUInt( pFile, pHist->GetLastPlayed() );
				}
			}
		}

		UFT_EndFile( pFile );
	}
}
#endif
