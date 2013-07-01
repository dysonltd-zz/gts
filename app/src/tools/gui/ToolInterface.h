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

#ifndef TOOLINTERFACE_H
#define TOOLINTERFACE_H

#include <string>

#include <QWidget>
#include <QtGui/QMenu>

#include "HelpViewer.h"
#include "WbKeyValues.h"
#include "WbConfig.h"

class Workbench;
class WbSchema;
class WbConfig;

class ToolInterface;

struct ToolFunction
{
    virtual ~ToolFunction() {}
    virtual void operator() ( ToolInterface& tool ) = 0;
};

class ToolInterface
{
public:
    virtual ~ToolInterface() {}
    virtual const QString Name() const = 0;
    virtual const HelpBookmark GetHelpText() const = 0;

    virtual QWidget* Widget() = 0;

    virtual bool TryToOpenTool( const WbConfig& config ) = 0;

    virtual void Reload( const WbConfig& parentConfig ) = 0;
    virtual void UpdateToolMenu( QMenu& toolMenu ) = 0;
    virtual void Activated() = 0;

    virtual void CallOnSelfAndActiveSubTools( ToolFunction& func ) = 0;

    virtual void AddFullWorkbenchSchemaSubTreeTo( WbSchema&      parentSchema,
                                                  const KeyName& schemaToAttachTo ) const = 0;

    virtual const WbConfig GetCurrentConfig() const = 0;

    virtual const WbSchema GetMostSpecificSubSchema() const = 0;

    virtual bool CanClose() const = 0;
    virtual const QString CannotCloseReason() const = 0;
};

#endif // TOOLINTERFACE_H
