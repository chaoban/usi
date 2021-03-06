/******************************************************************************//**
 *  @file SiSInt.h
 *  @class SiSInt
 *  @brief Handle the cstdint issue.
 * 
 *  
 *  @author CY Fan
 *  @date 2017-05-18, 09:19:48
 **********************************************************************************/
#ifndef SISINT_H_
#define SISINT_H_

#ifdef __linux__
	#include <cstdint>
#else
	#include <Windows.h>
	#if _MSC_VER < 1600
		typedef __int8 int8_t;
		typedef __int16 int16_t;
		typedef __int32 int32_t;
		typedef __int64 int64_t;
		typedef unsigned __int8 uint8_t;
		typedef unsigned __int16 uint16_t;
		typedef unsigned __int32 uint32_t;
		typedef unsigned __int64 uint64_t;
	#else
		#include <cstdint>
	#endif
#endif


#endif
