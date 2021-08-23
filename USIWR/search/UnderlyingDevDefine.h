/******************************************************************************//**
 *  @file UnderlyingDevDefine.h
 *  @class UnderlyingDevDefine
 *  @brief Underlying device general OS define.
 * 
 *  
 *  @author CY Fan
 *  @date 2016-01-11, 10:02:27
 **********************************************************************************/
#ifndef UNDERLYINGDEVDEFINE_H_
#define UNDERLYINGDEVDEFINE_H_

#include "UnderlyingDevKey.h"
#include "SysDefine.h"

#include "string"
#ifdef WIN32
#include <windows.h>
#endif

namespace SiS
{
#ifdef WIN32
	typedef DWORD SYS_ERR;
#else
	typedef int SYS_ERR;
#endif

typedef enum _IOStatus{
	IO_ST_INVALID,
	OPEN_FAIL,
	SYS_FATAL_ERR,
	DEVICE_BUSY,
	IO_DENY,
	IO_SESSION_FAIL,
	IO_SESSION_SUCCESS,
	IO_ST_END,
}IOStatus;

typedef enum _IOWRType
{
	WR_INVALID = 0,
	WRITE,
	READ,
	WR_END,
}WRType;

}; // SiS

#endif
