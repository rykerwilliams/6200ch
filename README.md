6200ch
======
6200ch Firewire Cable Box Channel Changer. Channel-change script for cable box via a 1394 (aka Firewire) connection.

Supported Models:
     * Motorola DCT-6200 
     * Motrola / Arris DCX3200M 

Author: Stacey Son

Original README From wiki page
=====
I was working on some code to record the video/audio via firewire, however, I have noticed that my cable provider has started to encrypt (5C/DTCP) many of the channels that I care to record. I am currently using a s-video cable, a PVR-250 and this code with mythtv until I figure out something better to do. This works with for DCT-6200 but may be easy to modify for other firewire tuners as well.

To use this with mythtv do the following:

(1) Make sure you have 1394/Firewire drivers installed in your kernel.

(2) Install libraw1394 and libavc1394. (On gentoo: "emerge libavc1394").

librom1394 is now part of libavc. Under some circumstances libavc must be listed first on the compile line (updated below, known for mythbuntu 10.10 & 11.04)


(3) Compile and install "6200ch":

     # cc -std=gnu99 -o 6200ch 6200ch.c -lavc1394 -lraw1394 -lrom1394
     # cp 6200ch /usr/local/bin
(4) Connect a 1394/Firewire cable from your computer to your DCT-6200 and test:

     # 6200ch <your_favorite_channel_number>
(5) Configure Mythtv to use the channel changer by running the "setup" program and adding to "/usr/local/bin/6200ch" to the "External channel change command" field under "Connect source to Input".

Enjoy!


Using 6200ch to add support for new devices to MythTV
=====
MythTV has an internal database of model and vendor IDs the correspond to revisions of Set Top Box hardware. To get the Model and Vendor ID of your box, you can run 6200ch -v -v $SomeNumber

     # ./6200ch -v -v 750
     # starting with node: 1
     # node 1: vendor_id = 0x00002ae1 model_id = 0x00000000
     # Warning: Unit Spec ID different.
     # Warning: Unit Software Version different.
     # AV/C Command: 750 = Op1=0x00487C27 Op2=0x00487C25 Op3=0x00487C20

If your device is not supported by the script, you can add the vendor and model IDs as seen [here]cgitgithub to add support for a new device. If you do so, please update the script on this page with your changes.
