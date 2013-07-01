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

#ifndef NULLCONFIGFILEWRITER_H
#define NULLCONFIGFILEWRITER_H

#include "WbConfigFileWriter.h"

/** @brief A WbConfigFileWriter that doesn't write anything!
 *
 * And it is successful when it does so.
 *
 */
class NullConfigFileWriter: public  WbConfigFileWriter
{
public:
    virtual WbConfigFileWriter* const Clone() const { return new NullConfigFileWriter; }

    virtual bool WriteTo( QIODevice& ioDevice ) { Q_UNUSED(ioDevice); return true; }
    virtual void StartConfigFile( const KeyName& name ) { Q_UNUSED(name); }
    virtual void EndConfigFile( const KeyName& name ) { Q_UNUSED(name); }
    virtual void WriteKey( const KeyName& name,
                           const KeyValue& value,
                           const KeyId& id ) { Q_UNUSED(name); Q_UNUSED(value); Q_UNUSED(id); }
    virtual void StartGroup( const KeyName& name,
                             const KeyId& id ) { Q_UNUSED(name); Q_UNUSED(id);}
    virtual void EndGroup( const KeyName& name,
                           const KeyId& id ) { Q_UNUSED(name); Q_UNUSED(id); }
    virtual void WriteSubConfig( const KeyName& name,
                                 const QFileInfo& configFileLocation,
                                 const KeyId& id )
    {
        Q_UNUSED(name);
        Q_UNUSED(configFileLocation);
        Q_UNUSED(id);
    }
};

#endif
