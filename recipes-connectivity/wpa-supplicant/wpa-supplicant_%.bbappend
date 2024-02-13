FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://wpa_supplicant-wlan0.conf"
SRC_URI += "file://51-wireless.network"


FILES:${PN} += " \
    ${base_libdir}/systemd/network/51-wireless.network \
"

do_install:append() {
    install -d ${D}${base_libdir}/systemd/network
    install -m 0644 ${WORKDIR}/51-wireless.network ${D}${base_libdir}/systemd/network
}

#SYSTEMD_AUTO_ENABLE = "enable"
SYSTEMD_SERVICE:${PN}:append = " wpa_supplicant@wlan0.service  "


do_install:append () {
   install -d ${D}${sysconfdir}/wpa_supplicant/
   install -D -m 600 ${WORKDIR}/wpa_supplicant-wlan0.conf ${D}${sysconfdir}/wpa_supplicant/
   
   install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants/
   ln -s ${systemd_unitdir}/system/wpa_supplicant@.service ${D}${sysconfdir}/systemd/system/multi-user.target.wants/wpa_supplicant@wlan0.service 


}
