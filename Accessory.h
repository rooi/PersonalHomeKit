//
//  PHKAccessory.h
//  Workbench
//
//  Created by Wai Man Chan on 9/27/14.
//
//

#ifndef __Workbench__Accessory__
#define __Workbench__Accessory__

#include "PHKAccessory.h"

class lightPowerState: public boolCharacteristics {
public:
    lightPowerState(int index, int* lightStength_=NULL)
    : boolCharacteristics(index, charType_on, premission_read|premission_write),
    lightStength(lightStength_) {}
    string value() {
        if (lightStength > 0)
            return "1";
        return "0";
    }
    void setValue(string str) {
        this->boolCharacteristics::setValue(str);
        if (_value) {
            if(lightStength) *lightStength = 1;
            setLightStrength(255);
        } else {
            if(lightStength) *lightStength = 0;
            setLightStrength(0);
        }
    }
private:
    int* lightStength;
};

class lightBrightness: public intCharacteristics {
public:
    lightBrightness(int index, int* lightStength_=NULL)
    : intCharacteristics(index, charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage),
    lightStength(lightStength_) {}
    void setValue(string str) {
        this->intCharacteristics::setValue(str);
        if(lightStength) *lightStength = _value;
        setLightStrength(2.55*_value);
    }
private:
    int* lightStength;
};

void initAccessorySet(Devices* devices);

#endif /* defined(__Workbench__Accessory__) */
