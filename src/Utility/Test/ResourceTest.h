#ifndef Map2X_Utility_Test_ResourceTest_h
#define Map2X_Utility_Test_ResourceTest_h
/*
    Copyright © 2007, 2008, 2009, 2010 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Map2X.

    Map2X is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Map2X is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

#include <QtCore/QObject>

namespace Map2X { namespace Utility { namespace Test {

class ResourceTest: public QObject {
    Q_OBJECT

    public:
        ResourceTest(QObject* parent = 0);

    private:
        QByteArray predisposition, consequence;

    private slots:
        void compile();
        void get();
};

}}}

#endif
