#include "IzModels/VariantListModel.h"

#include <QDebug>

IzModels::VariantListModel::VariantListModel(QObject* parent)
	: AbstractItemModel(parent)
{
}

const QVariantList &IzModels::VariantListModel::variantList() const
{
	return m_variantList;
}

void IzModels::VariantListModel::setVariantList(const QVariantList& variantList)
{
	if (m_variantList != variantList) {
		emit dataRefreshStarted();
		beginResetModel();

		m_variantList = variantList;

		// if first element is not empty - create role names
		if (!m_variantList.empty()) {
			QMap<QString, QVariant> tmp = m_variantList.first().toMap();
			int roleStartPosition = Qt::UserRole;
			QHash<int, QByteArray> rn;

			QMapIterator<QString, QVariant> it(tmp);
			while (it.hasNext()) {
				it.next();
				rn.insert(roleStartPosition, it.key().toUtf8());
				roleStartPosition++;
			}

			m_columnsCount = rn.size();
			cacheRoleNames(rn);
		} else {
			m_columnsCount = 0;
			clearCachedRoleNames();
		}

		endResetModel();
		emit dataRefreshEnded();
		emit variantListChanged();
	}
}

QModelIndex IzModels::VariantListModel::index(int row, int column, const QModelIndex& parent) const
{
	if (hasIndex(row, column, parent)) {
		return createIndex(row, column);
	}

	return {};
}

QModelIndex IzModels::VariantListModel::parent(const QModelIndex& child) const
{
	Q_UNUSED(child)

	return {};
}

int IzModels::VariantListModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return m_variantList.size();
}

int IzModels::VariantListModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return m_columnsCount;
}

QVariant IzModels::VariantListModel::data(const QModelIndex& index, int role) const
{
	// we are not supporing, for now, data reverting functionality in this model
	if (!index.isValid()) {
		return {};
	}

	if (role >= Qt::UserRole) {
		return m_variantList.at(index.row()).toMap().value(roleToRoleName(role));
	}

	return {};
}
