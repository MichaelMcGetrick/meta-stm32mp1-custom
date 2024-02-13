SUMMARY = "Test gtk application"
DESCRIPTION = "Custom recipe to test simple gtk application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Where to find source files (can be local, GitHub, etc.)
SRC_URI = "file://src"

# Where to keep downloaded source files (in tmp/work/...)
S = "${WORKDIR}/src"

DEPENDS = "gtk+3"

inherit pkgconfig



do_compile() {
	${CC} ${LDFLAGS} gtk_hello_world.c -o gtk_hello_world `pkg-config --cflags --libs gtk+-3.0`
}

do_install() {
	install -d ${D}${bindir}
	install -m 0755 gtk_hello_world ${D}${bindir}
}

FILES_${PN} += "${bindir}/gtk_hello_world"
