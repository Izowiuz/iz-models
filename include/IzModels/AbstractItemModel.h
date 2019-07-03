#ifndef IZMODELS_ABSTRACTITEMMODEL_H
#define IZMODELS_ABSTRACTITEMMODEL_H

#include <functional>

#include <QAbstractItemModel>
#include <QSet>

#include "IzModels/IzModels_Global.h"

namespace IzModels
{
	// TODO : cacheRoleNames -> sprawdzać powtórzenia wartości
	// TODO: lepiej zorganizowac funkcjonalność revertowania danych pod modelem
	// TODO: być może pozbyć się bool'a dataRevertEnabled?
	// TODO: zoptymalizować ustawianie starych wartości danych dla dużej ilości zmian
	class IZMODELSSHARED_EXPORT AbstractItemModel : public QAbstractItemModel
	{
		Q_OBJECT
		Q_DISABLE_COPY(AbstractItemModel)

		// true if data is being currently refreshed
		// auto set by emitting dataRefreshStarted and dataRefreshEnded signals
		Q_PROPERTY(bool isRefreshingData MEMBER m_isRefreshingData NOTIFY isRefreshingDataChanged FINAL)

		// count of the elements in the model
		Q_PROPERTY(int count READ rowCount NOTIFY isRefreshingDataChanged FINAL)

		// true if model is dirty, if it has changed or added rows
		Q_PROPERTY(bool isDirty READ isDirty NOTIFY isDirtyChanged FINAL)

		// amount of changed indexes
		Q_PROPERTY(int changedIndexes READ changedIndexes NOTIFY changedIndexesChanged FINAL)

		// amount of added indexes
		Q_PROPERTY(int addedIndexes READ addedIndexes NOTIFY addedIndexesChanged FINAL)

		// list of string used to define custom, editable role names
		// [roleName, defaultValue]
		// NOTICE: set only at model refresh
		// NOTICE: their int values start from 1000
		Q_PROPERTY(QVariantMap attachedRoles READ attachedRoles WRITE setAttachedRoles NOTIFY attachedRolesChanged FINAL)

		// if true model will allow to rever to the original data in case of data change
		// NOTICE: this also allows model to control its 'isDirty' state
		// WARNING: look out when changing this property when model is already working, this has Unforeseen Consequences λ
		Q_PROPERTY(bool dataRevertEnabled READ dataRevertEnabled WRITE setDataRevertEnabled NOTIFY dataRevertEnabledChanged FINAL)

	public:
		// reserved, injected roleNames
		enum class AbstractItemModelRoles : uint16_t {
			IsChanged = Qt::UserRole + 666,
			IsAdded
		};

		// ctor
		AbstractItemModel(QObject* parent = nullptr);

		// dtor
		virtual ~AbstractItemModel() = default;

		// transforms given QML role to column index
		// default implementation returns -1
		Q_INVOKABLE virtual int roleNameToColumn(const QString& roleName);

		// transforms given QML role name to Qt role
		Q_INVOKABLE int roleNameToRole(const QString& roleName) const;

		// transforms given Qt role to roleName
		Q_INVOKABLE QString roleToRoleName(int role) const;

		// returns data from row, column and roleName
		Q_INVOKABLE QVariant dataFromRoleName(int row, int column, const QString& roleName) const;

		// returns row's data as QVariantMap [roleName, value]
		Q_INVOKABLE QVariantMap rowAsMap(int row) const;

		// used to clear model data
		// supposed to emit begin and end reset model signals from QAIM()
		Q_INVOKABLE virtual void clearData();

		// used to revert index data to it's original values
		// NOTICE: does nothing if dataRevertEnabled is set to false
		Q_INVOKABLE void revertIndexData(const QModelIndex& index);

		// used to clear information about changed indexes
		// essentially cleans model from dirty data
		Q_INVOKABLE void clearChangedIndexes();

		// m_isRefreshingData getter
		bool isRefreshingData() const;

		// returns true if model's data was changed
		bool isDirty() const;

		// returns const iterators for m_cachedRoleNames QHAsh
		const auto cachedRoleNamesCBegin() const
		{
			return m_cachedRoleNames.cbegin();
		};

		const auto cachedRoleNamesCEnd() const
		{
			return m_cachedRoleNames.cend();
		};

		// m_cachedRoleNames getter
		const QHash<int, QByteArray>& cachedRoleNames() const;

		// m_attachedRoleLowerLimit getter
		int attachedRoleLowerLimit() const;

		// m_attachedRoles getter / setter
		const QVariantMap& attachedRoles() const;
		void setAttachedRoles(const QVariantMap& attachedRoles);

		// returns attached role value
		QVariant attachedRoleValue(const QModelIndex& index, int attachedRole) const;

		// sets attached property value
		// this function emits dataChanged signal
		bool setAttachedRoleValue(const QModelIndex& index, const QVariant& value, int attachedRole);

		// m_dataRevertEnabled getter / setter
		bool dataRevertEnabled() const;
		void setDataRevertEnabled(bool dataRevertEnabled);

		// returns true if given role is an attached role
		bool isAttachedRole(const QString& roleName) const;

		// returns amount of changed indexes
		int changedIndexes() const;

		// returns amount of added indexes
		int addedIndexes() const;

		// called on this model, moves fn
		// care needs to be taken calling this method: fn has to emit dataAboutToBeChanged() on it's own
		void callFunctionalOnData(std::function<void(void)> fn);

		// QAbstractItemModel interface start

		// default implementation returns cached role names
		// this is sufficient for 99% of cases :D
		QHash<int, QByteArray> roleNames() const override;

		// QAbstractItemModel interface end

	protected:
		// used to cache role names
		void cacheRoleNames(const QHash<int, QByteArray>& roleNames) const;

		// used to clear cached role names
		void clearCachedRoleNames() const;

		// returns true if index was changed
		bool indexWasChanged(const QModelIndex& index) const;

		// returns true if index was added
		bool indexWasAdded(const QModelIndex& index) const;

	private:
		// true if data is being currently refreshed
		bool m_isRefreshingData{ false };

		// cached role names names for use under QML views
		mutable QHash<int, QByteArray> m_cachedRoleNames;

		// reversed, cached role names
		mutable QHash<QByteArray, int> m_cachedReversedRoleNames;

		// changed ar model indexes
		// QHash<row, QHash<roleName, oldValue>>
		QHash<QPersistentModelIndex, QHash<int, QVariant>> m_changedBuffer;

		// added model indexes
		QSet<QPersistentModelIndex> m_addedBuffer;

		// number of removed indexes
		unsigned int m_removedBuffer{ 0 };

		// attached roles of the model
		QVariantMap m_attachedRoles;

		// values of the attached values
		QHash<QPair<QPersistentModelIndex, int>, QVariant> m_attachedRolesValues;

		// lower limit of the attached role names
		const int m_attachedRoleLowerLimit{ Qt::UserRole + 1000 };

		// if true model will allow to rever to the original data in case of data change
		bool m_dataRevertEnabled{ false };

		// functions used with data-revert functionality
		void onDataAboutToBeChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
		void onDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
		void onRowsInserted(const QModelIndex& parent, int first, int last);
		void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);

	signals:
		// Q_PROPERTY *Changed signals
		void isRefreshingDataChanged();
		void isDirtyChanged();
		void attachedRolesChanged();
		void dataRevertEnabledChanged();
		void changedIndexesChanged();
		void addedIndexesChanged();

		// can be emited just before data refresh operation
		void aboutToRefreshData();

		// can be emited to indicate data refresh operation start
		void dataRefreshStarted();

		// can be emited to indicate data refresh operation end
		// optional boolean parameter indicates end state of the operation
		void dataRefreshEnded(bool result = true);

		// emited when QML role names were cached
		void roleNamesCached() const;

		// ths signal has to be emited if data revert feature needs to be working properly
		void dataAboutToBeChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
	};
}   // namespace IzModels

#endif   // IZMODELS_ABSTRACTITEMMODEL_H
