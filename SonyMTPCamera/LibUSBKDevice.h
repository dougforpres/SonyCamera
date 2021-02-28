#pragma once
#include "pch.h"
#include "Device.h"
#include "libusbk/libusbk.h"
#include "PTPContainer.h"

// 130MB
#define MAX_BULK_RX_BUFFER_BYTES (130*1024*1024)

class LibUSBKDevice : public Device
{
public:
    LibUSBKDevice(KLST_DEVINFO_HANDLE deviceInfo, std::wstring DeviceId);
    LibUSBKDevice(const LibUSBKDevice& rhs);

    ~LibUSBKDevice();

//    virtual Device* Clone();

    virtual HANDLE Open();
    virtual bool Close();

    virtual bool Send(Message* out);
    virtual Message* Receive(Message* out);

    virtual std::wstring GetRegistryPath();
    virtual bool NeedsSession();

protected:
    virtual bool StartNotifications();
    virtual bool StopNotifications();

private:
//    static DWORD WINAPI _runBulkRx(LPVOID lpParameter);
    static DWORD WINAPI _runInterruptRx(LPVOID lpParameter);

    bool Tx(PTPContainer::Type type, DWORD command, DWORD dataLen, BYTE* data);

//    DWORD RunBulkRx();
    DWORD RunInterruptRx();

//    HANDLE m_hBulkRxThread = INVALID_HANDLE_VALUE;
    HANDLE m_hInterruptRxThread = INVALID_HANDLE_VALUE;
//    DWORD m_bulkRxThreadId = 0;
    DWORD m_interruptRxThreadId = 0;

    bool m_stopRx = false;
//    std::list<Message*> m_bulkRxQueue;
    std::list<DWORD> m_interruptRxQueue;
//    HANDLE m_bulkRxEvent = INVALID_HANDLE_VALUE;

    KUSB_HANDLE m_interfaceHandle = INVALID_HANDLE_VALUE;

    int m_vid = 0;  // Vendor ID
    int m_pid = 0;  // Product ID

    static unsigned long m_sequence;
    unsigned char m_inputPipe = 0;
    unsigned char m_outputPipe = 0;
    unsigned char m_interruptPipe = 0;
};