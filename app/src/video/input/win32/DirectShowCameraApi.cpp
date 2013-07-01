#include "DirectShowCameraApi.h"

#include <cstdlib>
#include <windows.h>
#include <dshow.h>
#include <cassert>
#include <comutil.h>
#include "VideoCaptureCv.h"
#include "CameraDescription.h"

namespace
{
    /** @brief Record of CameraDescription plus an interface-pointer
     *  to the DShow filter that represents the camera. Implements COM
     *  ref-counting manually because we have to build with Visual
     *  Studio Express, which does not include ATL.
     */
    struct DShowCamera
    {
        DShowCamera(const CameraDescription& descr, IBaseFilter* pf)
            : description(descr),
              pFilter(pf)
        {
            if (pFilter)
                pFilter->AddRef();
        }

        DShowCamera(const DShowCamera& other)
            : description(other.description),
              pFilter(other.pFilter)
        {
            if (pFilter)
                pFilter->AddRef();
        }
        
        DShowCamera& operator=(const DShowCamera& other)
        {
            description = other.description;
            if (pFilter)
                pFilter->Release();
            pFilter = other.pFilter;
            if (pFilter)
                pFilter->AddRef();
            return *this;
        }
        
        ~DShowCamera()
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

    typedef std::vector<DShowCamera> DShowCameraList;
    
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
        HRESULT hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
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

                        resolutions.insert( CameraDescription::Resolution(pVih->bmiHeader.biWidth, pVih->bmiHeader.biHeight) );
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
        HRESULT hr = FindPinInterface(pFilter, PIN_CATEGORY_PREVIEW, IID_PPV_ARGS(&pStreamConfig));
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
                    std::cerr << "Failed to set camera resolution HR=" << std::hex << hr << std::dec << std::endl;
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
        HRESULT result = CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS( &systemDeviceEnumerator ) );

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
                         DShowCameraList& cameraList,
                         const CameraApi& directShowApi )
    {
        IMoniker* videoDeviceMoniker;

        const ULONG GET_ONE = 1;
        ULONG* const UNNECESSARY = NULL;

        size_t directShowCameraIndex = 0;
        while ( videoDeviceEnumerator->Next( GET_ONE, &videoDeviceMoniker, UNNECESSARY ) == S_OK)
        {
            IBindCtx* const UNUSED_BINDCTX = NULL;
            IMoniker* const UNUSED_MONIKER = NULL;
            IPropertyBag* videoDeviceProperties = NULL;
            HRESULT hr = videoDeviceMoniker->BindToStorage( UNUSED_BINDCTX, UNUSED_MONIKER, IID_PPV_ARGS( &videoDeviceProperties ) );
            if ( SUCCEEDED(hr) )
            {
                CameraDescription descr = 
                    CameraDescription( directShowApi )
                    .WithName        ( GetProperty( L"FriendlyName", videoDeviceProperties ) )
                    .WithDescription ( GetProperty( L"Description",  videoDeviceProperties ) )
                    .WithUniqueId    ( GetProperty( L"DevicePath",   videoDeviceProperties ) );
                videoDeviceProperties->Release();

                // Try to get the IAMStreamConfig interface on the
                // camera. First we need to get an IBaseFilter...
                IBaseFilter* pFilter = NULL;
                hr = videoDeviceMoniker->BindToObject( UNUSED_BINDCTX, UNUSED_MONIKER, IID_PPV_ARGS( &pFilter ) );
                if (SUCCEEDED(hr))
                {
                    IAMStreamConfig* pStreamConfig = NULL;
                    hr = FindPinInterface(pFilter, PIN_CATEGORY_PREVIEW, IID_PPV_ARGS(&pStreamConfig));
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

                cameraList.push_back( DShowCamera(descr, pFilter) );

                if (pFilter)
                    pFilter->Release();
            }
            
            videoDeviceMoniker->Release();
            ++directShowCameraIndex;
        }
    }

    const DShowCameraList EnumerateDShowCameras(const DirectShowCameraApi& api)
    {
        DShowCameraList cameraList;
        
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

DirectShowCameraApi::DirectShowCameraApi()
{
}

DirectShowCameraApi::~DirectShowCameraApi()
{
}

/** @copydoc CameraApi::EnumerateCameras()
 *
 */
const CameraApi::CameraList DirectShowCameraApi::EnumerateCameras() const
{
    CameraList cameraList;

    HRESULT hr = InitComLibrary();
    if (SUCCEEDED(hr))
    {
        DShowCameraList dsCameraList = EnumerateDShowCameras(*this);

        for (size_t n = 0; n < dsCameraList.size(); ++n)
        {
            cameraList.push_back( dsCameraList[n].description );
        }

        ResetComLibrary();
    }
    return cameraList;
}

/** @copydoc CameraApi::CreateVideoSequenceForCamera()
 *
 */
VideoSequence* const DirectShowCameraApi::CreateVideoSequenceForCamera( const CameraDescription& camera ) const
{
    VideoSequence* vs = NULL;
    
    HRESULT hr = InitComLibrary();
    if (SUCCEEDED(hr))
    {
        DShowCameraList dsCameraList = EnumerateDShowCameras(*this);

        for (size_t n = 0; n < dsCameraList.size(); ++n)
        {
            if (dsCameraList[n].description.UniqueId() == camera.UniqueId() )
            {
                // found the matching camera; now set it to the
                // highest available resolution
                CameraDescription::Resolution res = GetHighestResolution(dsCameraList[n].description);
                if ((res.width > 0) && (res.height > 0))
                {
                    SetResolution(dsCameraList[n].pFilter, res);
                }
                vs = new VideoCaptureCv( CV_CAP_DSHOW + static_cast<int>( n ) );
                break;
            }
        }

        ResetComLibrary();
    }
    return vs;
}


