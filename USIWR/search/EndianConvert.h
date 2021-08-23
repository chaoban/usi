/******************************************************************************//**
 *  @file EndianConvert.h
 *  @class EndianConvert
 *  @brief Endian convert.
 * 
 *  The "endiaReverse" series is copied from boost.
 *  
 *  @author CY Fan
 *  @date 2017-05-15, 09:33:05
 **********************************************************************************/
#ifndef ENDIANCONVERT_H_
#define ENDIANCONVERT_H_

#ifdef __linux__
#include <string.h>
#endif

//#include <cstdint>
#include "SiSInt.h"

namespace SiS
{
namespace Model
{

class EndianConvert
{
    public:
	    EndianConvert()
		{}
	    ~EndianConvert()
		{}

		static int8_t endianReverse(int8_t x) 
		{
			return x;
		}

		static int16_t endianReverse(int16_t x) 
		{
			return (static_cast<uint16_t>(x) << 8)
				| (static_cast<uint16_t>(x) >> 8);
		}

		static int32_t endianReverse(int32_t x) 
		{
			uint32_t step16;
			step16 = static_cast<uint32_t>(x) << 16 | static_cast<uint32_t>(x) >> 16;
			return
				((static_cast<uint32_t>(step16) << 8) & 0xff00ff00)
				| ((static_cast<uint32_t>(step16) >> 8) & 0x00ff00ff);
		}

		static int64_t endianReverse(int64_t x) 
		{
			uint64_t step32, step16;
			step32 = static_cast<uint64_t>(x) << 32 | static_cast<uint64_t>(x) >> 32;
			step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16
				| (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
			return static_cast<int64_t>((step16 & 0x00FF00FF00FF00FFULL) << 8
					| (step16 & 0xFF00FF00FF00FF00ULL) >> 8);
		}
  
		static uint8_t endianReverse(uint8_t x) 
		{
			return x;
		}

		static uint16_t endianReverse(uint16_t x) 
		{
			return (x << 8)
				| (x >> 8);
		}

		static uint32_t endianReverse(uint32_t x)                            
		{
			uint32_t step16;
			step16 = x << 16 | x >> 16;
			return
				((step16 << 8) & 0xff00ff00)
				| ((step16 >> 8) & 0x00ff00ff);
		}

		static uint64_t endianReverse(uint64_t x) 
		{
			uint64_t step32, step16;
			step32 = x << 32 | x >> 32;
			step16 = (step32 & 0x0000FFFF0000FFFFULL) << 16
				| (step32 & 0xFFFF0000FFFF0000ULL) >> 16;
			return (step16 & 0x00FF00FF00FF00FFULL) << 8
				| (step16 & 0xFF00FF00FF00FF00ULL) >> 8;
		}

		template<class T, class BYTE>
		static BYTE * littleEndianToBytes(const T & source)
		{
			size_t len = sizeof(T) / sizeof(BYTE);
			BYTE * bytes = new BYTE[len];
			memset(bytes, 0, len);
#if defined(_WIN32) || defined(WIN32) || defined(__linux__)
			memcpy(bytes, (void*)(&source), len);
#else
			return 0;
#endif
			return bytes;
		}

		template<class T, class BYTE>
		//static T bytesToBigEndian(const BYTE * source, const int len)
		static T bytesToLittleEndian(const BYTE * source, const int len)
		{
			long int ret = 0;
			long int mask = 0x00ff;
			for ( int idx = 0; idx < len; idx++ )
			{
				int data = source[idx];
				ret = ret | ( ( data << (idx * 8) ) & mask );
				mask = mask << 8;
			}

			return static_cast<T>(ret);
		}

		template<class T, class BYTE>
		static BYTE * bigEndianToBytes(const T & source)
		{
			size_t len = sizeof(T) / sizeof(BYTE);
			BYTE * bytes = new BYTE[len];
			memset(bytes, 0, len);
#if defined(_WIN32) || defined(WIN32) || defined(__linux__)
			for ( int i = (len - 1); i >= 0; i-- )
			{
				bytes[i] = (BYTE)( source >> ( (len - 1 - i) * 8) );
			}
#else
			return 0;
#endif
			return bytes;
		}

		template<class T, class BYTE>
		static T bytesToBigEndian(const BYTE * source, const int len)
		{
			long int ret = 0;
			long int mask = 0x00ff;
			for ( int idx = (len - 1); idx >= 0; idx-- )
			{
				int data = source[idx];
				ret = ret | ( ( data << ((len - 1 - idx) * 8) ) & mask );
				mask = mask << 8;
			}

			return static_cast<T>(ret);
		}


	private:
        EndianConvert(const EndianConvert & _EndianConvert);
        const EndianConvert & operator=(const EndianConvert & rhs);
};

}; // Model
}; // SiS

#endif
