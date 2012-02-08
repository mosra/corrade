# Author: mosra <mosra@centrum.cz>
_pkgname=corrade
pkgname=mingw32-corrade
pkgver=0.2
pkgrel=1
pkgdesc="Multiplatform plugin management and utility library (mingw32)"
arch=('i686' 'x86_64')
url="https://github.com/mosra/corrade"
license=('LGPLv3')
makedepends=('cmake' 'corrade')
conflicts=('corrade-git')
source=("https://github.com/mosra/${_pkgname}/tarball/v${pkgver}/${_pkgname}-${pkgver}.tar.gz"
        "basic-mingw32.cmake")
md5sums=('d0859887f9abbaa836663965d4a521e4'
         '2da2a35597c5de37a13f8801053b3921')
options=(!buildflags !strip)

build() {
    mkdir -p "${srcdir}/build"
    cd "${srcdir}/build"

    cmake ../mosra-${_pkgname}-87ac3ee \
        -DCMAKE_TOOLCHAIN_FILE=../basic-mingw32.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/i486-mingw32
    make
}

package() {
    cd "$srcdir/build"
    make DESTDIR="$pkgdir/" install
}
