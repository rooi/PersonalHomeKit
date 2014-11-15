//
//  PHKNetworkIP.h
//  Workbench
//
//  Created by Wai Man Chan on 4/8/14.
//
//

#ifndef __Workbench__PHKNetworkIP__
#define __Workbench__PHKNetworkIP__

#include "PHKAccessory.h"

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dns_sd.h>
#include <cstring>
#include <string>
using namespace std;

#define IPv4 1
#define IPv6 0

class PHKConnection {
public:
    PHKConnection(Devices* devices_) : socketNumber(-1), devices(devices_) {};
    
    static void * connectionLoopFunc(void * This) {((PHKConnection *)This)->connectionLoop(); return NULL;}
    void* connectionLoop() const;
    
    void handlePairSetup(int subSocket, char *buffer) const;
    void handlePairVerify(int subSocket, char *buffer) const;
    
    pthread_t thread;
    int socketNumber;
    
    Devices* devices;
};

void broadcastMessage(char *buffer, size_t len);

class PHKNetworkIP {
    void setupSocket(DeviceStruct* device);
    TXTRecordRef buildTXTRecord(DeviceStruct* device);
    
    Devices* devices;

public:
    PHKNetworkIP(Devices* devices);
    void handleConnection();
    static vector<PHKConnection> connections;
    ~PHKNetworkIP();
};

class PHKNetworkMessageDataRecord {
public:
    unsigned char index = 0;
    char *data = 0;
    unsigned int length = 0;
    bool activate = false;
    ~PHKNetworkMessageDataRecord();
    PHKNetworkMessageDataRecord &operator=(const PHKNetworkMessageDataRecord&);
};

class PHKNetworkMessageData {
    PHKNetworkMessageDataRecord records[10];
    unsigned char count = 0;
public:
    PHKNetworkMessageData() {}
    PHKNetworkMessageData(const char *rawData, unsigned short len);
    PHKNetworkMessageData(const PHKNetworkMessageData &data);
    PHKNetworkMessageData &operator=(const PHKNetworkMessageData &);
    void rawData(const char **dataPtr, unsigned short *len);
    void addRecord(PHKNetworkMessageDataRecord& record);
    int recordIndex(unsigned char index);
    char *dataPtrForIndex(unsigned char index);
    unsigned int lengthForIndex(unsigned char index);
};

class PHKNetworkMessage {
    char method[5];
    char type[40];
public:
    char directory[20];
    PHKNetworkMessageData data;
    PHKNetworkMessage(const char *rawData);
    void getBinaryPtr(char **buffer, int *contentLength);
};

class PHKNetworkResponse {
    unsigned short responseCode;
    string responseType();
public:
    PHKNetworkMessageData data;
    PHKNetworkResponse(unsigned short _responseCode);
    void getBinaryPtr(char **buffer, int *contentLength);
};

#endif /* defined(__Workbench__PHKNetworkIP__) */
