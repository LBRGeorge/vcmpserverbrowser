#include "Servers.h"
#include "ServerQuery.h"

CServers::CServers( size_t uiID )
: m_uiID( uiID ), m_eType( E_ST_NONE ), m_usPort( 0 ), m_szPass( 0 ), m_ucGame( 0 ), m_bLocked( false ), m_ucServerPlayers( 0 ), 
m_szServerName( 0 ), m_szServerMode( 0 ), m_szServerMap( 0 ), m_ucServerMaxPlayers( 0 ), m_usServerPing( 9999 ), m_cQueryTime(0)
{
	*m_szIP = 0;

	SetServerName( "Unknown Server" );

	// Favs List
	m_uiListItem[ E_LT_FAV ] = -1;
	// Internet List
	m_uiListItem[ E_LT_INTERNET ] = -1;
	// Official List
	m_uiListItem[ E_LT_OFFICIAL ] = -1;
	// History List
	m_uiListItem[ E_LT_HISTORY ] = -1;
	// LAN List
	m_uiListItem[ E_LT_LAN ] = -1;

	for ( size_t ui = 0; ui < MAX_PLAYERS; ui++ )
		m_pPlayerList[ ui ] = 0;
}

CServers::CServers( size_t uiID, const CServers& p )
: m_uiID( uiID ), m_eType( p.GetType() ), m_usPort( p.GetServerPort() ), m_ucGame( p.GetGameID() ), m_bLocked( p.GetLocked() ), m_ucServerPlayers( p.GetServerPlayers() ), 
m_ucServerMaxPlayers( p.GetServerMaxPlayers() ), m_usServerPing( p.GetServerPing() ), m_cQueryTime(0)
{
	str_cpy( m_szIP, p.GetServerIP(), sizeof( m_szIP ) );

	SetServerName( p.GetServerName() );
	SetServerMode( p.GetServerMode() );
	SetServerMap( p.GetServerMap() );
	SetServerPass( p.GetServerPass() );

	// Favs List
	m_uiListItem[ E_LT_FAV ] = p.GetListItem( E_LT_FAV );
	// Internet List
	m_uiListItem[ E_LT_INTERNET ] = p.GetListItem( E_LT_INTERNET );
	// Official List
	m_uiListItem[ E_LT_OFFICIAL ] = p.GetListItem( E_LT_OFFICIAL );
	// History List
	m_uiListItem[ E_LT_HISTORY ] = p.GetListItem( E_LT_HISTORY );
	// LAN List
	m_uiListItem[ E_LT_LAN ] = p.GetListItem( E_LT_LAN );

	for ( size_t ui = 0; ui < MAX_PLAYERS; ui++ )
		m_pPlayerList[ ui ] = p.GetPlayer( ui );
}

CServers::~CServers(void)
{
	RemoveAllPlayers();

	if ( m_szServerName )
	{
		free( m_szServerName );
		m_szServerName = 0;
	}

	if ( m_szServerMode )
	{
		free( m_szServerMode );
		m_szServerMode = 0;
	}

	if ( m_szServerMap )
	{
		free( m_szServerMap );
		m_szServerMap = 0;
	}

	if ( m_szPass )
	{
		free( m_szPass );
		m_szPass = 0;
	}
}

void CServers::SetServerName( const char* sz )
{
	size_t uiLen = strlen( sz )+1;
	m_szServerName = (char*)realloc( m_szServerName, uiLen );
	str_cpyA( m_szServerName, sz, uiLen );
}

void CServers::SetServerMode( const char* sz )
{
	size_t uiLen = strlen( sz )+1;
	m_szServerMode = (char*)realloc( m_szServerMode, uiLen );
	str_cpyA( m_szServerMode, sz, uiLen );
}

void CServers::SetServerMap( const char* sz )
{
	size_t uiLen = strlen( sz )+1;
	m_szServerMap = (char*)realloc( m_szServerMap, uiLen );
	str_cpyA( m_szServerMap, sz, uiLen );
}

void CServers::SetServerPass( const char* sz )
{
	size_t uiLen = strlen( sz )+1;
	m_szPass = (char*)realloc( m_szPass, uiLen );
	str_cpyA( m_szPass, sz, uiLen );
}

CPlayers* CServers::AddPlayer( size_t ui )
{
	if ( ui < MAX_PLAYERS )
	{
		CPlayers* pPlayer = new CPlayers( ui );
		m_pPlayerList[ ui ] = pPlayer;

		return pPlayer;
	}
	return 0;
}

bool CServers::RemovePlayer( size_t ui )
{
	if ( ui < MAX_PLAYERS )
	{
		CPlayers* pPlayer = m_pPlayerList[ ui ];
		if ( pPlayer )
		{
			delete pPlayer;
			m_pPlayerList[ ui ] = 0;

			return true;
		}
	}
	return false;
}

bool CServers::RemovePlayer( CPlayers* p )
{
	if ( p )
	{
		size_t ui = p->GetID();

		if ( ui < MAX_PLAYERS )
		{
			CPlayers* pPlayer = m_pPlayerList[ ui ];
			if ( pPlayer )
			{
				delete pPlayer;
				m_pPlayerList[ ui ] = 0;

				return true;
			}
		}
	}
	return false;
}

void CServers::RemoveAllPlayers( void )
{
	size_t ui = 0, ui1 = 0;
	CPlayers* p = 0;
	while ( ( ui < MAX_PLAYERS ) && ( ui1 < m_ucServerPlayers ) )
	{
		p = m_pPlayerList[ ui ];
		if ( p )
		{
			delete p;
			m_pPlayerList[ ui ] = 0;

			ui1++;
		}
		ui++;
	}
}

void CServers::Query( void )
{
	m_cQueryTime = CServerQuery::Query( m_szIP, m_usPort );
}

void CServers::QueryWithPlayers( void )
{
	m_cQueryTime = CServerQuery::QueryWithPlayers( m_szIP, m_usPort );
}


void CServers::UpdateList( void )
{
	if ( m_eType & E_ST_FAV ) 
	{
		if ( m_uiListItem[ E_LT_FAV ] == -1 )
		{
			m_uiListItem[ E_LT_FAV ] = CMainWindow::AddToFavsList( this );
		}
		else CMainWindow::UpdateFavsList( m_uiListItem[ E_LT_FAV ] );

		CMainWindow::UpdateServerInfo( m_uiListItem[ E_LT_FAV ], this );
	}
	if ( m_eType & E_ST_INTERNET ) 
	{
		if ( m_uiListItem[ E_LT_INTERNET ] == -1 )
		{
			m_uiListItem[ E_LT_INTERNET ] = CMainWindow::AddToInternetList( this );
		}
		else CMainWindow::UpdateInternetList( m_uiListItem[ E_LT_INTERNET ] );

		CMainWindow::UpdateServerInfo( m_uiListItem[ E_LT_INTERNET ], this );
	}
	if ( m_eType & E_ST_OFFICIAL ) 
	{
		if ( m_uiListItem[ E_LT_OFFICIAL ] == -1 )
		{
			m_uiListItem[ E_LT_OFFICIAL ] = CMainWindow::AddToOfficialList( this );
		}
		else CMainWindow::UpdateOfficialList( m_uiListItem[ E_LT_OFFICIAL ] );

		CMainWindow::UpdateServerInfo( m_uiListItem[ E_LT_OFFICIAL ], this );
	}
#ifndef _NO_LAN_MODE
	if ( m_eType & E_ST_LAN ) 
	{
		if ( m_uiListItem[ E_LT_LAN ] == -1 )
		{
			m_uiListItem[ E_LT_LAN ] = CMainWindow::AddToLANList( this );
		}
		else CMainWindow::UpdateLANList( m_uiListItem[ E_LT_LAN ] );

		CMainWindow::UpdateServerInfo( m_uiListItem[ E_LT_LAN ], this );
	}
#endif
}

void CServers::UpdatePlayerList( void )
{
	CMainWindow::ClearPlayerList();

	size_t ui = 0, ui1 = 0;
	CPlayers* p = 0;
	while ( ( ui < MAX_PLAYERS ) && ( ui1 < m_ucServerPlayers ) )
	{
		p = m_pPlayerList[ ui ];
		if ( p )
		{
			switch ( CMainWindow::GetCurrentTab() )
			{
			case 0:
				if ( m_eType & E_ST_FAV ) 
				{
					if ( m_uiListItem[ E_LT_FAV ] != -1 )
						CMainWindow::AddToPlayerList( m_uiListItem[ E_LT_FAV ], p );
				}
				break;

			case 1:
				if ( m_eType & E_ST_INTERNET ) 
				{
					if ( m_uiListItem[ E_LT_INTERNET ] != -1 )
						CMainWindow::AddToPlayerList( m_uiListItem[ E_LT_INTERNET ], p );
				}
				break;

			case 2:
				if ( m_eType & E_ST_OFFICIAL ) 
				{
					if ( m_uiListItem[ E_LT_OFFICIAL ] != -1 )
						CMainWindow::AddToPlayerList( m_uiListItem[ E_LT_OFFICIAL ], p );
				}
				break;
#ifndef _NO_HISTORY
			case 3:
				if ( m_eType & E_ST_HISTORY ) 
				{
					if ( m_uiListItem[ E_LT_HISTORY ] != -1 )
						CMainWindow::AddToPlayerList( m_uiListItem[ E_LT_HISTORY ], p );
				}
				break;
#endif

#ifndef _NO_LAN_MODE
			case LAN_MODE_TAB_ID:
				if ( m_eType & E_ST_LAN ) 
				{
					if ( m_uiListItem[ E_LT_LAN ] != -1 )
						CMainWindow::AddToPlayerList( m_uiListItem[ E_LT_LAN ], p );
				}
				break;
#endif
			}


			ui1++;
		}
		ui++;
	}
}

#ifndef _NO_HISTORY
void CHistoryServer::UpdateList( void )
{
	if ( m_pServer )
	{
		if ( m_pServer->GetType() & E_ST_HISTORY ) 
		{
			if ( m_pServer->GetListItem( E_LT_HISTORY ) == -1 )
			{
				m_pServer->SetListItem( E_LT_HISTORY, CMainWindow::AddToHistoryList( this ) );

				CMainWindow::ReorderHistoryList();
			}
			else CMainWindow::UpdateHistoryList( m_pServer->GetListItem( E_LT_HISTORY ) );

			CMainWindow::UpdateServerInfo( m_pServer->GetListItem( E_LT_HISTORY ), m_pServer );
		}

		m_pServer->UpdateList();
	}
}
#endif

size_t CServerManager::m_uiCount = 0;
size_t CServerManager::m_uiLastFreeID = 0;
CServers* CServerManager::m_Servers[ MAX_SERVERS ] = { 0 };

CServers* CServerManager::New( void )
{
	size_t ui = m_uiLastFreeID;

	if ( ui < MAX_SERVERS )
	{
		CServers* p = new CServers( ui );
		m_Servers[ ui ] = p;
		++m_uiCount;

		++m_uiLastFreeID = FindFreeID();

		return p;
	}

	return NULL;
}

CServers* CServerManager::New( CServers* pOld )
{
	size_t ui = m_uiLastFreeID;

	if ( ui < MAX_SERVERS )
	{
		CServers* p = new CServers( ui, *pOld );
		m_Servers[ ui ] = p;
		++m_uiCount;

		++m_uiLastFreeID = FindFreeID();

		return p;
	}

	return NULL;
}

CServers* CServerManager::Find( const char* szIP, unsigned short usPort )
{
	size_t ui = 0, ui1 = 0;
	CServers* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			if ( p->GetServerPort() == usPort )
			{
				if ( str_equalA( p->GetServerIP(), szIP ) ) return p;
			}
			++ui1;
		}
		++ui;
	}

	return NULL;
}

CServers* CServerManager::FindIndex( unsigned int uiTab, size_t uiIndex )
{
	size_t ui = 0, ui1 = 0;
	CServers* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			if ( p->GetListItem( uiTab ) == uiIndex ) return p;
			++ui1;
		}
		++ui;
	}

	return NULL;
}

bool CServerManager::Remove( const CServers *p )
{
	if ( !p ) return false;

	size_t ui = p->GetID();

	m_Servers[ ui ] = 0;
	--m_uiCount;

	delete p;

	if ( ui < m_uiLastFreeID ) 
		m_uiLastFreeID = ui;

	return true;
}

void CServerManager::RemoveAll( void )
{
	size_t ui = 0, ui1 = 0;
	CServers* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			m_Servers[ ui ] = 0;

			delete p;

			++ui1;
		}

		++ui;
	}

	m_uiCount = 0;
	m_uiLastFreeID = 0;
}

size_t CServerManager::CountType( const E_SERVER_TYPE eType )
{
	size_t uiCount = 0;
	size_t ui = 0, ui1 = 0;
	CServers* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			if ( p->GetType() & eType ) uiCount++;
			ui1++;
		}
		ui++;
	}

	return uiCount;
}

size_t CServerManager::FindFreeID( void )
{
	for ( size_t ui = m_uiLastFreeID; ui < MAX_SERVERS; ++ui )
	{
		if ( !m_Servers[ ui ] ) return ui;
	}

	return MAX_SERVERS;
}

#ifndef _NO_HISTORY

size_t CHistoryServerManager::m_uiCount = 0;
size_t CHistoryServerManager::m_uiLastFreeID = 0;
CHistoryServer* CHistoryServerManager::m_Servers[ MAX_SERVERS ] = { 0 };

CHistoryServer* CHistoryServerManager::New( void )
{
	size_t ui = m_uiLastFreeID;

	if ( ui < MAX_SERVERS )
	{
		CHistoryServer* p = new CHistoryServer( ui );
		m_Servers[ ui ] = p;
		++m_uiCount;

		++m_uiLastFreeID = FindFreeID();

		return p;
	}

	return NULL;
}

CHistoryServer* CHistoryServerManager::New( CServers* pServer )
{
	size_t ui = m_uiLastFreeID;

	if ( ui < MAX_SERVERS )
	{
		CHistoryServer* p = new CHistoryServer( ui, pServer );
		m_Servers[ ui ] = p;
		++m_uiCount;

		++m_uiLastFreeID = FindFreeID();

		return p;
	}

	return NULL;
}

CHistoryServer* CHistoryServerManager::Find( const char* szIP, unsigned short usPort )
{
	size_t ui = 0, ui1 = 0;
	CHistoryServer* p = 0;
	CServers* pServer = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			pServer = p->GetServer();
			if ( pServer )
			{
				if ( pServer->GetServerPort() == usPort )
				{
					if ( str_equalA( pServer->GetServerIP(), szIP ) ) 
						return p;
				}
			}
			++ui1;
		}
		++ui;
	}

	return NULL;
}

CHistoryServer* CHistoryServerManager::FindIndex( size_t uiIndex )
{
	size_t ui = 0, ui1 = 0;
	CHistoryServer* p = 0;
	CServers* pServer = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			pServer = p->GetServer();
			if ( pServer )
			{
				if ( pServer->GetListItem( E_LT_HISTORY ) == uiIndex ) 
					return p;
			}
			++ui1;
		}
		++ui;
	}

	return NULL;
}

bool CHistoryServerManager::Remove( const CHistoryServer *p )
{
	if ( !p ) return false;

	size_t ui = p->GetID();

	m_Servers[ ui ] = 0;
	--m_uiCount;

	delete p;

	if ( ui < m_uiLastFreeID ) 
		m_uiLastFreeID = ui;

	return true;
}

void CHistoryServerManager::RemoveAll( void )
{
	size_t ui = 0, ui1 = 0;
	CHistoryServer* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			m_Servers[ ui ] = 0;

			delete p;

			++ui1;
		}

		++ui;
	}

	m_uiCount = 0;
	m_uiLastFreeID = 0;
}

size_t CHistoryServerManager::FindFreeID( void )
{
	for ( size_t ui = m_uiLastFreeID; ui < MAX_SERVERS; ++ui )
	{
		if ( !m_Servers[ ui ] ) return ui;
	}

	return MAX_SERVERS;
}
#endif
