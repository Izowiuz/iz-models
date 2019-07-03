#include "IzModels/AbstractItemModel.h"

#include <QDebug>
#include <QSignalBlocker>

IzModels::AbstractItemModel::AbstractItemModel(QObject* parent)
	: QAbstractItemModel(parent)
{
	// emit custom signals and set desired state when refreshing data started
	connect(this, &AbstractItemModel::dataRefreshStarted, this, [this]() {
		m_isRefreshingData = true;
		emit isRefreshingDataChanged();
	});

	// emit custom signals and set desired state when refreshing data ended
	connect(this, &AbstractItemModel::dataRefreshEnded, this, [this]() {
		m_isRefreshingData = false;
		emit isRefreshingDataChanged();
	});

	// TODO: to powinno być gdzieś indziej pewnie
	// reset m_changeBuffer on model reset
	connect(this, &AbstractItemModel::modelAboutToBeReset, this, [this]() {
		m_changedBuffer.clear();
		m_addedBuffer.clear();
		m_removedBuffer = 0;
		m_attachedRolesValues.clear();

		emit changedIndexesChanged();
		emit addedIndexesChanged();
		emit isDirtyChanged();
	});
}

int IzModels::AbstractItemModel::roleNameToColumn(const QString& roleName)
{
	Q_UNUSED(roleName)
	return -1;
}

int IzModels::AbstractItemModel::roleNameToRole(const QString& roleName) const
{
	return m_cachedReversedRoleNames.value(roleName.toUtf8());
}

QString IzModels::AbstractItemModel::roleToRoleName(int role) const
{
	return m_cachedRoleNames.value(role);
}

QVariant IzModels::AbstractItemModel::dataFromRoleName(int row, int column, const QString& roleName) const
{
	return data(index(row, column), roleNameToRole(roleName));
}

QVariantMap IzModels::AbstractItemModel::rowAsMap(int row) const
{
	QHashIterator<int, QByteArray> it(m_cachedRoleNames);
	QVariantMap ret;

	while (it.hasNext()) {
		it.next();
		ret.insert(m_cachedRoleNames.value(it.key()), dataFromRoleName(row, 0, m_cachedRoleNames.value(it.key())));
	}

	return ret;
}

void IzModels::AbstractItemModel::clearData()
{
	beginResetModel();
	endResetModel();
}

void IzModels::AbstractItemModel::revertIndexData(const QModelIndex& index)
{
	if (!index.isValid()) {
		qWarning() << "Got invalid index:" << index;
		return;
	}

	if (!m_dataRevertEnabled) {
		return;
	}

	// if index was added there is nothing to revert
	if (indexWasAdded(index)) {
		return;
	}

	if (!m_changedBuffer.contains(index)) {
		qWarning() << "Index:" << index << "has not jet been changed.";
		return;
	}

	QSignalBlocker signalBlocker(this);

	QVector<int> roleNames;
	roleNames.reserve(m_cachedRoleNames.size());
	QHash<int, QVariant> originalData = m_changedBuffer.value(index);
	QHashIterator<int, QVariant> it(originalData);

	while (it.hasNext()) {
		it.next();

		if (it.key() == static_cast<int>(AbstractItemModelRoles::IsChanged) || it.key() == static_cast<int>(AbstractItemModelRoles::IsAdded)) {
			continue;
		}

		setData(index, it.value(), it.key());
		roleNames.push_back(it.key());
	}

	signalBlocker.unblock();

	emit dataChanged(index, index, roleNames);
	emit changedIndexesChanged();
}

void IzModels::AbstractItemModel::clearChangedIndexes()
{
	m_changedBuffer.clear();
	m_addedBuffer.clear();
	m_removedBuffer = 0;

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), { static_cast<int>(AbstractItemModelRoles::IsChanged), static_cast<int>(AbstractItemModelRoles::IsAdded) });
	emit changedIndexesChanged();
	emit addedIndexesChanged();
	emit isDirtyChanged();
}

bool IzModels::AbstractItemModel::isRefreshingData() const
{
	return m_isRefreshingData;
}

bool IzModels::AbstractItemModel::isDirty() const
{
	return !m_changedBuffer.empty() || !m_addedBuffer.empty() || m_removedBuffer != 0;
}

const QHash<int, QByteArray>& IzModels::AbstractItemModel::cachedRoleNames() const
{
	return m_cachedRoleNames;
}

int IzModels::AbstractItemModel::attachedRoleLowerLimit() const
{
	return m_attachedRoleLowerLimit;
}

const QVariantMap& IzModels::AbstractItemModel::attachedRoles() const
{
	return m_attachedRoles;
}

void IzModels::AbstractItemModel::setAttachedRoles(const QVariantMap& attachedRoles)
{
	if (m_attachedRoles != attachedRoles) {
		m_attachedRoles = attachedRoles;
		emit attachedRolesChanged();
	}
}

QVariant IzModels::AbstractItemModel::attachedRoleValue(const QModelIndex& index, int attachedRole) const
{
	if (!index.isValid()) {
		qWarning() << "Got invalid index:" << index;
		return {};
	}

	return m_attachedRolesValues.value({ index, attachedRole }, m_attachedRoles.value(m_cachedRoleNames.value(attachedRole)));
}

bool IzModels::AbstractItemModel::setAttachedRoleValue(const QModelIndex& index, const QVariant& value, int attachedRole)
{
	if (!index.isValid()) {
		qWarning() << "Got invalid index:" << index;
		return false;
	}

	m_attachedRolesValues.insert({ index, attachedRole }, value);
	emit dataChanged(index, index, { attachedRole });

	// later we can return different values than just 'true'
	return true;
}

void IzModels::AbstractItemModel::cacheRoleNames(const QHash<int, QByteArray>& roleNames) const
{
	m_cachedRoleNames = roleNames;
	QHash<QByteArray, int> tmp;

	QHashIterator<int, QByteArray> it(m_cachedRoleNames);
	while (it.hasNext()) {
		it.next();
		tmp.insert(it.value(), it.key());
	}
	m_cachedReversedRoleNames.swap(tmp);

	// user defined, attached role names
	int currentRoleIndex = m_attachedRoleLowerLimit;
	QMapIterator<QString, QVariant> it2(m_attachedRoles);
	while (it2.hasNext()) {
		it2.next();
		m_cachedRoleNames.insert(currentRoleIndex, it2.key().toUtf8());
		m_cachedReversedRoleNames.insert(it2.key().toUtf8(), currentRoleIndex);
		currentRoleIndex++;
	}

	// injected role names
	m_cachedRoleNames.insert(static_cast<int>(AbstractItemModelRoles::IsChanged), "iz_isChanged");
	m_cachedRoleNames.insert(static_cast<int>(AbstractItemModelRoles::IsAdded), "iz_isAdded");
	m_cachedReversedRoleNames.insert("iz_isChanged", static_cast<int>(AbstractItemModelRoles::IsChanged));
	m_cachedReversedRoleNames.insert("iz_isAdded", static_cast<int>(AbstractItemModelRoles::IsAdded));

	emit roleNamesCached();
}

void IzModels::AbstractItemModel::clearCachedRoleNames() const
{
	m_cachedRoleNames.clear();
	m_cachedReversedRoleNames.clear();
}

bool IzModels::AbstractItemModel::indexWasChanged(const QModelIndex& index) const
{
	if (!index.isValid()) {
		qWarning() << "Got invalid index:" << index;
		return false;
	}

	return m_changedBuffer.contains(index);
}

bool IzModels::AbstractItemModel::indexWasAdded(const QModelIndex& index) const
{
	if (!index.isValid()) {
		qWarning() << "Got invalid index:" << index;
		return false;
	}

	return m_addedBuffer.contains(index);
}

void IzModels::AbstractItemModel::onDataAboutToBeChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
	Q_UNUSED(roles)

	// temporary variables
	bool lastState      = isDirty();
	int currentPosition = topLeft.row();

	do {
		QModelIndex tmpIndex = index(currentPosition, 0);
		QPersistentModelIndex pIndex(tmpIndex);

		// if index was added we dont add it to the change buffer
		if (indexWasAdded(tmpIndex)) {
			return;
		}

		if (!m_changedBuffer.contains(pIndex)) {
			QHash<int, QVariant> dataSnapshot;
			QHashIterator<int, QByteArray> it(m_cachedRoleNames);

			while (it.hasNext()) {
				it.next();

				if (it.key() == static_cast<int>(AbstractItemModelRoles::IsChanged)) {
					continue;
				}

				dataSnapshot.insert(it.key(), data(tmpIndex, it.key()));
			}

			// we also inserting Qt::DisplayData roles - for fun
			dataSnapshot.insert(Qt::DisplayRole, data(tmpIndex, Qt::DisplayRole));
			m_changedBuffer.insert(pIndex, dataSnapshot);
		}

		currentPosition++;
	} while (currentPosition < bottomRight.row() + 1);

	// we don't want to emit unnecessary signals
	if (lastState != isDirty()) {
		emit isDirtyChanged();
	}

	// we emit dataChanged signals to indicate probably changed states of indexes
	emit dataChanged(index(topLeft.row(), columnCount() - 1), index(bottomRight.row(), columnCount() - 1), { static_cast<int>(AbstractItemModelRoles::IsChanged) });
}

void IzModels::AbstractItemModel::onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
	// we backoff early if this is an injected in role
	if (!roles.empty() && ((roles[0] == static_cast<int>(AbstractItemModelRoles::IsChanged)) || (roles[0] == static_cast<int>(AbstractItemModelRoles::IsAdded)))) {
		return;
	}

	// temporary variables
	bool lastState      = isDirty();
	int currentPosition = topLeft.row();

	do {
		QModelIndex tmpIndex = index(currentPosition, 0);
		QPersistentModelIndex pIndex(tmpIndex);

		// if index was added we dont update it in the change buffer
		if (indexWasAdded(tmpIndex)) {
			return;
		}

		if (m_changedBuffer.contains(pIndex)) {
			QHash<int, QVariant> dataSnapshot;
			QHashIterator<int, QByteArray> it(m_cachedRoleNames);

			while (it.hasNext()) {
				it.next();

				if (it.key() == static_cast<int>(AbstractItemModelRoles::IsChanged)) {
					continue;
				}

				dataSnapshot.insert(it.key(), data(tmpIndex, it.key()));
			}

			// we also inserting Qt::DisplayData roles - for fun
			dataSnapshot.insert(Qt::DisplayRole, data(tmpIndex, Qt::DisplayRole));

			// buffer already contains this index, check if values are the same
			if (m_changedBuffer.value(pIndex) == dataSnapshot) {
				m_changedBuffer.remove(pIndex);
			}
		}
		currentPosition++;
	} while (currentPosition < bottomRight.row() + 1);

	// we don't want to emit unnecessary signals
	if (lastState != isDirty()) {
		emit isDirtyChanged();
	}

	// we emit dataChanged signals to indicate probably changed states of indexes
	emit dataChanged(index(topLeft.row(), columnCount() - 1), index(bottomRight.row(), columnCount() - 1), { static_cast<int>(AbstractItemModelRoles::IsChanged) });
	emit changedIndexesChanged();
}

void IzModels::AbstractItemModel::onRowsInserted(const QModelIndex& parent, int first, int last)
{
	Q_UNUSED(parent)

	// temporary variables
	bool lastState   = isDirty();
	int currentIndex = first;
	int lastIndex    = last;

	do {
		m_addedBuffer.insert(index(currentIndex, 0));
		currentIndex++;
	} while (currentIndex < lastIndex + 1);

	// we don't want to emit unnecessary signals
	if (lastState != isDirty()) {
		emit isDirtyChanged();
	}

	// we emit dataChanged signals to indicate probably changed states of indexes
	emit dataChanged(index(first, columnCount() - 1), index(last, columnCount() - 1), { static_cast<int>(AbstractItemModelRoles::IsAdded) });

	emit addedIndexesChanged();
}

void IzModels::AbstractItemModel::onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last)
{
	Q_UNUSED(parent)

	// temporary variables
	bool lastState   = isDirty();
	int currentIndex = first;
	int lastIndex    = last;

	do {
		QPersistentModelIndex pIndex = index(currentIndex, 0);
		if (m_addedBuffer.contains(pIndex)) {
			m_addedBuffer.remove(pIndex);
		} else {
			m_changedBuffer.remove(pIndex);
			m_removedBuffer++;
		}
		currentIndex++;
	} while (currentIndex < lastIndex + 1);

	// we don't want to emit unnecessary signals
	if (lastState != isDirty()) {
		emit isDirtyChanged();
	}

	emit addedIndexesChanged();
}

bool IzModels::AbstractItemModel::dataRevertEnabled() const
{
	return m_dataRevertEnabled;
}

void IzModels::AbstractItemModel::setDataRevertEnabled(bool dataRevertEnabled)
{
	if (m_dataRevertEnabled != dataRevertEnabled) {
		m_dataRevertEnabled = dataRevertEnabled;

		if (m_dataRevertEnabled) {
			connect(this, &AbstractItemModel::dataAboutToBeChanged, this, &AbstractItemModel::onDataAboutToBeChanged);
			connect(this, &AbstractItemModel::dataChanged, this, &AbstractItemModel::onDataChanged);
			connect(this, &AbstractItemModel::rowsInserted, this, &AbstractItemModel::onRowsInserted);
			connect(this, &AbstractItemModel::rowsAboutToBeRemoved, this, &AbstractItemModel::onRowsAboutToBeRemoved);
		} else {
			disconnect(this, &AbstractItemModel::dataAboutToBeChanged, this, &AbstractItemModel::onDataAboutToBeChanged);
			disconnect(this, &AbstractItemModel::dataChanged, this, &AbstractItemModel::onDataChanged);
			disconnect(this, &AbstractItemModel::rowsInserted, this, &AbstractItemModel::onRowsInserted);
			disconnect(this, &AbstractItemModel::rowsAboutToBeRemoved, this, &AbstractItemModel::onRowsAboutToBeRemoved);

			m_changedBuffer.clear();
			m_addedBuffer.clear();
			m_removedBuffer = 0;

			emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1), { static_cast<int>(AbstractItemModelRoles::IsChanged), static_cast<int>(AbstractItemModelRoles::IsAdded) });
			emit changedIndexesChanged();
			emit addedIndexesChanged();
			emit isDirtyChanged();
		}

		emit dataRevertEnabledChanged();
	}
}

bool IzModels::AbstractItemModel::isAttachedRole(const QString& roleName) const
{
	return m_attachedRoles.contains(roleName);
}

int IzModels::AbstractItemModel::changedIndexes() const
{
	return m_changedBuffer.size();
}

int IzModels::AbstractItemModel::addedIndexes() const
{
	return m_addedBuffer.size();
}

void IzModels::AbstractItemModel::callFunctionalOnData(std::function<void(void)> fn)
{
	auto f = std::move(fn);
	f();
}

QHash<int, QByteArray> IzModels::AbstractItemModel::roleNames() const
{
	return cachedRoleNames();
}
