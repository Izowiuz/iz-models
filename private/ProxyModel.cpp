#include "IzModels/ProxyModel.h"

#include <QDebug>
#include <QtConcurrent>

#include "IzModels/AbstractItemModel.h"

IzModels::ProxyModel::ProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent)
	, m_filterFutureWatcher(new QFutureWatcher<QSet<int>>(this))
{
	// watchers setup
	connect(m_filterFutureWatcher, &QFutureWatcher<QSet<int>>::finished, this, &ProxyModel::onDataFiltered);
}

IzModels::ProxyModel::ProxyModel(IzModels::AbstractItemModel* sourceModel, QObject* parent)
	: QSortFilterProxyModel(parent)
	, m_filterFutureWatcher(new QFutureWatcher<QSet<int>>(this))
{
	// watchers setup
	connect(m_filterFutureWatcher, &QFutureWatcher<QSet<int>>::finished, this, &ProxyModel::onDataFiltered);

	// source model setup
	sourceModel->setParent(this);
	setSourceModel(sourceModel);
	m_sourceAutoAssigned = true;
}

IzModels::ProxyModel::~ProxyModel()
{
	if (m_filterFutureWatcher->isRunning()) {
		m_filterFutureWatcher->waitForFinished();
	}
}

void IzModels::ProxyModel::filterData()
{
	// check state of source model
	if (m_sourceHandler->isRefreshingData()) {
		qWarning() << "filterData() called during model refreshing.";
		return;
	}

	// if watcher is running, cancel it
	if (m_filterFutureWatcher->isRunning()) {
		m_filterFutureWatcher->cancel();
		m_filterFutureWatcher->waitForFinished();
	}

	// set filtering state
	m_isFiltering = true;
	emit isFilteringChanged();

	// if filters are empty reset filtring
	if (m_filters.isEmpty()) {
		m_filteredIndexes.clear();

		m_isFiltering = false;
		emit isFilteringChanged();

		invalidateFilter();
		return;
	}

	// cache filters
	m_cachedFilters = m_filters;

	// set of model indexes
	m_filteredIndexes.clear();
	for (int i{ 0 }; i < m_sourceHandler->rowCount(); ++i) {
		m_modelIndexes.insert(i);
	}

	// launch concurrent filtering
	// clang-format off
	QFuture<QSet<int>> filteredData = QtConcurrent::filteredReduced<QSet<int>>(m_modelIndexes,
	[this](int row) {
	   int hits{ 0 };
	   QHashIterator<QPair<int, int>, Filter> it(m_cachedFilters);
	   while (it.hasNext()) {
		   it.next();
		   // we have to check if given column comes from 'attachedRole'
		   // if yes, we set column to 0
		   if (it.value().type == FilterType::QRegularExpression) {
			   if (m_sourceHandler->data(m_sourceHandler->index(row, it.key().first == -2
																? 0
																: it.key().first), it.key().second).toString().contains(it.value().regexValue)) {
				   hits++;
			   }
		   } else {
			   if (m_sourceHandler->data(m_sourceHandler->index(row, it.key().first == -2
																? 0
																: it.key().first), it.key().second) == it.value().stringValue) {
				   hits++;
			   }
		   }
	   }
	   return hits == m_cachedFilters.size();
	},
	[](QSet<int>& set, int row) {
	   set.insert(row);
	});
	// clang-format on
	m_filterFutureWatcher->setFuture(filteredData);
}

bool IzModels::ProxyModel::sourceIsValid() const
{
	return m_sourceHandler != nullptr;
}

bool IzModels::ProxyModel::applyFilter(int column, const QString& value, bool exactValue)
{
	// invalid column -> bail out
	if (column == -1) {
		return false;
	}

	QPair<int, int> pair{ column, Qt::DisplayRole };

	// value is empty -> remove associated filter
	if (value.isEmpty()) {
		if (m_filters.remove(pair) != 0) {
			filterData();
		}

		return true;
	}

	// check if filter actually changed
	// we are returning true here to be consistently react on the view side
	if (m_filters.value(pair).baseValue == value) {
		return true;
	}

	Filter filter;

	if (!exactValue) {
		// regex filter
		auto re = QRegularExpression(value, QRegularExpression::PatternOption::CaseInsensitiveOption);

		if (re.isValid()) {
			filter.type       = FilterType::QRegularExpression;
			filter.regexValue = std::move(re);
			filter.baseValue  = value;
			m_filters.insert(pair, filter);
		} else {
			return false;
		}
	} else {
		// exact filter
		filter.type        = FilterType::Exact;
		filter.stringValue = value;
		filter.baseValue   = value;
		m_filters.insert(pair, filter);
	}

	filterData();

	return true;
}

bool IzModels::ProxyModel::applyFilter(const QString& roleName, const QString& value, bool exactValue)
{
	// if given role is ot attached type we are setting it's column index to -2
	int columnIndex = m_sourceHandler->isAttachedRole(roleName) ? -2 : m_sourceHandler->roleNameToColumn(roleName);
	int role        = m_sourceHandler->roleNameToRole(roleName);

	// invalid role or column -> bail out
	if (columnIndex == -1 || role == -1) {
		return false;
	}

	QPair<int, int> pair{ columnIndex, role };

	// value is empty -> remove associated filter
	if (value.isEmpty()) {
		if (m_filters.remove(pair) != 0) {
			filterData();
		}

		return true;
	}

	// check if filter actually changed
	// we are returning true here to be consistently react on the view side
	if (m_filters.value(pair).baseValue == value) {
		return true;
	}

	Filter filter;

	if (!exactValue) {
		// regex filter
		auto re = QRegularExpression(value, QRegularExpression::PatternOption::CaseInsensitiveOption);

		if (re.isValid()) {
			filter.type       = FilterType::QRegularExpression;
			filter.baseValue  = value;
			filter.regexValue = std::move(re);
			m_filters.insert(pair, filter);
		} else {
			return false;
		}
	} else {
		// exact filter
		filter.type        = FilterType::Exact;
		filter.stringValue = value;
		filter.baseValue   = value;
		m_filters.insert(pair, filter);
	}

	filterData();

	return true;
}

void IzModels::ProxyModel::refilterData()
{
	filterData();
}

QVariant IzModels::ProxyModel::dataFromRoleName(int row, int column, const QString& roleName) const
{
	return data(index(row, column), m_sourceHandler->roleNameToRole(roleName));
}

QVariantMap IzModels::ProxyModel::rowAsMap(int row) const
{
	return m_sourceHandler->rowAsMap(mapToSource(index(row, 0)).row());
}

int IzModels::ProxyModel::sourceRow(int proxyRow) const
{
	return mapToSource(index(proxyRow, 0)).row();
}

int IzModels::ProxyModel::sourceColumn(int proxyColumn) const
{
	return mapToSource(index(0, proxyColumn)).column();
}

void IzModels::ProxyModel::callFunctionalOnData(std::function<void()> fn)
{
	auto f = std::move(fn);
	f();
}

bool IzModels::ProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
	Q_UNUSED(source_parent)
	return !m_filters.empty() ? m_filteredIndexes.contains(source_row) : true;
}

bool IzModels::ProxyModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
{
	Q_UNUSED(source_column)
	Q_UNUSED(source_parent)
	return true;
}

void IzModels::ProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
	if (m_sourceAutoAssigned) {
		qCritical() << "This instance of IzModels::ProxyModel has auto assigned source model. Resetting currently is not possible.";
		return;
	}

	auto sm = qobject_cast<AbstractItemModel*>(sourceModel);
	if (sm == nullptr) {
		qCritical() << "Object:" << sourceModel << "is not a subclass of IzModels::AbstractItemModel. Source model will be invalid.";
		return;
	}

	QSortFilterProxyModel::setSourceModel(sourceModel);

	// WARNING: mocno podejrzany konstrukt :P
	// source model connects
	m_sourceHandler = sm;

	connect(m_sourceHandler, &IzModels::AbstractItemModel::dataRefreshStarted, this, [this]() {
		m_filteredIndexes.clear();
		m_modelIndexes.clear();
	});

	// if data was refreshed and filters are defined refilter data
	connect(m_sourceHandler, &IzModels::AbstractItemModel::dataRefreshEnded, this, [this]() {
		if (!m_filters.empty()) {
			filterData();
		}
	});
}

void IzModels::ProxyModel::onDataFiltered()
{
	if (!m_filterFutureWatcher->isCanceled()) {
		m_filteredIndexes = m_filterFutureWatcher->result();

		beginResetModel();
		endResetModel();

		m_isFiltering = false;
		emit isFilteringChanged();
	}
}
