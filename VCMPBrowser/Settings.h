#pragma once

#include "MainWindow.h"

class CSettings
{
public:
	static void							SetDefaults				( void );

	static void							LoadSettings			( void );
	static void							WriteSettings			( void );

	static TCHAR*						GetName					( void ) 						{ return m_szName; }
	static void							SetName					( const TCHAR* sz )				{ str_cpy( m_szName, sz, sizeof( m_szName ) ); }

	static TCHAR*						GetGTAVCPath			( void ) 						{ return m_szGTAVCPath; }
	static void							SetGTAVCPath			( const TCHAR* sz )				{ str_cpy( m_szGTAVCPath, sz, sizeof( m_szGTAVCPath ) ); }

private:
	static TCHAR						m_szName[ 32 ];

	static TCHAR						m_szGTAVCPath[ MAX_PATH ];

};
