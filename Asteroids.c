#include "xscugic.h"
#include "xtmrctr.h"
#include "xparameters.h"
#include <XGpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <MyDisp.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

//Defines for interrupt IDs
#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID
#define GPIO_INT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR	// GPIO interrupt ID

//Global variables accessible from any function
XGpio input;
MYDISP display;
const float PI = 3.14159;
float angle = 180;
int cx = 120, cy = 160, w = 20;		// Represents the center point and height of the triangle (ship)
int ax[4];
int ay[4]; // Asteroid centers
int asteroidWidth = 25;
int missileCollision[4] = {1,1,1,1}; // Indicates which asteroid was destroyed
int mx[10] = {0,0,0,0,0,0,0,0,0,0};
int my[10] = {0,0,0,0,0,0,0,0,0,0}; // Missile centers
int activeMissile[10] = {0,0,0,0,0,0,0,0,0,0}; // Keeps track of active missiles
float missileAngle[10] = {0,0,0,0,0,0,0,0,0,0}; // For the missiles to head in the same direction (in radians)
int lives = 4;
int score = 0;

//Global Interrupt Controller
static XScuGic GIC;

//This function initalizes the GIC, including vector table setup and CPSR IRQ enable
void initIntrSystem(XScuGic * IntcInstancePtr) {

	XScuGic_Config *IntcConfig;
	IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
	XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, (void *)IntcInstancePtr);
	Xil_ExceptionEnable();

}

float distBetween(int x1,int y1,int x2,int y2) {
	return sqrt(pow(x2-x1,2) + pow(y2-y1,2));
}

void drawShip (int x, int y, float angle) {		// (x,y) is the center and angle is in degrees

	int x1, y1, x2, y2, x3, y3;		// (x1, y1) represents the front point
	angle = (angle*PI)/180;
	float theta1 = (120*PI/180) + angle;
	float theta2 = (240*PI/180) + angle;

	x1 = x + round((w/2)*sin(angle));
	y1 = y + round((w/2)*cos(angle));
	x2 = x + round((w/2)*sin(theta1));
	y2 = y + round((w/2)*cos(theta1));
	x3 = x + round((w/2)*sin(theta2));
	y3 = y + round((w/2)*cos(theta2));

	display.drawLine(x1, y1, x2, y2);
	display.drawLine(x2, y2, x3, y3);
	display.drawLine(x3, y3, x1, y1);

}

void drawAsteroids() {

	for (int i = 0; i < 4; i++) {
		if (missileCollision[i] == 1) {
				ax[i] = rand() % 215;
				ay[i] = rand() % 295;
				missileCollision[i] = 0;
		}
	}

}

void moveAsteroids() {

	int vx = 6, vy = 6;

	ax[0] += vx;
	ay[0] += vy;
	ax[1] -= (vx - 2);
	ay[1] -= (vy - 2);
	ax[2] += (vx - 4);
	ay[2] -= (vy - 4);
	ax[3] -= (vx - 5);
	ay[3] += (vy - 5);

	for (int i = 0; i < 4; i++) {
		if (ax[i] >= 240-asteroidWidth/2) ax[i] = asteroidWidth/2;
		else if (ax[i] <= asteroidWidth/2) ax[i] = 240-asteroidWidth/2;
		else if (ay[i] >= 320-asteroidWidth/2) ay[i] = asteroidWidth/2;
		else if (ay[i] <= asteroidWidth/2) ay[i] = 320-asteroidWidth/2;
	}

	for (int i = 0; i < 4; i++) {
		display.drawEllipse(true,ax[i]-asteroidWidth/2,ay[i]-asteroidWidth/2,ax[i]+asteroidWidth/2,ay[i]+asteroidWidth/2);
	}

}

void asteroidCollision() {

	for (int i = 0; i < 4;i++) {
		if(distBetween(cx,cy,ax[i],ay[i]) < (10 + asteroidWidth/2)) {
			cx = 120;
			cy = 160;
			angle = 180;
			lives--;
		}
	}
}

void drawMissiles(float angle) {

	angle = (angle*PI)/180;

	 for (int i = 0; i < 10; i++) {
		 if (activeMissile[i] == 0) {
			 mx[i] = cx + round((w/2)*sin(angle));
	         my[i] = cy + round((w/2)*cos(angle));

	         if (mx[i] <= 2 || mx[i] >= 238 || my[i] <= 2 || my[i] >= 318) {
	             activeMissile[i] = 0;
	             return;
	         }

	         display.drawEllipse(true,mx[i]-2,my[i]-2,mx[i]+2,my[i]+2);
	         missileAngle[i] = angle;
	         activeMissile[i] = 1;

	         display.drawEllipse(true,mx[i]-2,my[i]-2,mx[i]+2,my[i]+2);
	         break;
	     }
	 }


}

void moveMissiles(int i) {

	int vx = 5, vy = 5;

	mx[i] = mx[i] + vx*round(2*sin(missileAngle[i]));
	my[i] = my[i] + vy*round(2*cos(missileAngle[i]));

	if (mx[i] <= 2 || mx[i] >= 238 || my[i] <= 2 || my[i] >= 318) {
	    activeMissile[i] = 0;
	    return;
	}

	display.drawEllipse(true,mx[i]-2,my[i]-2,mx[i]+2,my[i]+2);

}

void missCollision() {

	for (int i = 0; i < 10; i++) {
		if (activeMissile[i] == 1) moveMissiles(i);
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++){
			if (distBetween(mx[j],my[j],ax[i],ay[i]) < (2 + asteroidWidth/2)) {
				missileCollision[i] = 1;
				activeMissile[j] = 0;
				score = score + 20;
			}
		}
	}

}

void buttonInterruptHandler(void *instancePointer) {

	if (XGpio_DiscreteRead(&input, 1) == 1) {	// turns ship clockwise (to the right)
		angle -= 20;
	}

	if (XGpio_DiscreteRead(&input, 1) == 2) {	// shoots missiles
		drawMissiles(angle);
	}

	if (XGpio_DiscreteRead(&input, 1) == 4)	{	// moves ship forward
		cx = cx + (5*sin((angle*PI)/180));
		cy = cy + (5*cos((angle*PI)/180));
	}


	if (XGpio_DiscreteRead(&input, 1) == 8) {	// turns ship counter-clockwise (to the left)
		angle += 20;
	}

	XGpio_InterruptClear(&input, GPIO_INT_ID);

}

int main() {

	initIntrSystem(&GIC);	// Initializes the Global Interrupt Controller (GIC)

	//Configure GPIO input and set direction as usual
	XGpio_Initialize(&input, XPAR_AXI_GPIO_0_DEVICE_ID);	// Initializes XGpio variable
	XGpio_SetDataDirection(&input, 1, 0xF);	// Sets direction to input

    //Configure GPIO interrupt as done in class
    XScuGic_Connect(&GIC, GPIO_INT_ID, (Xil_ExceptionHandler) buttonInterruptHandler, &input);	// Registers GPIO handler (what gets called when GPIO interrupt occurs)
    XGpio_InterruptEnable(&input, XGPIO_IR_CH1_MASK);	// Enables GPIO interrupt sources in GPIO interface block
    XGpio_InterruptGlobalEnable(&input);	// Turns on interrupt source
    XScuGic_Enable(&GIC, GPIO_INT_ID);		// Turns on GPIO interrupt source
    XScuGic_SetPriorityTriggerType(&GIC, GPIO_INT_ID, 0x8, 0x3);	// Sets priority level of GPIO interrupt to 2, meaning lower priority


	display.begin();
	display.clearDisplay(clrWhite);
	display.setForeground(clrBlack);


	while (true) {
		display.clearDisplay(clrWhite);


		drawAsteroids();
		drawShip(cx, cy, angle);
		moveAsteroids();
		asteroidCollision();
		missCollision();

		display.setForeground(clrWhite);

		char text[100];
		sprintf(text, "Lives: %d", lives);
		display.drawText(text,0,0);

		char myScore[100];
		sprintf(myScore, "Score: %d", score);
		display.drawText(myScore,130,0);

		if (lives == 0) break;

		display.setForeground(clrBlack);


		if (cy > 320-w/2) cy = w/2;
		else if (cy < w/2) cy = 320-w/2;
		else if (cx > 240-w/2) cx = w/2;
		else if (cx < w/2) cx = 240-w/2;


		for(int count = 0; count < 15000000; count++);
	}

	char mystring[15] = "Game Over";
	display.drawText(mystring,90,140);

}