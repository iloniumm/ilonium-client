PACKAGE_VERSION=0.2.9.3.0-ilonas-modpack
PACKAGE_NAME=armagetronad
PACKAGE_TITLE="Armagetron Advanced"

# depends on whether the current build is configured as client or server, maybe don't use?
# PACKAGE_NAME_FULL=armagetronad

# **********************************************************************

# used in several places, must be consistent
DEBIAN_VERSION_BASE=`echo ${PACKAGE_VERSION} | sed -e s,_,~,g -e s,-,+,g -e s,~rc~,~~rc~,`~ppa1 || exit $?
