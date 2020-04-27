#include "pch.h"
#include "PTPContainer.h"

/*
   Constructor
   Creates a container that will typically be used to send data to a device
 */
PTPContainer::PTPContainer(unsigned short containerType, unsigned short opcode, unsigned long sequenceNumber, unsigned long dataLen, BYTE * data)
    : _containerType(containerType),
      _opcode(opcode),
      _sequenceNumber(sequenceNumber)
{
    this->setData(dataLen, data);
}

/*
   Constructor
   Creates a container, usually as a result of received data from device
   various properties are extracted from the header of the data, and the remaining data is then saved
 */
PTPContainer::PTPContainer(unsigned long dataLen, BYTE * data)
{
    // PTP Packet must be minimum of 12 bytes long, any shorter and
    // we should throw an error
    if (dataLen >= 12 && data)
    {
        pPTPHeader header = (pPTPHeader)data;
        this->_containerType = header->containerType;
        this->_opcode = header->opcode;
        this->_sequenceNumber = header->sequenceNumber;

        this->setData(header->containerLength - sizeof(PTPHeader), data + sizeof(PTPHeader));
    }
    else
    {
//        throw new ...
    }
}

PTPContainer::~PTPContainer()
{
    this->setData(0, NULL);
}

/*
   Call with 0/NULL and will return size of buffer required
 */
unsigned long PTPContainer::toBytes(unsigned long bufferLen, BYTE * buffer)
{
    unsigned long containerLen = sizeof(PTPHeader) + this->_dataLen;

    if (bufferLen && buffer)
    {
        pPTPHeader header = (pPTPHeader)buffer;

        header->containerLength = sizeof(PTPHeader) + this->_dataLen;
        header->containerType = this->_containerType;
        header->opcode = this->_opcode;
        header->sequenceNumber = this->_sequenceNumber;

        if (this->_dataLen)
        {
            memcpy(buffer + sizeof(pPTPHeader), this->_data, this->_dataLen);
        }
    }

    return containerLen;
}

void PTPContainer::setData(unsigned long dataLen, BYTE * data)
{
    // We need to get rid of existing data first
    if (this->_data)
    {
        delete this->_data;
        this->_data = NULL;
        this->_dataLen = 0;
    }

    if (dataLen && data)
    {
        this->_data = new BYTE[dataLen];
        this->_dataLen = dataLen;
        memcpy(this->_data, data, dataLen);
    }
}

unsigned long PTPContainer::getDataLen()
{
    return this->_dataLen;
}

BYTE * PTPContainer::getData()
{
    return this->_data;
}
