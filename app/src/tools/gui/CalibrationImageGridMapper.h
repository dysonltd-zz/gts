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

#ifndef CALIBRATIONIMAGEGRIDMAPPER_H
#define CALIBRATIONIMAGEGRIDMAPPER_H

#include "ConfigKeyMapper.h"
#include "KeyName.h"
#include "ImageGrid.h"
#include "CalibrationSchema.h"
#include <vector>
#include <QtGui/qimage.h>

class CalibrationImageGridMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    CalibrationImageGridMapper( ImageGrid& imageGrid );

    virtual void CommitData( WbConfig& config );
    virtual void SetConfig( const WbConfig& config );

    void SetCurrentImage(const KeyId& imageId);

private:
    void UpdateImage( const WbConfig& config );
    void OverlayCornersIfPossible( const WbConfig& config );
    void UpdateGrid();
    bool ImageIsFound() const;
    QImage GetRealOrNotFoundImage() const;

    ImageGrid& m_grid;
    QImage m_image;
    KeyId  m_currentImageId;
    const QImage m_notFoundImage;
};

#endif
