# BeeMovieBeamer

A stupid gift for Junior  
based on a [tiktok from @jacuto](https://www.tiktok.com/@jacuto/video/7439527807316790574?lang=en)  
with help from Becca and Don (who don't want to be linked to)

## Safety/Disclaimers

I am an idiot that doesn't know what he is doing with microcontrollers! Lithium batteries are terrifying! And flammable!
It is highly recommended to gently unplug the battery when not in use and store it in a fire safe container away from anything combustible.

![BeeMovieBeamer render](https://github.com/user-attachments/assets/5b0ba572-1fcc-4ca3-a39d-aec45c3e1888)


## Button Functions

Most functions are accessible via the startup screen. If you're "lost" during operation, press the power button once to restart the device.

- **Power Button** (located on the side of the device, near the USB port)  
  Press to reset/power on and go to the startup screen. Double press to power off.
- **Button 1** (located next to the screen, in the center)  
  Plays The Bee Movie once, then ~~puts the device to sleep~~ disables the screen backlight to conserve battery.
- **Button 2** (located next to the screen, on top if the USB port is oriented upward)  
  Vertically flips the display.  
  *This button only works on the startup screen.
- **Button 3** (located next to the screen, on the bottom if the USB port is oriented upward)  
  Plays The Bee Movie on loop until the device is reset using the Power Button or the battery dies.  
  *This button only works on the startup screen. 
- **Button 4** (located on the side of the device, near the USB port)  
  Resets the "Lifetime Bees Movied" counter on the startup screen.  
  Press once for conformation prompt. Press twice to reset the counter.  
  *This button only works on the startup screen.
  The case allows access to this button using a paperclip or sim ejector tool.

## Disassembly for Safe Storage
  
1. Remove the hand strap.
2. Insert a fingernail or prying tool in the slots on the side of the case.
3. Remove the back panel by sliding it out.
4. _Carefully_ lift the battery, slide out the battery tray, and unplug the battery cable.  
 
## Materials

- [LILYGO T4 v1.3 with 2.4 inch display](https://lilygo.cc/products/t4?variant=42405660393653)
Be sure to get the Q368 variant. The other is not compatible with the 3d printed case included in this project.
- 3.7V Lipo Batterywith JST GH 1.25mm plug, max dimensions 6.5mm x 40mm x 65mm
[I used this one and replaced the stock plug with one which came packaged with the LILYGO T4 board](https://www.amazon.com/dp/B07S75L945). You could buy a smaller battery with the right plug, and fill the leftover space in the case with some scrap foam to prevent it from rattling around.
- [This micro USB cable fits the case perfectly](https://a.co/d/34nNxp7)
- 1 inch nylon webbing strap and tri-glide buckle (or your own solution, elastic would also work)
- 2x M2 screws

You also need access to a computer to build and deploy the software, 3D printer(s), and you will likely need to solder the battery cable.

## Instructions to Make Your Own

### Buliding the Software
1. [Download and install Visual Studio Code](https://code.visualstudio.com/Download)
2. Open Visual Studio Code and [install PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
3. [Download the project files from GitHub](https://github.com/galenriley/BeeMovieBeamer/archive/refs/heads/main.zip) and unzip somewhere on your computer.
4. From Visual Studio Code, click the PlatformIO icon on the left sidebar (it looks like an ant's head).
5. From the PlatformIO frame, click "Open Project" and navigate to the BeeMovieBeamer folder you unzipped earlier.
6. Connect the LILYGO T4 board to your computer via the micro usb port.
7. Near the bottom of Visual Studio Code, look for these icons. Mouse-over the Check icon, confirm the tooltip says "PlatformIO: Build", and click it.
<img width="318" height="41" alt="image" src="https://github.com/user-attachments/assets/c13983f5-89c3-44b5-b37c-6d5abdc35b39" />

8. After the Build process is complete, mouse over the Right Arrow icon, confirm the tooltip says "PlatformIO: Upload", and click it. Wait for the upload to complete and the board to reboot and show the software startup screen.

### Fabricating the Bee Movie Beamer
1. Find the 3D files in the project's "/case/" folder (you downloaded these already) and print using your preferred filament and settings. This is left as an exercise for the reader.
The case prints just fine with FDM, though the buttons are tiny and printing with resin would result in a better looking end product.
2. Sand/paint/seal/finish the parts as preferred.
Note: The clearances around the buttons are snug and you may need to sand away some material to achieve a satisfactory final fit, especially around the power button on the side.
3. Set the case on a stable surface and carefully position the buttons in their slots. Tweezers will come in handy.
4. Place the board in the case and secure with 2x screws.
5. If your battery didn't already have a 1.25mm plug, cut off the stock plug and solder the one which came packaged with the board.
Safety warning: Do not let the wires touch when soldering, for explosion and fire and pain reasons.
6. Plug in the battery, route the cable around the battery tray while installing it (the tray is to isolate the battery from being punctured by the IO pins on the board, you could also snip these pins off), and install the back lid of the case.
7. For the hand strap, feed the nylon webbing around the center bar of the triglide buckle and sew to itself.
8. On the case, feed the hand strap from the front to the back through one buckle, across the back lid, from the back to the front through the other buckle, and through both slots in the triglide. Adjust for your hand, cut to fit, then remove the strap to melt the raw end from fraying. Reinstall the strap.

Enjoy!
