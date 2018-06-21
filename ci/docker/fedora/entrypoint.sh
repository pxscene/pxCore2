#!/bin/bash -xe
#
# Author: Damian Wrobel <dwrobel@ertelnet.rybnik.pl>
#

groupadd --gid $GID "$USER"
useradd -G wheel --uid $UID --gid $GID "$USER"

echo "Hello ${USER}!"

cd "$CWD"

preserved_envs="PATH,DISPLAY,WAYLAND_DISPLAY,XDG_RUNTIME_DIR,CC,CXX"

if [ -f /etc/profile.d/ccache.sh ] ; then

    if [ -w "$CACHE_DIR" ] && [ -d "$CACHE_DIR" ] ; then
        export CCACHE_DIR=$CACHE_DIR/ccache
        sudo -u "$USER" mkdir -p $CCACHE_DIR
        sudo -u "$USER" --preserve-env=CCACHE_DIR ccache --set-config=max_size=2G
        preserved_envs="$preserved_envs,CCACHE_DIR"
    fi

    sed -i 's/unset\ CCACHE_DIR/#unset\ CCACHE_DIR/g' \
           /etc/profile.d/ccache.sh

    source /etc/profile.d/ccache.sh

fi

sudo -H -u "$USER" --preserve-env=$preserved_envs "$@"

