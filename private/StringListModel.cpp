#include "IzModels/StringListModel.h"

#include <QDebug>

IzModels::StringListModel::StringListModel(QObject* parent)
	: IzModels::AbstractItemModel(parent)
{
}

QModelIndex IzModels::StringListModel::index(int row, int column, const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return {};
}

QModelIndex IzModels::StringListModel::parent(const QModelIndex& child) const
{
	Q_UNUSED(child)
	return {};
}

int IzModels::StringListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 0;
}

int IzModels::StringListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 0;
}

QVariant IzModels::StringListModel::data(const QModelIndex& index, int role) const
{
	return {};
}

const QStringList& IzModels::StringListModel::stringList() const
{
	return m_data;
}

void IzModels::StringListModel::setStringList(const QStringList& stringList)
{
	if (m_data != stringList) {
		m_data = stringList;
		qDebug() << m_data;
		emit stringListChanged();
	}
}

QString IzModels::StringListModel::roleName() const
{
	return m_roleName;
}

void IzModels::StringListModel::setRoleName(const QString& roleName)
{
	if (m_roleName != roleName) {
		m_roleName = roleName;
		emit roleNameChanged();
	}
}
