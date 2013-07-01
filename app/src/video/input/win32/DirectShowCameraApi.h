#ifndef DIRECTSHOWCAMERAAPI_H_
#define DIRECTSHOWCAMERAAPI_H_

#include "../CameraApi.h"

/** @brief The implementation of CameraApi using DirectShow on Windows.
 *
 *  @todo These functions silently fail if errors occur from the DirectShow API.
 *  Should try to deal with the errors / report them, maybe.
 */
class DirectShowCameraApi : public CameraApi
{
public:
    DirectShowCameraApi();
    virtual ~DirectShowCameraApi();

    virtual const CameraList EnumerateCameras() const;
    virtual VideoSequence* const CreateVideoSequenceForCamera( const CameraDescription& camera ) const;

private:
};

#endif /* DIRECTSHOWCAMERAAPI_H_ */
