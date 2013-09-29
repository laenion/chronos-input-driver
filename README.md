chronos-input-driver
====================

Linux mouse and joystick driver for the Texas Instruments eZ430-Chronos watch

Compilation
-----------

After you downloaded and extracted the files just type `make` in the directory containing the extracted files. You need to have a C compiler like gcc installed on your system.

Usage
-----

This driver will only work on Linux as it is using the kernel's uinput framework.

1. As root: `modprobe uinput`
2. Plug your RF Access Point included with your watch into your system.
3. On your Chronos watch select "ACC" (for key presses and movement data) or "PPt" (key presses only) and initiate transfer with the lower right button. You should see a blinking wireless symbol now.
4. Change to the program directory and type `./chronos-input` for mouse mode or `./chronos-input -m j` for joystick mode

I'm assuming you are using the default firmware of the device. 

Make sure you have write permissions the uinput device (usually `/dev/uinput`) and read and write permissions to the access point device (usually `/dev/ttyACM0`).

For a list of all program options type `./chronos-input -h`

If you are using your watch in joystick mode I would recommend calibrating it using jscal, otherwise your joystick will probably be very slow.

Both the mouse and the joystick driver behave like real devices, have their own device entries (`/dev/input/[mouse?|js?]`) and thus can be configured and used with your existing applications.

Mouse mode
----------

When using the driver in mouse mode the keys / axis are set as follows:

    *  = Left click
    Up = Right click
    #  = middle click
    
    Rotating the clock to the left / right  = mouse cursor movement on x axis
    Rotating the clock forwards / backwards = mouse cursor movement on y axis

Joystick mode
-------------

When using the driver in joystick mode the keys / axis are as follows:

    *  = Button 1
    Up = Button 2
    #  = Button 3

    y-axis = Axis 1
    x-axis = Axis 2
    z-axis = Axis 3

Problems
--------

- Using PPt mode for presentations may not do what you expect: The buttons will generate left / middle / right clicks like in ACC mode, but most presentation programs need other keys to change slides. Use the Chronos Control Center if you want to define custom keyboard keys for these buttons.
- If the battery power is low (&lt;2.9V) the clock may randomly crash when transfering data. There is nothing I can do about that, so please change your batteries in that case.
- On some systems no data is transferred. As a workaround start the Chronos Control Center to correctly initialize the RF Access Point. If you have any idea what might be missing in my code please drop me a note.

Questions / Feedback
--------------------

You are stuck somewhere? Something in the documentation isn't as clear as it should be? Or is it simply not working?

Drop me a note (ignaz.forster@gmx.de) or open an issue in the issue tracker.
