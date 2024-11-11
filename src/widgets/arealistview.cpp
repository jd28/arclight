#include "arealistview.h"
#include "proxymodels.h"

#include "util/strings.h"

#include "nw/kernel/Resources.hpp"
#include "nw/kernel/Strings.hpp"
#include "nw/objects/Area.hpp"
#include "nw/objects/Module.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

// == AreaListItem ===========================================================
// ============================================================================

AreaListItem::AreaListItem(nw::Area* area, AreaListItem* parent)
    : AbstractTreeItem(0, parent)
    , area_(area)
    , type_{AreaListItemType::area}
{
}

QVariant AreaListItem::data(int column, int role) const
{
    Q_UNUSED(role);
    if (column != 0) { return {}; }
    if (!path_.isEmpty()) {
        return path_;
    } else if (area_) {
        return to_qstring(nw::kernel::strings().get(area_->name));
    }
    return {};
}

// == AreaListModel ===============================================================
// ============================================================================

AreaListModel::AreaListModel(nw::Module* module, QString path, QObject* parent)
    : AbstractTreeModel(parent)
    , module_{module}
    , path_{std::move(path)}
{
}

int AreaListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant AreaListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }

    AreaListItem* node = static_cast<AreaListItem*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        return node->data(0);
    }

    return {};
}

void AreaListModel::loadRootItems()
{
    for (size_t i = 0; i < module_->area_count(); ++i) {
        auto it = new AreaListItem(module_->get_area(i));
        addRootItem(it);
    }
}

// == AreaListView ===========================================================
// ============================================================================

AreaListView::AreaListView(nw::Module* module, QString path, QWidget* parent)
    : ArclightTreeView(parent)
    , module_{module}
    , path_{std::move(path)}

{
    setHeaderHidden(true);
    connect(this, &QTreeView::doubleClicked, this, &AreaListView::onDoubleClicked);
}

AreaListView::~AreaListView()
{
    delete model_;
}

void AreaListView::activateModel()
{
    filter_ = new FuzzyProxyModel(this);
    filter_->setRecursiveFilteringEnabled(true);
    filter_->setSourceModel(model_);
    filter_->sort(0);
    setModel(filter_);
}

AreaListModel* AreaListView::loadModel()
{
    model_ = new AreaListModel(module_, path_);
    model_->loadRootItems();
    return model_;
}

void AreaListView::onDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) { return; }
    QModelIndex sourceIndex = filter_->mapToSource(index);
    if (!sourceIndex.isValid()) { return; }

    emit itemDoubleClicked(static_cast<AreaListItem*>(sourceIndex.internalPointer()));
}
