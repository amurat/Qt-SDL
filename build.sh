cmake -G Xcode \
  -DCMAKE_PREFIX_PATH="$THIRDPARTY/Qt-6.8.4/lib/cmake" \
  -DPNG_PNG_INCLUDE_DIR="$THIRDPARTY/libpng-1.6.49/include" \
  -DPNG_LIBRARY_DEBUG="$THIRDPARTY/libpng-1.6.49/lib/libpng.a" \
  -DPNG_LIBRARY_RELEASE="$THIRDPARTY/libpng-1.6.49/lib/libpng.a" \
  -DPNG_LIBRARY_RELWITHDEBINFO="$THIRDPARTY/libpng-1.6.49/lib/libpng.a" \
  -DPNG_LIBRARY_MINSIZEREL="$THIRDPARTY/libpng-1.6.49/lib/libpng.a" \
  \
  -DJPEG_INCLUDE_DIR="$THIRDPARTY/libjpeg-turbo-2.1.0/include" \
  -DJPEG_LIBRARY_DEBUG="$THIRDPARTY/libjpeg-turbo-2.1.0/lib/libjpeg.a" \
  -DJPEG_LIBRARY_RELEASE="$THIRDPARTY/libjpeg-turbo-2.1.0/lib/libjpeg.a" \
  -DJPEG_LIBRARY_RELWITHDEBINFO="$THIRDPARTY/libjpeg-turbo-2.1.0/lib/libjpeg.a" \
  -DJPEG_LIBRARY_MINSIZEREL="$THIRDPARTY/libjpeg-turbo-2.1.0/lib/libjpeg.a" \
  \
  -S . -B build
