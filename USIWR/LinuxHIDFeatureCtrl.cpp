#include "LinuxHIDFeatureCtrl.h"

/* Linux */
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 */
#ifndef HIDIOCSFEATURE
#warning Please have your distro update the userspace kernel headers
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

LinuxHIDFeatureCtrl::LinuxHIDFeatureCtrl()
{

}

LinuxHIDFeatureCtrl::~LinuxHIDFeatureCtrl()
{
    close(fd);
}

bool LinuxHIDFeatureCtrl::openDevice()
{
    char *device = "/dev/hidraw0";

    /* Open the Device with non-blocking reads. In real life,
         don't use a hard coded path; use libudev instead. */
    fd = open(device, O_RDWR|O_NONBLOCK);

    if (fd < 0)
    {
        perror("Unable to open device");
        return false;
    }
    return true;
}

void LinuxHIDFeatureCtrl::setHIDFeature(unsigned char *buffer, int length)
{
    int res = ioctl(fd, HIDIOCSFEATURE(length), buffer);
    if (res < 0)
        //perror("HIDIOCSFEATURE");
        printf("HID Set Feature Error:%d\n",errno);
    else
        printf("HID Set Feature succeed.\n");
}

void LinuxHIDFeatureCtrl::getHIDFeature(unsigned char *buffer, int length)
{
    int res = ioctl(fd, HIDIOCGFEATURE(length), buffer);
    if (res < 0)
    {
        printf("HID Get Feature Error:%d\n",errno);
    }
    else
    {
        printf("HID Get Feature succeed.\n");
        for (int i = 0; i < res; i++)
            printf("0x%02X ", buffer[i]);
        puts("\n");
    }
}

string LinuxHIDFeatureCtrl::getDevicePath()
{
    return "/dev/hidraw0";
}
