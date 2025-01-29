#ifndef PROXYMODELS_H
#define PROXYMODELS_H

#include <QSortFilterProxyModel>

// == FuzzyProxyModel =========================================================
// ============================================================================

class FuzzyProxyModel : public QSortFilterProxyModel {
public:
    FuzzyProxyModel(QObject* parent = nullptr);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

public slots:
    void onFilterChanged(QString filter);

public:
    QString filter_;
};

// == EmptyFilterProxyModel ===================================================
// ============================================================================

class EmptyFilterProxyModel : public QSortFilterProxyModel {
public:
    EmptyFilterProxyModel(int columnToCheck, QObject* parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    int column_ = 0;
};

#endif // PROXYMODELS_H
