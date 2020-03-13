#pragma once

#include <windows.h>

#include "hidapi.h"

namespace USB
{

// Connection error codes
enum Error
{
    OK = 1,
    TIMEOUT = 2,
    DISCONNECTED = 3,
};

class Connector
{
public:
    static const UINT16 VID = 0x0483;
    static const UINT16 PID = 0x5750;
    static const int INTERFACE_INDEX = 0;
    // time in ms.
    static const int TIMEOUT = 1000;
    static const int MAX_PACK_SIZE = 64;

    static const UINT8 READY_PACK = 0x11;
    static const UINT8 CPU_GPU_PACK = 0x22;
    static const UINT8 ACK_PACK = 0x33;
    static const UINT8 DATA_PACK = 0x44;
    static const UINT8 UPGRED_PACK = 0x55;
    static const UINT8 UPGRED_READY_PACK = 0x66;
    static const UINT8 UPGRED_INFORM_PACK = 0x77;
    static const UINT8 UPGRED_DATA_PACK = 0x88;
    static const UINT8 UPGRED_STATUS_PACK = 0X99;

public:
    Connector();
    ~Connector();

    bool Connect();
    Error Read(BYTE *buf, int bufLen, int *actualLen);
    Error Write(BYTE *buf, int len);

private:
    hid_device *dev;

    std::runtime_error error(const char *msg);
};



class Filer 
{
public:
    UINT8 Buffer[128 * 1024];
    UINT8 Index;
    UINT8 Size;
    UINT8 Checksum;


public:
    Filer();
    ~Filer();
    bool getBin(const char* path);

};




} // namespace USB




