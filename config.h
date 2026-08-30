/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* AIX */
/* #undef AIX */

/* BeOS */
/* #undef BEOS */

/* Include pthread support for binary relocation? */
#define BR_PTHREAD 1

/* Define if your system deos not like the pointer tricks in eWall.h. */
/* #undef CAUTION_WALL */

/* Define if you wish to compile a dedicated server */
/* #undef DEDICATED */

/* Define if you wish to use the old and dirty OpenGL initialization method */
/* #undef DIRTY */

/* Define if you dont want to use a custom memory manager. */
#define DONTUSEMEMMANAGER 1

/* Use binary relocation? */
#define ENABLE_BINRELOC /**/

/* Define to 1 if you have the `atan2f' function. */
#define HAVE_ATAN2F 1

/* Define to 1 if you have the `clearenv' function. */
#define HAVE_CLEARENV 1

/* Define to 1 if you have the `cosf' function. */
#define HAVE_COSF 1

/* Define to 1 if you have the `expf' function. */
#define HAVE_EXPF 1

/* Define to 1 if you have the `fabsf' function. */
#define HAVE_FABSF 1

/* Define to 1 if you have the `floorf' function. */
#define HAVE_FLOORF 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `isblank' function. */
#define HAVE_ISBLANK 1

/* Define to 1 if you have the `pthread' library (-lpthread). */
#define HAVE_LIBPTHREAD 1

/* Define if you have the library SDL3 */
#define HAVE_LIBSDL 1

/* Define if you have the SDL3_image library */
#define HAVE_LIBSDL_IMAGE 1

/* Define if you have the SDL3_mixer library */
#define HAVE_LIBSDL_MIXER 1

/* Define to 1 if you have the `wsock32' library (-lwsock32). */
/* #undef HAVE_LIBWSOCK32 */

/* Define if you have the X11 library (-lX11). */
/* #undef HAVE_LIBX11 */

/* Define if you have the libxml2 library */
#define HAVE_LIBXML2 1

/* Define if you have the library ZThread. */
/* #undef HAVE_LIBZTHREAD */

/* Define to 1 if you have the `logf' function. */
#define HAVE_LOGF 1

/* Define if mimalloc is available */
#define HAVE_MIMALLOC 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `sinf' function. */
#define HAVE_SINF 1

/* Define if you have the type socklen_t. */
#define HAVE_SOCKLEN_T 1

/* Define to 1 if you have the `sqrtf' function. */
#define HAVE_SQRTF 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the `tanf' function. */
#define HAVE_TANF 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `wmemset' function. */
#define HAVE_WMEMSET 1

/* enables krawall */
/* #undef KRAWALL */

/* enables krawall server */
#define KRAWALL_SERVER /**/

/* Define if you have libcurl and it supports http */
/* #undef LIBCURL_PROTOCOL_HTTP */

/* GNU/Linux */
#define LINUX 1

/* Mac OS X */
/* #undef MACOSX */

/* maximal number of clients */
/* #undef MAXCLIENTS */

/* Name of package */
#define PACKAGE "armagetronad"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "armagetronad"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "armagetronad 0.2.9.3.0-ilonas-modpack"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "armagetronad"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.2.9.3.0-ilonas-modpack"

/* Define to the compiled in prefix where most game directories will be
   children of */
#define PREFIX "/usr/local"

/* Define to the suffix of all installation folders */
#define PROGDIR_SUFFIX ""

/* Define the short progam name (with possible -dedicated suffix) */
#define PROGNAME "armagetronad"

/* Define the short progam name */
#define PROGNAMEBASE "armagetronad"

/* Define the fully captialized project tile (witout possible " Server"
   suffix) */
#define PROGTITLE "Armagetron Advanced"

/* enables hacky support for deathmatch mode */
/* #undef RESPAWN_HACK */

/* Solaris */
/* #undef SOLARIS */

/* Define to 1 if all of the C90 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Define the top source directory */
#define TOP_SOURCE_DIR "."

/* Version number of package */
#define VERSION "0.2.9.3.0-ilonas-modpack"

/* Windows 9x/NT/2k/XP */
/* #undef WIN32 */

/* Define to 1 if the X Window System is missing or not being used. */
/* #undef X_DISPLAY_MISSING */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */
