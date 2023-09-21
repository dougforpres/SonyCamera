#include "pch.h"
#include "SonyCamera.h"
#include "Logger.h"
#include "CameraException.h"
#include "Locker.h"

#define SONY_GET_NEXT_HANDLE_ENABLE
#define SONY_GET_NEXT_HANDLE_EXTRA_PARAMS

SonyCamera::SonyCamera(Device* device)
    : Camera(device)
{
    LOGTRACE(L"In: SonyCamera::SonyCamera");
    LOGTRACE(L"Out: SonyCamera::SonyCamera");
}

SonyCamera::~SonyCamera()
{
    LOGTRACE(L"In: SonyCamera::~SonyCamera");
    LOGTRACE(L"Out: SonyCamera::~SonyCamera");
}

bool
SonyCamera::Initialize()
{
    LOGTRACE(L"In: SonyCamera::Initialize()");
    bool result = true;
    Message* tx;
    Message* rx;

    if (GetDevice()->NeedsSession())
    {
        tx = new Message(COMMAND_OPEN_SESSION);

        tx->AddParam(0x00000001);

        this->m_device->Send(tx);

        delete tx;
    }

    tx = new Message(COMMAND_GET_STORAGE_IDS);

    rx = m_device->Receive(tx);
    //  Ignore result of this method, a7siii (one sample) seems to return a non-success result
    //    result &= rx->IsSuccess();

    LOGTRACE(L"Get Storage ID's >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

#ifdef SONY_GET_NEXT_HANDLE_ENABLE
    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000001);
#ifdef SONY_GET_NEXT_HANDLE_EXTRA_PARAMS
    tx->AddParam(0x00000000);  // Test only sending one param (which is what MTP spec says it should be)
    tx->AddParam(0x00000000);
#endif

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Next Handle (1) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000002);
#ifdef SONY_GET_NEXT_HANDLE_EXTRA_PARAMS
    tx->AddParam(0x00000000);  // Test only sending one param (which is what MTP spec says it should be)
    tx->AddParam(0x00000000);
#endif

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Next Handle (2) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;
#endif

    tx = new Message(COMMAND_SONY_GET_PROPERTY_LIST);

    tx->AddParam(0x000000c8);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    m_supportedProperties = CameraSupportedProperties(rx);
    LOGTRACE(L"Get Supported Properties (1) >> %s", m_supportedProperties.AsString().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_NEXT_HANDLE);

    tx->AddParam(0x00000003);
    tx->AddParam(0x00000000);
    tx->AddParam(0x00000000);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Next Handle (3) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    tx = new Message(COMMAND_SONY_GET_PROPERTY_LIST);

    tx->AddParam(0x000000c8);

    rx = m_device->Receive(tx);
    result &= rx->IsSuccess();

    LOGTRACE(L"Get Supported Properties (2) >> %s", rx->Dump().c_str());

    delete tx;
    delete rx;

    RefreshSettings();

    LOGTRACE(L"Out: SonyCamera::Initialize() - result %d", result);

    SetInitialized(result);

    return result;
}

bool
SonyCamera::RefreshSettings()
{
    LOGTRACE(L"In: SonyCamera::RefreshSettings()");

    Message* tx;
    Message* rx;

    tx = new Message(COMMAND_READ_SETTINGS);
    rx = m_device->Receive(tx);

    if (rx->IsSuccess())
    {
        CameraSettings cs = CameraSettings(rx);

        // Only need to load up the fakes first time thru
        if (!m_settings)
        {
            LoadFakeProperties(&cs);
        }

        DWORD waitResult = WaitForSingleObject(m_hBusyMutex, 1000);

        if (waitResult == WAIT_OBJECT_0)
        {
            if (!m_settings)
            {
                m_settings = new CameraSettings(cs);
            }
            else
            {
                m_settings->Copy(cs);
            }
        }

        ReleaseMutex(m_hBusyMutex);

        delete tx;
        delete rx;
    }
    else
    {
        throw CameraException(L"Error reading settings from camera");
    }

    LOGTRACE(L"Out: SonyCamera::RefreshSettings()");

    return true;
}

bool
SonyCamera::SetProperty(const Property id, PropertyValue* value)
{
    LOGTRACE(L"In: SonyCamera::SetProperty(x%04x, %s)", (int)id, value->ToString().c_str());

    bool result = false;

    Message* tx;

    // Depending on the property type, we need to send different command
    std::unique_ptr<CameraProperty> prop(GetProperty(id));

    if (prop)
    {
        Locker lock(settingsLock);
        DWORD messageType;
        PropertyInfo* info = prop->GetInfo();

        if (info->GetAccess() == Accessibility::READ_WRITE)
        {
            messageType = COMMAND_SONY_SET_PROPERTY;
        }
        else if (info->GetSonySpare() != 0)
        {
            messageType = COMMAND_SONY_SET_PROPERTY2;
        }
        else
        {
            throw CameraException(L"Property is not writable");
        }

        tx = new Message(messageType);

        tx->AddParam((DWORD)id);

        size_t size = value->GetDataSize();

        if (size & 1) {
            size++;
        }

        BYTE* txdata = new BYTE[size];

        memset(txdata, 0, size);

        value->Write(txdata, size);
        tx->SetData(txdata, size);

        if (m_device->Send(tx))
        {
            result = true;
        }

        delete[] txdata;
        delete tx;
    }

    LOGTRACE(L"Out: SonyCamera::SetProperty(x%08x, %s), - result = %d", (int)id, value->ToString().c_str(), result);

    return result;
}

UINT16
SonyCamera::SetFocus(UINT16 focusPosition)
{
    DeviceInfo di = GetDeviceInfo();
    UINT16 maxStepSize = m_focusSteps.rbegin()->first;
    UINT16 focusDiff = abs(m_lastFocusPosition - focusPosition);

    if (m_currentFocusPosition < 0 || m_currentFocusPosition > GetFocusLimit()
        || di.GetFocusStartMode() == FocusStartMode::RESET_EVERY_TIME
        || (di.GetFocusStartMode() == FocusStartMode::RESET_BIGGER_MOVE && focusDiff > m_lastFocusDiff))
    {
        // The "+1" covers situation where biggest step is not an integer
        for (UINT16 resetPos = 0; resetPos < (GetFocusLimit() / m_focusSteps.rbegin()->second) + 1; resetPos++)
        {
            MoveFocus(maxStepSize);
        }
    }

    m_lastFocusPosition = m_currentFocusPosition;
    m_lastFocusDiff = abs(m_currentFocusPosition - focusPosition);

    if (abs(m_currentFocusPosition - focusPosition) <= m_focusSteps.begin()->second)
    {
        LOGTRACE(L"Not altering focus, we're as close as we can get @ %d vs desired %d", m_currentFocusPosition, focusPosition);

        return (UINT16)m_currentFocusPosition;
    }

    int attempts = 0;

    while (abs(m_currentFocusPosition - focusPosition) >= m_focusSteps[1] && attempts < maxStepSize * 10)
    {
        attempts++;

        // Calculate steps to get from here to there
        INT16 diff = focusPosition - m_currentFocusPosition;

        // We could just find largest step LESS than desired position,
        // but it would be more efficient to find largest step CLOSEST
        // to desired position, then repeat
        INT16 idealStep = 1;

        INT16 usize = abs(diff);
        INT16 bestDiff = abs(usize - m_focusSteps[idealStep]);

        for (UINT16 test_id = idealStep + 1; test_id <= maxStepSize; test_id += 1)
        {
            UINT diff2 = abs(usize - m_focusSteps[test_id]);

            if (diff2 < bestDiff)
            {
                idealStep = test_id;
                bestDiff = diff2;
            }
        }

        if (diff < 0)
        {
            idealStep = -idealStep;
        }

        MoveFocus(idealStep);
    }

    return (UINT16)m_currentFocusPosition;
}

UINT16
SonyCamera::GetFocusLimit()
{
    return m_focusLimit;
}

UINT16
SonyCamera::GetFocus()
{
    return m_focusSteps.empty() ? GetFocusLimit() + 1 : (UINT16)m_currentFocusPosition;
}

void
SonyCamera::SetFocusSteps(std::wstring steps)
{
    DeviceInfo di = GetDeviceInfo();

    m_focusSteps.clear();

    std::unique_ptr<CameraProperty> focusProp(GetProperty(Property::FocusControl));
    std::vector<double> deviceSteps;
    std::wstring s;
    std::wistringstream values(steps);

    // A list of comma separated pairs
    while (std::getline(values, s, L',')) {
        deviceSteps.push_back(_wtof(s.c_str()));
    }

    if (deviceSteps.empty() && di.GetFocusMagicNumber())
    {
        // Populate deviceSteps without scaling, then reprocess
        UINT16 maxStepSize = focusProp->GetInfo()->GetRangeHi()->GetINT16();

        for (UINT16 i = 0; i < maxStepSize; i++)
        {
            deviceSteps.push_back(pow(di.GetFocusMagicNumber(), i));
        }
    }

    if (!deviceSteps.empty())
    {
        UINT16 index = 1;
        m_focusLimit = *deviceSteps.begin();

        for (std::vector<double>::const_iterator it = deviceSteps.begin(); it != deviceSteps.end(); it++)
        {
            m_focusSteps[index] = (double)m_focusLimit / *it;
            index++;
        }

        for (std::map<UINT16, UINT16>::const_iterator it = m_focusSteps.begin(); it != m_focusSteps.end(); it++)
        {
            LOGINFO(L"Focus step %d = %d", (*it).first, (*it).second);
        }

        // We should probably reset focus so we know where we are
        m_currentFocusPosition = -1;
        SetFocus(m_focusLimit);
    }
}

void
SonyCamera::MoveFocus(INT16 step)
{
    short diff = m_focusSteps[abs(step)];

    LOGTRACE(L"Moving focus by %d (current = %d, step_size = %d).. ", step, m_currentFocusPosition, diff);

    PropertyValue pv(step);
    SetProperty(Property::FocusControl, &pv);

    //if (abs(step) == m_focusSteps.rbegin()->first)
    //{
    //    m_currentFocusPosition = step < 0 ? 0 : GetFocusLimit();
    //}
    //else
    //{
        if (step < 0)
        {
            m_currentFocusPosition -= diff;
        }
        else
        {
            m_currentFocusPosition += diff;
        }

        if (m_currentFocusPosition < 0)
        {
            m_currentFocusPosition = 0;
        }

        if (m_currentFocusPosition > GetFocusLimit())
        {
            m_currentFocusPosition = GetFocusLimit();
        }
//    }

    LOGTRACE(L" new position = %d\n", m_currentFocusPosition);

    Sleep(200 * abs(step));
}