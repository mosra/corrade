# Author: mosra <mosra@centrum.cz>
pkgname=corrade
pkgver=dev
pkgrel=1
pkgdesc="Multiplatform plugin management and utility library"
arch=('i686' 'x86_64')
url="http://mosra.cz/blog/corrade.php"
license=('LGPLv3')
makedepends=('cmake' 'qt')
options=(!strip)
provides=('corrade-git')

build() {
    mkdir -p "$startdir/build"
    cd "$startdir/build/"

    if [ "$CXX" = clang++ ] ; then
        newcxxflags=$(echo $CXXFLAGS | sed s/--param=ssp-buffer-size=4//g)
        export CXXFLAGS="$newcxxflags"
    fi

    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DBUILD_TESTS=TRUE
    make
}

check() {
    cd "$startdir/build"
    ctest --output-on-failure
}

package() {
    cd "$startdir/build"
    make DESTDIR="$pkgdir/" install
}
