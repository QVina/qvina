BASE=/opt/homebrew/opt/boost
BOOST_VERSION=1_90
BOOST_INCLUDE = $(BASE)/include
C_PLATFORM=-arch arm64 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -mmacosx-version-min=26.0
GPP=/usr/bin/c++
C_OPTIONS= -O3 -DNDEBUG -march=native -mtune=native
BOOST_LIB_VERSION=

include ../../makefile_common

LDFLAGS += -Wl,-dead_strip
