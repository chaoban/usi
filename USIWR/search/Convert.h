/******************************************************************************//**
 *  @file Convert.h
 *  @class Convert
 *  @brief Convert type.
 * 
 *  
 *  @author CY Fan
 *  @date 2016-04-22, 14:01:55
 **********************************************************************************/
#ifndef CONVERT_H_
#define CONVERT_H_

#include <sstream>
#include <string>
#include <locale>
#include <algorithm>

#include "EndianConvert.h"

namespace SiS
{
namespace Model
{

#define MAX_BUILD_TYPE_TO_STRING_LENGTH 20

class Convert
{
    public:
	    Convert();
	    ~Convert();

		static int toInt( const std::string & s )
		{
			return toValue<int>(s);
		}

		template<class T>
		static std::string toString( const T& t )
		{
			std::ostringstream stream;
			stream << t;

			return stream.str();
		}

		static std::string upperFrom( const std::string & s )
		{
			std::string upperStr = s;
			std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
			return upperStr;
		}

		static std::string lowerFrom( const std::string & s )
		{
			std::string upperStr = s;
			std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::tolower);
			return upperStr;
		}

		template<class T>
		static std::string toHexString( const T& t )
		{
			std::ostringstream stream;
			stream << std::hex << t;

			return stream.str();
		}

		static std::string fillZeroTo4Digit(const std::string & s)
		{
			std::ostringstream stream;
			stream.width(4);
			stream.fill('0');
			stream << std::right << s;

			return stream.str();
		}

		template<class T>
		static std::string toHexStringWithPrefix( const T& t )
		{
			return std::string("0x") + toHexString(t);
		}

		template<class T>
		static T toValue(const std::string & s)
		{
			std::istringstream stream (s);
			T t;
			stream >> t;
			return t;
		}

		static std::wstring stringToWString(const std::string & s)
		{
			std::wstring ws;
			for ( std::string::const_iterator sIt = s.begin(); sIt != s.end(); sIt++ )
			{
			    ws += (*sIt);
			}

			return ws;
		}

		/*
		   Convert(const Convert & _Convert);
		   const Convert & operator=(const Convert & rhs);
		   */
	private:
};

}; // Model
}; // SiS

#endif
