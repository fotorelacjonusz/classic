FROM ubuntu:bionic
RUN apt-get update
RUN apt-get install -y build-essential alien dpkg lintian
RUN apt-get install -y qt5-qmake qttools5-dev-tools qtbase5-dev qtscript5-dev qtwebengine5-dev libqt5xmlpatterns5-dev zlib1g-dev

ENV QT_SELECT=5

COPY . /build

WORKDIR /build/third-party/quazip/quazip
RUN make clean || true
RUN qmake CONFIG+=staticlib CONFIG+=c++11 CONFIG+=release LIBS+=-lz
RUN make

WORKDIR /build/third-party/qtftp/src
RUN make clean || true
RUN qmake CONFIG+=staticlib CONFIG+=c++11 CONFIG+=release
RUN make

WORKDIR /build
RUN make clean || true
RUN lrelease fotorelacjonusz.pro
RUN qmake CONFIG+=release
RUN make
