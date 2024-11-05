#pragma once

namespace nw {
struct Item;
struct ObjectHandle;
}

class QByteArray;
class QImage;

/// Deserializes an ObjectHandle from a QByteArray
nw::ObjectHandle deserialize_obj_handle(const QByteArray& data);

/// Serializes an ObjectHandle to a QByteArray
QByteArray serialize_obj_handle(nw::ObjectHandle hndl);

/// Creates an item icon
QImage item_to_image(const nw::Item* item, bool female);
