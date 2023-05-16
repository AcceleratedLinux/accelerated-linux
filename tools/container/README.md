# Automated build environment setup for DAL
The Digi Accelerated Linux build system requires a Linux host. In order to automate build system installation (and to isolate the build system from the host), the following procedures install the build environment for DAL inside a Docker container.

The Docker container can be installed and used on a wide variety of systems. It has been tested on Ubuntu 20.04LTS. On Windows it can run on an Ubuntu host inside VirtualBox. The only strictly-required component on the build host is Docker (capable of running a Linux container).

On Ubuntu 20.04LTS, install Docker Engine and the latest version of `docker-compose`:
```sh
sudo apt install docker docker.io
sudo curl -L https://github.com/docker/compose/releases/download/1.27.4/docker-compose-`uname -s`-`uname -m` -o /usr/local/bin/docker-compose
```
(This method is used instead of `apt` because the distribution package may be too old.)

Then add your user to the `docker` group, to be able to run docker commands without `sudo`:
```sh
sudo usermod -aG docker $USER
newgrp docker
```

## Preparation
Open the file `docker-compose.yml` in a text editor and set the parameters as needed, including:
* User name: for Git configuration
* User email: for Git configuration
* User ID: should match host UID from `id` command, for SSH key permission
* Group ID: should match host GID from `id` command, for SSH key permission
* Enter the correct path to the user's SSH private key on the host at `secrets:`...`file:`
* For the bind mount whose location inside the container is `target: /home/builder/dal`, enter the path to a corresponding host location in `source:`. Create this directory on the host if it doesn't yet exist.
* By default the DAL `.downloads` directory is mounted as a named volume. This allows it to be shared among different containers, but it is not directly accessible from the host. No further preparation is needed to use the default mount method. But to make `target: /home/builder/.downloads` accessible from the host as a bind mount, change the type from `volume` to `bind` and enter the path to a corresponding host location in `source:`. Create this directory on the host if it doesn't yet exist.
* Toolchains: by default, the DAL toolchains will be downloaded from `DAL_TOOLS_URL` and installed in the image. All the supported toolchains will be downloaded, which may require on the order of hours depending on network conditions. But it's also possible to download and install only one toolchain by completing the path to one particular installer in `DAL_TOOLS_URL`. Alternatively, the toolchains can be obtained separately and copied into the image: to build this way, change `DAL_TOOLS_METHOD` from `download` to `localcopy`, and put the desired toolchain installer(s) into a directory called `toolchains` in the local context path (i.e. alongside the Dockerfile).

## Build the container image
From a terminal or command window, change to this directory (i.e. where `docker-compose.yml` is located) and run:
```sh
docker-compose build dal
```

## Work from host shell
Instead of using an interactive shell inside the container, the provided `env.sh` script makes it easy to run individual commands from the host shell. First install the `dal` command into the host shell environment:
```sh
. env.sh
```
This method creates one-time containers which are removed after the command completes, so it's not necessary to clean them up manually. Also note that without arguments, the `dal` function opens an interactive shell. Upon exit, the created container is removed.

## Install DAL build environment (first time only)
From the host shell:
```sh
dal dal_install
```
Note: the `dal_install` script may appear to hang or fail with network-related errors (during `git` operations) when required repositories are on a server accessible only by VPN. In that case refer to "Docker and VPN tips" below.

## First time build
Run `make` to build the desired product image; for example:
```sh
dal make AcceleratedConcepts/sprite_default
```
(This may take several hours depending on network speed.)

The build artifacts will be created at `~/dal/images` inside the container. They are also available directly on the host, at the corresponding path in the bind-mounted host directory.

## Build again
Just run `make` (without `*_default`) to rebuild after changes:
```sh
dal make
```

## Tips
* To create a persistent container from the image and enter its shell:
```sh
docker-compose run --name <name> dal
```
(Don't miss the name: it will be helpful, as illustrated below...)
* The container's shell may not print anything upon attachment. Just type enter for a new prompt.
* Ubuntu packages can be installed inside the container as per usual; for example vim, emacs, or nano. (Note these changes apply only to the container instance created by the `run` command, not to the image it's based on, nor to any other container instances created from the image. But it's also possible to save a modified image from a running container instance.)
* When creating the container on a VirtualBox host with a VPN connection, it may be helpful to set `network_mode: "host"` in `docker-compose.yml` to prevent connectivity problems.
* The container doesn't have `sudo`. Use `su root` instead.
* To exit the container's shell without stopping it, use the escape sequence Ctrl-P, Ctrl-Q.
* Log back into the container this way:
```sh
docker attach <name>
```
* If the container is stopped, also restart it:
```sh
docker start -ai <name>
```
* Show running and stopped containers:
```sh
docker ps -a
```
* List images:
```sh
docker images
```
* Delete an obsolete or intermediate image:
```sh
docker rmi <name|id>
```
* Delete a container:
```sh
docker rm <name|id>
```
* If for any reason `docker-compose build` doesn't finish successfully, it is possible to shell into the intermediate image this way to troubleshoot:
```sh
docker run -dit --rm <name|id>
```
(The `--rm` option removes the temporary container upon exit.)
* Use the `docker cp` command to copy files into or out of the container, from or to the host, respectively.

## Docker and VPN tips
Even when configured for host networking mode (in `docker-compose.yml`), the container may have difficulty accessing sites on a VPN via the host's network interface (for example when running `git fetch`), or on the other hand it may have difficulty reaching sites outside the VPN. These issues arise because Docker may not automatically propagate the correct DNS servers into the container. To fix, configure the VPN's DNS server manually at `/etc/docker/daemon.json` on the host. For example:
```json
{
    "dns": ["10.10.8.62", "8.8.8.8"]
}
```
Here 10.10.8.62 is the VPN's DNS server and 8.8.8.8 is Google's DNS. The latter is needed as a generic fallback for public Internet sites (outside the VPN). Another common problem is that Docker's IP address pool may overlap with the VPN address space. To identify this problem, use `route -n` on the host (with VPN connected and a container running at the same time) and look carefully at the routes in the 172.0.0.0/8 range. If docker's IP address(es) are too close to the VPN's, Docker can be configured to use an address pool in a different range, for example as follows:
```json
{
    "dns": ["10.10.8.62", "8.8.8.8"],
    "bip": "172.111.0.1/16",
    "default-address-pools": [
        {"base":"172.112.0.0/16","size":24},
        {"base":"172.113.0.0/16","size":24}
    ]
}
```
The IP ranges shown are a suggestion; in general they should be chosen to avoid the VPN routes.

## Notes on management of SSH private key
* Under no circumstances will the contents of the private key *ever* be stored in the final container image.
* The Docker secrets mechanism is used at runtime to propagate the private key into the container. (Currently this is supported without swarm mode only via `docker-compose`.)

## Docker volumes, bind mounts, and secrets
This procedure creates several persistent filesystems in Docker which are mounted into the container but exist separately from the container image:

| Type | Path inside container | Name or default path on host | Description |
| --- | --- | --- | --- | --- |
| Secret | `/run/secrets/private_key` | `private_key` | SSH private key |
| Volume | `~/.downloads` | `dal-downloads` | Location into which packages are fetched |
| Bind | `~/dal` | `~/dal-build` | DAL source code repository |

For the sake of disk usage optimization and convenience, the `.downloads` and secret filesystems can be shared with other containers on the same host. (They are simply referenced by name.)

The DAL repository (`~/dal-build`) is a bind mount, shared with (i.e. mounted directly into) the host filesystem. Editing or Git operations (such as branching or commits) can be performed from the convenience of the host environment.

## Disk usage
The total space required for this build system (assuming all toolchains installed) is about 17 GB, allocated as shown:

| Docker image | Usage |
| --- | --- |
| `ubuntu:18.04` | 63 MB |
| `dal` | 10 GB |

| Volume or bind mount | Usage |
| --- | --- |
| `~/dal` | 7.1 GB |
| `~/.downloads` | 72 MB |

## Image distribution
It's also possible to distribute a fully-prepared build environment. If the goal is for the recipient to be able to rebuild without downloading or fetching _any_ new sources or packages, the following elements must be archived:
* Toolchains: use `localcopy` for `DAL_TOOLS_METHOD`, as described above.
* An archive of the bind-mounted filesystem at `~/dal`.
* An archive of the filesystem in the `~/.downloads` volume.

These elements can be distributed and copied or imported into the recipient's host system first. After which it should be possible for the recipient to follow _exactly the same_ procedure as described above to build and run their own container image. It is not recommended to redistribute the container image itself, because it has been built with user-specific information (although not with the user's SSH key, as discussed above). Instead, they can easily create a new image and rebuild, but without downloading, and in principle without access to any private remote repositories.
