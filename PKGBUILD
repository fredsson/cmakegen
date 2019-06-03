#Maintainer: Fredrik Andersson <fredrik.ks.andersson@gmail.com>
pkgname=cmakegen-git
pkgver=r40.115b37b
pkgrel=1
pkgdesc="Tool to automatically generate cmake files for c++ projects"
arch=("x86_64")
license=("MIT")
makedepends=("git" "cmake" "make")
depends=("cmake" "make")
source=("git+https://github.com/fredsson/cmakegen.git")
md5sums=("SKIP")

pkgver() {
  cd "$srcdir/${pkgname%-git}"
  ( set -o pipefail
    git describe --long 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' ||
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
  )
}

build() {
  cd "$srcdir/${pkgname%-git}"

  mkdir _build && cd _build
  cmake -DCMAKE_BUILD_TYPE=Release ..

  make
}

package() {
  cd "$srcdir/${pkgname%-git}"/_build

  make DESTDIR="$pkgdir/" install
}