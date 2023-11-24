#pragma once
#include "Camera.h"
#include "CameraSupportedProperties.h"
#include <unordered_map>
#include <map>
#include "PropertyInfo.h"

// Sony Commands - Seem to have a handle on these ones
/* Get all properties in one go */
constexpr auto COMMAND_READ_SETTINGS = 0x9209;

/* Get list of properties that the camera supports
   Seems to be broken into two sections... */
constexpr auto COMMAND_SONY_GET_PROPERTY_LIST = 0x9202;

   // Sony Commands - Unsure about these ones
constexpr auto COMMAND_SONY_GET_NEXT_HANDLE = 0x9201;

// Setting Properties:
//   ..._PROPERTY   = Properties where accessibility = READWRITE
//   ..._PROPERTY2  = Properties where accessibility = READONLY but SonySpecial != 0
constexpr auto COMMAND_SONY_SET_PROPERTY = 0x9205;
constexpr auto COMMAND_SONY_SET_PROPERTY2 = 0x9207;
// 1: d20d, 0000 - set shutter speed -1 ?
//    ^^^^
//    Property to set
// 2: ffff
//    ^^^^
//    Value to use


// Returns error?
constexpr auto COMMAND_SONY3 = 0x9208;

class SonyCamera : public Camera
{
public:
    SonyCamera(Device* device);
    ~SonyCamera();

    bool Initialize();
    bool RefreshSettings();
    bool SetProperty(const Property id, PropertyValue* value);
    virtual UINT16 SetFocus(UINT16 focusPosition);
    virtual UINT16 GetFocusLimit();
    virtual UINT16 GetFocus();
    virtual void SetFocusSteps(std::wstring steps, std::wstring sleeps, bool handsOff);

private:
    void ResetFocus();
    void MoveFocus(INT16 step);

    std::map<UINT16, UINT16> m_focusSteps;
    std::map<UINT16, UINT16> m_focusSleeps;
    INT16 m_currentFocusPosition = -1;
    UINT16 m_lastFocusPosition = 0;
    UINT16 m_lastFocusDiff = 0;
    UINT16 m_focusLimit = 0;
    bool m_focusHandsOff = false;
};

