/*
 * This accessory.cpp is configurated for light accessory
 */

#include "Accessory.h"

#include "PHKAccessory.h"

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

void initAccessorySet() {
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
    printf(lightwaverfConfig.c_str());
#endif // HomeKitLog
    
    printf("Initial Accessory\n");
    accSet = new AccessorySet();
    Accessory *lightAcc = new Accessory();
    addInfoServiceToAccessory(lightAcc, "Light 1", "ET", "Light", "12345678");
    accSet->addAccessory(lightAcc);
    
    Service *lightService = new Service(charType_lightBulb);
    lightAcc->addService(lightService);
    
    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    lightServiceName->setValue("Light");
    lightAcc->addCharacteristics(lightService, lightServiceName);
    
    boolCharacteristics *powerState = new boolCharacteristics(charType_on, premission_read|premission_write);
    powerState->setValue("Light");
    lightAcc->addCharacteristics(lightService, powerState);
    
    intCharacteristics *brightnessState = new intCharacteristics(charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
    brightnessState->setValue("Light");
    lightAcc->addCharacteristics(lightService, brightnessState);
};