#!/bin/bash

set -x

NULL=

PKG_CONFIG_DEFAULT_LIBDIR=$( pkg-config --variable pc_path pkg-config)
export PKG_CONFIG_LIBDIR="../../../nxcomp:../../lib/:../../../nxcompshad:$PKG_CONFIG_DEFAULT_LIBDIR"

SRC=.
PRJ="${SRC}/$@"

build() {
	PKG=$1
	shift
	[ -d ${SRC}/$PKG ] || return 0

	OPT="$@"

	pushd "${SRC}/$PKG"
	if [ -e Makefile ]; then
		make distclean
	fi

	if [ -e configure ]; then
		rm -f configure
	fi

	if [ ! -e configure ]; then
		./autogen.sh $OPT || exit 1
	fi

	make

	popd
}

VERSIONSTRING=$(cat ../../VERSION)

# FIXME: this is not clean, should be done in the configure.ac scripts
export CPPFLAGS="$CPPFLAGS -DNXAGENT_SERVER -DNX_VERSION_CURRENT_STRING='\"${VERSIONSTRING}\"'"
export CXXFLAGS="$CXXFLAGS -DNXAGENT_SERVER"
export CFLAGS="$CFLAGS -DNXAGENT_SERVER"

export CONFFLAGS="--enable-unix-transport		\
                  --enable-tcp-transport		\
                  --disable-xvmc			\
                  --disable-install-libxf86config	\
                  --disable-install-setuid		\
                  --disable-xorgcfg			\
                  --disable-kbd_mode			\
                  --disable-screensaver			\
                  --disable-xf86misc			\
                  --disable-xf86vidmode			\
                  --enable-ipv6				\
                  --enable-xinerama			\
                  --enable-glx				\
                  --enable-secure-rpc			\
                  --enable-mitshm			\
                  --enable-nxagent			\
                  ${NULL}"

export XSERVERCFLAGS_CFLAGS="						  \
                            -I`pwd`/../exports/include			  \
                            -I`pwd`/../exports/include/nx-X11		  \
                            -I`pwd`/../exports/include/nx-X11/extensions  \
                            -I`pwd`/Xserver/miext/cw			  \
                            ${NULL}"

export XSERVERCFLAGS_LIBS="						  \
                           ${NULL}"

build Xserver $CONFFLAGS --with-mesa-source=$PWD/${SRC}/../extras/Mesa/ PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" || exit 1