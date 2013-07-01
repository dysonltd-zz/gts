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

#include "CalibrationToolboxMapper.h"
#include "CalibrationSchema.h"
#include <QtGui/qapplication.h>
#include <QtGui/qstyle.h>

CalibrationToolBoxMapper::CalibrationToolBoxMapper( QToolBox& toolBox )
:
ConfigKeyMapper( CalibrationSchema::calibrationSuccessfulKey ),
m_toolBox( toolBox )
{
}

void CalibrationToolBoxMapper::CommitData( WbConfig& config )
{
    Q_UNUSED(config);
}

void CalibrationToolBoxMapper::SetConfig( const WbConfig& config )
{
    const int calibrationResultsPage = 1;
    const KeyValue succesfulKey( config.GetKeyValue( m_keyName ) );
    m_toolBox.setItemEnabled( calibrationResultsPage, !succesfulKey.IsNull() );
}

