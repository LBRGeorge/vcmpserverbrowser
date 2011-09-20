#pragma once

#include "MainWindow.h"

#ifdef _WITH_JUMPLIST

#include <Shobjidl.h>
#include <propkey.h>
#include <Propvarutil.h>

class CWin7
{
public:
	static void Init( void );
	static void Shutdown( void );

	static void SetAppID(HWND hWnd, bool bDestroy = false );

	static void	BeginList( void );
	static void EndList( void );

	static void CreateTasksList( void );
	static void	CreateCustomList( void );

	static ITaskbarList4* g_pTaskbarList;

private:
	static ICustomDestinationList* m_DestList;
	static IObjectArray* m_List;
	static IObjectCollection* m_CustomList;
};

#endif
