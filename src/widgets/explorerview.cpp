#include "explorerview.h"

#include "util/strings.h"

#include "nw/kernel/Resources.hpp"
#include "nw/resources/ResourceType.hpp"

#include "absl/container/flat_hash_map.h"

extern "C" {
#include "fzy/match.h"
}

#include <vector>

std::string resclass_misc = "Miscellaneous Resources";
std::string resclass_sound = "Audio";
std::string resclass_texture = "Textures, TGA";
std::string resclass_gamedata = "Game Data";
std::string resclass_texture_info = "Texture Information";
std::string resclass_config = "Configuration";
std::string resclass_image = "Textures, BMP";
std::string resclass_model = "Models";
std::string resclass_ui = "Game User Interface";
std::string resclass_script = "Scripts";
std::string resclass_cscript = "Scripts, Compiled";
std::string resclass_dialog = "Dialogs";
std::string resclass_blueprint_palette = "Blueprint Palettes";
std::string resclass_bp_waypoint = "Blueprint, Waypoints";
std::string resclass_bp_trigger = "Blueprint, Triggers";
std::string resclass_bp_sound = "Blueprint, Sounds";
std::string resclass_bp_placeable = "Blueprint, Placeables";
std::string resclass_bp_item = "Blueprint, Items";
std::string resclass_bp_encounter = "Blueprint, Encounters";
std::string resclass_bp_door = "Blueprint, Doors";
std::string resclass_bp_creature = "Blueprint, Creatures";
std::string resclass_bp_merchant = "Blueprint, Merchants";
std::string resclass_sound_set = "Sound Sets";
std::string resclass_textures_plt = "Textures, PLT";
std::string resclass_model_walk_door = "Models, Walkmesh, Door";
std::string resclass_model_walk_place = "Models, Walkmesh. Placeable";
std::string resclass_model_walk_tile = "Models, Walkmesh. Tile";
std::string resclass_area = "Area Data";
std::string resclass_module = "Module Data";
std::string resclass_plot = "Plot Data";
std::string resclass_texture_dds = "Textures, DDS";
std::string resclass_dscript = "Scripts, Debug Info";
std::string resclass_model_pheno = "Models, Pheno";
std::string resclass_texture_minimap = "Images, Minimap";
std::string resclass_texture_portrait = "Images, Portrait";
std::string resclass_texture_iconinv = "Images, Icon, Inventory";
std::string resclass_texture_iconfeat = "Images, Icon, Feat";
std::string resclass_texture_iconspell = "Images, Icon, Spell";
std::string resclass_texture_iconskill = "Images, Icon, Skill";
std::string resclass_texture_iconother = "Images, Icon, Other";
std::string resclass_texture_iconscroll = "Images, Icon, Scroll";
std::string resclass_shader = "Shader";
std::string resclass_material = "Materials";

// resclass_misc

struct ResclassPayload {
    nw::ResourceType::type restype;
    std::string_view resclass;
    std::string regex;
};

std::vector<ResclassPayload> resclasses = {
    // Sounds
    {nw::ResourceType::bmu, resclass_sound, ""},
    {nw::ResourceType::wav, resclass_sound, ""},

    // Images
    {nw::ResourceType::bmp, resclass_image, ""},
    {nw::ResourceType::gif, resclass_image, ""},
    {nw::ResourceType::png, resclass_image, ""},

    {nw::ResourceType::dds, resclass_texture_portrait, "po_.*"},
    {nw::ResourceType::tga, resclass_texture_portrait, "po_.*"},

    {nw::ResourceType::plt, resclass_textures_plt, ""},
    {nw::ResourceType::dds, resclass_texture_dds, ""},
    {nw::ResourceType::tga, resclass_texture, ""},

    {nw::ResourceType::mtr, resclass_material, ""},
    {nw::ResourceType::txi, resclass_texture_info, ""},

    // Config
    {nw::ResourceType::ini, resclass_config, ""},
    {nw::ResourceType::twoda, resclass_config, ""},
    {nw::ResourceType::tml, resclass_config, ""},

    {nw::ResourceType::mdl, resclass_model, ""},

    {nw::ResourceType::gui, resclass_ui, ""},

    // Scripts
    {nw::ResourceType::nss, resclass_script, ""},
    {nw::ResourceType::ncs, resclass_cscript, ""},
    {nw::ResourceType::ndb, resclass_dscript, ""},

    // Areas
    {nw::ResourceType::caf, resclass_area, ""},
    {nw::ResourceType::are, resclass_area, ""},
    {nw::ResourceType::git, resclass_area, ""},
    {nw::ResourceType::gic, resclass_area, ""},

    // Blueprints
    {nw::ResourceType::dlg, resclass_dialog, ""},
    {nw::ResourceType::itp, resclass_blueprint_palette, ""},
    {nw::ResourceType::utw, resclass_bp_waypoint, ""},
    {nw::ResourceType::utt, resclass_bp_trigger, ""},
    {nw::ResourceType::uts, resclass_bp_sound, ""},
    {nw::ResourceType::utp, resclass_bp_placeable, ""},
    {nw::ResourceType::uti, resclass_bp_item, ""},
    {nw::ResourceType::ute, resclass_bp_encounter, ""},
    {nw::ResourceType::utd, resclass_bp_door, ""},
    {nw::ResourceType::utc, resclass_bp_creature, ""},
    {nw::ResourceType::utm, resclass_bp_merchant, ""},

    // Module
    {nw::ResourceType::fac, resclass_module, ""},
    {nw::ResourceType::ifo, resclass_module, ""},

    // Shaders
    {nw::ResourceType::shd, resclass_shader, ""},

    // Plot
    {nw::ResourceType::ptm, resclass_plot, ""},
    {nw::ResourceType::ptt, resclass_plot, ""},

    // Sound Sets
    {nw::ResourceType::ssf, resclass_sound_set, ""},

    // Walkmesh
    {nw::ResourceType::dwk, resclass_model_walk_door, ""},
    {nw::ResourceType::invalid, resclass_model_walk_place, ""},
    {nw::ResourceType::invalid, resclass_model_walk_tile, ""},
};

// == ExplorerItem ============================================================
// ============================================================================

ExplorerItem::ExplorerItem(QString name, AbstractTreeItem* parent)
    : AbstractTreeItem(0, parent)
    , name_{std::move(name)}
    , kind_{ExplorerItemKind::category}
{
}

ExplorerItem::ExplorerItem(nw::Container* container, AbstractTreeItem* parent)
    : AbstractTreeItem(0, parent)
    , name_{to_qstring(container->name())}
    , container_{container}
    , kind_{ExplorerItemKind::container}
{
    absl::flat_hash_map<std::string_view, ExplorerItem*> map{
        {resclass_misc, new ExplorerItem(to_qstring(resclass_misc), this)},
        {resclass_sound, new ExplorerItem(to_qstring(resclass_sound), this)},
        {resclass_texture, new ExplorerItem(to_qstring(resclass_texture), this)},
        {resclass_gamedata, new ExplorerItem(to_qstring(resclass_gamedata), this)},
        {resclass_texture_info, new ExplorerItem(to_qstring(resclass_texture_info), this)},
        {resclass_config, new ExplorerItem(to_qstring(resclass_config), this)},
        {resclass_image, new ExplorerItem(to_qstring(resclass_image), this)},
        {resclass_model, new ExplorerItem(to_qstring(resclass_model), this)},
        {resclass_ui, new ExplorerItem(to_qstring(resclass_ui), this)},
        {resclass_script, new ExplorerItem(to_qstring(resclass_script), this)},
        {resclass_cscript, new ExplorerItem(to_qstring(resclass_cscript), this)},
        {resclass_dialog, new ExplorerItem(to_qstring(resclass_dialog), this)},
        {resclass_blueprint_palette, new ExplorerItem(to_qstring(resclass_blueprint_palette), this)},
        {resclass_bp_waypoint, new ExplorerItem(to_qstring(resclass_bp_waypoint), this)},
        {resclass_bp_trigger, new ExplorerItem(to_qstring(resclass_bp_trigger), this)},
        {resclass_bp_sound, new ExplorerItem(to_qstring(resclass_bp_sound), this)},
        {resclass_bp_placeable, new ExplorerItem(to_qstring(resclass_bp_placeable), this)},
        {resclass_bp_item, new ExplorerItem(to_qstring(resclass_bp_item), this)},
        {resclass_bp_encounter, new ExplorerItem(to_qstring(resclass_bp_encounter), this)},
        {resclass_bp_door, new ExplorerItem(to_qstring(resclass_bp_door), this)},
        {resclass_bp_creature, new ExplorerItem(to_qstring(resclass_bp_creature), this)},
        {resclass_bp_merchant, new ExplorerItem(to_qstring(resclass_bp_merchant), this)},
        {resclass_sound_set, new ExplorerItem(to_qstring(resclass_sound_set), this)},
        {resclass_textures_plt, new ExplorerItem(to_qstring(resclass_textures_plt), this)},
        {resclass_model_walk_door, new ExplorerItem(to_qstring(resclass_model_walk_door), this)},
        {resclass_model_walk_place, new ExplorerItem(to_qstring(resclass_model_walk_place), this)},
        {resclass_model_walk_tile, new ExplorerItem(to_qstring(resclass_model_walk_tile), this)},
        {resclass_area, new ExplorerItem(to_qstring(resclass_area), this)},
        {resclass_module, new ExplorerItem(to_qstring(resclass_module), this)},
        {resclass_plot, new ExplorerItem(to_qstring(resclass_plot), this)},
        {resclass_texture_dds, new ExplorerItem(to_qstring(resclass_texture_dds), this)},
        {resclass_dscript, new ExplorerItem(to_qstring(resclass_dscript), this)},
        {resclass_model_pheno, new ExplorerItem(to_qstring(resclass_model_pheno), this)},
        {resclass_texture_minimap, new ExplorerItem(to_qstring(resclass_texture_minimap), this)},
        {resclass_texture_portrait, new ExplorerItem(to_qstring(resclass_texture_portrait), this)},
        {resclass_texture_iconinv, new ExplorerItem(to_qstring(resclass_texture_iconinv), this)},
        {resclass_texture_iconfeat, new ExplorerItem(to_qstring(resclass_texture_iconfeat), this)},
        {resclass_texture_iconspell, new ExplorerItem(to_qstring(resclass_texture_iconspell), this)},
        {resclass_texture_iconskill, new ExplorerItem(to_qstring(resclass_texture_iconskill), this)},
        {resclass_texture_iconother, new ExplorerItem(to_qstring(resclass_texture_iconother), this)},
        {resclass_texture_iconscroll, new ExplorerItem(to_qstring(resclass_texture_iconscroll), this)},
        {resclass_shader, new ExplorerItem(to_qstring(resclass_shader), this)},
        {resclass_material, new ExplorerItem(to_qstring(resclass_material), this)},
    };

    for (const auto& rd : container_->all()) {
        std::string_view rc;
        for (const auto& thing : resclasses) {
            if (rd.name.type == thing.restype) {
                if (thing.regex.empty()) {
                    rc = thing.resclass;
                    break;
                } else if (std::regex_match(rd.name.resref.string(), std::regex(thing.regex))) {
                    rc = thing.resclass;
                    break;
                }
            }
        }

        if (rc.size()) {
            map[rc]->appendChild(new ExplorerItem(rd, this));
        } else {
            map[resclass_misc]->appendChild(new ExplorerItem(rd, this));
        }
    }

    for (const auto& [_, v] : map) {
        appendChild(v);
    }
}

ExplorerItem::ExplorerItem(nw::ResourceDescriptor res, AbstractTreeItem* parent)
    : AbstractTreeItem(0, parent)
    , name_{to_qstring(res.name.filename())}
    , descriptor_{res}
    , kind_{ExplorerItemKind::resource}
{
}

QVariant ExplorerItem::data(int column, int role) const
{
    Q_UNUSED(column);
    if (role == Qt::DisplayRole) {
        return name_;
    }

    return {};
}

// == ExplorerModel ===========================================================
// ============================================================================

ExplorerModel::ExplorerModel(QObject* parent)
    : AbstractTreeModel{parent}
{
}

int ExplorerModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant ExplorerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return {}; }
    auto it = static_cast<ExplorerItem*>(index.internalPointer());
    if (!it) { return {}; }
    return it->data(index.column(), role);
}

void ExplorerModel::loadRootItems()
{
    auto haks = new ExplorerItem("Haks");
    addRootItem(haks);

    for (auto hak : nw::kernel::resman().module_haks()) {
        auto h = new ExplorerItem(hak, haks);
        haks->appendChild(h);
    }
}

// == ExplorerProxy ===========================================================
// ============================================================================

ExplorerProxy::ExplorerProxy(QObject* parent)
    : FuzzyProxyModel(parent)
{
}

bool ExplorerProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    auto it = static_cast<ExplorerItem*>(index.internalPointer());
    if (it->kind_ == ExplorerItemKind::category && it->childCount() == 0) { return false; }

    if (filter_.isEmpty()) { return true; }
    auto data = index.data(Qt::DisplayRole);
    return has_match(filter_.toStdString().c_str(), data.toString().toStdString().c_str());
}

bool ExplorerProxy::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    auto lhs = static_cast<ExplorerItem*>(source_left.internalPointer());
    auto rhs = static_cast<ExplorerItem*>(source_right.internalPointer());

    // Don't sort containers.. they should be in the order added.
    if (lhs->kind_ == ExplorerItemKind::container && rhs->kind_ == ExplorerItemKind::container) {
        return false;
    }

    // The design of the source model won't allow categories and resources to be at the same treelevel
    return lhs->name_ < rhs->name_;
}

// == ExplorerView ============================================================
// ============================================================================

ExplorerView::ExplorerView(QWidget* parent)
    : ArclightTreeView(parent)
{
    setHeaderHidden(true);
    setSortingEnabled(false);
}

ExplorerView::~ExplorerView()
{
    delete model_;
}

void ExplorerView::activateModel()
{
    proxy_ = new ExplorerProxy(this);
    proxy_->setRecursiveFilteringEnabled(true);
    proxy_->setSourceModel(model_);
    setModel(proxy_);
    model()->sort(0);
    expandRecursively(model()->index(0, 0), 0);
}

AbstractTreeModel* ExplorerView::loadModel()
{
    model_ = new ExplorerModel();
    model_->loadRootItems();
    return model_;
}
