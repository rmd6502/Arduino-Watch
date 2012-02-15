Arduino based watch.  Currently uses Seeedstudio's film series - OLED shield + bluetooth shield
![watch picture](/Arduino-Watch/watch_image.jpg)

Manifest
========

MeetAndroid - modified version of MeetAndroid.  See amarino-toolkit.net

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
