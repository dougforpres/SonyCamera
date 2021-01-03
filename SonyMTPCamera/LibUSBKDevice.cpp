#include "pch.h"
#include "LibUSBKDevice.h"
#include "PTPContainer.h"
#include "Logger.h"
#include "Utils.h"

#define THREAD_WAIT_EXIT_SLEEP 100
#define THREAD_WAIT_EXIT_LOOPS 50

unsigned long LibUSBKDevice::m_sequence = 0;

LibUSBKDevice::LibUSBKDevice(KLST_DEVINFO_HANDLE deviceInfo, std::wstring DeviceId)
    : Device(DeviceId)
{
    std::wostringstream builder;

    builder << deviceInfo->Mfg << L' ' << deviceInfo->DeviceDesc;
    m_description = builder.str();

    builder.str(L"");
    builder << deviceInfo->DeviceDesc;
    m_friendlyName = builder.str();

    builder.str(L"");
    builder << deviceInfo->Mfg;
    m_manufacturer = builder.str();

    m_vid = deviceInfo->Common.Vid;
    m_pid = deviceInfo->Common.Pid;
}

LibUSBKDevice::LibUSBKDevice(const LibUSBKDevice& rhs)
    : Device(rhs),
      m_vid(rhs.m_vid),
      m_pid(rhs.m_pid)
{
}

LibUSBKDevice::~LibUSBKDevice()
{
}

Device*
LibUSBKDevice::Clone()
{
    return new LibUSBKDevice(*this);
}

bool
LibUSBKDevice::NeedsSession()
{
    return true;
}

HANDLE
LibUSBKDevice::Open()
{
    LOGTRACE(L"In: LibUSBKDevice::Open");

    if (m_openCount == 0)
    {
        LOGTRACE(L"  First time opened, doing some setup");

        m_bulkRxEvent = CreateEvent(NULL, true, false, L"LibUSBK Bulk Receive Wakeup Event");

        // Re-find the device
        KLST_HANDLE listHandle;

        if (LstK_Init(&listHandle, KLST_FLAG_NONE))
        {
            KLST_DEVINFO_HANDLE deviceInfo;

            if (LstK_FindByVidPid(listHandle, m_vid, m_pid, &deviceInfo))
            {
                if (UsbK_Init(&m_interfaceHandle, deviceInfo))
                {
                    WINUSB_PIPE_INFORMATION pipeInformation;

                    for (int pipeId = 0; UsbK_QueryPipe(m_interfaceHandle, 0, pipeId, &pipeInformation); pipeId++)
                    {
                        switch (pipeInformation.PipeType)
                        {
                        case UsbdPipeTypeBulk:
                            if (pipeInformation.PipeId & 0x80)
                            {
                                m_inputPipe = pipeInformation.PipeId;
                                LOGINFO(L"Input pipe = x%02x", m_inputPipe);

                                // Set up a timeout
                                unsigned int timeout = 30000;
                                UsbK_SetPipePolicy(m_interfaceHandle, pipeInformation.PipeId, PIPE_TRANSFER_TIMEOUT, sizeof(unsigned int), &timeout);
                            }
                            else
                            {
                                m_outputPipe = pipeInformation.PipeId;
                                LOGINFO(L"Output pipe = x%02x", m_outputPipe);
                            }
                            break;

                        case UsbdPipeTypeInterrupt:
                            m_interruptPipe = pipeInformation.PipeId;
                            LOGINFO(L"Interrupt pipe = x%02x", m_interruptPipe);

                            {
                                // Set up a timeout
                                unsigned int timeout = 30000;
                                UsbK_SetPipePolicy(m_interfaceHandle, pipeInformation.PipeId, PIPE_TRANSFER_TIMEOUT, sizeof(unsigned int), &timeout);
                            }
                            break;

                        default:
                            LOGERROR(L"Unknown pipe type: x%02x", (int)pipeInformation.PipeType);
                        }
                    }

                    m_openCount++;
                    m_handle = CreateEvent(nullptr, false, false, nullptr);
                    m_hBulkRxThread = CreateThread(NULL, 0, &_runBulkRx, this, 0, &m_bulkRxThreadId);
                    m_hInterruptRxThread = CreateThread(NULL, 0, &_runInterruptRx, this, 0, &m_interruptRxThreadId);

                    LOGINFO(L"x%p - Opened() with handle %p", (void*)this, (DWORD)m_handle);
                }
                else
                {
                    LOGERROR(L"! Failed to Open the device, error = %d", GetLastError());
                    // Release the IPortableDevice interface, because we cannot proceed
                    // with an unopen device.
                }

                LstK_Free(listHandle);
                StartNotifications();
            }
        }
    }
    else
    {
        m_openCount++;

        LOGTRACE(L"Incrementing OpenCount to %d", m_openCount);
    }

    LOGTRACE(L"Out: LibUSBKDevice::Open - now open %d times with handle x%08x", m_openCount, (DWORD)m_handle);

    return m_handle;
}

bool
LibUSBKDevice::Close()
{
    LOGTRACE(L"In: LibUSBKDevice::Close");

    bool closed = false;

    if (m_openCount)
    {
        if (m_openCount == 1)
        {
            StopNotifications();

            if (UsbK_Free(m_interfaceHandle))
            {
                m_openCount--;

                CloseHandle(m_handle);
                m_handle = INVALID_HANDLE_VALUE;

                LOGTRACE(L"LibUSBKDevice::Close - Closed()");

                m_stopRx = true;

                UsbK_AbortPipe(m_interfaceHandle, m_inputPipe);
                UsbK_AbortPipe(m_interfaceHandle, m_interruptPipe);

                // Wait for exit
                DWORD exitCode = 0;
                DWORD waitCount = 0;

                GetExitCodeThread(m_hBulkRxThread, &exitCode);

                while (exitCode == STILL_ACTIVE && waitCount < THREAD_WAIT_EXIT_LOOPS)
                {
                    waitCount++;

                    LOGINFO(L"Waiting for thread to exit %d / %d loops", waitCount, THREAD_WAIT_EXIT_LOOPS);

                    Sleep(THREAD_WAIT_EXIT_SLEEP);
                    GetExitCodeThread(m_hBulkRxThread, &exitCode);
                }

                GetExitCodeThread(m_hInterruptRxThread, &exitCode);

                waitCount = 0;

                while (exitCode == STILL_ACTIVE && waitCount < THREAD_WAIT_EXIT_LOOPS)
                {
                    waitCount++;

                    LOGINFO(L"Waiting for interrupt thread to exit %d / %d loops", waitCount, THREAD_WAIT_EXIT_LOOPS);

                    Sleep(THREAD_WAIT_EXIT_SLEEP);
                    GetExitCodeThread(m_hInterruptRxThread, &exitCode);
                }

                CloseHandle(m_bulkRxEvent);

                closed = true;
            }
        }
        else
        {
            m_openCount--;

            LOGTRACE(L"LibUSBKDevice::Close - Still open, opencount reduced to %d", m_openCount);

            closed = false;
        }
    }

    LOGWARN(L"Out: LibUSBK::Close (result = %d)", closed);

    return closed;
}

bool
LibUSBKDevice::StartNotifications()
{
    return false;
}

bool
LibUSBKDevice::StopNotifications()
{
    return false;
}

std::wstring
LibUSBKDevice::GetRegistryPath()
{
    std::wostringstream builder;

    builder << L"VID_" << std::hex << std::uppercase << std::setw(4) << std::setfill(L'0') << m_vid << L"&PID_" << std::hex << std::uppercase << std::setw(4) << std::setfill(L'0') << m_pid;

    return builder.str();
}

bool
LibUSBKDevice::Send(Message* out)
{
    Receive(out);

    return true;
}

Message*
LibUSBKDevice::Receive(Message* out)
{
    m_sequence++;

    PTPContainer* container = nullptr;

    int paramCount = out->GetParams().size();
    DWORD dl = paramCount * sizeof(unsigned long);

    if (dl > 0)
    {
        unsigned long* params = new unsigned long[paramCount];
        int index = 0;
        std::list<DWORD> lp = out->GetParams();

        for (std::list<unsigned long>::iterator it = lp.begin(); it != lp.end(); it++)
        {
            params[index] = (unsigned long)*it;
            index++;
        }

        Tx(PTPContainer::Type::Init, out->GetCommand(), paramCount * sizeof(long), (BYTE*)params);
    }
    else
    {
        Tx(PTPContainer::Type::Init, out->GetCommand(), 0, nullptr);
    }

    if (out->GetDataLen())
    {
        Tx(PTPContainer::Type::Data, out->GetCommand(), out->GetDataLen(), out->GetData());
    }


    DWORD waitResult = WaitForSingleObject(m_bulkRxEvent, 5000);
    Message* rxMessage = nullptr;

    if (waitResult == WAIT_OBJECT_0) {
        rxMessage = m_bulkRxQueue.front();
        m_bulkRxQueue.pop_front();
        ResetEvent(m_bulkRxEvent);
    }

    return rxMessage;
}

DWORD WINAPI
LibUSBKDevice::_runBulkRx(LPVOID lpParameter)
{
    LOGTRACE(L"In: LibUSBKDevice::_runBulkRx(x%08x)", lpParameter);

    DWORD result = ((LibUSBKDevice*)lpParameter)->RunBulkRx();

    LOGTRACE(L"Out: LibUSBKDevice::_RunBulkRx(x%08x) - returning %d", lpParameter, result);

    return result;
}

DWORD WINAPI
LibUSBKDevice::_runInterruptRx(LPVOID lpParameter)
{
    LOGTRACE(L"In: LibUSBKDevice::_runInterruptRx(x%08x)", lpParameter);

    DWORD result = ((LibUSBKDevice*)lpParameter)->RunInterruptRx();

    LOGTRACE(L"Out: LibUSBKDevice::_runInterruptRx(x%08x) - returning %d", lpParameter, result);

    return result;
}

DWORD
LibUSBKDevice::RunBulkRx()
{
    BYTE* rxBuffer = new BYTE[MAX_BULK_RX_BUFFER_BYTES];
    unsigned int transferred = 0;
    Message* rxMessage = nullptr;

    while (!m_stopRx)
    {
        if (UsbK_ReadPipe(m_interfaceHandle, m_inputPipe, rxBuffer, MAX_BULK_RX_BUFFER_BYTES, &transferred, nullptr))
        {
            // Got some data
            PTPContainer* container = new PTPContainer(transferred, rxBuffer);

#ifdef DEBUG
            LOGINFO(L"container: %s", container->Dump().c_str());
#endif

            switch (container->getContainerType())
            {
            case PTPContainer::Type::Data:
                LOGINFO(L"Got a data message: command = x%04x", container->getOpCode());
                rxMessage = new Message(container->getOpCode());
                rxMessage->SetData(container->getData(), container->getDataLen());
                break;

            case PTPContainer::Type::Response:
                LOGINFO(L"Got a result: command = x%04x", container->getOpCode());
                if (!rxMessage)
                {
                    LOGINFO(L"No prior data message, creating dummy one");
                    rxMessage = new Message(container->getOpCode());
                }

                rxMessage->SetCommand(container->getOpCode());

                m_bulkRxQueue.push_back(rxMessage);

#ifdef DEBUG
                LOGINFO(L"Received message %s", rxMessage->Dump().c_str());
#endif

                rxMessage = nullptr;

                // Trigger a receive event
                SetEvent(m_bulkRxEvent);
                break;
            }

            delete container;
        }
        else
        {
            DWORD lastError = GetLastError();

            switch (lastError)
            {
            case ERROR_SEM_TIMEOUT:
                // This is expected, it means no data in the timeout window
                break;

            case ERROR_OPERATION_ABORTED:
                // This is expected in shutdown/close scenario
                LOGINFO(L"RunBulkRx: Got Shutdown notification");
                break;

            default:
                LOGWARN(L"RunBulkRx: UsbK_ReadPipe returned error %d", lastError);
                break;
            }
        }
    }

    delete[] rxBuffer;

    return ERROR_SUCCESS;
}

DWORD
LibUSBKDevice::RunInterruptRx()
{
    BYTE* rxBuffer = new BYTE[MAX_BULK_RX_BUFFER_BYTES];
    unsigned int transferred = 0;

    while (!m_stopRx)
    {
        if (UsbK_ReadPipe(m_interfaceHandle, m_interruptPipe, rxBuffer, MAX_BULK_RX_BUFFER_BYTES, &transferred, nullptr))
        {
            LOGINFO(L"Received interrupt");
            // Got some data
            PTPContainer* container = new PTPContainer(transferred, rxBuffer);

#ifdef DEBUG
            LOGINFO(L"Interrupt: %s", container->Dump().c_str());
#endif

            switch (container->getOpCode())
            {
            case 0xc201:
                LOGINFO(L"Image Ready");
                break;

            case 0xc202:
                LOGINFO(L"Image Not Ready");
                break;

            case 0xc203:
                LOGINFO(L"Camera Property Changed");
                break;

            default:
                LOGWARN(L"Got unknown interrupt: x%04x", container->getOpCode());
                break;
            }

            delete container;
        }
        else
        {
            DWORD lastError = GetLastError();

            switch (lastError)
            {
            case ERROR_SEM_TIMEOUT:
                // This is expected, it means no data in the timeout window
                break;

            case ERROR_OPERATION_ABORTED:
                // This is expected in shutdown/close scenario
                LOGINFO(L"RunInterruptRx: Got Shutdown notification");
                break;

            default:
                LOGWARN(L"RunInterruptRx: UsbK_ReadPipe returned error %d", lastError);
                break;
            }
        }
    }

    delete[] rxBuffer;

    return ERROR_SUCCESS;
}

bool
LibUSBKDevice::Tx(PTPContainer::Type type, DWORD command, DWORD dataLen, BYTE* data)
{
    PTPContainer container = PTPContainer(type, command, m_sequence, dataLen, data);
    unsigned int txDataLen = container.toBytes(0, nullptr);
    unsigned int transferred = 0;
    BYTE* txData = new BYTE[txDataLen];

#ifdef DEBUG
    LOGINFO(L"Sending: %s", container.Dump().c_str());
#endif

    container.toBytes(txDataLen, txData);

#ifdef DEBUG
    LOGINFO(L"Bytes being sent = %s", DumpBytes(txData, txDataLen).c_str());
#endif

    if (UsbK_WritePipe(m_interfaceHandle, m_outputPipe, txData, txDataLen, &transferred, nullptr))
    {
        LOGTRACE(L"Sent %d bytes", transferred);
    }
    else
    {
        LOGERROR(L"Unable to send data - error %d", GetLastError());
    }

    delete[] txData;

    return true;
}