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
	E_LT_LAN,

	E_LT_COUNT
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
class CHistoryServer
{
public:
	CHistoryServer( size_t uiID )
		: m_uiID( uiID ), m_pServer( 0 ), m_uiLastPlayed(0)
	{
	}
	CHistoryServer( size_t uiID, CServers* p )
		: m_uiID( uiID ), m_pServer( p ), m_uiLastPlayed(0)
	{
	}
	virtual ~CHistoryServer( void )
	{
	}

	const unsigned int				GetID					( void ) const				{ return m_uiID; }

	CServers*						GetServer				( void ) const				{ return m_pServer; }
	void							SetServer				( CServers* p )				{ m_pServer = p; }

	const time_t					GetLastPlayed			( void ) const				{ return m_uiLastPlayed; }
	void							SetLastPlayed			( const time_t& ui )		{ m_uiLastPlayed = ui; }

	virtual void					UpdateList				( void );

private:
	unsigned int					m_uiID;

	CServers*						m_pServer;
	time_t							m_uiLastPlayed;
};
#endif

class CServerManager
{
public:
	static CServers*				New						( void );
	static CServers*				New						( CServers* pOld );

	static CServers*				Find					( size_t uiID )
	{
		if ( uiID < MAX_SERVERS ) return m_Servers[ uiID ];

		return 0;
	}

	static CServers*				Find					( const char* szIP, unsigned short usPort );

	static CServers*				FindIndex				( unsigned int uiTab, size_t uiIndex );
	
	static bool						Remove					( const CServers* p );

	static void						RemoveAll				( void );

	static size_t					Count					( void )					{ return m_uiCount; }
	static size_t					CountType				( const E_SERVER_TYPE eType );

	static size_t					FindFreeID				( void );

private:
	static size_t					m_uiCount;
	static size_t					m_uiLastFreeID;

	static CServers*				m_Servers[ MAX_SERVERS ];
};

#ifndef _NO_HISTORY
class CHistoryServerManager
{
public:
	static CHistoryServer*			New						( void );
	static CHistoryServer*			New						( CServers* pServer );

	static CHistoryServer*			Find					( size_t uiID )
	{
		if ( uiID < MAX_SERVERS ) return m_Servers[ uiID ];

		return 0;
	}

	static CHistoryServer*			Find					( const char* szIP, unsigned short usPort );

	static CHistoryServer*			FindIndex				( size_t uiIndex );
	
	static bool						Remove					( const CHistoryServer* p );

	static void						RemoveAll				( void );

	static size_t					Count					( void )					{ return m_uiCount; }

	static size_t					FindFreeID				( void );

private:
	static size_t					m_uiCount;
	static size_t					m_uiLastFreeID;

	static CHistoryServer*			m_Servers[ MAX_SERVERS ];
};
#endif
