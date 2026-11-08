AC_INIT([datalib],[0.1],[local-reorg])
AC_CONFIG_SRCDIR([src/binary.cc])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG([C++])
AC_ARG_WITH([qdp],AS_HELP_STRING([--without-qdp],[Serial build; QDP is not implemented in this checkpoint]),[],[with_qdp=no])
AS_IF([test "x$with_qdp" != xno],[AC_MSG_ERROR([QDP integration is on hold])])
AC_ARG_WITH([qdpxx],AS_HELP_STRING([--with-qdpxx=DIR],[Reserved; QDP integration is on hold]),[AC_MSG_ERROR([QDP integration is on hold])])
AC_ARG_VAR([OPENMP_CXXFLAGS],[Compiler flags enabling OpenMP])
AC_ARG_VAR([OPENMP_LIBS],[OpenMP runtime link flags])
AC_ARG_ENABLE([openmp],AS_HELP_STRING([--enable-openmp],[Require OpenMP (default yes)]),[],[enable_openmp=yes])
AS_IF([test "x$enable_openmp" != xyes],[AC_MSG_ERROR([This reorganization checkpoint requires OpenMP])])
CXXFLAGS="$CXXFLAGS $OPENMP_CXXFLAGS"
LIBS="$OPENMP_LIBS $LIBS"
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <omp.h>]],[[return omp_get_max_threads()<1;]])],[],[AC_MSG_ERROR([Working OpenMP flags/runtime required])])
AC_ARG_WITH([libxml2],AS_HELP_STRING([--with-libxml2=DIR],[libxml2 installation matching the parser]),[XML2_CONFIG="$withval/bin/xml2-config"])
AC_PATH_PROG([XML2_CONFIG],[xml2-config],[],[$PATH])
AS_IF([test -z "$XML2_CONFIG"],[AC_MSG_ERROR([xml2-config is required])])
DATALIB_XML_CFLAGS=`$XML2_CONFIG --cflags`
DATALIB_DEP_LIBS="`$XML2_CONFIG --libs` $OPENMP_LIBS"
AC_SUBST([DATALIB_XML_CFLAGS])
AC_SUBST([DATALIB_DEP_LIBS])
AC_ARG_WITH([filedb],
  AS_HELP_STRING([--with-filedb=DIR],[Enable standalone FILEDB checks and export FILEDB component flags]),
  [],[with_filedb=no])
FILEDB_CXXFLAGS=
FILEDB_LDFLAGS=
FILEDB_LIBS=
AS_IF([test "x$with_filedb" != xno],[
  AS_IF([test "x$with_filedb" = xyes || test -z "$with_filedb"],
    [AC_MSG_ERROR([--with-filedb requires the FILEDB installation prefix])])
  FILEDB_CXXFLAGS="-I$with_filedb/include"
  FILEDB_LDFLAGS="-L$with_filedb/lib"
  FILEDB_LIBS="-lfiledb -lfilehash -lpthread"
  datalib_saved_CPPFLAGS="$CPPFLAGS"
  datalib_saved_LDFLAGS="$LDFLAGS"
  datalib_saved_LIBS="$LIBS"
  CPPFLAGS="$CPPFLAGS $FILEDB_CXXFLAGS"
  LDFLAGS="$LDFLAGS $FILEDB_LDFLAGS"
  LIBS="$FILEDB_LIBS $LIBS"
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <DBString.h>
#include <ffdb_db.h>]],[[FILEDB::StringKey key("probe");
std::string bytes;
key.writeObject(bytes);
return ffdb_dbopen(bytes.c_str(), 0, 0, nullptr) == nullptr;]])],
    [],[AC_MSG_ERROR([Could not link FILEDB and filehash from --with-filedb])])
  CPPFLAGS="$datalib_saved_CPPFLAGS"
  LDFLAGS="$datalib_saved_LDFLAGS"
  LIBS="$datalib_saved_LIBS"
])
AM_CONDITIONAL([WITH_FILEDB],[test "x$with_filedb" != xno])
AC_SUBST([FILEDB_CXXFLAGS])
AC_SUBST([FILEDB_LDFLAGS])
AC_SUBST([FILEDB_LIBS])
AC_CONFIG_FILES([Makefile datalib-config])
AC_CONFIG_COMMANDS([config-executable],[chmod +x datalib-config])
AC_OUTPUT
