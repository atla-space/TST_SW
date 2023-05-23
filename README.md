Tensometric stand
=================
In order to test different kind of propelers/engines I have build a tensometric stand wi a simple app to record the thrust data that will significantly help with testing different designs.


Dependencies
============
Most of the dependencies can be installed by `Conan 1` package manager.

Extra dependency is library `soci` and `boost` (because soci depends on boost, but doesn't specify it). Aaaand `libwiringpi` for GPIO.

On deployment machine you also need a `mariadb`/`mysql` server.


Compiler
========
You need a compiler with C++20 modules support. Tested with `clang 16.0.5`.

Hardware
========

This software is not limited to any specific board, as long as it runs some reasonable version of the linux. I am running it on the `Odroid C4` board. I think it is a tremendous board because it supports interrupts on its pins. More of such boards. I am running kernel `Linux bullseye-server 6.2.0-odroid-arm64 #1 SMP PREEMPT aarch64 GNU/Linux`.

## Tensometer
A software driver dor the `HX711` is implemented. Most of the board with the HX711 I could find online are configured to 10 samples per second. It is possible, however, to change the frequency to 80 SPS by modifying the board. That is what I did and the code reflect this timing.

## LED
It is very usefull to have some kind of LED to indicate the state of the system. I am using a simple LED connected to the GPIO pin. The LED is connected to the 3V3 via resistor. The LED is turned on when the pin is set to `LOW` and turned off when the pin is set to `HIGH`. In practice you want your app to drive your LED so you know it is running. Much easier than carrying SSH terminal in your pocket and investigating why you can not connect via the browser just to find out it didn't started properly.
