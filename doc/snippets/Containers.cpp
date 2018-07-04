/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <cstdio>
#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

#include "Corrade/Containers/ScopedExit.h"
#include "Corrade/Containers/StridedArrayView.h"

using namespace Corrade;

int main() {

#ifdef __linux__
/* [ScopedExit-usage] */
{
    int fd = open("file.dat", O_RDONLY);
    Containers::ScopedExit e{fd, close};
} // fclose(f) gets called at the end of the scope
/* [ScopedExit-usage] */
#endif

/* [ScopedExit-lambda] */
FILE* f{};

{
    f = fopen("file.dat", "r");
    Containers::ScopedExit e{&f, [](FILE** f) {
        fclose(*f);
        *f = nullptr;
    }};
}

// f is nullptr again
/* [ScopedExit-lambda] */

/* [ScopedExit-returning-lambda] */
{
    auto closer = [](FILE* f) {
        return fclose(f) != 0;
    };

    FILE* f = fopen("file.dat", "r");
    Containers::ScopedExit e{f, static_cast<bool(*)(FILE*)>(closer)};
}
/* [ScopedExit-returning-lambda] */

{
/* [StridedArrayView-usage] */
struct Position {
    float x, y;
};

Position positions[]{{-0.5f, -0.5f}, { 0.5f, -0.5f}, { 0.0f,  0.5f}};

Containers::StridedArrayView<float> horizontalPositions{
    &positions[0].x, Containers::arraySize(positions), sizeof(Position)};

/* Move to the right */
for(float& x: horizontalPositions) x += 3.0f;
/* [StridedArrayView-usage] */
}

{

/* [StridedArrayView-usage-conversion] */
int data[] { 1, 42, 1337, -69 };

Containers::StridedArrayView<int> view1{data, 4, sizeof(int)};
Containers::StridedArrayView<int> view2 = data;
/* [StridedArrayView-usage-conversion] */
static_cast<void>(view2);
}

}
