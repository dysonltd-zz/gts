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

#ifndef NEWELEMENTWIZARD_H
#define NEWELEMENTWIZARD_H

#include <QtGui/QWizard>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include "Collection.h"

class CameraHardware;

class NewElementWizardPage : public QWizardPage
{
protected:
    static const QString mandatoryFieldSuffix;
};


class WizardStartPage : public NewElementWizardPage
{
public:
    WizardStartPage( const Collection& collection, const QString& title );

    virtual void initializePage();
    virtual bool isComplete() const;

    static const QString nameField;

private:
    QLabel*    m_explanationIcon;
    QLabel*    m_explanationInfo;
    QLineEdit* m_nameEdit;
    Collection m_collection;
};

class NewElementWizard : public QWizard
{
    Q_OBJECT

public:
    NewElementWizard( const Collection& collection,
                      const QString& elementType,
                      QWidget* const parent = 0 );
};

class RenameElementWizard : public QWizard
{
    Q_OBJECT

public:
    RenameElementWizard( const Collection& collection,
                         const QString& elementType,
                         QWidget* const parent = 0 );
};

#endif // NEWELEMENTWIZARD_H
