#ifndef STATICTWODAMODEL_H
#define STATICTWODAMODEL_H

#include <QAbstractItemModel>

namespace nw {
struct StaticTwoDA;
}

class StaticTwoDAModel : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit StaticTwoDAModel(const nw::StaticTwoDA* tda, int name_column, QObject* parent = nullptr);

    void setColumns(QList<int> columns);

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

private:
    const nw::StaticTwoDA* tda_;
    int name_column_;
    QList<int> columns_;
};

#endif // STATICTWODAMODEL_H
