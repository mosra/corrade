#ifndef Kompas_PluginManager_Test_PluginTest_h
#define Kompas_PluginManager_Test_PluginTest_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Kompas.

    Kompas is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Kompas is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include <QtCore/QObject>

#include "PluginManager/PluginManager.h"
#include "AbstractAnimal.h"
#include "AbstractFood.h"

namespace Kompas { namespace PluginManager { namespace Test {

class PluginTest: public QObject {
    Q_OBJECT

    private:
        PluginManager<AbstractAnimal>* manager;
        PluginManager<AbstractFood>* foodManager;

    public:
        PluginTest();

    private slots:
        void nameList();
        void errors();
        void staticPlugin();
        void dynamicPlugin();
        void hierarchy();
        void crossManagerDependencies();

        void reloadPluginDirectory();
        void reload();
};

}}}

#endif
