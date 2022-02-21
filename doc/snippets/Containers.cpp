/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020, 2021, 2022
              Vladimír Vondruš <mosra@centrum.cz>

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
#include <string>
#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#endif

#include "Corrade/Containers/Array.h"
#include "Corrade/Containers/ArrayTuple.h"
#include "Corrade/Containers/BigEnumSet.hpp"
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/LinkedList.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pair.h"
#include "Corrade/Containers/Pointer.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Containers/Triple.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Directory.h"

using namespace Corrade;

#define DOXYGEN_IGNORE(...) __VA_ARGS__
#define DOXYGEN_ELLIPSIS(...) __VA_ARGS__

namespace Other {
/* [EnumSet-usage] */
enum class Feature: unsigned int {
    Fast = 1 << 0,
    Cheap = 1 << 1,
    Tested = 1 << 2,
    Popular = 1 << 3
};

typedef Containers::EnumSet<Feature> Features;
CORRADE_ENUMSET_OPERATORS(Features)
/* [EnumSet-usage] */
}

/* [EnumSet-friend] */
class Application {
    private:
        enum class Flag: unsigned int {
            Redraw = 1 << 0,
            Exit = 1 << 1
        };

        typedef Containers::EnumSet<Flag> Flags;
        CORRADE_ENUMSET_FRIEND_OPERATORS(Flags)
};
/* [EnumSet-friend] */

enum class Feature: unsigned int;
typedef Containers::EnumSet<Feature> Features;
Utility::Debug& operator<<(Utility::Debug& debug, Features value);
/* [enumSetDebugOutput] */
enum class Feature: unsigned int {
    Fast = 1 << 0,
    Cheap = 1 << 1,
    Tested = 1 << 2,
    Popular = 1 << 3
};

// already defined to print values as e.g. Feature::Fast and Features(0xabcd)
// for unknown values
Utility::Debug& operator<<(Utility::Debug&, Feature);

typedef Containers::EnumSet<Feature> Features;
CORRADE_ENUMSET_OPERATORS(Features)

Utility::Debug& operator<<(Utility::Debug& debug, Features value) {
    return Containers::enumSetDebugOutput(debug, value, "Features{}", {
        Feature::Fast,
        Feature::Cheap,
        Feature::Tested,
        Feature::Popular});
}
/* [enumSetDebugOutput] */

namespace Big1 {
/* [BigEnumSet-usage1] */
/* 64 values at most */
enum class Feature: std::uint64_t {
    DeferredRendering = 1 << 0,
    AreaLights = 1 << 1,
    GlobalIllumination = 1 << 2,
    Shadows = 1 << 3,
    Reflections = 1 << 4,
    DOXYGEN_ELLIPSIS()
};

typedef Containers::EnumSet<Feature>
    Features;
CORRADE_ENUMSET_OPERATORS(Features)
/* [BigEnumSet-usage1] */
}

namespace Big2 {
/* [BigEnumSet-usage2] */
/* 256 values at most, for an 8-bit type */
enum class Feature: std::uint8_t {
    DeferredRendering = 0,
    AreaLights = 1,
    GlobalIllumination = 2,
    Shadows = 3,
    Reflections = 4,
    DOXYGEN_ELLIPSIS()
};

typedef Containers::BigEnumSet<Feature>
    Features;
CORRADE_ENUMSET_OPERATORS(Features)
/* [BigEnumSet-usage2] */
}

namespace Big3 {
enum class Feature: std::uint8_t;
typedef Containers::BigEnumSet<Feature> Features;
Utility::Debug& operator<<(Utility::Debug& debug, Features value);
/* [bigEnumSetDebugOutput] */
enum class Feature: std::uint8_t {
    Fast = 0,
    Cheap = 1,
    Tested = 2,
    Popular = 3
};

// already defined to print values as e.g. Feature::Fast and Features(0xab)
// for unknown values
Utility::Debug& operator<<(Utility::Debug&, Feature);

typedef Containers::BigEnumSet<Feature> Features;
CORRADE_ENUMSET_OPERATORS(Features)

Utility::Debug& operator<<(Utility::Debug& debug, Features value) {
    return Containers::bigEnumSetDebugOutput(debug, value, "Features{}");
}
/* [bigEnumSetDebugOutput] */
}

namespace LL1 {
class Object;
/* [LinkedList-list-pointer] */
class ObjectGroup: public Containers::LinkedList<Object> {
    DOXYGEN_ELLIPSIS()
};

class Object: public Containers::LinkedListItem<Object, ObjectGroup> {
    public:
        ObjectGroup* group() { return list(); }

    DOXYGEN_ELLIPSIS()
};
/* [LinkedList-list-pointer] */
}

namespace LL2 {
class Object;
/* [LinkedList-private-inheritance] */
class ObjectGroup: private Containers::LinkedList<Object> {
    friend Containers::LinkedList<Object>;
    friend Containers::LinkedListItem<Object, ObjectGroup>;

    public:
        Object* firstObject() { return first(); }
        Object* lastObject() { return last(); }

    DOXYGEN_ELLIPSIS()
};

class Object: private Containers::LinkedListItem<Object, ObjectGroup> {
    friend Containers::LinkedList<Object>;
    friend Containers::LinkedListItem<Object, ObjectGroup>;

    public:
        ObjectGroup* group() { return list(); }
        Object* previousObject() { return previous(); }
        Object* nextObject() { return next(); }

    DOXYGEN_ELLIPSIS()
};
/* [LinkedList-private-inheritance] */
}

int main() {

{
/* [Array-usage] */
/* Create an array with 5 integers and set them to some value */
Containers::Array<int> a{5};
int b = 0;
for(auto& i: a) i = b++;        // a == {0, 1, 2, 3, 4}

/* Create an array from given values */
auto c = Containers::array<int>({3, 18, -157, 0});
c[3] = 25;                      // c == {3, 18, -157, 25}
/* [Array-usage] */
}

{
/* [Array-usage-initialization] */
/* These two are equivalent */
Containers::Array<int> a1{5};
Containers::Array<int> a2{ValueInit, 5};

/* Array of 100 integers, uninitialized */
Containers::Array<int> b{NoInit, 100};

/* Array of a type with no default constructor. All five elements will be
   initialized to {5.2f, 0.5f, 1.0f}. */
struct Vec3 {
    explicit Vec3(float, float, float) {}
};
Containers::Array<Vec3> c{DirectInit, 5, 5.2f, 0.4f, 1.0f};

/* Array from an initializer list. These two are equivalent. */
Containers::Array<int> d1{InPlaceInit, {1, 2, 3, 4, -5, 0, 42}};
auto d2 = Containers::array<int>({1, 2, 3, 4, -5, 0, 42});
/* [Array-usage-initialization] */
}

/* [Array-usage-wrapping] */
{
    int* data = reinterpret_cast<int*>(std::malloc(25*sizeof(int)));

    // Will call std::free() on destruction
    Containers::Array<int> array{data, 25,
        [](int* data, std::size_t) { std::free(data); }};
}
/* [Array-usage-wrapping] */

{
typedef std::uint64_t GLuint;
void* glMapNamedBuffer(GLuint, int);
void glUnmapNamedBuffer(GLuint);
#define GL_READ_WRITE 0
std::size_t bufferSize{};
/* [Array-usage-deleter] */
class UnmapBuffer {
    public:
        explicit UnmapBuffer(GLuint id): _id{id} {}
        void operator()(char*, std::size_t) { glUnmapNamedBuffer(_id); }

    private:
        GLuint _id;
};

GLuint buffer = DOXYGEN_ELLIPSIS({});
char* data = reinterpret_cast<char*>(glMapNamedBuffer(buffer, GL_READ_WRITE));

// Will unmap the buffer on destruction
Containers::Array<char, UnmapBuffer> array{data, bufferSize, UnmapBuffer{buffer}};
/* [Array-usage-deleter] */
}

{
struct Face {
    int vertexCount;
    std::uint32_t vertices[4];
};

Containers::Array<Face> mesh;

/* [Array-growable] */
/* Optimistically reserve assuming the model consists of just triangles */
Containers::Array<std::uint32_t> triangles;
Containers::arrayReserve(triangles, mesh.size()*3);
for(const Face& face: mesh) {
    /* If it's a quad, convert to two triangles */
    if(face.vertexCount == 4) Containers::arrayAppend(triangles,
        {face.vertices[0], face.vertices[1], face.vertices[2],
         face.vertices[0], face.vertices[2], face.vertices[3]});
    /* Otherwise add as-is */
    else Containers::arrayAppend(triangles,
        {face.vertices[0], face.vertices[1], face.vertices[2]});
}
/* [Array-growable] */
}

{
/* [Array-growable-sanitizer] */
Containers::Array<int> a;
arrayReserve(a, 100);
arrayResize(a, 80);
a[80] = 5; // Even though the memory is there, this causes ASan to complain
/* [Array-growable-sanitizer] */
}

{
/* [Array-NoInit] */
struct Foo {
    explicit Foo(int) {}
};

Containers::Array<Foo> e{NoInit, 5};

int index = 0;
for(Foo& f: e) new(&f) Foo{index++};
/* [Array-NoInit] */
}

{
/* [arrayAllocatorCast] */
Containers::Array<char> data;
Containers::Array<float> floats =
    Containers::arrayAllocatorCast<float>(std::move(data));
arrayAppend(floats, 37.0f);
/* [arrayAllocatorCast] */
}

{
/* [Array-arrayView] */
Containers::Array<std::uint32_t> data;

Containers::ArrayView<std::uint32_t> a{data};
auto b = Containers::arrayView(data);
/* [Array-arrayView] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [Array-arrayView-const] */
const Containers::Array<std::uint32_t> data;

Containers::ArrayView<const std::uint32_t> a{data};
auto b = Containers::arrayView(data);
/* [Array-arrayView-const] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [ArrayView-usage] */
/* Convert from a compile-time-sized C array */
int data1[]{5, 17, -36, 185};
Containers::ArrayView<int> a = data1;               // a.size() == 4

/* Create a const view on a mutable Array */
Containers::Array<int> data2{15};
Containers::ArrayView<const int> b = data2;         // b.size() == 15

/* Construct from a pointer and explicit size */
float* data3 = DOXYGEN_ELLIPSIS({});
Containers::ArrayView<float> c{data3, 1337};        // c.size() == 1337
/* [ArrayView-usage] */
static_cast<void>(a);
static_cast<void>(b);
static_cast<void>(c);
}

{
int data1[]{5, 17, -36, 185};
/* [ArrayView-usage-void] */
Containers::ArrayView<const int> d = data1;         // d.size() == 4
Containers::ArrayView<const void> e = d;            // e.size() == 16
/* [ArrayView-usage-void] */
static_cast<void>(e);
}

{
/* [ArrayView-usage-access] */
Containers::ArrayView<int> view = DOXYGEN_ELLIPSIS({});

if(!view.empty()) {
    int min = view.front();
    for(int i: view) if(i < min) min = i;

    DOXYGEN_ELLIPSIS(static_cast<void>(min);)
}

if(view.size() > 2 && view[2] < 3) view[2] += 5;
/* [ArrayView-usage-access] */
}

{
/* [ArrayView-usage-slicing] */
int data[]{0, 10, 20, 30, 40, 50, 60};
Containers::ArrayView<int> view = data;

Containers::ArrayView<int> a = view.slice(3, 5);    // {30, 40, 50}
Containers::ArrayView<int> b = view.prefix(4);      // {0, 10, 20, 30}
Containers::ArrayView<int> c = view.suffix(2);      // {50, 60}
Containers::ArrayView<int> d = view.except(2);      // {0, 10, 20, 30, 40}
/* [ArrayView-usage-slicing] */
static_cast<void>(a);
static_cast<void>(b);
static_cast<void>(c);
static_cast<void>(d);

/* [ArrayView-usage-slicing2] */
int* end = view;
while(*end < 25) ++end;
Containers::ArrayView<int> numbersLessThan25 = view.prefix(end); // {0, 10, 20}

int* fortyfive = nullptr;
for(int& i: view) if(i == 45) {
    fortyfive = &i;
    break;
}
Containers::ArrayView<int> fortyfiveAndBeyond = view.suffix(fortyfive); // {}
/* [ArrayView-usage-slicing2] */
static_cast<void>(numbersLessThan25);
static_cast<void>(fortyfiveAndBeyond);

/* [ArrayView-usage-slicing3] */
int min3(Containers::ArrayView3<const int>);

int minOfFirstThree = min3(view.prefix<3>());
/* [ArrayView-usage-slicing3] */
static_cast<void>(minOfFirstThree);
}

{
/* [arrayView] */
std::uint32_t* data = DOXYGEN_ELLIPSIS(nullptr);

Containers::ArrayView<std::uint32_t> a{data, 5};
auto b = Containers::arrayView(data, 5);
/* [arrayView] */
static_cast<void>(b);
}

{
/* [arrayView-array] */
std::uint32_t data[15];

Containers::ArrayView<std::uint32_t> a{data};
auto b = Containers::arrayView(data);
/* [arrayView-array] */
static_cast<void>(b);
}

{
/* [arrayView-StaticArrayView] */
Containers::StaticArrayView<15, std::uint32_t> data;

Containers::ArrayView<std::uint32_t> a{data};
auto b = Containers::arrayView(data);
/* [arrayView-StaticArrayView] */
static_cast<void>(b);
}

{
/* [arrayCast] */
int data[15];
auto a = Containers::arrayView(data);           // a.size() == 15
auto b = Containers::arrayCast<char>(a);        // b.size() == 60
/* [arrayCast] */
static_cast<void>(b);
}

{
/* [arraySize] */
int data[15];

std::size_t size = Containers::arraySize(data); // size == 15
/* [arraySize] */
static_cast<void>(size);
}

{
struct VkAttachmentDescription {};
struct VkSubpassDescription {};
struct VkSubpassDependency {};
struct VkRenderPassCreateInfo {
    unsigned attachmentCount;
    const VkAttachmentDescription* pAttachments;
    unsigned subpassCount;
    const VkSubpassDescription* pSubpasses;
    unsigned dependencyCount;
    const VkSubpassDependency* pDependencies;
};
std::size_t subpassCount{}, dependencyCount{};
/* [ArrayTuple-usage] */
Containers::ArrayView<VkAttachmentDescription> attachments;
Containers::ArrayView<VkSubpassDescription> subpasses;
Containers::ArrayView<VkSubpassDependency> dependencies;
Containers::ArrayTuple data{
    {3, attachments},
    {subpassCount, subpasses},
    {dependencyCount, dependencies}
};

// Fill the attachment, subpass and dependency info...

VkRenderPassCreateInfo info{DOXYGEN_ELLIPSIS()};
info.attachmentCount = attachments.size();
info.pAttachments = attachments;
info.subpassCount = subpasses.size();
info.pSubpasses = subpasses;
info.dependencyCount = dependencies.size();
info.pDependencies = dependencies;
/* [ArrayTuple-usage] */
static_cast<void>(info);
}

{
/* [ArrayTuple-usage-nontrivial] */
Containers::ArrayView<std::string> strings;
Containers::ArrayView<Containers::Reference<std::string>> references;
Containers::ArrayTuple data{
    {ValueInit, 15, strings},
    {NoInit, 15, references}
};

/* Initialize all references to point to the strings */
for(std::size_t i = 0; i != strings.size(); ++i)
    new(references + i) Containers::Reference<std::string>{strings[i]};
/* [ArrayTuple-usage-nontrivial] */
}

#if defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT))
{
/* [ArrayTuple-usage-mmap] */
Containers::ArrayView<std::uint64_t> latencies;
Containers::ArrayView<float> averages;
Containers::ArrayTuple data{
    {{NoInit, 200*1024*1024, latencies},
     {NoInit, 200*1024*1024, averages}},
    [](std::size_t size, std::size_t)
        -> std::pair<char*, Utility::Directory::MapDeleter>
    {
        Containers::Array<char, Utility::Directory::MapDeleter> data =
            Utility::Directory::mapWrite("storage.tmp", size);
        Utility::Directory::MapDeleter deleter = data.deleter();
        return {data, deleter};
    }
};
/* [ArrayTuple-usage-mmap] */
}
#endif

{
/* [StaticArrayView-usage] */
Containers::ArrayView<int> data;

// Take elements 7 to 11
Containers::StaticArrayView<5, int> fiveInts = data.slice<5>(7);

// Convert back to ArrayView
Containers::ArrayView<int> fiveInts2 = data; // fiveInts2.size() == 5
Containers::ArrayView<int> threeInts = data.slice(2, 5);
/* [StaticArrayView-usage] */
static_cast<void>(fiveInts);
static_cast<void>(fiveInts2);
static_cast<void>(threeInts);
}

{
/* [staticArrayView] */
int* data = DOXYGEN_ELLIPSIS(nullptr);

Containers::StaticArrayView<5, int> a{data};
auto b = Containers::staticArrayView<5>(data);
/* [staticArrayView] */
static_cast<void>(b);
}

{
/* [staticArrayView-array] */
int data[15];

Containers::StaticArrayView<15, int> a{data};
auto b = Containers::staticArrayView(data);
/* [staticArrayView-array] */
static_cast<void>(b);
}

{
/* [arrayCast-StaticArrayView] */
int data[15];
auto a = Containers::staticArrayView(data); // a.size() == 15
Containers::StaticArrayView<60, char> b = Containers::arrayCast<char>(a);
/* [arrayCast-StaticArrayView] */
static_cast<void>(b);
}

{
/* [arrayCast-StaticArrayView-array] */
int data[15];
auto a = Containers::arrayCast<char>(data); // a.size() == 60
/* [arrayCast-StaticArrayView-array] */
static_cast<void>(a);
}

{
/* [enumSetDebugOutput-usage] */
// prints Feature::Fast|Feature::Cheap
Utility::Debug{} << (Feature::Fast|Feature::Cheap);

// prints Feature::Popular|Feature(0xdead)
Utility::Debug{} << (Feature::Popular|Feature(0xdead));

// prints Features{}
Utility::Debug{} << Features{};
/* [enumSetDebugOutput-usage] */
}

{
/* It's incorrect, of course, we're using the EnumSet instead of BigEnumSet
   here */
/* [bigEnumSetDebugOutput-usage] */
// prints Feature::Fast|Feature::Cheap
Utility::Debug{} << (Feature::Fast|Feature::Cheap);

// prints Feature::Popular|Feature(0xca)|Feature(0xfe)
Utility::Debug{} << (Feature::Popular|Feature(0xca)|Feature(0xfe));

// prints Features{}
Utility::Debug{} << Features{};
/* [bigEnumSetDebugOutput-usage] */
}

{
/* [LinkedList-usage] */
class Object: public Containers::LinkedListItem<Object> {
    // ...
};

Containers::LinkedList<Object> list;
list.insert(new Object);
list.insert(new Object);

list.erase(list.last());
/* [LinkedList-usage] */

/* [LinkedList-traversal] */
for(Object& o: list) {
    DOXYGEN_ELLIPSIS(static_cast<void>(o);)
}
/* [LinkedList-traversal] */

/* [LinkedList-traversal-classic] */
for(Object* i = list.first(); i; i = i->next()) {
    DOXYGEN_ELLIPSIS(static_cast<void>(i);)
}
/* [LinkedList-traversal-classic] */

{
Object *item = DOXYGEN_ELLIPSIS(nullptr), *before = DOXYGEN_ELLIPSIS(nullptr);
/* [LinkedList-move] */
if(item != before) {
    list.cut(item);
    list.move(item, before);
}
/* [LinkedList-move] */
}
}

{
/* [LinkedListItem-usage] */
class Item: public Containers::LinkedListItem<Item> {
    // ...
};
/* [LinkedListItem-usage] */
}

{
/* [optional] */
std::string value;

auto a = Containers::Optional<std::string>{value};
auto b = Containers::optional(value);
/* [optional] */
}

{
/* [optional-inplace] */
auto a = Containers::Optional<std::string>{InPlaceInit, 'a', 'b'};
auto b = Containers::optional<std::string>('a', 'b');
/* [optional-inplace] */
}

{
/* [pair] */
auto a = Containers::Pair<float, int>{35.0f, 7};
auto b = Containers::pair(35.0f, 7);
/* [pair] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [triple] */
auto a = Containers::Triple<float, int, bool>{35.0f, 7, true};
auto b = Containers::triple(35.0f, 7, true);
/* [triple] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [pointer] */
std::string* ptr = DOXYGEN_ELLIPSIS({});

auto a = Containers::Pointer<std::string>{ptr};
auto b = Containers::pointer(ptr);
/* [pointer] */
}

{
/* [pointer-inplace] */
auto a = Containers::Pointer<std::string>{InPlaceInit, 'a', 'b'};
auto b = Containers::pointer<std::string>('a', 'b');
/* [pointer-inplace] */
}

#ifdef __linux__
/* [ScopeGuard-usage] */
{
    int fd = open("file.dat", O_RDONLY);
    Containers::ScopeGuard e{fd, close};
} // fclose(f) gets called at the end of the scope
/* [ScopeGuard-usage] */

{
Containers::StringView filename;
/* [ScopeGuard-deferred] */
Containers::ScopeGuard e{NoCreate};

/* Read from stdin if desired, otherwise scope-guard an opened file */
int fd;
if(filename == "-") {
    fd = STDIN_FILENO;
} else {
    fd = open(filename.data(), O_RDONLY);
    e = Containers::ScopeGuard{fd, close};
}
/* [ScopeGuard-deferred] */
}
#endif

{
/* [ScopeGuard-lambda] */
FILE* f{};

{
    f = fopen("file.dat", "r");
    Containers::ScopeGuard e{&f, [](FILE** f) {
        fclose(*f);
        *f = nullptr;
    }};
}

// f is nullptr again
/* [ScopeGuard-lambda] */
}

/* [ScopeGuard-usage-no-handle] */
{
    Containers::ScopeGuard e{[]() {
        Utility::Debug{} << "We're done here!";
    }};
}
/* [ScopeGuard-usage-no-handle] */

/* [ScopeGuard-returning-lambda] */
{
    auto closer = [](FILE* f) {
        return fclose(f) != 0;
    };

    FILE* f = fopen("file.dat", "r");
    Containers::ScopeGuard e{f, static_cast<bool(*)(FILE*)>(closer)};
}
/* [ScopeGuard-returning-lambda] */

{
/* [StaticArray-usage] */
/* Create an array with 5 integers and set them to some value */
Containers::StaticArray<5, int> a;
int b = 0;
for(auto& i: a) i = b++;            // a == {0, 1, 2, 3, 4}

/* Create an array from given values */
Containers::StaticArray<4, int> c{3, 18, -157, 0};
c[3] = 25;                          // c == {3, 18, -157, 25}
/* [StaticArray-usage] */
}

{
/* [StaticArray-usage-initialization] */
/* These two are equivalent */
Containers::StaticArray<5, int> a1;
Containers::StaticArray<5, int> a2{DefaultInit};

/* Array of 100 integers, uninitialized */
Containers::StaticArray<100, int> b{NoInit};

/* Array of 4 values initialized in-place. These two are equivalent. */
Containers::StaticArray<4, int> c1{3, 18, -157, 0};
Containers::StaticArray<4, int> c2{InPlaceInit, 3, 18, -157, 0};

/* Array of a type with no default constructor. All five elements will be
   initialized to {5.2f, 0.5f, 1.0f}. */
struct Vec3 {
    explicit Vec3(float, float, float) {}
};
Containers::StaticArray<5, Vec3> d{DirectInit, 5.2f, 0.4f, 1.0f};
/* [StaticArray-usage-initialization] */
}

{
/* [StaticArray-NoInit] */
struct Foo {
    explicit Foo(int) {}
};

Containers::StaticArray<5, Foo> e{NoInit};

int index = 0;
for(Foo& f: e) new(&f) Foo{index++};
/* [StaticArray-NoInit] */
}

{
/* [StaticArray-arrayView] */
Containers::StaticArray<5, int> data;

Containers::ArrayView<int> a{data};
auto b = Containers::arrayView(data);
/* [StaticArray-arrayView] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StaticArray-arrayView-const] */
const Containers::StaticArray<5, int> data;

Containers::ArrayView<const int> a{data};
auto b = Containers::arrayView(data);
/* [StaticArray-arrayView-const] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StaticArray-staticArrayView] */
Containers::StaticArray<5, int> data;

Containers::StaticArrayView<5, int> a{data};
auto b = Containers::staticArrayView(data);
/* [StaticArray-staticArrayView] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StaticArray-staticArrayView-const] */
const Containers::StaticArray<5, int> data;

Containers::StaticArrayView<5, const int> a{data};
auto b = Containers::staticArrayView(data);
/* [StaticArray-staticArrayView-const] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StridedArrayView-usage] */
struct Position {
    float x, y;
};

Position positions[]{{-0.5f, -0.5f}, { 0.5f, -0.5f}, { 0.0f,  0.5f}};

Containers::StridedArrayView1D<float> horizontalPositions{positions,
    &positions[0].x, Containers::arraySize(positions), sizeof(Position)};

/* Move to the right */
for(float& x: horizontalPositions) x += 3.0f;
/* [StridedArrayView-usage] */
}

{
/* [StridedArrayView-usage-conversion] */
int data[] { 1, 42, 1337, -69 };

Containers::StridedArrayView1D<int> a{data, 4, sizeof(int)};
Containers::StridedArrayView1D<int> b = data;
/* [StridedArrayView-usage-conversion] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StridedArrayView-usage-reshape] */
int data3D[2*3*5];

Containers::StridedArrayView3D<int> a{data3D, {2, 3, 5}, {3*5*4, 5*4, 4}};
Containers::StridedArrayView3D<int> b{data3D, {2, 3, 5}};
/* [StridedArrayView-usage-reshape] */
}

{
std::uint32_t rgbaData[256*256*16]{};
/* [StridedArrayView-usage-3d] */
/* Sixteen 256x256 RGBA8 images */
Containers::StridedArrayView3D<std::uint32_t> images{rgbaData, {16, 256, 256}};

/* Make the center 64x64 pixels of each image opaque red */
for(auto&& image: images.slice({0, 96, 96}, {16, 160, 160}))
    for(auto&& row: image)
        for(std::uint32_t& pixel: row)
            pixel = 0xff0000ff;
/* [StridedArrayView-usage-3d] */

/* [StridedArrayView-usage-3d-slice-2d] */
Containers::StridedArrayView2D<std::uint32_t> image = images[4];
Containers::StridedArrayView2D<std::uint32_t> imageCenter =
    images.slice<2>({4, 96, 96}, {5, 160, 160});
/* [StridedArrayView-usage-3d-slice-2d] */
static_cast<void>(imageCenter);

/* [StridedArrayView-usage-inflate] */
/* First dimension is Y, second X, third R/G/B/A */
Containers::StridedArrayView3D<std::uint8_t> channels =
    Containers::arrayCast<3, std::uint8_t>(image);

Utility::Debug{} << channels[128][128][1]; // green channel, 0xff
/* [StridedArrayView-usage-inflate] */

/* [StridedArrayView-usage-rotate] */
/* Bottom left before is now bottom right */
Containers::StridedArrayView2D<std::uint32_t> rotated90DegLeft =
    image.transposed<0, 1>().flipped<0>();
/* [StridedArrayView-usage-rotate] */
static_cast<void>(rotated90DegLeft);

/* [StridedArrayView-usage-broadcast] */
int data[8] { 0, 1, 2, 3, 4, 5, 6, 7 };

/* 8x8 array with 0–7 repeated in every row */
Containers::StridedArrayView2D<int> gradient =
    Containers::StridedArrayView1D<int>{data}.slice<2>().broadcasted<1>(8);
/* [StridedArrayView-usage-broadcast] */
static_cast<void>(gradient);
}

{
struct Position {
    float x, y;
};
/* [stridedArrayView-data-member] */
Containers::ArrayView<Position> data;

Containers::StridedArrayView1D<float> a{data, &data[0].x, 9, sizeof(Position)};
auto b = Containers::stridedArrayView(data, &data[0].x, 9, sizeof(Position));
/* [stridedArrayView-data-member] */
static_cast<void>(b);
}

{
struct Position {
    float x, y;
};
/* [stridedArrayView-data] */
Containers::ArrayView<float> data;

Containers::StridedArrayView1D<float> a{data, 5, 8};
auto b = Containers::stridedArrayView(data, 5, 8);
/* [stridedArrayView-data] */
static_cast<void>(b);
}

{
/* [StridedArrayView-slice-member] */
struct Position {
    float x, y;
};

Containers::StridedArrayView1D<Position> data;
Containers::StridedArrayView1D<float> y = data.slice(&Position::y);
/* [StridedArrayView-slice-member] */
static_cast<void>(y);
}

{
/* [StridedArrayView-slice-member-function] */
class Color3 {
    public:
        DOXYGEN_ELLIPSIS()

        float& r()DOXYGEN_IGNORE({ return data[0]; } int foo);
        float& g()DOXYGEN_IGNORE({ return data[1]; } int bar);
        float& b()DOXYGEN_IGNORE({ return data[2]; } int baz);

    private:
        DOXYGEN_ELLIPSIS(float data[3];)
};

Containers::StridedArrayView1D<Color3> colors;
Containers::StridedArrayView1D<float> greens = colors.slice(&Color3::g);
/* [StridedArrayView-slice-member-function] */
static_cast<void>(greens);
}

{
/* [stridedArrayView] */
std::uint32_t* data = DOXYGEN_ELLIPSIS(nullptr);

Containers::StridedArrayView1D<std::uint32_t> a{data, 5};
auto b = Containers::stridedArrayView(data, 5);
/* [stridedArrayView] */
static_cast<void>(b);
}

{
/* [stridedArrayView-array] */
std::uint32_t data[15];

Containers::StridedArrayView1D<std::uint32_t> a{data};
auto b = Containers::stridedArrayView(data);
/* [stridedArrayView-array] */
static_cast<void>(b);
}

{
/* [stridedArrayView-ArrayView] */
Containers::ArrayView<std::uint32_t> data;

Containers::StridedArrayView1D<std::uint32_t> a{data};
auto b = Containers::stridedArrayView(data);
/* [stridedArrayView-ArrayView] */
static_cast<void>(b);
}

{
/* [stridedArrayView-StaticArrayView] */
Containers::StaticArrayView<15, std::uint32_t> data;

Containers::StridedArrayView1D<std::uint32_t> a{data};
auto b = Containers::stridedArrayView(data);
/* [stridedArrayView-StaticArrayView] */
static_cast<void>(b);
}

{
/* [arrayCast-StridedArrayView] */
struct Pixel {
    std::uint8_t r, g, b, a;
};

Pixel pixels[]{{0x33, 0xff, 0x99, 0x66}, {0x11, 0xab, 0x33, 0xff}};

auto red = Containers::StridedArrayView1D<std::uint8_t>{pixels, &pixels[0].r, 2, 4};
auto rgba = Containers::arrayCast<Pixel>(red);
/* [arrayCast-StridedArrayView] */
static_cast<void>(rgba);
}

{
/* [arrayCast-StridedArrayView-inflate] */
struct Rgb {
    std::uint8_t r, g, b;
};

Containers::ArrayView<Rgb> pixels;

Containers::StridedArrayView2D<Rgb> view{pixels, {128, 128}};
Containers::StridedArrayView3D<std::uint8_t> rgb =
    Containers::arrayCast<3, std::uint8_t>(view);
/* [arrayCast-StridedArrayView-inflate] */
static_cast<void>(rgb);
}

{
/* [StringView-usage-literal] */
using namespace Containers::Literals;

Containers::StringView a = "hello world!";
Containers::StringView b = "hello world!"_s;
/* [StringView-usage-literal] */
static_cast<void>(a);
static_cast<void>(b);
}

{
using namespace Containers::Literals;
/* [StringView-usage-literal-null] */
Containers::StringView a = "hello\0world!";     // a.size() == 5
Containers::StringView b = "hello\0world!"_s;   // a.size() == 12
/* [StringView-usage-literal-null] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StringView-usage-mutable] */
char a[] = "hello world!";
Containers::MutableStringView view = a;
view[5] = '\0';
/* [StringView-usage-mutable] */
static_cast<void>(a);
}

{
using namespace Containers::Literals;
/* [StringView-usage-slicing] */
Containers::StringView file = "Master Of Puppets.mp3";
Containers::StringView name = file.exceptSuffix(".mp3"); // "Master Of Puppets"
/* [StringView-usage-slicing] */
static_cast<void>(name);
}

{
/* [StringView-join] */
using namespace Containers::Literals;

Containers::String a = ", "_s.join({"hello", "world"});
/* [StringView-join] */
}

{
using namespace Containers::Literals;
/* [String-usage-literal-null] */
Containers::String a = "hello\0world!";         // a.size() == 5
Containers::String b = "hello\0world!"_s;       // a.size() == 12
/* [String-usage-literal-null] */
static_cast<void>(a);
static_cast<void>(b);
}

{
std::size_t size{};
/* [String-usage-wrapping] */
{
    /* Extra space for a null terminator */
    char* data = reinterpret_cast<char*>(std::malloc(size + 1));

    // Will call std::free() on destruction
    Containers::String string{data, size,
        [](char* data, std::size_t) { std::free(data); }};
}
/* [String-usage-wrapping] */
}

}
