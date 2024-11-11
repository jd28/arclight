#include "texuregallerymodel.h"

#include "widgets/util/strings.h"

#include "nw/formats/Image.hpp"
#include "nw/formats/Plt.hpp"
#include "nw/log.hpp"
#include "nw/resources/Directory.hpp"
#include "nw/resources/Erf.hpp"
#include "nw/resources/Key.hpp"
#include "nw/resources/Zip.hpp"
#include "nw/util/platform.hpp"

#include <QIcon>
#include <QImage>

inline std::unique_ptr<nw::Container> load_container(const std::filesystem::path& p)
{
    auto ext = nw::path_to_string(p.extension());

    if (std::filesystem::is_directory(p)) {
        return std::make_unique<nw::Directory>(p);
    } else if (nw::string::icmp(ext, ".hak")) {
        return std::make_unique<nw::Erf>(p);
    } else if (nw::string::icmp(ext, ".erf")) {
        return std::make_unique<nw::Erf>(p);
    } else if (nw::string::icmp(ext, ".zip")) {
        return std::make_unique<nw::Zip>(p);
    } else if (nw::string::icmp(ext, ".key")) {
        return std::make_unique<nw::Key>(p);
    }
    return nullptr;
}

TexureGalleryModel::TexureGalleryModel(const QString& path, QObject* parent)
    : QAbstractListModel(parent)
{
    container_ = load_container(path.toStdString());
    if (!container_) { return; }

    auto cb = [this](const nw::Resource& resource) {
        if (nw::ResourceType::check_category(nw::ResourceType::texture, resource.type)) {
            labels_.push_back(resource);
        }
    };

    container_->visit(cb);
    std::sort(std::begin(labels_), std::end(labels_));
}

QVariant TexureGalleryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return {};
}

int TexureGalleryModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return int(labels_.size());
}

int TexureGalleryModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant TexureGalleryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole) {
        auto it = cache_.find(labels_[index.row()]);
        if (it != std::end(cache_)) {
            return it->second;
        } else {
            if (labels_[index.row()].type == nw::ResourceType::plt) {
                nw::Plt plt{container_->demand(labels_[index.row()])};
                if (!plt.valid()) {
                    QVariant();
                }

                std::vector<uint8_t> colors;
                colors.resize(plt.width() * plt.height());
                for (size_t i = 0; i < plt.width() * plt.height(); ++i) {
                    colors[i] = plt.pixels()[i].color;
                }

                // These are pre-flipped
                QImage qi(colors.data(), plt.width(), plt.height(), QImage::Format_Grayscale8);
                qi.mirror();
                auto it2 = cache_.emplace(labels_[index.row()], QPixmap::fromImage(qi));
                return it2.first->second;
            } else {
                nw::Image img{container_->demand(labels_[index.row()])};
                if (!img.valid()) {
                    QVariant();
                }

                QImage qi(img.data(), img.width(), img.height(),
                    img.channels() == 4 ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
                if (qi.height() > 128 || qi.width() > 128) {
                    qi = qi.scaled(128, 128, Qt::KeepAspectRatio);
                }

                // These are pre-flipped
                if (img.is_bio_dds()) {
                    qi.mirror();
                }

                auto it2 = cache_.emplace(labels_[index.row()], QPixmap::fromImage(qi));
                return it2.first->second;
            }
        }
    } else if (role == Qt::SizeHintRole) {
        return QSize(128, 150);
    } else if (role == Qt::DisplayRole) {
        return to_qstring(labels_[index.row()].filename());
    } else if (role == Qt::TextAlignmentRole) {
        return (Qt::AlignBottom | Qt::AlignHCenter).toInt();
    } else if (role == Qt::ToolTipRole) {
        auto it = cache_.find(labels_[index.row()]);
        if (it == std::end(cache_)) {
            return {};
        }
        return to_qstring(fmt::format("{}x{}", it->second.width(), it->second.height()));
    }
    return QVariant();
}
