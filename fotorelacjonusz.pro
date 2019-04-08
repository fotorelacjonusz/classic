TEMPLATE = subdirs

SUBDIRS = \
    application \
    quazip

application.depends = quazip

quazip.file = third-party/fotorelacjonusz_quazip.pro
