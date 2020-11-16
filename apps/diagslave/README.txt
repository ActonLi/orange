                            FieldTalk(TM) diagslave
                          Linux Edition Read Me Notes

   Revision 3.2, 2019-04-01

   This  Read  Me  file  contains last-minute product information for the
   FieldTalk(TM) diagslave utility.

   diagslave  is  a  command  line  based Modbus slave simulator and test
   utility.
     _________________________________________________________________

Files part of the package

   README.txt, README.pdf
          These Read Me notes.

   LICENSE-FREE.txt, LICENSE-FREE.pdf
          License  containing the Terms & Conditions of use for this free
          software.

   linux_arm-eabihf/modpoll
          Linux   ARM  binary  for  ARM  boards  like  Raspberry  Pi  and
          BeagleBoard

   linux_i386/modpoll
          Linux x86 binary for 32-bit Intel x86 architectures

   linux_x86-64/modpoll
          Linux x86_64 binary for 64-bit Intel x86 architectures
     _________________________________________________________________

Usage

  Usage: diagslave [OPTIONS] [SERIALPORT]
  Arguments:
  serialport    Serial port when using Modbus ASCII or Modbus RTU protocol
                COM1, COM2 ...                on Windows
                /dev/ttyS0, /dev/ttyS1 ...    on Linux
  General options:
  -m ascii      Modbus ASCII protocol
  -m rtu        Modbus RTU protocol (default)
  -m tcp        MODBUS/TCP protocol
  -m udp        MODBUS UDP
  -m enc        Encapsulated Modbus RTU over TCP
  -o #          Master activity time-out in seconds (1.0 - 100, 3 s is default)
  -a #          Slave address (1-255 for RTU/ASCII, 0-255 for TCP)
  Options for MODBUS/TCP:
  -p #          TCP port number (502 is default)
  -c #          Connection time-out in seconds (1.0 - 3600, 60 s is default)
  Options for Modbus ASCII and Modbus RTU:
  -b #          Baudrate (e.g. 9600, 19200, ...) (19200 is default)
  -d #          Databits (7 or 8 for ASCII protocol, 8 for RTU)
  -s #          Stopbits (1 or 2, 1 is default)
  -p none       No parity
  -p even       Even parity (default)
  -p odd        Odd parity
  -4 #          RS-485 mode, RTS on while transmitting and another # ms after
  Options for Modbus RTU:
  -f #          Tolerance for inter-frame gap detection in ms (0-20)
     _________________________________________________________________

Release history

  Version 3.2 (2019-04-01)

     * Added -f # option to set tolerance for inter-frame gap detection

  Version 3.1 (2018-11-28)

     * Modbus UDP protocol added (-m udp)

  Version 3.0 (2018-07-09)

     * RTU  over  TCP protocol added, which is also known as encapsulated
       RTU (-m enc)

  Version 2.13 (2013-05-16)

     * Bug fix, slave ID of 255 for Modbus/TCP did not work

  Version 2.12 (2012-07-19)

     * RTU/ASCII: Added RS-485 mode for Win32, QNX and Linux platforms.
     * COMn syntax can now also be used for COM port number >= 10

  Version 2.11 (2011-03-05)

     * Protocol  is now auto-detected as RTU or TCP depending on value of
       first parameter

  Version 2.10 (2010-03-31)

     * Added connection time-out, changed -t to -o for activity time-out.

  Version 2.9 (2009-11-16)

     * Default baudrate is now 19200 as per Modbus standard.

  Version 2.8 (2009-10-26)

     * Using  the  -d  and  -s  command line parameters returned an error
       message in earlier releases.

  Version 2.7 (2009-10-20)

     * First release as standalone application.
     _________________________________________________________________

   Copyright (c) 2002-2019 proconX Pty Ltd. All rights reserved.

   Please refer to file LICENSE-FREE for license and distribution terms.

   THIS  SOFTWARE IS PROVIDED BY PROCONX AND CONTRIBUTORS "AS IS" AND ANY
   EXPRESS  OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT NOT LIMITED TO, THE
   IMPLIED  WARRANTIES  OF  MERCHANTABILITY  AND FITNESS FOR A PARTICULAR
   PURPOSE  ARE  DISCLAIMED. IN NO EVENT SHALL PROCONX OR CONTRIBUTORS BE
   LIABLE  FOR  ANY  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL  DAMAGES  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE  GOODS  OR  SERVICES;  LOSS  OF  USE,  DATA, OR PROFITS; OR
   BUSINESS  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER  IN  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
     _________________________________________________________________