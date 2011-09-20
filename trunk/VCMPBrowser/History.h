#pragma once

#include "MainWindow.h"

#ifndef _NO_HISTORY

class CHistory
{
public:
	static void							Load					( void );
	static void							Write					( void );

private:
	static void							ParseData				( unsigned char* szBuf );
	static void							ParsePasswordData		( unsigned char* szBuf );
};

#endif
