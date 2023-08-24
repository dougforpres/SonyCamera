#include "pch.h"
#include "CameraManager.h"
#include "SonyCamera.h"
#include "Logger.h"
#include "Registry.h"

CameraManager::CameraManager()
{

}

CameraManager::~CameraManager()
{
    std::unordered_map<HANDLE, Camera*>::iterator it = m_cameraMap.begin();

    while (it != m_cameraMap.end())
    {
        LOGWARN(L"-- deleting camera x%p with name %s", (it->first), (it->second)->GetDeviceInfo(false)->GetManufacturer().c_str());
        (*it).second->Close();
        delete (*it).second;
        it = m_cameraMap.erase(it);
    }
}

void
CameraManager::SetupSupportedDevices()
{
    LOGINFO(L"In: CameraManager::SetupSupportedDevices()");

    // Set up defaults for cameras
    registry.Open();


    // a1 - using MTP driver
    std::wstring key = L"Cameras\\Sony Corporation\\ILCE-1";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a1");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.15);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.15);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 8672);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5784);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 8640);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 5760);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
//    registry.SetStringDefault(key, L"Exposure Times", L"");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a5000
    key = L"Cameras\\Sony Corporation\\ILCE-5000";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a5000");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.22);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.22);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 5472);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 3656);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 5456);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 3632);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);
    registry.SetStringDefault(key, L"Exposure Times", L"0,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000");

    // a5100
    key = L"Cameras\\Sony Corporation\\ILCE-5100";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a5100");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // a6400
    key = L"Cameras\\Sony Corporation\\ILCE-6400";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6400");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a6000
    key = L"Cameras\\Sony Corporation\\ILCE-6000";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6000");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"0,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // a6100
    key = L"Cameras\\Sony Corporation\\ILCE-6100";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6100");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");

    // a6300
    key = L"Cameras\\Sony Corporation\\ILCE-6300";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6300");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200");

    // a6500
    key = L"Cameras\\Sony Corporation\\ILCE-6500";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6500");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200");

    // a6600
    key = L"Cameras\\Sony Corporation\\ILCE-6600";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6600");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");

    // a6700
    key = L"Cameras\\Sony Corporation\\ILCE-6700";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a6700");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR R");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.76);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.76);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6272);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4168);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6192);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4128);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a7
    key = L"Cameras\\Sony Corporation\\ILCE-7";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);

    // a7 II
    key = L"Cameras\\Sony Corporation\\ILCE-7M2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7 Mk II");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // a7 III
    key = L"Cameras\\Sony Corporation\\ILCE-7M3";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7 Mk III");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036, 68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400,128000");

    // a7 IV
    key = L"Cameras\\Sony Corporation\\ILCE-7M4";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7 Mk IV");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.09);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.09);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7040);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4688);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 7008);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4672);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400,128000");

    // a7C
    key = L"Cameras\\Sony Corporation\\ILCE-7C";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7C");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.95);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.95);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400,128000");

    // a7R
    key = L"Cameras\\Sony Corporation\\ILCE-7R";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.86);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.86);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7362);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4920);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 7360);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4912);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // a7RM2
    key = L"Cameras\\Sony Corporation\\ILCE-7RM2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk II");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.86);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.86);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5320);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a7RM3
    key = L"Cameras\\Sony Corporation\\ILCE-7RM3";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk III");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.86);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.86);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5320);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 7952);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 5304);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a7r IIIa
    key = L"Cameras\\Sony Corporation\\ILCE-7RM3A";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk IIIa");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.51);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.51);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5320);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 7952);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 5304);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a7R IV - using ms driver
    key = L"Cameras\\Sony Corporation\\ILCE-7RM4";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk IV");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.73);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.72);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 9600);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 6376);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7R IV - using libusbK driver
    key = L"Cameras\\VID_054C&PID_0CCC";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk IV");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.73);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.72);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 9600);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 6376);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7R IVa
    key = L"Cameras\\Sony Corporation\\ILCE-7RM4A";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk IVa");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.73);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.72);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 9600);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 6376);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a7R V
    key = L"Cameras\\Sony Corporation\\ILCE-7RM5";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7R Mk IVa");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.76);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.76);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 9568);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 6376);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 9504);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 6336);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400");

    // a7S
    key = L"Cameras\\Sony Corporation\\ILCE-7S";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7S");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 8.31);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 8.31);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 4256);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2848);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 4240);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 2832);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400,128000");

    // a7S II
    key = L"Cameras\\Sony Corporation\\ILCE-7SM2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7S Mk II");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 8.31);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 8.31);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 4256);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2848);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a7S III - using MTP driver
    key = L"Cameras\\Sony Corporation\\ILCE-7SM3";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7S Mk III");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 8.31);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 8.31);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 4256);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2848);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 4232);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 2832);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200,64000,80000,102400,128000");

    // a68
    key = L"Cameras\\Sony Corporation\\ILCA - 68";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a68");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4016);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // a7S III - using libusbK driver
    key = L"Cameras\\VID_054C&PID_0D18";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a7S Mk III");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 8.31);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 8.31);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 4256);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2848);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    key = L"Cameras\\Sony Corporation\\ILCA-77M2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a77 Mk II");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // a9
    key = L"Cameras\\Sony Corporation\\ILCE-9";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a9");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 5.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 5.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536,70536,71936,73536,75536,78336,81536,97536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // a68
    key = L"Cameras\\Sony Corporation\\ILCA-68";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a68");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.90);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.90);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4016);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600");

    // A99 MkII
    key = L"Cameras\\Sony Corporation\\ILCA-99M2";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a99 Mk II");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.51);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.51);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 7968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 5320);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);

    // SLT Alpha 58
    key = L"Cameras\\Sony Corporation\\SLT-A58";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"SLT-A58");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 4.27);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 4.27);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 5472);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 3656);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 5456);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 3632);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,100,200,400,800,1600,3200,6400,12800,16000");

    // SLT Alpha 99
    key = L"Cameras\\Sony Corporation\\SLT-A99V";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"SLT-A58");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 9.02);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 9.02);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 3968);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 2648);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 3936);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 2624);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 0);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 0);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);
    registry.SetStringDefault(key, L"Exposure Times", L"19660810, 16384010, 13107210, 9830410, 8519690, 6553610, 5242890, 3932170, 3276810, 2621450, 2097162, 1638410, 1310730, 1048586, 851978, 655370, 524298, 393226, 327690, 262154, 65539, 65540, 65541, 65542, 65544, 65546, 65549, 65551, 65556, 65561, 65566, 65576, 65586, 65596, 65616, 65636, 65661, 65696, 65736, 65786, 65856, 65936, 66036, 66176, 66336, 66536, 66786, 67136, 67536, 68036, 68736, 69536, 70536, 71936, 73536");
    registry.SetStringDefault(key, L"ISOs", L" 16777215, 50, 64, 80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000, 10000, 12800, 16000, 20000, 25600");

/*    // DSC RX10 Mk3
    key = L"Cameras\\Sony Corporation\\DSC-RX10M3";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"RX10 Mk III");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 2.40);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 2.40);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 5496);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 3672);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);
    */
    // DSC RX10 Mk4
    key = L"Cameras\\Sony Corporation\\DSC-RX10M4";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"RX10 Mk IV");
    registry.SetStringDefault(key, L"Sensor Name", L"CMOS");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 2.40);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 2.40);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 5496);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 3672);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 680);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 0);

    // ZV - E10
    key = L"Cameras\\Sony Corporation\\ZV - E10";

    registry.CreateKey(key);
    registry.SetStringDefault(key, L"", L"a ZV-E10");
    registry.SetStringDefault(key, L"Sensor Name", L"EXMOR");
    registry.SetDoubleDefault(key, L"Sensor X Size um", 3.91);
    registry.SetDoubleDefault(key, L"Sensor Y Size um", 3.91);
    registry.SetDWORDDefault(key, L"Sensor X Resolution", 6024);
    registry.SetDWORDDefault(key, L"Sensor Y Resolution", 4024);
    registry.SetDWORDDefault(key, L"AutoCropped X Resolution", 6000);
    registry.SetDWORDDefault(key, L"AutoCropped Y Resolution", 4000);
    registry.SetDWORDDefault(key, L"Preview X Resolution", 1024);
    registry.SetDWORDDefault(key, L"Preview Y Resolution", 576);
    registry.SetDoubleDefault(key, L"Exposure Time Min", 0.000125);
    registry.SetDoubleDefault(key, L"Exposure Time Max", 900.0);
    registry.SetDoubleDefault(key, L"Exposure Time Step", 0.000125);
    registry.SetDWORDDefault(key, L"Sensor Type", 2);
    registry.SetDWORDDefault(key, L"Supports Liveview", 1);
    registry.SetStringDefault(key, L"Exposure Times", L"0,19660810,16384010,13107210,9830410,8519690,6553610,5242890,3932170,3276810,2621450,2097162,1638410,1310730,1048586,851978,655370,524298,393226,327690,262154,65539,65540,65541,65542,65544,65546,65549,65551,65556,65561,65566,65576,65586,65596,65616,65636,65661,65696,65736,65786,65856,65936,66036,66176,66336,66536,66786,67136,67536,68036,68736,69536");
    registry.SetStringDefault(key, L"ISOs", L"16777215,50,64,80,100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600,32000,40000,51200");

    registry.Close();

    LOGINFO(L"Out: CameraManager::SetupSupportedDevices()");
}

HANDLE
CameraManager::CreateCamera(Device* device, DWORD flags)
{
    LOGTRACE(L"In: CameraManager::CreateCamera(device='%s', flags=x%08x)", device->GetFriendlyName().c_str(), flags);
    Camera* camera = nullptr;
    HANDLE hResult = INVALID_HANDLE_VALUE;

    // See if we already have this device
    for (std::unordered_map<HANDLE, Camera*>::iterator it = m_cameraMap.begin(); it != m_cameraMap.end() && hResult == INVALID_HANDLE_VALUE; it++)
    {
        if ((*it).second->GetId() == device->GetId())
        {
            camera = (*it).second;
            LOGTRACE(L"CameraManager::CreateCamera: Found existing camera with handle x%08x", (*it).first);
            hResult = CompatibleHandle(camera->Open());
        }
    }

    if (hResult == INVALID_HANDLE_VALUE)
    {
        // As long as the camera entry is in the registry, we're good to go
        std::wostringstream builder;

        LOGTRACE(L"CameraManager::CreateCamera: Not an existing camera, looking to see if '%s' is supported", device->GetFriendlyName().c_str());

        if ((flags & OPEN_OVERRIDE) || device->IsSupported())
        {
            // Make a new camera...
            LOGTRACE(L"CameraManager::CreateCamera: Creating new camera object for '%s'", device->GetFriendlyName().c_str());

            camera = new SonyCamera(device);

            hResult = AddCamera(camera->Open(), camera);
        }
        else
        {
            LOGWARN(L"CameraManager::CreateCamera: Couldn't find what camera to make for '%s'", device->GetFriendlyName().c_str());
        }
    }

    if (camera)
    {
        camera->GetDevice()->StartNotifications();
    }

    LOGTRACE(L"Out: CameraManager::CreateCamera - returning x%p", hResult);

    return hResult;
}

HANDLE
CameraManager::AddCamera(HANDLE hCamera, Camera* camera)
{
    LOGTRACE(L"In: CameraManager::AddCamera(x%p, x%p)", hCamera, camera);

    HANDLE shortHandle = CompatibleHandle(hCamera);
    // In 64-bit windows we could get a > 32-bit handle.  The ASCOM code is expecting a 32-bit value.
    // According to MSDN, only the bottom 32-bits are valid, so we can trim the top 32-bits
    m_cameraMap.insert(std::pair<HANDLE, Camera*>(shortHandle, camera));

    LOGTRACE(L"Out: CameraManager::AddCamera(x%p, x%p), added handle x%p", hCamera, camera, shortHandle);

    return shortHandle;
}

HANDLE
CameraManager::CompatibleHandle(HANDLE handle)
{
#if _WIN64
    uint64_t temp = (uint64_t)handle & 0xffffffff;

//    LOGTRACE(L"CameraManager::CompatibleHandle(x%p) = x%p", handle, temp);

    return (HANDLE)temp;
#else
    return handle;
#endif
}

void
CameraManager::RemoveCamera(HANDLE hCamera)
{
    LOGTRACE(L"In: CameraManager::RemoveCamera(x%p)", hCamera);

    m_cameraMap.erase(hCamera);

    LOGTRACE(L"Out: CameraManager::RemoveCamera(x%p)", hCamera);
}

Camera*
CameraManager::GetCameraForHandle(HANDLE hCamera)
{
    hCamera = CompatibleHandle(hCamera);

    std::unordered_map<HANDLE, Camera*>::iterator it = m_cameraMap.find(hCamera);

    if (it != m_cameraMap.end())
    {
        return (*it).second;
    }
    else
    {
        LOGWARN(L"Unable to find camera for handle x%p", hCamera);
        LOGWARN(L"I have %d cameras in my list", m_cameraMap.size());

        for (it = m_cameraMap.begin(); it != m_cameraMap.end(); it++)
        {
            LOGWARN(L"-- got a x%p with name %s", (it->first), (it->second)->GetDeviceInfo(false)->GetManufacturer().c_str());
        }

        return nullptr;
    }
}
