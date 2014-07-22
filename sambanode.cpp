#include "sambanode.h"
#include "macros.h"

using namespace yasem;

SambaNode::SambaNode(QObject *parent) :
    QObject(parent)
{

    SambaNode* pNode = qobject_cast<SambaNode*>(parent);
    if(pNode != NULL)
    {
        pNode->addChild(this);
    }
}

void SambaNode::addChild(SambaNode *child)
{
    children.append(child);
}

void SambaNode::print()
{
    DEBUG() << "Node:" << name << description << type << ", parent:" << qobject_cast<SambaNode*>(parent())->name;
    for(SambaNode* child: children)
    {
        child->print();
    }
}


yasem::SambaNode::~SambaNode()
{
    DEBUG() << "SambaNode" << name << "deleted";
}
