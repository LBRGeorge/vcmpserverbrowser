#include "MainWindow.h"
#include "ServerQuery.h"
#include "SettingsDlg.h"
#include "Favourites.h"
#include "PasswordDlg.h"
#include <shlobj.h>
#include <Windowsx.h>


// Enables styles on stuff
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define GB_FILTERS_WIDTH 400

extern void SplitIPAndPort( const char* sz, char szIP[], unsigned short& usPort, size_t uiIPBufSize );

HWND CMainWindow::m_hWnd = 0;
HWND CMainWindow::m_hStatusBar = 0;
HWND CMainWindow::m_hTabControl = 0;
HWND CMainWindow::m_hSideGroupBox = 0;
HWND CMainWindow::m_hFiltersGroupBox = 0;
HWND CMainWindow::m_hInfoGroupBox = 0;
HWND CMainWindow::m_hPlayerList = 0;
// List Views
HWND CMainWindow::m_hFavsList = 0;
HWND CMainWindow::m_hInternetList = 0;
HWND CMainWindow::m_hOfficialList = 0;
#ifndef _NO_HISTORY
HWND CMainWindow::m_hHistoryList = 0;
#endif
#ifndef _NO_LAN_MODE
HWND CMainWindow::m_hLANList = 0;
#endif
HWND CMainWindow::m_hRulesList = 0;

HIMAGELIST CMainWindow::m_hLockedImage = 0;

// Labels
HWND CMainWindow::m_hLblServerName = 0;
HWND CMainWindow::m_hLblServerIP = 0;
HWND CMainWindow::m_hLblServerPlayers = 0;
HWND CMainWindow::m_hLblServerPing = 0;

CServers* CMainWindow::m_lFavServers[ MAX_SERVERS ] = { 0 };
size_t CMainWindow::m_uiFavServers = 0;
CServers* CMainWindow::m_lInternetServers[ MAX_SERVERS ] = { 0 };
size_t CMainWindow::m_uiInternetServers = 0;
CServers* CMainWindow::m_lOfficialServers[ MAX_SERVERS ] = { 0 };
size_t CMainWindow::m_uiOfficialServers = 0;
#ifndef _NO_HISTORY
CHistoryServer* CMainWindow::m_lHistoryServers[ MAX_SERVERS ] = { 0 };
size_t CMainWindow::m_uiHistoryServers = 0;
#endif
#ifndef _NO_LAN_MODE
CServers* CMainWindow::m_lLANServers[ MAX_SERVERS ] = { 0 };
size_t CMainWindow::m_uiLANServers = 0;
#endif

CPlayers* CMainWindow::m_lPlayers[ MAX_PLAYERS ] = { 0 };
size_t CMainWindow::m_uiPlayers = 0;

BOOL SetClipboardText(LPCTSTR pszText)
{
   BOOL ok = FALSE;
   if(OpenClipboard( CMainWindow::GetMain() )) 
   {
	   size_t len = _tcslen( pszText ) + 1;
		// the text should be placed in "global" memory
		HGLOBAL hMem = GlobalAlloc( GMEM_SHARE | GMEM_MOVEABLE, len);
	   /*LPTSTR ptxt = (LPTSTR)GlobalLock(hMem);
	   lstrcpy(ptxt, pszText);*/
	   memcpy(GlobalLock(hMem), pszText, len);
	   GlobalUnlock(hMem);
	   ok = EmptyClipboard();
	   // set data in clipboard; we are no longer responsible for hMem
	   ok = (BOOL)SetClipboardData(CF_TEXT, hMem);

	   CloseClipboard(); // relinquish it for other windows
   }
   return ok;
}

void CreateContextMenu( HWND hwnd, HMENU hMenu, int index )
{
	HMENU hPopupMenu = GetSubMenu (hMenu, index);
	POINT pt;
	SetMenuDefaultItem (hPopupMenu, -1, TRUE);
	GetCursorPos (&pt);
	SetForegroundWindow (hwnd);
	TrackPopupMenu (hPopupMenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
	SetForegroundWindow (hwnd);
	DestroyMenu (hPopupMenu);
}

void StartGame( const char* szIP, unsigned short usPort, unsigned char ucGame, const char* szPass = 0 )
{
	if ( ( *szIP ) && ( usPort ) ) 
	{
		char szGtaExe[ 256 ] = { 0 };
		str_cpyA( szGtaExe, CSettings::GetGTAVCPath(), sizeof( szGtaExe ) );
		if ( !*szGtaExe )
		{
			if ( MessageBox( 0, _TEXT( "You have not set your GTAVC directory" ), "Error", MB_OKCANCEL ) == IDOK )
			{
				// No GTA3 path, fire up the settings dialog
				CSettingsDlg::Show( CMainWindow::GetMain() );
			}
			return;
		}

		if ( str_equal( CSettings::GetName(), "UnknownPlayer" ) )
		{
			if ( MessageBox( 0, _TEXT( "You have not set your nickname. Would you like to set it now?" ), "Warning", MB_YESNO ) == IDYES )
			{
				CSettingsDlg::Show( CMainWindow::GetMain() );
				return;
			}
		}

		char szParams[ 256 ] = { 0 };
		sprintf(szParams,"-c -h %s -p %i -n %s", szIP, usPort, CSettings::GetName() );

		if ( ( szPass ) && ( *szPass ) )
		{
			strcat( szParams, " -z " );
			strcat( szParams, szPass );
		}

		char szPath[ MAX_PATH ] = { 0 };
		// Strip the gta-vc.exe off the end of the path
		str_cpyA( szPath, szGtaExe, strlen( szGtaExe ) - 10 );

		int x = (int)ShellExecute( 0,"open",szGtaExe,szParams,szPath,NULL);

		if( ( x == ERROR_FILE_NOT_FOUND ) || ( x == ERROR_PATH_NOT_FOUND ) ) 
		{
			MessageBox( 0, _TEXT( "Could not find GTA-VC.exe." ), _TEXT( "Error" ), MB_OK );
		}

		/*PROCESS_INFORMATION pi; STARTUPINFO si;
		DWORD dwThreadID;

		ZeroMemory(&pi,sizeof(pi)); ZeroMemory(&si,sizeof(si));

		TCHAR szCurrentDir[ MAX_PATH ] = { 0 };
		GetCurrentDirectory( MAX_PATH, szCurrentDir );

		SetDllDirectory( szCurrentDir );
		//SetCurrentDirectory( CSettings::GetGTA3Path() );

		char DLLPathName[ MAX_PATH ] = { 0 };
		char** filepart = {NULL};

		size_t uiLen = strlen( DLLPathName )+1;

		BOOL bCreated = CreateProcess(szGtaExe, szParams, NULL, NULL, FALSE, CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS , NULL, szPath, &si, &pi);

		if ( !bCreated )
		{
			MessageBox( 0, _TEXT( "Could not find the GTA executable." ), _TEXT( "Error" ), MB_OK );
		}*/

#ifndef _NO_HISTORY
		else
		{
			CServers* pServer = CServerManager::Find( szIP, usPort );
			if ( pServer )
			{
				CHistoryServer* pHist = CHistoryServerManager::New( *pServer );
				if ( pHist )
				{
					pHist->SetLastPlayed( clock() );
					pHist->SetType( E_ST_HISTORY );

					pHist->UpdateList();
				}
			}
		}
#endif
	}
}

void ConnectToServer( HWND hwnd, CServers* pServer )
{
	pServer->Query();

	if ( pServer->GetLocked() )
	{
		// Show password dialog
		CPasswordDlg::SetServer( pServer->GetServerIP(), pServer->GetServerPort(), pServer->GetServerPass(), pServer->GetGameID() );
		CPasswordDlg::Show( hwnd );
	}
	else
	{
		// If it hasn't responded to a query, it'll have no type UNLESS ITS A FAVOURITE, NOOB
		// Changed to game ID, since this will be 0
		if ( ( pServer->GetGameID() ) && ( pServer->GetType() ) )
		{
			StartGame( pServer->GetServerIP(), pServer->GetServerPort(), pServer->GetGameID() );
		}
	}
}

INT_PTR CALLBACK AddServerDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:
		if (GetDlgCtrlID((HWND) wParam) != IDC_EDIT_SERV) 
		{ 
			SetFocus(GetDlgItem(hwnd, IDC_EDIT_SERV)); 
			return FALSE; 
		}

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				TCHAR szBuffer[ 128 ] = { 0 };

				// Lets update the internal Name stuff
				int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT_SERV));
				if(len > 0)
				{
					GetDlgItemText(hwnd, IDC_EDIT_SERV, szBuffer, 128);

					unsigned short usPort = 0;
					char szIP[ 64 ] = { 0 };

#ifdef _UNICODE
					char szCBuf[ 128 ] = { 0 };
					//MultiByteToWideChar( CP_ACP, 0, szCBuf, -1, szBuffer, _tcslen( szBuffer )+1 );
					int i = WideCharToMultiByte( CP_ACP, 0, szBuffer, -1, szCBuf, sizeof( szCBuf ), 0, 0 );

					SplitIPAndPort( szCBuf, szIP, usPort, sizeof( szIP ) );
#else
					SplitIPAndPort( szBuffer, szIP, usPort, sizeof( szIP ) );
#endif

					

					if ( !usPort ) usPort = 2301;

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
							MessageBox( hwnd, _TEXT( "This server already exists in your favourites" ), _TEXT( "Error" ), MB_OK );
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

					// Finally, close the dialog
					EndDialog(hwnd, IDOK);
				}
				else MessageBox( hwnd, _TEXT( "You did not enter a server" ), _TEXT( "Error" ), MB_OK );
			}
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;		
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemText( hwnd, IDC_LBL_VER, "0.3z R2" );
			return TRUE;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				EndDialog(hwnd, IDOK);
			}
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		{
		}
		break;

	case WM_SIZE:
		{
			CMainWindow::Resize();
		}
		break;

	case WM_COMMAND:
		{
			switch( LOWORD( wParam ) )
			{
			case ID_FILE_EXIT:
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;

			case ID_SERVER_ADDSERVER:
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG_ADDSERVER), hwnd, AddServerDlgProc);
				break;

			case ID_SERVER_REMOVESERVER:
				{
					int index = ListView_GetNextItem(CMainWindow::GetFavsList(),-1,LVNI_SELECTED);
					if(index != -1)
					{
						CServers* p = CMainWindow::m_lFavServers[ index ];
						if ( p )
						{
							CMainWindow::RemoveFromFavsList( p );

							HMENU hMenu = GetMenu( CMainWindow::GetMain() );
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHSERVER, MF_DISABLED );
						}
					}
					else MessageBox( hwnd, _TEXT( "No server is selected" ), _TEXT( "Remove Server" ), MB_OK );
					//DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG_ADDSERVER), hwnd, AddServerDlgProc);
				}
				break;

			case ID_SERVER_REFRESHMASTERLIST:
				{
					int iPage = TabCtrl_GetCurSel( CMainWindow::GetTabControl() );

					switch ( iPage )
					{
					case 1:
						CMainWindow::ClearInternetList();
						CServerQuery::UpdateInternetList();
						break;

					case 2:
						CMainWindow::ClearOfficialList();
						CServerQuery::UpdateOfficialList();
						break;
					}
				}
				break;

			case ID_TOOLS_SETTINGS:
				CSettingsDlg::Show( hwnd );
				break;

			case ID_HELP_HELPSUPPORT:
				{
					ShellExecute(NULL, "open", "http://vcmp.liberty-unleashed.co.uk", NULL, NULL, SW_SHOWNORMAL);
				}
				break;

			case ID_HELP_ABOUT:
				{
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG_ABOUT), hwnd, AboutDlgProc);
				}
				break;
		
			case ID_MENU_CONNECT:
				{
					int iPage = TabCtrl_GetCurSel( CMainWindow::GetTabControl() );
					
					switch ( iPage )
					{
					case 0:
						{
							int index = ListView_GetNextItem(CMainWindow::GetFavsList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lFavServers[ index ];
								if ( pServer )
								{
									ConnectToServer( hwnd, pServer );
								}
							}
						}
						break;

					case 1:
						{
							int index = ListView_GetNextItem(CMainWindow::GetInternetList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lInternetServers[ index ];
								if ( pServer )
								{
									ConnectToServer( hwnd, pServer );
								}
							}
						}
						break;

					case 2:
						{
							int index = ListView_GetNextItem(CMainWindow::GetOfficialList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lOfficialServers[ index ];
								if ( pServer )
								{
									ConnectToServer( hwnd, pServer );
								}
							}
						}
						break;

#ifndef _NO_HISTORY
					case 3:
						{
							int index = ListView_GetNextItem(CMainWindow::GetHistoryList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lHistoryServers[ index ];
								if ( pServer )
								{
									ConnectToServer( hwnd, pServer );
								}
							}
						}
						break;
#endif

#ifndef _NO_LAN_MODE
					case LAN_MODE_TAB_ID:
						{
							int index = ListView_GetNextItem(CMainWindow::GetLANList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lLANServers[ index ];
								if ( pServer )
								{
									ConnectToServer( hwnd, pServer );
								}
							}
						}
						break;
#endif
					}
				}
				break;

			case ID_MENU_COPYSERVERIP:
				{
					int iPage = TabCtrl_GetCurSel( CMainWindow::GetTabControl() );
					
					switch ( iPage )
					{
					case 0:
						{
							int index = ListView_GetNextItem(CMainWindow::GetFavsList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lFavServers[ index ];
								if ( pServer )
								{
									TCHAR szOut[ 64 ] = { 0 };
									_stprintf( szOut, "%s:%u",  pServer->GetServerIP(), pServer->GetServerPort() );
									SetClipboardText( szOut );
								}
							}
						}
						break;

					case 1:
						{
							int index = ListView_GetNextItem(CMainWindow::GetInternetList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lInternetServers[ index ];
								if ( pServer )
								{
									TCHAR szOut[ 64 ] = { 0 };
									_stprintf( szOut, "%s:%u",  pServer->GetServerIP(), pServer->GetServerPort() );
									SetClipboardText( szOut );
								}
							}
						}
						break;

					case 2:
						{
							int index = ListView_GetNextItem(CMainWindow::GetOfficialList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lOfficialServers[ index ];
								if ( pServer )
								{
									TCHAR szOut[ 64 ] = { 0 };
									_stprintf( szOut, "%s:%u",  pServer->GetServerIP(), pServer->GetServerPort() );
									SetClipboardText( szOut );
								}
							}
						}
						break;

#ifndef _NO_HISTORY
					case 3:
						{
							int index = ListView_GetNextItem(CMainWindow::GetHistoryList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lHistoryServers[ index ];
								if ( pServer )
								{
									TCHAR szOut[ 64 ] = { 0 };
									_stprintf( szOut, "%s:%u",  pServer->GetServerIP(), pServer->GetServerPort() );
									SetClipboardText( szOut );
								}
							}
						}
						break;
#endif

#ifndef _NO_LAN_MODE
					case LAN_MODE_TAB_ID:
						{
							int index = ListView_GetNextItem(CMainWindow::GetLANList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lLANServers[ index ];
								if ( pServer )
								{
									TCHAR szOut[ 64 ] = { 0 };
									_stprintf( szOut, "%s:%u",  pServer->GetServerIP(), pServer->GetServerPort() );
									SetClipboardText( szOut );
								}
							}
						}
						break;
#endif
					}
				}
				break;

			case ID_MENU_ADDTOFAVS:
				{
					int iPage = TabCtrl_GetCurSel( CMainWindow::GetTabControl() );
					
					switch ( iPage )
					{
					case 1:
						{
							int index = ListView_GetNextItem(CMainWindow::GetInternetList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lInternetServers[ index ];
								if ( pServer )
								{
									if ( !(pServer->GetType() & E_ST_FAV) )
									{
										pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_FAV) );

										pServer->Query();

										pServer->UpdateList();

										MessageBox( 0, _TEXT( "Added to favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
									}
									else MessageBox( 0, _TEXT( "This server is already in your favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
								}
							}
						}
						break;

					case 2:
						{
							int index = ListView_GetNextItem(CMainWindow::GetOfficialList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lOfficialServers[ index ];
								if ( pServer )
								{
									if ( !(pServer->GetType() & E_ST_FAV) )
									{
										pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_FAV) );

										pServer->Query();

										pServer->UpdateList();

										MessageBox( 0, _TEXT( "Added to favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
									}
									else MessageBox( 0, _TEXT( "This server is already in your favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
								}
							}
						}
						break;

#ifndef _NO_HISTORY
					case 3:
						{
							int index = ListView_GetNextItem(CMainWindow::GetHistoryList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lHistoryServers[ index ];
								if ( pServer )
								{
									if ( !(pServer->GetType() & E_ST_FAV) )
									{
										pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_FAV) );

										pServer->Query();

										pServer->UpdateList();

										MessageBox( 0, _TEXT( "Added to favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
									}
									else MessageBox( 0, _TEXT( "This server is already in your favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
								}
							}
						}
						break;
#endif

#ifndef _NO_LAN_MODE
					case LAN_MODE_TAB_ID:
						{
							int index = ListView_GetNextItem(CMainWindow::GetLANList(),-1,LVNI_SELECTED);
							if(index != -1)
							{
								CServers* pServer = CMainWindow::m_lLANServers[ index ];
								if ( pServer )
								{
									if ( !(pServer->GetType() & E_ST_FAV) )
									{
										pServer->SetType( (E_SERVER_TYPE)(pServer->GetType() | E_ST_FAV) );

										pServer->Query();

										pServer->UpdateList();

										MessageBox( 0, _TEXT( "Added to favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
									}
									else MessageBox( 0, _TEXT( "This server is already in your favourites." ), _TEXT( "Add To Favourites" ), MB_OK );
								}
							}
						}
						break;
#endif
					}
				}
				break;
			}
		}
		break;

	case WM_NOTIFY:
		{
			// get the tab message from lParam

			switch( ((LPNMHDR)lParam)->code )
			{
			case TCN_SELCHANGE:
				{
					// get the currently selected tab item
					int iPage = TabCtrl_GetCurSel( CMainWindow::GetTabControl() );

					HMENU hMenu = GetMenu( CMainWindow::GetMain() );

					CMainWindow::ClearPlayerList();
					CMainWindow::ClearServerInfo();

					// hide and show the appropriate tab view
					// based on which tab item was clicked
					switch (iPage)
					{
					case 0:
						{
							ShowWindow( CMainWindow::GetFavsList(), SW_SHOW );
							ShowWindow( CMainWindow::GetInternetList(), SW_HIDE );
							ShowWindow( CMainWindow::GetOfficialList(), SW_HIDE );
#ifndef _NO_HISTORY
							ShowWindow( CMainWindow::GetHistoryList(), SW_HIDE );
#endif
#ifndef _NO_LAN_MODE
							ShowWindow( CMainWindow::GetLANList(), SW_HIDE );
#endif

							EnableMenuItem( hMenu, ID_SERVER_ADDSERVER, MF_ENABLED );
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHMASTERLIST, MF_DISABLED );

#ifndef _NO_LAN_MODE
							CLANServerQuery::Stop();
#endif		
						}
						break;

						// Internet Tab
					case 1:
						{
							ShowWindow( CMainWindow::GetFavsList(), SW_HIDE );
							ShowWindow( CMainWindow::GetInternetList(), SW_SHOW );
							ShowWindow( CMainWindow::GetOfficialList(), SW_HIDE );
#ifndef _NO_HISTORY
							ShowWindow( CMainWindow::GetHistoryList(), SW_HIDE );
#endif
#ifndef _NO_LAN_MODE
							ShowWindow( CMainWindow::GetLANList(), SW_HIDE );
#endif

							EnableMenuItem( hMenu, ID_SERVER_ADDSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHMASTERLIST, MF_ENABLED );

#ifndef _NO_LAN_MODE
							CLANServerQuery::Stop();
#endif

							CServerQuery::UpdateInternetList();
						}
						break;

					case 2:
						{
							ShowWindow( CMainWindow::GetFavsList(), SW_HIDE );
							ShowWindow( CMainWindow::GetInternetList(), SW_HIDE );
							ShowWindow( CMainWindow::GetOfficialList(), SW_SHOW );
#ifndef _NO_HISTORY
							ShowWindow( CMainWindow::GetHistoryList(), SW_HIDE );
#endif
#ifndef _NO_LAN_MODE
							ShowWindow( CMainWindow::GetLANList(), SW_HIDE );
#endif

							EnableMenuItem( hMenu, ID_SERVER_ADDSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHMASTERLIST, MF_ENABLED );

#ifndef _NO_LAN_MODE
							CLANServerQuery::Stop();
#endif

							CServerQuery::UpdateOfficialList();
						}
						break;

#ifndef _NO_HISTORY
					case 3:
						{
							ShowWindow( CMainWindow::GetFavsList(), SW_HIDE );
							ShowWindow( CMainWindow::GetInternetList(), SW_HIDE );
							ShowWindow( CMainWindow::GetOfficialList(), SW_HIDE );
							ShowWindow( CMainWindow::GetHistoryList(), SW_SHOW );
#ifndef _NO_LAN_MODE
							ShowWindow( CMainWindow::GetLANList(), SW_HIDE );
#endif

							EnableMenuItem( hMenu, ID_SERVER_ADDSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHMASTERLIST, MF_DISABLED );

#ifndef _NO_LAN_MODE
							CLANServerQuery::Stop();
#endif
						}
						break;
#endif

#ifndef _NO_LAN_MODE
					case LAN_MODE_TAB_ID:
						{
							ShowWindow( CMainWindow::GetFavsList(), SW_HIDE );
							ShowWindow( CMainWindow::GetInternetList(), SW_HIDE );
							ShowWindow( CMainWindow::GetOfficialList(), SW_HIDE );
#ifndef _NO_HISTORY
							ShowWindow( CMainWindow::GetHistoryList(), SW_HIDE );
#endif
							ShowWindow( CMainWindow::GetLANList(), SW_SHOW );

							EnableMenuItem( hMenu, ID_SERVER_ADDSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHSERVER, MF_DISABLED );
							EnableMenuItem( hMenu, ID_SERVER_REFRESHMASTERLIST, MF_DISABLED );

							// We should only listen for lan stuff when we have this tab open
							CLANServerQuery::Start();

							CLANServerQuery::Broadcast( 2301 );
						}
						break;
#endif

					}
				}
				break;

			case NM_CLICK:
				{
					NMHDR* hdr = (NMHDR FAR*)lParam;
					if ( hdr->hwndFrom == CMainWindow::GetFavsList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetFavsList(),-1,LVNI_SELECTED);
						HMENU hMenu = GetMenu( CMainWindow::GetMain() );

						if(index != -1)
						{
							if ( CMainWindow::m_lFavServers[ index ] )
							{
								EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_ENABLED );
								CMainWindow::m_lFavServers[ index ]->QueryWithPlayers();

								CMainWindow::UpdateServerInfo( index, CMainWindow::m_lFavServers[ index ] );
							}
						}
						else 
						{
							EnableMenuItem( hMenu, ID_SERVER_REMOVESERVER, MF_DISABLED );
							CMainWindow::ClearPlayerList();

							CMainWindow::ClearServerInfo();
						}
					}

					else if ( hdr->hwndFrom == CMainWindow::GetInternetList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetInternetList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							if ( CMainWindow::m_lInternetServers[ index ] )
							{
								CMainWindow::m_lInternetServers[ index ]->QueryWithPlayers();
								CMainWindow::UpdateServerInfo( index, CMainWindow::m_lInternetServers[ index ] );
							}
						}
						else
						{
							CMainWindow::ClearPlayerList();

							CMainWindow::ClearServerInfo();
						}
					}

					else if ( hdr->hwndFrom == CMainWindow::GetOfficialList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetOfficialList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							if ( CMainWindow::m_lOfficialServers[ index ] )
							{
								CMainWindow::m_lOfficialServers[ index ]->QueryWithPlayers();
								CMainWindow::UpdateServerInfo( index, CMainWindow::m_lOfficialServers[ index ] );
							}
						}
						else
						{
							CMainWindow::ClearPlayerList();

							CMainWindow::ClearServerInfo();
						}
					}

#ifndef _NO_HISTORY
					else if ( hdr->hwndFrom == CMainWindow::GetHistoryList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetHistoryList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							if ( CMainWindow::m_lHistoryServers[ index ] )
							{
								CMainWindow::m_lHistoryServers[ index ]->QueryWithPlayers();
								CMainWindow::UpdateServerInfo( index, CMainWindow::m_lHistoryServers[ index ] );
							}
						}
						else
						{
							CMainWindow::ClearPlayerList();

							CMainWindow::ClearServerInfo();
						}
					}
#endif

#ifndef _NO_LAN_MODE
					else if ( hdr->hwndFrom == CMainWindow::GetLANList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetLANList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							if ( CMainWindow::m_lLANServers[ index ] )
							{
								CMainWindow::m_lLANServers[ index ]->QueryWithPlayers();
								CMainWindow::UpdateServerInfo( index, CMainWindow::m_lLANServers[ index ] );
							}
						}
						else
						{
							CMainWindow::ClearPlayerList();

							CMainWindow::ClearServerInfo();
						}
					}
#endif
					return TRUE;
				}
				break;

			case NM_DBLCLK:
				{
					NMHDR* hdr = (NMHDR FAR*)lParam;
					if ( hdr->hwndFrom == CMainWindow::GetFavsList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetFavsList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CServers* pServer = CMainWindow::m_lFavServers[ index ];
							if ( pServer )
							{
								ConnectToServer( hwnd, pServer );
							}
						}
					}

					else if ( hdr->hwndFrom == CMainWindow::GetInternetList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetInternetList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CServers* pServer = CMainWindow::m_lInternetServers[ index ];
							if ( pServer )
							{
								ConnectToServer( hwnd, pServer );
							}
						}
					}

					else if ( hdr->hwndFrom == CMainWindow::GetOfficialList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetOfficialList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CServers* pServer = CMainWindow::m_lOfficialServers[ index ];
							if ( pServer )
							{
								ConnectToServer( hwnd, pServer );
							}
						}
					}

#ifndef _NO_HISTORY
					else if ( hdr->hwndFrom == CMainWindow::GetHistoryList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetHistoryList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CServers* pServer = CMainWindow::m_lHistoryServers[ index ];
							if ( pServer )
							{
								ConnectToServer( hwnd, pServer );
							}
						}
					}
#endif

#ifndef _NO_LAN_MODE
					else if ( hdr->hwndFrom == CMainWindow::GetLANList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetLANList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CServers* pServer = CMainWindow::m_lLANServers[ index ];
							if ( pServer )
							{
								ConnectToServer( hwnd, pServer );
							}
						}
					}
#endif
				}
				break;

			case NM_RCLICK:
				{
					NMHDR* hdr = (NMHDR FAR*)lParam;
					HMENU hMenu = LoadMenu (NULL, MAKEINTRESOURCE (IDR_RCLICKMENU));
					if ( hdr->hwndFrom == CMainWindow::GetFavsList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetFavsList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CreateContextMenu( hwnd, hMenu, 0 );
						}
					}
					else if ( hdr->hwndFrom == CMainWindow::GetInternetList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetInternetList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CreateContextMenu( hwnd, hMenu, 1 );
						}
					}
					else if ( hdr->hwndFrom == CMainWindow::GetOfficialList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetOfficialList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CreateContextMenu( hwnd, hMenu, 1 );
						}
					}
#ifndef _NO_HISTORY
					else if ( hdr->hwndFrom == CMainWindow::GetHistoryList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetHistoryList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CreateContextMenu( hwnd, hMenu, 1 );
						}
					}
#endif
#ifndef _NO_LAN_MODE
					else if ( hdr->hwndFrom == CMainWindow::GetLANList() )
					{
						int index = ListView_GetNextItem(CMainWindow::GetLANList(),-1,LVNI_SELECTED);
						if(index != -1)
						{
							CreateContextMenu( hwnd, hMenu, 1 );
						}
					}
#endif

					DestroyMenu (hMenu);
				}
				break;

			case LVN_GETDISPINFO:
				{
					LV_DISPINFO* lvd = (LV_DISPINFO FAR*)lParam;
					if((((LPNMHDR)lParam)->hwndFrom == CMainWindow::GetFavsList()))
					{
						if ( !CMainWindow::m_lFavServers[ lvd->item.iItem ] ) 
						{
							lvd->item.pszText = _TEXT("");
							break;
						}

						switch(lvd->item.iSubItem)
						{
						case 0:
							{
								if ( CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetLocked() ) lvd->item.iImage = 1;
								else lvd->item.iImage = 0;
							}
							break;
						case 1:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerName(), -1, lvd->item.pszText, strlen( CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerName() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerName() );
#endif
							break;
						case 2:
							_stprintf( lvd->item.pszText, _TEXT("%u"), CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerPing() );
							break;
						case 3:
							_stprintf( lvd->item.pszText, _TEXT("%u/%u"), CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerPlayers(), CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerMaxPlayers() );
							break;
						case 4:
							{
								_tcscpy( lvd->item.pszText, CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerMap());
							}
							break;

						case 5:
							{
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerMode(), -1, lvd->item.pszText, strlen( CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerMode() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lFavServers[ lvd->item.iItem ]->GetServerMode() );
#endif
							}
							break;
						}
						//FillFavsList();
					}
					else if((((LPNMHDR)lParam)->hwndFrom == CMainWindow::GetInternetList()))
					{
						if ( !CMainWindow::m_lInternetServers[ lvd->item.iItem ] ) 
						{
							lvd->item.pszText = _TEXT("");
							break;
						}

						switch(lvd->item.iSubItem)
						{
						case 0:
							{
								if ( CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetLocked() ) lvd->item.iImage = 1;
								else lvd->item.iImage = 0;
							}
							break;
						case 1:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerName(), -1, lvd->item.pszText, strlen( CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerName() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerName() );
#endif
							break;
						case 2:
							_stprintf( lvd->item.pszText, _TEXT("%u"), CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerPing() );
							break;
						case 3:
							_stprintf( lvd->item.pszText, _TEXT("%u/%u"), CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerPlayers(), CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerMaxPlayers() );
							break;
						case 4:
							{
								_tcscpy( lvd->item.pszText, CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerMap());
							}
							break;

						case 5:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerMode(), -1, lvd->item.pszText, strlen( CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerMode() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lInternetServers[ lvd->item.iItem ]->GetServerMode() );
#endif
							break;
						}
						//FillInternetList();
					}
					else if((((LPNMHDR)lParam)->hwndFrom == CMainWindow::GetOfficialList()))
					{
						if ( !CMainWindow::m_lOfficialServers[ lvd->item.iItem ] ) 
						{
							lvd->item.pszText = _TEXT("");
							break;
						}

						switch(lvd->item.iSubItem)
						{
						case 0:
							{
								if ( CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetLocked() ) lvd->item.iImage = 1;
								else lvd->item.iImage = 0;
							}
							break;
						case 1:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerName(), -1, lvd->item.pszText, strlen( CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerName() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerName() );
#endif
							break;
						case 2:
							_stprintf( lvd->item.pszText, _TEXT("%u"), CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerPing() );
							break;
						case 3:
							_stprintf( lvd->item.pszText, _TEXT("%u/%u"), CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerPlayers(), CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerMaxPlayers() );
							break;
						case 4:
							{
								_tcscpy( lvd->item.pszText, CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerMap());
							}
							break;

						case 5:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerMode(), -1, lvd->item.pszText, strlen( CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerMode() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lOfficialServers[ lvd->item.iItem ]->GetServerMode() );
#endif
							break;
						}
						//FillOfficialList();
					}
#ifndef _NO_HISTORY
					else if((((LPNMHDR)lParam)->hwndFrom == CMainWindow::GetHistoryList()))
					{
						if ( !CMainWindow::m_lHistoryServers[ lvd->item.iItem ] ) 
						{
							lvd->item.pszText = _TEXT("");
							break;
						}
						switch(lvd->item.iSubItem)
						{
						case 0:
							{
								if ( CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetLocked() ) lvd->item.iImage = 1;
								else lvd->item.iImage = 0;
							}
							break;
						case 1:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerName(), -1, lvd->item.pszText, strlen( CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerName() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerName() );
#endif
							break;
						case 2:
							_stprintf( lvd->item.pszText, _TEXT("%u"), CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerPing() );
							break;
						case 3:
							_stprintf( lvd->item.pszText, _TEXT("%u/%u"), CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerPlayers(), CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerMaxPlayers() );
							break;
						case 4:
							{
								_tcscpy( lvd->item.pszText, CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerMap());
							}
							break;

						case 5:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerMode(), -1, lvd->item.pszText, strlen( CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerMode() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lHistoryServers[ lvd->item.iItem ]->GetServerMode() );
#endif
							break;
						}
						//FillHistoryList();
					}
#endif
#ifndef _NO_LAN_MODE
					else if((((LPNMHDR)lParam)->hwndFrom == CMainWindow::GetLANList()))
					{
						if ( !CMainWindow::m_lLANServers[ lvd->item.iItem ] ) 
						{
							lvd->item.pszText = _TEXT("");
							break;
						}

						switch(lvd->item.iSubItem)
						{
						case 0:
							{
								if ( CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetLocked() ) lvd->item.iImage = 1;
								else lvd->item.iImage = 0;
							}
							break;
						case 1:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerName(), -1, lvd->item.pszText, strlen( CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerName() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerName() );
#endif
							break;
						case 2:
							_stprintf( lvd->item.pszText, _TEXT("%u"), CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerPing() );
							break;
						case 3:
							_stprintf( lvd->item.pszText, _TEXT("%u/%u"), CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerPlayers(), CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerMaxPlayers() );
							break;
						case 4:
							{
								_tcscpy( lvd->item.pszText, CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerMap());
							}
							break;

						case 5:
#ifdef _UNICODE
							MultiByteToWideChar( CP_ACP, 0, CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerMode(), -1, lvd->item.pszText, strlen( CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerMode() )+1 );
#else
							strcpy( lvd->item.pszText, CMainWindow::m_lLANServers[ lvd->item.iItem ]->GetServerMode() );
#endif
							break;
						}
					}
#endif
					return TRUE;
				}
				break;
			}
		}
		break;

	case WM_UPDATESERVERLIST:
		{
			CServers* pServer = (CServers*)lParam;
			if ( pServer )
				pServer->UpdateList();
		}
		break;

	case WM_UPDATEPLAYERLIST:
		{
			CServers* pServer = (CServers*)lParam;
			if ( pServer )
				pServer->UpdatePlayerList();
		}
		break;

	case WM_CLOSE:
		{
			// Save the favs list
			CFavourites::Write();
			
			DestroyWindow(hwnd);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

bool CMainWindow::Create( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine )
{
	WNDCLASSEX wc;
	INITCOMMONCONTROLSEX    icex;
	HICON hIcon = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE(IDI_ICON1) );

	wc.cbSize = sizeof( WNDCLASSEX );
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = hIcon;
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName = _TEXT("VCMPBrowser");
	wc.hIconSm = hIcon;

	if ( !RegisterClassEx( &wc ) )
	{
#ifdef _DEBUG
		MessageBox( NULL, _TEXT("Failed to register a window"), _TEXT("Critical Failure"), MB_ICONEXCLAMATION | MB_OK );
#endif
		return false;
	}

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	if ( !InitCommonControlsEx(&icex) )
	{
#ifdef _DEBUG
		MessageBox( NULL, _TEXT("Failed to init common controls"), _TEXT("Critical Failure"), MB_ICONEXCLAMATION | MB_OK );
#endif
		return false;
	}	

	m_hWnd = CreateWindowEx( WS_EX_CLIENTEDGE, _TEXT("VCMPBrowser"), _TEXT("Vice City Multiplayer 0.3z R2 Server Browser - Created by VRocker"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL );

	if ( !m_hWnd )
	{
#ifdef _DEBUG
		MessageBox( NULL, _TEXT("Failed to create a window"), _TEXT("Critical Failure"), MB_ICONEXCLAMATION | MB_OK );
#endif
		return false;
	}

	SendMessage(m_hWnd, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);

	return true;
}

bool CMainWindow::CreateStatusBar( void )
{
	m_hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL,
		WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
		m_hWnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);

	int statwidths[] = {250, -1};

	SendMessage(m_hStatusBar, SB_SETPARTS, sizeof(statwidths)/sizeof(int), (LPARAM)statwidths);

	SendMessage(m_hStatusBar, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);
	return true;
}

bool CMainWindow::CreateTabControl( void )
{
	m_hTabControl = CreateWindowEx(0, WC_TABCONTROL, _TEXT(""),
		WS_CHILD|WS_CLIPSIBLINGS|WS_VISIBLE|TCS_TABS,
		0, 0, 
		300, 300, 
		m_hWnd, 
		NULL, 
		GetModuleHandle(NULL), 
		NULL); 

	SendMessage(m_hTabControl, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);

	TCITEM tie;
	tie.mask = TCIF_TEXT; 
	tie.pszText = _TEXT("Favourites"); 
	TabCtrl_InsertItem(m_hTabControl, 0, &tie);

	tie.mask = TCIF_TEXT; 
	tie.pszText = _TEXT("Internet"); 
	TabCtrl_InsertItem(m_hTabControl, 1, &tie);

	tie.mask = TCIF_TEXT; 
	tie.pszText = _TEXT("Official"); 
	TabCtrl_InsertItem(m_hTabControl, 2, &tie);

#ifndef _NO_HISTORY
	tie.mask = TCIF_TEXT; 
	tie.pszText = _TEXT("History"); 
	TabCtrl_InsertItem(m_hTabControl, 3, &tie);
#endif

#ifndef _NO_LAN_MODE
	tie.mask = TCIF_TEXT; 
	tie.pszText = _TEXT("LAN"); 
	TabCtrl_InsertItem(m_hTabControl, 4, &tie);
#endif

	TabCtrl_SetCurSel(m_hTabControl, 0);

	ShowWindow(m_hTabControl, SW_SHOW);
	UpdateWindow(m_hTabControl);

	return true;
}

bool CMainWindow::CreateGroupBox( void )
{
	// Side Group Box
	m_hSideGroupBox = CreateWindowEx( 0, WC_BUTTON, _TEXT(""), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | BS_GROUPBOX, 600, 0, 200, 500, m_hWnd, NULL, GetModuleHandle(NULL), NULL );
	SetWindowText( m_hSideGroupBox, _TEXT("Players") );

	m_hFiltersGroupBox = CreateWindowEx( 0, WC_BUTTON, _TEXT(""), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | BS_GROUPBOX, 0, 400, GB_FILTERS_WIDTH, 120, m_hWnd, NULL, GetModuleHandle(NULL), NULL );
	SetWindowText( m_hFiltersGroupBox, _TEXT("Rules") );

	m_hInfoGroupBox = CreateWindowEx( 0, WC_BUTTON, _TEXT(""), WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | BS_GROUPBOX, GB_FILTERS_WIDTH, 400, 450, 120, m_hWnd, NULL, GetModuleHandle(NULL), NULL );
	SetWindowText( m_hInfoGroupBox, _TEXT("Server Info") );

	HWND hServerName = CreateWindowEx( 0, WC_STATIC, _TEXT("Server Name:"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_RIGHT, 10, 20, 100, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );
	HWND hServerIP = CreateWindowEx( 0, WC_STATIC, _TEXT("Server IP:"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_RIGHT, 10, 45, 100, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );
	HWND hServerPlayers = CreateWindowEx( 0, WC_STATIC, _TEXT("Server Players:"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_RIGHT, 10, 70, 100, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );
	HWND hServerPing = CreateWindowEx( 0, WC_STATIC, _TEXT("Server Ping:"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_RIGHT, 10, 95, 100, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );

	m_hLblServerName = CreateWindowEx( 0, WC_STATIC, _TEXT("---"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_LEFT, 115, 20, 275, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );
	m_hLblServerIP = CreateWindowEx( 0, WC_STATIC, _TEXT("---"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_LEFT, 115, 45, 275, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );
	m_hLblServerPlayers = CreateWindowEx( 0, WC_STATIC, _TEXT("---"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_LEFT, 115, 70, 275, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );
	m_hLblServerPing = CreateWindowEx( 0, WC_STATIC, _TEXT("---"),  WS_CHILD | WS_VISIBLE | SS_EDITCONTROL | SS_LEFT, 115, 95, 275, 16, m_hInfoGroupBox, NULL, GetModuleHandle(NULL), NULL );

	//Static_SetText( m_hLblServerIP, "Testing" );

	HFONT font = (HFONT)GetStockObject(ANSI_VAR_FONT);
    if (font)
    {
        // now, get the complete LOGFONT describing this font
        LOGFONT lf;
        if (GetObject(font, sizeof(LOGFONT), &lf))
        {
            // set the weight to bold
            lf.lfWeight = FW_BOLD;

            // recreate this font with just the weight changed
            font = CreateFontIndirect(&lf);
        }
    }

	SendMessage(hServerName, WM_SETFONT, (WPARAM)font, 0);
	SendMessage(hServerIP, WM_SETFONT, (WPARAM)font, 0);
	SendMessage(hServerPlayers, WM_SETFONT, (WPARAM)font, 0);
	SendMessage(hServerPing, WM_SETFONT, (WPARAM)font, 0);

	SendMessage(m_hLblServerName, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);
	SendMessage(m_hLblServerIP, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);
	SendMessage(m_hLblServerPlayers, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);
	SendMessage(m_hLblServerPing, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);

	SendMessage(m_hSideGroupBox, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);
	SendMessage(m_hFiltersGroupBox, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);
	SendMessage(m_hInfoGroupBox, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);

	return true;
}

bool CMainWindow::CreateListBox( void )
{
	if ( !m_hSideGroupBox ) return false;

	m_hPlayerList = CreateWindowEx( 0, WC_LISTBOX, _TEXT(""), WS_CHILD | WS_VISIBLE | WS_VSCROLL, 2, 20, 194, 400, m_hSideGroupBox, NULL, GetModuleHandle(NULL), NULL );

	SendMessage(m_hPlayerList, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), 0);

	return true;
}

bool CMainWindow::CreateListViews( void )
{
	if ( !m_hTabControl ) return false;

	m_hLockedImage = ImageList_Create(16, 16, ILC_COLOR24, 0, 5);

	int index = ImageList_AddIcon( m_hLockedImage, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_UNLOCKED)));
	index = ImageList_AddIcon( m_hLockedImage, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LOCKED)));
	
	// Favs List
	{
		m_hFavsList = CreateWindowEx( 0, WC_LISTVIEW, _TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON | LVS_SHAREIMAGELISTS, 2, 20, 296, 278, m_hTabControl, NULL, GetModuleHandle(NULL), NULL );

		ListView_SetExtendedListViewStyle( m_hFavsList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

		ListView_SetImageList( m_hFavsList, m_hLockedImage, LVSIL_SMALL );

		// Setup the columsn
		LV_COLUMN lvC;

		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 25;
		lvC.pszText = _TEXT("");
		ListView_InsertColumn( m_hFavsList, 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 235;
		lvC.pszText = _TEXT("Hostname");
		ListView_InsertColumn( m_hFavsList, 1, &lvC);

		lvC.iSubItem = 2;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Ping");
		ListView_InsertColumn( m_hFavsList, 2, &lvC);

		lvC.iSubItem = 3;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Players");
		ListView_InsertColumn( m_hFavsList, 3, &lvC);

		lvC.iSubItem = 4;
		lvC.cx = 60;
		lvC.pszText = _TEXT("Map");
		ListView_InsertColumn( m_hFavsList, 4, &lvC);

		lvC.iSubItem = 5;
		lvC.cx = 150;
		lvC.pszText = _TEXT("Gamemode");
		ListView_InsertColumn( m_hFavsList, 5, &lvC);

		/*ListViewItem item( true, _TEXT("LU Test Server"), _TEXT("9999"), _TEXT("0/128"), _TEXT("GTA3") );

		AddToFavsList( item );*/
	}

	// Internet List
	{
		m_hInternetList = CreateWindowEx( 0, WC_LISTVIEW, _TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON | LVS_SHAREIMAGELISTS, 2, 20, 296, 278, m_hTabControl, NULL, GetModuleHandle(NULL), NULL );

		ListView_SetExtendedListViewStyle( m_hInternetList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

		ListView_SetImageList( m_hInternetList, m_hLockedImage, LVSIL_SMALL );

		// Setup the columsn
		LV_COLUMN lvC;

		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 25;
		lvC.pszText = _TEXT("");
		ListView_InsertColumn( m_hInternetList, 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 235;
		lvC.pszText = _TEXT("Hostname");
		ListView_InsertColumn( m_hInternetList, 1, &lvC);

		lvC.iSubItem = 2;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Ping");
		ListView_InsertColumn( m_hInternetList, 2, &lvC);

		lvC.iSubItem = 3;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Players");
		ListView_InsertColumn( m_hInternetList, 3, &lvC);

		lvC.iSubItem = 4;
		lvC.cx = 60;
		lvC.pszText = _TEXT("Map");
		ListView_InsertColumn( m_hInternetList, 4, &lvC);

		lvC.iSubItem = 5;
		lvC.cx = 150;
		lvC.pszText = _TEXT("Gamemode");
		ListView_InsertColumn( m_hInternetList, 5, &lvC);

		ShowWindow(m_hInternetList, SW_HIDE);
	}

	// Official List
	{
		m_hOfficialList = CreateWindowEx( 0, WC_LISTVIEW, _TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON | LVS_SHAREIMAGELISTS, 2, 20, 296, 278, m_hTabControl, NULL, GetModuleHandle(NULL), NULL );

		ListView_SetExtendedListViewStyle( m_hOfficialList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

		ListView_SetImageList( m_hOfficialList, m_hLockedImage, LVSIL_SMALL );

		// Setup the columsn
		LV_COLUMN lvC;

		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 25;
		lvC.pszText = _TEXT("");
		ListView_InsertColumn( m_hOfficialList, 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 235;
		lvC.pszText = _TEXT("Hostname");
		ListView_InsertColumn( m_hOfficialList, 1, &lvC);

		lvC.iSubItem = 2;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Ping");
		ListView_InsertColumn( m_hOfficialList, 2, &lvC);

		lvC.iSubItem = 3;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Players");
		ListView_InsertColumn( m_hOfficialList, 3, &lvC);

		lvC.iSubItem = 4;
		lvC.cx = 60;
		lvC.pszText = _TEXT("Map");
		ListView_InsertColumn( m_hOfficialList, 4, &lvC);

		lvC.iSubItem = 5;
		lvC.cx = 150;
		lvC.pszText = _TEXT("Gamemode");
		ListView_InsertColumn( m_hOfficialList, 5, &lvC);

		ShowWindow(m_hOfficialList, SW_HIDE);
	}

#ifndef _NO_HISTORY
	// History List
	{
		m_hHistoryList = CreateWindowEx( 0, WC_LISTVIEW, _TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_ICON | LVS_SHAREIMAGELISTS, 2, 20, 296, 278, m_hTabControl, NULL, GetModuleHandle(NULL), NULL );

		ListView_SetExtendedListViewStyle( m_hHistoryList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

		ListView_SetImageList( m_hHistoryList, m_hLockedImage, LVSIL_SMALL );

		// Setup the columsn
		LV_COLUMN lvC;

		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 25;
		lvC.pszText = _TEXT("");
		ListView_InsertColumn( m_hHistoryList, 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 235;
		lvC.pszText = _TEXT("Hostname");
		ListView_InsertColumn( m_hHistoryList, 1, &lvC);

		lvC.iSubItem = 2;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Ping");
		ListView_InsertColumn( m_hHistoryList, 2, &lvC);

		lvC.iSubItem = 3;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Players");
		ListView_InsertColumn( m_hHistoryList, 3, &lvC);

		lvC.iSubItem = 4;
		lvC.cx = 60;
		lvC.pszText = _TEXT("Map");
		ListView_InsertColumn( m_hHistoryList, 4, &lvC);

		lvC.iSubItem = 5;
		lvC.cx = 95;
		lvC.pszText = _TEXT("Last Played");
		ListView_InsertColumn( m_hHistoryList, 5, &lvC);

		ShowWindow(m_hHistoryList, SW_HIDE);
	}
#endif

#ifndef _NO_LAN_MODE
	// LAN Servers List
	{
		m_hLANList = CreateWindowEx( 0, WC_LISTVIEW, _TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_ICON | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS, 2, 20, 296, 278, m_hTabControl, NULL, GetModuleHandle(NULL), NULL );

		ListView_SetExtendedListViewStyle( m_hLANList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

		ListView_SetImageList( m_hLANList, m_hLockedImage, LVSIL_SMALL );

		// Setup the columsn
		LV_COLUMN lvC;

		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 25;
		lvC.pszText = _TEXT("");
		ListView_InsertColumn( m_hLANList, 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 235;
		lvC.pszText = _TEXT("Hostname");
		ListView_InsertColumn( m_hLANList, 1, &lvC);

		lvC.iSubItem = 2;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Ping");
		ListView_InsertColumn( m_hLANList, 2, &lvC);

		lvC.iSubItem = 3;
		lvC.cx = 50;
		lvC.pszText = _TEXT("Players");
		ListView_InsertColumn( m_hLANList, 3, &lvC);

		lvC.iSubItem = 4;
		lvC.cx = 60;
		lvC.pszText = _TEXT("Map");
		ListView_InsertColumn( m_hLANList, 4, &lvC);

		lvC.iSubItem = 5;
		lvC.cx = 150;
		lvC.pszText = _TEXT("Gamemode");
		ListView_InsertColumn( m_hLANList, 5, &lvC);

		ShowWindow(m_hLANList, SW_HIDE);
	}
#endif

	// Rules List
	if ( m_hFiltersGroupBox )
	{
		m_hRulesList = CreateWindowEx( 0, WC_LISTVIEW, _TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS , 2, 10, GB_FILTERS_WIDTH - 2, 100, m_hFiltersGroupBox, NULL, GetModuleHandle(NULL), NULL );

		ListView_SetExtendedListViewStyle( m_hRulesList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

		// Setup the columsn
		LV_COLUMN lvC;

		lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvC.fmt = LVCFMT_LEFT;

		lvC.iSubItem = 0;
		lvC.cx = 125;
		lvC.pszText = _TEXT("Key");
		ListView_InsertColumn( m_hRulesList, 0, &lvC);

		lvC.iSubItem = 1;
		lvC.cx = 250;
		lvC.pszText = _TEXT("Value");
		ListView_InsertColumn( m_hRulesList, 1, &lvC);

		/*ListViewItem item( true, _TEXT("LU Test Server"), _TEXT("9999"), _TEXT("0/128"), _TEXT("GTA3") );

		AddToFavsList( item );*/
	}

	return true;
}

void CMainWindow::Show( int nCmdShow )
{
	ShowWindow( m_hWnd, nCmdShow );
	UpdateWindow( m_hWnd );
}

void CMainWindow::Resize( void )
{
	RECT winRect;
	GetClientRect( m_hWnd, &winRect );

	int iStatusHeight = 0;
	int iGroupboxWidth = 0;
	int iGroupboxHeight = 0;
	int iFiltersBoxHeight = 0;
	int iFiltersBoxWidth = 0;

	if ( m_hStatusBar ) 
	{
		SendMessage( m_hStatusBar, WM_SIZE, 0, 0 );

		RECT statusRect;
		GetWindowRect( m_hStatusBar, &statusRect );
		iStatusHeight = statusRect.bottom - statusRect.top;
	}

	if ( m_hFiltersGroupBox )
	{
		SetWindowPos( m_hFiltersGroupBox, NULL, 2, winRect.bottom - 120 - iStatusHeight, GB_FILTERS_WIDTH, 120, SWP_NOZORDER );

		RECT filtersboxRect;
		GetWindowRect( m_hFiltersGroupBox, &filtersboxRect );
		iFiltersBoxWidth = filtersboxRect.right - filtersboxRect.left;
		iFiltersBoxHeight = filtersboxRect.bottom - filtersboxRect.top;

		if ( m_hRulesList )
			SetWindowPos( m_hRulesList, NULL, 2, 14, iFiltersBoxWidth - 4, iFiltersBoxHeight - 16, SWP_NOZORDER);
	}

	if ( m_hInfoGroupBox )
	{
		SetWindowPos( m_hInfoGroupBox, NULL, GB_FILTERS_WIDTH + 2, winRect.bottom - 120 - iStatusHeight, winRect.right - GB_FILTERS_WIDTH - 4, 120, SWP_NOZORDER );
	}

	if ( m_hSideGroupBox )
	{
		SetWindowPos( m_hSideGroupBox, NULL, winRect.right - 200, 0, 200, winRect.bottom - iStatusHeight - iFiltersBoxHeight, SWP_NOZORDER );

		RECT groupboxRect;
		GetWindowRect( m_hSideGroupBox, &groupboxRect );
		iGroupboxWidth = groupboxRect.right - groupboxRect.left;
		iGroupboxHeight = groupboxRect.bottom - groupboxRect.top;
		//iGroupboxWidth += 4;

		if ( m_hPlayerList )
			SetWindowPos( m_hPlayerList, NULL, 3, 15, iGroupboxWidth - 6, iGroupboxHeight - 18, SWP_NOZORDER );
	}

	if ( m_hTabControl ) 
	{
		// Need to do some sizing here
		SetWindowPos( m_hTabControl, NULL, 2, 0, winRect.right - iGroupboxWidth, winRect.bottom - iStatusHeight - iFiltersBoxHeight, SWP_NOZORDER);

		RECT tabRect;
		GetWindowRect( m_hTabControl, &tabRect );
		int iTabHeight = tabRect.bottom - tabRect.top;
		int iTabWidth = tabRect.right - tabRect.left;

		if ( m_hFavsList )
			SetWindowPos( m_hFavsList, NULL, 2, 20, iTabWidth - 6, iTabHeight - 22, SWP_NOZORDER);

		if ( m_hInternetList )
			SetWindowPos( m_hInternetList, NULL, 2, 20, iTabWidth - 6, iTabHeight - 22, SWP_NOZORDER);

		if ( m_hOfficialList )
			SetWindowPos( m_hOfficialList, NULL, 2, 20, iTabWidth - 6, iTabHeight - 22, SWP_NOZORDER);

#ifndef _NO_HISTORY
		if ( m_hHistoryList )
			SetWindowPos( m_hHistoryList, NULL, 2, 20, iTabWidth - 6, iTabHeight - 22, SWP_NOZORDER);
#endif

#ifndef _NO_LAN_MODE
		if ( m_hLANList )
			SetWindowPos( m_hLANList, NULL, 2, 20, iTabWidth - 6, iTabHeight - 22, SWP_NOZORDER);
#endif
	}
}

void CMainWindow::ClearServerInfo( void )
{
	Static_SetText( m_hLblServerName, "---" );
	Static_SetText( m_hLblServerIP, "---" );
	Static_SetText( m_hLblServerPlayers, "---" );
	Static_SetText( m_hLblServerPing, "---" );
}

void CMainWindow::UpdateServerInfo( size_t index, CServers* pServer )
{
	if ( !pServer ) 
		return;

	int selIndex = 0;
	int iPage = TabCtrl_GetCurSel( m_hTabControl );
	switch( iPage )
	{
	case 0:
		selIndex = ListView_GetNextItem(m_hFavsList,-1,LVNI_SELECTED);
		break;

	case 1:
		selIndex = ListView_GetNextItem(m_hInternetList,-1,LVNI_SELECTED);
		break;

	case 2:
		selIndex = ListView_GetNextItem(m_hOfficialList,-1,LVNI_SELECTED);
		break;

#ifndef _NO_HISTORY
	case 3:
		selIndex = ListView_GetNextItem(m_hHistoryList,-1,LVNI_SELECTED);
		break;
#endif

#ifndef _NO_LAN_MODE
	case LAN_MODE_TAB_ID:
		selIndex = ListView_GetNextItem(m_hLANList,-1,LVNI_SELECTED);
		break;
#endif

	default:
		selIndex = -1;
	}

	if ( selIndex == index )
	{
		Static_SetText( m_hLblServerName, pServer->GetServerName() );
		TCHAR szOut[ 64 ] = { 0 };
		_stprintf( szOut, "%s:%u",  pServer->GetServerIP(), pServer->GetServerPort() );
		Static_SetText( m_hLblServerIP, szOut );
		_stprintf( szOut, "%u/%u", pServer->GetServerPlayers(), pServer->GetServerMaxPlayers() );
		Static_SetText( m_hLblServerPlayers, szOut );
		_stprintf( szOut, "%u", pServer->GetServerPing() );
		Static_SetText( m_hLblServerPing, szOut );
	}
}

size_t CMainWindow::AddToFavsList( CServers* p )
{
	LV_ITEM litem;
	litem.mask = LVIF_IMAGE;
	/*if ( p->GetLocked() )
		litem.iImage = 1;
	else
		litem.iImage = 0;*/
	litem.iImage = I_IMAGECALLBACK;

	litem.iSubItem = 0;
	litem.iItem = m_uiFavServers++;
	size_t uiIndex = ListView_InsertItem( m_hFavsList, &litem );

	m_lFavServers[ uiIndex ] = p;
	return uiIndex;
}

void CMainWindow::RemoveFromFavsList( CServers* p )
{
	if ( !p ) return;

	size_t uiIndex = p->GetListItem( E_LT_FAV );
	
	if ( uiIndex == -1 ) return;

	p->SetListItem( E_LT_FAV, -1 );
	ListView_DeleteItem( m_hFavsList, uiIndex );
	m_lFavServers[ uiIndex ] = NULL;

	if ( p->GetType() == E_ST_FAV ) CServerManager::Remove( p );
	else p->SetType( (E_SERVER_TYPE)( p->GetType() ^ E_ST_FAV ) );

	// The following loop moves all entries below the removed one to move up in the list
	CServers* pOther = NULL;
	for ( size_t ui = uiIndex + 1; ui < m_uiFavServers; ui++ )
	{
		pOther = m_lFavServers[ ui ];
		if ( pOther )
		{
			m_lFavServers[ ui ] = NULL;

			LV_ITEM litem;
			litem.mask = LVIF_IMAGE;
			if ( pOther->GetLocked() )
				litem.iImage = 1;
			else
				litem.iImage = 0;

			litem.iSubItem = 0;
			litem.iItem = ui - 1;
			pOther->SetListItem( E_LT_FAV, ui - 1 );
			m_lFavServers[ ui - 1 ] = pOther;
		}
	}

	m_uiFavServers--;
}

void CMainWindow::ClearFavsList( void )
{
	CServers* p = NULL;
	for ( size_t ui = 0; ui < m_uiFavServers; ui++ )
	{
		p = m_lFavServers[ ui ];
		if ( p )
		{
			p->SetListItem( E_LT_FAV, -1 );
			m_lFavServers[ ui ] = NULL;

			if ( p->GetType() == E_ST_FAV ) CServerManager::Remove( p );
			else p->SetType( (E_SERVER_TYPE)( p->GetType() ^ E_ST_FAV ) );
		}
	}
	m_uiFavServers = 0;

	ListView_DeleteAllItems( m_hFavsList );
}

size_t CMainWindow::AddToInternetList( CServers* p )
{
	LV_ITEM litem;
	litem.mask = LVIF_IMAGE;
	litem.iImage = I_IMAGECALLBACK;
	litem.iSubItem = 0;
	litem.iItem = m_uiInternetServers;

	m_lInternetServers[ m_uiInternetServers ] = p;
	ListView_InsertItem( m_hInternetList, &litem );

	return m_uiInternetServers++;
}

void CMainWindow::ClearInternetList( void )
{
	CServers* p = NULL;
	for ( size_t ui = 0; ui < m_uiInternetServers; ui++ )
	{
		p = m_lInternetServers[ ui ];
		if ( p )
		{
			p->SetListItem( E_LT_INTERNET, -1 );
			m_lInternetServers[ ui ] = NULL;

			if ( p->GetType() == E_ST_INTERNET ) CServerManager::Remove( p );
			else p->SetType( (E_SERVER_TYPE)( p->GetType() ^ E_ST_INTERNET ) );

		}
	}
	m_uiInternetServers = 0;

	ListView_DeleteAllItems( m_hInternetList );
}

size_t CMainWindow::AddToOfficialList( CServers* p )
{
	LV_ITEM litem;
	litem.mask = LVIF_IMAGE;
	litem.iImage = I_IMAGECALLBACK;
	litem.iSubItem = 0;
	litem.iItem = m_uiOfficialServers;

	m_lOfficialServers[ m_uiOfficialServers ] = p;

	ListView_InsertItem( m_hOfficialList, &litem );

	return m_uiOfficialServers++;
}

void CMainWindow::ClearOfficialList( void )
{
	CServers* p = NULL;
	for ( size_t ui = 0; ui < m_uiOfficialServers; ui++ )
	{
		p = m_lOfficialServers[ ui ];
		if ( p )
		{
			p->SetListItem( E_LT_OFFICIAL, -1 );
			m_lOfficialServers[ ui ] = NULL;

			if ( p->GetType() == E_ST_INTERNET ) CServerManager::Remove( p );
			else p->SetType( (E_SERVER_TYPE)( p->GetType() ^ E_LT_OFFICIAL ) );

		}
	}
	m_uiOfficialServers = 0;

	ListView_DeleteAllItems( m_hOfficialList );
}

#ifndef _NO_HISTORY
size_t CMainWindow::AddToHistoryList( CHistoryServer* p )
{
	LV_ITEM litem;
	litem.mask = LVIF_IMAGE;
	litem.iImage = I_IMAGECALLBACK;
	litem.iSubItem = 0;
	litem.iItem = m_uiHistoryServers;

	m_lHistoryServers[ m_uiHistoryServers ] = p;

	ListView_InsertItem( m_hHistoryList, &litem );

	return m_uiHistoryServers++;
}

void CMainWindow::ClearHistoryList( void )
{
	CServers* p = NULL;
	for ( size_t ui = 0; ui < m_uiHistoryServers; ui++ )
	{
		p = m_lHistoryServers[ ui ];
		if ( p )
		{
			p->SetListItem( E_LT_HISTORY, -1 );
			m_lHistoryServers[ ui ] = NULL;

			if ( p->GetType() == E_LT_HISTORY ) CServerManager::Remove( p );
			else p->SetType( (E_SERVER_TYPE)( p->GetType() ^ E_LT_HISTORY ) );

		}
	}
	m_uiHistoryServers = 0;

	ListView_DeleteAllItems( m_hHistoryList );
}
#endif

#ifndef _NO_LAN_MODE
size_t CMainWindow::AddToLANList( CServers* p )
{
	LV_ITEM litem;
	litem.mask = LVIF_IMAGE;
	litem.iImage = I_IMAGECALLBACK;
	litem.iSubItem = 0;
	litem.iItem = m_uiLANServers;

	m_lLANServers[ m_uiLANServers ] = p;

	ListView_InsertItem( m_hLANList, &litem );
	
	return m_uiLANServers++;
}
#endif

void CMainWindow::UpdateFavsList( size_t index )
{
	ListView_Update( m_hFavsList, index );

}

void CMainWindow::UpdateInternetList( size_t index )
{
	ListView_Update( m_hInternetList, index );
}

void CMainWindow::UpdateOfficialList( size_t index )
{
	ListView_Update( m_hOfficialList, index );
}

#ifndef _NO_HISTORY
void CMainWindow::UpdateHistoryList( size_t index )
{
	ListView_Update( m_hHistoryList, index );
}
#endif

#ifndef _NO_LAN_MODE
void CMainWindow::UpdateLANList( size_t index )
{
	ListView_Update( m_hLANList, index );
}
#endif

void CMainWindow::AddToPlayerList( size_t index, CPlayers* p )
{
	int iPage = GetCurrentTab();
	int selIndex = 0;
	switch( iPage )
	{
	case 0:
		selIndex = ListView_GetNextItem(m_hFavsList,-1,LVNI_SELECTED);
		break;

	case 1:
		selIndex = ListView_GetNextItem(m_hInternetList,-1,LVNI_SELECTED);
		break;

	case 2:
		selIndex = ListView_GetNextItem(m_hOfficialList,-1,LVNI_SELECTED);
		break;

#ifndef _NO_HISTORY
	case 3:
		selIndex = ListView_GetNextItem(m_hHistoryList,-1,LVNI_SELECTED);
		break;
#endif

#ifndef _NO_LAN_MODE
	case LAN_MODE_TAB_ID:
		selIndex = ListView_GetNextItem(m_hLANList,-1,LVNI_SELECTED);
		break;
#endif

	default:
		selIndex = -1;
	}

	if ( selIndex == index )
	{
		if ( m_hPlayerList )
			ListBox_AddString( m_hPlayerList, p->GetName() );
	}
}

void CMainWindow::ClearPlayerList()
{
	if ( m_hPlayerList )
		ListBox_ResetContent( m_hPlayerList );
}

int CMainWindow::GetCurrentTab( void )
{
	return TabCtrl_GetCurSel( m_hTabControl );
}