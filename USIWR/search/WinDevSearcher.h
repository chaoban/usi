/******************************************************************************//**
 *  @file WinDevSearcher.h
 *  @class WinDevSearcher
 *  @brief Windows device searcher.
 * 
 *  
 *  @author CY Fan
 *  @date 2016-02-02, 14:25:27
 **********************************************************************************/
#ifndef WINDEVSEARCHER_H_
#define WINDEVSEARCHER_H_

#include "SysDefine.h"
#include "UnderlyingDevKey.h"
#include <windows.h>
#include <wchar.h>

/* for hid device */
#include <guiddef.h>

extern "C" {
#include <SetupAPI.h>
#include <hidsdi.h>
#include <hidclass.h>
#include "hid.h"
}

namespace SiS
{
namespace Model
{

class WinDevSearcher
{
    public:
	    WinDevSearcher();
	    ~WinDevSearcher();
		
		DeviceSet listDevice(const AttributeValue protocol) const;

	private:
        WinDevSearcher(const WinDevSearcher & _WinDevSearcher);
        const WinDevSearcher & operator=(const WinDevSearcher & rhs);

		DeviceSet searchHID_USB() const;
		DeviceSet searchHID_USB_BRIDGE() const;
		DeviceSet searchHID_OVER_I2C() const;
		DeviceSet searchHID_OVER_SPI() const;

		DeviceSet searchHID() const;

        void assignDid(DeviceAttrTable & devTable) const;

		/**
		 *  @brief Get all hid deivce information we need.
		 *  @public
		 *  @param[in] hDevInfo The device information got from Win32 API.
		 *  @param[in] DevInfoData The device information got rom Win32 API.
		 *  @return A device attrubute table.
		 * 
		 */
		DeviceAttrTable grabHIDAttrbuties( HDEVINFO hDvcInfo, PSP_INTERFACE_DEVICE_DATA DvcInfoData ) const;

        /**
         *  @brief Get physical location paths by given device info structure.
         *  @private.
         *  @param[in] hDevInfo The device information got from Win32 API.
         *  @param[in] DevInfoData Device information structure from Win32 API.
         *  @param[in] backwradTime How many level do we look back for parent.
         *  @return Location paths string.
         */
        std::string getLocationPaths( HDEVINFO hDvcInfo, SP_DEVINFO_DATA devInfoData, size_t backwardTime = 1 ) const;

		/**
		 *  @brief Get physical location by given device info structure.
		 *  @private.
		 *  @param[in] hDevInfo The device information got from Win32 API.
		 *  @param[in] DevInfoData Device information structure from Win32 API.
		 *  @param[in] backwradTime How many level do we look back for parent.
		 *  @return Location string.
		 *
		 *  There are 2 possible format for retrun string:
		 *  1. Port_#0003.Hub_#0006.
		 *  2. 0000.001d.0000.001.004.004.003.003.000
		 *
		 *  The format (1) is a stand format for 817 HID class. And the format (2) is a stand format
		 *  for 819 HID class. In this case, we have to backwrod to it's parent class to get location that have 
		 *  format like (1).
		 * 
		 */
		std::string getLocation( HDEVINFO hDvcInfo, SP_DEVINFO_DATA devInfoData, size_t backwardTime = 1 ) const;

		/**
		 *  @brief Is the given device multi-interface or not.
		 *  @private
		 *  @param[in] hDevInfo The device information got from Win32 API.
		 *  @param[in] DevInfoData Device information structure from Win32 API.
		 *  @return Yes or not.
		 * 
		 */
		//bool isMultiInterface( HDEVINFO hDvcInfo, SP_DEVINFO_DATA devInfoData ) const;

		/**
		 *  @brief Get hub and port information from string.
		 *  @private
		 *  @param[in] strBuf Raw string from Win32 API.
		 *  @param[out] table A device table.
		 *  @return Parse success or not.
		 * 
		 */
		bool parseUsbHubString(const char * strBuf, DeviceAttrTable & table) const;

		/**
		 *  @brief Get SiS device hid descriptor.
		 *  @public
		 *  @param[in, out] table A device table with partial content.
		 *  @return Is sis device or not.
		 * 
		 *  We will
		 *  a) parse hid descriptor by table[DEV_PATH].
		 *  b) check that is it a sis device.
		 */
		bool parseSiSHIDDesc(DeviceAttrTable & table) const;

		std::string getComport(const unsigned short vid, const unsigned short pid) const;

		/**
		 *  @brief Basicly, it a wrap of Win32 API(SetupDiGetDeviceRegistryProperty).
		 *  @private
		 *  @return Success or not.
		 * 
		 */
		static BOOL getDeviceRegistryProperty(
				HDEVINFO devInfoSet, 
				PSP_DEVINFO_DATA devInfoData,
				DWORD property,
				DWORD & propertyRegDataType,
				BYTE * & propertyData);

		static BOOL getDeviceRegistryProperty(
				HDEVINFO devInfoSet, 
				PSP_DEVINFO_DATA devInfoData,
				DWORD property,
				BYTE * & propertyData);

		/**
		 *  @brief Get device type by birdge information.
		 *  @private
		 *  @param[in] pid PID geted from HW.
		 *  @param[in, out] table A device table with partial content.
		 *  @return Get successfully or not.
		 * 
		 */
		bool getDeviceTypeByBridgePID(const unsigned short pid, DeviceAttrTable & table) const;

		char * wcharToChar(wchar_t * src) const;

		void debugLog(const std::string msg) const;

};

}; // Model
}; // SiS

#endif
