#ifndef PLUGINDEPENDENCY_H
#define PLUGINDEPENDENCY_H

#include "enums.h"

#include <QObject>

namespace yasem
{

class PluginDependency {
public:
    QString dependency_name; // A name of registered plugin dependency
    PluginRole role;         // A role of a dependency.
                             // You should declare at least one of dependency_name or role (or both).
    bool required;           // Set this flag if plugin cannot be loaded without this dependency.
    PluginDependency(QString name, bool required = true) {
        this->dependency_name = name;
        this->required = required;
    }

    PluginDependency(PluginRole role, bool required = true) {
        this->role = role;
        this->required = required;
    }

    PluginDependency(QString name, PluginRole role, bool required = true) {
        this->dependency_name = name;
        this->role = role;
        this->required = required;
    }

};

}

#endif // PLUGINDEPENDENCY_H
