# Author: mosra <mosra@centrum.cz>
pkgname=corrade
pkgver=0.2
pkgrel=1
pkgdesc="Multiplatform plugin management and utility library"
arch=('i686' 'x86_64')
url="https://github.com/mosra/corrade"
license=('LGPLv3')
makedepends=('cmake')
conflicts=('corrade-git')
source=("https://github.com/mosra/${pkgname}/tarball/v${pkgver}/${pkgname}-${pkgver}.tar.gz")
md5sums=('d0859887f9abbaa836663965d4a521e4')

build() {
    mkdir -p "${srcdir}/build"
    cd "${srcdir}/build"

    cmake ../mosra-${pkgname}-87ac3ee \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    make
}

package() {
    cd "$srcdir/build"
    make DESTDIR="$pkgdir/" install
}
