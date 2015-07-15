#include "configimpl.h"
#include "macros.h"

using namespace yasem;

ConfigImpl::ConfigImpl(QObject *parent) :
    SDK::Config(parent)
{

}

bool ConfigImpl::addConfigGroup(SDK::ConfigTreeGroup *group)
{
    if(!m_config_groups.contains(group->m_key))
        m_config_groups.insert(group->m_key, group);
    return true;
}

QHash<const QString&, SDK::ConfigTreeGroup *> ConfigImpl::getConfigGroups()
{
    return m_config_groups;
}

void ConfigImpl::setItemDirty(SDK::ConfigItem* item, bool value)
{
    if(item == NULL) return;
    item->m_is_dirty = value;
    setItemDirty(item->getParentItem(), value);
}

bool ConfigImpl::addBuiltInConfigGroup(SDK::ConfigTreeGroup *group)
{
    group->m_is_built_in = true;
    return addConfigGroup(group);
}


SDK::ConfigTreeGroup *ConfigImpl::getDefaultGroup(const QString &id)
{
    if(m_config_groups.contains(id))
        return m_config_groups.value(id);

    WARN() << "Default group" << id << "not found!";
    return NULL;

}

void ConfigImpl::save(SDK::ConfigContainer *container)
{    
    if(container == NULL)
    {
        DEBUG() << "Saving entire config tree...";
        for(SDK::ConfigTreeGroup* gr: m_config_groups)
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
        QSettings* settings = SDK::Core::instance()->settings(config_file);
        settings->beginGroup(container->getKey());

        for(SDK::ConfigItem* item: container->getItems())
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
    for(SDK::ConfigItem* item: container->getItems())
    {
        if(item->isContainer())
        {
            SDK::ConfigContainer* subcontainer = static_cast<SDK::ConfigContainer*>(item);
            Q_ASSERT(subcontainer != NULL);
            save(subcontainer);
        }
    }
}

void ConfigImpl::reset(SDK::ConfigContainer *container)
{
    if(container == NULL)
    {
        DEBUG() << "Reseting entire config tree...";
        for(SDK::ConfigTreeGroup* gr: m_config_groups)
            reset(gr);
        return;
    }

    container->reset();
}


void ConfigImpl::load(SDK::ConfigContainer *container)
{
    if(container == NULL)
    {
        DEBUG() << "Loading entire config tree...";
        for(SDK::ConfigTreeGroup* gr: m_config_groups)
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

    QSettings* settings = SDK::Core::instance()->settings(config_file);
    settings->beginGroup(container->getKey());

    for(SDK::ConfigItem* item: container->getItems())
    {
        item->setDirty(false);
        if(!item->isContainer())
        {
            QVariant val = settings->value(item->getKey());
            DEBUG() << "....loading item " << item->getKey() << ", value " << val;
            if(val.isNull())
                item->setValue(item->getDefaultValue());
            else
                item->setValue(val);
        }
        else
        {
            DEBUG() << ".... Loading settings container " << item->getKey();
            load(dynamic_cast<SDK::ConfigContainer*>(item));
        }
    }

    settings->endGroup();
}

SDK::ConfigItem *ConfigImpl::findItem(const QString &path)
{
    return findItem(path.split("/"));
}

SDK::ConfigItem *ConfigImpl::findItem(const QStringList &path)
{
    SDK::ConfigItem* result = NULL;
    if(!path.isEmpty())
    {
        DEBUG() << "Looking in root";
        if(path.length() >= 2)
        {
            int first_index = 0;
            if(path.at(0).isEmpty())
                first_index = 1;

            for(SDK::ConfigItem* item: m_config_groups)
            {
                if(item->getKey() == path.at(first_index))
                {
                    result = item->findItemByPath(path.mid(first_index+1));
                    break;
                }
            }
        }
        else
        {
            for(SDK::ConfigItem* item: m_config_groups)
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
