#include "WinHIDFeatureCtrl.h"
#include "WinDevSearcher.h"
#include <QString>

using namespace SiS;
using namespace SiS::Model;

WinHIDFeatureCtrl::WinHIDFeatureCtrl()
    :hDevice(INVALID_HANDLE_VALUE)
{
}

WinHIDFeatureCtrl::~WinHIDFeatureCtrl()
{
    CloseHandle(hDevice);
}

void WinHIDFeatureCtrl::setHIDFeature(unsigned char *buffer, int length)
{
    bool bSuccess = HidD_SetFeature(hDevice, buffer, length);
    if(bSuccess == false)
    {
        printf("HID Set Feature Error:%d\n",GetLastError());
//        for(int i =0; i < length; i++)
//        {
//            printf("0x%02X ", buffer[i]);
//        }
//        printf("\n");
    }
    else
    {
        printf("HID Set Feature succeed.\n");
    }
}

void WinHIDFeatureCtrl::getHIDFeature(unsigned char *buffer, int length)
{
    bool bSuccess = HidD_GetFeature(hDevice, buffer, length);
    if(bSuccess == false)
    {
        printf("HID Get Feature Error:%d\n",GetLastError());
//        for(int i =0; i < length; i++)
//        {
//            printf("0x%02X ", buffer[i]);
//        }
//        printf("\n");
    }
    else
    {
        printf("HID Get Feature succeed.\n");
        for(int i =0; i < length; i++)
        {
            printf("0x%02X ", buffer[i]);
        }
        printf("\n");
    }
}

string WinHIDFeatureCtrl::getDevicePath()
{
    WinDevSearcher ws;
    DeviceSet ds = ws.listDevice(HID_USB);

    for(int i = 0; i < ds.size(); i++)
    {
        for(DeviceAttrTable::iterator it= ds[i].begin(); it != ds[i].end(); it++)
        {
            printf("[%s]:",it->first.c_str());
            printf("%s\n",it->second.c_str());
        }
//        if(ds[i][SUCCESSIVE_INTERFACE_NAME] == HID_FEATURE)
//            return ds[i][DEV_PATH];
    }
    return "";
}

bool WinHIDFeatureCtrl::openDevice()
{
    // Test 1
//    hDevice = CreateFile(L"\\\\?\\hid#VID_0457&PID_6596&COL04#6&3B5125B9&0&0003#{4d1e55b2-f16f-11cf-88cb-001111000030}", GENERIC_READ | GENERIC_WRITE/*MAXIMUM_ALLOWED*/,
//                   FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    string path = getDevicePath();
    printf("Open path:%s\n",path.c_str());
#ifdef UNICODE
    int nwchar = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, NULL, 0);
    TCHAR * devPath = new TCHAR[nwchar];
    MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, (LPWSTR)devPath, nwchar);
//    hDevice = CreateFile(devPath, GENERIC_READ | GENERIC_WRITE/*MAXIMUM_ALLOWED*/,
//                   FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
//                   0, NULL );
    hDevice = CreateFile(devPath, 0/*MAXIMUM_ALLOWED*/,
                   FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                   0, NULL );
#else
//    hDevice = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE/*MAXIMUM_ALLOWED*/,
//                   FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
//                   0, NULL );
    hDevice = CreateFile(devPath, 0/*MAXIMUM_ALLOWED*/,
                   FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                   0, NULL );
#endif
    if ( hDevice == INVALID_HANDLE_VALUE )
    {
        printf("Open SIS Device Fail, Error code:%d\n",GetLastError());
        return false;
    }
    printf("Open SIS Device succeed.\n");
    return true;
}
