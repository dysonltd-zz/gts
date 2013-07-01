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

#ifndef CALIBRATIONIMAGETABLEMAPPER_H
#define CALIBRATIONIMAGETABLEMAPPER_H

#include "ConfigKeyMapper.h"
#include <QtGui/QTableWidget>
#include "KeyName.h"
#include "CalibrationSchema.h"
#include <QtGui/QAction>
#include <QtCore/qlist.h>
#include <QtGui/QMessageBox>

class CalibrationImageTableMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    static const int nameColumn  = 0;
    static const int errorColumn = 1;

    static const int idRoleOnName = Qt::UserRole;
    static const int absoluteFileNameRoleOnName = Qt::UserRole+1;

    CalibrationImageTableMapper( QTableWidget& imagesTable )
    :
        ConfigKeyMapper( CalibrationSchema::imageFileKey ),
        m_table( imagesTable )
    {
        QAction* const removeItemAction = new QAction( tr( "Remove" ), &m_table );
        removeItemAction->setShortcut( QKeySequence( QKeySequence::Delete ) );
        removeItemAction->setShortcutContext( Qt::WidgetShortcut );

        QObject::connect( removeItemAction,
                          SIGNAL( triggered() ),
                          this,
                          SLOT( RemoveItemTriggered() ) );

        m_table.addAction(removeItemAction);
        m_table.setContextMenuPolicy( Qt::ActionsContextMenu );
    }

    virtual void CommitData( WbConfig& config )
    {
        std::vector< KeyId > idsToKeep;
        for (int i = 0; i < m_table.rowCount(); ++i)
        {
            idsToKeep.push_back(m_table.item(i, nameColumn)->data(idRoleOnName).toString());
        }

        config.RemoveOldKeys( CalibrationSchema::imageFileKey,  idsToKeep );
        config.RemoveOldKeys( CalibrationSchema::imageErrorKey, idsToKeep );
        SetConfig( config ); // Since we need to re-number the images and we won't
                             // get our SetConfig called as we're requesting the update
    }

    virtual void SetConfig( const WbConfig& config )
    {
        typedef WbKeyValues::ValueIdPairList ValueIdPairList;
        m_table.clearContents();
        const ValueIdPairList cameraImageFiles(config.GetKeyValues(CalibrationSchema::imageFileKey));
        m_table.setRowCount(cameraImageFiles.size());

        for (int i = 0; i < (int)cameraImageFiles.size(); ++i)
        {
            SetTableRow(i, config, cameraImageFiles.at( i ));
        }
    }

private slots:
    void RemoveItemTriggered()
    {
        const int currentRow = m_table.currentRow();
        if (currentRow != -1)
        {
            int userConfirmation = ComfirmDeletion(currentRow);
            if (userConfirmation == QMessageBox::Yes)
            {
                m_table.removeRow(currentRow);
                DataChanged();
            }
        }
    }

private:

    int ComfirmDeletion(const int currentRow)
    {
        QMessageBox userPrompt(&m_table);
        userPrompt.setWindowTitle(tr("Remove Image"));
        userPrompt.setText(tr("Really remove %1?  This action cannot be undone. ")
                           .arg(m_table.item(currentRow, nameColumn)->text()));
        userPrompt.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        userPrompt.setDefaultButton(QMessageBox::No);
        const QString imageFileName(m_table.item(currentRow,
                                                 nameColumn)->data(absoluteFileNameRoleOnName).toString());
        const QSize thumbnailSize(64, 64);
        userPrompt.setIconPixmap(QPixmap::fromImage(QImage(imageFileName).scaled(thumbnailSize)));
        return userPrompt.exec();
    }

    void SetTableRow( const int row,
                      const WbConfig& config,
                      const WbKeyValues::ValueIdPair& calibImageFileIdPair  )
    {
        const QString fileName( calibImageFileIdPair.value.ToQString() );
        const QString absoluteFileName( config.GetAbsoluteFileNameFor( fileName ) );
        const KeyValue errorKeyValue( config.GetKeyValue( CalibrationSchema::imageErrorKey,
                                                          calibImageFileIdPair.id ) );
        double error = -1;
        if ( !errorKeyValue.IsNull() )
        {
            error = errorKeyValue.ToDouble();
        }
        m_table.setItem( row, errorColumn, CreateErrorItem( error ) );
        m_table.setItem( row, nameColumn,  CreateNameItem( row,
                                                           calibImageFileIdPair.id,
                                                           fileName,
                                                           absoluteFileName ) );
    }

    QTableWidgetItem* const CreateErrorItem( const double error )
    {
        QString errorString;
        if ( error >= 0 )
        {
            errorString = QString::number( error );
        }
        QTableWidgetItem* const descriptionItem = new QTableWidgetItem(errorString);
        descriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        return descriptionItem;
    }

    QTableWidgetItem* const CreateNameItem(const int row,
                                           const KeyId& id,
                                           const QString& fileName,
                                           const QString absoluteFileName)
    {
        QTableWidgetItem* const nameItem = new QTableWidgetItem(tr("Image %1").arg(row+1));
        nameItem->setData(Qt::ToolTipRole, fileName);
        nameItem->setData(idRoleOnName, id);
        nameItem->setData(absoluteFileNameRoleOnName, absoluteFileName);
        return nameItem;
    }

    QTableWidget& m_table;
};

#endif
