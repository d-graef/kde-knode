#! /bin/sh
$EXTRACTRC */*.ui >> rc.cpp || exit 11
$XGETTEXT *.cpp kleo/*.cpp kleo/*.h ui/*.cpp ui/*.h \
	backends/*/*.cpp backends/*/*.h -o $podir/libkleopatra.pot
