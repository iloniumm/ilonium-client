#!/bin/bash
# Collects a built MinGW tree into a folder that runs on a machine with nothing
# installed on it: the executable, every library it reaches for, and the codecs
# SDL loads by name at the moment it first sees a file of that type.
#
# Run from the top of the source tree inside a MINGW64 shell, after make.

set -e

PACKAGE=${PACKAGE:-Retrocycles}
MINGW=${MINGW:-/mingw64}
pkg=docker/build/context/winsource

# the template the packaging script builds the folder from
if [ ! -d docker/build/context/codeblocks ]; then
    git clone --depth 1 https://gitlab.com/armagetronad/build_codeblocks.git docker/build/context/codeblocks
fi

mkdir -p docker/build/context/source docker/build/context/build

cp -r Makefile configure bootstrap.sh config.h.in version.m4 aclocal.m4 \
      configure.ac Makefile.am Makefile.in batch desktop language models \
      resource scripts sound textures AUTHORS CHANGELOG.md COPYING NEWS news \
      README.md INSTALL ChangeLog src docker/build/context/source/ || true
cp -r language docker/build/context/build/
cp -r config docker/build/context/build/
mkdir -p docker/build/context/build/src/doc
cp -r src/doc/* docker/build/context/build/src/doc/ || true

( cd docker/build/context && bash winsource.sh )

cp src/armagetronad_main.exe "$pkg/$PACKAGE.exe"

# started by the game from beside it, so it has to travel with it
cp src/tron/retrocycles_media_bridge.ps1 "$pkg/"

# Which certificates to trust. The library here is built against OpenSSL, and
# OpenSSL on Windows does not read the store the rest of the system uses: it
# looks for a file. Without one, every request to our own backend failed before
# it was sent - the chat, the reports and the count of who is playing, all three
# at once, with an error that reads like a fault at the far end.
for candidate in "$MINGW/ssl/certs/ca-bundle.crt" "$MINGW/etc/ssl/certs/ca-bundle.crt" \
                 "/usr/ssl/certs/ca-bundle.crt" "/etc/ssl/certs/ca-bundle.crt"; do
    if [ -f "$candidate" ]; then
        cp "$candidate" "$pkg/ca-bundle.crt"
        break
    fi
done

# Libraries carry their soname in the file name, so it moves with the upstream
# version - pinning exact names here only produces a package that quietly loses
# a library the next time one of them is bumped. These are a starting point;
# the dependency walk below is what decides what actually ships.
cp "$MINGW/bin/SDL3.dll" "$pkg/"
for pattern in '*mimalloc*' 'SDL3_image*' 'SDL3_mixer*' 'libxml2*' 'libpng*' 'libcurl*'; do
    cp $MINGW/bin/$pattern.dll "$pkg/" 2>/dev/null || true
done

# SDL_image and SDL_mixer do not link their codecs, they load them by name the
# first time a file of that type shows up. Nothing in the import table mentions
# them, so a dependency walk cannot find them and the package shipped without
# any decoder nobody had listed by hand. Ask the libraries themselves what they
# are going to reach for.
command -v strings >/dev/null || { echo "strings unavailable, cannot resolve codecs"; exit 1; }
for lib in SDL3_image SDL3_mixer; do
    [ -f "$MINGW/bin/$lib.dll" ] || continue
    for codec in $(strings "$MINGW/bin/$lib.dll" | grep -oE '^lib[A-Za-z0-9_+]+(-[0-9]+)?\.dll$' | sort -u); do
        if [ -f "$MINGW/bin/$codec" ] && [ ! -f "$pkg/$codec" ]; then
            echo "codec for $lib: $codec"
            cp "$MINGW/bin/$codec" "$pkg/"
        elif [ ! -f "$MINGW/bin/$codec" ]; then
            echo "  note: $lib can use $codec, not installed - that format will not load"
        fi
    done
done

# Walk the import tables until nothing new turns up, so the package is closed
# over its own dependencies however deep they nest.
echo "collecting library dependencies"
while true; do
    added=0
    for f in "$pkg"/*.dll "$pkg/$PACKAGE.exe"; do
        [ -f "$f" ] || continue
        for dll_path in $(ldd "$f" | awk -v m="$MINGW/bin/" '$3 ~ m {print $3}'); do
            [ -f "$dll_path" ] || continue
            dll_name=$(basename "$dll_path")
            if [ ! -f "$pkg/$dll_name" ]; then
                echo "  $dll_name"
                cp "$dll_path" "$pkg/"
                added=1
            fi
        done
    done
    [ "$added" = "0" ] && break
done

# A truncated or mangled library only shows itself as a refusal to start on the
# player's machine, with nothing to say why. Check here instead, where the build
# can still fail loudly.
echo "verifying the package"
bad=0
for f in "$pkg"/*.dll "$pkg/$PACKAGE.exe"; do
    [ -f "$f" ] || continue
    size=$(stat -c%s "$f")
    magic=$(head -c 2 "$f")
    if [ "$size" -lt 1024 ] || [ "$magic" != "MZ" ]; then
        echo "  BROKEN: $f (size $size, magic '$magic')"
        bad=1
    fi
done

unresolved=0
for f in "$pkg"/*.dll "$pkg/$PACKAGE.exe"; do
    [ -f "$f" ] || continue
    while read -r name arrow target rest; do
        case "$target" in
            *not*|"")
                # Windows brings its own: opengl32, glu32, dsound and the rest
                # of the system libraries live in the machine and must not
                # travel with us - shipping a copy is both wrong and worse than
                # useless. We can only be at fault for what MinGW provides, so
                # that is the only absence worth stopping the build for.
                if [ -f "$MINGW/bin/$name" ]; then
                    echo "  MISSING: $(basename "$f") wants $name, MinGW has it and the package does not"
                    unresolved=1
                fi
                ;;
            $MINGW/bin/*)
                if [ ! -f "$pkg/$(basename "$target")" ]; then
                    echo "  MISSING: $(basename "$f") wants $name, not in package"
                    unresolved=1
                fi
                ;;
        esac
    done < <(ldd "$f")
done

[ "$unresolved" = "0" ] || { echo "package has unresolved library dependencies, refusing to publish"; exit 1; }
[ "$bad" = "0" ] || { echo "packaged binaries are not all valid, refusing to publish"; exit 1; }

# It travels beside the game and is started from there, so a package without it
# loses the media widget silently rather than loudly.
[ -f "$pkg/retrocycles_media_bridge.ps1" ] || { echo "the media bridge is missing from the package"; exit 1; }

# Without it the client cannot reach its own backend at all, and finds out only
# once it is on somebody else's machine.
[ -s "$pkg/ca-bundle.crt" ] || { echo "the certificate list is missing from the package"; exit 1; }

echo "  all $(ls "$pkg"/*.dll | wc -l) libraries and the executable look sane, no unresolved imports"
