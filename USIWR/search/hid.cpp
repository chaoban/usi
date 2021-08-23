//#include "stdafx.h"
/*++

Copyright (c) 1996    Microsoft Corporation

Module Name:

    pnp.c

Abstract:

    This module contains the code
    for finding, adding, removing, and identifying hid devices.

Environment:

    User mode

--*/

#include "guiddef.h"
#include <basetyps.h>
#include <stdlib.h>
#include <wtypes.h>
#include <setupapi.h>


extern "C" {
#include "hidsdi.h"
#include "hid.h"
}

#include <strsafe.h>
#include <intsafe.h>
//#include <StaticAssert.h>

//#include "SisTouchIOWindows.h"

#pragma warning(disable:28146) // Warning is meant for kernel mode drivers 


BOOLEAN
OpenHidDevice (
    __in     LPTSTR          DevicePath,
    __in     BOOL           HasReadAccess,
    __in     BOOL           HasWriteAccess,
    __in     BOOL           IsOverlapped,
    __in     BOOL           IsExclusive,
    __in     UINT           IsDebug,
    __inout  PHID_DEVICE    HidDevice
)
/*++
RoutineDescription:
    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific hid device,
    open that device and fill in all the relivant information in the given
    HID_DEVICE structure.

    return if the open and initialization was successfull or not.

--*/
{
//	UNUSED(IsDebug);
    DWORD   accessFlags = 0;
    DWORD   sharingFlags = 0;
    BOOLEAN bSuccess;
    INT     iDevicePathSize;


    if (NULL == DevicePath)
    {
        return (FALSE);
    }

    iDevicePathSize = (INT)strlen((char*)DevicePath) + 1;
	//iDevicePathSize = (INT)QTStrlen(DevicePath) + 1;
    
    HidDevice -> DevicePath = (LPTSTR)malloc(iDevicePathSize);
	//HidDevice -> DevicePath = (LPTSTR)malloc(iDevicePathSize*BYTEPERCHAR);

    if (NULL == HidDevice -> DevicePath) 
    {
        return (FALSE);
    }

    StringCbCopy(HidDevice -> DevicePath, iDevicePathSize, DevicePath);
	//QTStrcpy(HidDevice -> DevicePath, iDevicePathSize, DevicePath);

    if (HasReadAccess)
    {
        accessFlags |= GENERIC_READ;
    }

    if (HasWriteAccess)
    {
        accessFlags |= GENERIC_WRITE;
    }

    if (!IsExclusive)
    {
        sharingFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }
    
    //
    //  The hid.dll api's do not pass the overlapped structure into deviceiocontrol
    //  so to use them we must have a non overlapped device.  If the request is for
    //  an overlapped device we will close the device below and get a handle to an
    //  overlapped device
    //

    //LOGD("[hid.cpp]open hid device %s", DevicePath);
    HidDevice->HidDevice = CreateFile (DevicePath,
                                   accessFlags,
                                   sharingFlags,
                                   NULL,        // no SECURITY_ATTRIBUTES structure
                                   OPEN_EXISTING, // No special create flags
                                   0,   // Open device as non-overlapped so we can get data
                                   NULL);       // No template file

    if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) 
    {
        free(HidDevice -> DevicePath);
        HidDevice -> DevicePath = NULL ;
        return FALSE;
    }
    //LOGD("[hid.cpp]open hid device %s OK!", DevicePath);

    HidDevice -> OpenedForRead = HasReadAccess;
    HidDevice -> OpenedForWrite = HasWriteAccess;
    HidDevice -> OpenedOverlapped = IsOverlapped;
    HidDevice -> OpenedExclusive = IsExclusive;
    
    //
    // If the device was not opened as overlapped, then fill in the rest of the
    //  HidDevice structure.  However, if opened as overlapped, this handle cannot
    //  be used in the calls to the HidD_ exported functions since each of these
    //  functions does synchronous I/O.
    //

    //LOGD("[hid.cpp] prepare get preparsed data");
    if (!HidD_GetPreparsedData (HidDevice->HidDevice, &HidDevice->Ppd)) 
    {
        free(HidDevice -> DevicePath);
        HidDevice -> DevicePath = NULL ;
        CloseHandle(HidDevice -> HidDevice);
        HidDevice -> HidDevice = INVALID_HANDLE_VALUE ;
        return FALSE;
    }

    //LOGD("[hid.cpp] prepare get attribute data");
    if (!HidD_GetAttributes (HidDevice->HidDevice, &HidDevice->Attributes)) 
    {
        free(HidDevice -> DevicePath);
        HidDevice -> DevicePath = NULL;
        CloseHandle(HidDevice -> HidDevice);
        HidDevice -> HidDevice = INVALID_HANDLE_VALUE;
        HidD_FreePreparsedData (HidDevice->Ppd);
        HidDevice->Ppd = NULL;

        return FALSE;
    }

    //LOGD("[hid.cpp] prepare get caps");
    if (!HidP_GetCaps (HidDevice->Ppd, &HidDevice->Caps))
    {
        free(HidDevice -> DevicePath);
        HidDevice -> DevicePath = NULL;
        CloseHandle(HidDevice -> HidDevice);
        HidDevice -> HidDevice = INVALID_HANDLE_VALUE;
        HidD_FreePreparsedData (HidDevice->Ppd);
        HidDevice->Ppd = NULL;

        return FALSE;
    }

    //
    // At this point the client has a choice.  It may chose to look at the
    // Usage and Page of the top level collection found in the HIDP_CAPS
    // structure.  In this way it could just use the usages it knows about.
    // If either HidP_GetUsages or HidP_GetUsageValue return an error then
    // that particular usage does not exist in the report.
    // This is most likely the preferred method as the application can only
    // use usages of which it already knows.
    // In this case the app need not even call GetButtonCaps or GetValueCaps.
    //
    // In this example, however, we will call FillDeviceInfo to look for all
    //    of the usages in the device.
    //

    //LOGD("[hid.cpp] prepare fill deviceinfo");
    bSuccess = FillDeviceInfo(HidDevice);

    if (FALSE == bSuccess)
    {
        CloseHidDevice(HidDevice);
        return (FALSE);
    }
    //LOGD("[hid.cpp] fill deviceinof OK!");

    if (IsOverlapped)
    {
        CloseHandle(HidDevice->HidDevice);

        HidDevice->HidDevice = CreateFile (DevicePath,
                                       accessFlags,
                                       sharingFlags,
                                       NULL,        // no SECURITY_ATTRIBUTES structure
                                       OPEN_EXISTING, // No special create flags
                                       FILE_FLAG_OVERLAPPED, // Now we open the device as overlapped
                                       NULL);       // No template file

        if (INVALID_HANDLE_VALUE == HidDevice->HidDevice) 
        {
            CloseHidDevice(HidDevice);
            return FALSE;
        }
    }

    return (TRUE);
}

BOOLEAN
FillDeviceInfo(
    IN  PHID_DEVICE HidDevice
)
{
    ULONG                           numValues;
    USHORT                         numCaps;
    PHIDP_BUTTON_CAPS     buttonCaps;
    PHIDP_VALUE_CAPS       valueCaps;
    PHID_DATA                    data;
    ULONG                           i;
    USAGE                           usage;
    UINT                              dataIdx;
    ULONG                           newFeatureDataLength;
    ULONG                           tmpSum;    

    //LOGD("[hid.cpp][filldevInfo][input] prepare input data buffer");
    //
    // setup Input Data buffers.
    //

    //
    // Allocate memory to hold on input report
    //

    HidDevice->InputReportBuffer = (PCHAR) 
        calloc (HidDevice->Caps.InputReportByteLength, sizeof (CHAR));


    //
    // Allocate memory to hold the button and value capabilities.
    // NumberXXCaps is in terms of array elements.
    //
    
    HidDevice->InputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc (HidDevice->Caps.NumberInputButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
    {
        free(HidDevice->InputReportBuffer);
        HidDevice->InputReportBuffer = NULL;
        free(buttonCaps);
        HidDevice->InputButtonCaps = NULL;
        return (FALSE);
    }

    HidDevice->InputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc (HidDevice->Caps.NumberInputValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
    {
        free(HidDevice->InputReportBuffer);
        HidDevice->InputReportBuffer = NULL;
        free(buttonCaps);
        HidDevice->InputButtonCaps = NULL;
        free(valueCaps);
        HidDevice->InputValueCaps = NULL;    
        return(FALSE);
    }

    //
    // Have the HidP_X functions fill in the capability structure arrays.
    //

    numCaps = HidDevice->Caps.NumberInputButtonCaps;

    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetButtonCaps (HidP_Input,
                            buttonCaps,
                            &numCaps,
                            HidDevice->Ppd)))
        {
            return(FALSE);
        }
    }

    numCaps = HidDevice->Caps.NumberInputValueCaps;

    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetValueCaps (HidP_Input,
                           valueCaps,
                           &numCaps,
                           HidDevice->Ppd)))
        {
            return(FALSE);
        }
    }


    //
    // Depending on the device, some value caps structures may represent more
    // than one value.  (A range).  In the interest of being verbose, over
    // efficient, we will expand these so that we have one and only one
    // struct _HID_DATA for each value.
    //
    // To do this we need to count up the total number of values are listed
    // in the value caps structure.  For each element in the array we test
    // for range if it is a range then UsageMax and UsageMin describe the
    // usages for this range INCLUSIVE.
    //
    
    numValues = 0;
    for (i = 0; i < HidDevice->Caps.NumberInputValueCaps; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            numValues += valueCaps->Range.UsageMax - valueCaps->Range.UsageMin + 1;
            if(valueCaps->Range.UsageMin >= valueCaps->Range.UsageMax + (HidDevice->Caps).NumberInputButtonCaps)
            {
                return (FALSE); // overrun check
            }
        }
        else
        {
            numValues++;
        }
    }

    valueCaps = HidDevice->InputValueCaps;

    //
    // Allocate a buffer to hold the struct _HID_DATA structures.
    // One element for each set of buttons, and one element for each value
    // found.
    //

    HidDevice->InputDataLength = HidDevice->Caps.NumberInputButtonCaps
                               + numValues;

    HidDevice->InputData = data = (PHID_DATA)
        calloc (HidDevice->InputDataLength, sizeof (HID_DATA));

    if (NULL == data)
    {
        free(data);
        HidDevice->InputData = NULL;
        return (FALSE);
    }

    //LOGD("[hid.cpp][filldevInfo][input] fill button data");
    //
    // Fill in the button data
    //
    dataIdx = 0;
    for (i = 0;
         i < HidDevice->Caps.NumberInputButtonCaps;
         i++, data++, buttonCaps++, dataIdx++) 
    {  
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;
        if (buttonCaps->IsRange) 
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Input,
                                                buttonCaps->UsagePage,
                                                HidDevice->Ppd);
        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }


    //LOGD("[hid.cpp][filldevInfo][input] fill value data");
    //
    // Fill in the value data
    //

    for (i = 0; i < HidDevice->Caps.NumberInputValueCaps ; i++, valueCaps++)
    {
        if (valueCaps->IsRange) 
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++) 
            {
                if(dataIdx >= (HidDevice->InputDataLength))
                {
                    return (FALSE); // error case
                }
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
                dataIdx++;
            }
        } 
        else
        {
            if(dataIdx >= (HidDevice->InputDataLength))
            {
                return (FALSE); // error case
            }        
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
            dataIdx++;
        }
    }

    //LOGD("[hid.cpp][filldevInfo][output] prepare ouput data buffer");
    //
    // setup Output Data buffers.
    //

    HidDevice->OutputReportBuffer = (PCHAR)
        calloc (HidDevice->Caps.OutputReportByteLength, sizeof (CHAR));

    HidDevice->OutputButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc (HidDevice->Caps.NumberOutputButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
    {
        free(HidDevice->OutputReportBuffer);
        HidDevice->OutputReportBuffer = NULL;
        free(buttonCaps);
        HidDevice->OutputButtonCaps = NULL;
        return (FALSE);
    }    

    HidDevice->OutputValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc (HidDevice->Caps.NumberOutputValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
    {
        free(valueCaps);
        HidDevice->OutputValueCaps = NULL;
        return (FALSE);
    }

    numCaps = HidDevice->Caps.NumberOutputButtonCaps;
    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetButtonCaps (HidP_Output,
                            buttonCaps,
                            &numCaps,
                            HidDevice->Ppd)))
        {
            return(FALSE);
        }
    }

    numCaps = HidDevice->Caps.NumberOutputValueCaps;
    if(numCaps > 0)
        {
        if(HIDP_STATUS_SUCCESS != (HidP_GetValueCaps (HidP_Output,
                           valueCaps,
                           &numCaps,
                           HidDevice->Ppd)))
        {
            return(FALSE);
        }
    }

    numValues = 0;
    for (i = 0; i < HidDevice->Caps.NumberOutputValueCaps; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            numValues += valueCaps->Range.UsageMax
                       - valueCaps->Range.UsageMin + 1;
        } 
        else
        {
            numValues++;
        }
    }
    valueCaps = HidDevice->OutputValueCaps;

    HidDevice->OutputDataLength = HidDevice->Caps.NumberOutputButtonCaps
                                + numValues;

    HidDevice->OutputData = data = (PHID_DATA)
       calloc (HidDevice->OutputDataLength, sizeof (HID_DATA));

    if (NULL == data)
    {
        free(data);
        HidDevice->OutputData = NULL;
        return (FALSE);
    }

    //LOGD("[hid.cpp][filldevInfo][output] fill button data");
    for (i = 0;
         i < HidDevice->Caps.NumberOutputButtonCaps;
         i++, data++, buttonCaps++) 
    {
        if (i >= HidDevice->OutputDataLength)
        {
            return (FALSE);
        }


        if ( HidDevice->Caps.NumberOutputValueCaps > 0 )
        {
            if(FAILED(ULongAdd((HidDevice->Caps).NumberOutputButtonCaps ,
                (valueCaps->Range).UsageMax, &tmpSum))) 
            {
                return(FALSE);
            }        

            if((valueCaps->Range).UsageMin == tmpSum)
            {
                return (FALSE);
            }
        }

        
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange)
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }

        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                   HidP_Output,
                                                   buttonCaps->UsagePage,
                                                   HidDevice->Ppd);

        data->ButtonData.Usages = (PUSAGE)
            calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }

    //LOGD("[hid.cpp][filldevInfo][output] fill value data");
    for (i = 0; i < HidDevice->Caps.NumberOutputValueCaps ; i++, valueCaps++)
    {
        if (valueCaps->IsRange)
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++) 
            {
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
            }
        }
        else
        {
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
        }
    }

    //LOGD("[hid.cpp][filldevInfo][feature] prepare feature data buffer");
    //
    // setup Feature Data buffers.
    //

    HidDevice->FeatureReportBuffer = (PCHAR)
           calloc (HidDevice->Caps.FeatureReportByteLength, sizeof (CHAR));

    HidDevice->FeatureButtonCaps = buttonCaps = (PHIDP_BUTTON_CAPS)
        calloc (HidDevice->Caps.NumberFeatureButtonCaps, sizeof (HIDP_BUTTON_CAPS));

    if (NULL == buttonCaps)
    {
        free(HidDevice->FeatureReportBuffer);
        HidDevice->FeatureReportBuffer = NULL;
        free(buttonCaps);
        HidDevice->FeatureButtonCaps = NULL;
        return (FALSE);
    }

    HidDevice->FeatureValueCaps = valueCaps = (PHIDP_VALUE_CAPS)
        calloc (HidDevice->Caps.NumberFeatureValueCaps, sizeof (HIDP_VALUE_CAPS));

    if (NULL == valueCaps)
    {
        free(HidDevice->FeatureReportBuffer);
        HidDevice->FeatureReportBuffer = NULL;
        free(buttonCaps);
        HidDevice->FeatureButtonCaps = NULL;
        free(valueCaps);
        HidDevice->FeatureValueCaps = NULL;
        return (FALSE);
    }

    numCaps = HidDevice->Caps.NumberFeatureButtonCaps;
    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetButtonCaps (HidP_Feature,
                            buttonCaps,
                            &numCaps,
                            HidDevice->Ppd)))
        {
            return(FALSE);
        }
    }

    numCaps = HidDevice->Caps.NumberFeatureValueCaps;
    if(numCaps > 0)
    {
        if(HIDP_STATUS_SUCCESS != (HidP_GetValueCaps (HidP_Feature,
                           valueCaps,
                           &numCaps,
                           HidDevice->Ppd)))
        {
            return(FALSE);
        }
    }

    numValues = 0;
    for (i = 0; i < HidDevice->Caps.NumberFeatureValueCaps; i++, valueCaps++) 
    {
        if (valueCaps->IsRange) 
        {
            numValues += valueCaps->Range.UsageMax
                       - valueCaps->Range.UsageMin + 1;
        }
        else
        {
            numValues++;
        }
    }
    valueCaps = HidDevice->FeatureValueCaps;

    if(FAILED(ULongAdd(HidDevice->Caps.NumberFeatureButtonCaps,
                                 numValues, &newFeatureDataLength))) 
    {
        return(FALSE);
    }

    HidDevice->FeatureDataLength = newFeatureDataLength;

    HidDevice->FeatureData = data = (PHID_DATA)
        calloc (HidDevice->FeatureDataLength, sizeof (HID_DATA));

    if (NULL == data)
    {
        free(data);
        HidDevice->FeatureData = NULL;
        return (FALSE);
    }

    //LOGD("[hid.cpp][filldevInfo][feature] prepare button data");
    dataIdx = 0;
    for (i = 0;
         i < HidDevice->Caps.NumberFeatureButtonCaps;
         i++, data++, buttonCaps++, dataIdx++) 
    {
        data->IsButtonData = TRUE;
        data->Status = HIDP_STATUS_SUCCESS;
        data->UsagePage = buttonCaps->UsagePage;

        if (buttonCaps->IsRange)
        {
            data->ButtonData.UsageMin = buttonCaps -> Range.UsageMin;
            data->ButtonData.UsageMax = buttonCaps -> Range.UsageMax;
        }
        else
        {
            data -> ButtonData.UsageMin = data -> ButtonData.UsageMax = buttonCaps -> NotRange.Usage;
        }
        
        data->ButtonData.MaxUsageLength = HidP_MaxUsageListLength (
                                                HidP_Feature,
                                                buttonCaps->UsagePage,
                                                HidDevice->Ppd);
        data->ButtonData.Usages = (PUSAGE)
             calloc (data->ButtonData.MaxUsageLength, sizeof (USAGE));

        data->ReportID = buttonCaps -> ReportID;
    }

    //LOGD("[hid.cpp][filldevInfo][feature] prepare value data");
    for (i = 0; i < HidDevice->Caps.NumberFeatureValueCaps ; i++, valueCaps++) 
    {
        if (valueCaps->IsRange)
        {
            for (usage = valueCaps->Range.UsageMin;
                 usage <= valueCaps->Range.UsageMax;
                 usage++)
            {
                if(dataIdx >= (HidDevice->FeatureDataLength))
                {
                    return (FALSE); // error case
                }
                data->IsButtonData = FALSE;
                data->Status = HIDP_STATUS_SUCCESS;
                data->UsagePage = valueCaps->UsagePage;
                data->ValueData.Usage = usage;
                data->ReportID = valueCaps -> ReportID;
                data++;
                dataIdx++;
            }
        } 
        else
        {
            if(dataIdx >= (HidDevice->FeatureDataLength))
            {
                return (FALSE); // error case
            }
            data->IsButtonData = FALSE;
            data->Status = HIDP_STATUS_SUCCESS;
            data->UsagePage = valueCaps->UsagePage;
            data->ValueData.Usage = valueCaps->NotRange.Usage;
            data->ReportID = valueCaps -> ReportID;
            data++;
            dataIdx++;
        }
    }

    return (TRUE);
}

VOID
CloseHidDevices(
    IN  PHID_DEVICE HidDevices,
    IN  ULONG       NumberDevices
)
{
    ULONG   Index;

    for (Index = 0; Index < NumberDevices; Index++) 
    {
        CloseHidDevice(HidDevices+Index);
    }

    return;
}

VOID
CloseHidDevice (
    IN PHID_DEVICE HidDevice
)
{


    for( unsigned int i = 0; i < HidDevice->InputDataLength; i++ )
    {
        if ( HidDevice->InputData[i].ButtonData.Usages != NULL )
        {
            free( HidDevice->InputData[i].ButtonData.Usages );
        }
    }

    for( unsigned int i = 0; i < HidDevice->OutputDataLength; i++ )
    {
        if ( HidDevice->OutputData[i].ButtonData.Usages != NULL )
        {
            free( HidDevice->OutputData[i].ButtonData.Usages );
        }
    }

    for( unsigned int i = 0; i < HidDevice->FeatureDataLength; i++ )
    {
        if ( HidDevice->FeatureData[i].ButtonData.Usages != NULL )
        {
            free( HidDevice->FeatureData[i].ButtonData.Usages );
        }
    }





    if (NULL != HidDevice -> DevicePath)
    {
        free(HidDevice -> DevicePath);
        HidDevice -> DevicePath = NULL;
    }

    if (INVALID_HANDLE_VALUE != HidDevice -> HidDevice)
    {
        CloseHandle(HidDevice -> HidDevice);
        HidDevice -> HidDevice = INVALID_HANDLE_VALUE;
    }
    
    if (NULL != HidDevice -> Ppd)
    {
        HidD_FreePreparsedData(HidDevice -> Ppd);
        HidDevice -> Ppd = NULL;
    }

    if (NULL != HidDevice -> InputReportBuffer)
    {
        free(HidDevice -> InputReportBuffer);
        HidDevice -> InputReportBuffer = NULL;
    }

    if (NULL != HidDevice -> InputData)
    {
        free(HidDevice -> InputData);
        HidDevice -> InputData = NULL;
    }

    if (NULL != HidDevice -> InputButtonCaps)
    {
        free(HidDevice -> InputButtonCaps);
        HidDevice -> InputButtonCaps = NULL;
    }

    if (NULL != HidDevice -> InputValueCaps)
    {
        free(HidDevice -> InputValueCaps);
        HidDevice -> InputValueCaps = NULL;
    }

    if (NULL != HidDevice -> OutputReportBuffer)
    {
        free(HidDevice -> OutputReportBuffer);
        HidDevice -> OutputReportBuffer = NULL;
    }

    if (NULL != HidDevice -> OutputData)
    {
        free(HidDevice -> OutputData);
        HidDevice -> OutputData = NULL;
    }

    if (NULL != HidDevice -> OutputButtonCaps) 
    {
        free(HidDevice -> OutputButtonCaps);
        HidDevice -> OutputButtonCaps = NULL;
    }

    if (NULL != HidDevice -> OutputValueCaps)
    {
        free(HidDevice -> OutputValueCaps);
        HidDevice -> OutputValueCaps = NULL;
    }

    if (NULL != HidDevice -> FeatureReportBuffer)
    {
        free(HidDevice -> FeatureReportBuffer);
        HidDevice -> FeatureReportBuffer = NULL;
    }

    if (NULL != HidDevice -> FeatureData) 
    {
        free(HidDevice -> FeatureData);
        HidDevice -> FeatureData = NULL;
    }

    if (NULL != HidDevice -> FeatureButtonCaps) 
    {
        free(HidDevice -> FeatureButtonCaps);
        HidDevice -> FeatureButtonCaps = NULL;
    }

    if (NULL != HidDevice -> FeatureValueCaps) 
    {
        free(HidDevice -> FeatureValueCaps);
        HidDevice -> FeatureValueCaps = NULL;
    }

     return;
}

