#!/bin/bash
# Turns a built tree into an AppImage.
#
# Lives here rather than inside a workflow file because two of them call it now,
# and packaging that exists twice is packaging that only gets fixed once.
#
# Run from the top of the source tree, after make. PACKAGE names the result.

set -e

PACKAGE=${PACKAGE:-Retrocycles}
APPDIR=${APPDIR:-$(pwd)/appdir}

make install DESTDIR="$APPDIR"

( cd docker/build/context && APPDIR="$APPDIR" PACKAGE="$PACKAGE" bash portable/build )

if [ ! -x appimagetool-x86_64.AppImage ]; then
    curl -sSLO https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

# the icon the desktop shows before the thing is even unpacked
find "$APPDIR" -name "*.png" -path "*/icons/*48x48*" -exec cp {} "$APPDIR/.DirIcon" \; || true

# --appimage-extract-and-run: build machines rarely have FUSE, and without it
# the tool cannot mount itself to run
./appimagetool-x86_64.AppImage --appimage-extract-and-run --no-appstream "$APPDIR" "$PACKAGE-x86_64.AppImage"

echo "packaged $PACKAGE-x86_64.AppImage"
