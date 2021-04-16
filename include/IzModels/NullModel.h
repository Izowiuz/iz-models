#pragma once

#include "IzModels/AbstractItemModel.h"
#include "IzModels_Global.h"

namespace IzModels
{
    // A NULL object implementation
    class IZMODELSSHARED_EXPORT NullModel final : public AbstractItemModel
    {
        Q_OBJECT
        Q_DISABLE_COPY(NullModel)

    public:
        // ctor
        NullModel(QObject* parent = nullptr);

        // dtor
        ~NullModel() = default;

        // QAbstractItemModel interface start

        QModelIndex index(int row, int column, const QModelIndex& parent) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        int rowCount(const QModelIndex& parent) const override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant data(const QModelIndex& index, int role) const override;

        // QAbstractItemModel interface end

        // AbstractItemModel interface start

        int roleNameToColumn(const QString& roleName) override;

        // AbstractItemModel interface end
    };

}   // namespace IzModels
