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

#ifndef CALIBRATIONTOOLBOXMAPPER_H
#define CALIBRATIONTOOLBOXMAPPER_H

#include "ConfigKeyMapper.h"
#include "KeyName.h"
#include <QtGui/qtoolbox.h>

class CalibrationToolBoxMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    CalibrationToolBoxMapper( QToolBox& toolBox );

    virtual void CommitData( WbConfig& config );
    virtual void SetConfig( const WbConfig& config );

    QToolBox& m_toolBox;
};

#endif
