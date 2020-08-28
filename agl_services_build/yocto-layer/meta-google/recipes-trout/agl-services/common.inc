require sources.inc

S = "${WORKDIR}/${FETCH_CODE_PREFIX}"

def force_create_symlink(src, dst):
    src = os.path.relpath(src, os.path.dirname(dst))
    tmplink = '{}.templink'.format(dst)
    os.symlink(src, tmplink)
    os.rename(tmplink, dst)

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
            force_create_symlink(src, dst)
        else:
            dst = entry
        os.system('install -m 0755 {} {}'.format(dst, install_dir))

OECMAKE_TARGET_COMPILE = "${@trout_compile_targets(d)}"

inherit pkgconfig cmake

python do_unpack_append() {
    os.chdir(d.getVar('S'))
    linkfile_rules = d.getVar('SRC_SYMLINKS').split()
    for src, dst in map(lambda x: x.split(':'), linkfile_rules):
        force_create_symlink(src, dst)
}

python do_install() {
    trout_do_install(d)
}
