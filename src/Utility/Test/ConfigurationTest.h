#ifndef Kompas_Utility_Test_ConfigurationTest_h
#define Kompas_Utility_Test_ConfigurationTest_h
/*
    Copyright © 2007, 2008, 2009, 2010, 2011 Vladimír Vondruš <mosra@centrum.cz>

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

namespace Kompas { namespace Utility { namespace Test {

class ConfigurationTest: public QObject {
    Q_OBJECT

    public:
        ConfigurationTest();

    private slots:
        void parse();
        void parseDirect();
        void empty();
        void invalid();
        void readonly();
        void readonlyWithoutFile();
        void truncate();
        void whitespaces();
        void types();

        void eol_data();
        void eol();

        void uniqueGroups();
        void uniqueKeys();
        void stripComments();

        void autoCreation();
        void directValue();

        /** @todo Merge into parse() and uniqueGroups() */
        void hierarchic();
        void hierarchicUnique();
};

}}}

#endif
