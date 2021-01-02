#include "pch.h"
#include "MTPDevice.h"
#include <PortableDevice.h>
#include <WpdMtpExtensions.h>
#include "Logger.h"
#include "Registry.h"

MTPDevice::MTPDevice(IPortableDeviceManager* PortableDeviceManager, std::wstring DeviceId)
    : Device(DeviceId),
      m_manager(PortableDeviceManager)
{
    LOGTRACE(L"In: MTPDevice::MTPDevice(deviceId='%s')", m_id.c_str());

    if (m_manager)
    {
        InitDescription();
        InitFriendlyName();
        InitManufacturer();

        m_manager->AddRef();
    }

    LOGTRACE(L"Out: MTPDevice::MTPDevice(deviceId='%s')", DeviceId.c_str());
}

MTPDevice::MTPDevice(const MTPDevice& rhs)
    : Device(rhs),
      m_manager(rhs.m_manager)
{
    LOGTRACE(L"In: MTPDevice::MTPDevice[copy](deviceId='%s')", m_id.c_str());

    if (m_manager)
    {
        m_manager->AddRef();
    }

    LOGTRACE(L"Out: MTPDevice::MTPDevice[copy](deviceId='%s')", m_id.c_str());
}

Device*
MTPDevice::Clone()
{
    LOGTRACE(L"MTPDevice::Clone of %s", GetFriendlyName().c_str());
    return new MTPDevice(*this);
}

MTPDevice::~MTPDevice()
{
    LOGTRACE(L"In: MTPDevice::~MTPDevice()");

    while (m_openCount > 0)
    {
        Close();
    }

    if (m_clientInformation)
    {
        m_clientInformation->Release();
        m_clientInformation = nullptr;
    }

    if (m_manager)
    {
        m_manager->Release();
    }

    LOGTRACE(L"Out: MTPDevice::~Device()");
}

HANDLE
MTPDevice::Open()
{
    LOGTRACE(L"In: MTPDevice::Open");

    if (m_openCount == 0)
    {
        LOGTRACE(L"  First time opened, doing some setup");

        CreateClientInformation();

        LOGTRACE(L"Opening PortableDevice via COM");

        HRESULT hr = CoCreateInstance(CLSID_PortableDeviceFTM,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_device));

        if (SUCCEEDED(hr))
        {
            hr = m_device->Open(m_id.c_str(), m_clientInformation);

            if (hr == E_ACCESSDENIED)
            {
                LOGWARN(L"Failed to Open the device for Read Write access, will open it for Read-only access instead");
                m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
                hr = m_device->Open(m_id.c_str(), m_clientInformation);
            }

            if (SUCCEEDED(hr))
            {
                m_openCount++;
                m_handle = CreateEvent(nullptr, false, false, nullptr);

                LOGINFO(L"x%p - Opened() with handle %p", (void*)this, (DWORD)m_handle);
            }
            else
            {
                LOGERROR(L"! Failed to Open the device, hr = x%lx", hr);
                // Release the IPortableDevice interface, because we cannot proceed
                // with an unopen device.
                m_device->Release();
                m_device = nullptr;
            }
        }
        else
        {
            LOGERROR(L"! Failed to CoCreateInstance CLSID_PortableDeviceFTM, hr = x%lx", hr);
        }

        StartNotifications();
    }
    else
    {
        m_openCount++;

        LOGTRACE(L"Incrementing OpenCount to %d", m_openCount);
    }

    LOGTRACE(L"Out: MTPDevice::Open - now open %d times with handle x%08x", m_openCount, (DWORD)m_handle);

    return m_handle;
}

bool
MTPDevice::Close()
{
    LOGTRACE(L"In: MTPDevice::Close");

    bool closed = false;

    if (m_openCount)
    {
        if (m_openCount == 1)
        {
            StopNotifications();

            HRESULT hr = m_device->Close();

            if (SUCCEEDED(hr))
            {
                m_openCount--;
                m_device->Release();
                m_device = nullptr;

                CloseHandle(m_handle);
                m_handle = INVALID_HANDLE_VALUE;

                LOGTRACE(L"Out: Device::Close - Closed()");

                closed = true;
            }
        }
        else
        {
            m_openCount--;

            LOGTRACE(L"Out: Device::Close - Still open, opencount reduced to %d", m_openCount);

            closed = false;
        }
    }

    LOGWARN(L"Out: Device::Close (result = %d)", closed);

    return closed;
}

bool
MTPDevice::StartNotifications()
{
    if (!m_eventHandler)
    {
        // Set up event handler
        m_eventHandler = new DeviceEventHandler(m_device);
    }

    return true;
}

bool
MTPDevice::StopNotifications()
{
    if (m_eventHandler)
    {
        delete m_eventHandler;
        m_eventHandler = nullptr;
    }

    return true;
}

std::wstring
MTPDevice::GetRegistryPath()
{
    return m_manufacturer + L"\\" + m_friendlyName;
}

void
MTPDevice::InitFriendlyName()
{
    if (m_friendlyName.empty())
    {
        LOGTRACE(L"In: MTPDevice::InitFriendlyName");

        HRESULT hr;
        LPCWSTR* data;
        DWORD propertyLength = 0;
        DWORD propertyType = 0;
        std::wstring result;

        hr = m_manager->GetDeviceFriendlyName(m_id.c_str(), NULL, &propertyLength);

        if (FAILED(hr))
        {
            LOGERROR(L"! Failed to get number of bytes for device friendlyName for '%s'- hr = 0x%lx", m_id.c_str(), hr);
        }
        else
        {
            if (propertyLength > 0)
            {
                data = new LPCWSTR[propertyLength];
                ZeroMemory((void*)data, propertyLength * sizeof(WCHAR));

                hr = m_manager->GetDeviceFriendlyName(m_id.c_str(), (WCHAR*)data, &propertyLength);

                if (SUCCEEDED(hr))
                {
                    m_friendlyName = std::wstring((WCHAR*)data);
                }
                else
                {
                    LOGERROR(L"! Failed to get device friendlyName for '%s' - error x%lx", m_id.c_str(), hr);
                }

                delete[] data;
            }
            else {
                // Looks like the property is empty, so lets just
                // return success
                m_friendlyName = std::wstring();
                hr = ERROR_SUCCESS;
            }
        }

        LOGTRACE(L"Out: Device::InitFriendlyName - setting '%s'", m_friendlyName.c_str());
    }
}

void
MTPDevice::InitManufacturer()
{
    if (m_manufacturer.empty())
    {
        LOGTRACE(L"In: MTPDevice::InitManufacturer");

        HRESULT hr;
        LPCWSTR* data;
        DWORD propertyLength = 0;
        DWORD propertyType = 0;
        std::wstring result;

        hr = m_manager->GetDeviceManufacturer(m_id.c_str(), NULL, &propertyLength);

        if (FAILED(hr))
        {
            LOGERROR(L"! Failed to get number of bytes for device manufacturer for '%s'- hr = 0x%lx", m_id.c_str(), hr);
        }
        else
        {
            if (propertyLength > 0)
            {
                data = new LPCWSTR[propertyLength];
                ZeroMemory((void*)data, propertyLength * sizeof(WCHAR));

                hr = m_manager->GetDeviceManufacturer(m_id.c_str(), (WCHAR*)data, &propertyLength);

                if (SUCCEEDED(hr))
                {
                    m_manufacturer = std::wstring((WCHAR*)data);
                }
                else
                {
                    LOGERROR(L"! Failed to get device manufacturer for '%s' - error x%lx", m_id.c_str(), hr);
                }

                delete[] data;
            }
            else {
                // Looks like the property is empty, so lets just
                // return success
                m_manufacturer = std::wstring();
                hr = ERROR_SUCCESS;
            }
        }

        LOGTRACE(L"Out: MTPDevice::InitManufacturer - setting '%s'", m_manufacturer.c_str());
    }
}

void
MTPDevice::InitDescription()
{
    if (m_description.empty())
    {
        LOGTRACE(L"In: MTPDevice::InitDescription");

        HRESULT hr;
        LPCWSTR* data;
        DWORD propertyLength = 0;
        DWORD propertyType = 0;
        std::wstring result;

        hr = m_manager->GetDeviceDescription(m_id.c_str(), NULL, &propertyLength);

        if (FAILED(hr))
        {
            LOGERROR(L"! Failed to get number of bytes for device description for '%s'- hr = 0x%lx", m_id.c_str(), hr);
        }
        else
        {
            if (propertyLength > 0)
            {
                data = new LPCWSTR[propertyLength];
                ZeroMemory((void*)data, propertyLength * sizeof(WCHAR));

                hr = m_manager->GetDeviceDescription(m_id.c_str(), (WCHAR*)data, &propertyLength);

                if (SUCCEEDED(hr))
                {
                    m_description = std::wstring((WCHAR*)data);
                }
                else
                {
                    LOGERROR(L"! Failed to get device description for '%s' - error x%lx", m_id.c_str(), hr);
                }

                delete[] data;
            }
            else {
                // Looks like the property is empty, so lets just
                // return success
                m_description = std::wstring();
                hr = ERROR_SUCCESS;
            }
        }

        LOGTRACE(L"Out: MTPDevice::InitDescription - setting '%s'", m_description.c_str());
    }
}

// Creates and populates an IPortableDeviceValues with information about
// this application.  The IPortableDeviceValues is used as a parameter
// when calling the IPortableDevice::Open() method.
void
MTPDevice::CreateClientInformation()
{
    if (!m_clientInformation)
    {
        LOGTRACE(L"In: MTPDevice::CreateClientInformation");

        // Client information is optional.  The client can choose to identify itself, or
        // to remain unknown to the driver.  It is beneficial to identify yourself because
        // drivers may be able to optimize their behavior for known clients. (e.g. An
        // IHV may want their bundled driver to perform differently when connected to their
        // bundled software.)

        // CoCreate an IPortableDeviceValues interface to hold the client information.
        //<SnippetDeviceEnum7>
        HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&m_clientInformation));

        if (IsSuccess(hr, L"CoCreateInstance CLSID_PortableDeviceValues"))
        {
            // Attempt to set all bits of client information
            hr = m_clientInformation->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);

            IsSuccess(hr, L"set WPD_CLIENT_NAME");

            hr = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);

            IsSuccess(hr, L"set WPD_CLIENT_MAJOR_VERSION");

            hr = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
            IsSuccess(hr, L"set WPD_CLIENT_MINOR_VERSION");

            hr = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
            IsSuccess(hr, L"set WPD_CLIENT_REVISION");

            //  Some device drivers need to impersonate the caller in order to function correctly.  Since our application does not
            //  need to restrict its identity, specify SECURITY_IMPERSONATION so that we work with all devices.
            hr = m_clientInformation->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);

            if (IsSuccess(hr, L"set WPV_CLIENT_SECURITY_QUALITY_OF_SERVICE"))
            {
                LOGTRACE(L"Out: Device::CreateClientInformation - Success");
            }
        }
    }
}

Message*
MTPDevice::InternalSend(Device::Op kind, Message* out)
{
    LOGTRACE(L"In: MTPDevice::InternalSend");

    HRESULT  hr = S_OK;
    IPortableDeviceValues* pDevValues;
    Message* result = new Message(0);
    PROPERTYKEY opType;

    switch (kind)
    {
    case Op::CommandOnly:
        LOGTRACE(L"  Sending Command only, no send/receive data block");
        opType = WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITHOUT_DATA_PHASE;
        break;

    case Op::ReceiveData:
        LOGTRACE(L"  Sending Command, expecting data in return");
        opType = WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_READ;
        break;

    case Op::SendData:
        LOGTRACE(L"  Sending Command and data, not expecting data back");
        opType = WPD_COMMAND_MTP_EXT_EXECUTE_COMMAND_WITH_DATA_TO_WRITE;
        break;
    }

    // First we create an object that contains the command we need to send
    hr = CoCreateInstance(CLSID_PortableDeviceValues,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDevValues));

    if (IsSuccess(hr, L"CoCreateInstance (CLSID_PortableDeviceValues"))
    {
        if (pDevValues != NULL)
        {
            hr = pDevValues->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                opType.fmtid);

            IsSuccess(hr, L"IPortableDeviceValues::SetGuidValue");

            hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                opType.pid);

            // Specify the actual MTP opcode to execute here
            if (IsSuccess(hr, L"IPortableDeviceValues::SetGuidValue"))
            {
                hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPERATION_CODE,
                    (ULONG)out->GetCommand());

                IsSuccess(hr, L"  Failed to set Operation Code");
            }

            // GetDevicePropValue requires the property code as an MTP parameter
            // MTP parameters need to be first put into a PropVariantCollection
            IPortableDevicePropVariantCollection* spMtpParams = nullptr;

            if (IsSuccess(hr, L"Protect CoCreateInstance(CLSID_PortableDevicePropVariantCollection"))
            {
                hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&spMtpParams));

                IsSuccess(hr, L"  Failed to Create CLSID_PortableDevicePropVariantCollection");

                std::list<DWORD> params = out->GetParams();

                if (!params.empty())
                {
                    for (std::list<DWORD>::iterator it = params.begin(); it != params.end(); it++)
                    {
                        PROPVARIANT pvParam = { 0 };
                        pvParam.vt = VT_UI4;
                        pvParam.ulVal = *it;

                        if (hr == S_OK)
                        {
                            hr = spMtpParams->Add(&pvParam);
                            IsSuccess(hr, L"Add property");
                        }
                    }
                }

                // Add MTP parameters collection to our main parameter list
                if (IsSuccess(hr, L"Protect SetIPortableDevicePropVariantCollectionValue"))
                {
                    hr = pDevValues->SetIPortableDevicePropVariantCollectionValue(
                        WPD_PROPERTY_MTP_EXT_OPERATION_PARAMS, spMtpParams);

                    IsSuccess(hr, L"SetIPortableDevicePropVariantCollectionValue");
                }

                if (kind == Op::SendData)
                {
                    // Specify the number of bytes that arrive with this command. This allows us to 
                    // send the data in chunks if required (multiple WRITE_DATA commands). In this case,
                    //  send the data in a single chunk
                    if (hr == S_OK)
                    {
                        hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE,
                            out->GetDataLen());

                        IsSuccess(hr, L"SetUnsignedIntegerValue");
                    }
                }
            }

            if (spMtpParams)
            {
                spMtpParams->Release();
                spMtpParams = nullptr;
            }
        }
    }

    IPortableDeviceValues* spResults = nullptr;

    hr = m_device->SendCommand(0, pDevValues, &spResults);

    // Check if the driver was able to send the command by interrogating WPD_PROPERTY_COMMON_HRESULT
    HRESULT hrCmd = S_OK;
    if (IsSuccess(hr, L"SendCommand"))
    {
        hr = spResults->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &hrCmd);
        IsSuccess(hr, L"Send Command complete");
    }

    if (hr == S_OK)
    {
        hr = hrCmd;
        IsSuccess(hr, L"Send Command actual send result");
    }

    // If the transfer was initiated successfully, the driver returns a context cookie
    LPWSTR pwszContext = NULL;

    if (hr == S_OK)
    {
        hr = spResults->GetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, &pwszContext);
        IsSuccess(hr, L"Get transfer context");
    }

    if (hr == S_OK && kind == Op::SendData)
    {
        // Use the WPD_COMMAND_MTP_EXT_WRITE_DATA command to send the data
        (void)pDevValues->Clear();

        hr = pDevValues->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
            WPD_COMMAND_MTP_EXT_WRITE_DATA.fmtid);

        if (IsSuccess(hr, L"SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY) (Send Data)"))
        {
            hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                WPD_COMMAND_MTP_EXT_WRITE_DATA.pid);
        }

        // Specify the same context that we received earlier
        if (IsSuccess(hr, L"SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID) (Send Data)"))
        {
            hr = pDevValues->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, pwszContext);
        }

        // Specify the number of bytes that arrive with this command. This allows us to 
        // send the data in chunks if required (multiple WRITE_DATA commands). In this case,
        //  send the data in a single chunk
        if (IsSuccess(hr, L"SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT) (Send Data)"))
        {
            hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_WRITE,
                out->GetDataLen());
        }

        // Provide the data that needs to be transferred
        if (IsSuccess(hr, L"SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_WRITE) (Send Data)"))
        {
            hr = pDevValues->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, out->GetData(), out->GetDataLen());
        }

        // Send the data to the device
        spResults = NULL;
        if (IsSuccess(hr, L"SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA) (Send Data)"))
        {
            hr = m_device->SendCommand(0, pDevValues, &spResults);
        }

        // Check if the data was sent successfully by interrogating COMMON_HRESULT
        if (IsSuccess(hr, L"SendCommand (Send Data)"))
        {
            hr = spResults->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &hrCmd);
        }

        if (IsSuccess(hr, L"GetErrorValue (Send Data)"))
        {
            hr = hrCmd;
        }

        // The driver informs us about the number of bytes that were actually transferred. Normally this
        // should be the same as the number that we provided.
        DWORD cbBytesWritten = 0;
        if (IsSuccess(hr, L"Error Value (Send Data)"))
        {
            hr = spResults->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_WRITTEN,
                &cbBytesWritten);
        }

        LOGTRACE(L"successfully wrote %d bytes", cbBytesWritten);
    }

    if (hr == S_OK && kind == Op::ReceiveData)
    {
        // The driver indicates how many bytes will be transferred. This is important to
        // retrieve because we have to read all the data that the device sends us (even if it
        // is not the size we were expecting); otherwise, we run the risk of the device going out of sync
        // with the driver.
        ULONG cbReportedDataSize = 0;
        ULONG cbOptimalDataSize = 0;

        hr = spResults->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_OPTIMAL_TRANSFER_BUFFER_SIZE,
            &cbOptimalDataSize);

        hr = spResults->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_TOTAL_DATA_SIZE,
            &cbReportedDataSize);

        LOGTRACE(L"optimal data size = %d, reported data size = %d", cbOptimalDataSize, cbReportedDataSize);

        // Note: The driver provides an additional property, WPD_PROPERTY_MTP_EXT_OPTIMAL_TRANSFER_BUFFER_SIZE,
        // which suggests the chunk size that the date should be retrieved in. If your application will be 
        // transferring a large amount of data (>256K), use this property to break down the
        // transfer into small chunks so that your app is more responsive.
        // We'll skip this here because device properties are never that big (especially BatteryLevel).
        // If no data will be transferred, skip reading in the data
        if (IsSuccess(hr, L"Get Rx Data size"))
        {
            // Send the command to transfer the data
            if (spResults)
            {
                spResults->Release();
                spResults = nullptr;
            }

            if (cbReportedDataSize == 0)
            {
                LOGTRACE(L"No data to read, skipping read");
            }
            else
            {
                // Tell the message to pre-allocate data (this will save a lot of reallocation/copy operations)
                result->Allocate(cbReportedDataSize);

                // Use the WPD_COMMAND_MTP_EXT_READ_DATA command to read in the data
                // Allocate a buffer for the command to read data into - this should 
                // be the same size as the number of bytes we are expecting to read (per chunk, if applicable).
                // Use up to 8MB buffer, currently the optimal buffer size seems to be 256kB
                registry.Open();
                bool maximumMemoryMode = (bool)registry.GetDWORD(L"", L"Maximum Memory Mode", 0);
                registry.Close();

                ULONG bufferSize = maximumMemoryMode ? cbReportedDataSize : min(cbReportedDataSize, max(cbOptimalDataSize * 16, 2 ^ 23));
                LOGTRACE(L"Using a transfer buffer of %d bytes (maxmum memory mode is %s)", bufferSize, maximumMemoryMode ? L"ON" : L"OFF");

                BYTE* pbBufferIn = NULL;
                if (IsSuccess(hr, L"SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT) (Read Data)"))
                {
                    pbBufferIn = (BYTE*)CoTaskMemAlloc(bufferSize);
                    if (pbBufferIn == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if (IsSuccess(hr, L"CoTaskMemAlloc"))
                {
                    ULONG totalBytesRead = 0;

                    do
                    {
                        ULONG xferSize = min(cbReportedDataSize, bufferSize);

                        hr = pDevValues->Clear();

                        if (IsSuccess(hr, L"Clear parameter storage (Read Data)"))
                        {
                            hr = pDevValues->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
                                WPD_COMMAND_MTP_EXT_READ_DATA.fmtid);
                        }

                        if (IsSuccess(hr, L"SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY) (Read Data)"))
                        {
                            hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
                                WPD_COMMAND_MTP_EXT_READ_DATA.pid);
                        }

                        // Specify the same context that we received earlier
                        if (IsSuccess(hr, L"SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID) (Read Data)"))
                        {
                            hr = pDevValues->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, pwszContext);
                        }

                        // Specify the number of bytes to transfer as a parameter
                        if (IsSuccess(hr, L"SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT) (Read Data)"))
                        {
                            hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ,
                                xferSize);
                        }

                        // Pass the allocated buffer as a parameter
                        if (IsSuccess(hr, L"SetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_TRANSFER_NUM_BYTES_TO_READ) (Read Data)"))
                        {
                            hr = pDevValues->SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA,
                                pbBufferIn, bufferSize);
                        }

                        if (IsSuccess(hr, L"SetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA) (Read Data)"))
                        {
                            hr = m_device->SendCommand(0, pDevValues, &spResults);
                        }

                        // Check if the driver was able to transfer the data
                        hrCmd = S_OK;
                        if (IsSuccess(hr, L"SendCommand (Read Data)"))
                        {
                            hr = spResults->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &hrCmd);
                        }

                        if (IsSuccess(hr, L"GetErrorValue (Read Data)"))
                        {
                            hr = hrCmd;
                        }

                        // IMPORTANT: The API does not actually transfer the data into the buffer we provided earlier.
                        // Instead, it is available in the results collection.
                        BYTE* pbBufferOut = NULL;
                        ULONG cbBytesRead = 0;

                        if (IsSuccess(hr, L"Error Value (Read Data)"))
                        {
                            hr = spResults->GetBufferValue(WPD_PROPERTY_MTP_EXT_TRANSFER_DATA, &pbBufferOut, &cbBytesRead);

                            totalBytesRead += cbBytesRead;

                            if (IsSuccess(hr, L"GetBufferValue (WPD_PROPERTY_MTP_EXT_TRANSFER_DATA) (Read Data)"))
                            {
                                result->AddData(pbBufferOut, cbBytesRead);
                                LOGTRACE(L"Read %d of %d bytes", totalBytesRead, cbReportedDataSize);
                            }
                        }

                        CoTaskMemFree(pbBufferOut);
                        pbBufferOut = NULL;

                        // Send the command to transfer the data
                        if (spResults)
                        {
                            spResults->Release();
                            spResults = nullptr;
                        }
                    } while (IsSuccess(hr, L"Loop read") && totalBytesRead < cbReportedDataSize);
                }

                if (pbBufferIn)
                {
                    CoTaskMemFree(pbBufferIn);
                }
            }
        }
    }

    // Use the WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER command to signal transfer completion
    (void)pDevValues->Clear();
    if (IsSuccess(hr, L"Protect WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER"))
    {
        hr = pDevValues->SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY,
            WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.fmtid);
    }

    if (IsSuccess(hr, L"SetGuidValue(WPD_PROPERTY_COMMON_COMMAND_CATEGORY) (END)"))
    {
        hr = pDevValues->SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID,
            WPD_COMMAND_MTP_EXT_END_DATA_TRANSFER.pid);
    }

    // Specify the same context that we received earlier
    if (IsSuccess(hr, L"SetUnsignedIntegerValue(WPD_PROPERTY_COMMON_COMMAND_ID) (END)"))
    {
        hr = pDevValues->SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT, pwszContext);
    }

    // Send the completion command

    if (spResults)
    {
        spResults->Release();
        spResults = nullptr;
    }

    if (IsSuccess(hr, L"SetStringValue(WPD_PROPERTY_MTP_EXT_TRANSFER_CONTEXT) (END)"))
    {
        hr = m_device->SendCommand(0, pDevValues, &spResults);
    }

    // Check if the driver successfully ended the data transfer
    if (IsSuccess(hr, L"SendCommand (END)"))
    {
        hr = spResults->GetErrorValue(WPD_PROPERTY_COMMON_HRESULT, &hrCmd);
    }

    if (IsSuccess(hr, L"GetErrorValue (END)"))
    {
        hr = hrCmd;
    }

    // If the command was executed successfully, check the MTP response code to see if the
    // device can handle the command and the data. Be aware that there is a distinction between the command
    // and the data being successfully sent to the device and the command and data being handled successfully 
    // by the device
    DWORD dwResponseCode;
    if (IsSuccess(hr, L"Error Value (END)"))
    {
        hr = spResults->GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE, &dwResponseCode);
    }

    if (IsSuccess(hr, L"GetUnsignedIntegerValue(WPD_PROPERTY_MTP_EXT_RESPONSE_CODE) (END)"))
    {
        result->SetCommand((WORD)dwResponseCode);

        logger.Log(result->IsSuccess() ? Logger::LogLevel::Trace : Logger::LogLevel::Warn, L"Command x%04x result x%04x", out->GetCommand(), result->GetCommand());
    }

    if (spResults)
    {
        spResults->Release();
        spResults = nullptr;
    }

    // If response parameters are present, they will be contained in the WPD_PROPERTY_MTP_EXT_RESPONSE_PARAMS 
    // property. SetDevicePropValue does not return additional response parameters, so skip this code 


    // Free up any allocated memory
    CoTaskMemFree(pwszContext);

    if (pDevValues)
    {
        pDevValues->Release();
        pDevValues = nullptr;
    }

    LOGTRACE(L"Out: MTPDevice::InternalSend");

    return result;
}

bool
MTPDevice::Send(Message* out)
{
    LOGTRACE(L"In: MTPDevice::Send");

    Op method = Op::CommandOnly;

    if (out->GetDataLen())
    {
        method = Op::SendData;
    }

    InternalSend(method, out);

    LOGTRACE(L"Out: MTPDevice::InternalSend");

    return true;
}

Message*
MTPDevice::Receive(Message* out)
{
    LOGTRACE(L"In: MTPDevice::Receive");

    Message* result = InternalSend(Op::ReceiveData, out);

    LOGTRACE(L"Out: MTPDevice::Receive");

    return result;
}

bool
MTPDevice::IsSuccess(HRESULT hr, const wchar_t* message)
{
    bool result = SUCCEEDED(hr);

    if (!result)
    {
        LOGERROR(L"Failed executing %s (hr = x%08x)", message, hr);
    }

    return result;
}