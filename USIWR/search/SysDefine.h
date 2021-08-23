/******************************************************************************//**
 *  @file SysDefine.h
 *  @class SysDefine
 *  @brief Some system-wide common setting/define.
 * 
 *  
 *  @author CY Fan
 *  @date 2016-05-17, 15:41:28
 **********************************************************************************/
#ifndef SYSDEFINE_H_
#define SYSDEFINE_H_

#include "string"

namespace SiS
{
namespace Model
{

const std::string WindowsName = "Windows";
const std::string LinuxName = "Linux";

/*
 * Protocol.
 */
const std::string PROTOCOL_INVALID = "INVALID";
const std::string HID_USB = "HID_USB";
const std::string HID_USB_BRIDGE = "HID_USB_BRIDGE";
const std::string I2C = "I2C";
const std::string PROTOCOL_NONE = "NONE";
const std::string DO_NOT_CARE = "DO_NOT_CARE";
const std::string NO_MATTER = "NO_MATTER";
const std::string HID_OVER_SPI = "HID_SPI";
const std::string HID_FEATURE = "HID_FEATURE";


/*
 * Invalid usb port and hub.
 * In 819, it is possible that we can't get the location.
 */
const std::string INVALID_HUB_ID = "4096";
const std::string INVALID_PORT_ID = "4096";

/* linux only */
const std::string HID_RAW = "HID_RAW";

/*
 * Windows only
 */
const std::string HID_OVER_I2C = "HID_I2C";

/* write/read by sychronous */
const std::string WR_SYNCH = "WR_SYNCH";
/* using GetFeature/SetFeature */
const std::string WR_SYNCH_HID_CONTROL = "WR_SYNCH_HID_CONTROL";

/*
 * Device type
 */
const std::string SIS_817 = "817";
const std::string AEGIS = "817";

const std::string SIS_819 = "819";
const std::string HYDRA = "819";

/*
 * Bridge type
 */
const std::string SIS_BRIDGE_PID_F817 = "F817";
const std::string SIS_BRIDGE_PID_F818 = "F818";

#ifdef __linux__
#define VA_COPY(dst, src) va_copy(dst, src)
#else
#define VA_COPY(dst, src) dst = src
#endif

#ifdef __linux__
typedef unsigned char BYTE;
#endif

}; // Model
}; // SiS


#endif
