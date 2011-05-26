#include "SettingsDlg.h"
#include <Shlobj.h>
#include <Windowsx.h>

#define getA(c)		(((c)&0xff000000)>>24)
#define getR(c)		(((c)&0x00ff0000)>>16)
#define getG(c)		(((c)&0x0000ff00)>>8)
#define getB(c)		((c)&0x000000ff)

static int CALLBACK BrowseCallbackVCProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	// If the BFFM_INITIALIZED message is received
	// set the path to the start path.
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		{
			if (NULL != lpData)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}
		}
		break;

	case BFFM_SELCHANGED:
		{
			TCHAR szText[MAX_PATH] = {0};
            SHGetPathFromIDList(
                reinterpret_cast<LPITEMIDLIST>(lParam), szText);

			_tcscat( szText, _TEXT( "\\gta-vc.exe" ) );
			if ( GetFileAttributes(szText) != INVALID_FILE_ATTRIBUTES )
				SendMessage( hwnd, BFFM_ENABLEOK, 0, TRUE );
			else
				SendMessage( hwnd, BFFM_ENABLEOK, 0, FALSE );
			
		}
		break;
	}

	return 0; // The function should always return 0.
}

INT_PTR CALLBACK SettingsDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:
		SetDlgItemText( hwnd, IDC_EDIT_NAME, CSettings::GetName() );
		SetDlgItemText( hwnd, IDC_EDIT_GTAVC, CSettings::GetGTAVCPath() );

		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				TCHAR szBuffer[ MAX_PATH ] = { 0 };

				// Lets update the internal Name stuff
				int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT_NAME));
				if(len > 0)
				{
					GetDlgItemText(hwnd, IDC_EDIT_NAME, szBuffer, MAX_PATH);

					CSettings::SetName( szBuffer );
				}

				// ... and the GTAVC path
				len = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT_GTAVC));
				if(len > 0)
				{
					GetDlgItemText(hwnd, IDC_EDIT_GTAVC, szBuffer, MAX_PATH);

					TCHAR szTmp[ MAX_PATH ] = { 0 };
					str_cpy( szTmp, szBuffer, sizeof( szTmp ) );
					if ( GetFileAttributes( szTmp ) == INVALID_FILE_ATTRIBUTES )
						if ( MessageBox( 0, _TEXT( "You have entered an invalid GTA:VC location. Would you like to continue anyway?" ), _TEXT( "Warning" ), MB_YESNO ) == IDNO )
							break;

					CSettings::SetGTAVCPath( szBuffer );
				}

				CSettings::WriteSettings();

				// Finally, close the dialog
				EndDialog(hwnd, IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDC_BTN_GTAVC_BROWSE:
			{
				OPENFILENAME ofn;

				ZeroMemory(&ofn, sizeof(ofn));

				TCHAR szFileName[MAX_PATH] = _TEXT( "" );

				int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT_GTAVC));
				if(len > 0)
				{
					GetDlgItemText(hwnd, IDC_EDIT_GTAVC, szFileName, MAX_PATH);
				}

				BROWSEINFO bi  = { 0 };
				bi.lpszTitle   = _T("Select your GTA:VC Directory");
				bi.hwndOwner = hwnd;
				bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_VALIDATE;
				bi.lpfn           = BrowseCallbackVCProc;
				bi.lParam         = (LPARAM) szFileName;
		

				LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
				if ( pidl)
				{
					SHGetPathFromIDList( pidl, szFileName );
					TCHAR szTmp[ MAX_PATH ] = { 0 };
					str_cpy( szTmp, szFileName, sizeof( szTmp ) );
					_tcscat( szTmp, _TEXT( "\\gta-vc.exe" ) );

					SetDlgItemText(hwnd, IDC_EDIT_GTAVC, szTmp );

					CoTaskMemFree(pidl);
				}
			}
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void CSettingsDlg::Show( HWND hwnd )
{
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG_SETTINGS), hwnd, SettingsDlgProc);
}