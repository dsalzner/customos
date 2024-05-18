# Custom OS

Writing a Custom Operating System.

Current Features:

* 32-bit unprotected (Ring 0) basic operating system
* boots with Grub and runs on emulators such as QEmu
* handles keyboard input via interrupts
* can read and write to fat32 disk through a custom built lean PCI- and IDE-driver and the fatfs filesystem libraries
* color graphics with 320x200 pixel and 256-bit colour
* a terminal/shell written in Rust that supports a small command set: list directory (```ls <path>```), read file (```rf <filename>```), ...

![CustomOS showing a terminal and reading/writing to disk](img/terminal-readfile.gif)

For technical information see:

* [Pt1-CrossCompiler](https://www.dennissalzner.de/programming/2021/12/11/Sa-CustomOsPt1-CrossCompiler.html)
* [Pt2-Kernel](https://www.dennissalzner.de/programming/2021/12/15/Mi-CustomOsPt2-Kernel.html)
* [Pt3-KeyboardPolling](https://www.dennissalzner.de/programming/2021/12/16/Do-CustomOsPt3-KeyboardPolling.html)
* [Pt4-Interrupts](https://www.dennissalzner.de/programming/2021/12/20/Di-CustomOsPt4-Interrupts.html)
* [Pt5-DiskIo](https://www.dennissalzner.de/programming/2023/08/16/Mi-CustomOsPt5-DiskIo.html)
* [Pt6-Graphics](https://www.dennissalzner.de/programming/2024/01/27/Sa-CustomOsPt6-Graphics.html)
* [Pt7-Shell-Rust](https://www.dennissalzner.de/programming/2024/01/28/So-CustomOsPt7-Shell-Rust.html)
* [Pt8-Terminal](https://www.dennissalzner.de/programming/2024/02/04/So-CustomOsPt8-Terminal.html)
* [Pt9-Enhancements](https://www.dennissalzner.de/programming/2024/02/04/Sa-CustomOsPt9-Enhancements.html)

## Dependencies

The code is inspired by many other public domain CustomOS GitHub repos cited in the blog posts and most notably the guides and examples from the osdev-forums and wiki [1]

1] https://wiki.osdev.org

## Contributing & Compiling from source

The code can be compiled from source on Linux using the Makefiles. 

For this you will need to first build the custom compile chain by running ``./Pt1-CrossCompiler/gccbuild.sh```.

The ```gccbuild.sh``` script and Makefiles have been tested under Ubuntu 20.04 through 23.10. 

Anyone is welcome to contribute, see [Guide on Contributing](CONTRIBUTING.md)

## Releases

### 2024-05-18

* added a basic text editor application written in Rust to read/write/edit files
* usability improvements for shell scripts and makefiles
* fixed some bugs

### 2024-05-03

* Code accompanying Blog Posts Pt1 to Pt9
* features: cross-compile toolchain build script, basic kernel, interrupt handled keyboard input, read and write files to disk, graphical terminal written in rust.

##
