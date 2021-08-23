/******************************************************************************//**
 *  @file WinVerHelper.h
 *  @class WinVerHelper
 *  @brief Windows version helper.
 * 
 *  Maybe remove this file in the feature.
 *  
 *  @author CY Fan
 *  @date 2016-08-30, 14:52:52
 **********************************************************************************/
#ifndef WINVERHELPER_H_
#define WINVERHELPER_H_

namespace SiS
{
namespace Model
{

class WinVerHelper
{
    public:
	    ~WinVerHelper();

		static bool isVistaOrGreater();
	private:
	    WinVerHelper();
        WinVerHelper(const WinVerHelper & _WinVerHelper);
        const WinVerHelper & operator=(const WinVerHelper & rhs);
};

}; // Model
}; // SiS

#endif
