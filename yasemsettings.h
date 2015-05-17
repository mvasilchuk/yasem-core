#ifndef CONFIGITEM
#define CONFIGITEM

#include "macros.h"

#include <QString>
#include <QVariant>
#include <QList>
#include <QHash>

namespace yasem {

class ConfigTreeGroup;
class YasemSettings;
class YasemSettingsImpl;
class Core;
class CoreImpl;

class ConfigItem: public QObject {
    Q_OBJECT
public:
    enum ItemType {
        CONTAINER, // Internal use only!
        STRING,
        BOOL,
        INT,
        LONG,
        DOUBLE,
        LIST,
        GROUPBOX
    };

    ConfigItem(const QString &key, const QString &title, const QVariant &value = "", ItemType type = STRING):
        QObject(),
        m_key(key),
        m_title(title),
        m_value(value),
        m_default_value(value),
        m_type(type),
        m_parent_item(NULL),
        m_is_dirty(false)
    {

    }

    virtual ~ConfigItem() {}

    ItemType getType() { return m_type; }
    QVariant getValue() { return m_value; }
    QString getTitle() { return m_title; }
    QString getKey() const { return m_key; }
    ConfigItem* getParentItem() { return m_parent_item; }

    QList<ConfigItem*> getItems() const { return m_items; }

    void addItem(ConfigItem* item)
    {
        item->m_parent_item = this;
        m_items.append(item);
    }

    ConfigItem* item(const QString &key) const
    {
        for(ConfigItem* item: m_items)
        {
            if(item->getKey() == key) return item;
        }
        return NULL;
    }

    void setDirty(bool dirty = true)
    {
        m_is_dirty = dirty;
    }

    bool isDirty() const
    {
        return m_is_dirty;
    }

    QVariant value() const { return m_value; }
    void setValue(const QVariant& value) {
        if(!m_is_dirty)
            m_default_value = m_value;
        m_value = value;
    }
    QVariant getDefaultValue() const
    {
        return m_value;
    }

    void reset()
    {
        this->setDirty(false);
        this->setValue(getDefaultValue());
        emit reseted();
        for(ConfigItem* item: m_items)
        {
            item->reset();
        }
    }

    bool isContainer() { return m_type == CONTAINER; }

    ConfigItem* findItemByKey(const QString& key)
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

    ConfigItem* findItemByPath(const QString &path)
    {
        return findItemByPath(path.split("/"));
    }

    ConfigItem* findItemByPath(const QStringList &path)
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
signals:
    void reseted();
    void saved();

protected:
    QString m_key;
    QString m_title;
    QVariant m_value;
    QVariant m_default_value;
    ItemType m_type;
    ConfigItem* m_parent_item;
    bool m_is_dirty;
    QList<ConfigItem*> m_items;

friend class YasemSettings;
friend class YasemSettingsImpl;
};

class ListConfigItem: public ConfigItem
{
    Q_OBJECT
public:
    ListConfigItem(const QString &key, const QString &title, const QVariant &value = ""):
        ConfigItem(key, title, value, ConfigItem::LIST)
    {

    }

    QMap<QString, QVariant>& options() { return m_options; }
    void setOptions(QMap<QString, QVariant> options)
    {
        this->m_options = options;
    }

protected:
    QMap<QString, QVariant> m_options;
};

class ConfigContainer: public ConfigItem
{
    Q_OBJECT
public:
    enum ContainerType {
        ROOT,
        CONFIG_GROUP,
        PAGE
    };


    ConfigContainer(const QString &config_file, const QString& id, const QString &title, ContainerType type):
        ConfigItem(id, title, QVariant(), CONTAINER),
        m_container_type(type),
        m_config_file(config_file),
        m_is_built_in(false)
    {
    }
    virtual ~ConfigContainer() {}

    ContainerType getContainerType() { return m_container_type; }
    QString getConfigFile() const {
        QString config = "";
        if(!m_config_file.isEmpty())
            config = m_config_file;
        else if(m_parent_item != NULL)
        {
            ConfigContainer* parent = dynamic_cast<ConfigContainer*>(m_parent_item);
            Q_ASSERT(parent != NULL);
            config = parent->getConfigFile();
        }
        Q_ASSERT_X(!config.isEmpty(), "YasemSettings", "Configuration item should belongs to a configuration file!");
        return config;
    }

protected:
    ContainerType m_container_type;
    QString m_config_file;
    bool m_is_built_in;

    friend class YasemSettings;
    friend class YasemSettingsImpl;
};

class ConfigTreeGroup: public ConfigContainer {
    Q_OBJECT
public:

    ConfigTreeGroup(const QString &config_file, const QString& id, const QString &title):
        ConfigContainer(config_file, id, title, CONFIG_GROUP)
    {

    }
    ConfigTreeGroup(const QString& id, const QString &title):
        ConfigTreeGroup("", id, title)
    {

    }
    virtual ~ConfigTreeGroup() {}

    bool isBuiltInGroup() { return m_is_built_in; }

protected:


    friend class YasemSettings;
    friend class YasemSettingsImpl;
};

class YasemSettings: public QObject
{
    Q_OBJECT

public:
    static QString SETTINGS_GROUP_APPEARANCE;
    static QString SETTINGS_GROUP_MEDIA;
    static QString SETTINGS_GROUP_OTHER;

    YasemSettings(QObject* parent): QObject(parent) {};
    virtual ~YasemSettings() {}

    virtual bool addConfigGroup(ConfigTreeGroup* group) = 0;

    virtual QHash<const QString&, ConfigTreeGroup*> getConfigGroups() = 0;
    virtual ConfigTreeGroup* getDefaultGroup(const QString &id) = 0;

public slots:
    virtual void save(ConfigContainer* container = 0) = 0;
    virtual void reset(ConfigContainer* container = 0) = 0;
    virtual void load(ConfigContainer* container = 0) = 0;
    virtual void setItemDirty(ConfigItem* item, bool value = true) = 0;
    virtual ConfigItem* findItem(const QStringList& path) = 0;
    virtual ConfigItem* findItem(const QString& path) = 0;
protected:
    QHash<const QString&, ConfigTreeGroup*> m_config_groups;
    virtual bool addBuiltInConfigGroup(ConfigTreeGroup* group) = 0;

    friend class Core;
    friend class CoreImpl;
};

}

#endif // CONFIGITEM

