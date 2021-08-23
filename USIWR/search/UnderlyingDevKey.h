/******************************************************************************//**
 *  @file UnderlyingDevKey.h
 *  @class UnderlyingDevKey
 *  @brief The underlying device attribute key.
 * 
 *  
 *  @author CY Fan
 *  @date 2016-01-11, 09:54:21
 **********************************************************************************/
#ifndef UNDERLYINGDEVKEY_H_
#define UNDERLYINGDEVKEY_H_

#include "string"
#include "vector"
#include "map"

namespace SiS
{

typedef std::string AttributeKey;
typedef std::string AttributeValue;
typedef std::map<AttributeKey, AttributeValue> DeviceAttrTable;

typedef std::string DevicePath;
//typedef std::vector< std::pair<DevicePath, DeviceAttrTable> > DeviceSet;
typedef std::vector<DeviceAttrTable> DeviceSet;

/*
 * OS
 */
const std::string OS = "OS";

/*
 * Protocol
 */
const std::string PROTOCOL = "Protocol";
const std::string SUB_PROTOCOL = "SubProtocol";
/* key word INTERFACE is not available in Windows QT */
const std::string INTERFACE_NAME = "Interface"; // interface
const std::string SUCCESSIVE_INTERFACE_NAME = "Successive_Interface"; // interface
const std::string REPORT_ID_IN = "ReportIdIn";
const std::string REPORT_ID_OUT = "ReportIdOut";
const std::string REPORT_COUNT_IN = "ReportCountIn";
const std::string REPORT_COUNT_OUT = "ReportCountOut";

/* hardware information */
const std::string DEV_PATH = "DevicePath";
/* linux only */
const std::string DEV_NUM = "DeviceNumber"; // linux device major and minor number
const std::string HIDRAW_NUM = "HidrawNumber"; // linux device major and minor number

const std::string PARENT_PATH = "ParentPath";
const std::string LOCATION_PATHS = "LocationPaths";
const std::string PHYS_LOCATION = "PhysLocation";
const std::string HUB_ID = "hubId";
const std::string PORT_ID = "portId";
const std::string VID = "vid";
const std::string PID = "pid";
const std::string DEV_TYPE = "DeviceType";
const std::string COMPORT_NUMBER = "ComportNumber";
const std::string D_ID = "Did";

/* IO API featrue */
/*
 * Because XP can't use ReadFileEx, we have to implement no-blocking IO by
 * thread.
 */
const std::string WR_API = "WRApi";

}; // SiS

#endif
