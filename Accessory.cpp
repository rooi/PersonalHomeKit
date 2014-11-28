/*
 * This accessory.cpp is configurated for light accessory
 */

#include "Accessory.h"

#include "PHKAccessory.h"

#include "Configuration.h"

#include <fstream>

//Global Level of light strength
int lightStength = 0;
int fanSpeedVal = 0;


AccessorySet *accSet;

string GetStdoutFromCommand(string cmd) {
    
    string data;
    FILE * stream;
    const int max_buffer = 256;
    char buffer[max_buffer];
    cmd.append(" 2>&1");
    
    stream = popen(cmd.c_str(), "r");
    if (stream) {
        while (!feof(stream))
            if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
    return data;
}

Accessory* CreateLightWaveRFAccessory(string name, string type)
{
    Accessory *lightAcc = new Accessory();
    
    string model = "Light";
    if(type == "D") model = "Dimmer";
    
    addInfoServiceToAccessory(lightAcc, name.c_str(), "KlikAanKlikUit", model.c_str(), "12345678");

    Service *lightService = new Service(charType_lightBulb);
    lightAcc->addService(lightService);

    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    lightServiceName->setValue(name.c_str());
    lightAcc->addCharacteristics(lightService, lightServiceName);

    boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write);
    powerState->setValue("LightSwitch");
    lightAcc->addCharacteristics(lightService, powerState);

    if(model == "Dimmer") {
        intCharacteristics *brightnessState = new intCharacteristics(charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
        brightnessState->setValue("LightDimmer");
        lightAcc->addCharacteristics(lightService, brightnessState);
    }
    
    return lightAcc;
}

void initAccessorySet() {
    printf("Initial Accessory\n");
    accSet = new AccessorySet();
    
    // Read in lightwaverf login details
    string line;
    string usernameLRF = "";
    string passwordLRF = "";
    ifstream myfile ("../lightwaverflogin.txt");
    if (myfile.is_open())
    {
        int lineNr = 0;
        while ( getline (myfile,line) )
        {
            lineNr++;
            if(lineNr == 1) usernameLRF = line;
            else if(lineNr == 2) passwordLRF = line;
        }
        myfile.close();
    }
    else{
        printf("Unable to open file: ../lightwaverflogin.txt\n");
        return;
    }
    
#if HomeKitLog == 1
    printf(usernameLRF.c_str()); printf("\n");
    printf(passwordLRF.c_str()); printf("\n");
#endif // HomeKitLog
    
    string lightwaverfCmd = "lightwaverf update ";
    lightwaverfCmd += usernameLRF;
    lightwaverfCmd += " ";
    lightwaverfCmd += passwordLRF;
#if HomeKitLog == 1
    printf(lightwaverfCmd.c_str()); printf("\n");
#endif // HomeKitLog
    
    //system(lightwaverfCmd.c_str());
    string lightwaverfConfig = GetStdoutFromCommand(lightwaverfCmd);
    
#if HomeKitLog == 1
    printf(lightwaverfConfig.c_str()); printf("\n");
#endif // HomeKitLog
    
    // Find room
    size_t roomBegin = lightwaverfConfig.find("{\"room\"=>[{\"name\"==>\"") + 21;
    string roomName = "";
    if(roomBegin!=string::npos) roomName = lightwaverfConfig.substr(roomBegin);
    size_t roomEnd = roomName.find("\"");
    if(roomEnd!=string::npos) roomName = roomName.substr(0,roomEnd);
    printf("Room = "); printf(roomName.c_str()); printf("\n");
    
    size_t devicesBegin = lightwaverfConfig.find("\"device\"=>[") + 11;
    size_t devicesEnd = lightwaverfConfig.find("]}, ");
    int rooms = 0;
    while( devicesBegin!=string::npos && devicesEnd!=string::npos && rooms < 100) {
        rooms++;
        string devicesString = lightwaverfConfig.substr(devicesBegin,devicesEnd-devicesBegin);
        printf(devicesString.c_str()); printf("\n");
        
        // Find devices in this room
        size_t deviceBegin = devicesString.find("{");
        size_t deviceEnd = devicesString.find("}");
        int devicesInRoom = 0;
        while (deviceBegin!=string::npos && devicesEnd!=string::npos && devicesInRoom < 100) {
            devicesInRoom++;
            string deviceString = devicesString.substr(deviceBegin, deviceEnd-deviceBegin);
            
            string thisDeviceName = "";
            size_t deviceNameBegin = deviceString.find("\"name\"=>\"");
            if(deviceNameBegin!=string::npos) thisDeviceName = deviceString.substr(deviceNameBegin + 9);
            size_t deviceNameEnd = thisDeviceName.find("\"");
            if(deviceNameEnd!=string::npos) thisDeviceName = thisDeviceName.substr(0,deviceNameEnd);
            
            string thisDeviceType = "";
            size_t deviceTypeBegin = deviceString.find("\"type\"=>\"");
            if(deviceTypeBegin!=string::npos) thisDeviceType = deviceString.substr(deviceTypeBegin + 9);
            size_t deviceTypeEnd = thisDeviceType.find("\"");
            if(deviceTypeEnd!=string::npos) thisDeviceType = thisDeviceType.substr(0,deviceTypeEnd);
            
            printf("Device: name = "); printf(thisDeviceName.c_str());
            printf(" type = "); printf(thisDeviceType.c_str()); printf("\n");
            
            // Create the accessory
            if(!thisDeviceName.empty()) {
                accSet->addAccessory(CreateLightWaveRFAccessory(thisDeviceName, thisDeviceType));
            }
            
            // Next device
            devicesString = devicesString.substr(deviceEnd + 1);
            deviceBegin = devicesString.find("{");
            deviceEnd = devicesString.find("}");
        }
        
        
        // Find next room
        lightwaverfConfig = lightwaverfConfig.substr(devicesEnd+4);
        
        roomBegin = lightwaverfConfig.find("{\"name\"=>\"");
        if(roomBegin!= string::npos) {
            roomName = lightwaverfConfig.substr(roomBegin + 10);
            roomEnd = roomName.find("\"");
            if(roomEnd!=string::npos) {
                roomName = roomName.substr(0,roomEnd);
                printf("Room = "); printf(roomName.c_str()); printf("\n");
            }
        }
        
        // Find next devices in this room
        devicesBegin = lightwaverfConfig.find("\"device\"=>[");
        devicesEnd = lightwaverfConfig.find("]}");
        
    }
};