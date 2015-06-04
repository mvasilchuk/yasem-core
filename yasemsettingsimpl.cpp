#include "yasemsettingsimpl.h"
#include "macros.h"

using namespace yasem;

YasemSettingsImpl::YasemSettingsImpl(QObject *parent) :
    YasemSettings(parent)
{

}

bool yasem::YasemSettingsImpl::addConfigGroup(ConfigTreeGroup *group)
{
    if(!m_config_groups.contains(group->m_key))
        m_config_groups.insert(group->m_key, group);
    return true;
}

QHash<const QString&, ConfigTreeGroup *> yasem::YasemSettingsImpl::getConfigGroups()
{
    return m_config_groups;
}

void yasem::YasemSettingsImpl::setItemDirty(ConfigItem* item, bool value)
{
    if(item == NULL) return;
    item->m_is_dirty = value;
    setItemDirty(item->getParentItem(), value);
}

bool yasem::YasemSettingsImpl::addBuiltInConfigGroup(ConfigTreeGroup *group)
{
    group->m_is_built_in = true;
    return addConfigGroup(group);
}


ConfigTreeGroup *yasem::YasemSettingsImpl::getDefaultGroup(const QString &id)
{
    if(m_config_groups.contains(id))
        return m_config_groups.value(id);

    WARN() << "Default group" << id << "not found!";
    return NULL;

}

void yasem::YasemSettingsImpl::save(ConfigContainer *container)
{    
    if(container == NULL)
    {
        DEBUG() << "Saving entire config tree...";
        for(ConfigTreeGroup* gr: m_config_groups)
            save(gr);
        return;
    }

    DEBUG() << "Saving item" << container->getTitle();

    container->setDirty(false);

    QString config_file = container->getConfigFile();

    //First saving the items we can save (leaves)...
    if(!config_file.isEmpty())
    {
        DEBUG() << "Saving container" << container->getKey() << "to" << config_file << "...";
        QSettings* settings = Core::instance()->settings(config_file);
        settings->beginGroup(container->getKey());

        for(ConfigItem* item: container->getItems())
        {
            if(!item->isContainer() && item->isDirty())
            {
                QVariant value = item->getValue();
                if(value.isNull())
                    value = item->getDefaultValue();

                settings->setValue(item->getKey(), value);
                item->setDirty(false);
                emit item->saved();
            }
        }
        settings->endGroup();
    }
    //else
    //    DEBUG() << "Config tree item" << container->getTitle() << "doesn't have config file";

    //... then tree groups, pages, etc.
    for(ConfigItem* item: container->getItems())
    {
        if(item->isContainer())
        {
            ConfigContainer* subcontainer = static_cast<ConfigContainer*>(item);
            Q_ASSERT(subcontainer != NULL);
            save(subcontainer);
        }
    }
}

void yasem::YasemSettingsImpl::reset(ConfigContainer *container)
{
    if(container == NULL)
    {
        DEBUG() << "Reseting entire config tree...";
        for(ConfigTreeGroup* gr: m_config_groups)
            reset(gr);
        return;
    }

    container->reset();
}


void yasem::YasemSettingsImpl::load(ConfigContainer *container)
{
    if(container == NULL)
    {
        DEBUG() << "Loading entire config tree...";
        for(ConfigTreeGroup* gr: m_config_groups)
            load(gr);
        return;
    }

    DEBUG() << "Loading config tree item" << container->getTitle();

    QString config_file = container->getConfigFile();
    if(config_file.isEmpty())
    {
        DEBUG() << "Config tree item" << container->getTitle() << "doesn't have config file";
        return;
    }

    QSettings* settings = Core::instance()->settings(config_file);
    settings->beginGroup(container->getKey());

    for(ConfigItem* item: container->getItems())
    {
        item->setDirty(false);
        if(!item->isContainer())
        {
            QVariant val = settings->value(item->getKey());
            if(val.isNull())
                item->setValue(item->getDefaultValue());
            else
                item->setValue(val);
        }
    }

    settings->endGroup();
}

ConfigItem *yasem::YasemSettingsImpl::findItem(const QString &path)
{
    return findItem(path.split("/"));
}

ConfigItem *YasemSettingsImpl::findItem(const QStringList &path)
{
    ConfigItem* result = NULL;
    if(!path.isEmpty())
    {
        DEBUG() << "Looking in root";
        if(path.length() >= 2)
        {
            for(ConfigItem* item: m_config_groups)
            {
                if(item->getKey() == path.at(1))
                {
                    result = item->findItemByPath(path.mid(2));
                    break;
                }
            }
        }
        else
        {
            for(ConfigItem* item: m_config_groups)
            {
                if(item->getKey() == path.at(0))
                {
                    result = item;
                    break;
                }
            }
        }
    }

    Q_ASSERT(result != NULL);

    return result;
}
