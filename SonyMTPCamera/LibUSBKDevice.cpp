#include "pch.h"
#include "LibUSBKDevice.h"
#include "PTPContainer.h"
#include "Logger.h"
#include "Utils.h"

#define THREAD_WAIT_EXIT_SLEEP 100
#define THREAD_WAIT_EXIT_LOOPS 50

#define INTERRUPT_TIMEOUT 45000
#define READ_TIMEOUT 10000

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
                    unsigned int interruptTimeout = INTERRUPT_TIMEOUT;;
                    unsigned int readTimeout = READ_TIMEOUT;

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
                                UsbK_SetPipePolicy(m_interfaceHandle, pipeInformation.PipeId, PIPE_TRANSFER_TIMEOUT, sizeof(unsigned int), &readTimeout);
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
                                UsbK_SetPipePolicy(m_interfaceHandle, pipeInformation.PipeId, PIPE_TRANSFER_TIMEOUT, sizeof(unsigned int), &interruptTimeout);
                            }
                            break;

                        default:
                            LOGERROR(L"Unknown pipe type: x%02x", (int)pipeInformation.PipeType);
                            break;
                        }
                    }

                    m_openCount++;
                    m_handle = CreateEvent(nullptr, false, false, nullptr);
                    m_hInterruptRxThread = CreateThread(NULL, 0, &_runInterruptRx, this, 0, &m_interruptRxThreadId);

                    LOGINFO(L"x%p - Opened() with handle %p", (void*)this, (DWORD)m_handle);
                }
                else
                {
                    LOGERROR(L"! Failed to Open the device, error = %d", GetLastError());
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

                GetExitCodeThread(m_hInterruptRxThread, &exitCode);

                waitCount = 0;

                while (exitCode == STILL_ACTIVE && waitCount < THREAD_WAIT_EXIT_LOOPS)
                {
                    waitCount++;

                    LOGINFO(L"Waiting for interrupt thread to exit %d / %d loops", waitCount, THREAD_WAIT_EXIT_LOOPS);

                    Sleep(THREAD_WAIT_EXIT_SLEEP);
                    GetExitCodeThread(m_hInterruptRxThread, &exitCode);
                }

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

    if (paramCount > 0)
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

    // Every command has multiple phases
    // The sent data (above) is phase 1 (Init)
    // If the command is a "sendy" one (we are sending data to the device, phase 2 (Data) is done here, otherwise we expect phase 2 to be data
    // reveived from the device.
    // The device then always sends a phase 3 (Response)
    // If there is no data to return, the device can skip phase 2
    bool rxDone = false;
    BYTE* rxBuffer = new BYTE[MAX_BULK_RX_BUFFER_BYTES];
    unsigned int transferred = 0;
    Message* rxMessage = nullptr;

    while (!rxDone)
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
                LOGINFO(L"Rx DATA x%04x (%d bytes)", container->getOpCode(), transferred);
                rxMessage = new Message(container->getOpCode());
                rxMessage->SetData(container->getData(), container->getDataLen());
                break;

            case PTPContainer::Type::Response:
                LOGINFO(L"Rx RESPONSE x%04x (%d bytes)", container->getOpCode(), transferred);
                if (!rxMessage)
                {
                    LOGINFO(L"No prior data message, creating dummy one");
                    rxMessage = new Message(container->getOpCode());
                }

                rxMessage->SetCommand(container->getOpCode());
                rxDone = true;

#ifdef DEBUG
                LOGINFO(L"Received message %s", rxMessage->Dump().c_str());
#endif
                break;

            default:
                LOGINFO(L"Got unknown response type: %d", container->getContainerType());
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
                LOGINFO(L"Got Shutdown notification");
                break;

            default:
                LOGWARN(L"UsbK_ReadPipe returned error %d", lastError);
                break;
            }

            rxDone = true;
        }
    }

    delete[] rxBuffer;

    return rxMessage;
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
LibUSBKDevice::RunInterruptRx()
{
    BYTE* rxBuffer = new BYTE[MAX_BULK_RX_BUFFER_BYTES];
    unsigned int transferred = 0;

    while (!m_stopRx)
    {
        if (UsbK_ReadPipe(m_interfaceHandle, m_interruptPipe, rxBuffer, MAX_BULK_RX_BUFFER_BYTES, &transferred, nullptr))
        {
            LOGINFO(L"Rx interrupt");
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
    bool result;

#ifdef DEBUG
    LOGINFO(L"Sending: %s", container.Dump().c_str());
#endif

    container.toBytes(txDataLen, txData);

#ifdef DEBUG
    LOGINFO(L"Bytes being sent = %s", DumpBytes(txData, txDataLen).c_str());
#endif

    if (UsbK_WritePipe(m_interfaceHandle, m_outputPipe, txData, txDataLen, &transferred, nullptr))
    {
        LOGTRACE(L"Tx REQUEST x%04x (%d bytes)", command, transferred);
        result = true;
    }
    else
    {
        LOGERROR(L"Unable to send command x%04x data - error %d", command, GetLastError());
        result = false;
    }

    delete[] txData;

    return result;
}