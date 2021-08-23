/******************************************************************************//**
 *  @file AUString.h
 *  @brief Define AUChar that represents char/wchar_t. AUString that represents 
 *  std::string/std::wstring.
 * 
 *  
 *  @author CY Fan
 *  @date 2016-06-14, 09:38:28
 **********************************************************************************/
#ifndef AUSTRING_H_
#define AUSTRING_H_

#include <string>

#include <sstream>

namespace SiS
{

#if defined(UNICODE) || defined(_UNICODE)
	/*
	typedef AUChar wchar_t;
	typedef AUString std::wstring;
	typedef AUStringSteam std::wstringstream
	*/
	typedef wchar_t AUChar ;
	typedef std::wstring AUString;
	typedef std::wstringstream AUStringSteam;
#define AUTEXT(t) L#t
#else
	/*
	typedef AUChar char;
	typedef AUString std::string;
	typedef AUStringSteam std::stringstream
	*/
	typedef char AUChar;
	typedef std::string AUString;
	typedef std::stringstream AUStringSteam;
#define AUTEXT(t) #t
#endif

}; // SiS

#endif
