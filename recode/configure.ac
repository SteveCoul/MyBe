AC_INIT(recode,1.1,stevecoul@icloud.com)
AM_INIT_AUTOMAKE()

AC_PROG_CXX()

AC_CHECK_HEADERS(libavutil/imgutils.h,[], AC_MSG_ERROR([Need ffmpeg setup]),[] )
AC_CHECK_LIB(avformat,avformat_close_input,[], AC_MSG_ERROR([Need ffmpeg libs setup]), [] )
AC_CHECK_LIB(avcodec,avcodec_decode_video2,[], AC_MSG_ERROR([avcodec lib missing on LDFLAGS]), [] )

AC_OUTPUT(Makefile)

