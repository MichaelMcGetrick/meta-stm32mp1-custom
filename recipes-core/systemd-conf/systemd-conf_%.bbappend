FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://51-wireless.network \
"

FILES:${PN} += " \
    ${base_libdir}/systemd/network/51-wireless.network \
"

do_install:append() {
    install -d ${D}${base_libdir}/systemd/network
    install -m 0644 ${WORKDIR}/51-wireless.network ${D}${base_libdir}/systemd/network
}
