#pragma once

enum class Accessibility {
    READ_ONLY           = 0x00,
    READ_WRITE          = 0x01,
    WRITE_ONLY_BUTTON   = 0x81,
    UNKNOWN_82          = 0x82,
    UNKNOWN_83          = 0x83,
};

enum class FormMode {
    NONE        = 0x00,
    RANGE       = 0x01,
    ENUMERATION = 0x02,
};

enum class DataType {
    UNKNOWN     = 0x0000,
    INT8        = 0x0001,
    UINT8       = 0x0002,
    INT16       = 0x0003,
    UINT16      = 0x0004,
    INT32       = 0x0005,
    UINT32      = 0x0006,
    INT64       = 0x0007,
    UINT64      = 0x0008,
    INT128      = 0x0009,
    UINT128     = 0x000a,
    AINT8       = 0xa001,
    AUINT8      = 0xa002,
    AINT16      = 0xa003,
    AUINT16     = 0xa004,
    AINT32      = 0xa005,
    AUINT32     = 0xa006,
    AINT64      = 0xa007,
    AUINT64     = 0xa008,
    AINT128     = 0xa009,
    AUINT128    = 0xa00a,
    STR         = 0xffff,
};

enum class Property {
    Undefined                   = 0x5000,
    BatteryLevel                = 0x5001,
    FunctionalMode              = 0x5002,
    ImageSize                   = 0x5003,
    CompressionSetting          = 0x5004,
    WhiteBalance                = 0x5005,
    RGBGain                     = 0x5006,
    FNumber                     = 0x5007,
    FocalLength                 = 0x5008,
    FocalDistance               = 0x5009,
    FocusMode                   = 0x500a,
    ExposureMeteringMode        = 0x500b,
    FlashMode                   = 0x500c,  // Interval shooting on = 2, off = 3
    ExposureTime                = 0x500d,
    ExposureProgramMode         = 0x500e,
    ExposureIndex               = 0x500f,
    ExposureBiasCompensation    = 0x5010,
    DateTime                    = 0x5011,
    CaptureDelay                = 0x5012,
    StillCaptureMode            = 0x5013,
    Contrast                    = 0x5014,
    Sharpness                   = 0x5015,
    DigitalZoom                 = 0x5016,
    EffectMode                  = 0x5017,
    BurstNumber                 = 0x5018,
    BurstInterval               = 0x5019,
    TimelapseNumber             = 0x501a,
    TimelapseInterval           = 0x501b,
    FocusMeteringMode           = 0x501c,
    UploadURL                   = 0x501d,
    Artist                      = 0x501e,
    CopyrightInfo               = 0x501f,

    FlashCompensation           = 0xd200,
    DROAutoHDR                  = 0xd201, // **  on = 1, off = 31
    JPEGImageSize               = 0xd203,
    ShutterSpeed                = 0xd20d,
    //                          = 0xd20e,
    CustomWhiteBalance          = 0xd20f,
    //                          = 0xd210,
    AspectRatio                 = 0xd211,
    //                          = 0xd212,
    ShutterButtonStatus         = 0xd213,
    //                          = 0xd214,
    PhotoBufferStatus           = 0xd215,
    AutoExposureLock            = 0xd217,  //** NEW Off = 1, On = 2
    Battery                     = 0xd218,
    FlashExposureLock           = 0xd219, //** NEW Off = 1, On = 2
    //                          = 0xd21b,
    //                          = 0xd21c,
    Aperture                    = 0xd21d,
    ISO                         = 0xd21e,
    //                          = 0xd21f,
    //                          = 0xd221, // Power? 1 = on, 0 = off ??  Saw it go to 0 once when camera switched off just prior to disconnect
    //                          = 0xd222,
    FocusArea                   = 0xd22c,
    FocusAssistMode             = 0xd22d,
    //                          = 0xd22e,
    FocusAssistZoom             = 0xd22f,
    FocusAssistCoord            = 0xd230,
    LiveView                    = 0xd231,
    FocusAssistSize             = 0xd232,
    //                          = 0xd233,
    //FocusMode2                = 0xd235,
    //                          = 0xd236,
    AutoWhileBalanceLock        = 0xd24e, //** NEW Off = 1, On = 2
    //IntervalShooting          = 0xd24f,
    //                          = 0xd250,

    // Buttons (push then release to invoke feature)
    ShutterHalfDown             = 0xd2c1,
    ShutterFullDown             = 0xd2c2,
    //Button                    = 0xd2c3, // ?? set AEL?  (1 off, 2 on)
    //Button?                   = 0xd2c5,
    //Button                    = 0xd2c7,
    //Button                    = 0xd2c8,
    //Button                    = 0xd2c9 ?? set FEL?  (1 off, 2 on)
    FocusAssistModeButton       = 0xd2cb,   // DOWN = 2, UP = 1 - Cycles thru options Off -> 1x -> 5x -> 11x -> Off
    FocusAssistCancelButton     = 0xd2cc, // DOWN = 2, UP = 1   - Immediately exit focus assist mode
    //Button                    = 0xd2cd,
    //Button                    = 0xd2ce,
    //Button                    = 0xd2cf,
    //Button                    = 0xd2d0,
    //Button?                   = 0xd2d1,
    //Button                    = 0xd2d2,
    //Button                    = 0xd2d3,
    //Button                    = 0xd2d4,
    //Button                    = 0xd2d9, // ?? set AWBL?  (1 off, 2 on)
};

// ObjectAdded
// ObjectRemoved
// StoreRemoved
// CaptureComplete
// StoreFull
// DeviceReset
// DevicePropChanged

enum class EventTypes {
    Undefined               = 0x4000,
    CancelTransaction       = 0x4001,
    ObjectAdded             = 0x4002,
    ObjectRemoved           = 0x4003,
    StoreAdded              = 0x4004,
    StoreRemoved            = 0x4005,
    DevicePropChanged       = 0x4006,
    ObjectInfoChanged       = 0x4007,
    DeviceInfoChanged       = 0x4008,
    RequestObjectTransfer   = 0x4009,
    StoreFull               = 0x400a,
    DeviceReset             = 0x400b,
    StoreInfoChanged        = 0x400c,
    CaptureComplete         = 0x400d,
    UnreportedStatus        = 0x400e,
};
