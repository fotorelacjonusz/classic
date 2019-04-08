# This file adds some overrides to main QuaZIP project file.
#
# It sets up some basic settings, ensures that archive library is produced,
# and that all the files are created outside git submodule, leaving it sane.

include(quazip/quazip/quazip.pro)

TARGET = quazip
DESTDIR = ../target

# Build an archive library (for static linking) instead of shared one.
# QuaZIP is pretty small, there is no need for dynamic linking.
# Also, we can use any version we want.
CONFIG += staticlib

# Enable C++11 explicitly, which should make proper stdlib available.
# Required to compile at least on OS X.
CONFIG += c++11

# Link dynamically against zlib.
LIBS += -lz
