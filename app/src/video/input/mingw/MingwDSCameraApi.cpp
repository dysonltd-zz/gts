#include "MingwDSCameraApi.h"

#include "VideoCaptureCv.h"
#include "CameraDescription.h"

#include "Logging.h"

#include <windows.h>
#include <amvideo.h>
#include <initguid.h>
#include <objbase.h>
#include <objidl.h>
#include <ocidl.h>
#include <errors.h>

#define WIDTHBYTES(BTIS)  ((DWORD)(((BTIS)+31) & (~31)) / 8)
#define DIBWIDTHBYTES(BI) (DWORD)(BI).biBitCount) * (DWORD)WIDTHBYTES((DWORD)(BI).biWidth
#define _DIBSIZE(BI)      (DIBWIDTHBYTES(BI) * (DWORD)(BI).biHeight)
#define DIBSIZE(BI)       ((BI).biHeight < 0 ? (-1)*(_DIBSIZE(BI)) : _DIBSIZE(BI))

DEFINE_GUID( CLSID_VideoInputDeviceCategory, 0x860BB310, 0x5D01,
             0x11d0, 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86);
DEFINE_GUID( CLSID_SystemDeviceEnum, 0x62BE5D10, 0x60EB, 0x11d0,
             0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 );
DEFINE_GUID( IID_IBaseFilter, 0x56a86895, 0x0ad4, 0x11ce,
             0xb0, 0x3a, 0x00, 0x20, 0xaf, 0x0b, 0xa7, 0x70 );
DEFINE_GUID( IID_ICreateDevEnum, 0x29840822, 0x5b84, 0x11d0,
             0xbd, 0x3b, 0x00, 0xa0, 0xc9, 0x11, 0xce, 0x86 );
DEFINE_GUID( IID_IAMStreamConfig, 0xc6e13340, 0x30ac, 0x11d0,
             0xa1, 0x8c, 0x00, 0xa0, 0xc9, 0x11, 0x89, 0x56 );
DEFINE_GUID( IID_IKsPropertySet, 0x31EFAC30L, 0x515C, 0x11d0,
             0xA9, 0xAA, 0x00, 0xAA, 0x00, 0x61, 0xBE, 0x93 );
DEFINE_GUID( AMPROPSETID_Pin, 0x9b00f101, 0x1567, 0x11d1,
             0xb3, 0xf1, 0x0, 0xaa, 0x0, 0x37, 0x61, 0xc5 );
DEFINE_GUID( PIN_CATEGORY_PREVIEW, 0xfb6c4282, 0x0353, 0x11d1,
             0x90, 0x5f, 0x00, 0x00, 0xc0, 0xcc, 0x16, 0xba );
DEFINE_GUID (FORMAT_VideoInfo, 0x05589f80, 0xc356, 0x11ce,
             0xbf, 0x01, 0x00, 0xaa, 0x00, 0x55, 0x59, 0x5a );

#define AMPROPERTY_PIN_CATEGORY 0

#define INTERFACE IAMStreamConfig
DECLARE_INTERFACE_(IAMStreamConfig,IUnknown)
{
   STDMETHOD(QueryInterface)(THIS_ REFIID,PVOID*) PURE;
   STDMETHOD_(ULONG,AddRef)(THIS) PURE;
   STDMETHOD_(ULONG,Release)(THIS) PURE;
   STDMETHOD(SetFormat)(THIS_ AM_MEDIA_TYPE*) PURE;
   STDMETHOD(GetFormat)(THIS_ AM_MEDIA_TYPE**) PURE;
   STDMETHOD(GetNumberOfCapabilities)(THIS_ int*,int*) PURE;
   STDMETHOD(GetStreamCaps)(THIS_ int,AM_MEDIA_TYPE**,BYTE*) PURE;
};
#undef INTERFACE

#define INTERFACE IKsPropertySet
DECLARE_INTERFACE_(IKsPropertySet,IUnknown)
{
  STDMETHOD(Set)(THIS_ REFIID,ULONG,LPVOID,ULONG,LPVOID,ULONG) PURE;
  STDMETHOD(Get)(THIS_ REFIID,ULONG,LPVOID,ULONG,LPVOID,ULONG,ULONG*) PURE;
  STDMETHOD(QuerySupported)(THIS_ REFIID,ULONG,ULONG*) PURE;
};
#undef INTERFACE

namespace
{
    /** @brief Record of CameraDescription plus an interface-pointer
     *  to the DShow filter that represents the camera. Implements COM
     *  ref-counting manually because we have to build with Visual
     *  Studio Express, which does not include ATL.
     */
    struct MingwDSCamera
    {
        MingwDSCamera(const CameraDescription& descr, IBaseFilter* pf)
            : description(descr),
              pFilter(pf)
        {
            if (pFilter)
                pFilter->AddRef();
        }

        MingwDSCamera(const MingwDSCamera& other)
            : description(other.description),
              pFilter(other.pFilter)
        {
            if (pFilter)
                pFilter->AddRef();
        }

        MingwDSCamera& operator=(const MingwDSCamera& other)
        {
            description = other.description;
            if (pFilter)
                pFilter->Release();
            pFilter = other.pFilter;
            if (pFilter)
                pFilter->AddRef();
            return *this;
        }

        ~MingwDSCamera()
        {
            if (pFilter)
            {
                pFilter->Release();
                pFilter = 0;
            }
        }

        CameraDescription    description;
        IBaseFilter*         pFilter;
    };

    typedef std::vector<MingwDSCamera> MingwDSCameraList;

    /** @brief Release the format block for a media type.
     *
     * Implemented here so we don't need to link to the DShow
     * base-class libraries.
     */
    void FreeMediaType(AM_MEDIA_TYPE& mt)
    {
        if (mt.cbFormat != 0)
        {
            CoTaskMemFree((PVOID)mt.pbFormat);
            mt.cbFormat = 0;
            mt.pbFormat = NULL;
        }
        if (mt.pUnk != NULL)
        {
            // pUnk should not be used.
            mt.pUnk->Release();
            mt.pUnk = NULL;
        }
    }

    /** @brief Delete a media type structure that was allocated on the
     * heap.
     *
     * Implemented here so we don't need to link to the DShow
     * base-class libraries.
     */
    void DeleteMediaType(AM_MEDIA_TYPE *pmt)
    {
        if (pmt != NULL)
        {
            FreeMediaType(*pmt);
            CoTaskMemFree(pmt);
        }
    }

    bool PinMatchesCategory(IPin *pPin, REFGUID category)
    {
        bool bFound = false;

        IKsPropertySet *pKs = NULL;
#if defined(__MINGW32__) || defined(__GNUC__)
        HRESULT hr = pPin->QueryInterface(IID_IKsPropertySet, reinterpret_cast<void**>(&pKs));
#else
        HRESULT hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
#endif
        if (SUCCEEDED(hr))
        {
            GUID pinCategory;
            DWORD cbReturned;
            hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
                          &pinCategory, sizeof(GUID), &cbReturned);
            if (SUCCEEDED(hr) && (cbReturned == sizeof(GUID)))
            {
                bFound = (pinCategory == category);
            }
            pKs->Release();
        }
        return bFound;
    }

    /** @brief Find a specific COM-interface on a DShow filter.
     *  @param pFilter the filter to search
     *  @param iid the IID of the interface to search for
     *  @param ppUnk receives the interface pointer
     */
    HRESULT FindPinInterface(IBaseFilter *pFilter,
                             REFGUID category,
                             REFGUID iid,
                             void **ppUnk)
    {
        if (!pFilter || !ppUnk)
            return E_POINTER;

        HRESULT hr = E_FAIL;
        IEnumPins *pEnum = 0;
        if (FAILED(pFilter->EnumPins(&pEnum)))
            return E_FAIL;

        // Query every pin for the interface.
        IPin *pPin = 0;
        while (S_OK == pEnum->Next(1, &pPin, 0))
        {
            if (PinMatchesCategory(pPin, category))
            {
                hr = pPin->QueryInterface(iid, ppUnk);
                pPin->Release();
                if (SUCCEEDED(hr))
                    break;
            }
        }
        pEnum->Release();
        return hr;
    }

    HRESULT GetAvailableResolutions(IAMStreamConfig* pStreamConfig,
                                    CameraDescription::Resolutions& resolutions)
    {
        int iCount=0, iSize=0;
        HRESULT hr = pStreamConfig->GetNumberOfCapabilities(&iCount, &iSize);
        if (FAILED(hr))
            return hr;

        if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS))
        {
            for (int i=0; i < iCount; i++)
            {
                VIDEO_STREAM_CONFIG_CAPS scc;
                AM_MEDIA_TYPE *pmt = NULL;
                hr = pStreamConfig->GetStreamCaps(i, &pmt, (BYTE*)&scc);
                if (SUCCEEDED(hr))
                {
                    if (pmt->formattype == FORMAT_VideoInfo)
                    {
                        VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;

                        resolutions.insert( CameraDescription::Resolution(pVih->bmiHeader.biWidth,
                                                                          pVih->bmiHeader.biHeight) );
                    }
                    DeleteMediaType(pmt);
                }
            }
        }

        return S_OK;
    }

    HRESULT SetResolution(IBaseFilter* pFilter, const CameraDescription::Resolution& resolution)
    {
        IAMStreamConfig* pStreamConfig = NULL;
#if defined(__MINGW32__) || defined(__GNUC__)
        HRESULT hr = FindPinInterface(pFilter, PIN_CATEGORY_PREVIEW, IID_IAMStreamConfig, reinterpret_cast<void**>(&pStreamConfig));
#else
        HRESULT hr = FindPinInterface(pFilter, PIN_CATEGORY_PREVIEW, IID_PPV_ARGS(&pStreamConfig));
#endif
        if (FAILED(hr))
            return hr;

        AM_MEDIA_TYPE *pmt = NULL;
        hr = pStreamConfig->GetFormat(&pmt);
        if (SUCCEEDED(hr))
        {
            if (pmt->formattype == FORMAT_VideoInfo)
            {
                VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;

                pVih->bmiHeader.biWidth  = resolution.width;
                pVih->bmiHeader.biHeight = resolution.height;
                pVih->bmiHeader.biSizeImage = DIBSIZE(pVih->bmiHeader);

                hr = pStreamConfig->SetFormat(pmt);
                if (FAILED(hr))
                {
                    LOG_ERROR(QObject::tr("Failed to set camera resolution (%1)!").arg(hr));
                }
            }
            DeleteMediaType(pmt);
        }

        pStreamConfig->Release();
        return S_OK;
    }

    /** Get the DirectShow device enumerator for the video device category.
     *
     *  @param [out] videoDeviceEnumerator The enumerator for the video device category.
     *  @return An @a HRESULT to show if the operation was successful. Fails if we
     *  can't create the system device enumerator, or it there are no video devices available.
     */
    HRESULT GetVideoDeviceCategoryEnumerator( IEnumMoniker** videoDeviceEnumerator )
    {
        ICreateDevEnum* systemDeviceEnumerator = NULL;
#if defined(__MINGW32__) || defined(__GNUC__)
        HRESULT result = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&systemDeviceEnumerator) );
#else
        HRESULT result = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &systemDeviceEnumerator ) );
#endif

        if ( SUCCEEDED( result ) )
        {
            const DWORD ENUMERATE_ALL = 0;
            result = systemDeviceEnumerator->CreateClassEnumerator( CLSID_VideoInputDeviceCategory,
                                                                    videoDeviceEnumerator,
                                                                    ENUMERATE_ALL );

            if ( result == S_FALSE )
            {
                //There weren't any devices found of this category -- treat as an error to
                //stop processing
                result = VFW_E_NOT_FOUND;
            }
            systemDeviceEnumerator->Release();
        }
        return result;
    }

    /** @brief Get a camera property.
     *
     *  @param propertyName The name used by DirectShow for the property.
     *  @param properties   The IPropertyBag interface from the video device.
     *
     *  @return The property value as a @a std::wstring.
     */
    const std::wstring GetProperty( const std::wstring& propertyName, IPropertyBag* const properties )
    {
        std::wstring propertyValue;
        VARIANT propertyVariant;
        VariantInit( &propertyVariant );

        IErrorLog* const IGNORE_ERRORS = NULL;

        const HRESULT result = properties->Read( propertyName.c_str(), &propertyVariant, IGNORE_ERRORS );

        if ( SUCCEEDED( result ) )
        {
            if ( propertyVariant.bstrVal )
            {
                propertyValue = propertyVariant.bstrVal;
                VariantClear( &propertyVariant );
            }
        }
        return propertyValue;
    }

    /** @brief Fill out a list of cameras by iterating using the provided video device enumerator.
     *
     *  @param videoDeviceEnumerator An IEnumMoniker pointing to an device enumerator for the video device class.
     *  @param cameraList            The list to populate with cameras.
     *  @param directShowApi         A reference to the API object enumerating the devices.
     */
    void FillCameraList( IEnumMoniker* const videoDeviceEnumerator,
                         MingwDSCameraList& cameraList,
                         const CameraApi& mingwDSApi )
    {
        IMoniker* videoDeviceMoniker;

        const ULONG GET_ONE = 1;
        ULONG* const UNNECESSARY = NULL;

        size_t mingwDSCameraIndex = 0;

        while ( videoDeviceEnumerator->Next( GET_ONE, &videoDeviceMoniker, UNNECESSARY ) == S_OK)
        {
            IBindCtx* const UNUSED_BINDCTX = NULL;
            IMoniker* const UNUSED_MONIKER = NULL;
            IPropertyBag* videoDeviceProperties = NULL;

#if defined(__MINGW32__) || defined(__GNUC__)
            HRESULT hr = videoDeviceMoniker->BindToStorage( UNUSED_BINDCTX, UNUSED_MONIKER, IID_IPropertyBag, reinterpret_cast<void**>(&videoDeviceProperties) );
#else
            HRESULT hr = videoDeviceMoniker->BindToStorage( UNUSED_BINDCTX, UNUSED_MONIKER, IID_PPV_ARGS( &videoDeviceProperties ) );
#endif

            if ( SUCCEEDED(hr) )
            {
                CameraDescription descr =
                    CameraDescription( mingwDSApi )
                    .WithName        ( GetProperty( L"FriendlyName", videoDeviceProperties ) )
                    .WithDescription ( GetProperty( L"Description",  videoDeviceProperties ) )
                    .WithUniqueId    ( GetProperty( L"DevicePath",   videoDeviceProperties ) );
                videoDeviceProperties->Release();

                // Try to get the IAMStreamConfig interface on the
                // camera. First we need to get an IBaseFilter...
                IBaseFilter* pFilter = NULL;
#if defined(__MINGW32__) || defined(__GNUC__)
                hr = videoDeviceMoniker->BindToObject( UNUSED_BINDCTX, UNUSED_MONIKER, IID_IBaseFilter, reinterpret_cast<void**>(&pFilter) );
#else
                hr = videoDeviceMoniker->BindToObject( UNUSED_BINDCTX, UNUSED_MONIKER, IID_PPV_ARGS( &pFilter ) );
#endif

                if (SUCCEEDED(hr))
                {
                    IAMStreamConfig* pStreamConfig = NULL;
#if defined(__MINGW32__) || defined(__GNUC__)
                    hr = FindPinInterface(pFilter, PIN_CATEGORY_PREVIEW, IID_IAMStreamConfig, reinterpret_cast<void**>(&pStreamConfig));
#else
                    hr = FindPinInterface(pFilter, PIN_CATEGORY_PREVIEW, IID_PPV_ARGS(&pStreamConfig));
#endif

                    if (SUCCEEDED(hr))
                    {
                        // get available resolutions from the
                        // stream-config interface
                        CameraDescription::Resolutions resolutions;

                        hr = GetAvailableResolutions(pStreamConfig, resolutions);
                        if (SUCCEEDED(hr))
                        {
                            descr.SetResolutions(resolutions);
                        }
                        pStreamConfig->Release();
                    }
                }

                cameraList.push_back( MingwDSCamera(descr, pFilter) );

                if (pFilter)
                    pFilter->Release();
            }

            videoDeviceMoniker->Release();
            ++mingwDSCameraIndex;
        }
    }

    const MingwDSCameraList EnumerateMingwDSCameras(const MingwDSCameraApi& api)
    {
        MingwDSCameraList cameraList;

        IEnumMoniker* videoDeviceCategoryEnumerator = NULL;

        HRESULT result = GetVideoDeviceCategoryEnumerator( &videoDeviceCategoryEnumerator );

        if ( SUCCEEDED( result ) )
        {
            FillCameraList( videoDeviceCategoryEnumerator, cameraList, api );
            videoDeviceCategoryEnumerator->Release();
        }

        return cameraList;
    }

    CameraDescription::Resolution GetHighestResolution(const CameraDescription& camera)
    {
        CameraDescription::Resolution res;

        CameraDescription::Resolutions resolutions = camera.GetResolutions();
        for (CameraDescription::Resolutions::const_iterator i = resolutions.begin(); i != resolutions.end(); ++i)
        {
            if (res < *i)
            {
                res = *i;
            }
        }

        return res;
    }

    /** @brief Initialise the COM library.
     *
     */
    const HRESULT InitComLibrary()
    {
        const LPVOID RESERVED = NULL; //Must be null as reserved in API
        return CoInitializeEx( RESERVED, COINIT_APARTMENTTHREADED );
    }

    /** @brief Uninitialise the COM library.
     *
     */
    void ResetComLibrary()
    {
        CoUninitialize();
    }

}

MingwDSCameraApi::MingwDSCameraApi()
{
}

MingwDSCameraApi::~MingwDSCameraApi()
{
}

/** @copydoc CameraApi::EnumerateCameras()
 *
 */
const CameraApi::CameraList MingwDSCameraApi::EnumerateCameras() const
{
    CameraList cameraList;

    HRESULT hr = InitComLibrary();
    if (SUCCEEDED(hr))
    {
        MingwDSCameraList mdsCameraList = EnumerateMingwDSCameras(*this);

        for (size_t n = 0; n < mdsCameraList.size(); ++n)
        {
            cameraList.push_back( mdsCameraList[n].description );
        }

        ResetComLibrary();
    }
    return cameraList;
}

/** @copydoc CameraApi::CreateVideoSequenceForCamera()
 *
 */
VideoSequence* const MingwDSCameraApi::CreateVideoSequenceForCamera( const CameraDescription& camera ) const
{
    VideoSequence* vs = NULL;

    HRESULT hr = InitComLibrary();
    if (SUCCEEDED(hr))
    {
        MingwDSCameraList mdsCameraList = EnumerateMingwDSCameras(*this);

        for (size_t n = 0; n < mdsCameraList.size(); ++n)
        {
            if (mdsCameraList[n].description.UniqueId() == camera.UniqueId() )
            {
                // found the matching camera; now set it to the
                // highest available resolution
                CameraDescription::Resolution res = GetHighestResolution(mdsCameraList[n].description);

                if ((res.width > 0) && (res.height > 0))
                {
                    SetResolution(mdsCameraList[n].pFilter, res);
                }

                vs = new VideoCaptureCv( CV_CAP_DSHOW + static_cast<int>( n ) );

                break;
            }
        }

        ResetComLibrary();
    }
    return vs;
}


