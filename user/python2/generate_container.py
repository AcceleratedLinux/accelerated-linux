"""
Python 2.7 script to generate the LXC container rootfs for the
Python compatibility container (intended to provide Python API compatibility
between DAL and code written for legacy Digi devices).
"""

from __future__ import print_function
import argparse
import io
import os
import tarfile


ETC_GROUP = """\
root:x:0:root
dal:x:65535:dal
"""

ETC_PASSWD = """\
root:x:0:0:root:/:/bin/sh
dal:x:65534:65534:dal:/:/bin/sh
"""

ETC_PROFILE = """\
export PS1="lxc # "
export PATH=/usr/local/python2/bin:$PATH
"""

ETC_RESOLV_CONF = """\
# Default assumption is that the DAL host is reachable at 192.168.254.1
nameserver 192.168.254.1
"""

ETC_DAL_BUILD_TXT = os.environ.get('VERSIONSTR', 'unknown')


def fixup(tarinfo):
    tarinfo.uid = tarinfo.gid = 165536
    tarinfo.uname = tarinfo.gname = ''

    # Configuring with --without-ensurepip does not actually exclude the
    # package from the build output. So we must ignore it.
    if tarinfo.type == tarfile.DIRTYPE and tarinfo.name.endswith('ensurepip'):
        return None
    # Leave out the include/ directory.
    if tarinfo.type == tarfile.DIRTYPE and tarinfo.name.endswith('/include'):
        return None
    # To align with the DBL build, we will exclude .py files.
    if tarinfo.name.endswith('.py'):
        return None
    # Don't include any .exe files (distutils/command/wininst*.exe)
    if tarinfo.name.endswith('.exe'):
        return None
    # Don't include libpythonX.X.a.
    if tarinfo.name.endswith('.a'):
        return None

    # Exclude everything in the lib/pythonX.X/config directory except for
    # Makefile.
    if '/config/' in tarinfo.name and tarinfo.type == tarfile.REGTYPE:
        path_parts = tarinfo.name.split(os.sep)
        if (
            len(path_parts) > 3  # should never not be the case
            # .../lib/pythonX.X/config/___
            and path_parts[-4] == 'lib'
            and path_parts[-2] == 'config'
            and path_parts[-1] != 'Makefile'
        ):
            return None

    return tarinfo


def main(output_file, python_build):
    with tarfile.open(output_file, 'w:gz') as tar:
        tar.add(
            python_build,
            arcname='rootfs',
            recursive=True,
            filter=fixup,
        )

        # Create baseline root directory entries for the container
        directories = (
            'bin', 'dev', 'etc', 'lib', 'sys', 'var',
            'usr', 'proc', 'sbin', 'opt',
        )
        for d in directories:
            ti = tarfile.TarInfo('rootfs/' + d)
            ti.type = tarfile.DIRTYPE
            ti.mode = 0o755
            fixup(ti)
            tar.addfile(ti)

        # TODO: Copy any other system libraries into the container,
        # such as libc, libncurses, etc?
        # Not necessary if we rely on the "Clone DAL" option.

        # Generate baseline files for /etc.
        for name, contents in (
            ('etc/group', ETC_GROUP),
            ('etc/passwd', ETC_PASSWD),
            # NOTE: This only gets used if you run something like
            # lxc python_2_7_18 -p sh -l
            ('etc/profile', ETC_PROFILE),
            ('etc/resolv.conf', ETC_RESOLV_CONF),
            # Record which DAL build this container came from.
            ('etc/dal-build.txt', ETC_DAL_BUILD_TXT),
        ):
            ti = tarfile.TarInfo('rootfs/' + name)
            ti.type = tarfile.REGTYPE
            ti.mode = 0o644
            ti.size = len(contents)
            fixup(ti)
            tar.addfile(ti, io.BytesIO(contents))

        # For utmost compatibility and convenience, make extra symlinks
        # to the Python 2.7 interpreter to various locations that
        # a user might expect them to be located at.
        #
        # NOTE:
        # This also makes 'lxc python_2_7_lxc -p python' work properly,
        # as 'lxc-init' has a search path of:
        # '/usr/local/bin:/bin:/usr/bin'
        # When PATH is not defined.
        #
        # NOTE2: When DAL-7620 is accepted and implemented,
        # these extra symlinks could go away, as we would then
        # be able to provide the PATH to the lxc environment
        # by doing the following:
        # lxc.environment = PATH=/usr/local/python2/bin:/sbin:/usr/sbin:/bin:/usr/bin
        for target, link_location in (
            ('../python2/bin/python2.7', 'usr/local/bin/'),
            ('../local/python2/bin/python2.7', 'usr/sbin/'),
        ):
            for link_name in ('python', 'python2', 'python2.7'):
                ti = tarfile.TarInfo('rootfs/' + link_location + link_name)
                ti.type = tarfile.SYMTYPE
                ti.linkname = target
                ti.size = 0
                fixup(ti)
                tar.addfile(ti)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-o', '--output-file',
        required=True,
        type=str,
        help='.tgz filename to create',
    )
    parser.add_argument(
        '--python-build',
        required=True,
        type=str,
        help='Directory path to build/*-install directory',
    )
    args = parser.parse_args()

    if not os.path.isdir(args.python_build):
        parser.error(
            "--python-build: {}: Not a directory".format(args.python_build)
        )

    main(
        output_file=args.output_file,
        python_build=args.python_build,
    )
