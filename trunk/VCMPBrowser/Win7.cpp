#include "Win7.h"
#include "Servers.h"

#ifdef _WITH_JUMPLIST

const wchar_t* g_wszAppID = L"UnleashedGames.VCMPBrowser.Main.1";

ITaskbarList4 *CWin7::g_pTaskbarList = NULL;
ICustomDestinationList* CWin7::m_DestList = NULL;
IObjectArray* CWin7::m_List = NULL;
IObjectCollection* CWin7::m_CustomList = NULL;

typedef HRESULT (__stdcall *SETCURRENTPROCESSEXPLICITAPPUSERMODELIDPROC)(PCWSTR AppID);

// This function is to stop it all breaking on XP
void SetAppUserModelId( LPCWSTR sz )
{
	// try to load Shell32.dll
	HMODULE hmodShell32 = LoadLibrary("shell32.dll");
	if (hmodShell32 != NULL)
	{
		// see if the function is exposed by the current OS
		SETCURRENTPROCESSEXPLICITAPPUSERMODELIDPROC pfnSetCurrentProcessExplicitAppUserModelID =
			reinterpret_cast<SETCURRENTPROCESSEXPLICITAPPUSERMODELIDPROC>(GetProcAddress(hmodShell32,
			"SetCurrentProcessExplicitAppUserModelID"));
		if (pfnSetCurrentProcessExplicitAppUserModelID != NULL)
		{
			pfnSetCurrentProcessExplicitAppUserModelID( sz );
		}

		FreeLibrary(hmodShell32);
	}
}

typedef HRESULT (__stdcall *SHGetPropertyStoreForWindow_Type)( HWND hwnd, REFIID riid, void** ppv );

HRESULT SHGetPropertyStoreForWindow2( HWND hwnd, REFIID riid, void** ppv )
{
	// try to load Shell32.dll
	HRESULT hr = S_FALSE;
	HMODULE hmodShell32 = LoadLibrary("shell32.dll");
	if (hmodShell32 != NULL)
	{
		// see if the function is exposed by the current OS
		SHGetPropertyStoreForWindow_Type pfnSHGetPropertyStoreForWindow =
			reinterpret_cast<SHGetPropertyStoreForWindow_Type>(GetProcAddress(hmodShell32,
			"SHGetPropertyStoreForWindow"));
		if (pfnSHGetPropertyStoreForWindow != NULL)
		{
			hr = pfnSHGetPropertyStoreForWindow( hwnd, riid, ppv );
		}

		FreeLibrary(hmodShell32);
	}
	return hr;
}

HRESULT _CreateShellLink(PCSTR pszArguments, PCWSTR pszTitle, LPCSTR szDescription, IShellLink **ppsl)
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
					hr = psl->SetDescription( szDescription );
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
}

void CWin7::Init()
{
	SetAppUserModelId( g_wszAppID );

	CoCreateInstance( CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pTaskbarList));

	HRESULT hr = CoCreateInstance( CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_DestList));
	if ( SUCCEEDED( hr ) )
		m_DestList->SetAppID( g_wszAppID );
	else
		m_DestList = 0;
}

void CWin7::Shutdown()
{
	if ( m_DestList )
	{
		m_DestList->Release();
		m_DestList = 0;
	}
}

void CWin7::SetAppID(HWND hWnd, bool bDestory )
{
    IPropertyStore *pps;
    HRESULT hr = SHGetPropertyStoreForWindow2(hWnd, IID_PPV_ARGS(&pps));
    if (SUCCEEDED(hr))
    {
        PROPVARIANT pv;
		if ( !bDestory )
			hr = InitPropVariantFromString(g_wszAppID, &pv);
		else
			PropVariantInit(&pv);

        if (SUCCEEDED(hr))
        {
            hr = pps->SetValue(PKEY_AppUserModel_ID, pv);
            PropVariantClear(&pv);
        }
        pps->Release();
    }
}

void CWin7::BeginList()
{
	if ( m_DestList )
	{
		UINT cMinSlots = 0;
        HRESULT hr = m_DestList->BeginList(&cMinSlots, IID_PPV_ARGS(&m_List));
		if ( !SUCCEEDED( hr ) )
			m_List = 0;
	}
}

void CWin7::EndList()
{
	if ( m_DestList )
		m_DestList->CommitList();

	if ( m_CustomList )
	{
		m_CustomList->Release();
		m_CustomList = 0;
	}
	if ( m_List )
	{
		m_List->Release();
		m_List = 0;
	}
}

void CWin7::CreateTasksList()
{
	if ( m_DestList )
	{
		if ( m_List )
		{
			IObjectCollection *poc;
			HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
			if (SUCCEEDED(hr))
			{
				IShellLink * psl;

				hr = _CreateShellLink("/Tab-Favs", L"Favourites", "Open Favourites Tab", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}
				hr = _CreateShellLink("/Tab-Internet", L"Internet", "Open Internet Tab", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}
				hr = _CreateShellLink("/Tab-Official", L"Official", "Open Official Tab", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}
#ifndef _NO_HISTORY
				hr = _CreateShellLink("/Tab-History", L"History", "Open History Tab", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}
#endif
#ifndef _NO_LAN_MODE
				hr = _CreateShellLink("/Tab-Lan", L"LAN", "Open LAN Tab", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}
#endif

				if (SUCCEEDED(hr))
				{
					hr = _CreateSeparatorLink(&psl);
					if (SUCCEEDED(hr))
					{
						hr = poc->AddObject(psl);
						psl->Release();
					}
				}

				hr = _CreateShellLink("/Win-Settings", L"Settings", "Open the Settings dialog", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
				}

				hr = _CreateShellLink("/Win-Help", L"Help", "Get Help & Support and the forums", &psl);
				if (SUCCEEDED(hr))
				{
					hr = poc->AddObject(psl);
					psl->Release();
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
						hr = m_DestList->AddUserTasks(poa);
						poa->Release();
					}
				}

				poc->Release();
			}
		}
	}
}

void CWin7::CreateCustomList()
{
#ifndef _NO_HISTORY
	if ( m_DestList )
	{
		if ( m_List )
		{
			HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&m_CustomList));
			if (SUCCEEDED(hr))
			{
				IShellLink * psl;

				unsigned int uiMax = CMainWindow::m_uiHistoryServers;
				if ( uiMax > 5 )
					uiMax = 5;

				unsigned int ui = 0, ui1 = 0;

				CHistoryServer* pHist = 0;
				CServers* pServer = 0;
				char szBuf[ 128 ] = { 0 };
				wchar_t wszBuf[ 128 ] = { 0 };
				while ( ( ui < MAX_SERVERS ) && ( ui1 < uiMax ) )
				{
					pHist = CMainWindow::m_lHistoryServers[ ui ];
					if ( pHist )
					{
						pServer = pHist->GetServer();
						if ( pServer )
						{
							sprintf( szBuf, "lu://connect/%s:%u", pServer->GetServerIP(), pServer->GetServerPort() );
							MultiByteToWideChar( CP_ACP, 0, pServer->GetServerName(), -1, wszBuf, strlen( pServer->GetServerName() )+1 );
							hr = _CreateShellLink( szBuf, wszBuf, "", &psl);
							if (SUCCEEDED(hr))
							{
								hr = m_CustomList->AddObject(psl);
								psl->Release();
							}
							++ui1;
						}
					}
					++ui;
				}
				

				if (SUCCEEDED(hr))
				{
					IObjectArray * poa;
					hr = m_CustomList->QueryInterface(IID_PPV_ARGS(&poa));
					if (SUCCEEDED(hr))
					{
						// Add the tasks to the Jump List. Tasks always appear in the canonical "Tasks"
						// category that is displayed at the bottom of the Jump List, after all other
						// categories.
						hr = m_DestList->AppendCategory( L"Recent Servers", poa);
						poa->Release();
					}
				}				
			}
			else
				m_CustomList = 0;
		}
	}
#endif
}

#endif
