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

#include "CollectionToolWidget.h"

#include "ui_CollectionToolWidget.h"

#include "MainWindow.h"
#include "WbDefaultKeys.h"
#include "WbConfigTools.h"
#include "OStreamConfigFileWriter.h"
#include "ToolTabsContainerWidget.h"
#include "NewElementWizard.h"
#include "WbConfig.h"

#include <QtCore/QString>
#include <QtGui/QMessageBox>


#include "Debugging.h"

CollectionToolWidget::CollectionToolWidget( const QString&  userFriendlyElementName,
                                            const WbSchema& collectionSchema,
                                            const WbSchema& elementSchema,
                                            QWidget*        parent,
                                            MainWindow*     mainWindow ) :
    Tool                      ( parent, CreateCombinedSchema( collectionSchema, elementSchema ) ),
    m_userFriendlyElementName ( userFriendlyElementName ),
    m_collectionSchema        ( collectionSchema ),
    m_elementSchema           ( elementSchema ),
    m_ui                      ( new Ui::CollectionToolWidget ),
    m_ownedWidgets            (),
    m_subToolsTabs            ( 0 ),
    m_mainWindow              ( mainWindow )
{
    m_ui->setupUi( this );

    CreateSubToolTabs( mainWindow );

    QObject::connect( m_ui->m_nameComboBox,
                      SIGNAL( activated(int) ),
                      this,
                      SLOT( NameComboActivated(const int) ) );

    AddMapper( WbDefaultKeys::descriptionKey, m_ui->m_descriptionPlainTextBox );
    RegisterCollectionCombo( m_ui->m_nameComboBox, GetCollection(), true );
}

CollectionToolWidget::~CollectionToolWidget()
{
    delete m_ui;
}

void CollectionToolWidget::showEvent(QShowEvent*)
{
    PRINT_VAR_MESSAGE( Name(), "Showing" );
}

void CollectionToolWidget::hideEvent(QHideEvent*)
{
    PRINT_VAR_MESSAGE( Name(), "Hiding" );
}

/** @brief Occurs only when changed by user (not programmatically),
 *   but also if user activates already-selected item.
 *
 * @param comboIndex
 */
void CollectionToolWidget::NameComboActivated( const int comboIndex )
{
    QVariant userData = m_ui->m_nameComboBox->itemData( comboIndex );
    if ( userData.isValid() )
    {
        ReloadToElement( userData.toString() );
    }
    else
    {
        ReloadToCollection();
    }
}

void CollectionToolWidget::ReloadToConfig( const WbConfig newConfig )
{
    if ( !newConfig.IsTheSameAs( GetCurrentConfig() ) )
    {
        if ( m_mainWindow )
        {
            m_mainWindow->MergeWithActivePath( WbPath::FromWbConfig( newConfig ) );
            m_mainWindow->Reload();
        }
    }
}

void CollectionToolWidget::ReloadToElement( const KeyId& elementId )
{
    ReloadToConfig( GetCollection().ElementById( elementId ) );
}

void CollectionToolWidget::ReloadToCollection()
{
    ReloadToConfig( GetCollection().CollectionConfig() );
}

void CollectionToolWidget::CreateSubToolTabs( MainWindow* const mainWindow )
{
    m_subToolsTabs = new ToolTabsContainerWidget( mainWindow, this );
    m_subToolsTabs->setObjectName( QString::fromUtf8("m_subToolTabs") );

    QSizePolicy tabsSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding);
    tabsSizePolicy.setHorizontalStretch( 0 );
    tabsSizePolicy.setVerticalStretch  ( 100 );
    m_subToolsTabs->setSizePolicy( tabsSizePolicy );

    assert( this->layout() );
    this->layout()->addWidget( m_subToolsTabs );
}

void CollectionToolWidget::AddSubTool( ToolInterface* const subTool )
{
    m_subToolsTabs->AddTool( subTool );
}

void CollectionToolWidget::AddToolDetail( QLabel*  const detailLabel,
                                          QWidget* const detailEntryBox )
{
    m_ui->m_toolDetailsLayout->addRow( detailLabel, detailEntryBox );
    m_ownedWidgets.push_back( detailLabel );
    m_ownedWidgets.push_back( detailEntryBox );
}

/** @brief Returns null if we're displaying at collection level
 *
 * @return
 */
const WbConfig CollectionToolWidget::GetElementConfig() const
{
    return GetCurrentConfig().FindAncestor( m_elementSchema.Name() );
}

void CollectionToolWidget::SetName( const QString& name )
{
    m_ui->m_nameComboBox->setCurrentIndex( m_ui->m_nameComboBox->findText( name ) );
}

bool CollectionToolWidget::TryToOpenTool( const WbConfig& config )
{
    const bool openedThisTool = Tool::TryToOpenTool( config );
    bool openedSubTool = false;
    if ( openedThisTool )
    {
        openedSubTool = m_subToolsTabs->TryToOpenTool( config );
    }
    return openedThisTool || openedSubTool;
}

void CollectionToolWidget::CallOnSelfAndActiveSubTools( ToolFunction& func )
{
    Tool::CallOnSelfAndActiveSubTools( func );
    m_subToolsTabs->CallOnActiveTools( func );
}

bool CollectionToolWidget::CurrentConfigIsElement() const
{
    return GetCurrentConfig().SchemaIsDescendantOf( m_elementSchema.Name() );
}

void CollectionToolWidget::ReloadCurrentConfigToolSpecific()
{
    if ( CurrentConfigIsElement() )
    {
        const KeyValue name = GetCurrentConfig().GetKeyValue( WbDefaultKeys::displayNameKey );
        SetName( name.ToQString() );
    }
    else
    {
        SetName( selectItemString );
    }
}

void CollectionToolWidget::SetEnabled( const bool shoudEnable )
{
    Tool::SetEnabled( shoudEnable );
    Widget()->setEnabled( true );
    for ( size_t i = 0; i < m_ownedWidgets.size(); ++i )
    {
        QWidget* widget = m_ownedWidgets.at( i );
        widget->setEnabled( shoudEnable );
    }
    m_ui->m_descriptionPlainTextBox->setEnabled( shoudEnable );
    m_subToolsTabs->setEnabled( shoudEnable );

    m_ui->m_nameComboBox->setEnabled( true );
}

const WbSchema CollectionToolWidget::GetFullWorkbenchSchemaSubTree() const
{
    WbSchema schema( GetHandledSchema() );

    m_subToolsTabs->AddToolsFullWorkbenchSchemaSubTreeTo( schema,
                                                          m_elementSchema.Name() );

    return schema;
}

Collection CollectionToolWidget::GetCollection() const
{
    Collection collection( m_collectionSchema.Name(), m_elementSchema.Name() );
    collection.SetConfig( GetCurrentConfig() );
    return collection;
}

void CollectionToolWidget::ReloadAll( const WbConfig& configToUse )
{
    if ( m_mainWindow )
    {
        m_mainWindow->MergeWithActivePath( WbPath::FromWbConfig( configToUse ) );
        m_mainWindow->Reload();
    }
}

NewElementWizard* const CollectionToolWidget::CreateNewElementWizard()
{
    NewElementWizard* const newElementWizard = new NewElementWizard(GetCollection(),
                                                                    m_userFriendlyElementName,
                                                                    this);
    AddExtraNewElementWizardPages(newElementWizard);
    return newElementWizard;
}

void CollectionToolWidget::AddExtraNewElementWizardPages( NewElementWizard* const wizard )
{
    Q_UNUSED(wizard);
}

RenameElementWizard* const CollectionToolWidget::CreateRenameElementWizard()
{
    RenameElementWizard* const renameElementWizard = new RenameElementWizard(GetCollection(),
                                                                             m_userFriendlyElementName,
                                                                             this);
    AddExtraRenameElementWizardPages(renameElementWizard);
    return renameElementWizard;
}

void CollectionToolWidget::AddExtraRenameElementWizardPages( RenameElementWizard* const wizard )
{
    Q_UNUSED(wizard);
}

const WbSchema CollectionToolWidget::CreateCombinedSchema( const WbSchema& collectionSchema,
                                                           const WbSchema& elementSchema )
{
    WbSchema combinedSchema( collectionSchema );
    combinedSchema.AddSubSchema( elementSchema, WbSchemaElement::Multiplicity::Many );
    return combinedSchema;
}

const WbConfig CollectionToolWidget::GetMappedConfig() const
{
    return GetElementConfig();
}

const WbSchema CollectionToolWidget::CreateElementWorkbenchSubSchema(
                                const KeyName& schemaName, const QString& defaultName )
{
    WbSchema schema( CreateWorkbenchSubSchema( schemaName, defaultName ) );
    schema.AddSingleValueKey( WbDefaultKeys::descriptionKey,
                              WbSchemaElement::Multiplicity::One );
    return schema;
}

void CollectionToolWidget::UpdateToolMenu( QMenu& toolMenu )
{
    Tool::UpdateToolMenu( toolMenu );
    toolMenu.setTitle( m_userFriendlyElementName );
    toolMenu.addAction( tr( "&New..." ),
                        this,
                        SLOT( NewElement() ),
                        QKeySequence( tr( "Ctrl+Shift+N" ) ) );

    if ( CurrentConfigIsElement() )
    {
        toolMenu.addAction( tr( "&Rename..." ),
                            this,
                            SLOT( RenameElement() ),
                            QKeySequence( tr( "Ctrl+Shift+R" ) ) );

        toolMenu.addAction( tr( "&Delete" ),
                            this,
                            SLOT( DeleteElement() ),
                            QKeySequence( tr( "Ctrl+Shift+D" ) ) );
    }
}

void CollectionToolWidget::NewElement()
{
    std::auto_ptr< NewElementWizard > newElementWizard( CreateNewElementWizard() );
    if ( newElementWizard->exec() )
    {
        const KeyValue newElementName(
            KeyValue::from( newElementWizard
                                       ->field( WizardStartPage::nameField )
                                       .toString() ) );

        WbConfig newElement( GetCollection().AddNewElement( newElementName ) );
        SetToolSpecificConfigItems( newElement, *newElementWizard );
        ReloadAll( newElement );
    }
    else
    {
        PRINT( "User cancelled" );
    }
}

void CollectionToolWidget::RenameElement()
{
    std::auto_ptr< RenameElementWizard > renameElementWizard( CreateRenameElementWizard() );
    if ( renameElementWizard->exec() )
    {
        const KeyValue newElementName(
            KeyValue::from( renameElementWizard
                                       ->field( WizardStartPage::nameField )
                                       .toString() ) );

        WbConfig selectedElement = GetElementConfig();
        selectedElement.SetKeyValue( WbDefaultKeys::displayNameKey,
                                    newElementName);
        SetToolSpecificConfigItems( selectedElement, *renameElementWizard );
        ReloadAll( selectedElement );
    }
    else
    {
        PRINT( "User cancelled" );
    }
}

void CollectionToolWidget::DeleteElement()
{
    WbConfig selectedElement = GetElementConfig();

    const WbConfig parent = selectedElement.GetParent();
    const KeyId id = parent.FindSubConfigId( selectedElement );

    // Look to see if there are dependencies on this item.
    if ( !selectedElement.DependentExists( KeyValue::from(id) ) )
    {
        GetCollection().DeleteElement( id );

        ReloadAll( parent );
    }
    else
    {
        QMessageBox::information( 0,
                               QObject::tr( "Error" ),
                               QObject::tr( "Object is in use - cannot delete. "
                                            "Remove references then try again." ) );
    }
}

void CollectionToolWidget::SetToolSpecificConfigItems( WbConfig newElement,
                                                       NewElementWizard& wizard )
{
    Q_UNUSED(newElement);
    Q_UNUSED(wizard);
}

void CollectionToolWidget::SetToolSpecificConfigItems( WbConfig newElement,
                                                       RenameElementWizard& wizard )
{
    Q_UNUSED(newElement);
    Q_UNUSED(wizard);
}

bool CollectionToolWidget::CanClose() const
{
    if ( !m_subToolsTabs->ActiveToolCanClose() )
    {
        return false;
    }
    return Tool::CanClose();
}

const QString CollectionToolWidget::CannotCloseReason() const
{
    return m_subToolsTabs->ActiveToolCannotCloseReason();
}
