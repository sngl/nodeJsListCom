//
//  listCom.c
//  listCom
//
//  Created by Angelo Scialabba on 11/13/13.
//  

#include <stdio.h>
#include <AvailabilityMacros.h>
#include <sys/param.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/serial/IOSerialKeys.h>

typedef struct SerialDevice {
    char port[MAXPATHLEN];
    char locationId[MAXPATHLEN];
    char vendorId[MAXPATHLEN];
    char productId[MAXPATHLEN];
    char manufacturer[MAXPATHLEN];
    char serialNumber[MAXPATHLEN];
} stSerialDevice;


typedef struct DeviceListItem {
    struct SerialDevice value;
    struct DeviceListItem *next;
    int* length;
} stDeviceListItem;





static kern_return_t FindModems(io_iterator_t *matchingServices)
{
    kern_return_t kernResult;
    CFMutableDictionaryRef classesToMatch;
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch != NULL)
    {
        CFDictionarySetValue(classesToMatch,
                             CFSTR(kIOSerialBSDTypeKey),
                             CFSTR(kIOSerialBSDAllTypes));
    }
    
    kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, matchingServices);
    
    return kernResult;
}

static void ExtractUsbInformation(stSerialDevice *serialDevice, IOUSBDeviceInterface **deviceInterface)
{
    kern_return_t kernResult;
    UInt32 locationID;
    kernResult = (*deviceInterface)->GetLocationID(deviceInterface, &locationID);
    if (KERN_SUCCESS == kernResult)
    {
        snprintf(serialDevice->locationId, 11, "0x%08x", locationID);
    }
    
    UInt16 vendorID;
    kernResult = (*deviceInterface)->GetDeviceVendor(deviceInterface, &vendorID);
    if (KERN_SUCCESS == kernResult)
    {
        snprintf(serialDevice->vendorId, 7, "0x%04x", vendorID);
    }
    
    UInt16 productID;
    kernResult = (*deviceInterface)->GetDeviceProduct(deviceInterface, &productID);
    if (KERN_SUCCESS == kernResult)
    {
        snprintf(serialDevice->productId, 7, "0x%04x", productID);
    }
    
    
}

static io_registry_entry_t GetUsbDevice(char* pathName)
{
    io_registry_entry_t device = 0;
    
    CFMutableDictionaryRef classesToMatch = IOServiceMatching(kIOUSBDeviceClassName);
    if (classesToMatch != NULL)
    {
        io_iterator_t matchingServices;
        kern_return_t kernResult = IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, &matchingServices);
        if (KERN_SUCCESS == kernResult)
        {
            io_service_t service;
            Boolean deviceFound = false;
            
            while ((service = IOIteratorNext(matchingServices)) && !deviceFound)
            {
                CFStringRef bsdPathAsCFString = (CFStringRef) IORegistryEntrySearchCFProperty(service, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, kIORegistryIterateRecursively);
                
                if (bsdPathAsCFString)
                {
                    Boolean result;
                    char bsdPath[MAXPATHLEN];
                    
                    // Convert the path from a CFString to a C (NUL-terminated)
                    result = CFStringGetCString(bsdPathAsCFString,
                                                bsdPath,
                                                sizeof(bsdPath),
                                                kCFStringEncodingUTF8);
                    
                    CFRelease(bsdPathAsCFString);
                    
                    if (result && (strcmp(bsdPath, pathName) == 0))
                    {
                        deviceFound = true;
                        //memset(bsdPath, 0, sizeof(bsdPath));
                        device = service;
                    }
                    else
                    {
                        // Release the object which are no longer needed
                        (void) IOObjectRelease(service);
                    }
                }
            }
            // Release the iterator.
            IOObjectRelease(matchingServices);
        }
    }
    
    return device;
}


stDeviceListItem* GetSerialDevices()
{
    kern_return_t kernResult;
    io_iterator_t serialPortIterator;
    char bsdPath[MAXPATHLEN];
    
    FindModems(&serialPortIterator);
    
    io_service_t modemService;
    kernResult = KERN_FAILURE;
    Boolean modemFound = false;
    
    // Initialize the returned path
    *bsdPath = '\0';
    
    stDeviceListItem* devices = NULL;
    stDeviceListItem* lastDevice = NULL;
    int length = 0;
    
    while ((modemService = IOIteratorNext(serialPortIterator)))
    {
        CFTypeRef bsdPathAsCFString;
        
        bsdPathAsCFString = IORegistryEntrySearchCFProperty(modemService, kIOServicePlane, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, kIORegistryIterateRecursively);
        
        if (bsdPathAsCFString)
        {
            Boolean result;
            
            // Convert the path from a CFString to a C (NUL-terminated)
            
            result = CFStringGetCString((CFStringRef) bsdPathAsCFString,
                                        bsdPath,
                                        sizeof(bsdPath),
                                        kCFStringEncodingUTF8);
            CFRelease(bsdPathAsCFString);
            
            //printf("Result %d\n",result);
            if (result)
            {
                stDeviceListItem *deviceListItem = (stDeviceListItem*) malloc(sizeof(stDeviceListItem));
                stSerialDevice *serialDevice = &(deviceListItem->value);
                strcpy(serialDevice->port, bsdPath);
                //printf("Path %s\n",bsdPath);
                memset(serialDevice->locationId, 0, sizeof(serialDevice->locationId));
                memset(serialDevice->vendorId, 0, sizeof(serialDevice->vendorId));
                memset(serialDevice->productId, 0, sizeof(serialDevice->productId));
                serialDevice->manufacturer[0] = '\0';
                serialDevice->serialNumber[0] = '\0';
                deviceListItem->next = NULL;
                deviceListItem->length = &length;
                
                
                io_registry_entry_t device = GetUsbDevice(bsdPath);
                
                if (1) {
                    
                    if (devices == NULL) {
                        devices = deviceListItem;
                    }
                    else {
                        lastDevice->next = deviceListItem;
                    }
                    
                    lastDevice = deviceListItem;
                    length++;
                    
                    modemFound = true;
                    kernResult = KERN_SUCCESS;
                    
                } else free(deviceListItem);
                
                
                
                
                if (device) {
                    CFStringRef manufacturerAsCFString = (CFStringRef) IORegistryEntrySearchCFProperty(device,
                                                                                                       kIOServicePlane,
                                                                                                       CFSTR(kUSBVendorString),
                                                                                                       kCFAllocatorDefault,
                                                                                                       kIORegistryIterateRecursively);
                    
                    if (manufacturerAsCFString)
                    {
                        Boolean result;
                        char manufacturer[MAXPATHLEN];
                        
                        // Convert from a CFString to a C (NUL-terminated)
                        result = CFStringGetCString(manufacturerAsCFString,
                                                    manufacturer,
                                                    sizeof(manufacturer),
                                                    kCFStringEncodingUTF8);
                        
                        if (result) {
                            strcpy(serialDevice->manufacturer, manufacturer);
                        }
                        
                        CFRelease(manufacturerAsCFString);
                    }
                    
                    CFStringRef serialNumberAsCFString = (CFStringRef) IORegistryEntrySearchCFProperty(device,
                                                                                                       kIOServicePlane,
                                                                                                       CFSTR(kUSBSerialNumberString),
                                                                                                       kCFAllocatorDefault,
                                                                                                       kIORegistryIterateRecursively);
                    
                    if (serialNumberAsCFString)
                    {
                        Boolean result;
                        char serialNumber[MAXPATHLEN];
                        
                        // Convert from a CFString to a C (NUL-terminated)
                        result = CFStringGetCString(serialNumberAsCFString,
                                                    serialNumber,
                                                    sizeof(serialNumber),
                                                    kCFStringEncodingUTF8);
                        
                        if (result) {
                            strcpy(serialDevice->serialNumber, serialNumber);
                        }
                        
                        CFRelease(serialNumberAsCFString);
                    }
                    
                    IOCFPlugInInterface **plugInInterface = NULL;
                    SInt32 score;
                    HRESULT res;
                    
                    IOUSBDeviceInterface **deviceInterface = NULL;
                    
                    kernResult = IOCreatePlugInInterfaceForService(device, kIOUSBDeviceUserClientTypeID, kIOCFPlugInInterfaceID,
                                                                   &plugInInterface, &score);
                    
                    if ((kIOReturnSuccess != kernResult) || !plugInInterface) {
                        continue;
                    }
                    
                    // Use the plugin interface to retrieve the device interface.
                    res = (*plugInInterface)->QueryInterface(plugInInterface, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                                                             (LPVOID*) &deviceInterface);
                    
                    // Now done with the plugin interface.
                    (*plugInInterface)->Release(plugInInterface);
                    
                    if (res || deviceInterface == NULL) {
                        continue;
                    }
                    
                    // Extract the desired Information
                    ExtractUsbInformation(serialDevice, deviceInterface);
                    
                    
                    
                    
                    // Release the Interface
                    (*deviceInterface)->Release(deviceInterface);
                    
                    // Release the device
                    (void) IOObjectRelease(device);
                }
                
                
                
                //uv_mutex_unlock(&list_mutex);
            }
        }
        
        // Release the io_service_t now that we are done with it.
        (void) IOObjectRelease(modemService);
    }
    
    IOObjectRelease(serialPortIterator); // Release the iterator.
    
    return devices;
}
