An atmega based usb hid consumer decive for volume, pause/play, next and previous functions
===========================================================================================

see also [http://www.pengurobotics.com/projects/media_controller](http://www.pengurobotics.com/projects/media_controller)

and [http://www.thingiverse.com/thing:126130](http://www.thingiverse.com/thing:126130) for the 3D files

Building the firmware
---------------------
you need the vusb files from 
https://github.com/obdev/v-usb or http://www.obdev.at/products/vusb/download.html

more specifically: the "usbdrv" folder needs to be copied to your cloned repository.

step by step example:

clone this repository

    git clone https://github.com/pengumc/atmega_multimedia_control

grab the vusb files    

    wget http://www.obdev.at/downloads/vusb/vusb-20121206.tar.gz

extract

    tar -xf vusb-20121206.tar.gz

cp the usbdrv folder to the cloned repository

    cp -r vusb-20121206/usbdrv atmega_multimedia_control/
    
build firmware

    cd atmega_multimedia_control
    make
    
