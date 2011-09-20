#include "MainWindow.h"
#include "ServerQuery.h"
#include "Settings.h"
#include "Favourites.h"
#include "History.h"
#include <shlobj.h>
#include <direct.h>
#include "SettingsDlg.h"

#include "Win7.h"

#ifdef _WITH_JUMPLIST
#include <Shobjidl.h>
#include <propkey.h>
#include <Propvarutil.h>

#pragma comment(lib, "Shlwapi") 
#endif

HANDLE g_hListMutex = 0; 

TCHAR g_szAppDataDir[ MAX_PATH ] = { 0 };

extern void SplitIPAndPort( const char* sz, char szIP[], unsigned short& usPort, size_t uiIPBufSize );
extern void StartGame( const char* szIP, unsigned short usPort, unsigned char ucGame, const char* szPass = 0 );

bool ProcessCommandLine( LPSTR lpCmdLine )
{
	if ( *lpCmdLine )
	{
		char szCmds[ 256 ] = { NULL };
		str_cpyA( szCmds, lpCmdLine, sizeof( szCmds ) );

		bool bStartFromCmd = false;
		char szIP[ 128 ] = { 0 };
		unsigned short usPort = 0;
		char szPass[ 128 ] = { 0 };

		if ( str_prefix( szCmds, "vcmp://", 5 ) )
		{
			char* opt = strtok( szCmds+5, "/" );	// Get a space delimited token.
			while ( opt )
			{
				/*const char* szBlah = strstr( opt, ":" );

				if ( szBlah )
				{
				SplitIPAndPort( opt, szIP, usPort, sizeof( szIP ) );

				if ( !usPort ) usPort = 2301;
				}
				else str_cpyA( szPass, opt, sizeof( szPass ) );*/

				if ( str_equalA( opt, "addfav" ) )
				{
					opt = strtok(NULL, " ");

					SplitIPAndPort( opt, szIP, usPort, sizeof( szIP ) );

					if ( !usPort ) 
						usPort = 2301;

					TCHAR szMsg[ 128 ] = { 0 };
					_stprintf( szMsg, _TEXT( "Would you like to add %s:%u to your favourites?" ), szIP, usPort );
					if ( MessageBox( 0, szMsg, _TEXT( "Add Favourite" ), MB_OKCANCEL ) != IDOK )
						break;

					struct hostent     *he;
					if ( ( he = gethostbyname( szIP ) ) == NULL ) 
					{
						MessageBox( 0, _TEXT("Unable to resolve IP of server"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
						break;
					}

					str_cpyA( szIP, inet_ntoa( *( (in_addr*)he->h_addr ) ), sizeof( szIP ) );

					CServers* pServer = CServerManager::Find( szIP, usPort );
					if ( !pServer )
					{
						pServer = CServerManager::New();
						if ( pServer )
						{
							pServer->SetServerIP( szIP );
							pServer->SetServerPort( usPort );

							pServer->SetServerName( "(Waiting for server data...)" );
						}
					}
					else 
					{
						if ( pServer->GetType() & E_ST_FAV )
						{
							MessageBox( 0, _TEXT( "This server already exists in your favourites" ), _TEXT( "Error" ), MB_OK );
							break;
						}
					}

					if ( pServer )
					{
						if ( !(pServer->GetType() & E_ST_FAV) ) 
							pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_FAV) );

						pServer->Query();

						pServer->UpdateList();
					}
				}
				else if ( str_equalA( opt, "connect" ) )
				{
					opt = strtok(NULL, " ");

					SplitIPAndPort( opt, szIP, usPort, sizeof( szIP ) );

					TCHAR szMsg[ 128 ] = { 0 };
					_stprintf( szMsg, _TEXT( "Would you like to connect to %s:%u" ), szIP, usPort );
					if ( MessageBox( 0, szMsg, _TEXT( "Connect to Server" ), MB_OKCANCEL ) != IDOK )
						break;

					if ( !usPort ) usPort = 2301;

					struct hostent     *he;
					if ( ( he = gethostbyname( szIP ) ) == NULL ) 
					{
						MessageBox( 0, _TEXT("Unable to resolve IP of server"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
						break;
					}

					StartGame( szIP, usPort, 1 );
				}

				opt = strtok(NULL, " ");			// Get next token.			
			}
		}
		else if ( *szCmds == '/' )
		{
			if ( str_equal( szCmds, "/Tab-Favs" ) )
			{
				CMainWindow::SetCurrentTab( 0 );
			}
			else if ( str_equal( szCmds, "/Tab-Internet" ) )
			{
				CMainWindow::SetCurrentTab( 1 );
			}
			else if ( str_equal( szCmds, "/Tab-Official" ) )
			{
				CMainWindow::SetCurrentTab( 2 );
			}
#ifndef _NO_HISTORY
			else if ( str_equal( szCmds, "/Tab-History" ) )
			{
				CMainWindow::SetCurrentTab( 3 );
			}
#endif
#ifndef _NO_LAN_MODE
			else if ( str_equal( szCmds, "/Tab-Lan" ) )
			{
				CMainWindow::SetCurrentTab( LAN_MODE_TAB_ID );
			}
#endif

			else if ( str_equal( szCmds, "/Win-Settings" ) )
			{
				CSettingsDlg::Show( CMainWindow::GetMain() );
			}

			else if ( str_equal( szCmds, "/Win-Help" ) )
			{
				ShellExecute(NULL, "open", "http://vcmp.liberty-unleashed.co.uk", NULL, NULL, SW_SHOWNORMAL);
				return false;
			}

		}
		else
		{
			char* opt = strtok( szCmds, " " );	// Get a space delimited token.
			while ( opt )
			{
				if ( str_equal(opt, "-c") )	// If current token is "-hook"...
				{
					bStartFromCmd = true;			// Set the hook to be enabled
				}
				else if ( str_equal(opt, "-h") )	// If current token is "-ip"...
				{
					opt = strtok(NULL, " ");
					str_cpyA( szIP, opt, sizeof( szIP ) );			// Set the server IP
				}
				else if ( str_equal(opt, "-p") )	// If current token is "-port"...
				{
					opt = strtok(NULL, " ");
					usPort = static_cast< unsigned short >( atoi( opt ) );			// Set the server port
				}
				opt = strtok(NULL, " ");			// Get next token.			
			}

			if ( bStartFromCmd )
			{
				if ( !usPort ) usPort = 5192;

				if ( *szIP )
				{
					struct hostent     *he;
					if ( ( he = gethostbyname( szIP ) ) != NULL )
					{
						StartGame( szIP, usPort, 1 );
						return false;
					}
					else
						MessageBox( 0, _TEXT("Unable to resolve IP of server"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
				}
			}
		}
	}

	return true;
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	g_hListMutex = CreateMutex( NULL, FALSE, _TEXT( "ListMutex-{83E9C941-A921-45fd-AE14-FA7716E18BC7}" ) );

	bool bAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

	if ( bAlreadyRunning )
	{
		HWND hwnd = FindWindow( NULL, _TEXT("Vice City Multiplayer 0.3z R2 Server Browser - Created by VRocker") );
		if ( hwnd )
		{
			ShowWindow( hwnd, SW_SHOWNORMAL );
			SetForegroundWindow( hwnd );

			COPYDATASTRUCT myData;
			myData.dwData = 0;
			myData.cbData = strlen(lpCmdLine)+1;
			myData.lpData = lpCmdLine;
			SendMessage( hwnd, WM_COPYDATA, 0, (LPARAM)&myData );
		}
		else
		{
			// If everything else fails for some reason use the old messagebox
			MessageBox( 0, _TEXT( "An instance of this application is already running" ), _TEXT( "Error" ), MB_OK );
		}

		ReleaseMutex( g_hListMutex );

		return 1;
	}

#ifdef _WITH_JUMPLIST
	CoInitialize( NULL );

	CWin7::Init();
#endif

	if ( !CMainWindow::Create( hInstance, hPrevInstance, lpCmdLine ) )
		return 0;

	CMainWindow::CreateStatusBar();
	CMainWindow::CreateGroupBox();
	CMainWindow::CreateListBox();
	CMainWindow::CreateTabControl();
	CMainWindow::CreateListViews();

	char szRunningPath[ MAX_PATH + 1 ] = { 0 };
	GetModuleFileName( hInstance, szRunningPath, MAX_PATH );

	if ( *szRunningPath )
	{
		size_t uiLen = strlen( szRunningPath );

		for ( unsigned int ui = uiLen - 1; ui; ui-- )
		{
			if ( szRunningPath[ ui ] == '\\' )
			{
				szRunningPath[ ui ] = '\0';
				break;
			}
		}
		
		SetCurrentDirectory( szRunningPath );
	}

	SHGetSpecialFolderPath( 0, g_szAppDataDir, CSIDL_APPDATA, false );

	TCHAR szFile[ 384 ] = { NULL };
	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "\\VCMP" ) );
	_tmkdir( szFile );


	CSettings::LoadSettings();

	CServerQuery::Init();

	CFavourites::Load();
#ifndef _NO_HISTORY
	CHistory::Load();
#endif

#ifdef _WITH_JUMPLIST
	CWin7::BeginList();
	CWin7::CreateTasksList();
	CWin7::CreateCustomList();
	CWin7::EndList();
#endif

	if ( !ProcessCommandLine( lpCmdLine ) )
		goto quitApp;
	
	CServerQuery::Start();

	CMainWindow::Show( nCmdShow );

	MSG msg;

	// The > 0 is needed because GetMessage can return -1
	while ( GetMessage( &msg, NULL, 0, 0 ) > 0 )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
		//Sleep(1);
	}

quitApp:
#ifdef _WITH_JUMPLIST
	CWin7::Shutdown();

	CoUninitialize();
#endif

	CServerQuery::Stop();

	CServerManager::RemoveAll();

	ReleaseMutex( g_hListMutex );
	CloseHandle( g_hListMutex );

	return (int)msg.wParam;
}