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

#ifndef BUTTONLABELMAPPER_H_
#define BUTTONLABELMAPPER_H_

#include "ConfigKeyMapper.h"
#include <QtGui/QPushButton>
#include "KeyName.h"

class ButtonLabelMapper : public ConfigKeyMapper
{
public:
    ButtonLabelMapper(QPushButton& button,
                       const QString& keyIndependentLabel,
                       const QString& nonNullKeyLabelSuffixFormat,
                       const KeyName& firstKeyName,
                       const KeyName& secondKeyName) :
        ConfigKeyMapper(firstKeyName),
        m_button(button),
        m_keyIndependentLabel(keyIndependentLabel),
        m_nonNullKeyLabelSuffixFormat(nonNullKeyLabelSuffixFormat),
        m_firstKeyName (firstKeyName),
        m_secondKeyName(secondKeyName)
    {
    }

    virtual void SetConfig(const WbConfig& config)
    {
        QString label(m_keyIndependentLabel);
        const QString firstKey(KeyString(config, m_firstKeyName));
        const QString secondKey(KeyString(config, m_secondKeyName));
        if (!firstKey.isEmpty() && !secondKey.isEmpty())
        {
            label.append(m_nonNullKeyLabelSuffixFormat.arg(firstKey, secondKey));
        }
        m_button.setText(label);
    }

    virtual void CommitData(WbConfig& config)
    {
        Q_UNUSED(config);
    }

    virtual bool Maps(QWidget* const widget) const
    {
        return widget == &m_button;
    }

private:
    const QString KeyString(const WbConfig& config, const KeyName& keyName) const
    {
        return config.GetKeyValue(keyName).ToQString();
    }

    QPushButton& m_button;
    const QString m_keyIndependentLabel;
    const QString m_nonNullKeyLabelSuffixFormat;
    const KeyName m_firstKeyName;
    const KeyName m_secondKeyName;
};

#endif
