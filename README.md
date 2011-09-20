Qemu with -chardev psmouse
==========================

Introduction
------------

This is a patched Qemu so that you can run a VM but use your PS/2
mouse directly inside the VM.
The intended purpose of this patch is to get initialisation sequences
for various touchpads (e.g. ALPS) for unsupported hardware.

Building
--------

The build instructions are the same as standard Qemu.
I use:

	$ ./configure --prefix=/usr --sysconfdir=/etc --audio-drv-list=sdl --audio-card-list=ac97,sb16,es1370,hda --enable-docs --disable-werror
	$ make -j3

Your requirements may vary.

Running
-------

I run qemu straight from the source tree using:

	$ ~/source/qemu/x86_64-softmmu/qemu-system-x86_64 -net nic,vlan=1,model=rtl8139 -net user,vlan=1 -chardev ps2mouse,id=ps2raw,path=$SERIO_RAW -cdrom /media/window | tee mouse.log

Questions
---------

Mail me for any questions you might have about this.

Tai Chi Minh Ralph Eastwood <tcmreastwood@gmail.com>
