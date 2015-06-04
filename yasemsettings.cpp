#include "yasemsettings.h"

using namespace yasem;

QString YasemSettings::SETTINGS_GROUP_APPEARANCE = "appearance";
QString YasemSettings::SETTINGS_GROUP_MEDIA = "media";
QString YasemSettings::SETTINGS_GROUP_PLUGINS = "plugins";
QString YasemSettings::SETTINGS_GROUP_OTHER = "other";


ConfigTreeGroup::ConfigTreeGroup(const QString &config_file, const QString &id, const QString &title):
    ConfigContainer(config_file, id, title, CONFIG_GROUP)
{

}

ConfigTreeGroup::ConfigTreeGroup(const QString &id, const QString &title):
    ConfigTreeGroup("", id, title)
{

}

ConfigTreeGroup::~ConfigTreeGroup() {}

bool ConfigTreeGroup::isBuiltInGroup() { return m_is_built_in; }

ConfigItem::ConfigItem(const QString &key, const QString &title, const QVariant &value, ConfigItem::ItemType type):
    QObject(),
    m_key(key),
    m_title(title),
    m_value(value),
    m_default_value(value),
    m_type(type),
    m_is_dirty(false)
{

}

ConfigItem::~ConfigItem() {}

ConfigItem::ItemType ConfigItem::getType() { return m_type; }

QVariant ConfigItem::getValue() { return m_value; }

QString ConfigItem::getTitle() { return m_title; }

QString ConfigItem::getKey() const { return m_key; }

ConfigItem *ConfigItem::getParentItem() const { return dynamic_cast<ConfigItem*>(parent()); }

QList<ConfigItem *> ConfigItem::getItems() const { return m_items; }

void ConfigItem::addItem(ConfigItem *item)
{
    item->setParent(this);
    m_items.append(item);
}

ConfigItem *ConfigItem::item(const QString &key) const
{
    for(ConfigItem* item: m_items)
    {
        if(item->getKey() == key) return item;
    }
    return NULL;
}

void ConfigItem::setDirty(bool dirty)
{
    m_is_dirty = dirty;
}

bool ConfigItem::isDirty() const
{
    return m_is_dirty;
}

QVariant ConfigItem::value() const { return m_value; }

void ConfigItem::setValue(const QVariant &value) {
    if(!m_is_dirty)
        m_default_value = m_value;
    m_value = value;
}

QVariant ConfigItem::getDefaultValue() const
{
    return m_value;
}

void ConfigItem::reset()
{
    this->setDirty(false);
    this->setValue(getDefaultValue());
    emit reseted();
    for(ConfigItem* item: m_items)
    {
        item->reset();
    }
}

bool ConfigItem::isContainer() { return m_type == CONTAINER; }

ConfigItem *ConfigItem::findItemByKey(const QString &key)
{
    ConfigItem* result = NULL;
    for(ConfigItem* item: m_items)
    {
        if(item->getKey() == key)
            result = item;
    }
    Q_ASSERT(result != NULL);
    return result;
}

ConfigItem *ConfigItem::findItemByPath(const QString &path)
{
    return findItemByPath(path.split("/"));
}

ConfigItem *ConfigItem::findItemByPath(const QStringList &path)
{
    DEBUG() << "Looking in " << getKey() << path;
    if(path.isEmpty()) return NULL;
    for(ConfigItem* item: m_items)
    {
        if(item->getKey() == path.at(0))
        {
            if(path.length() == 1)
                return item;
            return item->findItemByPath(path.mid(1));
        }
    }
    return NULL;
}

ListConfigItem::ListConfigItem(const QString &key, const QString &title, const QVariant &value):
    ConfigItem(key, title, value, ConfigItem::LIST)
{

}

QMap<QString, QVariant> &ListConfigItem::options() { return m_options; }

void ListConfigItem::setOptions(QMap<QString, QVariant> options)
{
    this->m_options = options;
}


ConfigContainer::ConfigContainer(const QString &config_file, const QString &id, const QString &title, ConfigContainer::ContainerType type):
    ConfigItem(id, title, QVariant(), CONTAINER),
    m_container_type(type),
    m_config_file(config_file),
    m_is_built_in(false)
{
}

ConfigContainer::~ConfigContainer() {}

ConfigContainer::ContainerType ConfigContainer::getContainerType() { return m_container_type; }

QString ConfigContainer::getConfigFile() const {
    QString config = "";
    if(!m_config_file.isEmpty())
        config = m_config_file;
    else
    {
        ConfigContainer* cont = dynamic_cast<ConfigContainer*>(parent());
        Q_ASSERT(cont);
        config = cont->getConfigFile();
    }
    Q_ASSERT_X(!config.isEmpty(), "YasemSettings", "Configuration item should belongs to a configuration file!");
    return config;
}
