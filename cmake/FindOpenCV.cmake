#
# The following variables are optionally searched for:
#       OpenCV_ROOT_DIR         Base directory of OpenCV 2 tree to use.
#

include(FindPackageHandleStandardArgs)

# If Windows or MinGW, path should always be set
if(WIN32 OR MINGW)

    find_path(OpenCV2_ROOT_DIR
		      NAMES include/opencv2/opencv.hpp
		      PATHS "${OpenCV_ROOT_DIR}"
			  NO_DEFAULT_PATH
	)

# If Unix, first try and use command line option. Then try typical locations
else()

    if(OpenCV_ROOT_DIR)
        find_path(OpenCV2_ROOT_DIR
				  NAMES include/opencv2/opencv.hpp
				  PATHS "${OpenCV_ROOT_DIR}"
				  NO_DEFAULT_PATH
		)
    else()

        set(OpenCV2_POSSIBLE_ROOT_DIRS
				/usr/local                                      # Linux: default dir by CMake
				/usr                                            # Linux
				/opt/local                                      # OS X: default MacPorts location
		)
        find_path(OpenCV2_ROOT_DIR
				  NAMES include/opencv2/opencv.hpp
				  PATHS "${OpenCV2_POSSIBLE_ROOT_DIRS}"
		)
    endif()

endif()

if(WIN32 OR MINGW)
	set( OpenCV2_CORE_INCLUDE_DIR  "${OpenCV2_ROOT_DIR}/modules/core/include" )
	set( OpenCV2_IMGPROC_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/imgproc/include" )
	set( OpenCV2_FEATURES2D_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/features2d/include" )
	set( OpenCV2_FLANN_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/flann/include" )
	set( OpenCV2_CALIB3D_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/calib3d/include" )
	set( OpenCV2_OBJDETECT_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/objdetect/include" )
	set( OpenCV2_LEGACY_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/legacy/include" )
	set( OpenCV2_CONTRIB_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/contrib/include" )
	set( OpenCV2_HIGHGUI_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/highgui/include" )
	set( OpenCV2_ML_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/ml/include" )
	set( OpenCV2_VIDEO_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/video/include" )
	set( OpenCV2_GPU_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/modules/gpu/include" )
else()
	set( OpenCV2_CORE_INCLUDE_DIR  "${OpenCV2_ROOT_DIR}/include/opencv2/core" )
	set( OpenCV2_IMGPROC_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/imgproc" )
	set( OpenCV2_FEATURES2D_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/features2d" )
	set( OpenCV2_FLANN_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/flann" )
	set( OpenCV2_CALIB3D_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/calib3d" )
	set( OpenCV2_OBJDETECT_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/objdetect" )
	set( OpenCV2_LEGACY_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/legacy" )
	set( OpenCV2_CONTRIB_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/contrib" )
	set( OpenCV2_HIGHGUI_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/highgui" )
	set( OpenCV2_ML_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/ml" )
	set( OpenCV2_VIDEO_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/video" )
	set( OpenCV2_GPU_INCLUDE_DIR "${OpenCV2_ROOT_DIR}/include/opencv2/gpu" )
endif()

if(WIN32 OR MINGW)
    set(OPENCV2_LIBRARY_SEARCH_PATHS
        "${OpenCV2_ROOT_DIR}/build/lib"
        "${OpenCV2_ROOT_DIR}/build/lib"
        "${OpenCV2_ROOT_DIR}/build/lib"
        "${OpenCV2_ROOT_DIR}/build/x86/vc10/lib"
        "${OpenCV2_ROOT_DIR}/build/x86/vc10/lib/release"
        "${OpenCV2_ROOT_DIR}/build/x86/vc10/lib/debug"
        )

    find_library(OpenCV2_CORE_LIBRARY
                 NAMES opencv_core246 opencv_core240 opencv_core220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_IMGPROC_LIBRARY
                 NAMES opencv_imgproc246 opencv_imgproc240 opencv_imgproc220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_FEATURES2D_LIBRARY
                 NAMES opencv_features2d246 opencv_features2d240 opencv_features2d220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_FLANN_LIBRARY
                 NAMES opencv_flann246 opencv_flann240 opencv_flann220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_CALIB3D_LIBRARY
                 NAMES opencv_calib3d246 opencv_calib3d240 opencv_calib3d220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_OBJDETECT_LIBRARY
                 NAMES opencv_objdetect246 opencv_objdetect240 opencv_objdetect220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_LEGACY_LIBRARY
                 NAMES opencv_legacy246 opencv_legacy240 opencv_legacy220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_CONTRIB_LIBRARY
                 NAMES opencv_contrib246 opencv_contrib240 opencv_contrib220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_HIGHGUI_LIBRARY
                 NAMES opencv_highgui246 opencv_highgui240 opencv_highgui220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_ML_LIBRARY
                 NAMES opencv_ml246 opencv_ml240 opencv_ml220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_VIDEO_LIBRARY
                 NAMES opencv_video246 opencv_video240 opencv_video220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_GPU_LIBRARY
                 NAMES opencv_gpu246 opencv_gpu240 opencv_gpu220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_FFMPEG_LIBRARY
                 NAMES opencv_ffmpeg246 opencv_ffmpeg240 opencv_ffmpeg220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
    find_library(OpenCV2_TS_LIBRARY
                 NAMES opencv_ts246 opencv_ts240 opencv_ts220
                 PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS}
				 NO_DEFAULT_PATH
                 )
else()
    set(OPENCV2_LIBRARY_SEARCH_PATHS
        "${OpenCV2_ROOT_DIR}/lib"
        "${OpenCV2_ROOT_DIR}/lib/release"
        "${OpenCV2_ROOT_DIR}/lib/debug"
        )
    find_library(OpenCV2_CORE_LIBRARY       NAMES opencv_core       PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_IMGPROC_LIBRARY    NAMES opencv_imgproc    PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_FEATURES2D_LIBRARY NAMES opencv_features2d PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_FLANN_LIBRARY      NAMES opencv_flann      PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_CALIB3D_LIBRARY    NAMES opencv_calib3d    PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_OBJDETECT_LIBRARY  NAMES opencv_objdetect  PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_LEGACY_LIBRARY     NAMES opencv_legacy     PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_CONTRIB_LIBRARY    NAMES opencv_contrib    PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_HIGHGUI_LIBRARY    NAMES opencv_highgui    PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_ML_LIBRARY         NAMES opencv_ml         PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_VIDEO_LIBRARY      NAMES opencv_video      PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_library(OpenCV2_GPU_LIBRARY        NAMES opencv_gpu        PATHS ${OPENCV2_LIBRARY_SEARCH_PATHS} NO_DEFAULT_PATH)
endif()

set(OpenCV2_INCLUDE_DIRS
    ${OpenCV2_ROOT_DIR}/include
    ${OpenCV2_ROOT_DIR}/include/opencv
    ${OpenCV2_ROOT_DIR}/include/opencv2
    ${OpenCV2_CORE_INCLUDE_DIR}
    ${OpenCV2_IMGPROC_INCLUDE_DIR}
    ${OpenCV2_FEATURES2D_INCLUDE_DIR}
    ${OpenCV2_FLANN_INCLUDE_DIR}
    ${OpenCV2_CALIB3D_INCLUDE_DIR}
    ${OpenCV2_OBJDETECT_INCLUDE_DIR}
    ${OpenCV2_LEGACY_INCLUDE_DIR}
    ${OpenCV2_CONTRIB_INCLUDE_DIR}
    ${OpenCV2_HIGHGUI_INCLUDE_DIR}
    ${OpenCV2_ML_INCLUDE_DIR}
    ${OpenCV2_VIDEO_INCLUDE_DIR}
    ${OpenCV2_GPU_INCLUDE_DIR}
    )

set(OpenCV2_LIBRARIES
    ${OpenCV2_CORE_LIBRARY}
    ${OpenCV2_IMGPROC_LIBRARY}
    ${OpenCV2_FEATURES2D_LIBRARY}
    ${OpenCV2_FLANN_LIBRARY}
    ${OpenCV2_CALIB3D_LIBRARY}
    ${OpenCV2_OBJDETECT_LIBRARY}
    ${OpenCV2_LEGACY_LIBRARY}
    ${OpenCV2_CONTRIB_LIBRARY}
    ${OpenCV2_HIGHGUI_LIBRARY}
    ${OpenCV2_ML_LIBRARY}
    ${OpenCV2_VIDEO_LIBRARY}
    ${OpenCV2_GPU_LIBRARY}
    )
	
if(WIN32 OR MINGW)
    set(OpenCV2_INCLUDE_DIRS
        ${OpenCV2_INCLUDE_DIRS}
        )
    set(OpenCV2_LIBRARIES
        ${OpenCV2_LIBRARIES}
        ${OpenCV2_TS_LIBRARY}
        )
endif()

find_package_handle_standard_args(OpenCV2 "Could not find OpenCV2" OpenCV2_LIBRARIES OpenCV2_INCLUDE_DIRS)
