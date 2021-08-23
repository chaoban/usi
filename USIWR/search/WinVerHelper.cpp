/**********************************************************************************
 * Implement WinVerHelper.h
 *********************************************************************************/

#include "WinVerHelper.h"
#include <windows.h>

#define XP_MAJOR 5

using namespace SiS::Model;

WinVerHelper::WinVerHelper()
{}

WinVerHelper::~WinVerHelper()
{}

bool WinVerHelper::isVistaOrGreater()
{
	OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

	bool bIsVistaOrGreater = osvi.dwMajorVersion > XP_MAJOR ? true : false;

	return bIsVistaOrGreater;
}
