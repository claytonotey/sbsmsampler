AC_INIT(sbsms,1.7.1)

#------------------------------------------------------------------------------------
# Library's "official" release version:

LIBSBSMS_RELEASE_MAJOR=1
LIBSBSMS_RELEASE_MINOR=7
LIBSBSMS_RELEASE_BUILD=1
AC_CONFIG_SRCDIR([src/sbsms.cpp])
AC_CANONICAL_TARGET([])
AC_CONFIG_HEADER(src/config.h)
m4_include([m4/ac_c99_func_lrint.m4])
m4_include([m4/ac_c99_func_lrintf.m4])
AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_SHARED dnl allows for optimizations

#------------------------------------------------------------------------------------
# The following is the libtool / shared library version. This doesn't have to
# do anything with the release version. It MUST conform to the following rules:
#
#  1. Start with version information of `0:0:0' for each libtool library.
#  2. Update the version information only immediately before a public release of
#     your software. More frequent updates are unnecessary, and only guarantee
#     that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update, then
#     increment revision (`c:r:a' becomes `c:r+1:a').
#  4. If any interfaces have been added, removed, or changed since the last update,
#     increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then increment
#     age.
#  6. If any interfaces have been removed since the last public release, then set age
#     to 0.

SHARED_VERSION_INFO="8:1:7"
LT_INIT

AC_LANG([C++])

AC_LANG_CPLUSPLUS
AC_PROG_CXX

AC_CHECK_FUNCS(malloc calloc free memcpy)

AC_CHECK_LIB([m],floor)
AC_CHECK_FUNCS(floor sqrt log)

AC_C99_FUNC_LRINT()
AC_C99_FUNC_LRINTF()

AC_SUBST(SHLIB_VERSION_ARG)
AC_SUBST(SHARED_VERSION_INFO)

AC_CHECK_SIZEOF(short,2)
AC_CHECK_SIZEOF(int,4)
AC_CHECK_SIZEOF(long,4)
AC_CHECK_SIZEOF(long long,8)

SBSMS_EXTRA_OBJ=""
SBSMS_BIN=""

AC_ARG_ENABLE(universal_binary,[  --enable-universal_binary enable universal binary build: (default: disable)],[enable_universal_binary=$enableval],[enable_universal_binary=no])

AC_ARG_ENABLE(static,[  --enable-static enable static build: (default: disable)],[enable_static=$enableval],[enable_static=no])

AC_ARG_ENABLE(multithreaded,[  --enable-multithreaded enable multithreaded build: (default: disable)],[enable_multithreaded=$enableval],[enable_multithreaded=no])

AC_ARG_ENABLE(debug,[  --enable-debug enable debug build: (default: disable)],[enable_debug=$enableval],[enable_debug=no])

AC_ARG_ENABLE(sndfile,[  --enable-sndfile enable sndfile support: (default: disable)],[enable_sndfile=$enableval],[enable_sndfile=no])

AC_ARG_ENABLE(mp3,[  --enable-mp3 enable mp3 support (default: disable)],[enable_mp3=$enableval],[enable_mp3=no])

AC_ARG_ENABLE(portaudio,[  --enable-portaudio enable portaudio support: (default: disable)],[enable_portaudio=$enableval],[enable_portaudio=no])

AC_ARG_ENABLE(wx,[  --enable-wx enable wxWidgets support (default: disable)],[enable_wx=$enableval],[enable_wx=no])

AC_ARG_ENABLE(vst,[  --enable-vst enable VST support (default: disable)],[enable_vst=$enableval],[enable_vst=no])

AC_ARG_ENABLE(sse, [AC_HELP_STRING([--enable-sse],[enable SSE optimizations])], enable_sse=$enableval, enable_sse=yes)

AC_ARG_ENABLE(programs,[  --enable-programs Build SBSMS programs as well as library (default: enable)],[enable_progs=$enableval],[enable_progs=yes])


if test x$enable_mp3 = xyes; then
   enable_sndfile=yes;
   PKG_CHECK_MODULES(MAD, mad, [], [AC_CHECK_LIB(mad, mad_decoder_init, [ MAD_CFLAGS=""; mad_libs="-lmad" ], [ AC_MSG_ERROR(You need to have libmad installed for mp3 support.)])])
   if test "x$MAD_LIBS" = "x"; then
	    MAD_LIBS=$mad_libs
   fi
   AC_SUBST(MAD_LIBS)
   AC_DEFINE(HAVE_MAD,1,[Define to use libmad])
fi

AM_CONDITIONAL(ENABLE_MP3, test x$enable_mp3 = xyes)

PKG_CHECK_MODULES(SNDFILE, sndfile >= 1.0.2, ac_cv_sndfile=1, ac_cv_sndfile=0)

if test x$enable_sndfile = xyes; then
   # check for presence of libsndfile
   if test "$ac_cv_sndfile" = "0"; then
      echo "Sorry, you need libsndfile to --enable-sndfile.  Install libsndfile or --disable-sndfile"
      exit -1;
   else
      AC_SUBST(SNDFILE_CFLAGS)
      AC_SUBST(SNDFILE_LIBS)
      AC_DEFINE(HAVE_SNDFILE,1,[Define to use libsndfile])
   fi
else
    SNDFILE_LIBS=""
fi
AM_CONDITIONAL(ENABLE_SNDFILE, test x$enable_sndfile = xyes)

PKG_CHECK_MODULES(PORTAUDIO, portaudio-2.0 >= 0.0.19, ac_cv_portaudio=1, ac_cv_portaudio=0)

if test x$enable_portaudio = xyes; then
   # check for presence of libportaudio
   if test "$ac_cv_portaudio" = "0"; then
      echo "Sorry, you need libportaudio-2.0 to --enable-portaudio.  Install libportaudio or --disable-portaudio"
      exit -1;
   else
      AC_SUBST(PORTAUDIO_CFLAGS)
      AC_SUBST(PORTAUDIO_LIBS)
      AC_DEFINE(HAVE_PORTAUDIO,1,[Define to use libportaudio])
   fi
else
    PORTAUDIO_LIBS=""
fi
AM_CONDITIONAL(ENABLE_PORTAUDIO, test x$enable_portaudio = xyes)


CXXFLAGS=`echo $CXXFLAGS | sed -e "s/-g//" -e "s/-O2//"`

if test x$enable_debug = xyes; then
   CXXFLAGS="$CXXFLAGS -g -O3"
else
   CXXFLAGS="$CXXFLAGS -O3 -fomit-frame-pointer"
fi

if test x$enable_static = xyes; then
   CXXFLAGS="$CXXFLAGS"
else
   case "$target_os" in
     darwin*)
	CXXFLAGS="$CXXFLAGS -mdynamic-no-pic"
	;;
	*)
	;;
   esac
fi

CXXFLAGS="$CXXFLAGS -mmacosx-version-min=10.5 -isysroot /src/MacOSX-SDKs/MacOSX10.5.sdk"
#CXXFLAGS="$CXXFLAGS -mmacosx-version-min=10.6 -isysroot /src/MacOSX-SDKs/MacOSX10.6.sdk"

case "$target_os" in
     darwin*)
	macosx="yes"
	;;
     *)
	macosx="no"
	;;
esac

AC_SUBST(SBSMS_BIN)
AM_CONDITIONAL(MACOSX, test "$macosx" = "yes")


if test x$enable_sse = xyes; then

   if test "${ax_cv_cxx_compiler_vendor}" = "gnu"; then
      AX_CHECK_COMPILER_FLAGS(-msse, [SBSMS_CFLAGS="$SBSMS_CFLAGS -msse"],[echo "Need a version of gcc with -msse"; exit 1;])
   fi

   AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <xmmintrin.h>]], [[__m128 v,w; float p[8]; v = _mm_loadu_ps(p); _mm_storeu_ps(p,v); w = _mm_loadl_pi(w,(const __m64*)p); w = _mm_loadh_pi(w,(const __m64*)p); _mm_storel_pi((__m64 *)p, w); _mm_storeh_pi((__m64 *)p, w); v = _mm_add_ps(v,w); v = _mm_sub_ps(v,w); v = _mm_mul_ps(v,w); v = _mm_shuffle_ps(v,w,_MM_SHUFFLE(0,1,2,3)); w = _mm_set1_ps(0.0f);]])], [sse_ok=yes], [sse_ok=no])

   if test x$sse_ok = xyes; then
      AC_DEFINE(ENABLE_SSE,1,[Define to enable sse])
   fi
fi

if test x$enable_progs = xyes; then
	# we would like programs, so work out which ones can be built
	# sbsms can always be built (doesn't need optional libs)
	SBSMS_PROGS="sbsms"
	if test x$enable_vst = xyes; then
     SBSMS_PROGS="$SBSMS_PROGS sbsmsampler"
  fi
	if test x$enable_portaudio = xyes; then
		# portaudio available, build sbsmsplay
		SBSMS_PROGS="$SBSMS_PROGS sbsmsplay"
		if test x$enable_wx = xyes; then
			# wx available, build wxsbsmsplayer
			SBSMS_PROGS="$SBSMS_PROGS wxsbsmsplayer"
		fi
	fi
else
	# no progs please
	SBSMS_PROGS=""
fi
SBSMS_PROGS="sbsmsampler sbsms"
AC_SUBST(SBSMS_PROGS)

if test x$enable_vst = xyes; then
   AC_SUBST(VST_CFLAGS)
   AC_SUBST(VST_LIBS)
   AC_SUBST(VSTSDK_DIR)
   AC_SUBST(VSTSDKROOT_DIR)
   AC_SUBST(VSTGUI_DIR)
   AC_DEFINE(HAVE_VST,1,[Define to use libvst])
fi

AM_CONDITIONAL(ENABLE_VST, test x$enable_vst = xyes)

if test x$enable_multithreaded = xyes; then
    AC_DEFINE(MULTITHREADED,1,[Define to compile multithreaded sbsms])
fi

AM_INIT_AUTOMAKE([subdir-objects])

AC_OUTPUT(Makefile src/Makefile libsbsms.spec sbsms.pc)
