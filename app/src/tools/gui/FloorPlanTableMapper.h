#ifndef FLOORPLANTABLEMAPPER_H
#define FLOORPLANTABLEMAPPER_H

#include "KeyName.h"
#include "ConfigKeyMapper.h"
#include "FloorPlanSchema.h"

#include <QtGui/QTableWidget>
#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtGui/QMessageBox>
#include <QSignalMapper>
#include <QPushButton>
#include <QIcon>

class FloorPlanTableMapper: public ConfigKeyMapper
{
    Q_OBJECT

public:
    static const int btnColumn   = 0;
    static const int nameColumn  = 1;

    static const int NUMCOLUMN   = 2;

    static const int idRole = Qt::UserRole;
    static const int rotRole = Qt::UserRole+1;

    FloorPlanTableMapper( QTableWidget& floorPlanTable ) :
        ConfigKeyMapper( FloorPlanSchema::camIdKey ),
        m_table( floorPlanTable )
    {
        QAction* const removeItemAction = new QAction( tr( "Remove" ), &m_table );
        removeItemAction->setShortcut( QKeySequence( QKeySequence::Delete ) );
        removeItemAction->setShortcutContext( Qt::WidgetShortcut );
        connect( removeItemAction, SIGNAL( triggered() ),
                 this, SLOT( RemoveItemTriggered() ) );
        m_table.addAction(removeItemAction);
        m_table.setContextMenuPolicy( Qt::ActionsContextMenu );

		m_signalMapper = new QSignalMapper(this);

        connect( m_signalMapper, SIGNAL( mapped( int ) ),
                 this, SLOT( clicked( int ) ) );
    }

    virtual void CommitData( WbConfig& config )
    {
        std::vector< KeyId > idsToKeep;
        for (int i = 0; i < m_table.rowCount(); ++i)
        {
            idsToKeep.push_back(m_table.item(i, nameColumn)->data(idRole).toString());
        }

        config.RemoveOldKeys( FloorPlanSchema::camIdKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::camGridRowKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::camGridColKey, idsToKeep );
        config.RemoveOldKeys( FloorPlanSchema::camRotationKey, idsToKeep );

        for (int i = 0; i < m_table.rowCount(); ++i)
        {
		    config.SetKeyValue( FloorPlanSchema::camRotationKey,
                                KeyValue::from( m_table.item(i, nameColumn)->data(rotRole).toInt() ),
                                m_table.item(i, nameColumn)->data(idRole).toString() );
        }

        SetConfig( config ); // Since we need to re-number the entries and we won't
                             // get SetConfig called as we're requesting the update
    }

    virtual void SetConfig( const WbConfig& config )
    {
        typedef WbKeyValues::ValueIdPairList ValueIdPairList;
        m_table.clearContents();
        const ValueIdPairList cameraIds(config.GetKeyValues(FloorPlanSchema::camIdKey));
        m_table.setRowCount(cameraIds.size());
		m_table.setColumnCount(NUMCOLUMN);

        for (int i = 0; i < (int)cameraIds.size(); ++i)
        {
            SetTableRow(i, config, cameraIds.at( i ));
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

	void clicked(int row)
	{
	    int rot = m_table.item( row, nameColumn )->data(rotRole).toInt();
	    m_table.item( row, nameColumn )->setData(rotRole, (rot + 90) % 360);
		DataChanged();
	}

private:

    int ComfirmDeletion(const int currentRow)
    {
        QMessageBox userPrompt(&m_table);
        userPrompt.setWindowTitle(tr("Remove Entry"));
        userPrompt.setText(tr("Really remove %1?  This action cannot be undone.")
                           .arg(m_table.item(currentRow, nameColumn)->text()));
        userPrompt.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        userPrompt.setDefaultButton(QMessageBox::No);
        return userPrompt.exec();
    }

    void SetTableRow( const int row,
                      const WbConfig& config,
                      const WbKeyValues::ValueIdPair& cameraIdPair  )
    {
        const QString camName( cameraIdPair.value.ToQString() );

        const KeyValue rotKeyValue( config.GetKeyValue( FloorPlanSchema::camRotationKey,
                                                          cameraIdPair.id ) );

		int camRot = -1;

        if ( !rotKeyValue.IsNull() ) camRot = rotKeyValue.ToInt();

        m_table.setItem( row, nameColumn, CreateNameItem( cameraIdPair.id, camRot, QString( camName ) ));

		m_table.setCellWidget(row, btnColumn, CreateButton( row ));
	}

    QTableWidgetItem* const CreateItem( const int value )
    {
        QString itemString;
        if ( value >= 0 )
        {
            itemString = QString::number( value );
        }
        QTableWidgetItem* const descriptionItem = new QTableWidgetItem(itemString);
        descriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        return descriptionItem;
    }

    QTableWidgetItem* const CreateNameItem(const KeyId& id,
										   const int rot,
                                           const QString& camName )
    {
        QTableWidgetItem* const nameItem = new QTableWidgetItem(camName);
        nameItem->setData(idRole, id);
        nameItem->setData(rotRole, rot);
        return nameItem;
    }

	QPushButton* const CreateButton( const int row )
    {
	    QPushButton *button = new QPushButton( &m_table );
		button->setIcon(QIcon(":/rotate.png"));

		m_signalMapper->setMapping( button, row );
   	    connect(button, SIGNAL( clicked() ), m_signalMapper, SLOT( map() ));

        return button;
    }

    QTableWidget& m_table;

	QSignalMapper* m_signalMapper;
};

#endif // FLOORPLANTABLEMAPPER_H
