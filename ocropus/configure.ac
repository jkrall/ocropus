# Copyright 2008 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz 
# or its licensors, as applicable.
# 
# You may not use this file except under the terms of the accompanying license.
# 
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You may
# obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
# Project: OCRopus - the open source document analysis and OCR system
# File: configure.ac
# Purpose: configuring OCRopus build
# Responsible: kofler
# Reviewer: 
# Primary Repository: http://ocropus.googlecode.com/svn/trunk/
# Web Sites: www.iupr.org, www.dfki.de

DIE() {
    echo "ERROR: $*" 1>&2
    exit 1
}

# -- Prelude --
AC_INIT(ocropus, 0.3, )
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([ocr-utils/ocr-utils.cc])

tesseract_prefix=/usr
AC_ARG_WITH(tesseract,
[
  --with-tesseract=...    the root of a non-default tesseract installation.
                          (default is /usr/)
                          Should be the same directory as was given to
                          tesseract's configure script through --prefix=],
AC_SUBST(tesseract_prefix,$withval))

if [[ "$tesseract_prefix" = "no" ]]; then
    AC_SUBST(notesseract, 1)
else
    AC_ARG_VAR(tessheaders, Location of Tesseract's headers)
    AC_ARG_VAR(tesslibs, Location of Tesseract's libraries)

    if [[ x$tessheaders = x ]]; then
        tessheaders=$tesseract_prefix/include/tesseract
    fi
    AC_CHECK_FILE($tessheaders/baseapi.h,,
            AC_MSG_ERROR([Could not find tesseract! Choose --without-tesseract if you do not want to use it or specify the installation path with --with-tesseract=<path>]))
    if [[ x$tesslibs = x ]]; then
        tesslibs=$tesseract_prefix/lib
    fi
    LDFLAGS="$LDFLAGS -L$tesslibs"
fi
AM_CONDITIONAL([notesseract], [test "$notesseract" = 1])

iulib_prefix=/usr/local
AC_ARG_WITH(iulib,
[  --with-iulib=...        the root of a non-default iulib installation.
                          (default is /usr/local/)
                          Should be the same directory as was given to
                          iulib's configure script through --prefix=],
AC_SUBST(iulib_prefix,$withval) )

AC_ARG_VAR(iulibheaders, Location of iulib's headers)
AC_ARG_VAR(colibheaders, Location of iulib's colib headers)
AC_ARG_VAR(iuliblibs, Location of iulib)

if [[ x$iulibheaders = x ]]; then
    iulibheaders=$iulib_prefix/include
fi
AC_CHECK_FILE($iulibheaders/iulib/imgio.h,,
    AC_MSG_ERROR([please install iulib or specify a non-default location by '--with-iulib=...']))

if [[ x$iuliblibs = x ]]; then
    iuliblibs=$iulib_prefix/lib
fi
LDFLAGS="$LDFLAGS -L$iuliblibs"

AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([float.h malloc.h stdint.h stdlib.h string.h unistd.h wchar.h])

# verify we are on a POSIX system
AC_CHECK_HEADER(unistd.h,,AC_SUBST(noposix, 1))


# --- libz (required) ---
AC_CHECK_LIB(z, compress,,
    AC_MSG_ERROR([no zlib; please install zlib or equivalent]))

# --- libpng (required) ---
AC_CHECK_HEADER(png.h,,
    AC_MSG_ERROR([no png.h; please install libpng12-dev or equivalent]))
AC_CHECK_LIB(png, png_sig_cmp,,
    AC_MSG_ERROR([no png_sig_cmp; please install libpng12-dev or equivalent]))

# --- libjpg (required) ---
AC_CHECK_HEADER(jpeglib.h,,
    AC_MSG_ERROR([no jpeglib.h; please install libjpeg62-dev or equivalent]))
AC_CHECK_LIB(jpeg, jpeg_std_error,,
    AC_MSG_ERROR([no jpeglib.h; please install libjpeg62-dev or equivalent]))

# --- libtiff (required) ---
AC_CHECK_LIB(tiff, TIFFOpen,,
    AC_MSG_ERROR([no TIFFOpen; please install libtiff4-dev or equivalent]))

AC_LANG_CPLUSPLUS

# --- iulib (required) ---
# NB: we can only use functions with C linkage here
AC_CHECK_LIB(iulib, exit,,
    AC_MSG_ERROR([no iulib; please install iulib first (see INSTALL)]))

# --- libpthread (needed by tesseract) ---
AC_CHECK_LIB(pthread, pthread_create,,)

# --- libtesseract (ahouls become optional) ---
if test x$notesseract != x1; then
    AC_CHECK_LIB(tesseract_full, err_exit,, AC_MSG_ERROR([Could not find tesseract! Choose --without-tesseract if you do not want to use it.]))
fi

# --- openFST (optional) ---
AC_ARG_WITH(fst,[  --with-fst           enable OpenFST language modelling],
    [ac_cv_use_fst=$withval], [ac_cv_use_fst=no])
if test x$ac_cv_use_fst != xno; then
    AC_CHECK_LIB(pthread, pthread_create,,AC_SUBST(nofst,1))
    AC_CHECK_HEADER(fst/lib/fst.h,,AC_SUBST(nofst, 1))
    AC_CHECK_LIB(fst, main,,AC_SUBST(nofst, 1))
else
    AC_SUBST(nofst, 1)
fi
AM_CONDITIONAL([nofst], [test "$nofst" = 1])

# ocroscript is now a separate project

# --- leptonica (sometimes installed as 'liblept' sometimes as 'leptonica') ---
AC_ARG_VAR(leptheaders, Location of leptonicas headers)
AC_ARG_WITH(leptonica,[  --without-leptonica        disable use of Leptonica], [use_leptonica=$withval], [use_leptonica=yes])
if test x$use_leptonica != xno; then
    if test x$leptheaders != x; then
        AC_CHECK_HEADER($leptheaders/allheaders.h,,AC_SUBST(leptheaders,no))
    else
        AC_SUBST(leptheaders,no)
    fi
    if test x$use_leptonica != xyes; then
        AC_CHECK_HEADER($use_leptonica/include/liblept/allheaders.h,
            AC_SUBST(leptheaders,$use_leptonica/include/liblept),
            AC_SUBST(leptheaders,no))
    fi
    if test x$leptheaders = xno; then
        AC_CHECK_HEADER(/usr/local/include/leptonica/allheaders.h,AC_SUBST(leptheaders,/usr/local/include/leptonica))
    fi
    if test x$leptheaders = xno; then
        AC_CHECK_HEADER(/usr/local/include/liblept/allheaders.h,AC_SUBST(leptheaders,/usr/local/include/liblept))
    fi
    if test x$leptheaders = xno; then
        AC_CHECK_HEADER(/usr/include/leptonica/allheaders.h,AC_SUBST(leptheaders,/usr/include/leptonica))
    fi
    if test x$leptheaders = xno; then
        AC_CHECK_HEADER(/usr/include/liblept/allheaders.h,AC_SUBST(leptheaders,/usr/include/liblept))
    fi
    if test x$leptheaders = xno; then
        AC_SUBST(use_leptonica, no)
        AC_MSG_ERROR([Could not find headers for leptonica! You can set the path with 'leptheaders=pathtoheaders'. Choose --without-leptonica if you do not want to use it.])
    fi
    # ??? how to set the path correctly ?
    LDFLAGS="$LDFLAGS -L$leptheaders/../../lib"
    AC_CHECK_LIB(lept,pixCreate,,AC_MSG_ERROR([leptonica not found! Choose --without-leptonica if you don't want to use it.]))
fi
AM_CONDITIONAL([use_leptonica], [test x$use_leptonica == xyes])

# --- GSL (optional for glinerec) ---
AC_SUBST(use_gsl, "yes")
AC_CHECK_LIB(gslcblas, abort,,AC_SUBST(use_gsl, "no"))
AC_CHECK_LIB(gsl, gsl_error,,AC_SUBST(use_gsl, "no"))
AC_CHECK_LIB(blas, exit,,AC_SUBST(use_gsl, "no"))
AM_CONDITIONAL([use_gsl], [test x$use_gsl == xyes])


# --- SDL (optional for graphical debugging) ---
AC_ARG_WITH(SDL,[  --without-SDL           disable SDL (graphical debugging)],
    [ac_cv_use_sdl=$withval], [ac_cv_use_sdl=yes])
if test x$ac_cv_use_sdl != xno; then
    AC_CHECK_LIB(SDL,SDL_Init,,AC_SUBST(nosdl,1))
    AC_CHECK_LIB(SDL_gfx,rotozoomSurface,,AC_SUBST(nosdl,1))
else
    AC_SUBST(nosdl, 1)
fi
AM_CONDITIONAL([nosdl], [test "$nosdl" = 1])


# --- OpenMP (optional for multi threading) ---
# -- AC_OPENMP macro is new in autoconf 2.62 --
#AC_OPENMP
#CXXFLAGS="$OPENMP_CXXFLAGS $CXXFLAGS"
#AC_SUBST(OPENMP_CXXFLAGS)
AC_ARG_WITH(OpenMP,[  --without-OpenMP           disable OpenMP (Multi-Processing)],
    [ac_cv_use_openmp=$withval], [ac_cv_use_openmp=yes])
if test x$ac_cv_use_openmp != xno; then
    AC_CHECK_LIB(gomp,GOMP_parallel_start,,AC_SUBST(noopenmp,1))
else
    AC_SUBST(noopenmp, 1)
fi
if  [[ "$noopenmp" != "1" ]]; then
	LDFLAGS="$LDFLAGS -lgomp"
	CXXFLAGS="$CXXFLAGS -fopenmp"
fi
AM_CONDITIONAL([noopenmp], [test "$noopenmp" = 1])


AC_TYPE_INT64_T
AC_TYPE_PID_T

AC_FUNC_FORK
AC_CHECK_FUNCS([dup2 memset sqrt strchr strdup strrchr])

# enable -Wl,-E if possible (needed to load Lua libraries dynamically)
OLDLDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS -Wl,-E"
AC_TRY_LINK([],[],[],[LDFLAGS="$OLDLDFLAGS"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo
echo "OK! You can build and install OCRopus the usual way:"
echo 
echo "    make"
echo "    sudo make install"
echo
echo
echo "To recognize something, you can use"
echo
echo "    ocroscript recognize data/pages/alice_1.png"
echo
