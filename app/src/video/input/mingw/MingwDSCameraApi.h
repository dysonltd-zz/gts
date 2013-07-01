#ifndef MINGWDSCAMERAAPI_H_
#define MINGWDSCAMERAAPI_H_

#include "../CameraApi.h"

/** @brief The implementation of CameraApi using DirectShow on MinGW.
 *
 *  @todo These functions silently fail if errors occur from the DirectShow API.
 *  Should try to deal with the errors / report them, maybe.
 */
class MingwDSCameraApi : public CameraApi
{
public:
    MingwDSCameraApi();
    virtual ~MingwDSCameraApi();

    virtual const CameraList EnumerateCameras() const;
    virtual VideoSequence* const CreateVideoSequenceForCamera( const CameraDescription& camera ) const;
private:
};

#endif /* MINGWDSCAMERAAPI_H_ */
