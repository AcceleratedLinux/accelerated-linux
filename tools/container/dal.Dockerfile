ARG DAL_TOOLS_METHOD=download

FROM ubuntu:18.04 as dal_base

MAINTAINER Michael Burr <michael.burr@digi.com>

# Install host packages
RUN dpkg --add-architecture i386 \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential curl doxygen libncurses5-dev xutils-dev pkg-config \
    gettext lzma-dev liblzma-dev flex bison autoconf libtool bin86 \
    zlib1g-dev genisoimage genext2fs tcl libssl-dev cmake \
    libdbus-glib-1-dev gtk-doc-tools autogen lynx u-boot-tools byacc \
    haserl ruby-sass git iasl libc6-i386 libc6-dev-i386 genisoimage \
    syslinux lib32stdc++6 liblzma-dev:i386 zlib1g:i386 gawk intltool \
    autopoint bc libelf-dev python-pip python-m2crypto syslinux-utils \
    ant device-tree-compiler scons python3-pip bmap-tools gengetopt \
    python3-venv gperf net-tools wget cpio kmod libncurses5:i386 expat:i386 \
    groff-base rsync autoconf-archive libcap2-bin
RUN pip install attrs future

# Download DAL toolchains
FROM dal_base as dal_base_download
ONBUILD ARG DAL_TOOLS_URL=http://eng.digi.com/builds/DAL/tools
ONBUILD RUN wget -r -np -nd "$DAL_TOOLS_URL" \
        --wait 2 --random-wait \
        --accept '*-linux*-tools-*.sh' \
        -P toolchains

# Copy DAL toolchains from local context
FROM dal_base as dal_base_localcopy
ONBUILD RUN mkdir toolchains
ONBUILD COPY toolchains/*-linux*-tools-*.sh toolchains/

FROM dal_base_$DAL_TOOLS_METHOD as dal

# Install all toolchains
RUN for installer in toolchains/*.sh; \
    do \
        echo "Installing: $installer..."; \
        chmod +x $installer; \
        yes "" | $installer && printf '\n'; \
    done

# Create link to newest-version toolchain for each prefix, but also allow
# older versions to be chosen via update-alternatives
RUN cd toolchains \
    && prefixes=$(ls -1 *-linux*-tools-*.sh \
        | sed -E 's;-tools-[0-9]{8}.sh$;;' | sort | uniq) \
    && for prefix in $prefixes; \
    do \
        echo "Prefix: $prefix"; \
        i=0; for installer in $(ls -1 $prefix-tools* | sort); \
        do \
            toolchain=$(basename "$installer" .sh); \
            path="$(ls -d /usr/local/*/"$toolchain"/*/bin)"; \
            path="${path%/bin}"; \
            echo "Priority: $i: $path"; \
            test -d "$path" \
            && update-alternatives --install /usr/local/"$prefix" \
                "$prefix" "$path" $i; \
            i=$(( i + 1 )); \
        done; \
    done
RUN rm -r toolchains

# Create firmware deployment location
RUN install -m 0777 -d /tftpboot

# Prepare runtime entrypoint script
COPY entry.sh /usr/local/bin/entry
RUN chmod +x /usr/local/bin/entry

# Prepare installation script
COPY dal_install.sh /usr/local/bin/dal_install
RUN chmod a+x /usr/local/bin/dal_install

# Set default root password, so that user can su
RUN echo root:root | chpasswd

# Set up a user account (this allows runtime access to the host user's SSH key)
ARG USER=builder
ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID $USER \
    && useradd -m -u $UID -g $GID $USER

# Give up root permissions
USER $USER
WORKDIR /home/$USER

# Git configuration
ARG USER_NAME="First Last"
ARG USER_EMAIL="someone@somewhere"
RUN git config --global user.name "$USER_NAME" \
    && git config --global user.email $USER_EMAIL \
    && git config --global push.default simple

# Make directories for DAL downloads, sources and build
ARG DAL_INSTALL_PATH=/home/$USER/dal
RUN install -d .downloads "$DAL_INSTALL_PATH"

ENTRYPOINT [ "/usr/local/bin/entry" ]
CMD [ "/bin/bash" ]
