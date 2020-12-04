ARG BASE_IMAGE=ubuntu
ARG BASE_IMAGE_VERSION=focal
ARG DEBIAN_FRONTEND=noninteractive

FROM ${BASE_IMAGE}:${BASE_IMAGE_VERSION}
RUN apt-get update
RUN apt-get install -y build-essential alien cmake dpkg lintian
RUN apt-get install -y qt5-qmake qttools5-dev-tools qtbase5-dev \
    libqt5xmlpatterns5-dev libqt5websockets5-dev zlib1g-dev

ENV QT_SELECT=5

COPY . /build

WORKDIR /build/third-party/quazip/quazip
RUN make clean || true
RUN cmake -S . -DBUILD_SHARED_LIBS:BOOL=OFF
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
