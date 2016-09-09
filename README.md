# dbo-get

`dbo-get` is a command-line utility similar to `apt-get`, used to retrieve and manage artifacts ("packages") from
dev.bukkit.org. Projects are uniquely identified via their URL slug (for technical reasons).

## Installation
Coming soon. A Windows installer and `deb` package will be made available on initial release, possibly alongside an
`rpm` package.

## Building
On *nix systems, simply run `make` in the project directory. Note that on Debian, the `libcurl4-openssl-dev` and
`libssh2-1-dev` packages must be installed. The build has not been tested on any other distributions.

On Windows, Visual Studio or MSBuild >=14.0 must be installed. Additionally, `curl.lib` and `ssh2.lib` (and/or their
debug configuration counterparts) must be available on the library search path. To build, simply open the solution in
the IDE and build it, or run `msbuild` from the command line in the project directory.

## Usage
`dbo-get` has four subcommands:

| Command | Description |
| `store` | Sets the current location for storing packages. Each store has its own unique local index.
| `install <projects>...` | Installs the given projects to disk.
| `upgrade` | Attempts to upgrade installed projects.
| `remove <projects>...` | Attempts to remove the given projects from disk.
