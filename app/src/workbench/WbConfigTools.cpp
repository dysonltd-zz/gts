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

#include "WbConfigTools.h"
#include "WbDefaultKeys.h"
#include "Collection.h"

#include <QtCore/QDir>

namespace WbConfigTools
{
    namespace
    {
        const KeyValue GetFileNameToForConfig(WbConfig config,
                                               const QString& possiblyRelativeFileName,
                                               const FileNameMode& mode)
        {
            const QString absoluteFileName(
                    config.GetAbsoluteFileNameFor(possiblyRelativeFileName));

            const WbConfig topLevelCfg(config.FindRootAncestor());
            const QDir topLevelDir(topLevelCfg.GetAbsoluteFileInfo().absoluteDir());
            const bool fileIsInternal =
                        !topLevelDir.relativeFilePath(absoluteFileName).startsWith("..");

            return KeyValue::from(ConvertFileName(config, absoluteFileName, mode, fileIsInternal));
        }
    }

    void AddComboBoxItemForElement(QComboBox& comboBox,
                                    const WbConfig::SubConfigs::ValueIdPair& element)
    {
        const QString elementName(element
                                       .value
                                       .GetKeyValue(WbDefaultKeys::displayNameKey)
                                       .ToQString());

        comboBox.addItem(elementName, QVariant(element.id));
        comboBox.setItemData(comboBox.count()-1,
                              element
                                 .value
                                 .GetKeyValue(WbDefaultKeys::descriptionKey)
                                 .ToQString(),
                              Qt::ToolTipRole);
    }

    void FillOutComboBoxWithCollectionElements(QComboBox& comboBox,
                                                const Collection& collection)
    {
        for (size_t i = 0; i < collection.NumElements(); ++i)
        {
            AddComboBoxItemForElement(comboBox, collection.ElementAt(i));
        }
    }

    const QString ConvertFileName(const WbConfig& config,
                                   const QString& absoluteFileName,
                                   const FileNameMode& mode,
                                   const bool fileIsInternal)
    {
        switch (mode)
        {
            case FileNameMode_Relative:
                return config.GetAbsoluteFileInfo()
                            .absoluteDir().relativeFilePath(absoluteFileName);

            case FileNameMode_RelativeInsideWorkbench:
                if (fileIsInternal)
                {
                    return config.GetAbsoluteFileInfo()
                            .absoluteDir().relativeFilePath(absoluteFileName);
                }
                else
                {
                    return config.GetAbsoluteFileNameFor(absoluteFileName);
                }

            case FileNameMode_Absolute:
                return config.GetAbsoluteFileNameFor(absoluteFileName);

            default:
                assert(!"Unhandled FileNameMode");
                return QString();
        }
    }

    const QString GetFileName(const WbConfig& config, const KeyName& fileNameKeyName)
    {
        const KeyValue fileNameKeyValue(config.GetKeyValue(fileNameKeyName));
        const QString fileName(fileNameKeyValue.ToQString());
        return config.GetAbsoluteFileNameFor(fileName);
    }

    void SetFileName(WbConfig config,
                      const QString& possiblyRelativeFileName,
                      const KeyName& fileNameKeyName,
                      const FileNameMode& mode,
                      const KeyId& keyId)
    {
        config.SetKeyValue(fileNameKeyName, GetFileNameToForConfig(config, possiblyRelativeFileName, mode), keyId);
    }

    KeyId AddFileName(WbConfig config,
                       const QString& possiblyRelativeFileName,
                       const KeyName& fileNameKeyName,
                       const FileNameMode& mode)
    {
        return config.AddKeyValue(fileNameKeyName, GetFileNameToForConfig(config, possiblyRelativeFileName, mode));
    }

    const QString DisplayNameOf(const WbConfig& config)
    {
        return config.GetKeyValue(WbDefaultKeys::displayNameKey).ToQString();
    }

}
