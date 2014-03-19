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

#ifndef WBCONFIGTOOLS_H
#define WBCONFIGTOOLS_H

#include <QtGui/QComboBox>
#include "WbConfig.h"
#include "KeyName.h"

class Collection;

namespace WbConfigTools
{
    void FillOutComboBoxWithCollectionElements( QComboBox& comboBox,
                                                const Collection& collection );

    void AddComboBoxItemForElement( QComboBox& comboBox,
                                    const WbConfig::SubConfigs::ValueIdPair& element );



    const QString DisplayNameOf( const WbConfig& config );

    enum FileNameMode
    {
        FileNameMode_Relative,
        FileNameMode_RelativeInsideWorkbench,
        FileNameMode_Absolute
    };

    const QString ConvertFileName( const WbConfig& config,
                                   const QString& absoluteFileName,
                                   const FileNameMode& mode,
                                   const bool fileIsInternal );

    const QString GetFileName( const WbConfig& config, const KeyName& fileNameKeyName );

    KeyId AddFileName( WbConfig config,
                       const QString& possiblyRelativeFileName,
                       const KeyName& fileNameKeyName,
                       const FileNameMode& mode );

    void SetFileName( WbConfig config,
                      const QString& possiblyRelativeFileName,
                      const KeyName& fileNameKeyName,
                      const FileNameMode& mode,
                      const KeyId& keyId = KeyId() );
}

#endif // WBCONFIGTOOLS_H
