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

#include "TrackModel.h"

#include <QFile>
#include <QDebug>

#include "TrackView.h"

TrackModel::TrackModel(QObject* parent) :
    QAbstractTableModel(parent),
    csvData(),
    delData(),
    selData(),
    header(),
    maxColumn(0)
{
    // Construct a new TrackModel with parent.
}

TrackModel::TrackModel(QIODevice* file,
                       QObject*   parent,
                       bool       withHeader,
                       QChar      separator) :
    QAbstractTableModel(parent),
    csvData(),
    delData(),
    selData(),
    header(),
    maxColumn(0)
{
    // Constructs a TrackModel from a QIODevice file as source
    // withHeader specifies whether the data on the device contains
    // a header or not. Separator is the separator to use for
    // the columns. Commonly used separators are ',' '\\t' ';'

    setSource(file, withHeader, separator);
}

TrackModel::TrackModel(const QString filename,
                       QObject*      parent,
                       bool          withHeader,
                       QChar         separator) :
    QAbstractTableModel(parent),
    csvData(),
    delData(),
    selData(),
    header(),
    maxColumn(0)
{
    // Constructs a TrackModel from filename as source.
    // withHeader specifies whether the data in the file
    // contains a header or not. separator is the separator
    // to use for the columns. Commonly used separators
    // are ','  '\\t' ';'

    QFile src(filename);

    setSource(&src, withHeader, separator);
}

TrackModel::~TrackModel()
{
}

int TrackModel::rowCount(const QModelIndex& parent) const
{

    if (parent.row() != -1 && parent.column() != -1) return 0;

    return csvData.count();
}

int TrackModel::columnCount(const QModelIndex& parent) const
{
    if (parent.row() != -1 && parent.column() != -1) return 0;

    return maxColumn;
}

QVariant TrackModel::data(const QModelIndex& idx, int role) const
{
    if (idx.parent() != QModelIndex()) return QVariant();

    if (role == Qt::DisplayRole ||
        role == Qt::EditRole)
    {
        return csvData[idx.row()].section(QChar(1), idx.column(), idx.column());
    }

    if (role == Qt::BackgroundRole)
    {
        if (!delData[idx.row()])
        {
            return QVariant(Qt::red);
        }
    }

    if (role == IS_DELETED)
    {
        return delData[idx.row()];
    }

    if (role == IS_SELECTED)
    {
        return selData[idx.row()];
    }

    return QVariant();
}

QVariant TrackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < header.count() && orientation == Qt::Horizontal && (role == Qt::DisplayRole ||
                                                                      role == Qt::EditRole ||
                                                                      role == IS_DELETED))
    {
        return header[section];
    }
    else
    {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
}

void TrackModel::setSource(const QString filename, bool withHeader, QChar separator)
{
    // Reads the CSV data from file. withHeader specifies whether the
    // data on the device contains a header or not. separator is the
    // separator to use for the columns. Commonly used separators are
    // ','  '\\t' ';'

    QFile src(filename);

    setSource(&src, withHeader, separator);
}

void TrackModel::setSource(QIODevice *file, bool withHeader, QChar separator)
{
    // Reads the cvs data from filename. withHeader specifies whether
    // the data in the file contains a header or not. separator is the
    // separator to use for the columns. most widely used separators
    // are ','  '\\t' ';'

    QString l;
    int size;
    bool isQuoted, headerSet = false;

    if (!file->isOpen()) file->open(QIODevice::ReadOnly);

    if (withHeader)
        maxColumn = 0;
    else
        maxColumn = header.size();

	csvData.clear();
	delData.clear();
	selData.clear();

    while (!file->atEnd())
    {
        l = file->readLine();
        l.remove('\n');
        l.remove('\r');
        size = l.length();
        isQuoted = false;

        for (int i = 0; i < size; i++)
        {
            if (i > 0)
            {
                if (l[i] == '"' && l[i-1] != '\\') isQuoted = !isQuoted;
                else if (!isQuoted && l[i] == separator) l[i] = QChar(1);
            }
            else
            {
                if (l[i] == '"') isQuoted = !isQuoted;
                else if (!isQuoted && l[i] == separator) l[i] = QChar(1);
            }
        }

        if (l.count(QChar(1)) + 1 > maxColumn) maxColumn = l.count(QChar(1)) + 1;

        if (withHeader && !headerSet)
        {
            header = l.split(QChar(1));
            headerSet = true;
        }
        else
        {
            csvData.append(l);
            delData.append(true);
            selData.append(false);
        }
    }

    file->close();
}

void TrackModel::setHeaderData(const QStringList data)
{
    header = data;

    emit headerDataChanged(Qt::Horizontal, 0, data.count());
}

bool TrackModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
    if (index.parent() != QModelIndex()) return false;

    QString before, after;

    if (role == Qt::DisplayRole ||
        role == Qt::EditRole)
    {
        if (index.row() >= rowCount() ||
            index.row() < 0 ||
            index.column() >= columnCount() ||
            index.column() < 0) return false;

        if (index.column() != 0)
            before = csvData[index.row()].section(QChar(1), 0, index.column() - 1) + QChar(1);
        else
            before = "";

        after = csvData[index.row()].section(QChar(1), index.column() + 1);
        csvData[index.row()] = before + data.toString() + QChar(1) + after;

        emit dataChanged(index, index);

        return true;
    }

    if (role == IS_DELETED)
    {
        if (index.row() >= rowCount() ||
            index.row() < 0 ||
            index.column() >= columnCount() ||
            index.column() < 0) return false;

        delData[index.row()] = data.toBool();

        emit dataChanged(index, index);

        return true;
    }

    if (role == IS_SELECTED)
    {
        if (index.row() >= rowCount() ||
            index.row() < 0 ||
            index.column() >= columnCount() ||
            index.column() < 0) return false;

        selData[index.row()] = data.toBool();

        emit dataChanged(index, index);

        return true;
    }

    return false;
}

bool TrackModel::insertRow(int row, const QModelIndex& parent)
{
    return insertRows(row, 1, parent);
}

bool TrackModel::insertRows(int row, int count, const QModelIndex& parent)
{
    if (parent != QModelIndex() || row < 0) return false;

    emit beginInsertRows(parent, row, row + count);

    if (row >= rowCount())
    {
        for (int i = 0; i < count; i++)
        {
            csvData << "";
            delData << true;
            selData << false;
        }
    }
    else
    {
        for (int i = 0; i < count; i++)
        {
            csvData.insert(row, "");
            delData.insert(row, true);
            selData.insert(row, false);
        }
    }

    emit endInsertRows();

    return true;
}

bool TrackModel::removeRow(int row, const QModelIndex& parent)
{
    return removeRows(row, 1, parent);
}

bool TrackModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent != QModelIndex() || row < 0) return false;

    if (row >= rowCount()) return false;

    if (row + count >= rowCount()) count = rowCount() - row;

    emit beginRemoveRows(parent, row, row + count);

    for (int i = 0; i < count; i++)
    {
        csvData.removeAt(row);
        delData.removeAt(row);
        selData.removeAt(row);
    }

    emit endRemoveRows();

    return true;
}

bool TrackModel::insertColumn(int col, const QModelIndex& parent)
{
    return insertColumns(col, 1, parent);
}

bool TrackModel::insertColumns(int col, int count, const QModelIndex& parent)
{
    if (parent != QModelIndex() || col < 0) return false;

    beginInsertColumns(parent, col, col + count - 1);

    if (col < columnCount())
    {
        QString before, after;
        for (int i = 0; i < rowCount(); i++)
        {
            if (col > 0)
                before = csvData[i].section(QChar(1), 0, col - 1) + QChar(1);
            else
                before = "";

            after = csvData[i].section(QChar(1), col);
            csvData[i] = before + QString(count, QChar(1)) + after;
        }
    }

    for (int i = 0; i < count; i++)
        header.insert(col, "");

    maxColumn += count;
    endInsertColumns();

    return true;
}

bool TrackModel::removeColumn(int col, const QModelIndex& parent)
{
    return removeColumns(col, 1, parent);
}

bool TrackModel::removeColumns(int col, int count, const QModelIndex& parent)
{
    if (parent != QModelIndex() || col < 0) return false;

    if (col >= columnCount()) return false;

    if (col + count >= columnCount()) count = columnCount() - col;

    emit beginRemoveColumns(parent, col, col + count);

    QString before, after;

    for (int i = 0; i < rowCount(); i++)
    {
        if (col > 0)
            before = csvData[i].section(QChar(1), 0, col - 1) + QChar(1);
        else
            before = "";

        after = csvData[i].section(QChar(1), col + count);
        csvData[i] = before + after;
    }

    for (int i = 0; i < count; i++)
        header.removeAt(col);

    emit endRemoveColumns();

    return true;
}

void TrackModel::toCSV(QIODevice* dest, bool withHeader, QChar separator)
{
    // Writes the CSV data to file. withHeader specifies whether to
    // write the header or not. separator is the separator to use
    // for the columns. Commonly used separators are ','  '\\t' ';'

    int row, col, rows, cols;
    rows = rowCount();
    cols = columnCount();
    QString data;

    if (!dest->isOpen()) dest->open(QIODevice::WriteOnly |
                                    QIODevice::Truncate);

    if (withHeader)
    {
        data = "";

        for (col = 0; col < cols; ++col)
        {
            data += header.at(col);

            if (col < cols - 1) data += separator;
        }

        data += '\n';
        dest->write(data.toLatin1());
    }

    for (row = 0; row < rows; ++row)
    {
        if (delData[row])
        {
            data = "";

            for (col = 0; col < cols; ++col)
            {
                data += csvData[row].section(QChar(1), col, col);

                if (col < cols - 1) data += separator;
            }

            data += '\n';
            dest->write(data.toLatin1());
        }
    }

    dest->close();
}

void TrackModel::toCSV(const QString filename, bool withHeader, QChar separator)
{
    // Writes the CSV data to filename. withHeader specifies whether
    // to write the header or not. separator is the separator to use
    // for the columns. Commonly used separators are ','  '\\t' ';'

    QFile dest(filename);

    toCSV(&dest, withHeader, separator);
}

Qt::ItemFlags TrackModel::flags(const QModelIndex& index) const
{
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}
