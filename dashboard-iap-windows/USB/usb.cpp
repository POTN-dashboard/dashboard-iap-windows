#include <cstdio>
#include <stdexcept>
#include <string>
#include <comdef.h>
#include <iostream>

#include "usb.hpp"

#pragma comment(lib, "hidapi.lib")

using namespace USB;

std::runtime_error USB::Connector::error(const char *msg)
{
    std::string errorStr = "[USB] ";
    errorStr += msg;
    if (NULL != dev)
    {
        _bstr_t error(hid_error(dev));
        const char *errorMsg = error;
        errorStr += ": ";
        errorStr += errorMsg;
    }
    return std::runtime_error(errorStr);
}

USB::Connector::Connector() : dev(NULL)   //初始化列表， 字符串
{
    int res = hid_init();
    if (0 != res)
    {
        throw error("Init hidapi fail");
    }
}

USB::Connector::~Connector()
{
    if (NULL != dev)
    {
        hid_close(dev);
    }
    hid_exit();
    dev = NULL;
}

bool USB::Connector::Connect()
{
    hid_device_info *devs = hid_enumerate(VID, PID);
    hid_device_info *curDev = devs;
    char path[6400];
    path[0] = 0;
    while (NULL != curDev)
    {                        //windows中，只有一个接口时，接口编号的变量无效，为-1
        if (INTERFACE_INDEX == curDev->interface_number || -1 == curDev->interface_number) 
        {
            strcpy_s(path, curDev->path);
            break;
        }   
        curDev = curDev->next;
    }
    if (0 == path[0])
    {
        hid_free_enumeration(devs);
        return false;
    }
    dev = hid_open_path(path);
    if (NULL == dev)
    {
        throw error("Open device fail");
    }
    hid_free_enumeration(devs);
    return true;
}

Error USB::Connector::Read(BYTE *buf, int bufLen, int *actualLen)
{
    int res = hid_read_timeout(dev, buf, bufLen, TIMEOUT);
    if (0 == res)                       // 超时 返回0
    {
        return Error::TIMEOUT;
    }
    if (0 < res)                //正常 返回 读到包的 字节数
    {
        *actualLen = res;
        // printf("[USB] Read %d bytes\n", res);
        return Error::OK;
    }
    wprintf(L"[USB] Read fail: %ls\n", hid_error(dev));         //出错返回-1
    return Error::DISCONNECTED;
}

Error USB::Connector::Write(BYTE *buf, int len)
{
    BYTE writeBuf[MAX_PACK_SIZE + 1];
    writeBuf[0] = 0;
    memcpy(writeBuf + 1, buf, len);
    int res = hid_write(dev, writeBuf, (size_t)len + 1);
    if (-1 == res)
    {
        wprintf(L"[USB] Write fail: %ls\n", hid_error(dev));
        return Error::DISCONNECTED;
    }
    // wprintf(L"[USB] Write %d bytes\n", res);
    return Error::OK;
}


USB::Filer::Filer()
{
    memset(Buffer, 0, sizeof(Buffer));
    Index = 0;
    Size = 0;
    Checksum = 0;
    PackNum = 1;
}


USB::Filer::~Filer()
{

}

bool Filer::getBin(const char* path)
{
    FILE* fp = NULL;
    errno_t err = 0;

    memset(Buffer, 0, sizeof(Buffer));          //每次读取文件都重新对类的变量 初始化
    Index = 0;
    Size = 0;
    Checksum = 0;
    PackNum = 1;

    if ((err = fopen_s(&fp, path, "rb")) != 0) {      //二进制可读打开
        puts( "make sure the bin file are the same place with exe " );
        system("pause");       // DOS调用 黑框框不会闪退 
        exit(0);
        return false;
    }
    if (fp != 0) {                  //文件打开成功
        fseek(fp, 0L, SEEK_END);
        Size = ftell(fp);    //获取文件大小
        rewind(fp);		//重新恢复位置指针的位置，回到文件的开头
        printf("this bin is total %d bytes\n", Size);
        fread(Buffer, 1, Size, fp);

        for(UINT32 i = 0; i < Size; i++){
            Checksum += Buffer[i];          //计算校验和
        }
        Checksum = ~Checksum + 1;       // 取补码
        fclose(fp);
    }
    return true;

}