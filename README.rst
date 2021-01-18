===========
QEMU-CHOMPTECH README
===========

This a naive experiment to build a qemu model for the chomptech SOC.
This is not "released" software. This is work in progress.

FAQ
========

Q: What is it?

A: A model for a closed source chip and closed source firmware (read: reverse engineering a product)

Q: But Why?

A: Because it is fun

Q: Can I use this for X, Y or Z?

A: No

Q: Why is your git history so ugly (useless commit comments, strange rebases, roundhouse-commits)?

A: I only work parttime on this, so I don't really care. As soon the project reaches a state where there is a stable fundament, 
I'll force push with an "Initial Commit" comment and stick to the common rules (atomic commits, descriptive comments, etc.). 
For the moment I just punch out what I have hacked so far.

Building
========

To build:

``./configure --target-list="arm-softmmu" --enable-fdt --disable-vnc --disable-kvm --prefix=$(pwd)/release``

``make``

Running
========

To Run:

``./arm-softmmu/qemu-system-arm -device loader,file=roms/chomptech/BIOS.bin -drive if=none,format=raw,file=flashdata.bin -machine chomptech -d unimp -S -gdb tcp::1234 --serial stdio``

Where BIOS.bin is the Mask-ROM from the chip and flashdata.bin is a copy of the NAND flash.

You can attach an arm-eabi-none-gdb after starting using:

``arm-eabi-none-gdb``

and then attach using 

``target remote :1234``


 2021 rrrh - cheers
