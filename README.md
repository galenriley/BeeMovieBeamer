# BeeMovieBeamer

A stupid gift for Junior  
based on a [tiktok from @jacuto](https://www.tiktok.com/@jacuto/video/7439527807316790574?lang=en)  
with help from Becca and Don

## Safety/Disclaimers

I am an idiot that doesn't know what he is doing with microcontrollers! Lithium batteries are terrifying! And flammable!
It is highly recommended to gently unplug the battery when not in use and store it in a fire safe container away from anything combustible.

![BeeMovieBeamer render](https://github.com/user-attachments/assets/5b0ba572-1fcc-4ca3-a39d-aec45c3e1888)


## Button Functions

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
  The case allows access to this button using the provided sim ejector tool, or use a paperclip.

## Disassembly
  
1. Remove the hand strap.
2. Insert a fingernail or prying tool in the slots on the side of the case.
3. Remove the back panel by sliding it out.
4. _Carefully_ lift the battery, slide out the battery tray, and unplug the battery cable.  
 
## Materials

- [LILYGO T4 v1.3 with 2.4 inch display](https://lilygo.cc/products/t4?variant=42405660393653)
- 654065 3.7V Lipo Battery, 2000mAh with JST GH 1. 25mm plug ([I used this one and soldered on the plug which came packaged with the LILYGO T4 board](https://www.amazon.com/dp/B07S75L945))
- [This micro USB cable fits the case perfectly](https://a.co/d/34nNxp7)
- 1 inch nylon strap
- tri-glide buckle

## Instructions to Make Your Own

# Buliding the Software
1. [Download and install Visual Studio Code](https://code.visualstudio.com/Download)
2. Open Visual Studio Code and [install PlatformIO IDE](https://platformio.org/install/ide?install=vscode)
3. [Download the project files from GitHub](https://github.com/galenriley/BeeMovieBeamer/archive/refs/heads/main.zip) and unzip somewhere on your computer.
4. From Visual Studio Code, click the PlatformIO icon on the left sidebar (it looks like an ant's head).
5. From the PlatformIO frame, click "Open Project" and navigate to the BeeMovieBeamer folder you unzipped earlier.
6. Connect the LILYGO T4 board to your computer via the micro usb port.
7. Near the bottom of Visual Studio Code, look for these icons. Mouse-over the Check icon, confirm the tooltip says "PlatformIO: Build", and click it.
<img width="636" height="82" alt="image" src="https://github.com/user-attachments/assets/c13983f5-89c3-44b5-b37c-6d5abdc35b39" />
8. After the Build process is complete, mouse over the Right Arrow icon, confirm the tooltip says "PlatformIO: Upload", and click it. Wait for the upload to complete and the board to reboot and show the software startup screen.

# Fabricating the Box
1. 

