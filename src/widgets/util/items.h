#pragma once

namespace nw {
struct Item;
}

class QImage;

QImage item_to_image(const nw::Item* item, bool female);
