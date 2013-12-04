/** -----------------------------
Created by Angelo Scialabba
email: scialabba.angelo@gmail.com
On 04/12/2013
-------------------------------**/

stDeviceListItem* List()
{
    HDEVINFO DevInfoHandle;
    SP_DEVINFO_DATA DevInfoData;
    int i,foundEntries,k;
    GUID arduinoguid;
    stDeviceListItem* deviceList,*current;
    DWORD friendlyNameLenght,hardwareIdLenght;
    char VID[10],PID[10];
    friendlyNameLenght = 13;
    hardwareIdLenght = 13;


    //creating the Arduino GUID object
    arduinoguid.Data1=0x4D36E978;
    arduinoguid.Data2=0xE325;
    arduinoguid.Data3=0x11CE;
    arduinoguid.Data4[0]=0xBF;
    arduinoguid.Data4[1]=0xC1;
    arduinoguid.Data4[2]=0x08;
    arduinoguid.Data4[3]=0x00;
    arduinoguid.Data4[4]=0x2B;
    arduinoguid.Data4[5]=0xE1;
    arduinoguid.Data4[6]=0x03;
    arduinoguid.Data4[7]=0x18;

    DevInfoHandle = SetupDiGetClassDevs(&arduinoguid,
                                   NULL,
                                   0,
                                   DIGCF_PRESENT);

    if (DevInfoHandle == INVALID_HANDLE_VALUE)
    {
        printf("Error: invalid handle value\n");
        return NULL;
    }

    deviceList = NULL;
    foundEntries = 0;

    DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0; SetupDiEnumDeviceInfo(DevInfoHandle,i,
                                    &DevInfoData); i++)
    {
        DWORD DataT;
        char friendlyName[128];
        char hardwareId[128];
        DWORD buffersize = 128;
        //get the Displayed name and COM port
        while (!SetupDiGetDeviceRegistryProperty(
                    DevInfoHandle,
                    &DevInfoData,
                    SPDRP_FRIENDLYNAME ,
                    &DataT,
                    (PBYTE)friendlyName,
                    buffersize,
                    &friendlyNameLenght))
        {
            if (GetLastError() ==
                    ERROR_INSUFFICIENT_BUFFER)
            {
                printf("Error: Insufficent buffer\n");
                break;
            }
            else
            {
                printf("Error: Unknown\n");
                break;
            }

        }
        //get the Hardware ID string
        while (!SetupDiGetDeviceRegistryProperty(
                    DevInfoHandle,
                    &DevInfoData,
                    SPDRP_HARDWAREID ,
                    &DataT,
                    (PBYTE)hardwareId,
                    buffersize,
                    &hardwareIdLenght))
        {
            if (GetLastError() ==
                    ERROR_INSUFFICIENT_BUFFER)
            {
                printf("Error: Insufficent buffer\n");
                return NULL;
                break;
            }
            else
            {
                return NULL;
                printf("Error: Unknown\n");
                break;
            }
        }

        //create the list or create the next list item
        if (deviceList == NULL){

            deviceList = (stDeviceListItem*)malloc(sizeof(stDeviceListItem));
            deviceList->next = NULL;
            current = deviceList;
            foundEntries++;

        } else {

            current->next = (stDeviceListItem*)malloc(sizeof(stDeviceListItem));
            current = current->next;
            current->next = NULL;
            foundEntries++;
        }

        //parse strings to get vid and pid
        k = 0;
        while (hardwareId[k] != '\0')
        {

            if (hardwareId[k] == '\\')
            {
                strncpy(VID,&hardwareId[k+1],8);
                VID[8] = '\0';


            }

            if (hardwareId[k] == '&')
            {
                strncpy(PID,&hardwareId[k+1],8);
                PID[8] = '\0';
                break;

            }

            k++;


        }

        //copy properties to the list item
        if (current != NULL) {

            strcpy(current->value.port,friendlyName);
            strcpy(current->value.vendorId,VID);
            strcpy(current->value.productId,PID);

        } else {

            exit(-1);

        }


    }
	//put the list size in the first element
    if (deviceList != NULL) {

        deviceList->length = (int*)malloc(sizeof(int));
        *(deviceList->length) = foundEntries;

    }
    SetupDiDestroyDeviceInfoList(DevInfoHandle);
    return deviceList;
}
