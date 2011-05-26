#pragma once

#include "Servers.h"

class CServerQuery
{
public:
	static void						Init					( void );
	static void						Start					( void );
	static void						Stop					( void );

	static clock_t					Query					( const char* szIP, unsigned short usPort );
	static clock_t					QueryWithPlayers		( const char* szIP, unsigned short usPort );

	static void						UpdateInternetList		( void );
	static void						UpdateOfficialList		( void );

	static SOCKET					m_pSocket;

private:
	static HANDLE					m_hThread;
};
