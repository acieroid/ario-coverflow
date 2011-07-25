# Contributor: Jaroslav Lichtblau <dragonlord@aur.archlinux.org>
# Contributor: Tim Yang <tdy@gmx.com>
# Contributor: Skottish <skottish97215@gmail.com>

pkgname=ario-coverflow
pkgver=20110725
pkgrel=1
pkgdesc="A lightweight GTK+ client for MPD and XMMS2 inspired by Rhythmbox with coverflow plugin."
arch=('i686' 'x86_64')
url="http://ario-player.sourceforge.net/"
license=('GPL')
depends=('avahi' 'curl' 'libglade' 'libsoup>=2.4' 'taglib' 'gtkglext' 'glew' 'freeglut')
makedepends=('intltool' 'libnotify' 'perlxml' 'pkgconfig' 'subversion' 'git' 'scons')
provides=('ario')
conflicts=('ario')
options=('!libtool' '!strip')
source=('http://awesom.eu/~acieroid/upload/ario-list.patch' 'http://awesom.eu/~acieroid/upload/sconstruct.patch')
md5sums=('2c7e453ccbc3ead06b1900183373ae2b' '2ac7ca710cdf7e295f8512e7c06d5cc2')

_svntrunk="https://ario-player.svn.sourceforge.net/svnroot/ario-player/trunk"
_svnmod="ario"
_svnrev=757

_gitroot="git://github.com/acieroid/${pkgname}.git"
_gitname="${pkgname}"

build() {
  cd "${srcdir}"

  msg "Connecting to the $_svnmod SVN server..."
  if [ -d "$_svnmod/.svn" ]; then
    cd $_svnmod && svn up -r $_svnrev
    msg2 "Local files updated"
  else
    svn co $_svntrunk --config-dir ./ -r $_svnrev $_svnmod
    msg2 "SVN checkout done"
  fi

  rm -rf "${srcdir}/$_svnmod-build"
  cp -r "${srcdir}/$_svnmod" "${srcdir}/$_svnmod-build"
  cd "${srcdir}/$_svnmod-build"

  msg "Patching ario..."
  patch -p0 < ../ario-list.patch

  msg "Starting make of ario..."
  ./autogen.sh --prefix=/usr --enable-debug || return 1
  make || return 1
  make DESTDIR="${pkgdir}" install || return 1

  install -dm755 "${pkgdir}/usr/share/pixmaps"
  ln -sf /usr/share/icons/hicolor/48x48/apps/ario.png \
    "${pkgdir}/usr/share/pixmaps/ario.png"
  install -Dm645 data/ario.desktop "${pkgdir}/usr/share/applications/ario.desktop"

  cd "$srcdir"
  msg "Connecting to GIT server...."

  if [ -d $_gitname ] ; then
    cd $_gitname && git pull origin
    msg "The local files are updated."
  else
    git clone $_gitroot $_gitname
    cd $_gitname
  fi  

  msg "GIT checkout done or server timeout"
  msg "Patching ario-coverflow"
  patch -p1 < ../sconstruct.patch

  msg "Starting make of ario-coverflow..."
  scons DEBUG=1
  scons install DESTDIR="${pkgdir}/usr/lib/ario/plugins/"
}
