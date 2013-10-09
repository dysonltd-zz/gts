/*
 * Copyright (C) 2007-2013 Dyson Technology Ltd, all rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "RoomsCollection.h"
#include "CamerasCollection.h"
#include "CameraPositionsCollection.h"
#include "CalibrationSchema.h"
#include "ExtrinsicCalibrationSchema.h"
#include "CameraPositionSchema.h"
#include "RoomLayoutSchema.h"
#include "FloorPlanSchema.h"
#include "CalibrationAlgorithm.h"
#include "WbConfigTools.h"
#include "WbConfig.h"
#include "GroundPlaneUtility.h"
#include "OpenCvUtility.h"
#include "RobotMetrics.h"
#include "CameraCalibration.h"
#include "FileUtilities.h"
#include "FileDialogs.h"
#include "Message.h"
#include "Logging.h"

#include <QFileDialog>
#include <QtGlobal>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <iostream>
#include <algorithm>

namespace FloorPlanning
{
    bool LoadFile( WbConfig config, KeyId cameraPosition, IplImage** camImg, QString fileName, CvPoint2D32f* offset, bool unWarp )
    {
        bool successful = true;

        // Get configuration information
        Collection camerasCollection( CamerasCollection() );
        Collection cameraPositionsCollection( CameraPositionsCollection() );

        camerasCollection.SetConfig( config );
        cameraPositionsCollection.SetConfig( config );

        const KeyId camPosId = cameraPosition;

        LOG_INFO(QObject::tr("Camera position id: %1").arg(camPosId));

        const WbConfig camPosConfig = cameraPositionsCollection.ElementById( camPosId );

        if (camPosConfig.IsNull()) successful = false;

        CvMat* cameraMtx = cvCreateMat( 3, 3, CV_32F );
        CvMat* distortionCoeffs = cvCreateMat( 5, 1, CV_32F );
        CvMat* inverseCoeffs = cvCreateMat( 5, 1, CV_32F );

        CvMat* rot = cvCreateMat( 3, 3, CV_32F );
        CvMat* trans = cvCreateMat( 1, 3, CV_32F );

        if (successful)
        {
            const KeyId camId = camPosConfig.GetKeyValue(CameraPositionSchema::cameraIdKey).ToKeyId();

            LOG_INFO(QObject::tr("Camera id: %1").arg(camId));

            WbConfig cameraConfig = camerasCollection.ElementById( camId );

            if (cameraConfig.IsNull()) successful = false;

            if (successful)
            {
                // Intrinsic Parameters
                const WbConfig cameraIntrisicConfig( cameraConfig.GetSubConfig( CalibrationSchema::schemaName ) );

                if (cameraIntrisicConfig.IsNull()) successful = false;

                if (successful)
                {
                    const bool calibrationWasSuccessful = cameraIntrisicConfig
                                        .GetKeyValue( CalibrationSchema::calibrationSuccessfulKey )
                                        .ToBool();
                    const bool cameraMtxValid = cameraIntrisicConfig
                                    .GetKeyValue( CalibrationSchema::cameraMatrixKey )
                                    .ToCvMat( *cameraMtx );
                    const bool distortionCoeffsValid = cameraIntrisicConfig
                                    .GetKeyValue( CalibrationSchema::distortionCoefficientsKey )
                                    .ToCvMat( *distortionCoeffs );
                    const bool inverseCoeffsValid = cameraIntrisicConfig
                                    .GetKeyValue( CalibrationSchema::invDistortionCoefficientsKey )
                                    .ToCvMat( *inverseCoeffs );

                    successful = calibrationWasSuccessful &&
                                 cameraMtxValid &&
                                 distortionCoeffsValid &&
                                 inverseCoeffsValid;
                }

                // Extrinsic Parameters
                const WbConfig cameraExtrisicConfig(camPosConfig.GetSubConfig(ExtrinsicCalibrationSchema::schemaName));

                if (cameraExtrisicConfig.IsNull()) successful = false;

                if (successful)
                {
                    const bool rotMatValid = cameraExtrisicConfig
                                    .GetKeyValue( ExtrinsicCalibrationSchema::rotationMatrixKey )
                                    .ToCvMat( *rot );
                    const bool transValid = cameraExtrisicConfig
                                    .GetKeyValue( ExtrinsicCalibrationSchema::translationKey )
                                    .ToCvMat( *trans );

                    successful = rotMatValid && transValid;
                }
            }
        }

        if (successful)
        {
            // Load file
            IplImage* imgGrey = cvLoadImage( fileName.toAscii(), CV_LOAD_IMAGE_GRAYSCALE );

            if (unWarp)
            {
                *camImg = GroundPlaneUtility::unwarpGroundPlane( imgGrey,
                                                                 cameraMtx,
                                                                 distortionCoeffs,
                                                                 inverseCoeffs,
                                                                 rot,
                                                                 trans,
                                                                 offset );
            }
            else
            {
                *camImg = cvCloneImage(imgGrey);
            }

            cvReleaseImage( &imgGrey );
        }

        cvReleaseMat( &cameraMtx );
        cvReleaseMat( &distortionCoeffs );
        cvReleaseMat( &inverseCoeffs );

        cvReleaseMat( &rot );
        cvReleaseMat( &trans );

        return successful;
    }

    bool CheckMappingIsComplete(WbConfig config)
    {
        bool allMapped = true;

        // for each camera,
        //    for each mapping,
        //       if camera1 = camera OR camera2 = camera
        //          found = true;
        //          break;
        //    if !found
        //       break
        // return found

        const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
        const QStringList cameraPositionIds(roomLayoutConfig
                                            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                            .ToQStringList() );

        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

        for ( int n = 0; n < cameraPositionIds.size(); ++n )
        {
            const KeyId camPosId = cameraPositionIds.at( n );

            bool found = false;

            LOG_INFO(QObject::tr("Checking mapping for %1.").arg(camPosId));

            for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
            {
                const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
                const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

                if ((camPosId == camera1Id) || (camPosId == camera2Id))
                {
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                allMapped = false;
                break;
            }
        }

        return allMapped;
    }

    bool IsBase(WbConfig config, KeyId camId)
    {
        //  for each mapping,
        //    if camera1 == camera
        //      base = true

        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

        bool base = false;

        for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
        {
            const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );

            if (camId == camera1Id)
            {
                base = true;
                break;
            }
        }

        return base;
    }

    bool IsRef(WbConfig config, KeyId camId)
    {
        //  for each mapping,
        //    if camera2 == camera
        //      ref = true

        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

        bool ref = false;

        for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
        {
            const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

            if (camId == camera2Id)
            {
                ref = true;
                break;
            }
        }

        return ref;
    }

    std::vector<KeyId> FindRoot(WbConfig config)
    {
        std::vector<KeyId> rootCamera;

        // for each camera,
        //    for each mapping,
        //       if camera2 == camera
        //          root = false
        //    if root
        //       add to set

        const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
        const QStringList cameraPositionIds(roomLayoutConfig
                                            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                            .ToQStringList() );

        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

        for ( int n = 0; n < cameraPositionIds.size(); ++n )
        {
            const KeyId camPosId = cameraPositionIds.at( n );

            bool root = true;

            for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
            {
                const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
                const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

                if (camPosId == camera2Id)
                {
                    root = false;
                    break;
                }
            }

            if (root && IsBase(config, camPosId))
            {
                rootCamera.push_back(camPosId);
            }
        }

        return rootCamera;
    }

    std::vector<KeyId> FindChain(WbConfig config, KeyId camId, KeyId rootId, std::vector<KeyId> mappingChain)
    {
        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

        for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
        {
            const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
            const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

            LOG_INFO(QObject::tr("Camera1 id = %1.").arg(camera1Id));
            LOG_INFO(QObject::tr("Camera2 id = %1.").arg(camera2Id));

            if (camId == camera2Id)
            {
                if (std::find(mappingChain.begin(), mappingChain.end(), camera1Id) == mappingChain.end())
                {
                    mappingChain.push_back(camera1Id);

                    if (camera1Id != rootId)
                    {
                        LOG_INFO(QObject::tr("Find chain for %1 - %2.").arg(camera1Id)
                                                                       .arg(rootId));

                        mappingChain = FindChain(config, camera1Id, rootId, mappingChain);
                    }

                    if (mappingChain.back() == rootId)
                    {
                        LOG_INFO("Found.");

                        break;
                    }
                    else
                    {
                        mappingChain.pop_back();
                    }
                }
            }
        }

        return mappingChain;
    }

    bool CheckRootMapping(WbConfig config, KeyId rootId)
    {
        bool allMapped = true;

        // for each camera in root,
        //   for each camera
        //      if camera /= root
        //         if !FindChain (camera, root)
        //             = false;
        //            break;
        //   if found
        //      theRoot = root
        //      break
        //   else

        const WbConfig roomLayoutConfig(config.GetParent().GetSubConfig( RoomLayoutSchema::schemaName ) );
        const QStringList cameraPositionIds(roomLayoutConfig
                                            .GetKeyValue(RoomLayoutSchema::cameraPositionIdsKey)
                                            .ToQStringList() );

        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );
        for ( int n = 0; n < cameraPositionIds.size(); ++n )
        {
            const KeyId camPosId = cameraPositionIds.at( n );

            if ((camPosId != rootId) && IsRef(config, camPosId))
            {
                LOG_INFO(QObject::tr("Find chain for %1 - %2.").arg(camPosId)
                                                               .arg(rootId));

                std::vector<KeyId> chain = FindChain(config, camPosId, rootId, std::vector<KeyId>());

                if (chain.size() == 0)
                {
                    LOG_INFO("Not found.");

                    allMapped = false;
                    break;
                }
            }
        }

        return allMapped;
    }

    void ComputeTransform(WbConfig config, KeyId refId, std::vector<KeyId> chain, CvMat* transform)
    {
        const WbKeyValues::ValueIdPairList cameraMappingIds = config.GetKeyValues( FloorPlanSchema::homographyKey );

        for (std::vector<KeyId>::iterator elt = chain.begin(); elt != chain.end(); ++elt)
        {
            for (WbKeyValues::ValueIdPairList::const_iterator it = cameraMappingIds.begin(); it != cameraMappingIds.end(); ++it)
            {
                const KeyId camera1Id( config.GetKeyValue( FloorPlanSchema::camera1IdKey, it->id ).ToKeyId() );
                const KeyId camera2Id( config.GetKeyValue( FloorPlanSchema::camera2IdKey, it->id ).ToKeyId() );

                if ((camera1Id == *elt) && (camera2Id == refId))
                {
                    CvMat* homography = cvCreateMat( 3, 3, CV_32F );

                    const bool homographyValid = config
                                    .GetKeyValue( FloorPlanSchema::homographyKey, it->id )
                                    .ToCvMat( *homography );
                    Q_UNUSED(homographyValid);

                    CvMat* tmp = cvCreateMat( 3, 3, CV_32F );
                    cvMatMul( homography, transform, tmp );

                    cvmSet(transform,0,0, cvmGet(tmp,0,0));
                    cvmSet(transform,0,1, cvmGet(tmp,0,1));
                    cvmSet(transform,0,2, cvmGet(tmp,0,2));
                    cvmSet(transform,1,0, cvmGet(tmp,1,0));
                    cvmSet(transform,1,1, cvmGet(tmp,1,1));
                    cvmSet(transform,1,2, cvmGet(tmp,1,2));
                    cvmSet(transform,2,0, cvmGet(tmp,2,0));
                    cvmSet(transform,2,1, cvmGet(tmp,2,1));
                    cvmSet(transform,2,2, cvmGet(tmp,2,2));

                    cvReleaseMat(&tmp);
                    cvReleaseMat(&homography);

                    refId = *elt;
                }
            }
        }
    }
}
