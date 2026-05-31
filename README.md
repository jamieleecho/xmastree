# xmastree

xmastree is a Multi Vue program for designing Christmas trees.

It is also the example app for **MVKit**, a small, AppKit-inspired application
framework for Multi-Vue extracted from this project — see [mvkit/](mvkit/).

## Using xmastree

### Running on MAME

To use xmastree on MAME, you need:

* MAME v0.269 or higher with the coco 3 module.

You can launch MAME with a command like:

```bash
~/Applications/mame/mame coco3 -cfg_directory assets/mame-cfgs -window -rompath ~/Applications/mame/roms -ext:fdc:wd17xx:0 525qd -flop1 build/xmastree.os9
```

Substitue `~/Applications/mame/mame` and `~/Applications/mame/roms` for your MAME
installation and ROM locations.

After MAME starts up, enter the following in the MAME CoCo 3 window:

```text
DOS
```

After Multi Vue is loaded:

1. Press the `/dd` icon.
2. Use the mouse to double click on the `xmastree` icon.

### Running on real hardware

To run on real hardware you need:

* A CoCo 3 with 512KB or more RAM
* An 80 track disk drive connected to drive 0
* Tandy High-Resolution Joystick Interface plugged into the left joystick port
* A mouse plugged into the High-Resolution Joystick interface
* A mechanism to transfer `build/xmastree.os9` to an 80 track disk

To use `xmastree` on a real CoCo 3.

1. First transfer `build/xmastree.os9` to an 80 track disk.
2. Turn on the CoCo 3 and all of its peripherals.
3. Insert the disk into drive 0.
4. Enter `DOS` on the CoCo 3.

After Multi Vue is loaded:

1. Press the `/dd` icon.
2. Use the mouse to double click on the `xmastree` icon.

## Building xmastree

### Requirements

* Unix-like system (tested with MacOS Tahoe)
* Typical Unix tools such as `make` and `git`
* [Docker 29.1 or later](https://docs.docker.com/desktop/)
* [coco-dev](https://github.com/jamieleecho/coco-dev) - this is installed by the Makefile
* [Visual Studio Code](https://code.visualstudio.com/download) recommended

### Getting Started

Let's start a terminal window - we will call this the `coco-dev terminal window`. In
the `coco-dev terminal window` enter the following:

```bash
git clone https://github.com/jamieleecho/xmastree.git
cd xmastree
make
```

This should successfully create `build/xmastree.os9` which you can verify by:

```bash
ls build/xmastree.os9
```

Now let's start another terminal window - we will call this the `local terminal window`.
In the `local terminal window` enter the following:

```bash
make run
```

If you have your MAME installed in `~/Applications/mame` and your roms in
`~/Applications/mame/roms`, a CoCo 3 MAME window should appear with the built disk mounted.

### Useful make commands

* `make all` -- build the xmastree.os9 file.
* `make clean` -- clean out the folder.
* `make help` -- Show this help message.
* `make real-clean` -- clean out the folder including the cmoc_os9 checkout.
* `make run` -- Run the disk image in the MAME CoCo 3 emulator

### Using Visual Studio Code

This repo includes a Visual Studio Code Dev Container that provides the full
build toolchain (the `coco-dev` image). See [these documents](https://code.visualstudio.com/docs/devcontainers/containers) for more information.

## Acknowledgements

### [cmoc](http://sarrazip.com/dev/cmoc.html)

The C files in this project are compiled with the CMOC compiler.

### [LWTOOLS](https://www.lwtools.ca)

LWTOOLS are used to assemble and link the binaries created by this project.

### [nitros9](https://github.com/nitros9project/nitros9)

This repo includes a disk image that is built from a modified build of nitros9.

### [ToolShed](https://github.com/nitros9project/toolshed)

ToolShed is used to build the disk images in this repo.
