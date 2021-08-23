/**********************************************************************************
 * Implement WinDevSearcher.h
 *********************************************************************************/

#include "WinDevSearcher.h"
#include "UnderlyingDevDefine.h"

#include <cfgmgr32.h>
#include <devpkey.h>
#include <devpropdef.h>

/* for wchar_t */
#include <stdlib.h>
#include <algorithm>
#include <limits.h>

#include "Convert.h"
#include "WinVerHelper.h"
#include "USBDevNode.h"

#define TEST_819
#ifdef TEST_819
#include "AUString.h"
#endif

/* HID GUID */
static const GUID GUID_DEVINTERFACE_HIDCLASS = 
{ 0x4D1E55B2L, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } };

#if !defined(GUID_DEVINTERFACE_USB_DEVICE)
	static const GUID GUID_DEVINTERFACE_USB_DEVICE = { 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
#endif


#define MAXPATH 1024
#define MAX_REG_VALUE_LEN 4096
#define REG_PORT_NAME AUTEXT(PortName)
#define REG_PORT_NAME_PREFIX "COM"

using namespace SiS::Model;
using namespace SiS;
//using namespace cloudbox::data;

WinDevSearcher::WinDevSearcher()
{}

WinDevSearcher::~WinDevSearcher()
{}

DeviceSet WinDevSearcher::listDevice(const AttributeValue protocol) const
{
	DeviceSet set;

	if ( HID_USB == protocol )
	{
		set = this->searchHID_USB();
	}
	else if ( HID_USB_BRIDGE == protocol )
	{
		set = this->searchHID_USB_BRIDGE();
	}
	else if ( HID_OVER_I2C == protocol )
	{
		set = this->searchHID_OVER_I2C();
	}
	else if ( HID_OVER_SPI == protocol )
	{
		set = this->searchHID_OVER_SPI();
	}
	else if ( true == protocol.empty() )
	{
		// scan all
		DeviceSet _setHID_USB = this->searchHID_USB();
		set.insert(set.begin(), _setHID_USB.begin(), _setHID_USB.end());

		DeviceSet _setHID_USB_BRIDGE = this->searchHID_USB_BRIDGE();
		if ( false == _setHID_USB_BRIDGE.empty() )
		{
			for ( DeviceSet::iterator bridge = _setHID_USB_BRIDGE.begin(); bridge != _setHID_USB_BRIDGE.end(); bridge++ )
			{
			    set.push_back((*bridge));
			}
		}

		DeviceSet _setHID_OVER_I2C = this->searchHID_OVER_I2C();
		if ( false == _setHID_OVER_I2C.empty() )
		{
			for ( DeviceSet::iterator i2c = _setHID_OVER_I2C.begin(); i2c != _setHID_OVER_I2C.end(); i2c++ )
			{
			    set.push_back((*i2c));
			}
		}

		DeviceSet _setHID_OVER_SPI = this->searchHID_OVER_SPI();
		if ( false == _setHID_OVER_SPI.empty() )
		{
			for ( DeviceSet::iterator spiDev = _setHID_OVER_SPI.begin(); spiDev != _setHID_OVER_SPI.end(); spiDev++ )
			{
			    set.push_back((*spiDev));
			}
		}
	}
	else
	{
		this->debugLog("using a invalid protocol " + protocol);
		// do nothing
	}

	return set;
}

DeviceSet WinDevSearcher::searchHID_USB() const
{
	DeviceSet _set = this->searchHID();
	DeviceSet set;
	for ( DeviceSet::iterator devIt = _set.begin(); devIt != _set.end(); devIt++ )
	{
	    if ( HID_USB == (*devIt)[PROTOCOL] )
	    //if ( I2C == (*devIt)[PROTOCOL] )
		{
			set.push_back((*devIt));
		}
	}
	return set;
}

DeviceSet WinDevSearcher::searchHID_USB_BRIDGE() const
{
	DeviceSet _set = this->searchHID();
	DeviceSet set;
	for ( DeviceSet::iterator devIt = _set.begin(); devIt != _set.end(); devIt++ )
	{
	    if ( HID_USB_BRIDGE == (*devIt)[PROTOCOL] )
		{
			set.push_back((*devIt));
		}
	}
	return set;
}

DeviceSet WinDevSearcher::searchHID_OVER_I2C() const
{
	DeviceSet _set = this->searchHID();
	DeviceSet set;
	for ( DeviceSet::iterator devIt = _set.begin(); devIt != _set.end(); devIt++ )
	{
	    if ( HID_OVER_I2C == (*devIt)[PROTOCOL] )
		{
			set.push_back((*devIt));
		}
	}
	return set;
}

DeviceSet WinDevSearcher::searchHID_OVER_SPI() const
{
	DeviceSet _set = this->searchHID();
	DeviceSet set;
	for ( DeviceSet::iterator devIt = _set.begin(); devIt != _set.end(); devIt++ )
	{
	    if ( HID_OVER_SPI == (*devIt)[PROTOCOL] )
		{
			set.push_back((*devIt));
		}
	}
	return set;
}

DeviceSet WinDevSearcher::searchHID() const
{
	//searchCOM();
	DeviceSet set;

    /* 
     * list all hid device on this host
     */
	const GUID * hidGuid = &GUID_DEVINTERFACE_HIDCLASS;
	HDEVINFO hDevInfo = SetupDiGetClassDevs( 
			hidGuid,
			NULL, 
			NULL, 
			DIGCF_PRESENT|DIGCF_INTERFACEDEVICE 
			);

	SP_INTERFACE_DEVICE_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof ( SP_INTERFACE_DEVICE_DATA );

	ULONG nGuessCount = MAXLONG;
	for( ULONG iDevIndex = 0; iDevIndex < nGuessCount; iDevIndex++ )
	{
		if( SetupDiEnumDeviceInterfaces( hDevInfo, 0, hidGuid, iDevIndex, &deviceInfoData ) )// fill deviceInfoData by hDevInfo
		{
			DeviceAttrTable devTable = this->grabHIDAttrbuties(hDevInfo, &deviceInfoData);

			/* for test Linux I2C command */
			devTable[OS] = WindowsName;
			//devTable[OS] = LinuxName;

			if ( false == devTable.empty() )
			{
				this->debugLog("find device " + devTable[DEV_PATH]);
			}
			if ( true == this->parseSiSHIDDesc(devTable) )
			{
				/* copy protocol to interface for convenient */
				std::string pro = devTable[PROTOCOL];
				devTable[INTERFACE_NAME] = pro;

                /* assign device id */
                assignDid(devTable);

				set.push_back(devTable);
			}

		}
		else if( GetLastError() == ERROR_NO_MORE_ITEMS ) //No more items
		{
			break;
		}
	}
	SetupDiDestroyDeviceInfoList( hDevInfo );

	return set;
}

void WinDevSearcher::assignDid(DeviceAttrTable & devTable) const
{
    std::string location = devTable[PHYS_LOCATION];
    std::string locationPaths = devTable[LOCATION_PATHS];

    if( !location.empty() )
    {
        devTable[D_ID] = location;
    }
    else if( !locationPaths.empty() )
    {
        devTable[D_ID] = locationPaths;
    }
    else
    {
        devTable[D_ID] = devTable[INTERFACE_NAME];
    }
}

DeviceAttrTable WinDevSearcher::grabHIDAttrbuties( HDEVINFO hDvcInfo, PSP_INTERFACE_DEVICE_DATA DvcInfoData ) const
{
	DeviceAttrTable retTable;

	/* prepare some stuff for Win32 API */
    ULONG iReqLen = 0;
    SetupDiGetInterfaceDeviceDetail(hDvcInfo, DvcInfoData, NULL, 0, &iReqLen, NULL );

    ULONG iDevDataLen = iReqLen; //sizeof(SP_FNCLASS_DEVICE_DATA) + 512;
    PSP_INTERFACE_DEVICE_DETAIL_DATA pDevData = ( PSP_INTERFACE_DEVICE_DETAIL_DATA )malloc( iDevDataLen );

    SP_DEVINFO_DATA did;
    did.cbSize = sizeof( SP_DEVINFO_DATA );

    pDevData->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    if( SetupDiGetInterfaceDeviceDetail( hDvcInfo, DvcInfoData, pDevData, iDevDataLen, &iReqLen, &did ) )// fill pDevData
    {
		if ( iReqLen > MAXPATH )
		{
            //printf( "warning: device request path > MAXPATH, please contact programmer\n" );
            this->debugLog("warning: device request path > MAXPATH, please contact programmer");
		}

		/* device path */
		TCHAR _sDevNameBuf[MAXPATH];
		memset(_sDevNameBuf, 0, MAXPATH);

#ifdef UNICODE
        wcscpy_s( _sDevNameBuf, MAXPATH, pDevData->DevicePath );
#else
        strcpy_s( _sDevNameBuf, MAXPATH, pDevData->DevicePath );
#endif
		//devicePath = _sDevNameBuf;
		//TODO: convert wchar_t to char
	
#ifdef UNICODE
		char * devPathChar = this->wcharToChar(_sDevNameBuf);
		if ( 0 == devPathChar )
		{
			retTable.clear();
			this->debugLog("can't convert wchar_t device path to char\n");
			return retTable;
		}
		retTable[DEV_PATH] = std::string(devPathChar);
		delete[] devPathChar;
#else
		retTable[DEV_PATH] = std::string((char*)_sDevNameBuf);
#endif
        /* get Location paths */
        std::string locationPaths = getLocationPaths(hDvcInfo, did);
        retTable[LOCATION_PATHS] = locationPaths;

        /* get Location information */
		std::string location = getLocation(hDvcInfo, did);

		/* get port and hub */
		if ( true == location.empty() )
		{
            retTable[PHYS_LOCATION] = location;
			retTable[HUB_ID] = INVALID_HUB_ID;
			retTable[PORT_ID] = INVALID_PORT_ID;
			return retTable;
		}
		else
		{
			//retTable[PHYS_LOCATION] = std::string(location);
			if ( true == parseUsbHubString(location.c_str(), retTable) )
			{
				retTable[PHYS_LOCATION] = location;
				free( pDevData );
				return retTable;
			}

			/*
			 * The format of location is :
			 * 0000.001d.0000.001.004.004.003.003.000
			 */
			
			/* We guess this is a composite device, so we get the location again */
			location = getLocation(hDvcInfo, did, 2);
			
			if ( true == parseUsbHubString(location.c_str(), retTable) )
			{
				retTable[PHYS_LOCATION] = location;
				free( pDevData );
				return retTable;
			}

			// TODO : remove here
			/*
			if ( false == WinVerHelper::isVistaOrGreater() )
			{
				retTable[PHYS_LOCATION] = location;
				this->debugLog("parse location fail");
				retTable[HUB_ID] = INVALID_HUB_ID;
				retTable[PORT_ID] = INVALID_PORT_ID;
				return retTable;
			}

			this->debugLog("parse location in XP, just give invalid location");
			retTable[PHYS_LOCATION] = location;
			retTable[HUB_ID] = INVALID_HUB_ID;
			retTable[PORT_ID] = INVALID_PORT_ID;
			*/
			retTable[PHYS_LOCATION] = location;
			retTable[HUB_ID] = INVALID_HUB_ID;
			retTable[PORT_ID] = INVALID_PORT_ID;
			//retTable.clear();
		}
    }

    free( pDevData );

    return retTable;
}

std::string WinDevSearcher::getLocationPaths( HDEVINFO hDvcInfo, SP_DEVINFO_DATA devInfoData, size_t backwardTime ) const
{
    std::string retLocation;

    /* get instance id */
    DWORD _requiredSize = 0;
    DWORD requiredSize = 0;
    SetupDiGetDeviceInstanceId(hDvcInfo, &devInfoData, NULL, 0, &_requiredSize);
    requiredSize = _requiredSize;
    TCHAR * SIDTmp = new TCHAR[requiredSize];
    memset(SIDTmp, 0, requiredSize);
    //SetupDiGetDeviceInstanceId(hDvcInfo, &did, SIDTmp, requiredSize, &_requiredSize);
    SetupDiGetDeviceInstanceId(hDvcInfo, &devInfoData, SIDTmp, requiredSize, &_requiredSize);
    delete[] SIDTmp;

    /* get hub and port */
    DEVINST devInstParent;
    TCHAR szDeviceInstanceID [MAX_DEVICE_ID_LEN];
    CONFIGRET status = CR_SUCCESS;
    DEVINST currentDevInst = devInfoData.DevInst;
    for ( unsigned int bIdx = 0; bIdx < backwardTime; bIdx++ )
    {
        status = CM_Get_Parent( &devInstParent, currentDevInst, 0 );
        if ( status != CR_SUCCESS )
        {
            break;
        }
        currentDevInst = devInstParent;
    }
    if ( status == CR_SUCCESS )
    {
        status = CM_Get_Device_ID ( devInstParent, szDeviceInstanceID, sizeof( szDeviceInstanceID )/sizeof( szDeviceInstanceID[0] ), 0 );
        if ( status == CR_SUCCESS )
        {
            SP_DEVINFO_DATA parentDevInfo ;
            parentDevInfo.cbSize = sizeof( SP_DEVINFO_DATA );
            int result = SetupDiOpenDeviceInfo( hDvcInfo, szDeviceInstanceID, NULL, 0, &parentDevInfo );

            if ( result )
            {
                /* first call for size */
                DWORD regDataType = 0;
                BYTE * localBuffer;
                BOOL getProSuccess = getDeviceRegistryProperty(
                        hDvcInfo,
                        (PSP_DEVINFO_DATA)(&parentDevInfo),
                        SPDRP_LOCATION_PATHS,
                        regDataType,
                        localBuffer
                        );
                /*
                BOOL getProSuccess = SetupDiGetDeviceRegistryProperty(
                        hDvcInfo,
                        &parentDevInfo,
                        SPDRP_LOCATION_INFORMATION,
                        &regDataType,
                        NULL,
                        0,
                        &reqSize
                        );

                int bufSize = reqSize;
                BYTE * localBuffer = new BYTE[bufSize];
                memset(localBuffer, 0, bufSize);
                getProSuccess = SetupDiGetDeviceRegistryProperty(
                        hDvcInfo,
                        &parentDevInfo,
                        SPDRP_LOCATION_INFORMATION,
                        &regDataType,
                        localBuffer,
                        bufSize,
                        &reqSize
                        );
                        */
                if ( TRUE == getProSuccess )
                {
#ifdef UNICODE
                    wchar_t * wlocation = (wchar_t*)localBuffer;
                    //wprintf(L"get Location %s\n", wlocation);
                    char * location = this->wcharToChar(wlocation);
                    if ( 0 == location )
                    {
                        //retTable.clear();
                        printf("convert fail\n");
                        this->debugLog("convert wchar location to char fail\n");
                        return retLocation;
                    }
                    //this->debugLog("convert wchar location to char fail\n");
                    retLocation = std::string(location);
                    delete[] location;
                    //printf("get location %s\n", retLocation.c_str());
                    return retLocation;
#else
                    char * location = (char*)localBuffer;
#endif
                    retLocation = std::string(location);
                    /*
                    retTable[PHYS_LOCATION] = std::string(location);
                    if ( false == parseUsbHubString(location, retTable) )
                    {
                        // TODO : remove here
                        if ( false == WinVerHelper::isVistaOrGreater() )
                        {
                            return retTable;
                        }
                        //retTable.clear();
                        this->debugLog("parse location fail");
                        retTable[HUB_ID] = INVALID_HUB_ID;
                        retTable[PORT_ID] = INVALID_PORT_ID;
                        return retTable;
                    }
                    */
#ifdef UNICODE
                    delete[] location;
#endif
                }
                else
                {
                    if ( ERROR_INVALID_DATA == GetLastError() )
                    {
                        this->debugLog("There is no location in this host");
                    }
                    else
                    {
                        this->debugLog("get registry property fail");
                    }
                }

                delete[] localBuffer;
            }
            else
            {
                this->debugLog("open parent information fail");
            }
        }
        else
        {
            this->debugLog("get parent instance id fail");
        }
    }
    else
    {
        this->debugLog("find parent fail");
    }

    return retLocation;
}

std::string WinDevSearcher::getLocation( HDEVINFO hDvcInfo, SP_DEVINFO_DATA devInfoData, size_t backwardTime ) const
{
	std::string retLocation;

	/* get instance id */
	DWORD _requiredSize = 0;
	DWORD requiredSize = 0;
	SetupDiGetDeviceInstanceId(hDvcInfo, &devInfoData, NULL, 0, &_requiredSize);
	requiredSize = _requiredSize;
	TCHAR * SIDTmp = new TCHAR[requiredSize];
	memset(SIDTmp, 0, requiredSize);
	//SetupDiGetDeviceInstanceId(hDvcInfo, &did, SIDTmp, requiredSize, &_requiredSize);
	SetupDiGetDeviceInstanceId(hDvcInfo, &devInfoData, SIDTmp, requiredSize, &_requiredSize);
	delete[] SIDTmp;

	/* get hub and port */
	DEVINST devInstParent;
	TCHAR szDeviceInstanceID [MAX_DEVICE_ID_LEN]; 
	CONFIGRET status = CR_SUCCESS;
	DEVINST currentDevInst = devInfoData.DevInst;
	for ( unsigned int bIdx = 0; bIdx < backwardTime; bIdx++ )
	{
		status = CM_Get_Parent( &devInstParent, currentDevInst, 0 );
		if ( status != CR_SUCCESS )
		{
			break;
		}
		currentDevInst = devInstParent;
	}
	if ( status == CR_SUCCESS )
	{
		status = CM_Get_Device_ID ( devInstParent, szDeviceInstanceID, sizeof( szDeviceInstanceID )/sizeof( szDeviceInstanceID[0] ), 0 );
		if ( status == CR_SUCCESS )  
		{
			SP_DEVINFO_DATA parentDevInfo ;
			parentDevInfo.cbSize = sizeof( SP_DEVINFO_DATA );
			int result = SetupDiOpenDeviceInfo( hDvcInfo, szDeviceInstanceID, NULL, 0, &parentDevInfo );

			if ( result )
			{
				/* first call for size */
				DWORD regDataType = 0;
				BYTE * localBuffer;
				BOOL getProSuccess = getDeviceRegistryProperty(
						hDvcInfo,
						(PSP_DEVINFO_DATA)(&parentDevInfo),
						SPDRP_LOCATION_INFORMATION,
						regDataType,
						localBuffer
						);
				/*
				BOOL getProSuccess = SetupDiGetDeviceRegistryProperty(
						hDvcInfo,
						&parentDevInfo,
						SPDRP_LOCATION_INFORMATION,
						&regDataType,
						NULL,
						0,
						&reqSize
						);

				int bufSize = reqSize;
				BYTE * localBuffer = new BYTE[bufSize];
				memset(localBuffer, 0, bufSize);
				getProSuccess = SetupDiGetDeviceRegistryProperty(
						hDvcInfo,
						&parentDevInfo,
						SPDRP_LOCATION_INFORMATION,
						&regDataType,
						localBuffer,
						bufSize,
						&reqSize
						);
						*/
				if ( TRUE == getProSuccess )
				{
#ifdef UNICODE
					wchar_t * wlocation = (wchar_t*)localBuffer;
					//wprintf(L"get Location %s\n", wlocation);
					char * location = this->wcharToChar(wlocation);
					if ( 0 == location )
					{
						//retTable.clear();
						printf("convert fail\n");
						this->debugLog("convert wchar location to char fail\n");
						return retLocation;
					}
					//this->debugLog("convert wchar location to char fail\n");
					retLocation = std::string(location);
					delete[] location;
					//printf("get location %s\n", retLocation.c_str());
					return retLocation;
#else
					char * location = (char*)localBuffer;
#endif
					retLocation = std::string(location);
					/*
					retTable[PHYS_LOCATION] = std::string(location);
					if ( false == parseUsbHubString(location, retTable) )
					{
						// TODO : remove here
						if ( false == WinVerHelper::isVistaOrGreater() )
						{
							return retTable;
						}
						//retTable.clear();
						this->debugLog("parse location fail");
						retTable[HUB_ID] = INVALID_HUB_ID;
						retTable[PORT_ID] = INVALID_PORT_ID;
						return retTable;
					}
					*/
#ifdef UNICODE
					delete[] location;
#endif
				}
				else
				{
					if ( ERROR_INVALID_DATA == GetLastError() )
					{
						this->debugLog("There is no location in this host");
					}
					else
					{
						this->debugLog("get registry property fail");
					}
				}

				delete[] localBuffer;
			}
			else
			{
				this->debugLog("open parent information fail");
			}
		}
		else
		{
			this->debugLog("get parent instance id fail");
		}
	}
	else
	{
		this->debugLog("find parent fail");
	}

	return retLocation;
}

bool WinDevSearcher::parseUsbHubString(const char * strBuf, DeviceAttrTable & table)  const
{
	/*
	 * Parse somethingg like this "Port_#0001.Hub_#0006"
	 */
	const std::string portPrefix = "Port_#";
	const std::string hubPrefix = "Hub_#";
	const int idLen = 4;
	std::string rawStr(strBuf);

	int portPos = rawStr.find(portPrefix);
	if ( std::string::npos == portPos )
	{
		this->debugLog("parse location fail : " + rawStr);
		return false;
	}
	int hubPos = rawStr.find(hubPrefix);
	if ( std::string::npos == hubPos )
	{
		this->debugLog("parse location fail : " + rawStr);
		return false;
	}

	std::string portid = rawStr.substr( portPos + portPrefix.size(), idLen);
	std::string hubid = rawStr.substr( hubPos + hubPrefix.size(), idLen);

	table[HUB_ID] = hubid;
	table[PORT_ID] = portid;

	return true;
}

bool WinDevSearcher::parseSiSHIDDesc(DeviceAttrTable & table) const
{
    bool isFind = false;
	unsigned short vid = 0;
	unsigned short pid = 0;
	unsigned int reportIDIn = 0;
	unsigned int reportIDOut = 0;
    PHID_DEVICE HidDevice = ( PHID_DEVICE ) calloc( 1, sizeof ( HID_DEVICE ) );

#ifdef UNICODE
    int nwchar = MultiByteToWideChar(CP_ACP, 0, table[DEV_PATH].c_str(), -1, NULL, 0);
    TCHAR * devPath = new TCHAR[nwchar];
    MultiByteToWideChar(CP_ACP, 0, table[DEV_PATH].c_str(), -1, (LPWSTR)devPath, nwchar);
#endif
	//memcpy(devPath, table[DEV_PATH].c_str(), table[DEV_PATH].size());
	//char * devPath = new char[table[DEV_PATH].size()];
	//memcpy(devPath, table[DEV_PATH].c_str(), table[DEV_PATH].size());

    if (NULL != HidDevice) 
    {
        //if ( OpenHidDevice( devPath, FALSE, FALSE, FALSE, FALSE, 0, HidDevice ) == TRUE )
        if ( OpenHidDevice( (LPTSTR)devPath, FALSE, FALSE, FALSE, FALSE, 0, HidDevice ) == TRUE )
        {
            printf("Orz: %ls, %d\n",devPath, HidDevice->FeatureDataLength);
            //LOGD("open hid device %s success!", DevicePath);
			this->debugLog("open hid device " + table[DEV_PATH] + " success");
            if(HidDevice->FeatureDataLength > 10)
            {
                // using for pen HID
                vid = HidDevice->Attributes.VendorID;
                pid = HidDevice->Attributes.ProductID;

                /* default useing the first id */
                reportIDIn = HidDevice->InputData[0].ReportID;
                reportIDOut = HidDevice->OutputData[0].ReportID;

                table[PROTOCOL] = HID_USB;
                table[SUCCESSIVE_INTERFACE_NAME] = HID_FEATURE;

                table[DEV_TYPE] = SIS_819;
                table[WR_API] = WR_SYNCH_HID_CONTROL;
                isFind = true;

                printf("Cloud Orz: %ls, %d\n",devPath, HidDevice->FeatureDataLength);
            }
			/*
			 * We have to detect 819 first because 819 retains report id 0x9/0xa.
			 */
			if ( 4 == HidDevice->FeatureDataLength && (0x2d == HidDevice->FeatureData[0].ReportID) &&
					(0x21 == HidDevice->FeatureData[1].ReportID) &&
					(0x25 == HidDevice->FeatureData[2].ReportID) &&
					(0x29 == HidDevice->FeatureData[3].ReportID))
			{
				vid = HidDevice->Attributes.VendorID;
				pid = HidDevice->Attributes.ProductID;

				/* default useing the first id */
				reportIDIn = HidDevice->InputData[0].ReportID;
				reportIDOut = HidDevice->OutputData[0].ReportID;

                table[PROTOCOL] = HID_USB;
                //table[PROTOCOL] = HID_OVER_SPI;
				table[SUCCESSIVE_INTERFACE_NAME] = PROTOCOL_NONE;
                //table[PROTOCOL] = HID_OVER_I2C;

				table[DEV_TYPE] = SIS_819;
				table[WR_API] = WR_SYNCH_HID_CONTROL;
				/*
				 * try to find com port number if there is one.
				 */
				std::string comport = getComport(vid, pid);
				if ( false == comport.empty() )
				{
					table[COMPORT_NUMBER] = comport;
				}
				isFind = true;
			}
			if ( 5 == HidDevice->FeatureDataLength && (0x2d == HidDevice->FeatureData[0].ReportID) &&
					(0x21 == HidDevice->FeatureData[1].ReportID) &&
					(0x25 == HidDevice->FeatureData[2].ReportID) &&
					(0x29 == HidDevice->FeatureData[3].ReportID) &&
					(0x2f == HidDevice->FeatureData[4].ReportID))
			{
				vid = HidDevice->Attributes.VendorID;
				pid = HidDevice->Attributes.ProductID;

				/* default useing the first id */
				reportIDIn = HidDevice->InputData[0].ReportID;
				reportIDOut = HidDevice->OutputData[0].ReportID;

                table[PROTOCOL] = HID_OVER_SPI;
				table[SUCCESSIVE_INTERFACE_NAME] = PROTOCOL_NONE;

				table[DEV_TYPE] = SIS_819;
				table[WR_API] = WR_SYNCH_HID_CONTROL;
				isFind = true;
			}
			else if ( HidDevice->InputDataLength == 1 && HidDevice->OutputDataLength == 1 )
            {                
                if ( HidDevice->InputData[0].ReportID == 0x0a && HidDevice->OutputData[0].ReportID == 0x09 )
                {
					reportIDIn = HidDevice->InputData[0].ReportID;
					reportIDOut = HidDevice->OutputData[0].ReportID;
                    if ( HidDevice->InputData[0].ButtonData.MaxUsageLength == 63 && HidDevice->OutputData[0].ButtonData.MaxUsageLength == 63 )
                    {
                        /*
                        deviceType = DEV_817_USB;
                        */
                        vid = HidDevice->Attributes.VendorID;
                        pid = HidDevice->Attributes.ProductID;

						table[PROTOCOL] = HID_USB;
						table[SUCCESSIVE_INTERFACE_NAME] = PROTOCOL_NONE;

						table[DEV_TYPE] = AEGIS;

                        isFind = true;
                    }
                    else if ( HidDevice->InputData[0].ButtonData.MaxUsageLength == 18 && HidDevice->OutputData[0].ButtonData.MaxUsageLength == 18 )
                    {
                        /*
                        deviceType = DEV_816_USB, do nothing.
                        */
						this->debugLog("find 816 device, not support");
                    }
                    else if ( HidDevice->InputData[0].ButtonData.MaxUsageLength == 57 && HidDevice->OutputData[0].ButtonData.MaxUsageLength == 57 )
                    {
                        /*
                        deviceType = DEV_817_HID_OVER_I2C;
                        */
						/*
                        __deviceDef.vid = HidDevice->Attributes.VendorID;
                        __deviceDef.pid = HidDevice->Attributes.ProductID;
                        __deviceDef.type = I2C_817;
						*/
                        vid = HidDevice->Attributes.VendorID;
                        pid = HidDevice->Attributes.ProductID;
						table[PROTOCOL] = HID_OVER_I2C;
						table[SUCCESSIVE_INTERFACE_NAME] = PROTOCOL_NONE;
						//printf("AEGIS trans to Hydra!\n");
						table[DEV_TYPE] = AEGIS;
						//table[DEV_TYPE] = HYDRA;
                        isFind = true;
                    }
                }
                else if ( HidDevice->InputData[0].ReportID == 0x35 && HidDevice->OutputData[0].ReportID == 0x53 )
                {
                    /*
                    deviceType = DEV_816_USB_BRG;
                    */
					/*
                    __deviceDef.vid = HidDevice->Attributes.VendorID;
                    __deviceDef.pid = HidDevice->Attributes.ProductID;
                    isFind = true;
					*/
					this->debugLog("find 816 device, not support");
                }
            }
            else if ( HidDevice->InputDataLength == 2 && HidDevice->OutputDataLength == 1 )
			{
				if ( HidDevice->InputData[0].ReportID == 0xa && HidDevice->InputData[1].ReportID == 0xc )
				{
					vid = HidDevice->Attributes.VendorID;
					pid = HidDevice->Attributes.ProductID;
					table[PROTOCOL] = HID_OVER_I2C;
					table[SUCCESSIVE_INTERFACE_NAME] = PROTOCOL_NONE;
					table[DEV_TYPE] = HYDRA;
					isFind = true;
				}
			}
			else if ( HidDevice->InputDataLength == 2 && HidDevice->OutputDataLength == 2 )
			{
				if ( ( HidDevice->InputData[0].ReportID == 0x35 && HidDevice->InputData[1].ReportID == 0x0a ) && ( HidDevice->OutputData[0].ReportID == 0x53 && HidDevice->OutputData[1].ReportID == 0x09 ) )
				{
					reportIDIn = HidDevice->InputData[0].ReportID;
					reportIDOut = HidDevice->OutputData[0].ReportID;
					/*
					   deviceType = DEV_817_USB_BRG;
					   */
					/*
					__deviceDef.vid = HidDevice->Attributes.VendorID;
					__deviceDef.pid = HidDevice->Attributes.ProductID;
					__deviceDef.type = USB_817;
					*/
					vid = HidDevice->Attributes.VendorID;
					pid = HidDevice->Attributes.ProductID;

					isFind = getDeviceTypeByBridgePID(pid, table);
				}
			}

			CloseHidDevice( HidDevice );
		}
		else
		{
			//DWORD code = GetLastError();
			//LOGD("open hid device %s fail! by %04x\n", DevicePath, code);
			//printf("the code is %x\n", code);
			this->debugLog("open hid device " + table[DEV_PATH] + " fail");

			/*
			   if ( code == ERROR_SHARING_VIOLATION )
			   m_isNeedReInstall = 1;
			   */
		}

		free( HidDevice );
		HidDevice = NULL;
	}

	if ( true == isFind )
	{
		table[VID] = Convert::fillZeroTo4Digit(Convert::toHexString(vid));
		table[PID] = Convert::fillZeroTo4Digit(Convert::toHexString(pid));
		table[REPORT_ID_IN] = Convert::toHexString(reportIDIn);
		table[REPORT_ID_OUT] = Convert::toHexString(reportIDOut);
	}

	return isFind;
}

//void WinDevSearcher::searchCOM() const
std::string WinDevSearcher::getComport(const unsigned short vid, const unsigned short pid) const
{
	std::string comport;
	
	AUChar _vid[10];
	memset(_vid, 0, 10);
#if defined(UNICODE) || defined(_UNICODE)
	swprintf(_vid, L"%04x", vid);
#else
	sprintf(_vid, "%04x", vid);
#endif

	AUChar _pid[10];
	memset(_pid, 0, 10);
#if defined(UNICODE) || defined(_UNICODE)
	swprintf(_pid, L"%04x", pid);
#else
	sprintf(_pid, "%04x", pid);
#endif

	AUString targetVID = AUString(AUTEXT(VID_)) + AUString(_vid);
	AUString targetPID = AUString(AUTEXT(PID_)) + AUString(_pid);

	std::transform(targetVID.begin(), targetVID.end(), targetVID.begin(), ::toupper);
	std::transform(targetPID.begin(), targetPID.end(), targetPID.begin(), ::toupper);

	HDEVINFO hDevInfo = SetupDiGetClassDevs( 
			&GUID_DEVINTERFACE_USB_DEVICE,
			NULL, 
			NULL, 
			DIGCF_PRESENT|DIGCF_INTERFACEDEVICE 
			);

	SP_INTERFACE_DEVICE_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof ( SP_INTERFACE_DEVICE_DATA );


	SP_DEVINFO_DATA devInfo;
	BOOL bMoreItems = TRUE;
	int nIndex = 0;
	while (bMoreItems)
	{
		//Enumerate the current device
		devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
		bMoreItems = SetupDiEnumDeviceInfo(hDevInfo, nIndex++, &devInfo);
		if (bMoreItems)
		{
			//Did we find a serial port for this device
			//BOOL bAdded = FALSE;
			HKEY interfaceKey = SetupDiOpenDevRegKey(hDevInfo, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
			if ( INVALID_HANDLE_VALUE == interfaceKey )
			{
				printf("open key fail with %d\n", GetLastError());
				continue;
			}
			DWORD valueType;
			AUChar valueBuf[MAX_REG_VALUE_LEN];
			DWORD valueLen = MAX_REG_VALUE_LEN;
			DWORD ret = RegQueryValueEx(interfaceKey, TEXT("SymbolicName"), NULL, &valueType, (LPBYTE)valueBuf, &valueLen);
			//ret = RegQueryValueEx(interfaceKey, TEXT("SelectiveSuspendEnabled"), NULL, &valueType, (LPBYTE)valueBuf, &valueLen);
			if ( ERROR_SUCCESS == ret )
			{
				//wprintf(L"get symbolic name %s", valueBuf);
				AUString sn(valueBuf);
				//if ( std::string::npos != sn.find(L"VID_0457") && std::string::npos != sn.find(L"PID_D819") )
				if ( std::string::npos != sn.find(targetVID) && std::string::npos != sn.find(targetPID) )
				{
					USBDevNode * root = USBDevNode::buildTree(hDevInfo, devInfo);

					if ( root->numChild() < 2 )
					{
						/* it only has a HID interface */
						delete root;
						break;
					}

					for ( unsigned int cIdx = 0; cIdx < root->numChild(); cIdx++ )
					{
						SP_DEVINFO_DATA childDevInfo;
						childDevInfo.cbSize = sizeof(SP_DEVINFO_DATA);
						BOOL success = SetupDiOpenDeviceInfo(hDevInfo, root->child(cIdx)->instanceIDStr().c_str(), NULL, 0, &childDevInfo);
						if ( FALSE == success )
						{
							printf("setup open child fail with %d\n", GetLastError());
							continue;
						}
						HKEY childKey = SetupDiOpenDevRegKey(hDevInfo, &childDevInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
						if ( INVALID_HANDLE_VALUE == childKey )
						{
							printf("open key fail with %d\n", GetLastError());
							continue;
						}
						valueLen = MAX_REG_VALUE_LEN;
						//ret = RegQueryValueEx(childKey, L"PortName", NULL, &valueType, (LPBYTE)valueBuf, &valueLen);
						ret = RegQueryValueEx(childKey, REG_PORT_NAME, NULL, &valueType, (LPBYTE)valueBuf, &valueLen);
						if ( ERROR_SUCCESS == ret )
						{
							//wprintf(L"get port name %s", valueBuf);
#if defined(UNICODE) || defined(_UNICODE)
							char * _value = wcharToChar(valueBuf);
							if ( 0 != _value )
							{
								comport = std::string(_value);
								delete[] _value;
							}
#else
							comport = std::string(valueBuf);
#endif
						}
						else
						{
							//printf("get port name fail with %d(%d)\n", ret, GetLastError());
						}
					}
					delete root;
				}
			}
			/*
			while( RegEnumKeyEx(interfaceKey, kIdx++, name, &nameSize, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS )
			{
				nameSize = 1024;
				//printf("key : %s\n", name);
			}
			*/
		}
	}

	SetupDiDestroyDeviceInfoList( hDevInfo );

	if ( false == comport.empty() )
	{
		std::string comportPrefix(REG_PORT_NAME_PREFIX);
		size_t pos = comport.find(comportPrefix);
		if ( std::string::npos == pos )
		{
			comport = "";
			return comport;
		}
		comport = comport.substr(pos + comportPrefix.size(), comport.size() - comportPrefix.size());
	}

	return comport;
}

BOOL WinDevSearcher::getDeviceRegistryProperty(
		HDEVINFO devInfoSet, 
		PSP_DEVINFO_DATA devInfoData,
		DWORD property,
		DWORD & propertyRegDataType,
		BYTE * & propertyData)
{
	DWORD reqSize = 0;
	DWORD regDataType = 0;
	BOOL getProSuccess = SetupDiGetDeviceRegistryProperty(
			devInfoSet,
			devInfoData,
			property,
			&regDataType,
			NULL,
			0,
			&reqSize
			);


	propertyData = new BYTE[reqSize];
	memset(propertyData, 0, reqSize);

	int bufSize = reqSize;
	getProSuccess = SetupDiGetDeviceRegistryProperty(
			devInfoSet,
			devInfoData,
			property,
			&regDataType,
			propertyData,
			bufSize,
			&reqSize
			);

	propertyRegDataType = regDataType;

	return getProSuccess;
}

BOOL WinDevSearcher::getDeviceRegistryProperty(
		HDEVINFO devInfoSet, 
		PSP_DEVINFO_DATA devInfoData,
		DWORD property,
		BYTE * & propertyData)
{
	DWORD regDataType = 0;

	return getDeviceRegistryProperty(
			devInfoSet,
			devInfoData,
			property,
			regDataType,
			propertyData);
}


void WinDevSearcher::debugLog(const std::string msg) const
{
#ifdef _DEBUG_LOG
	printf("%s\n", msg.c_str());
#endif
}

bool WinDevSearcher::getDeviceTypeByBridgePID(const unsigned short pid, DeviceAttrTable & table) const
{
	bool isFound = false;

	if ( Convert::toHexString(pid) == Convert::lowerFrom(SIS_BRIDGE_PID_F818) )
	{
		table[DEV_TYPE] = HYDRA;
	}
	else if ( Convert::toHexString(pid) == Convert::lowerFrom(SIS_BRIDGE_PID_F817) )
	{
		table[DEV_TYPE] = AEGIS;
	}
	else
	{
		return isFound;
	}

	table[PROTOCOL] = HID_USB;
	table[SUCCESSIVE_INTERFACE_NAME] = I2C;
	table[SUB_PROTOCOL] = HID_USB_BRIDGE;
	isFound = true;

	return isFound;
}

char * WinDevSearcher::wcharToChar(wchar_t * src) const
{
	std::size_t wlen = std::wcslen(src);
	char * ret = new char[ (wlen * 2) + 1];
	memset(ret, 0, (wlen * 2) + 1);
	int num = std::wcstombs( ret, src, (wlen * 2) + 1);
	if ( num == (std::size_t) - 1 )
	{
		delete[] ret;
		return 0;
	}
	ret[(wlen*2)] = '\0';
	return ret;
}
