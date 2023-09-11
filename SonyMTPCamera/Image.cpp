#include "pch.h"
#include "Image.h"
#include "libraw/libraw.h"
#include "libjpeg//turbojpeg.h"
#include "Logger.h"
#include "Registry.h"
#include <codecvt>

static const DWORD EXIF_IDENT = 0x002a4949;
static const DWORD JPEG_IDENT = 0xe1ffd8ff;

std::wstring Image::s_savePath;

Image::Image(ObjectInfo* info, Message* message)
    : MessageReader(message),
      m_info(info)
{
    LOGTRACE(L"In: Image::Image @x%p - Creating from USB Data", (void*)this);

    DWORD offset = 0;
    DWORD ident = GetDWORD(offset);

    if (ident == EXIF_IDENT)
    {
        m_inputMode = InputMode::ARW;
        LOGTRACE(L"Image identified as ARW");
    }
    else if (ident == JPEG_IDENT)
    {
        m_inputMode = InputMode::JPEG;
        LOGTRACE(L"Image identified as JPEG");
    }
    else
    {
        // Preview images have some cruft at the beginning that I think is
        // something like focus point array or similar
        m_inputMode = InputMode::JPEG;
        m_imageOffset = ident;
        LOGTRACE(L"Image identified as Preview (JPEG)");
    }

    LOGTRACE(L"Out: Image::Image - Created");
}

Image::~Image()
{
    LOGTRACE(L"In: Image::~Image @x%p", (void*)this);

    if (m_pixelData && m_outputMode != OutputMode::PASSTHRU)
    {
        LOGTRACE(L"Cleaning up cached pixel data (%d bytes)", m_pixelDataSize);
        delete[] m_pixelData;
        m_pixelData = nullptr;
    }

    if (m_info)
    {
        delete m_info;
        m_info = nullptr;
    }

    LOGTRACE(L"Out: Image::~Image");
}

DWORD
Image::GetImageDataSize()
{
    LOGTRACE(L"In: Image::GetImageDataSize");

    DWORD result = 0;

    if (EnsurePixelsProcessed())
    {
        result = m_pixelDataSize;
    }

    LOGTRACE(L"Out: Image::GetImageDataSize - returning %d", result);

    return result;
}

BYTE*
Image::GetImageData()
{
    BYTE* result = nullptr;

    LOGTRACE(L"In:: Image::GetImageData");

    if (EnsurePixelsProcessed())
    {
        result = m_pixelData;
    }

    LOGTRACE(L"Out: Image::GetImageData - result x%p", result);

    return result;
}

DWORD
Image::GetMetaDataSize()
{
    LOGTRACE(L"InOut: Image::GetMetaDataSize() = x%04x", m_metaDataSize);

    return m_metaDataSize;
}

BYTE*
Image::GetMetaData()
{
    LOGTRACE(L"InOut:: Image::GetMetaData() = x%08p", m_metaData);

    return m_metaData;
}

void
Image::SetCrop(int top, int left, int bottom, int right)
{
    m_cropTop = top;
    m_cropLeft = left;
    m_cropBottom = bottom;
    m_cropRight = right;
}

LibRaw*
Image::InitializeLibRaw(bool process)
{
    LOGTRACE(L"Image:: InitLibRaw(%d)", process ? 1  : 0);
    LibRaw* libraw = new LibRaw(0);

    int r = libraw->open_buffer(GetData(), GetDataLength());

    if (r != LIBRAW_SUCCESS)
    {
        LOGERROR(L"Error opening ARW data in libraw %s", LibRawErrorToString(libraw, r).c_str());

        delete libraw;

        return nullptr;
    }

    registry.Open();

    // These settings appear to be used by libraw when decoding to RGB and during testing had no effect on the RAW data
    libraw->imgdata.params.use_camera_wb = registry.GetDWORD(L"libraw", L"use_camera_wb", libraw->imgdata.params.use_camera_wb);
    libraw->imgdata.params.output_bps = 16;
    libraw->imgdata.params.gamm[0] = registry.GetDouble(L"libraw", L"gamm0", libraw->imgdata.params.gamm[0]);
    libraw->imgdata.params.gamm[1] = registry.GetDouble(L"libraw", L"gamm1", libraw->imgdata.params.gamm[1]);
    libraw->imgdata.params.no_auto_bright = registry.GetDWORD(L"libraw", L"no_auto_bright", libraw->imgdata.params.no_auto_bright);
    libraw->imgdata.params.user_qual = registry.GetDWORD(L"libraw", L"user_qual", libraw->imgdata.params.user_qual);
    libraw->imgdata.params.user_flip = registry.GetDWORD(L"libraw", L"user_flip", libraw->imgdata.params.user_flip);

    registry.Close();

    r = libraw->unpack();

    if (r != LIBRAW_SUCCESS)
    {
        LOGERROR(L"LibRaw - problem unpacking data %s", LibRawErrorToString(libraw, r).c_str());
    }

    if (process)
    {
        r = libraw->dcraw_process();

        if (r != LIBRAW_SUCCESS)
        {
            LOGERROR(L"Unable to dcraw_process %s", LibRawErrorToString(libraw, r).c_str());
        }
    }

    return libraw;
}

bool
Image::ProcessARWData()
{
    LOGTRACE(L"In: Image:: ProcessARWData()");

    BYTE* result = nullptr;
    LibRaw* libraw = nullptr;
    libraw_processed_image_t* processed = nullptr;
    int r = 0;
    int linesize = 0;

    // Both RGB and RGGB require libraw to process the data to some extent or another
    switch (m_outputMode)
    {
    case OutputMode::RGB:
        LOGINFO(L"Output mode RGB...");
        libraw = InitializeLibRaw(true);

        if (libraw)
        {
            processed = libraw->dcraw_make_mem_image(&r);

            if (r != LIBRAW_SUCCESS)
            {
                LOGERROR(L"Unable to dcraw_make_mem_image %s", LibRawErrorToString(libraw, r).c_str());
            }
            else
            {
                if (processed)
                {
                    if (processed->data_size > 0)
                    {
                        m_pixelDataSize = processed->data_size;
                        m_pixelData = new BYTE[m_pixelDataSize];
                        memcpy(m_pixelData, processed->data, processed->data_size);
                        m_info->SetWidth(libraw->imgdata.sizes.iwidth);
                        m_info->SetHeight(libraw->imgdata.sizes.iheight);
                    }
                    else
                    {
                        LOGERROR(L"libraw failed to make memory image, BUT DIDN'T ERROR!");
                    }

                    LibRaw::dcraw_clear_mem(processed);
                    processed = nullptr;
                }
            }
        }
        else
        {
            LOGERROR(L"Unable to open image using libraw");
        }
        break;

    case OutputMode::RGGB:
        LOGINFO(L"Output mode RGGB...");
        libraw = InitializeLibRaw(false);

        if (libraw)
        {
            r = libraw->raw2image();

            if (r != LIBRAW_SUCCESS)
            {
                LOGERROR(L"Unable to raw2image %s", LibRawErrorToString(libraw, r).c_str());
            }

            m_rawWidth = libraw->imgdata.sizes.iwidth;
            m_rawHeight = libraw->imgdata.sizes.iheight;
            linesize = 4 * 2 * m_rawWidth;

            {
                std::string colorsStr = libraw->imgdata.idata.cdesc;
                LOGINFO(L"RAW file color format = %S", colorsStr.c_str());

                int xoffs = 0;
                int yoffs = 0;

                std::ostringstream builder;

                builder << colorsStr[libraw->COLOR(0, 0)];
                builder << colorsStr[libraw->COLOR(0, 1)];
                builder << colorsStr[libraw->COLOR(1, 0)];
                builder << colorsStr[libraw->COLOR(1, 1)];

                LOGINFO(L"Raw data layout in form: %S", builder.str().c_str());

                libraw_raw_inset_crop_t* crops = &(libraw->imgdata.sizes.raw_inset_crops[0]);
                ushort rawCropLeft   = crops->cleft;
                ushort rawCropTop    = crops->ctop;

                m_croppedHeight = crops->cheight;
                m_croppedWidth  = crops->cwidth;

                // libraw sets top/left to be 0xffff if they're 0
                if (rawCropLeft > m_rawWidth)
                {
                    LOGDEBUG(L"Camera didn't set left-crop value, assuming 0");
                    rawCropLeft = 0;
                }

                if (rawCropTop > m_rawHeight)
                {
                    LOGDEBUG(L"Camera didn't set top-crop value, assuming 0");
                    rawCropTop = 0;
                }

//                m_croppedWidth = libraw->imgdata.sizes.raw_crop.cwidth;
//                m_croppedHeight = libraw->imgdata.sizes.raw_crop.cheight;

                if (m_cropLeft < 0) {
                    LOGINFO(L"Crop Mode set to auto, using info from image");
                    m_cropLeft = rawCropLeft * 2; // expressed in 2 pixel units because RGGB;
                    m_cropTop = rawCropTop * 2;
                    m_cropRight = m_rawWidth - m_croppedWidth - m_cropLeft;
                    m_cropBottom = m_rawHeight - m_croppedHeight - m_cropTop;
                }

                ushort width = ushort(m_rawWidth - m_cropLeft - m_cropRight);
                ushort height = ushort(m_rawHeight - m_cropTop - m_cropBottom);

                LOGINFO(L"Raw size is %d x %d, crop (T,L,R,B) = %d, %d, %d, %d, result will be %d x %d", m_rawWidth, m_rawHeight, m_cropTop, m_cropLeft, m_cropBottom, m_cropRight, width, height);

                m_pixelDataSize = width * height * sizeof(short);
                m_pixelData = new BYTE[m_pixelDataSize];

                for (int y = m_cropTop; y < int(m_rawHeight - m_cropBottom); y++)
                {
                    ushort* pds = (ushort*)m_pixelData;
                    int readOffset = y * m_rawWidth;
                    int writeOffset = (m_pixelDataSize / sizeof(short)) - (y - m_cropTop + 1) * width;

                    int i0 = libraw->COLOR(y, 0);
                    int i1 = libraw->COLOR(y, 1);

                    ushort* ptr = (ushort*)(libraw->imgdata.image + readOffset);

                    for (int x = 0; x < width; x += 2)
                    {
                        pds[x + writeOffset] = *(ptr + i0);
                        ptr += 4;
                        pds[x + 1 + writeOffset] = *(ptr + i1);
                        ptr += 4;
                    }
                }

                m_info->SetWidth(width);
                m_info->SetHeight(height);
            }
        }
        else
        {
            LOGERROR(L"Unable to open image using libraw");
        }
        break;

    case OutputMode::PASSTHRU:
        LOGINFO(L"Output mode PASSTHRU...");
        // We're passing the raw in to out, so just shuffle the pointers
        m_pixelData = GetData();
        m_pixelDataSize = GetDataLength();
        break;

    default:
        LOGERROR(L"Unsupported output mode %d for ARW file (only supports RGB, RGGB, PASSTHRU)", m_outputMode);
    }

    if (libraw)
    {
        delete libraw;
        libraw = nullptr;
    }

    LOGTRACE(L"Out: Image:: ProcessARWData()");

    return m_pixelData != nullptr;
}

bool
Image::ProcessJPEGData()
{
    LOGTRACE(L"In: Image:: ProcessJPEGData()");

    tjhandle tj = nullptr;
    BYTE* result = nullptr;
    int r = 0;
    int width = 0;
    int height = 0;
    int pitch = 0;
    BYTE* temp = nullptr;
    int imageDataSize = GetDataLength() - m_imageOffset;
    BYTE* imageData = GetData() + m_imageOffset;

    if (m_imageOffset)
    {
        m_metaDataSize = m_imageOffset;
        m_metaData = GetData();
    }

    // Don't care about these two, but are required
    int jpegSubsamp = 0;
    int jpegColorspace = 0;

    // RGB requires libjpeg to process the data to some extent or another
    switch (m_outputMode)
    {
    case OutputMode::RGB:
        LOGINFO(L"Output mode RGB...");
        tj = tjInitDecompress();

        r = tjDecompressHeader3(tj, imageData, imageDataSize, &width, &height, &jpegSubsamp, &jpegColorspace);

        if (r != ERROR_SUCCESS)
        {
            LOGERROR(L"Unable to Decode jpeg header, error %s", TurboJPEGErrorToString(tj, r).c_str());
        }

        pitch = tjPixelSize[TJPF_RGB];

        {
            BYTE* temp = new BYTE[width * height * pitch];
            r = tjDecompress2(tj, imageData, imageDataSize, temp, width, width * pitch, height, TJPF_RGB, TJFLAG_ACCURATEDCT);

            if (r == ERROR_SUCCESS)
            {
                // Stretch the bytes out from byte to short
                m_pixelDataSize = width * height * pitch * sizeof(short);
                m_pixelData = new BYTE[m_pixelDataSize];
                memset(m_pixelData, 0, m_pixelDataSize);

                for (int s = 0; s < width * height * pitch; s++)
                {
                    m_pixelData[s * 2] = temp[s];
                }
            }
            else
            {
                LOGERROR(L"Unable to Decompress image, error %s", TurboJPEGErrorToString(tj, r).c_str());
            }

            delete[] temp;
        }

        r = tjDestroy(tj);

        if (r != ERROR_SUCCESS)
        {
            LOGERROR(L"Unable to Destroy TJ instance, error %s", TurboJPEGErrorToString(tj, r).c_str());
        }

        m_info->SetWidth(width);
        m_info->SetHeight(height);
        break;

    case OutputMode::JPEG:
        m_outputMode = OutputMode::PASSTHRU;
        // fall thru

    case OutputMode::PASSTHRU:
        LOGINFO(L"Output mode PASSTHRU...");
        // We're passing the raw in to out, so just shuffle the pointers
        m_pixelData = (GetData() + m_imageOffset);
        m_pixelDataSize = (GetDataLength() - m_imageOffset);
        break;

    default:
        LOGERROR(L"Unsupported output mode %d for JPEG file (only supports RGB, JPEG, PASSTHRU)", m_outputMode);
        break;
    }

    LOGTRACE(L"Out: Image:: ProcessJPEGData()");

    return m_pixelData != nullptr;
}

bool
Image::EnsurePixelsProcessed()
{
    bool result = false;

    LOGTRACE(L"In: EnsurePixelsProcessed..");
    if (!m_pixelData)
    {
        if (GetDataLength())
        {
            switch (m_inputMode)
            {
            case InputMode::ARW:
                result = ProcessARWData();
                break;

            case InputMode::JPEG:
                result = ProcessJPEGData();
                break;

            default:
                LOGERROR(L"Unknown incoming image format %d", (int)m_inputMode);
                break;
            }
        }
        else
        {
            LOGWARN(L"Unable to process image as it is NULL");
        }
    }
    else
    {
        result = true;
    }

    LOGTRACE(L"In: EnsurePixelsProcessed - returning %d", result);

    return result;
}

OutputMode
Image::GetOutputMode()
{
    return m_outputMode;
}

void
Image::SetOutputMode(OutputMode mode)
{
    m_outputMode = mode;
}

InputMode
Image::GetInputMode()
{
    return m_inputMode;
}

DWORD
Image::GetDataLength()
{
    LOGTRACE(L"In: Image::GetDataLength");

    DWORD result = GetMessageSize();

    LOGTRACE(L"Out: Image::GetDataLength - result %p", result);

    return result;
}

BYTE *
Image::GetData()
{
    LOGTRACE(L"In: Image::GetData");

    BYTE* result = GetRawMessage();

    LOGTRACE(L"Out: Image::GetData - result @ x%p", (void*)result);

    return result;
}

std::wstring
Image::WidenString(const char* shortString)
{
    static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

    return converter.from_bytes(shortString);
}

std::wstring
Image::LibRawErrorToString(LibRaw* libraw, int librawError)
{
    std::wostringstream builder;

    builder << WidenString(libraw->strerror(librawError)) << L" (" << librawError << L")";

    return builder.str();
}

std::wstring
Image::TurboJPEGErrorToString(tjhandle tj, int tjError)
{
    std::wostringstream builder;

    builder << WidenString(tjGetErrorStr2(tj)) << L" (" << tjError << L")";

    return builder.str();
}

DWORD
Image::GetWidth()
{
    return m_info->GetWidth();
}

DWORD
Image::GetHeight()
{
    return m_info->GetHeight();
}

bool
Image::StringEndsWith(const std::wstring& mainStr, const std::wstring& toMatch)
{
    if (mainStr.size() >= toMatch.size() &&
        mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void
Image::SaveFile(std::wstring path)
{
//    LOGTRACE(L"In: Image::SaveFile('%s')", path.c_str());
    registry.Open();

    // The first time we try to generate/use path from registry we save it into m_savePath.
    // This means that the path will not change until the DLL is reloaded.
    if (path.empty())
    {
        path = s_savePath;
    }

    if (path.empty())
    {
        registry.Open();

        std::wostringstream builder;

        if (registry.GetDWORD(L"", L"File Auto Save", 0))
        {
            path = registry.GetString(L"", L"File Save Path", L"");
        }

        if (!path.empty())
        {
            DWORD autoSaveAddDate = registry.GetDWORD(L"", L"File Save Path Add Date", 0);
            DWORD autoSaveCreateMultipleDirectories = registry.GetDWORD(L"", L"File Save Path Create Multiple Directories", 0);

            builder << path;

            if (autoSaveAddDate)
            {
                SYSTEMTIME now;

                GetLocalTime(&now);

                if (!StringEndsWith(path, L"\\"))
                {
                    builder << L"\\";
                }

                builder << now.wYear << L"-" << std::setfill(L'0') << std::setw(2) << now.wMonth << L"-" << std::setfill(L'0') << std::setw(2) << now.wDay;
            }

            int attempt = 0;

            do
            {
                std::wostringstream path_builder;

                path_builder << builder.str();

                if (attempt > 0)
                {
                    path_builder << L" (" << attempt << L")";
                }

                DWORD err = EnsureDirectory(path_builder.str());

                if (err != ERROR_SUCCESS)
                {
                    if (err == ERROR_ALREADY_EXISTS)
                    {
                        if (autoSaveCreateMultipleDirectories)
                        {
                            LOGWARN(L"Directory '%s' already exists, trying alternate", path_builder.str().c_str());
                            attempt += 1;
                        }
                        else
                        {
                            // Directory is present and flag not set - use it
                            s_savePath = path_builder.str();
                        }
                    }
                    else
                    {
                        // Cannot create directory (and doesn't already exist)
                        LOGERROR(L"Unable to create directory '%s': Error = 0x%x", path_builder.str().c_str(), err);
                        path_builder.clear();
                    }
                }
                else {
                    s_savePath = path_builder.str();
                }
            } while (s_savePath.empty() && attempt < 20);

            path = s_savePath;
        }

        registry.Close();
    }

    if (!path.empty())
    {
        std::wostringstream file_builder;
        
        file_builder << path;

        if (!StringEndsWith(path, L"\\"))
        {
            file_builder << L"\\";
        }

        file_builder << m_info->GetFilename();

        path = file_builder.str();
    }

    if (!path.empty())
    {
        LOGTRACE(L"Saving image as '%s'", path.c_str());

        // Create the file, overwriting if needed (this is needed for the preview JPEG's which are all same filename)
        HANDLE hfile = CreateFile(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hfile != INVALID_HANDLE_VALUE)
        {
            DWORD offset = 0;
            DWORD size = GetDataLength();
            DWORD bytesWritten = 0;

            if (m_inputMode == InputMode::JPEG)
            {
                offset = m_imageOffset;
                size = size - m_imageOffset;
            }

            WriteFile(hfile, GetData() + offset, size, &bytesWritten, nullptr);
            CloseHandle(hfile);
        }
        else
        {
            LOGWARN(L"Unable to open output file, error x%08x", GetLastError());
        }
    }
    else
    {
        LOGINFO(L"Did not save file, path not specified");
    }

//    LOGTRACE(L"Out: Image::SaveFile('%s')", path.c_str());
}

DWORD
Image::EnsureDirectory(const std::wstring dir)
{
    DWORD type = GetFileAttributes(dir.c_str());

    if (type == INVALID_FILE_ATTRIBUTES)
    {
        LOGTRACE(L"'%s' not found", dir.c_str());

        // Parent does not seem to exist, try to get parent of parent
        static const std::wstring separators(L"\\/");

        // If the specified directory name doesn't exist, do our thing
        std::size_t slashIndex = dir.find_last_of(separators);

        if (slashIndex != std::wstring::npos) {
            std::wstring p = dir.substr(0, slashIndex);

            LOGTRACE(L"Checking parent '%s'", p.c_str());

            if (EnsureDirectory(dir.substr(0, slashIndex)) == ERROR_ALREADY_EXISTS)
            {
                // Create the last directory on the path (the recursive calls will have taken
                // care of the parent directories by now)
                BOOL result = CreateDirectory(dir.c_str(), nullptr);

                LOGTRACE(L"Create '%s' %s", dir.c_str(), result ? L"Succeeded" : L"Failed");

                return result ? ERROR_SUCCESS : GetLastError();
            }
        }
        else
        {
            return ERROR_DIRECTORY;
        }
    }
    else if (!(type & FILE_ATTRIBUTE_DIRECTORY))
    {
        LOGERROR(L"Attempt to retrieve parent directory '%s' failed - type is not directory.", dir.c_str());

        return ERROR_BAD_PATHNAME;
    }
    else {
        return ERROR_ALREADY_EXISTS;
    }
}

void
Image::SetDuration(double duration)
{
    m_duration = duration;
}

double
Image::GetDuration()
{
    return m_duration;
}

DWORD
Image::GetRawWidth()
{
    return m_rawWidth;
}

DWORD
Image::GetRawHeight()
{
    return m_rawHeight;
}

DWORD
Image::GetCroppedWidth()
{
    return m_croppedWidth;
}

DWORD
Image::GetCroppedHeight()
{
    return m_croppedHeight;
}