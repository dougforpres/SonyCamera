#pragma once
#include <windows.h>

#define CONTAINER_TYPE_COMMAND  0x0001
#define CONTAINER_TYPE_DATA     0x0002
#define CONTAINER_TYPE_RESPONSE 0x0003

#define OPCODE_GET_DEVICE_INFO 0x1001
#define OPCODE_OPEN_SESSION    0x1002
#define OPCODE_CLOSE_SESSION   0x1003

typedef struct {
    unsigned long containerLength;
    unsigned short containerType;
    unsigned short opcode;
    unsigned long sequenceNumber;
} PTPHeader, *pPTPHeader;

class PTPContainer
{
public:
    PTPContainer(unsigned short containerType, unsigned short opcode, unsigned long transactionId, unsigned long dataLen = 0, BYTE *data = NULL);
    PTPContainer(unsigned long dataLen, BYTE *data);
    ~PTPContainer();

    unsigned long toBytes(unsigned long bufferLen, BYTE *buffer);
    void setData(unsigned long dataLen, BYTE *data);
    unsigned long getDataLen();
    BYTE *getData();

protected:
    unsigned short _containerType;
    unsigned short _opcode;
    unsigned long _sequenceNumber;
    BYTE *_data;
    unsigned long _dataLen;

private:
};

