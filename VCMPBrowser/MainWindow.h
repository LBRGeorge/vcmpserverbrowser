#pragma once

#define VC_EXTRALEAN
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>
#include "Utils.h"
#include "resource.h"
#include "uft/uft.h"
#include "http/http_fetcher.h"

#ifndef MAX_SERVERS
	#define MAX_SERVERS 250
#endif

#ifndef MAX_PLAYERS
	#define MAX_PLAYERS 128
#endif

class CServers;
class CHistoryServer;
class CPlayers;

// Comment this to enable a LAN tab
#define _NO_LAN_MODE
//#define _NO_HISTORY

#define _WITH_JUMPLIST 1

#ifndef _NO_LAN_MODE
#ifdef _NO_HISTORY
#define LAN_MODE_TAB_ID 3
#else
#define LAN_MODE_TAB_ID 4
#endif
#endif

#define _USE_OLD_UPDATER 0

#define WM_UPDATESERVERLIST (WM_APP+1)
#define WM_UPDATEPLAYERLIST (WM_APP+2)
#define WM_UPDATERULESLIST (WM_APP+3)
#define WM_UPDATEHISTORYSERVERLIST (WM_APP+4)

enum E_LISTSORT
{
	E_LS_ASCENDING,
	E_LS_DESCENDING
};

struct sColumnSorting
{
	sColumnSorting()
		: uiLastSort( 0 ), eSorting( E_LS_DESCENDING )
	{
	}

	unsigned int uiLastSort;
	E_LISTSORT eSorting;
};

class CMainWindow
{
public:
	static bool							Create						( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine );
	static bool							CreateStatusBar				( void );
	static bool							CreateTabControl			( void );
	static bool							CreateGroupBox				( void );
	static bool							CreateListBox				( void );
	static bool							CreateListViews				( void );

	static void							Show						( int nCmdShow );

	static void							Resize						( void );

	// Access Methods
	static HWND							GetMain						( void )							{ return m_hWnd; }
	static HWND							GetTabControl				( void )							{ return m_hTabControl; }		

	static HWND							GetFavsList					( void )							{ return m_hFavsList; }
	static HWND							GetInternetList				( void )							{ return m_hInternetList; }
	static HWND							GetOfficialList				( void )							{ return m_hOfficialList; }
#ifndef _NO_HISTORY
	static HWND							GetHistoryList				( void )							{ return m_hHistoryList; }
#endif
#ifndef _NO_LAN_MODE
	static HWND							GetLANList					( void )							{ return m_hLANList; }
#endif
	static HWND							GetRulesList				( void )							{ return m_hRulesList; }

	// Label Stuff
	static HWND							GetServerNameLabel			( void )							{ return m_hLblServerName; }
	static HWND							GetServerIPLabel			( void )							{ return m_hLblServerIP; }
	static HWND							GetServerPlayersLabel		( void )							{ return m_hLblServerPlayers; }
	static HWND							GetServerPingLabel			( void )							{ return m_hLblServerPing; }

	static void							ClearServerInfo				( void );
	static void							UpdateServerInfo			( size_t index, CServers* pServer );

	// Add to list stuff
	static size_t						AddToFavsList				( CServers* p );
	static void							RemoveFromFavsList			( CServers* p );
	static void							ClearFavsList				( void );

	static size_t						AddToInternetList			( CServers* p );
	static void							ClearInternetList			( void );

	static size_t						AddToOfficialList			( CServers* p );
	static void							ClearOfficialList			( void );

#ifndef _NO_HISTORY
	static size_t						AddToHistoryList			( CHistoryServer* p );
	static void							ClearHistoryList			( void );
#endif

#ifndef _NO_LAN_MODE
	static size_t						AddToLANList				( CServers* p );
#endif

	static void							UpdateFavsList				( size_t index );
	static void							ReorderFavsList				( int iColumn );
	static void							UpdateInternetList			( size_t index );
	static void							ReorderInternetList			( int iColumn );
	static void							UpdateOfficialList			( size_t index );
	static void							ReorderOfficialList			( int iColumn );
#ifndef _NO_HISTORY
	static void							UpdateHistoryList			( size_t index );
	static void							ReorderHistoryList			( void );
#endif
#ifndef _NO_LAN_MODE
	static void							UpdateLANList				( size_t index );
#endif


	static void							AddToPlayerList				( size_t index, CPlayers* p );
	static void							ClearPlayerList				( void );

	static int							GetCurrentTab				( void );
	static void							SetCurrentTab				( int );

private:
	static HWND							m_hWnd;
	static HWND							m_hStatusBar;
	static HWND							m_hTabControl;
	static HWND							m_hSideGroupBox;
	static HWND							m_hFiltersGroupBox;
	static HWND							m_hPlayerList;

	// List Views
	static HWND							m_hFavsList;
	static HWND							m_hInternetList;
	static HWND							m_hOfficialList;
#ifndef _NO_HISTORY
	static HWND							m_hHistoryList;
#endif
#ifndef _NO_LAN_MODE
	static HWND							m_hLANList;
#endif
	static HWND							m_hRulesList;

	static HIMAGELIST					m_hLockedImage;

	// Labels
	static HWND							m_hLblServerName;
	static HWND							m_hLblServerIP;
	static HWND							m_hLblServerPing;
	static HWND							m_hLblServerPlayers;

public:
	// List Items
	static CServers*					m_lFavServers[ MAX_SERVERS ];
	static size_t						m_uiFavServers;

	static CServers*					m_lInternetServers[ MAX_SERVERS ];
	static size_t						m_uiInternetServers;

	static CServers*					m_lOfficialServers[ MAX_SERVERS ];
	static size_t						m_uiOfficialServers;

#ifndef _NO_HISTORY
	static CHistoryServer*				m_lHistoryServers[ MAX_SERVERS ];
	static size_t						m_uiHistoryServers;
#endif

#ifndef _NO_LAN_MODE
	static CServers*					m_lLANServers[ MAX_SERVERS ];
	static size_t						m_uiLANServers;
#endif

	static CPlayers*					m_lPlayers[ MAX_PLAYERS ];
	static size_t						m_uiPlayers;
};
