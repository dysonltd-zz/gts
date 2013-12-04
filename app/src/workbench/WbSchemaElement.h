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

#ifndef WBSCHEMAELEMENT_H
#define WBSCHEMAELEMENT_H

#include <iostream>
#include <string>

#include "WbKeyValues.h"
#include "KeyValue.h"

class WbConfig;
class WbConfigFileReader;
class WbConfigFileWriter;

class WbSchemaElement
{
public:
    struct Multiplicity
    {
        enum Type
        {
            One,
            Many
        };
    };

    WbSchemaElement(const KeyName& name, const Multiplicity::Type& muliplicity);
    virtual ~WbSchemaElement();
    virtual const KeyName GetKeyName() const;
    virtual const WbSchemaElement::Multiplicity::Type GetMultiplicity() const;

    virtual WbSchemaElement* const Clone() const = 0;
    virtual void ReadFrom(WbConfigFileReader& reader, WbConfig& config) const = 0;
    virtual bool WriteTo (WbConfigFileWriter& writer, const WbConfig& config) const = 0;
    virtual void SetDefaultTo(WbConfig& config) const = 0;

    virtual void PrintOn(std::ostream& os, const std::string& indent) const = 0;

private:
    KeyName m_keyName;
    Multiplicity::Type m_muliplicity;
};

#endif
