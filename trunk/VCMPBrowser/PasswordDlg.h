#pragma once

#include "Settings.h"

class CPasswordDlg
{
public:
	static void						Show					( HWND hwnd );

	static void						SetServer				( const char* szIP, const unsigned short usPort, const char* szPass, unsigned char ucGame )
	{
		str_cpyA( m_szIP, szIP, sizeof( m_szIP ) );
		m_usPort = usPort;
		str_cpyA( m_szPass, szPass, sizeof( m_szPass ) );
		m_ucGame = ucGame;
	}

	static char						m_szIP[ 16 ];
	static unsigned short			m_usPort;
	static char						m_szPass[ 64 ];
	static unsigned char			m_ucGame;
};
