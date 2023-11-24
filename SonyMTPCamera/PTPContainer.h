#pragma once
#include <windows.h>

constexpr auto CONTAINER_TYPE_COMMAND = 0x0001;
constexpr auto CONTAINER_TYPE_DATA = 0x0002;
constexpr auto CONTAINER_TYPE_RESPONSE = 0x0003;

constexpr auto OPCODE_GET_DEVICE_INFO = 0x1001;
constexpr auto OPCODE_OPEN_SESSION = 0x1002;
constexpr auto OPCODE_CLOSE_SESSION = 0x1003;

typedef struct {
    unsigned long containerLength;
    unsigned short containerType;
    unsigned short opcode;
    unsigned long sequenceNumber;
} PTPHeader, *pPTPHeader;

class PTPContainer
{
public:
    enum class Type {
        Init = CONTAINER_TYPE_COMMAND,
        Data = CONTAINER_TYPE_DATA,
        Response = CONTAINER_TYPE_RESPONSE,
    };

    PTPContainer(PTPContainer::Type containerType, unsigned short opcode, unsigned long transactionId, unsigned long dataLen = 0, BYTE *data = NULL);
    PTPContainer(unsigned long dataLen, BYTE *data);
    ~PTPContainer();

    unsigned long toBytes(unsigned long bufferLen, BYTE *buffer);
    void setData(unsigned long dataLen, BYTE *data);
    unsigned long getDataLen() const;
    BYTE *getData();
    Type getContainerType() const;
    unsigned long getSequenceNumber() const;
    unsigned short getOpCode() const;

    std::wstring Dump();

protected:
    Type _containerType;
    unsigned short _opcode = 0;
    unsigned long _sequenceNumber = 0;
    BYTE *_data = nullptr;
    unsigned long _dataLen = 0;

private:
};
