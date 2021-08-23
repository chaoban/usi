#ifndef WINHIDFEATURECTRL_H
#define WINHIDFEATURECTRL_H

#include <QCoreApplication>
#include <Windows.h>
#include <hidsdi.h>
#include <string>
using namespace std;

class USIWRArgParser;

class WinHIDFeatureCtrl
{
public:
    ~WinHIDFeatureCtrl();

    static WinHIDFeatureCtrl& instance()
    {
        static WinHIDFeatureCtrl ins;
        return ins;
    }

    bool openDevice();
    void setHIDFeature(unsigned char* buffer, int length);
    void getHIDFeature(unsigned char* buffer, int length);
    string getDevicePath();

private:
    WinHIDFeatureCtrl();
    HANDLE hDevice;
};

#define SHIDFeatureCtrl WinHIDFeatureCtrl::instance()

#endif // WINHIDFEATURECTRL_H
