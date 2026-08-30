#!/bin/bash

# The bundler rewrites every library path in the binary afterwards, and the
# replacements are longer than what the linker wrote. Without room reserved
# up front install_name_tool refuses the change and asks for a relink, so the
# padding is requested here rather than discovered at packaging time.
LDFLAGS="${LDFLAGS} -Wl,-headerpad_max_install_names"
export LDFLAGS

$(dirname $0)/../../configure --disable-restoreold --enable-automakedefaults \
    --disable-useradd --disable-sysinstall --disable-initscripts \
    --disable-uninstall --disable-etc --disable-games \
    --prefix=/Contents \
    --bindir=/Contents/MacOS \
    --datadir=/Contents/Resources \
    --libdir=/Contents/Frameworks \
    "$@"
