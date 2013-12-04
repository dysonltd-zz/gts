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

#ifndef WBSUBSCHEMA_H
#define WBSUBSCHEMA_H

#include <QtCore/QString>

#include "WbSchemaElement.h"
#include "WbSchema.h"

/// class WbSubSchema
class WbSubSchema : public WbSchemaElement
{
public:
    WbSubSchema(const WbSchema& schema,
                 const Multiplicity::Type& multiplicity,
                 const QString& defaultFileName);

    virtual WbSubSchema* const Clone() const;

    void ReadFrom(WbConfigFileReader& reader, WbConfig& config) const;
    bool WriteTo (WbConfigFileWriter& writer, const WbConfig& config) const;
    virtual void PrintOn(std::ostream& os, const std::string& indent) const;
    virtual void SetDefaultTo(WbConfig& config) const;

    WbSchema& ModifiableSchema();

private:
    WbSchema m_schema;
    QString  m_defaultFileName;
};

#endif
