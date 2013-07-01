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

#ifndef ROOMTABLEMAPPER_H
#define ROOMTABLEMAPPER_H

#include "ConfigKeyMapper.h"
#include <QtCore/qobject.h>
#include <QtGui/QTableWidget>
#include "WbConfig.h"
#include "WbDefaultKeys.h"
#include <QtGui/qwidget.h>
#include "WbConfigTools.h"
#include <QtCore/qdebug.h>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QItemEditorCreatorBase>
#include "CameraPositionsCollection.h"
#include <cassert>
#include "RoomLayoutSchema.h"

namespace
{
    const int nameColumn        = 0;
    const int descriptionColumn = 1;
}

class ComboEditor : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QString id READ Id WRITE SetId USER true)

public:
    ComboEditor( const QModelIndex& tableIndex,
                 const Collection& collection,
                 QWidget* parent = 0 )
    :
        QComboBox( parent ),
        m_tableIndex( tableIndex )
    {
        WbConfigTools::FillOutComboBoxWithCollectionElements( *this, collection );

        QObject::connect( this,
                          SIGNAL( currentIndexChanged( int ) ),
                          this,
                          SLOT( NotifyCurrentIdChanged() ) );
    }

    const QString Id() const
    {
        return qVariantValue<QString>( itemData( currentIndex(), Qt::UserRole ) );
    }

    void SetId( const QString& id )
    {
        setCurrentIndex( findData( id, Qt::UserRole ) );
    }

signals:
    void CurrentIdChanged( const QModelIndex& tableIndex, const QString& newId ) const;

private slots:
    void NotifyCurrentIdChanged() const
    {
        emit CurrentIdChanged( m_tableIndex, Id() );
    }

private:
    QModelIndex m_tableIndex;
};


class ComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ComboDelegate( const Collection& collection )
    :
        QStyledItemDelegate(),
        m_collection ( collection )
        {
        }

    virtual ~ComboDelegate() {}

    void setEditorData( QWidget *editor, const QModelIndex& index ) const
    {
        const QString id( index.model()->data( index, Qt::UserRole ).toString() );

        ComboEditor* comboEditor = static_cast<ComboEditor*>(editor);
        comboEditor->SetId( id );
    }


    void setModelData( QWidget* editor,
                       QAbstractItemModel* model,
                       const QModelIndex& index ) const
    {
        ComboEditor* comboEditor = static_cast<ComboEditor*>(editor);

        const QString newDisplayName( comboEditor->currentText() );
        const QString newId( comboEditor->Id() );
        model->setData( index, newDisplayName, Qt::DisplayRole );
        model->setData( index, newId,          Qt::UserRole );
    }

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        ComboEditor* newComboEditor = new ComboEditor( index, m_collection, parent );

        QObject::connect( newComboEditor,
                          SIGNAL( CurrentIdChanged( const QModelIndex&,
                                                    const QString& ) ),
                          this,
                          SIGNAL( CurrentIdChanged( const QModelIndex&,
                                                    const QString& ) ) );
        return newComboEditor;
    }

signals:
    void CurrentIdChanged( const QModelIndex& tableIndex, const QString& newId ) const;

private:
    Collection m_collection;
};

class RoomTableMapper: public ConfigKeyMapper
{
    Q_OBJECT
public:
    RoomTableMapper( QTableWidget& layoutTable )
    :
        ConfigKeyMapper( RoomLayoutSchema::cameraPositionIdsKey ),
        m_table( layoutTable ),
        m_cameraPositionsCollection( CameraPositionsCollection() )
    {
    }

    void AddRow()
    {
        const int addedRow = m_table.rowCount();
        m_table.insertRow( addedRow );
        // @todo could make this auto-select next UNUSED cameraPosition
        QString camPosId;
        if ( m_cameraPositionsCollection.NumElements() > 0 )
        {
            camPosId = m_cameraPositionsCollection.ElementAt( 0 ).id;
        }
        SetCameraPositionRow( addedRow, camPosId );
        DataChanged();
    }

    void RemoveCurrentRow()
    {
        const QList<QTableWidgetItem*> selectedItems = m_table.selectedItems();
        if ( selectedItems.size() > 0 )
        {
            const int currentRow = selectedItems.at( 0 )->row();
            if ( ( currentRow >= 0 ) && ( currentRow < m_table.rowCount() ) )
            {
                m_table.removeRow( currentRow );
                DataChanged();
            }
        }
    }

    virtual void CommitData( WbConfig& config )
    {
        KeyValue cameraPosIds;
        for ( int row = 0; row < m_table.rowCount(); ++row )
        {
            const QString cameraPosId( m_table.item( row, nameColumn )
                                               ->data( Qt::UserRole ).toString() );
            if ( !cameraPosId.isEmpty() )
            {
                cameraPosIds << cameraPosId;
            }
        }
        config.SetKeyValue( m_keyName, cameraPosIds );
    }

    virtual void SetConfig( const WbConfig& config )
    {
        m_table.clearContents();
        const QStringList cameraPositionIds(
                            config.GetKeyValue( m_keyName ).ToQStringList() );
        m_table.setRowCount( cameraPositionIds.size() );

        const Collection::StatusType status =
                                        m_cameraPositionsCollection.SetConfig( config );

        if ( status == Collection::Status_Ok )
        {
            ComboDelegate* const delegate = new ComboDelegate( m_cameraPositionsCollection );

            QObject::connect( delegate,
                              SIGNAL( CurrentIdChanged( const QModelIndex&,
                                                        const QString& ) ),
                              this,
                              SLOT( CameraPositionChanged( const QModelIndex&,
                                                           const QString& ) ) );

            m_table.setItemDelegateForColumn( nameColumn, delegate );

            for ( int i = 0; i < cameraPositionIds.size(); ++i )
            {
                SetCameraPositionRow( i, cameraPositionIds.at( i ) );
            }
        }
    }

private slots:
    void CameraPositionChanged( const QModelIndex& index, const QString& newId )
    {
        SetCameraPositionRow( index.row(), newId );
        DataChanged();
    }

private:

    const WbConfig GetCamPosConfig( const QString& camPosId )
    {
        return m_cameraPositionsCollection.ElementById( KeyId( camPosId ) );
    }

    void SetCameraPositionRow( const int row, const QString&  camPosId )
    {
        const WbConfig camPosConfig( GetCamPosConfig( camPosId ) );
        m_table.setItem( row, descriptionColumn, CreateDescriptionItem( camPosConfig ) );
        m_table.setItem( row, nameColumn, CreateNameItem( camPosConfig, camPosId ) );
    }

    QTableWidgetItem* const CreateDescriptionItem( const WbConfig& camPosConfig )
    {
        QTableWidgetItem* const descriptionItem =
            new QTableWidgetItem(
                camPosConfig.GetKeyValue( WbDefaultKeys::descriptionKey ).ToQString() );
        descriptionItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
        return descriptionItem;
    }

    QTableWidgetItem* const CreateNameItem( const WbConfig& camPosConfig,
                                            const QString&  camPosId )
    {
        QTableWidgetItem* const nameItem =
            new QTableWidgetItem(
                camPosConfig.GetKeyValue( WbDefaultKeys::displayNameKey ).ToQString() );
        nameItem->setData( Qt::UserRole, camPosId );
        return nameItem;
    }

    QTableWidget& m_table;
    Collection m_cameraPositionsCollection;
};

#endif
