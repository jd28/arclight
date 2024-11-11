#include "strings.h"

#include <QString>

QString to_qstring(std::string_view view)
{
    return QString::fromUtf8(view.data(), static_cast<qsizetype>(view.size()));
}
