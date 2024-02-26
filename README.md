# Recreating-Asteroids-with-FPGA
Final project for UB EE 379. The task was to recreate the Atari Asteroids game in C. The code is then ran and played on a Zybo Z7 Development board with a PMod MTDS display attachment. In the game, the player moves the ship around the screen using three pushbuttons on the Zybo board. The player must avoid asteroids that are traveling across the screen at different speeds while also trying to destroy them with missiles, which are activated with a pushbutton. Starting with three lives, a life is lost whenever the player collides with an asteroid, and it is game over once all lives are lost. The software was written using the Xilinx Vitis Integrated Development Environment, tested on the Zybo board using the pushbuttons, and visualized on the LCD display.

##  Built Using
* [Vitis](https://www.xilinx.com/products/design-tools/vitis/vitis-platform.html) - Software platform optimized for Xilinx FPGA
* [Zybo Z7](https://digilent.com/shop/zybo-z7-zynq-7000-arm-fpga-soc-development-board/) - FPGA board the program was loaded to
* [PMod Display](https://digilent.com/reference/pmod/pmodmtds/start) - LCD display

## Demo
https://www.youtube.com/watch?v=LpBX4KFL6hI 

## Authors

* **Kevin C** 
* **Michelle M** 
