inherit mender-licensing

SUB_FOLDER:arm = "arm"
SUB_FOLDER:aarch64 = "aarch64"
SUB_FOLDER:x86-64 = "x86_64"

COMPATIBLE_HOSTS = "arm|aarch64|x86_64"

# "lsb-ld" is needed because Yocto by default does not provide a cross platform
# dynamic linker. On x86_64 this manifests as a missing
# `/lib64/ld-linux-x86-64.so.2`
RDEPENDS:${PN} = "lsb-ld xz liblzma"

FILES:${PN} = " \
    ${sysconfdir}/mender/mender-binary-delta.conf \
    ${datadir}/mender/modules/v3/mender-binary-delta \
"

INSANE_SKIP:${PN} = "already-stripped"

do_version_check() {
    if [ ! -e ${S}/${SUB_FOLDER}/mender-binary-delta ]; then
        bbfatal "No mender-binary-delta binary found. Have you added the package to SRC_URI?"
    fi

    if ! ${@'true' if d.getVar('MENDER_DEVMODE') else 'false'}; then
        if ! strings ${S}/${SUB_FOLDER}/mender-binary-delta | fgrep -q "${PN} ${PV}"; then
            bbfatal "String '${PN} ${PV}' not found in binary. Is it the correct version? Check with --version. Possible candidates: $(strings ${S}/${SUB_FOLDER}/mender-binary-delta | grep '${PN} [a-f0-9]')"
        fi
    fi
}
addtask do_version_check after do_patch before do_install

do_configure() {
    # We need the sed operation because bitbake misinterprets the JSON closing
    # brace as end of function, unless we put spaces in front of it.
    sed -e 's/^ //' <<EOF | cat > ${WORKDIR}/mender-binary-delta.conf
 {
   "RootfsPartA": "${MENDER_ROOTFS_PART_A}",
   "RootfsPartB": "${MENDER_ROOTFS_PART_B}"
 }
EOF
}

do_install() {
    install -d -m 755 ${D}${datadir}/mender/modules/v3
    install -m 755 ${S}/${SUB_FOLDER}/mender-binary-delta ${D}${datadir}/mender/modules/v3/mender-binary-delta

    install -d -m 755 ${D}${sysconfdir}/mender
    install -m 644 ${WORKDIR}/mender-binary-delta.conf ${D}${sysconfdir}/mender/mender-binary-delta.conf
}
