#pragma once

#include <QFutureWatcher>
#include <QPair>
#include <QRegularExpression>
#include <QSet>
#include <QSortFilterProxyModel>

#include "IzModels_Global.h"

namespace IzModels
{
    class AbstractItemModel;

    // TODO: wykorzystać NullModel
    // TODO: byc może pozbyć się m_sourceAutoAssigned
    class IZMODELSSHARED_EXPORT ProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT
        Q_DISABLE_COPY(ProxyModel)

        // true if model is currently filtering data
        Q_PROPERTY(bool isFiltering MEMBER m_isFiltering NOTIFY isFilteringChanged FINAL)

        // filter types
        enum class FilterType : uint8_t {
            QRegularExpression,
            Exact
        };

        // filter definition
        struct Filter {
            // tye of the filter
            FilterType type;

            // string value of the filter
            QString stringValue;

            // regex value of the filter
            QRegularExpression regexValue;

            // value used to generate filter value
            QString baseValue;

            // operator ==
            // compares baseValue
            inline bool operator==(const Filter& other) const
            {
                return baseValue == other.baseValue;
            }

            // operator !=
            // compares baseValue
            inline bool operator!=(const Filter& other) const
            {
                return baseValue != other.baseValue;
            }
        };

    public:
        // ctor
        ProxyModel(QObject* parent = nullptr);

        // automatically assigns and reparents source model
        ProxyModel(AbstractItemModel* sourceModel, QObject* parent = nullptr);

        // dtor
        virtual ~ProxyModel();

        // returns true if source model is valid
        bool sourceIsValid() const;

        // applies new filter to given column
        // this bool as a return is a little bogus ...
        Q_INVOKABLE bool applyFilter(int column, const QString& value, bool exactValue = false);

        // applies new filter to given role name
        // this bool as a return is a little bogus ...
        Q_INVOKABLE bool applyFilter(const QString& roleName, const QString& value, bool exactValue = false);

        // refilters model
        Q_INVOKABLE void refilterData();

        // returns data from row, column and roleName
        Q_INVOKABLE QVariant dataFromRoleName(int row, int column, const QString& roleName) const;

        // returns row's data as QVariantMap [roleName, value]
        Q_INVOKABLE QVariantMap rowAsMap(int row) const;

        // returns source row for given proxy row
        Q_INVOKABLE int sourceRow(int proxyRow) const;

        // returns source column for given proxy column
        Q_INVOKABLE int sourceColumn(int proxyColumn) const;

        // called on this model, moves fn
        // care needs to be taken calling this method: fn has to emit dataAboutToBeChanged() on it's own
        void callFunctionalOnData(std::function<void(void)> fn);

    protected:
        // QSortFilterProxyModel start

        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
        bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;

        // QSortFilterProxyModel end

        // QAbstractProxyModel interface start

        void setSourceModel(QAbstractItemModel* sourceModel) override;

        // QAbstractProxyModel interface start

    private:
        // starts data filtering process
        void filterData();

        // set of prefiltered indexes
        QSet<int> m_modelIndexes;

        // set of filtered indexes - only those will be shown in connected view
        QSet<int> m_filteredIndexes;

        // parses filtered data
        void onDataFiltered();

        // true if model is currently filtering data
        bool m_isFiltering{ false };

        // [column, role] -> filter value hash
        QHash<QPair<int, int>, Filter> m_filters;

        // cached column filters
        // WARNING: this member is used during filter operation
        QHash<QPair<int, int>, Filter> m_cachedFilters;

        // filter data future watcher
        QFutureWatcher<QSet<int>>* m_filterFutureWatcher;

        // source model handler
        AbstractItemModel* m_sourceHandler{ nullptr };

        // true if source model was auto assigned to this proxy instance
        // this prevents resetting source to new model
        bool m_sourceAutoAssigned{ false };

        // dummy model

    signals:
        // Q_PROPERTY *Changed signals
        void isFilteringChanged();
    };
}   // namespace IzModels
