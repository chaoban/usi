#ifndef LINUXHIDFEATURECTRL_H
#define LINUXHIDFEATURECTRL_H

#include <QCoreApplication>
#include <string>
using namespace std;


class LinuxHIDFeatureCtrl
{
public:
    ~LinuxHIDFeatureCtrl();

    static LinuxHIDFeatureCtrl& instance()
    {
        static LinuxHIDFeatureCtrl ins;
        return ins;
    }

    bool openDevice();
    void setHIDFeature(unsigned char* buffer, int length);
    void getHIDFeature(unsigned char* buffer, int length);
    string getDevicePath();

private:
    LinuxHIDFeatureCtrl();
    int fd;
};

#define SHIDFeatureCtrl LinuxHIDFeatureCtrl::instance()

#endif // LINUXHIDFEATURECTRL_H
