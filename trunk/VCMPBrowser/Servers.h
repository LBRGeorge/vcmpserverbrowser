#pragma once

#include "MainWindow.h"

enum E_SERVER_TYPE
{
	E_ST_NONE = 0x00,
	E_ST_FAV = 0x01,
	E_ST_INTERNET = 0x02,
	E_ST_OFFICIAL = 0x04,
	E_ST_HISTORY = 0x08,
	E_ST_LAN = 0x10,
};

enum E_LISTTYPE
{
	E_LT_FAV,
	E_LT_INTERNET,
	E_LT_OFFICIAL,
	E_LT_HISTORY,
	E_LT_LAN
};

class CPlayers
{
public:
	CPlayers( size_t uiID )
		: m_uiID( uiID ), m_szPlayerName( 0 )
	{
	}
	~CPlayers()
	{
		if ( m_szPlayerName )
		{
			free( m_szPlayerName );
			m_szPlayerName = 0;
		}
	}

	size_t							GetID					( void ) const				{ return m_uiID; }

	const char*						GetName					( void ) const				{ return m_szPlayerName; }
	void							SetName					( const char* sz )
	{
		size_t uiLen = strlen( sz )+1;
		m_szPlayerName = (char*)realloc( m_szPlayerName, uiLen );
		str_cpyA( m_szPlayerName, sz, uiLen );
	}

private:
	size_t							m_uiID;

	char*							m_szPlayerName;

};

class CServers
{
public:
	CServers( size_t uiID );
	CServers( size_t uiID, const CServers& p );
	~CServers(void);

	size_t							GetID					( void ) const				{ return m_uiID; }

	E_SERVER_TYPE					GetType					( void ) const				{ return m_eType; }
	void							SetType					( const E_SERVER_TYPE e )	{ m_eType = e; }

	// The IP should only be set if you know its an IP (like from the masterlist)
	const char*						GetServerIP				( void ) const				{ return m_szIP; }
	void							SetServerIP				( const char* sz )			{ str_cpyA( m_szIP, sz, sizeof( m_szIP ) ); }

	// Cant use GetPort because windows nicked it :(
	unsigned short					GetServerPort			( void ) const				{ return m_usPort; }
	void							SetServerPort			( const unsigned short us )	{ m_usPort = us; }

	const char*						GetServerPass			( void ) const				
	{ 
		if ( m_szPass )
			return m_szPass; 
		else
			return "";
	}
	void							SetServerPass			( const char* sz );


	unsigned char					GetGameID				( void ) const				{ return m_ucGame; }
	void							SetGameID				( unsigned char uc )		{ m_ucGame = uc; }

	bool							GetLocked				( void ) const				{ return m_bLocked; }
	void							SetLocked				( const bool b )			{ m_bLocked = b; }

	const char*						GetServerName			( void ) const				
	{ 
		if ( m_szServerName )
			return m_szServerName; 
		else
			return "";
	}
	void							SetServerName			( const char* sz );

	const char*						GetServerMode			( void ) const				
	{ 
		if ( m_szServerMode )
			return m_szServerMode; 
		else
			return "";
	}
	void							SetServerMode			( const char* sz );

	const char*						GetServerMap			( void ) const				
	{ 
		if ( m_szServerMap )
			return m_szServerMap; 
		else
			return "";
	}
	void							SetServerMap			( const char* sz );

	unsigned char					GetServerPlayers		( void ) const				{ return m_ucServerPlayers; }
	void							SetServerPlayers		( unsigned char uc )		{ m_ucServerPlayers = uc; }

	unsigned char					GetServerMaxPlayers		( void ) const				{ return m_ucServerMaxPlayers; }
	void							SetServerMaxPlayers		( unsigned char uc )		{ m_ucServerMaxPlayers = uc; }

	unsigned short					GetServerPing			( void ) const				{ return m_usServerPing; }
	void							SetServerPing			( unsigned short us )		{ m_usServerPing = us; }

	size_t							GetListItem				( unsigned char uc ) const	{ return m_uiListItem[ uc ]; }
	void							SetListItem				( unsigned char uc, size_t ui ) { m_uiListItem[ uc ] = ui; }

	clock_t							GetServerQueryTime		( void ) const				{ return m_cQueryTime; }
	void							SetServerQueryTime		( clock_t c )				{ m_cQueryTime = c; }

	// Player Stuff
	CPlayers*						AddPlayer				( const size_t ui );
	bool							RemovePlayer			( const size_t ui );
	bool							RemovePlayer			( CPlayers* p );

	void							RemoveAllPlayers		( void );

	CPlayers*						GetPlayer				( size_t ui ) const
	{
		if ( ui < MAX_PLAYERS ) return m_pPlayerList[ ui ];

		return 0;
	}

	void							Query					( void );
	void							QueryWithPlayers		( void );

	virtual void					UpdateList				( void );
	void							UpdatePlayerList		( void );
	void							UpdateRuleList			( void );

protected:
	size_t							m_uiID;

	E_SERVER_TYPE					m_eType;

	char							m_szIP[ 16 ];
	char*							m_szPass;

	unsigned short					m_usPort;
	unsigned char					m_ucGame;
	bool							m_bLocked;

	char*							m_szServerName;
	char*							m_szServerMode;
	char*							m_szServerMap;

	unsigned char					m_ucServerPlayers;
	unsigned char					m_ucServerMaxPlayers;

	unsigned short					m_usServerPing;

	clock_t							m_cQueryTime;

	size_t							m_uiListItem[ 5 ];

	CPlayers*						m_pPlayerList[ MAX_PLAYERS ];
};

#ifndef _NO_HISTORY
class CHistoryServer : public CServers
{
public:
	CHistoryServer( size_t uiID )
		: CServers( uiID ), m_uiLastPlayed(0)
	{
	}
	CHistoryServer( size_t uiID, const CServers& p )
		: CServers( uiID, p )
	{
	}
	~CHistoryServer( void )
	{
	}

	const clock_t					GetLastPlayed			( void ) const				{ return m_uiLastPlayed; }
	void							SetLastPlayed			( const clock_t& ui )		{ m_uiLastPlayed = ui; }

	virtual void					UpdateList				( void );

private:
	clock_t							m_uiLastPlayed;
};
#endif

template <class T>
class CTmplServerManager
{
public:
	static T*						New						( void );
	static T*						New						( const CServers& pOld );

	static T*						Find					( size_t uiID )
	{
		if ( uiID < MAX_SERVERS ) return m_Servers[ uiID ];

		return 0;
	}

	static T*						Find					( const char* szIP, unsigned short usPort );

	static T*						FindIndex				( size_t uiIndex );
	
	static bool						Remove					( const T* p );

	static void						RemoveAll				( void );

	static size_t					Count					( void )					{ return m_uiCount; }
	static size_t					CountType				( const E_SERVER_TYPE eType );

	static size_t					FindFreeID				( void );

private:
	static size_t					m_uiCount;
	static size_t					m_uiLastFreeID;

	static T*						m_Servers[ MAX_SERVERS ];
};

template <class T>
size_t CTmplServerManager<T>::m_uiCount = 0;
template <class T>
size_t CTmplServerManager<T>::m_uiLastFreeID = 0;
template <class T>
T* CTmplServerManager<T>::m_Servers[ MAX_SERVERS ] = { 0 };

template <class T>
T* CTmplServerManager<T>::New( void )
{
	size_t ui = m_uiLastFreeID;

	if ( ui < MAX_SERVERS )
	{
		T* p = new T( ui );
		m_Servers[ ui ] = p;
		m_uiCount++;

		m_uiLastFreeID++;
		m_uiLastFreeID = FindFreeID();

		return p;
	}

	return NULL;
}

template <class T>
T* CTmplServerManager<T>::New( const CServers& pOld )
{
	size_t ui = m_uiLastFreeID;

	if ( ui < MAX_SERVERS )
	{
		T* p = new T( ui, pOld );
		m_Servers[ ui ] = p;
		m_uiCount++;

		m_uiLastFreeID++;
		m_uiLastFreeID = FindFreeID();

		return p;
	}

	return NULL;
}

template <class T>
T* CTmplServerManager<T>::Find( const char* szIP, unsigned short usPort )
{
	size_t ui = 0, ui1 = 0;
	T* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			if ( p->GetServerPort() == usPort )
			{
				if ( str_equalA( p->GetServerIP(), szIP ) ) return p;
			}
			ui1++;
		}
		ui++;
	}

	return NULL;
}

template <class T>
T* CTmplServerManager<T>::FindIndex( size_t uiIndex )
{
	size_t ui = 0, ui1 = 0;
	T* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			if ( p->GetListItem() == uiIndex ) return p;
			ui1++;
		}
		ui++;
	}

	return NULL;
}

template <class T>
bool CTmplServerManager<T>::Remove( const T *p )
{
	if ( !p ) return false;

	size_t ui = p->GetID();

	m_Servers[ ui ] = 0;
	m_uiCount--;

	delete p;

	if ( ui < m_uiLastFreeID ) m_uiLastFreeID = ui;

	return true;
}

template <class T>
void CTmplServerManager<T>::RemoveAll( void )
{
	size_t ui = 0, ui1 = 0;
	T* p = 0;
	while ( ( ui < MAX_SERVERS ) && ( ui1 < m_uiCount ) )
	{
		p = m_Servers[ ui ];
		if ( p )
		{
			m_Servers[ ui ] = 0;

			delete p;

			ui1++;
		}

		ui++;
	}

	m_uiCount = 0;
	m_uiLastFreeID = 0;
}

template <class T>
size_t CTmplServerManager<T>::CountType( const E_SERVER_TYPE eType )
{
	size_t uiCount = 0;
	size_t ui = 0, ui1 = 0;
	T* p = 0;
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

template <class T>
size_t CTmplServerManager<T>::FindFreeID( void )
{
	for ( size_t ui = m_uiLastFreeID; ui < MAX_SERVERS; ui++ )
	{
		if ( !m_Servers[ ui ] ) return ui;
	}

	return MAX_SERVERS;
}


#define CServerManager CTmplServerManager< CServers >
#ifndef _NO_HISTORY
#define CHistoryServerManager CTmplServerManager< CHistoryServer >
#endif
