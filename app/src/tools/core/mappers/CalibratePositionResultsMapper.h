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

#ifndef CALIBRATEPOSITIONRESULTSMAPPER_H
#define CALIBRATEPOSITIONRESULTSMAPPER_H

#include "ConfigKeyMapper.h"
#include "KeyName.h"

#include <QtGui/QTextBrowser>

class CalibratePositionResultsMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    CalibratePositionResultsMapper(QTextBrowser& textBrowser);

    virtual void CommitData(WbConfig& config);
    virtual void SetConfig(const WbConfig& config);

private:
    const QString GetSuccessOrFailureText(const WbConfig& config, bool successful) const;

    const QString MatrixText(const WbConfig& config, const KeyName keyName,
                              const int rows, const int columns);

    QTextBrowser& m_textBrowser;

    static const QString startHeader;
    static const QString endHeader;
    static const QString startBody;
    static const QString endBody;
};

#endif // CALIBRATEPOSITIONRESULTSMAPPER_H
