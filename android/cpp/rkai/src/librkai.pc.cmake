prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}/@CMAKE_INSTALL_BINDIR@
libdir=${prefix}/@CMAKE_INSTALL_LIBDIR@
includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@

Name: librkai
Description: Rikkei AI interface library
Version: @LIBRARY_VERSION@
Libs: -L${libdir} -lrkai
Libs.private: -L${libdir} -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lngt
Cflags: -I${includedir}
