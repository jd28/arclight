#include "propertiesview.h"

#include "qtpropertybrowser/qteditorfactory.h"
#include "qtpropertybrowser/qtpropertybrowser.h"
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "util/strings.h"

#include "nw/resources/Resref.hpp"

PropertiesView::PropertiesView(QWidget* parent)
    : QWidget{parent}
{
    editor_ = new QtTreePropertyBrowser(this);
    groups_ = new QtGroupPropertyManager(this);
    ints_ = new QtIntPropertyManager(this);
    editor_->setFactoryForManager(ints_, new QtSpinBoxFactory(this));
    strings_ = new QtStringPropertyManager(this);
    editor_->setFactoryForManager(strings_, new QtLineEditFactory(this));
    bools_ = new QtBoolPropertyManager(this);
    editor_->setFactoryForManager(bools_, new QtCheckBoxFactory(this));
    enums_ = new QtEnumPropertyManager(this);
    editor_->setFactoryForManager(enums_, new QtEnumEditorFactory(this));
}

QtProperty* PropertiesView::addGroup(QString name)
{
    auto prop = groups_->addProperty(std::move(name));
    return prop;
}

QtProperty* PropertiesView::addPropertyBool(QString name, bool value)
{
    auto prop = bools_->addProperty(name);
    bools_->setValue(prop, value);
    return prop;
}

QtProperty* PropertiesView::addPropertyInt(QString name, int value, std::optional<int> min, std::optional<int> max)
{
    auto prop = ints_->addProperty(std::move(name));
    ints_->setValue(prop, value);
    if (min) { ints_->setMinimum(prop, *min); }
    if (max) { ints_->setMaximum(prop, *max); }
    return prop;
}

QtProperty* PropertiesView::addPropertyEnum(QString name, int value, QStringList enums, QList<QVariant> data)
{
    auto prop = enums_->addProperty(std::move(name));
    enums_->setValue(prop, value);
    enums_->setEnumNames(prop, std::move(enums));
    enums_->setEnumData(prop, std::move(data));
    return prop;
}

QtProperty* PropertiesView::addPropertyString(QString name, const QString& value, QRegularExpression regex, QCompleter* completer)
{
    auto prop = strings_->addProperty(std::move(name));
    strings_->setValue(prop, value);
    if (regex.isValid()) {
        strings_->setRegExp(prop, regex);
    }
    if (completer) {
        strings_->setCompleter(prop, completer);
    }
    return prop;
}

QtProperty* PropertiesView::addPropertyString(QString name, const std::string& value, QRegularExpression regex, QCompleter* completer)
{
    return addPropertyString(std::move(name), to_qstring(value), std::move(regex), completer);
}

QtProperty* PropertiesView::addPropertyString(QString name, const nw::Resref& value, QRegularExpression regex, QCompleter* completer)
{
    return addPropertyString(std::move(name), to_qstring(value.string()), std::move(regex), completer);
}

QtBoolPropertyManager* PropertiesView::bools() const
{
    return bools_;
}

QtAbstractPropertyBrowser* PropertiesView::editor() const
{
    return editor_;
}

QtEnumPropertyManager* PropertiesView::enums() const
{
    return enums_;
}

QtGroupPropertyManager* PropertiesView::groups() const
{
    return groups_;
}

QtIntPropertyManager* PropertiesView::ints() const
{
    return ints_;
}

QtStringPropertyManager* PropertiesView::strings() const
{
    return strings_;
}
