#!/bin/sh

# quick pack of an OSX app bundle for TBT
# by Jaromil

if ! [ -r /opt/local/lib/libslang.2.dylib ]; then
	echo "No libslang2 found, you need to install MacPorts and build it"
	echo "# sudo port install slang2 +universal"
	exit 0
fi

cp /opt/local/lib/libslang.2.dylib libslang.2.dylib
install_name_tool -id @executable_path/../Frameworks/libslang.2.dylib libslang.2.dylib 

echo "Compiling universal binary"

if ! [ -r tbt.i386 ]; then
cd ../..
make clean
CXXFLAGS="-arch i386" ./configure
make
cp src/tbt contrib/mac/tbt.i386
cd -
fi

if ! [ -r tbt.x86_64 ]; then
cd ../..
make clean
CXXFLAGS="-arch x86_64" ./configure
make
cp src/tbt contrib/mac/tbt.x86_64
cd -
fi


lipo tbt.i386 tbt.x86_64 -create -output tbt

cp tbt TBT.app/Contents/MacOS

install_name_tool -change \
	/opt/local/lib/libslang.2.dylib \
	@executable_path/../Frameworks/libslang.2.dylib \
	TBT.app/Contents/MacOS/tbt

