#include "Accessory.h"

#include "PHKAccessory.h"
#include "PHKLightWaveRFAccessory.h"


class lightService: public Service {
    stringCharacteristics serviceName;
    lightPowerState powerState;
    lightBrightness brightness;
public:
    lightService(int index, string deviceName): Service(index, charType_lightBulb),
    serviceName(index+1, charType_serviceName, premission_read, 0), powerState(index+2), brightness(index+3)
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



//For bridge, create more than one subclass, and insert in main accessory
//Also change the MainAccessorySet
class MainAccessory: public Accessory {
    infoService info;
    lightService light;
public:
    MainAccessory(int aid, DeviceStruct device): Accessory(aid),
    info(1, device), light(info.serviceID+info.numberOfCharacteristics()+1, device.name) {}
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

//For bridge, change the subject to dynamic assign
class MainAccessorySet: public AccessorySet {
    std::vector<LightWaveRFAccessory*> accessories;
public:
    MainAccessorySet(Devices* devices) {
        for(unsigned int i=0;i<devices->size();i++) {
            int aid = i+1;
            accessories.push_back(new LightWaveRFAccessory(aid, devices->at(i), "WoonKamer"));
        }
    }
    virtual ~MainAccessorySet() {
        while(accessories.size()) {
            delete accessories.at(accessories.size()-1);
            accessories.pop_back();
        }
    }
    short numberOfAccessory() { return accessories.size(); }
    Accessory * accessoryAtIndex(int index) { return accessories.at(index); }
};

AccessorySet *accSet;

void initAccessorySet(Devices* devices) {
    printf("Initial Accessory\n");
    accSet = new MainAccessorySet(devices);
};
