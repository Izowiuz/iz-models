#pragma once

#include "IzModels/AbstractItemModel.h"
#include "IzModels_Global.h"

// TODO: refaktor m_data -> m_stringList
// TODO: skończyc jeżeli to miałoby się kiedyś przydać
namespace IzModels
{
    // Simple QStringList -> AbstractItemModel transform model
    class IZMODELSSHARED_EXPORT StringListModel : public AbstractItemModel
    {
        Q_OBJECT
        Q_DISABLE_COPY(StringListModel)

        // model data
        Q_PROPERTY(QStringList stringList READ stringList WRITE setStringList NOTIFY stringListChanged FINAL)

        // role name used to get data from QML
        Q_PROPERTY(QString roleName READ roleName WRITE setRoleName NOTIFY roleNameChanged FINAL)

    public:
        // ctor
        explicit StringListModel(QObject* parent = nullptr);

        // dtor
        ~StringListModel() = default;

        // QAbstractItemModel interface start

        QModelIndex index(int row, int column, const QModelIndex& parent) const override;
        QModelIndex parent(const QModelIndex& child) const override;
        int rowCount(const QModelIndex& parent) const override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant data(const QModelIndex& index, int role) const override;

        // QAbstractItemModel interface start

        // m_data getter / setter
        const QStringList& stringList() const;
        void setStringList(const QStringList& stringList);

        // m_roleName getter / setter
        QString roleName() const;
        void setRoleName(const QString& roleName);

    private:
        // internal data of the model
        QStringList m_data;

        // role name used to get data from QML
        QString m_roleName;

    signals:
        // Q_PROPERTY *Changed signals
        void stringListChanged();
        void roleNameChanged();
    };
}   // namespace IzModels
