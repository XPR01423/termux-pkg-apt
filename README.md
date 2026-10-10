# pkg

**A lightweight APT wrapper written in C, featuring Termux-style command shortcuts for Debian-based Linux distributions.**

## Overview

`pkg` is a small command-line utility that wraps APT and DPKG commands with convenient shortcuts inspired by Termux's `pkg` command interface.

It is designed for Debian-based systems, including Debian and Ubuntu, while keeping its implementation simple and lightweight.

## Features

- Written in C
- Termux-style command shortcuts
- APT and DPKG integration
- Static compilation support
- No shell required to execute package manager commands
- Simple command-line interface
- GPL-3.0-or-later licensed

## Supported Commands

| Command | Description |
|---|---|
| `pkg update` | Update package lists |
| `pkg upgrade` | Update package lists and upgrade installed packages |
| `pkg install <package>` | Install packages |
| `pkg remove <package>` | Remove packages |
| `pkg reinstall <package>` | Reinstall packages |
| `pkg search <query>` | Search for packages |
| `pkg show <package>` | Display package information |
| `pkg files <package>` | List files installed by a package |
| `pkg clean` | Clear the package cache |
| `pkg autoclean` | Remove obsolete cached package files |
| `pkg autoremove` | Remove unused packages |
| `pkg list-all` | List available packages |
| `pkg list-installed` | List installed packages |

## Command Shortcuts

The program supports command shortcuts inspired by Termux's `pkg` interface. Shortcuts are intentionally omitted from the help menu.

Examples:

```bash
pkg upd
pkg up
pkg i curl
pkg se curl
pkg sh curl
pkg rei curl
pkg un curl
pkg f curl
pkg autoc
pkg cl
pkg list-a
pkg list-i
```

## Requirements

- Debian-based Linux distribution
- GCC or another compatible C compiler for building from source
- APT and DPKG
- Appropriate privileges to perform system package management operations

## Build

Clone the repository:

```bash
git clone https://github.com/USERNAME/REPOSITORY.git
cd REPOSITORY
```

Compile a statically linked executable:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic \
    -static pkg.c -o pkg
```

Run the help menu:

```bash
./pkg help
```

Check the executable:

```bash
file ./pkg
```

Static linking depends on the availability of the required static libraries on the build system.

## Installation

To install the executable locally:

```bash
sudo install -Dm755 pkg /usr/local/bin/pkg
```

After installation, run:

```bash
pkg help
```

**Note:** The executable name `pkg` may conflict with an existing command on some systems. Check for conflicts before installing it system-wide.

## Permissions

This program does not invoke `sudo` internally. Run commands requiring administrative privileges with appropriate permissions, for example:

```bash
sudo pkg upd
sudo pkg up
sudo pkg i curl
```

## License

This project is licensed under the GNU General Public License v3.0 or, at your option, any later version. See [LICENSE](LICENSE) for details.
