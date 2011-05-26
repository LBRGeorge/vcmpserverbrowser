#include "PasswordDlg.h"
#include <Windowsx.h>
#include "Servers.h"

extern void StartGame( const char* szIP, unsigned short usPort, unsigned char ucGame, const char* szPass = 0 );

char CPasswordDlg::m_szIP[ 16 ] = { 0 };
unsigned short CPasswordDlg::m_usPort = 0;
char CPasswordDlg::m_szPass[ 64 ] = { 0 };
unsigned char CPasswordDlg::m_ucGame = 0;

INT_PTR CALLBACK PasswordDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	case WM_INITDIALOG:
		SetDlgItemText( hwnd, IDC_EDIT_PASS, CPasswordDlg::m_szPass );

		if ( *CPasswordDlg::m_szPass )
			Button_SetCheck( GetDlgItem(hwnd, IDC_CHECK1), TRUE );

		if (GetDlgCtrlID((HWND) wParam) != IDC_EDIT_PASS) 
		{ 
			SetFocus(GetDlgItem(hwnd, IDC_EDIT_PASS)); 
			return FALSE; 
		}
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				TCHAR szBuffer[ 64 ] = { 0 };

				// Lets update the internal Name stuff
				int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT_PASS));
				if(len > 0)
				{
					GetDlgItemText(hwnd, IDC_EDIT_PASS, szBuffer, MAX_PATH);

					
					CServers* pServer = CServerManager::Find( CPasswordDlg::m_szIP, CPasswordDlg::m_usPort );
					if ( pServer )
					{
						if ( Button_GetCheck( GetDlgItem(hwnd, IDC_CHECK1) ) )
							pServer->SetServerPass( szBuffer );
						else
							pServer->SetServerPass( "" );
					}

					StartGame( CPasswordDlg::m_szIP, CPasswordDlg::m_usPort, CPasswordDlg::m_ucGame, szBuffer );
				}

				// Finally, close the dialog
				EndDialog(hwnd, IDOK);
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

void CPasswordDlg::Show( HWND hwnd )
{
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DLG_PASS), hwnd, PasswordDlgProc);
}