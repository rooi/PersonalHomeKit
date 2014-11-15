//
//  main.cpp
//  Personal_HomeKit
//
//  Created by Wai Man Chan on 4/8/14.
//
//

#include <iostream>
#include <fstream>
#include "PHKNetworkIP.h"
#include "Accessory.h"
extern "C" {
#include "PHKArduinoLightInterface.h"
}

using namespace std;

int main(int argc, const char * argv[]) {
    
    Devices devices;
    
    DeviceStruct device1;
    device1.name = "Funnel";
    device1.identity = "12:00:54:23:51:13";
    device1.uuid = "9FCF7180-6CAA-4174-ABC0-E3FAE58E3ADD";
    
    DeviceStruct device2;
    device2.name = "Greggs";
    device2.identity = "12:00:54:23:52:14";
    device2.uuid = "9FCF7180-6CAA-4174-ABC0-E3FAE58E3ADD";
    
    devices.push_back(device1);
    devices.push_back(device2);
    
    // insert code here...
    initAccessorySet(&devices);
    //setupPort();
    
    PHKNetworkIP networkIP(&devices);
    do {
        networkIP.handleConnection();
    } while (true);
    return 0;
}
