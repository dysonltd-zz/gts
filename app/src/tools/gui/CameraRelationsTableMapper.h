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

#ifndef CameraRelationsTableMapper_H
#define CameraRelationsTableMapper_H

#include "ConfigKeyMapper.h"
#include <QtGui/QTableWidget>
#include "KeyName.h"
#include "FloorPlanSchema.h"
#include <QtGui/QAction>
#include <QtCore/qlist.h>
#include <QtGui/QMessageBox>
#include "CameraPositionsCollection.h"

#include <iostream>

class CameraRelationsTableMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    static const int camera1Column = 0;
    static const int camera2Column = 1;

    static const int idRole = Qt::UserRole;

    CameraRelationsTableMapper( QTableWidget& mappingsTable ) :
        ConfigKeyMapper( FloorPlanSchema::camera1IdKey ),
        m_table( mappingsTable ),
        m_cameraPositionsCollection( CameraPositionsCollection() )
    {
        QAction* const removeItemAction = new QAction( tr( "Remove" ), &m_table );
        removeItemAction->setShortcut( QKeySequence( QKeySequence::Delete ) );
        removeItemAction->setShortcutContext( Qt::WidgetShortcut );
        connect( removeItemAction, SIGNAL( triggered() ), this, SLOT( RemoveItemTriggered() ) );
        m_table.addAction(removeItemAction);
        m_table.setContextMenuPolicy( Qt::ActionsContextMenu );
    }

    virtual void CommitData( WbConfig& config )
    {
        std::vector< KeyId > idsToKeep;

        for (int i = 0; i < m_table.rowCount(); ++i)
        {
            idsToKeep.push_back(m_table.item(i, camera1Column)->data(idRole).toString());
        }

        config.RemoveOldKeys( FloorPlanSchema::camera1IdKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::camera2IdKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::camera1ImgKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::camera2ImgKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::homographyKey, idsToKeep );

        SetConfig( config ); // Since we need to re-number the entries and we won't
                             // get our SetConfig called as we're requesting the update
    }

    virtual void SetConfig( const WbConfig& config )
    {
        typedef WbKeyValues::ValueIdPairList ValueIdList;

        m_table.clearContents();

        const ValueIdList mappings(config.GetKeyValues(FloorPlanSchema::camera1IdKey));
        m_table.setRowCount(mappings.size());
        m_table.setColumnCount( 2 );

        const Collection::StatusType status =
                                        m_cameraPositionsCollection.SetConfig( config );

        if ( status == Collection::Status_Ok )
        {
            for (int i = 0; i < (int)mappings.size(); ++i)
            {
                SetTableRow(i, config, mappings.at( i ));
            }
        }
    }

private slots:
    void RemoveItemTriggered()
    {
        const int currentRow = m_table.currentRow();

        if (currentRow != -1)
        {
            int userConfirmation = ComfirmDeletion();

            if (userConfirmation == QMessageBox::Yes)
            {
                m_table.removeRow(currentRow);
                DataChanged();
            }
        }
    }

private:

    int ComfirmDeletion()
    {
        QMessageBox userPrompt(&m_table);
        userPrompt.setWindowTitle(tr("Remove Entry"));
        userPrompt.setText(tr("Really remove?  This action cannot be undone. "));
        userPrompt.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        userPrompt.setDefaultButton(QMessageBox::No);

        return userPrompt.exec();
    }

    const WbConfig GetCamPosConfig( const QString& camPosId )
    {
        return m_cameraPositionsCollection.ElementById( KeyId( camPosId ) );
    }

    void SetTableRow( const int row,
                      const WbConfig& config,
                      const WbKeyValues::ValueIdPair& mapping  )
    {
        const KeyValue camera1Value( config.GetKeyValue( FloorPlanSchema::camera1IdKey, mapping.id ) );
        const KeyValue camera2Value( config.GetKeyValue( FloorPlanSchema::camera2IdKey, mapping.id ) );

        const WbConfig camPosConfig1( GetCamPosConfig( camera1Value.ToQString() ) );
        const WbConfig camPosConfig2( GetCamPosConfig( camera2Value.ToQString() ) );

        QTableWidgetItem* const camera1Item = new QTableWidgetItem(tr("%1").arg(camPosConfig1.GetKeyValue( KeyName("name") ).ToQString()));
        QTableWidgetItem* const camera2Item = new QTableWidgetItem(tr("%1").arg(camPosConfig2.GetKeyValue( KeyName("name") ).ToQString()));

		camera1Item->setData(idRole, mapping.id);

		m_table.setItem( row, camera1Column, camera1Item);
		m_table.setItem( row, camera2Column, camera2Item);
    }

    QTableWidget& m_table;

    Collection m_cameraPositionsCollection;
};

#endif
