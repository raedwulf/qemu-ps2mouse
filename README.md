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

First you need to make sure your ps/2 mouse is linked with a ``/dev/serio_raw`` .
This can be done by going through  ``/sys/bus/serio/devices/serio*`` and finding the one currently linked with your psmouse driver... it will have a file ``protocol``.
To turn serio2 (yours is probably different) into a ``serio_raw`` device,

	$ sudo echo -n serio_raw > /sys/bus/serio/devices/serio2/drvctl

I run qemu straight from the source tree using, note (``/dev/serio_raw0`` appears on the first creation, it may change incrementally if you switch back to ``psmouse``) :

	$ ~/source/qemu/x86_64-softmmu/qemu-system-x86_64 -net nic,vlan=1,model=rtl8139 -net user,vlan=1 -chardev ps2mouse,id=ps2raw,path=/dev/serio_raw0 -cdrom /media/window | tee mouse.log

Getting back to another driver, e.g. psmouse...

	$ sudo echo -n psmouse > /sys/bus/serio/devices/serio2/drvctl

Questions
---------

Mail me for any questions you might have about this.

Tai Chi Minh Ralph Eastwood <tcmreastwood@gmail.com>
