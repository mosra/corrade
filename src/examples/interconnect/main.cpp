/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017 Vladimír Vondruš <mosra@centrum.cz>

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

#include <string>
#include <Corrade/Interconnect/Emitter.h>
#include <Corrade/Interconnect/Receiver.h>
#include <Corrade/Utility/Debug.h>

using namespace Corrade;

class RemoteControl: public Interconnect::Emitter {
    public:
        Signal triggered(const std::string& password, int timeout) {
            return emit(&RemoteControl::triggered, password, timeout);
        }
};

class Bomb: public Interconnect::Receiver {
    public:
        void launch(const std::string& password, int timeout);
};

void Bomb::launch(const std::string& password, int timeout) {
    if(password != "terrorist69") {
        Utility::Error() << "Wrong password. No apocalypse will be performed.";
        return;
    }

    Utility::Warning() << "Launching bomb in" << timeout << "seconds.";

    // ...

    delete this; // commit suicide
}

int main(int, char**) {
    RemoteControl rc;
    Bomb *bomb1 = new Bomb,
         *bomb2 = new Bomb,
         *bomb3 = new Bomb;

    Interconnect::connect(rc, &RemoteControl::triggered, *bomb1, &Bomb::launch);
    Interconnect::connect(rc, &RemoteControl::triggered, *bomb2, &Bomb::launch);
    Interconnect::connect(rc, &RemoteControl::triggered, *bomb3, &Bomb::launch);

    Utility::Debug() << "Successfully installed" << rc.signalConnectionCount() << "bombs.";

    rc.triggered("terrorist69", 60); // Launch all connected bombs after 60 seconds

    if(rc.signalConnectionCount()) {
        Utility::Error() << "Mission failed!" << rc.signalConnectionCount() << "bombs didn't explode!";
        return 1;
    }

    Utility::Debug() << "Mission succeeded!";
    return 0;
}
