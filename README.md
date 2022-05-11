# Violet - Lightweight STUN/TURN server

[![License: GPL v2 or later](https://img.shields.io/badge/License-GPL_v2_or_later-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Build](https://github.com/paullouisageneau/violet/actions/workflows/build.yml/badge.svg)](https://github.com/paullouisageneau/violet/actions/workflows/build.yml)
[![Gitter](https://badges.gitter.im/libjuice/violet.svg)](https://gitter.im/libjuice/violet?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
[![Discord](https://img.shields.io/discord/903257095539925006?logo=discord)](https://discord.gg/jXAP8jp3Nn)

Violet is a lightweight STUN/TURN server ([RFC8489](https://www.rfc-editor.org/rfc/rfc8489.html) and [RFC8656](https://www.rfc-editor.org/rfc/rfc8656.html)) written in C without dependencies, based on [libjuice](https://github.com/paullouisageneau/libjuice).

Violet is licensed under GPLv2 or later, see [LICENSE](https://github.com/paullouisageneau/violet/blob/master/LICENSE).

![Oompa-Loompas rolling Violet, from Charlie and the Chocolate Factory](https://github.com/paullouisageneau/violet/blob/master/image.png?raw=true)

> "Mercy! Save us!" yelled Mrs Beauregarde. "[...] Violet, you’re **turn**ing violet, Violet!" [...]
>
> "Squeeze her," said Mr Wonka. "We've got to squeeze the **juice** out of her immediately."
>
> -- Charlie and the Chocolate Factory, Roald Dahl

## Dependencies

None!

## Building

### Clone repository and submodules

```bash
$ git clone https://github.com/paullouisageneau/violet.git
$ cd violet
$ git submodule update --init --recursive
```

### Build with CMake

```bash
$ cmake -B build
$ cd build
$ make -j2
```

## Running

Running the TURN server with default options is as simple as:
```bash
$ ./violet --credentials=USER:PASSWORD
```

Available options can be listed with the `--help` (or `-h`) flag:
```bash
$ ./violet --help
```

## Links

Violet is available as a [package on AUR](https://aur.archlinux.org/packages/violet/).

