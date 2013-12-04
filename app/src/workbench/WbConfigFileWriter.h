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

#ifndef WBCONFIGFILEWRITER_H
#define WBCONFIGFILEWRITER_H

#include "WbKeyValues.h"

#include <QtCore/QFileInfo>
#include <QtCore/QIODevice>

class WbConfigFileWriter
{
public:
    virtual WbConfigFileWriter* const Clone() const = 0;

    virtual bool WriteTo(QIODevice& ioDevice) = 0;
    virtual void StartConfigFile(const KeyName& name) = 0;
    virtual void EndConfigFile(const KeyName& name) = 0;
    virtual void WriteKey(const KeyName& name, const KeyValue& value, const KeyId& id) = 0;
    virtual void StartGroup(const KeyName& name, const KeyId& id) = 0;
    virtual void EndGroup(const KeyName& name, const KeyId& id) = 0;
    virtual void WriteSubConfig(const KeyName& name, const QFileInfo& configFileLocation, const KeyId& id) = 0;
};

#endif // WBCONFIGFILEWRITER_H
