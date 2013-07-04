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

#ifndef TRACK_MODEL_H
#define TRACK_MODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QIODevice>
#include <QChar>
#include <QString>
#include <QStringList>
#include <QModelIndex>

class TrackModel : public QAbstractTableModel
{
    // The TrackModel class provides a QAbstractTableModel
    // for track results which are persisted in CSV files.

    Q_OBJECT

public:
    enum modelColumn
    {
        COLUMN_TIME = 0,
        COLUMN_POSX,
        COLUMN_POSY,
        COLUMN_HEADING,
        COLUMN_ERROR,
        COLUMN_WGM
    };

    static const int IS_DELETED = Qt::UserRole;
    static const int IS_SELECTED = Qt::UserRole+1;

public:
    TrackModel(QObject *parent = 0);
    explicit TrackModel(QIODevice *file, QObject *parent = 0, bool withHeader = false, QChar separator = ',');
    explicit TrackModel(const QString filename, QObject *parent = 0, bool withHeader = false, QChar separator = ',');
    ~TrackModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& data, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    void setHeaderData(const QStringList data);

    bool insertRow(int row, const QModelIndex& parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool removeRow(int row, const QModelIndex& parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool insertColumn(int col, const QModelIndex& parent = QModelIndex());
    bool insertColumns(int col, int count, const QModelIndex& parent = QModelIndex());
    bool removeColumn(int col, const QModelIndex& parent = QModelIndex());
    bool removeColumns(int col, int count, const QModelIndex& parent = QModelIndex());

    void setSource(QIODevice *file, bool withHeader = false, QChar separator = ',');
    void setSource(const QString filename, bool withHeader = false, QChar separator = ',');

    void toCSV(QIODevice *file, bool withHeader = false, QChar separator = ',');
    void toCSV(const QString filename, bool withHeader = false, QChar separator = ',');

    Qt::ItemFlags flags(const QModelIndex& index) const;

private:
    QStringList csvData;
    QList<bool> delData;
    QList<bool> selData;
    QStringList header;
    int         maxColumn;
};

#endif // TRACK_MODEL_H
