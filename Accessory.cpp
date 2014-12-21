/*
 * This accessory.cpp is configurated for light accessory
 */

#include "Accessory.h"

#include "PHKAccessory.h"

#include "Configuration.h"

#include <fstream>
#include <queue>
#include <pthread.h>
#include <sstream>

//Global Level of light strength
int lightStength = 0;
int fanSpeedVal = 0;


AccessorySet *accSet;

void lightIdentify(bool oldValue, bool newValue) {
    printf("Start Identify Light\n");
}

void fanIdentify(bool oldValue, bool newValue) {
    printf("Start Identify Fan\n");
}

static string GetStdoutFromCommand(string cmd) {
    
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

class LightwaveRFCommandQueue {
    queue<string> commands;
    pthread_t thread;
    static pthread_mutex_t lock;
    static bool done;
    static void* process(void* ptr) {
        queue<string>* cmds = (queue<string>*)ptr;
        while(cmds->size()) {
            pthread_mutex_lock(&lock);
            string cmd = cmds->front();
            cmds->pop();
            pthread_mutex_unlock(&lock);
#if HomeKitLog == 1
            printf(cmd.c_str()); printf("\n");
#endif
            GetStdoutFromCommand(cmd);
            sleep(1);
        }
#if HomeKitLog == 1
        printf("Setting processing thread to done\n");
#endif
        pthread_mutex_lock(&lock);
        done = true;
        pthread_mutex_unlock(&lock);
#if HomeKitLog == 1
        printf("Finished processing commands\n");
#endif
    }
public:
    LightwaveRFCommandQueue() {
        if (pthread_mutex_init(&lock, NULL) != 0)
        {
            printf("\n mutex init failed\n");
        }
    }
    virtual ~LightwaveRFCommandQueue() {
        pthread_mutex_destroy(&lock);
    }
    void addCommand(string cmd) {
        pthread_mutex_lock(&lock);
#if HomeKitLog == 1
        printf(cmd.c_str()); printf("\n");
#endif
        commands.push(cmd);
        if(done) {
            done = false;
#if HomeKitLog == 1
            printf("Creating command thread\n");
#endif
            pthread_create(&this->thread, NULL, LightwaveRFCommandQueue::process, &commands);
        }
        pthread_mutex_unlock(&lock);
    }
};
bool LightwaveRFCommandQueue::done = true;
pthread_mutex_t LightwaveRFCommandQueue::lock;

LightwaveRFCommandQueue lightwaveRFCommandQueue;

class lightwaveRFPowerState: public boolCharacteristics {
    string _room;
    string _name;
public:
    lightwaveRFPowerState(string room, string name, unsigned short _type, int _premission): boolCharacteristics(_type, _premission), _room(room), _name(name) {}
    string value() {
        if (lightStength > 0)
            return "1";
        return "0";
    }
    void setValue(string str) {
        this->boolCharacteristics::setValue(str);
        string cmd = "lightwaverf ";
        cmd += "\""; cmd += _room; cmd += "\" ";
        cmd += "\""; cmd += _name; cmd += "\" ";
        if (_value) {
            lightStength = 255;
            setLightStrength(255);
            cmd += "on";
        } else {
            lightStength = 0;
            setLightStrength(0);
            cmd += "off";
        }
        //system(cmd.c_str());
        lightwaveRFCommandQueue.addCommand(cmd);
#if HomeKitLog == 1
        string msg = "Setting ";
        msg += _room; msg += " ";
        msg += _name; msg += " ";
        msg += "light power state\n";
        printf(msg.c_str());
#endif // HomeKitLog
    }
};

class lightwaveRFBrightness: public intCharacteristics {
    string _room;
    string _name;
public:
    lightwaveRFBrightness(string room, string name, unsigned short _type, int _premission, int minVal, int maxVal, int step, unit charUnit):intCharacteristics(_type, _premission, minVal, maxVal, step, charUnit), _room(room), _name(name) {}
    void setValue(string str) {
        this->intCharacteristics::setValue(str);
        lightStength = _value;
        setLightStrength(2.55*_value);
        
        string cmd = "lightwaverf ";
        cmd += "\""; cmd += _room; cmd += "\" ";
        cmd += "\""; cmd += _name; cmd += "\" ";
        //cmd += std::to_string(_value);
        std::ostringstream ostr;
        ostr << _value;
        cmd += ostr.str();
        //system(cmd.c_str());
        lightwaveRFCommandQueue.addCommand(cmd);
#if HomeKitLog == 1
        string msg = "Setting ";
        msg += _room; msg += " ";
        msg += _name; msg += " ";
        msg += "light strenght\n";
        printf(msg.c_str());
#endif // HomeKitLog
    }
};

Accessory* CreateLightWaveRFAccessory(string room, string name, string type)
{
    Accessory *lightAcc = new Accessory();
    
    string model = "Light";
    if(type == "D") model = "Dimmer";
    
    string serial = "123" + name + "4";
    
    addInfoServiceToAccessory(lightAcc, name.c_str(), "KlikAanKlikUit", model.c_str(), serial.c_str(), &lightIdentify);

    Service *lightService = new Service(charType_lightBulb);
    lightAcc->addService(lightService);

    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, premission_read, 0);
    lightServiceName->setValue(name.c_str());
    lightAcc->addCharacteristics(lightService, lightServiceName);

    lightwaveRFPowerState *powerState = new lightwaveRFPowerState(room, name, charType_on, premission_read|premission_write);
    //powerState->setValue("LightSwitch");
    lightAcc->addCharacteristics(lightService, powerState);

    if(model == "Dimmer") {
        lightwaveRFBrightness *brightnessState = new lightwaveRFBrightness(room, name, charType_brightness, premission_read|premission_write, 0, 100, 1, unit_percentage);
        //brightnessState->setValue("LightDimmer");
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
    ifstream myfile (lightwaveLoginFile);
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
    sleep(1);
    
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
#if HomeKitLog == 1
        //printf(devicesString.c_str()); printf("\n");
#endif
        
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
#if HomeKitLog == 1
            printf("Device: name = "); printf(thisDeviceName.c_str());
            printf(" type = "); printf(thisDeviceType.c_str()); printf("\n");
#endif
            
            // Create the accessory
            if(!thisDeviceName.empty()) {
                accSet->addAccessory(CreateLightWaveRFAccessory(roomName, thisDeviceName, thisDeviceType));
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
#if HomeKitLog == 1
                printf("Room = "); printf(roomName.c_str()); printf("\n");
#endif
            }
        }
        
        // Find next devices in this room
        devicesBegin = lightwaverfConfig.find("\"device\"=>[");
        devicesEnd = lightwaverfConfig.find("]}");
    }
};
