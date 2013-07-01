#include "HardwareAbstraction.h"
#include "CameraHardware.h"
#include "MingwDSCameraApi.h"

#ifndef NDEBUG
# include "FakeCameraApi.h"
#endif

namespace HardwareAbstraction
{
    CameraHardware* const CreateCameraHardware()
    {
        CameraHardware* const newCameraHardware = new CameraHardware;

        std::unique_ptr< CameraApi > newMingwDSApi( new MingwDSCameraApi );
        newCameraHardware->AddApi( newMingwDSApi );
#ifndef NDEBUG
        std::unique_ptr< CameraApi > newFakeApi( new FakeCameraApi );
        newCameraHardware->AddApi( newFakeApi );
#endif
        return newCameraHardware;
    }
}
