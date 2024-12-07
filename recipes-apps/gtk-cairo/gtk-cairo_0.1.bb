SUMMARY = "Get GTK-CAIRO recipe"
DESCRIPTION = "Custom recipe to build gtk_cairo.c application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Where to find source files (can be local, GitHub, etc.)
SRC_URI = "file://src"

# Where to keep downloaded source files (in tmp/work/...)
S = "${WORKDIR}/src"


DEPENDS = "gtk+3"

inherit pkgconfig


# Cross-compile source code
do_compile() {
    ${CC} ${LDFLAGS} gtk_cairo.c graphic_lx.c mcp3008.c graph.c -lm -o gtk_cairo `pkg-config --cflags --libs gtk+-3.0`
}

# Create /usr/bin in rootfs and copy program to it
do_install() {
    install -d ${D}${bindir}
    install -m 0755 gtk_cairo ${D}${bindir}
}

FILES_${PN} += "${bindir}/gtk_cairo"
