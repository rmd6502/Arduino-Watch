Arduino based watch.  Currently uses Seeedstudio's film series - OLED shield + bluetooth shield
![watch picture](https://github.com/rmd6502/Arduino-Watch/blob/master/watch_image.jpg?raw=true)

Manifest
========

MeetAndroid - modified version of MeetAndroid.  See amarino-toolkit.net

There are two commands recognized right now:
  * t(10-digit unix time)^S - set the time and date.  You can use _expr `date +"%s"` - 3600 \* 5_ to generate the date for your locale (update the
 number of seconds subtracted for your time zone).
  * x(message)^S - send message to the watch.  It will vibrate the motor for .5 sec

Time        - Modified version of the Time library to work with a 32kHz crystal to keep time

ardwatch    - the watch code itself

motor.pdf   - subcircuit for the motor control

testscreen  - display the time and date on the screen

ttimer2     - test sketch for timer2 with crystal

Limitations
===========
Still working on getting Bluetooth working.  It does blink but isn't always discoverable.

Also, just to dash your hopes to the ground early, it doesn't work with iOS, and may never.  The problem is iOS restricts the use of the
Bluetooth Serial Port Profile, so if you're not jailbroken you can't use it.
