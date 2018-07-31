# Programming ATSAMD51J18 with OpenOCD

To get an idea, see [this tutorial](http://omzlo.com/articles/programming-the-samd21-using-atmel-ice-with-openocd-(updated)).

Unfortunately, the current version of OpenOCD (0.10.0) does not ship a patch
that is needed to work properly with the ATSAMD51J18.

This means, you may have to compile your own patched version of openocd.


## Get openocd and patch it
We need [this patch](http://openocd.zylin.com/#/c/4272/).
The code below takes care of downloading and applying it.

```
git clone http://git.code.sf.net/p/openocd/code openocd
cd openocd
git fetch http://openocd.zylin.com/openocd refs/changes/72/4272/5 && git checkout FETCH_HEAD
```

## Compile and install it

You will need a few dependencies / tools. Under Ubuntu 16, you can install most of them with:
```
sudo apt install autoconf build-essential cmake gdb-arm-none-eabi libtool libtool-bin libhidapi-dev libusb-dev libusb-1.0-0-dev pkg-config
```

If you don't want it under `/usr/local`, change the prefix in the configure command below.

```
./bootstrap
./configure     --prefix=/usr/local     --enable-aice     --enable-amtjtagaccel     --enable-armjtagew     --enable-cmsis-dap     --enable-dummy     --enable-ftdi     --enable-gw16012     --enable-jlink     --enable-jtag_vpi     --enable-opendous     --enable-openjtag_ftdi     --enable-osbdm     --enable-legacy-ft2232_libftdi     --enable-parport     --disable-parport-ppdev     --enable-parport-giveio     --enable-presto_libftdi     --enable-remote-bitbang     --enable-rlink     --enable-stlink     --enable-ti-icdi     --enable-ulink     --enable-usb-blaster-2     --enable-usb_blaster_libftdi     --enable-usbprog     --enable-vsllink
make -j8
make install
```
