# TODO(chenhaosjtuacm): set it to the correct license, license file is needed
LICENSE = "CLOSED"

# TODO(chenhaosjtuacm): Set to a public-available manifest repository
SRC_URI = "\
    file://google-trout-agl-services.tar.gz \
"

S = "${WORKDIR}/google-trout-agl-services"

def trout_compile_targets(d):
    install_entries = d.getVar('TROUT_target_install').split()
    compile_targets = map(lambda x: x.split(':')[0], install_entries)
    return ' '.join(compile_targets)

def trout_do_install(d):
    install_entries = d.getVar('TROUT_target_install').split()
    os.chdir(d.getVar('B') + '/bin')
    install_dir = '{}{}'.format(d.getVar('D'), d.getVar('bindir'))
    os.system('install -d {}'.format(install_dir))
    for entry in install_entries:
        if ':' in entry:
            src, dst = entry.split(':')
            tmplink = '{}.templink'.format(dst)
            os.symlink(src, tmplink)
            os.rename(tmplink, dst)
        else:
            dst = entry
        os.system('install -m 0755 {} {}'.format(dst, install_dir))

OECMAKE_TARGET_COMPILE = "${@trout_compile_targets(d)}"

inherit pkgconfig cmake

python do_install() {
    trout_do_install(d)
}
