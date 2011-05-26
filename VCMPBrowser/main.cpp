#include "MainWindow.h"
#include "ServerQuery.h"
#include "Settings.h"
#include "Favourites.h"
#include <shlobj.h>
#include <direct.h>

HANDLE g_hListMutex = 0; 

TCHAR g_szAppDataDir[ MAX_PATH ] = { 0 };

extern void SplitIPAndPort( const char* sz, char szIP[], unsigned short& usPort, size_t uiIPBufSize );
extern void StartGame( const char* szIP, unsigned short usPort, unsigned char ucGame, const char* szPass = 0 );

/*HRESULT _CreateShellLink(PCSTR pszArguments, PCWSTR pszTitle, IShellLink **ppsl)
{
    IShellLink *psl;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));
    if (SUCCEEDED(hr))
    {
        // Determine our executable's file path so the task will execute this application
        CHAR szAppPath[MAX_PATH];
        if (GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath)))
        {
            hr = psl->SetPath(szAppPath);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetArguments(pszArguments);
                if (SUCCEEDED(hr))
                {
                    // The title property is required on Jump List items provided as an IShellLink
                    // instance.  This value is used as the display name in the Jump List.
                    IPropertyStore *pps;
                    hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                    if (SUCCEEDED(hr))
                    {
                        PROPVARIANT propvar;
                        hr = InitPropVariantFromString(pszTitle, &propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->SetValue(PKEY_Title, propvar);
                            if (SUCCEEDED(hr))
                            {
                                hr = pps->Commit();
                                if (SUCCEEDED(hr))
                                {
                                    hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                                }
                            }
                            PropVariantClear(&propvar);
                        }
                        pps->Release();
                    }
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        psl->Release();
    }
    return hr;
}

HRESULT _CreateSeparatorLink(IShellLink **ppsl)
{
    IPropertyStore *pps;
    HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pps));
    if (SUCCEEDED(hr))
    {
        PROPVARIANT propvar;
        hr = InitPropVariantFromBoolean(TRUE, &propvar);
        if (SUCCEEDED(hr))
        {
            hr = pps->SetValue(PKEY_AppUserModel_IsDestListSeparator, propvar);
            if (SUCCEEDED(hr))
            {
                hr = pps->Commit();
                if (SUCCEEDED(hr))
                {
                    hr = pps->QueryInterface(IID_PPV_ARGS(ppsl));
                }
            }
            PropVariantClear(&propvar);
        }
        pps->Release();
    }
    return hr;
}*/

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	g_hListMutex = CreateMutex( NULL, FALSE, _TEXT( "ListMutex-{83E9C941-A921-45fd-AE14-FA7716E18BC7}" ) );

	bool bAlreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

	if ( bAlreadyRunning )
	{
		MessageBox( 0, _TEXT( "An instance of this application is already running" ), _TEXT( "Error" ), MB_OK );

		ReleaseMutex( g_hListMutex );

		return 1;
	}

	if ( !CMainWindow::Create( hInstance, hPrevInstance, lpCmdLine ) )
		return 0;

	CMainWindow::CreateStatusBar();
	CMainWindow::CreateGroupBox();
	CMainWindow::CreateListBox();
	CMainWindow::CreateTabControl();
	CMainWindow::CreateListViews();

	/*CoInitialize( NULL );

	ICustomDestinationList *pcdl;
	HRESULT hr = CoCreateInstance(
		CLSID_DestinationList, 
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_PPV_ARGS(&pcdl));

	if (SUCCEEDED(hr))
	{
		pcdl->SetAppID( L"AppID-{83E9C941-A921-45fd-AE14-FA7716E18BC6}" );
		 UINT cMinSlots;
        IObjectArray *poaRemoved;
        hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
		if (SUCCEEDED(hr))
		{
			// Add content to the Jump List.
			{

				IObjectCollection *poc;
				HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
				if (SUCCEEDED(hr))
				{
					IShellLink * psl;
					hr = _CreateShellLink("/Tab-Favs", L"Favourites", &psl);
					if (SUCCEEDED(hr))
					{
						hr = poc->AddObject(psl);
						psl->Release();
					}
					hr = _CreateShellLink("/Tab-Internet", L"Internet", &psl);
					if (SUCCEEDED(hr))
					{
						hr = poc->AddObject(psl);
						psl->Release();
					}
					hr = _CreateShellLink("/Tab-Official", L"Official", &psl);
					if (SUCCEEDED(hr))
					{
						hr = poc->AddObject(psl);
						psl->Release();
					}
					hr = _CreateShellLink("/Tab-Lan", L"Official", &psl);
					if (SUCCEEDED(hr))
					{
						hr = poc->AddObject(psl);
						psl->Release();
					}

					if (SUCCEEDED(hr))
					{
						hr = _CreateSeparatorLink(&psl);
						if (SUCCEEDED(hr))
						{
							hr = poc->AddObject(psl);
							psl->Release();
						}
					}

					if (SUCCEEDED(hr))
					{
						IObjectArray * poa;
						hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
						if (SUCCEEDED(hr))
						{
							// Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
							// category that is displayed at the bottom of the Jump List, after all other
							// categories.
							hr = pcdl->AddUserTasks(poa);
							poa->Release();
						}
					}
					poc->Release();
				}

				hr = pcdl->CommitList();
			}
			poaRemoved->Release();
		}
		pcdl->Release();
	}*/
 
	SHGetSpecialFolderPath( 0, g_szAppDataDir, CSIDL_APPDATA, false );

	TCHAR szFile[ 384 ] = { NULL };
	_tcscpy( szFile, g_szAppDataDir );
	_tcscat( szFile, _TEXT( "/VCMP" ) );
	_tmkdir( szFile );


	CSettings::LoadSettings();

	CServerQuery::Init();

	CFavourites::Load();

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

			/*if ( ( *szIP ) && ( usPort ) ) 
			{
			// START THE GAME
			return false;
			}*/
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
				if ( !usPort ) usPort = 2301;

				if ( *szIP )
				{
					struct hostent     *he;
					if ( ( he = gethostbyname( szIP ) ) != NULL )
					{
						StartGame( szIP, usPort, 1 );
						goto quitApp;
					}
					else
						MessageBox( 0, _TEXT("Unable to resolve IP of server"), _TEXT("Error"), MB_ICONEXCLAMATION | MB_OK );
				}
			}
		}
	}
	
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
	CServerQuery::Stop();

#ifndef _NO_LAN_MODE
	CLANServerQuery::Stop();
#endif	

	CServerManager::RemoveAll();

	ReleaseMutex( g_hListMutex );
	CloseHandle( g_hListMutex );

	return (int)msg.wParam;
}