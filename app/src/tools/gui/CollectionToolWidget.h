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

#ifndef COLLECTIONTOOLWIDGET_H
#define COLLECTIONTOOLWIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include "Tool.h"
#include <QtGui/QComboBox>

namespace Ui
{
    class CollectionToolWidget;
}
class MainWindow;
class ToolTabsContainerWidget;
class NewElementWizard;

/** @brief A class.
 *
 */
class CollectionToolWidget : public Tool
{
    Q_OBJECT

public:
    explicit CollectionToolWidget( const QString&  userFriendlyElementName,
                                   const WbSchema& collectionSchema,
                                   const WbSchema& elementSchema,
                                   QWidget* parent,
                                   MainWindow* mainWindow );
    ~CollectionToolWidget();

    virtual bool TryToOpenTool( const WbConfig& config );
    virtual void CallOnSelfAndActiveSubTools( ToolFunction& func );

    virtual bool CanClose() const;
    virtual const QString CannotCloseReason() const;

protected:
    void AddToolDetail( QLabel* const detailLabel, QWidget* const detailEntryBox );
    void AddSubTool( ToolInterface* const subTool );
//    void AddNewElementWizardPage( QWizardPage* )
    void SetName( const QString& name );
    Collection GetCollection() const;

    void ReloadAll( const WbConfig& configToUse );

    virtual void UpdateToolMenu( QMenu& toolMenu );

    virtual void SetEnabled( const bool shoudEnable );

    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);

    virtual void AddExtraNewElementWizardPages( NewElementWizard* const wizard );

    virtual void SetToolSpecificConfigItems( WbConfig newElement,
                                             NewElementWizard& wizard );


    static const WbSchema CreateElementWorkbenchSubSchema( const KeyName& schemaName,
                                                           const QString& defaultName );

private slots:
    void NameComboActivated( const int comboIndex );
    void NewElement();

private:
    void CreateSubToolTabs( MainWindow* const mainWindow );
    void ReloadToElement( const KeyId& elementId );
    void ReloadToConfig( const WbConfig newConfig );
    void ReloadToCollection();
    NewElementWizard* const CreateNewElementWizard();
    bool CurrentConfigIsElement() const;

    virtual const WbSchema GetFullWorkbenchSchemaSubTree() const;

    virtual void ReloadCurrentConfigToolSpecific();
    virtual const WbConfig GetMappedConfig() const;

    const WbConfig GetElementConfig() const;

    static const WbSchema CreateCombinedSchema( const WbSchema& collectionSchema,
                                                const WbSchema& elementSchema );


    const QString  m_userFriendlyElementName;
    const WbSchema m_collectionSchema;
    const WbSchema m_elementSchema;

    Ui::CollectionToolWidget* m_ui;
    std::vector< QWidget* > m_ownedWidgets;
    ToolTabsContainerWidget* m_subToolsTabs;
    MainWindow* m_mainWindow;
};

#endif // COLLECTIONTOOLWIDGET_H
