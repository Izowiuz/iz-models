#include "IzModels/NullModel.h"

#include <QDebug>

IzModels::NullModel::NullModel(QObject* parent)
    : AbstractItemModel(parent)
{
}

QModelIndex IzModels::NullModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    return {};
}

QModelIndex IzModels::NullModel::parent(const QModelIndex& child) const
{
    Q_UNUSED(child)
    return {};
}

int IzModels::NullModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 0;
}

int IzModels::NullModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 0;
}

QVariant IzModels::NullModel::data(const QModelIndex& index, int role) const
{
    Q_UNUSED(index)
    Q_UNUSED(role)
    return {};
}

int IzModels::NullModel::roleNameToColumn(const QString& roleName)
{
    return -1;
}
