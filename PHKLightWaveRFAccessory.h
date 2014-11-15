//
//  PHKAccessory.h
//  Workbench
//
//  Created by Roy Arents on 11/15/14.
//
//
#ifndef __Workbench__PHKLightWaveRFAccessory__
#define __Workbench__PHKLightWaveRFAccessory__

#include "Accessory.h"

class infoServiceLightWaveRF: public Service {
    stringCharacteristics name;
    stringCharacteristics manufactuer;
    stringCharacteristics modelName;
    stringCharacteristics serialNumber;
    identifyCharacteristics identify;
    string room;
public:
    infoServiceLightWaveRF(int index, DeviceStruct device, string room_): Service(index, charType_accessoryInfo),
    name(index+1, charType_serviceName, premission_read, 0),
    manufactuer(index+2, charType_manufactuer, premission_read, 0),
    modelName(index+3, charType_modelName, premission_read, 0),
    serialNumber(index+4, charType_serialNumber, premission_read, 0),
    identify(index+5), room(room) {
        name.setValue(device.nameAsChar());
        manufactuer.setValue(manufactuerName);
        modelName.setValue(device.nameAsChar());
        serialNumber.setValue(device.uuidAsChar());
    }
    virtual short numberOfCharacteristics() { return 5; }
    virtual characteristics *characteristicsAtIndex(int index) {
        switch (index-1-serviceID) {
            case 0:
                return &name;
            case 1:
                return &manufactuer;
            case 2:
                return &modelName;
            case 3:
                return &serialNumber;
            case 4:
                return &identify;
        }
        return 0;
    }
};

class lightServiceLightWaveRF: public Service {
    stringCharacteristics serviceName;
    lightPowerState powerState;
    lightBrightness brightness;
    int lightStength;
    string room;
public:
    lightServiceLightWaveRF(int index, string deviceName, string room_)
     : Service(index, charType_lightBulb),
       serviceName(index+1, charType_serviceName, premission_read, 0),
       powerState(index+2, &lightStength),
       brightness(index+3, &lightStength),
       room(room_)
    {
        serviceName.setValue(deviceName);
        powerState.setValue("false");
    }
    inline virtual short numberOfCharacteristics() { return 3; }
    inline virtual characteristics *characteristicsAtIndex(int index) {
        switch (index-1-serviceID) {
            case 0:
                return &serviceName;
            case 1:
                return &powerState;
            case 2:
                return &brightness;
        }
        return 0;
    }
};

class LightWaveRFAccessory: public Accessory {
    infoServiceLightWaveRF info;
    lightServiceLightWaveRF light;
public:
    LightWaveRFAccessory(int aid, DeviceStruct device, string room): Accessory(aid),
    info(1, device, room), light(info.serviceID+info.numberOfCharacteristics()+1, device.name, room) {}
    inline virtual short numberOfService() { return 2; }
    inline virtual Service *serviceAtIndex(int index) {
        switch (index-1) {
            case 0:
                return &info;
            case 1:
                return &light;
        }
        return 0;
    }
};

#endif /* __Workbench__PHKLightWaveRFAccessory__ */
