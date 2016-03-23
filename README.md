
# Serial Port FIFO Bridge for Mac OS X

This "bridge" program relays data between a serial port and a pair of named pipes (FIFOs).
The pipes must be created independently, such as by mkfifo(1).


## Synopsis

```
serial2fifo <serial-device> <s2c-fifo> <c2s-fifo>
```


## Description

![UML Collaboration Diagram](doc/UML-A.png?raw-true "UML Collaboration Diagram of the Bridge Program and its Peers")

The bridge program facilitates bi-directional communication between a serial-connected hardware device (`s`) and a client program (`c`).

The bridge program exchanges data with the serial-connected hardware device via its operating system device file.
This device file consitutes a bi-directional communication channel.

The bridge program exchanges data with the client program via a pair of named pipes.
The pair of named pipes consitute a bi-directional communication channel.

The bridge program relays data between the two bi-directional communication channels, so that the serial-connected hardware device and the client program communicate with one another.


## Running the Program

Connect the hardware device to the serial port, if it is not already connected. 
Detect the name of the device file (typically of the form `/dev/tty...`).

Create a pair of named pipes, if they do not already exist. Refer to mkfifo(1) in the operating system manual:
```
mkfifo serial-to-client
mkfifo client-to-serial
```

Start the bridge program, using the name of the device file you detected, and the names of the pipes:
```
serial2fifo /dev/tty.usbserial-XXXXXXXX serial-to-client client-to-serial
```

Start your client program, which will write to the serial device via the *client-to-serial* pipe, and read from the serial device via the *serial-to-client* pipe.
Typically, you will need to tell your client program the names of the pipes:

```
MyClientProgram client-to-serial serial-to-client
```

If either the client program or the hardware device disconnect, the bridge program will exit.
Restart the bridge program to begin another session.


## System Requirements

The program was developed for Mac OS X 10.5.
In order to compile this C/C++ language program, you will need Apple Developer Tools of suitable vintage.


## Compiling the Source Code

The source code comprises `serial2fifo.cc` and `makefile`.

```
cd src
make
```


# Background

This bridge program relays data between two, bi-directional communication channels:
one bi-directional channel being a serial port, the other being a pair of named pipes.

A serial port constitutes a bi-directional communication channel.
Serial ports are named, and apper in the file system as special files.

A pipe is a uni-directional communication channel.
A pair of pipes may constitute a bi-directional communication channel.
Pipes may be given names, and appear in the file system as special files.

By giving names to pipes and serial ports, they can be located.

Pipes facilitate inter-process communication (IPC).
A pipe is also known as a first-in-first-out (FIFO) mechanism, because it preserves the order of data transmitted through it.

Today, many computers have Universal Serial Bus (USB) ports, instead of RS-232 ("serial") ports.
However, RS-232 serial devices may be connected to universal serial buses by hardware adapters.
And USB devices may be presented as RS-232 devices by software adapters.
This bridge program can participate in scenarios involving such adapters.

To interface with the serial port, the bridge uses an operating system device file (typically `/dev/tty...`).
The names of device files vary.
Device files typically pre-exist, or are created automatically by the operating system in response to hardware events, such as the connection of a device.

You can often detect the file name of a hardware device by listing the device directory before the device is connected, then listing the device directory again after the hardware device is connected.
Sometimes device files persist, even when the device is not connected; this is often so for wireless devices. 
This can make it more difficult to detect the file names of wireless devices.

You can name pipes, used by this bridge program, whatever you prefer.
In a bi-directional context, the frame of reference for reading and writing is often ambiguous.
For clarity in this documentation, long names were used which explicitly communicate the direction and frame of reference.
You may prefer something simple, if ambiguous, like `transmit` and `receive`.


## Motivation

I originally developed this bridge program so that software prototypes developed in Java language could talk to serial-connected hardware devices.

Java Runtime Environments (JREs) often lack support for serial ports.
This bridge program presents the serial device via a pair of named pipes in the file system.
The named pipes are accessible in Java by ordinary file input/output APIs.

Thus, this bridge allows Java programs to communicate with serial-connected hardware devices.
However, there is nothing Java-specific about the bridge program.
It will work with any client.


## Known Defects or Limitations

- Because this program was developed for proprietary purposes, involving a specific device, the serial port settings happen to be hard-coded (9600 baud, 8 data bits, et cetera). These parameters may be changed by editing the source code.

- The program will exit if the hardware device or client hang up. This is by design, but it could be construed as a nuissance or defect.


## License Terms

This software is provided to you under the [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0) (the "License").
You may not use this software except in compliance with the License.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.
IN NO EVENT SHALL THE AUTHORS OR RIGHTS HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE, OR ITS USE OR OTHER DEALINGS.
REFER TO THE LICENSE FOR SPECIFIC LANGUAGE GOVERNING PERMISSIONS AND LIMITATIONS.

