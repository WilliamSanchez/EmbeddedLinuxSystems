SUMMARY = "bitbake-layers recipe"
DESCRIPTION = "Recipe created by bitbake-layers"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302" 

SRC_URI = "file://helloworld.c"


S = "${WORKDIR}"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  Example recipe created by bitbake-layers   *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

do_compile(){
	${CC} ${CFLAGS} ${LDFLAGS} helloworld.c -o helloworld
}

do_install(){
	install -d ${D}${bindir}
	install -m 0755 helloworld ${D}${bindir}
}
