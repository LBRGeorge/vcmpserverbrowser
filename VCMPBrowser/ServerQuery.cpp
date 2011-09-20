#include "ServerQuery.h"
#include <process.h>
#include "vcmpq/vcmpq.h"

SOCKET CServerQuery::m_pSocket = 0;
HANDLE CServerQuery::m_hThread = 0;

bool bThreadRunning = true;

void SplitIPAndPort( const char* sz, char szIP[], unsigned short& usPort, size_t uiIPBufSize = 16 )
{
	const char* szIPAndPort = sz;

	const char* szPort = strstr( szIPAndPort, ":" );

	if ( szPort ) 
	{
		*szPort++;
		if ( is_numA( szPort ) ) usPort = atoi( szPort );

		str_cpyA( szIP, szIPAndPort, ( szPort - szIPAndPort ) );
	}
	else
	{
		usPort = 0;

		str_cpyA( szIP, szIPAndPort, uiIPBufSize );
	}
}

unsigned __stdcall QueryListen( void* p )
{
	char buf[ 768 ] = { 0 };

	SOCKET pSocket = CServerQuery::m_pSocket;
	while ( ( bThreadRunning ) && ( pSocket ) )
	{
		sockaddr_in sa;
		sa.sin_family = AF_INET;
		int iLen = sizeof( sa );
		int uiBufLen = recvfrom( pSocket, buf, sizeof( buf ), 0, ( sockaddr* )&sa, &iLen );

		if ( uiBufLen != -1 ) 
		{
			// Below is VERY messy and rubbish. Needs redoing but meh
			unsigned int uiLen = 4;
			unsigned char szIP1[ 1 ] = { 0 };
			unsigned char szIP2[ 1 ] = { 0 };
			unsigned char szIP3[ 1 ] = { 0 };
			unsigned char szIP4[ 1 ] = { 0 };
			unsigned char szPort1[ 1 ] = { 0 };
			unsigned char szPort2[ 1 ] = { 0 };
			unsigned char szType[ 1 ] = { 0 };
			memcpy( &szIP1[ 0 ], buf + uiLen, 1 );
			uiLen++;
			memcpy( &szIP2[ 0 ], buf + uiLen, 1 );
			uiLen++;
			memcpy( &szIP3[ 0 ], buf + uiLen, 1 );
			uiLen++;
			memcpy( &szIP4[ 0 ], buf + uiLen, 1 );
			uiLen++;
			memcpy( &szPort1[ 0 ], buf + uiLen, 1 );
			uiLen++;
			memcpy( &szPort2[ 0 ], buf + uiLen, 1 );
			uiLen++;
			memcpy( &szType[ 0 ], buf + uiLen, 1 );
			uiLen++;

			char szIP[ 16 ] = { 0 };
			sprintf_s( szIP, "%i.%i.%i.%i", szIP1[ 0 ], szIP2[ 0 ], szIP3[ 0 ], szIP4[ 0 ] ); 
			unsigned short usPort = szPort1[0];
			unsigned short usPort2 = szPort2[0];
			usPort2 <<= 8;

			usPort += usPort2;

			CServers* pServer = CServerManager::Find( szIP, usPort );

			// Check if this server is valid
			if ( !pServer ) 
				continue;

			// Bad naming since it was converted from LU
			// Game ID is set to 1 here to say that the server is responding to querys so we can connect
			pServer->SetGameID( 1 );

			if ( szType[ 0 ] == 'i' )
			{
				char szIsPassworded[ 1 ] = { 0 };
				char szPlayers[ 2 ] = { 0 };
				char szMaxPlayers[ 2 ] = { 0 };
				char szNameLen[ 4 ] = { 0 };
				char szServerName[ 128 ] = { 0 };
				char szModeLen[ 4 ] = { 0 };
				char szModeName[ 64 ] = { 0 };
				char szMapLen[ 4 ] = { 0 };
				char szMapName[ 64 ] = { 0 };
				memcpy( &szIsPassworded[ 0 ], buf + uiLen, 1 );
				uiLen++;
				memcpy( &szPlayers[ 0 ], buf + uiLen, 2 );
				uiLen += 2;
				memcpy( &szMaxPlayers[ 0 ], buf + uiLen, 2 );
				uiLen += 2;
				memcpy( &szNameLen[ 0 ], buf + uiLen, 4 );
				uiLen += 4;
				unsigned int uiNameLen = (char)szNameLen[ 0 ];
				memcpy( &szServerName[ 0 ], buf + uiLen, uiNameLen );
				szServerName[ uiNameLen ] = 0;
				uiLen += uiNameLen;
				memcpy( &szModeLen[ 0 ], buf + uiLen, 4 );
				uiLen += 4;
				unsigned int uiModeLen = (char)szModeLen[ 0 ];
				memcpy( &szModeName[ 0 ], buf + uiLen, uiModeLen );
				szModeName[ uiModeLen ] = 0;
				uiLen += uiModeLen;
				memcpy( &szMapLen[ 0 ], buf + uiLen, 4 );
				uiLen += 4;
				unsigned int uiMapLen = (char)szMapLen[ 0 ];
				memcpy( &szMapName[ 0 ], buf + uiLen, uiMapLen );
				szMapName[ uiMapLen ] = 0;
				uiLen += uiMapLen;

				if ( (char)szIsPassworded[ 0 ] ) pServer->SetLocked( true );
				else pServer->SetLocked( false );

				pServer->SetServerName( szServerName );
				pServer->SetServerPlayers( (char)szPlayers[ 0 ] );
				pServer->SetServerMaxPlayers( (char)szMaxPlayers[ 0 ] );
				pServer->SetServerMode( szModeName );
				pServer->SetServerMap( szMapName );

				unsigned short usPing = clock() - pServer->GetServerQueryTime();
				if ( usPing > 9999 ) usPing = pServer->GetServerPing();
				pServer->SetServerPing( usPing );

				PostMessage( CMainWindow::GetMain(), WM_UPDATESERVERLIST, 0, (LPARAM)pServer );
			}

			else if ( szType[ 0 ] == 'c' )
			{
				// Player List
				char szPlayers[ 2 ] = { 0 };
				memcpy( &szPlayers[ 0 ], buf + uiLen, 1 );
				uiLen += 2;

				unsigned char ucPlayers = (unsigned char)szPlayers[ 0 ];
				unsigned char ucNameLen = 0;
				int iScore = 0;

				if ( pServer )
				{
					unsigned short usPing = clock() - pServer->GetServerQueryTime();
					if ( usPing > 9999 ) usPing = pServer->GetServerPing();
					pServer->SetServerPing( usPing );

					pServer->RemoveAllPlayers();

					for ( unsigned char uc = 0; uc < ucPlayers; uc++ )
					{
						char szNameLen[ 2 ] = { 0 };
						char szName[ 32 ] = { 0 };
						char szScore[ 4 ] = { 0 };

						memcpy( &szNameLen[ 0 ], buf + uiLen, 1 );
						uiLen++;

						ucNameLen = (unsigned char)szNameLen[ 0 ];
						memcpy( &szName[ 0 ], buf + uiLen, ucNameLen );
						szName[ ucNameLen ] = 0;
						uiLen += ucNameLen;
						memcpy( &szScore[ 0 ], buf + uiLen, 4 );
						uiLen += 4;

						iScore = (unsigned char)szScore[ 0 ];
						unsigned int uiScore2 = (unsigned char)szScore[1];
						uiScore2 <<= 8;

						iScore += uiScore2;

						unsigned int uiScore3 = (unsigned char)szScore[2];
						uiScore3 <<= 16;

						iScore += uiScore3;

						unsigned int uiScore4 = (unsigned char)szScore[3];
						uiScore4 <<= 24;

						iScore += uiScore4;

						if ( pServer )
						{
							CPlayers* pPlayer = pServer->AddPlayer( uc );
							pPlayer->SetName( szName );
						}
					}

					PostMessage( CMainWindow::GetMain(), WM_UPDATEPLAYERLIST, 0, (LPARAM)pServer );
				}
			}
		}


		pSocket = CServerQuery::m_pSocket;

		Sleep(0);
	}

	_endthreadex( 0 );

	return 0;
}

void CServerQuery::Init( void )
{
	WSADATA wsaData;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != 0 )
	{
		MessageBox( 0, _TEXT( "Failed to initialise WinSock" ), _TEXT( "Error" ), MB_OK );
	}

	m_pSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

	u_long arg=0;
	ioctlsocket(m_pSocket, FIONBIO, &arg);
}

void CServerQuery::Start( void )
{
	unsigned int threadID = 0;
	m_hThread = (HANDLE)_beginthreadex( NULL, 0, &QueryListen, NULL, 0, &threadID );
}

void CServerQuery::Stop( void )
{
	bThreadRunning = false;

	CloseHandle( m_hThread );

	closesocket( m_pSocket );
	m_pSocket = 0; 

	WSACleanup();
}

clock_t CServerQuery::Query( const char* szIP, unsigned short usPort )
{
	struct hostent     *he;
	if ( ( he = gethostbyname( szIP ) ) == NULL ) 
	{
		MessageBox( 0, _TEXT("Unable to resolve IP of server"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
		return 0;
	}

	SOCKADDR_IN	pTarget;
	pTarget.sin_family = AF_INET; // address family Internet
	pTarget.sin_port = htons( usPort ); //Port to connect on
	pTarget.sin_addr.s_addr = *( (unsigned long*)he->h_addr ); //Target IP

	static unsigned char szBuf[ 12 ]= { 0 };
	vcmpq_request pRequest;

	pRequest.ulIP = pTarget.sin_addr.s_addr;
	pRequest.usPort = usPort;
	pRequest.eRequestType = VCMPQ_HEADERTYPE_QUERY;
	VCMPQ_CreateQuery( pRequest, szBuf );
	
	if ( m_pSocket )
	{
		int iLen = sendto( m_pSocket, (char*)szBuf, 11, 0, (const sockaddr*)&pTarget, sizeof( struct sockaddr ) );

		if ( iLen == 0 )
		{
			MessageBox( 0, _TEXT("Failed to query"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
		}
	}

	return clock();
}

clock_t CServerQuery::QueryWithPlayers( const char* szIP, unsigned short usPort )
{

	struct hostent     *he;
	if ( ( he = gethostbyname( szIP ) ) == NULL ) 
	{
		MessageBox( 0, _TEXT("Unable to resolve IP of server"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
		return 0;
	}

	SOCKADDR_IN	pTarget;
	pTarget.sin_family = AF_INET; // address family Internet
	pTarget.sin_port = htons( usPort ); //Port to connect on
	pTarget.sin_addr.s_addr = *( (unsigned long*)he->h_addr ); //Target IP

	static unsigned char szBuf[ 12 ]= { 0 };
	vcmpq_request pRequest;

	pRequest.ulIP = pTarget.sin_addr.s_addr;
	pRequest.usPort = usPort;
	pRequest.eRequestType = VCMPQ_HEADERTYPE_QUERY;
	VCMPQ_CreateQuery( pRequest, szBuf );

	if ( m_pSocket )
	{
		int iLen = sendto( m_pSocket, (char*)szBuf, 11, 0, (const sockaddr*)&pTarget, sizeof( struct sockaddr ) );

		if ( iLen == 0 )
		{
			MessageBox( 0, _TEXT("Failed to query"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
		}
		else
		{
			pRequest.eRequestType = VCMPQ_HEADERTYPE_PLAYERS;
			VCMPQ_CreateQuery( pRequest, szBuf );

			iLen = sendto( m_pSocket, (char*)szBuf, 11, 0, (const sockaddr*)&pTarget, sizeof( struct sockaddr ) );

			if ( iLen == 0 )
			{
				MessageBox( 0, _TEXT("Failed to query"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
			}
		}
	}

	return clock();
}

void CServerQuery::UpdateInternetList( void )
{
	char* szList;
	int uiDataLen = http_fetch( "vicecitymultiplayer.com/servers.php", &szList );

	if ( uiDataLen == -1 )
	{
		MessageBox( 0, _TEXT( "Unable to contact master list" ), _TEXT( "Error" ), MB_ICONEXCLAMATION | MB_OK );
		return;
	}

	char* oldList = szList;

	char* context	= NULL;
	// Have to use strtok_r here since we use strtok in the Query, and it screws things over
	// Could be done better but this isnt the most amazing code ever
	szList = strtok_s( szList, "\n", &context );
	while ( szList ) 
	{
		unsigned short usPort = 0;
		char szIP[ 16 ] = { 0 };
		SplitIPAndPort( szList, szIP, usPort );
		if ( usPort )
		{
			CServers* pServer = CServerManager::Find( szIP, usPort );
			if ( !pServer )
			{
				pServer = CServerManager::New();
				if ( pServer )
				{
					pServer->SetServerIP( szIP );
					pServer->SetServerPort( usPort );
				}
			}

			if ( pServer )
			{
				if ( !(pServer->GetType() & E_ST_INTERNET) ) 
					pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_INTERNET) );

				pServer->Query();
			}
		}

		szList = strtok_s( 0, "\n", &context );
	}

	free( oldList );
}

void CServerQuery::UpdateOfficialList( void )
{
	char* szList;
	int uiDataLen = http_fetch( "vicecitymultiplayer.com//official.txt", &szList );

	if ( uiDataLen == -1 )
	{
		MessageBox( 0, _TEXT( "Unable to contact master list" ), _TEXT( "Error" ), MB_ICONEXCLAMATION | MB_OK );
		return;
	}

	char* oldList = szList;

	char* context	= NULL;
	szList = strtok_s( szList, "\n", &context );
	while ( szList ) 
	{
		unsigned short usPort = 0;
		char szIP[ 16 ] = { 0 };
		SplitIPAndPort( szList, szIP, usPort );
		if ( usPort )
		{
			CServers* pServer = CServerManager::Find( szIP, usPort );
			if ( !pServer )
			{
				pServer = CServerManager::New();
				if ( pServer )
				{
					pServer->SetServerIP( szIP );
					pServer->SetServerPort( usPort );
				}
			}

			if ( pServer )
			{
				if ( !(pServer->GetType() & E_ST_OFFICIAL) ) 
					pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_OFFICIAL) );

				pServer->Query();
			}
		}

		szList = strtok_s( 0, "\n", &context );
	}

	free( oldList );
}