#ifndef IZMODELS_VARIANTLISTMODEL_H
#define IZMODELS_VARIANTLISTMODEL_H

#include "IzModels/AbstractItemModel.h"
#include "IzModels_Global.h"

namespace IzModels
{
	// Simple QVariantList -> AbstractItemModel transform model
	// first element defines column count and available roleNames
	class IZMODELSSHARED_EXPORT VariantListModel : public AbstractItemModel
	{
		Q_OBJECT
		Q_DISABLE_COPY(VariantListModel)

		// variant list used to populate model data
		Q_PROPERTY(QVariantList variantList READ variantList WRITE setVariantList NOTIFY variantListChanged FINAL)

	public:
		// ctor
		explicit VariantListModel(QObject* parent = nullptr);

		// dtor
		~VariantListModel() = default;

		// m_variantList getter . setter
		const QVariantList& variantList() const;
		void setVariantList(const QVariantList& variantList);

		// QAbstractItemModel interface start

		QModelIndex index(int row, int column, const QModelIndex& parent) const override;
		QModelIndex parent(const QModelIndex& child) const override;
		int rowCount(const QModelIndex& parent) const override;
		int columnCount(const QModelIndex& parent) const override;
		QVariant data(const QModelIndex& index, int role) const override;

		// QAbstractItemModel interface end

	private:
		// internal model data
		QVariantList m_variantList;

		// 'columns' count
		int m_columnsCount{ 0 };

	signals:
		// Q_PROPERTY *Changed signals
		void variantListChanged();
	};
}   // namespace IzModels

#endif   // IZMODELS_VARIANTLISTMODEL_H
