#include "sambaimpl.h"
#include "macros.h"
#include "sambanode.h"

#include <QProcess>
#include <QStringList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QFileInfo>

using namespace yasem;

SambaImpl::SambaImpl(QObject* parent) : Samba(parent)
{
    sambaTree = new SDK::SambaNode(this);
    sambaTree->type = SDK::SAMBA_ROOT;
    sambaTree->name = "ROOT";
}

SambaImpl::~SambaImpl()
{
    STUB();
    for(MountPointInfo* m_point: mountPoints)
        m_point->unmount(true);

}

QList<SDK::SambaNode*> SambaImpl::domains()
{
    if(sambaTree->children.size() == 0)
        makeTree();

    return sambaTree->children;
}

QList<SDK::SambaNode*> yasem::SambaImpl::hosts(const QString &domainName)
{
    if(sambaTree->children.size() == 0)
        makeTree();

    for(SDK::SambaNode* domain: sambaTree->children)
    {
        if(domain->name == domainName)
        {
            return domain->children;
        }
    }
    return QList<SDK::SambaNode*>();
}

QList<SDK::SambaNode*> SambaImpl::shares(const QString &hostName)
{
    if(sambaTree->children.size() == 0)
        makeTree();

    for(SDK::SambaNode* domain: sambaTree->children)
    {
        for(SDK::SambaNode* host: domain->children)
        {
            if(host->name == hostName)
            {
                return host->children;
            }
        }
    }
    return QList<SDK::SambaNode*>();
}

void SambaImpl::makeTree()
{
    DEBUG() << "-----  TREE START ------";
    #ifdef Q_OS_UNIX
    bool use_broadcasting = SDK::Core::instance()->settings()->value("samba/use_broadcasting", false).toBool();

    QProcess smbtree;
    QStringList params;
    params << "-N";
    if(use_broadcasting)
        params << "-b";

    smbtree.start("smbtree", params);
    if (!smbtree.waitForStarted())
    {
        ERROR() << "smbtree not started!";
    }

    smbtree.waitForFinished();

    QRegularExpression nodeRegex("^(?:\\t){0,2}(?:\\\\){0,2}(?:\\t){0,2}(?<host>[\\w\\d-\\\\\\$]*)(?:\\s+(?<description>.*?))$");
    nodeRegex.setPatternOptions(QRegularExpression::MultilineOption);

    SDK::SambaNode* last_node = sambaTree;

    while(smbtree.canReadLine())
    {
        SDK::SambaNodeType line_type = SDK::SAMBA_UNDEFINED;

        QString line = smbtree.readLine();

        if(line.startsWith("\t\t"))
            line_type = SDK::SAMBA_SHARE;
        else if(line.startsWith("\t"))
            line_type = SDK::SAMBA_HOST;
        else
            line_type = SDK::SAMBA_DOMAIN;

        SDK::SambaNode* parent = NULL;
        switch(line_type)
        {
            case SDK::SAMBA_SHARE:
            {
                switch(last_node->type)
                {
                    case SDK::SAMBA_SHARE:   parent = qobject_cast<SDK::SambaNode*>(last_node->parent());                 break;
                    case SDK::SAMBA_HOST:    parent = last_node;                                                     break;
                    default:            WARN() << "Broken share node" << line << line_type << last_node->name;  break;
                }
                break;
            }
            case SDK::SAMBA_HOST:{
                switch(last_node->type)
                {
                    case SDK::SAMBA_SHARE:   parent = qobject_cast<SDK::SambaNode*>(last_node->parent()->parent());       break;
                    case SDK::SAMBA_HOST:    parent = qobject_cast<SDK::SambaNode*>(last_node->parent());                 break;
                    case SDK::SAMBA_DOMAIN:  parent = last_node;                                                     break;
                    default:            WARN() << "Broken host node" << line << last_node->name;                break;
                }
                break;
            }
            case SDK::SAMBA_DOMAIN: {
                parent = sambaTree;
            }
            default: break;
        }

        if(parent == NULL)
        {
            WARN() << "Parent for node" << line << "not found!";
            continue;
        }

        SDK::SambaNode* node = new SDK::SambaNode(parent);
        node->type = line_type;
        QRegularExpressionMatch nodeMatch = nodeRegex.match(line);
        if(nodeMatch.hasMatch())
        {
            node->name = nodeMatch.captured(1);

            if(line_type == SDK::SAMBA_SHARE)
            {
                int len_to_shrink = dynamic_cast<SDK::SambaNode*>(node->parent())->name.length() + 1;
                node->name = node->name.right(node->name.length() - len_to_shrink);
            }
            node->description = nodeMatch.captured(2);
        }
        last_node = node;

    }

    for(SDK::SambaNode* node: sambaTree->children)
        node->print();

    smbtree.close();
    smbtree.kill();
    #else
    ONLY_SUPPORTED_ON_UNIX;
    #endif //Q_OS_UNIX
    DEBUG() << "----- TREE END ----";
}

bool SambaImpl::mount(const QString &what, const QString &where, const QString &options)
{
    MountPointInfo* m_point = new MountPointInfo(this);
    bool mounted = m_point->mount(what, where, options, true);
    if(mounted)
        mountPoints.append(m_point);
    return mounted;
}

bool SambaImpl::unmount(const QString &path)
{
    for(MountPointInfo* m_point: mountPoints)
        if(m_point->host() == path || m_point->mountPoint() == path)
        {
            return m_point->unmount();
        }
    return false;
}
