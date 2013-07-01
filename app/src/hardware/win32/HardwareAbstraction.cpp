#include "HardwareAbstraction.h"
#include "CameraHardware.h"
#include "DirectShowCameraApi.h"

#ifndef NDEBUG
# include "FakeCameraApi.h"
#endif

namespace HardwareAbstraction
{
    CameraHardware* const CreateCameraHardware()
    {
        CameraHardware* const newCameraHardware = new CameraHardware;

        std::unique_ptr< CameraApi > newDSApi( new DirectShowCameraApi );
        newCameraHardware->AddApi( newDSApi );
#ifndef NDEBUG
        std::unique_ptr< CameraApi > newFakeApi( new FakeCameraApi );
        newCameraHardware->AddApi( newFakeApi );
#endif
        return newCameraHardware;
    }
}