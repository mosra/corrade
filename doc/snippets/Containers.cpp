/*
    This file is part of Corrade.

    Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
                2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>

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
#include "Corrade/Containers/GrowableArray.h"
#include "Corrade/Containers/EnumSet.hpp"
#include "Corrade/Containers/LinkedList.h"
#include "Corrade/Containers/Optional.h"
#include "Corrade/Containers/Pointer.h"
#include "Corrade/Containers/Reference.h"
#include "Corrade/Containers/ScopeGuard.h"
#include "Corrade/Containers/StaticArray.h"
#include "Corrade/Containers/StridedArrayView.h"
#include "Corrade/Containers/String.h"
#include "Corrade/Containers/StringView.h"
#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/Directory.h"

using namespace Corrade;

#define DOXYGEN_IGNORE(...) __VA_ARGS__

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

namespace LL1 {
class Object;
/* [LinkedList-list-pointer] */
class ObjectGroup: public Containers::LinkedList<Object> {
    // ...
};

class Object: public Containers::LinkedListItem<Object, ObjectGroup> {
    public:
        ObjectGroup* group() { return list(); }

    // ...
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

    // ...
};

class Object: private Containers::LinkedListItem<Object, ObjectGroup> {
    friend Containers::LinkedList<Object>;
    friend Containers::LinkedListItem<Object, ObjectGroup>;

    public:
        ObjectGroup* group() { return list(); }
        Object* previousObject() { return previous(); }
        Object* nextObject() { return next(); }

    // ...
};
/* [LinkedList-private-inheritance] */
}

int main() {

{
/* [Array-usage] */
// Create default-initialized array with 5 integers and set them to some value
Containers::Array<int> a{5};
int b = 0;
for(auto& i: a) i = b++; // a = {0, 1, 2, 3, 4}

// Create array from given values
Containers::Array<int> c{Containers::InPlaceInit, {3, 18, -157, 0}};
c[3] = 25; // b = {3, 18, -157, 25}
/* [Array-usage] */
}

{
/* [Array-initialization] */
// These are equivalent
Containers::Array<int> a1{5};
Containers::Array<int> a2{Containers::DefaultInit, 5};

// Array of 100 zeros
Containers::Array<int> b{Containers::ValueInit, 100};

// Array of type with no default constructor
struct Vec3 {
    explicit Vec3(float, float, float) {}
};
Containers::Array<Vec3> c{Containers::DirectInit, 5, 5.2f, 0.4f, 1.0f};

// Array from an initializer list
Containers::Array<int> d{Containers::InPlaceInit, {1, 2, 3, 4, -5, 0, 42}};

// Manual construction of each element
struct Foo {
    explicit Foo(int) {}
};
Containers::Array<Foo> e{Containers::NoInit, 5};
int index = 0;
for(Foo& f: e) new(&f) Foo(index++);
/* [Array-initialization] */
}

/* [Array-wrapping] */
{
    int* data = reinterpret_cast<int*>(std::malloc(25*sizeof(int)));

    // Will call std::free() on destruction
    Containers::Array<int> array{data, 25,
        [](int* data, std::size_t) { std::free(data); }};
}
/* [Array-wrapping] */

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
{
typedef std::uint64_t GLuint;
void* glMapNamedBuffer(GLuint, int);
void glUnmapNamedBuffer(GLuint);
#define GL_READ_WRITE 0
std::size_t bufferSize{};
/* [Array-deleter] */
class UnmapBuffer {
    public:
        explicit UnmapBuffer(GLuint id): _id{id} {}
        void operator()(char*, std::size_t) { glUnmapNamedBuffer(_id); }

    private:
        GLuint _id;
};

GLuint buffer;
char* data = reinterpret_cast<char*>(glMapNamedBuffer(buffer, GL_READ_WRITE));

// Will unmap the buffer on destruction
Containers::Array<char, UnmapBuffer> array{data, bufferSize, UnmapBuffer{buffer}};
/* [Array-deleter] */
}
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
{
/* [ArrayView-usage] */
// `a` gets implicitly converted to const array view
void printArray(Containers::ArrayView<const float> values);
Containers::Array<float> a;
printArray(a);

// Wrapping compile-time array with size information
constexpr const int data[]{ 5, 17, -36, 185 };
Containers::ArrayView<const int> b = data; // b.size() == 4

// Wrapping general array with size information
const int* data2;
Containers::ArrayView<const int> c{data2, 3};
/* [ArrayView-usage] */
static_cast<void>(b);
}
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

{
/* [ArrayView-void-usage] */
Containers::Array<int> a(5);

Containers::ArrayView<void> b(a); // b.size() == 20
/* [ArrayView-void-usage] */
static_cast<void>(b);
}

{
/* [ArrayView-const-void-usage] */
Containers::Array<int> a(5);

Containers::ArrayView<const void> b(a); // b.size() == 20
/* [ArrayView-const-void-usage] */
static_cast<void>(b);
}

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
{
/* [arrayView] */
std::uint32_t* data;

Containers::ArrayView<std::uint32_t> a{data, 5};
auto b = Containers::arrayView(data, 5);
/* [arrayView] */
static_cast<void>(b);
}
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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
std::int32_t data[15];
auto a = Containers::arrayView(data); // a.size() == 15
auto b = Containers::arrayCast<char>(a); // b.size() == 60
/* [arrayCast] */
static_cast<void>(b);
}

{
/* [arraySize] */
std::int32_t a[5];

std::size_t size = Containers::arraySize(a); // size == 5
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

VkRenderPassCreateInfo info{DOXYGEN_IGNORE()};
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
    {Containers::ValueInit, 15, strings},
    {Containers::NoInit, 15, references}
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
    {{Containers::NoInit, 200*1024*1024, latencies},
     {Containers::NoInit, 200*1024*1024, averages}},
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

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
{
/* [staticArrayView] */
std::uint32_t* data;

Containers::StaticArrayView<5, std::uint32_t> a{data};
auto b = Containers::staticArrayView<5>(data);
/* [staticArrayView] */
static_cast<void>(b);
}
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

{
/* [staticArrayView-array] */
std::uint32_t data[15];

Containers::StaticArrayView<15, std::uint32_t> a{data};
auto b = Containers::staticArrayView(data);
/* [staticArrayView-array] */
static_cast<void>(b);
}

{
/* [arrayCast-StaticArrayView] */
std::int32_t data[15];
auto a = Containers::staticArrayView(data); // a.size() == 15
Containers::StaticArrayView<60, char> b = Containers::arrayCast<char>(a);
/* [arrayCast-StaticArrayView] */
static_cast<void>(b);
}

{
/* [arrayCast-StaticArrayView-array] */
std::int32_t data[15];
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
/* [LinkedList-usage] */
class Object: public Containers::LinkedListItem<Object> {
    // ...
};

Containers::LinkedList<Object> list;
list.insert(new Object);
list.insert(new Object);

list.erase(list.last());
/* [LinkedList-usage] */

#if defined(__GNUC__) || defined( __clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* [LinkedList-traversal] */
for(Object& o: list) {
    // ...
}
/* [LinkedList-traversal] */
#if defined(__GNUC__) || defined( __clang__)
#pragma GCC diagnostic pop
#endif

/* [LinkedList-traversal-classic] */
for(Object* i = list.first(); i; i = i->next()) {
    // ...
}
/* [LinkedList-traversal-classic] */

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
{
Object *item, *before;
/* [LinkedList-move] */
if(item != before) {
    list.cut(item);
    list.move(item, before);
}
/* [LinkedList-move] */
}
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
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
auto a = Containers::Optional<std::string>{Containers::InPlaceInit, 'a', 'b'};
auto b = Containers::optional<std::string>('a', 'b');
/* [optional-inplace] */
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
Containers::ScopeGuard e{Containers::NoCreate};

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
// Create default-initialized array with 5 integers and set them to some value
Containers::StaticArray<5, int> a;
int b = 0;
for(auto& i: a) i = b++; // a = {0, 1, 2, 3, 4}

// Create array from given values
Containers::StaticArray<4, int> c{3, 18, -157, 0};
c[3] = 25; // b = {3, 18, -157, 25}
/* [StaticArray-usage] */
}

{
/* [StaticArray-initialization] */
// These two are equivalent
Containers::StaticArray<5, int> a1;
Containers::StaticArray<5, int> a2{Containers::DefaultInit};

// Array of 100 zeros
Containers::StaticArray<100, int> b{Containers::ValueInit};

// Array of 4 values initialized in-place (these two are equivalent)
Containers::StaticArray<4, int> c1{3, 18, -157, 0};
Containers::StaticArray<4, int> c2{Containers::InPlaceInit, 3, 18, -157, 0};

// Array of type with no default constructor
struct Vec3 {
    explicit Vec3(float, float, float) {}
};
Containers::StaticArray<5, Vec3> d{Containers::DirectInit, 5.2f, 0.4f, 1.0f};

// Manual construction of each element
struct Foo {
    explicit Foo(int) {}
};
Containers::StaticArray<5, Foo> e{Containers::NoInit};
int index = 0;
for(Foo& f: e) new(&f) Foo(index++);
/* [StaticArray-initialization] */
}

{
/* [StaticArray-arrayView] */
Containers::StaticArray<5, std::uint32_t> data;

Containers::ArrayView<std::uint32_t> a{data};
auto b = Containers::arrayView(data);
/* [StaticArray-arrayView] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StaticArray-arrayView-const] */
const Containers::StaticArray<5, std::uint32_t> data;

Containers::ArrayView<const std::uint32_t> a{data};
auto b = Containers::arrayView(data);
/* [StaticArray-arrayView-const] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StaticArray-staticArrayView] */
Containers::StaticArray<5, std::uint32_t> data;

Containers::StaticArrayView<5, std::uint32_t> a{data};
auto b = Containers::staticArrayView(data);
/* [StaticArray-staticArrayView] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StaticArray-staticArrayView-const] */
const Containers::StaticArray<5, std::uint32_t> data;

Containers::StaticArrayView<5, const std::uint32_t> a{data};
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
Containers::StridedArrayView2D<std::uint32_t> image = images[5];
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

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
{
/* [pointer] */
std::string* ptr;

auto a = Containers::Pointer<std::string>{ptr};
auto b = Containers::pointer(ptr);
/* [pointer] */
}
#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

{
/* [pointer-inplace] */
auto a = Containers::Pointer<std::string>{Containers::InPlaceInit, 'a', 'b'};
auto b = Containers::pointer<std::string>('a', 'b');
/* [pointer-inplace] */
}

{
/* [StringView-literal] */
using namespace Containers::Literals;

Containers::StringView a = "hello world!";
Containers::StringView b = "hello world!"_s;
/* [StringView-literal] */
static_cast<void>(a);
static_cast<void>(b);
}

{
using namespace Containers::Literals;
/* [StringView-literal-null] */
Containers::StringView a = "hello\0world!";     // a.size() == 5
Containers::StringView b = "hello\0world!"_s;   // a.size() == 12
/* [StringView-literal-null] */
static_cast<void>(a);
static_cast<void>(b);
}

{
/* [StringView-mutable] */
char a[] = "hello world!";
Containers::MutableStringView view = a;
view[5] = '\0';
/* [StringView-mutable] */
static_cast<void>(a);
}

{
using namespace Containers::Literals;
/* [String-literal-null] */
Containers::String a = "hello\0world!";         // a.size() == 5
Containers::String b = "hello\0world!"_s;       // a.size() == 12
/* [String-literal-null] */
static_cast<void>(a);
static_cast<void>(b);
}

}
