/*
Coffemaker microcontroller inputs
*/

#include <MKL25Z4.h>

void delayMs(int n) {
    int i;
    SysTick->LOAD = 41940 - 1;
    SysTick->CTRL = 0x5; // Enable the timer and choose sysclk as the clock source
    for(i = 0; i < n; i++) {
        while((SysTick->CTRL & 0x10000) == 0)/* wait until the COUNT flag is set */{ }
    }
}
void delayUs(int n) {
    int i;
    SysTick->LOAD = 10485 - 1;
    SysTick->CTRL = 0x5; // Enable the timer and choose sysclk as the clock source
    for(i = 0; i < n; i++) {
        while((SysTick->CTRL & 0x10000) == 0)/* wait until the COUNT flag is set */{}
    }
}

//Initialization functions
void init() {
//Clk
    SIM->SCGC5 |= 0x400;     // enable clock to Port B 
    SIM->SCGC5 |= 0x0800;    // enable clock to Port C
    SIM->SCGC5 |= 0x1000;    // enable clock to Port D
//Sensor 1
    //PTC12 TRIG1
    PORTC->PCR[12] = 0x100;  // make pin as GPIO
    PTC->PDDR |= 0x1000;     // make pin as output
    //PTC13 ECHO1
    PORTC->PCR[13] = 0x100;  // make pin as GPIO
    PTC->PDDR &= ~0x2000;    // make pin as input
//Sensor 2
    //PTC10 TRIG2
    PORTC->PCR[16] = 0x100;  // make pin as GPIO
    PTC->PDDR |= 0x10000;    // make pin as output
    //PTC11 ECHO2
    PORTC->PCR[17] = 0x100;  // make pin as GPIO
    PTC->PDDR &= ~0x20000;   // make pin as input
//Sensor 3
    //PTC4 TRIG2
    PORTC->PCR[4] = 0x100;   // make pin as GPIO
    PTC->PDDR |= 0x10;       // make pin as output
    //PTC5 ECHO2
    PORTC->PCR[5] = 0x100;   // make pin as GPIO
    PTC->PDDR &= ~0x20;      // make pin as input
//Sensor 4
    //PTC0 TRIG2
    PORTC->PCR[0] = 0x100;   // make pin as GPIO
    PTC->PDDR |= 0x1;        // make pin as output
    //PTC1 ECHO2
    PORTC->PCR[1] = 0x100;   // make pin as GPIO
    PTC->PDDR &= ~0x2;       // make pin as input
//Sensor 5
    //PTC8 TRIG2
    PORTC->PCR[8] = 0x100;   // make pin as GPIO
    PTC->PDDR |= 0x100;      // make pin as output
    //PTC9 ECHO2
    PORTC->PCR[9] = 0x100;   // make pin as GPIO
    PTC->PDDR &= ~0x200;     // make pin as input
// LED 1 Amarillo
    //PTD0
    PORTD->PCR[0] |= 0x100;  // make pin as GPIO
    PTD->PDDR |= 0x1;        // make pin as output
// LED 2 Azul
    //PTD5
    PORTD->PCR[5] |= 0x100;  //make pin as GPIO
    PTD->PDDR |= 0x20;       // make pin as output
}

//Read sensors
double readSensor1() {
    double time = 0;
    PTC->PDOR &= ~0x1;    // TRIG1 low
    delayUs(2);
    PTC->PDOR |= 0x1000;  // TRIG1 high
    delayUs(10);
    PTC->PDOR &= ~0x1000; // TRIG1 low
    while (!(PTC->PDIR & 0x2000)); // Wait until ECHO1 activates
    do {
        time = time + 1;
        delayUs(1);
    } while (PTC->PDIR & 0x2000);  // Wait until ECHO1 turns off
    return time;
}
double readSensor2() {
    double time = 0;
    PTC->PDOR &= ~0x10000; // TRIG2 low
    delayUs(2);
    PTC->PDOR |= 0x10000;  // TRIG2 high
    delayUs(10);
    PTC->PDOR &= ~0x10000; // TRIG2 low
    while (!(PTC->PDIR & 0x20000)); // Wait until ECHO2 activates
    do {
        time = time + 1;
        delayUs(1);
    } while (PTC->PDIR & 0x20000);  // Wait until ECHO2 turns off
    return time;
}
double readSensor3() {
    double time = 0;
    PTC->PDOR &= ~0x10; // TRIG3 low
    delayUs(2);
    PTC->PDOR |= 0x10;  // TRIG3 high
    delayUs(10);
    PTC->PDOR &= ~0x10; // TRIG3 low
    while (!(PTC->PDIR & 0x20)); // Wait until ECHO3 activates
    do {
        time = time + 1;
        delayUs(1);
    } while (PTC->PDIR & 0x20);  // Wait until ECHO3 turns off
    return time;
}
double readSensor4() {
    double time = 0;
    PTC->PDOR &= ~0x1;   // TRIG4 low
    delayUs(2);
    PTC->PDOR |= 0x1;    // TRIG4 high
    delayUs(10);
    PTC->PDOR &= ~0x1;   // TRIG4 low
    while (!(PTC->PDIR & 0x2)); // Wait until ECHO4 activates
    do {
        time = time + 1;
        delayUs(1);
    } while (PTC->PDIR & 0x2);  // Wait until ECHO4 turns off
    return time;
}
double readSensor5() {
    double time = 0;
    PTC->PDOR &= ~0x100;   // TRIG5 low
    delayUs(2);
    PTC->PDOR |= 0x100;    // TRIG5 high
    delayUs(10);
    PTC->PDOR &= ~0x100;   // TRIG5 low
    while (!(PTC->PDIR & 0x200)); // Wait until ECHO5 activates
    do {
        time = time + 1;
        delayUs(1);
    } while (PTC->PDIR & 0x200);  // Wait until ECHO5 turns off
    return time;
}
int main() {
    init();
    for(;;){
        double d  =  5;
        double s1 = readSensor1();
        double s2 = readSensor2();
        double s3 = readSensor3();
        double s4 = readSensor4();
        double s5 = readSensor5();
        /*
        if (s1 > d)
        {
            PTD->PDOR |= 0x1;
        }
        if (s2 > d)
        {
            PTD->PDOR |= 0x1;
        }
        if (s3 > d)
        {
            PTD->PDOR |= 0x1;
        }
        if (s4 > d)
        {
            PTD->PDOR |= 0x1;
        } */
        if (s5 > d)
        {
            PTD->PDOR |= 0x1;
        } 
        
        /*if((s1 < d)&&(s2 < d)&&(s3 < d)&&(s4 < d)&&(s5 < d))
        {
            PTD->PDOR |= 0x20;
        }*/
        if(s5 < d)
        {
            PTD->PDOR |= 0x20;
        }
        delayMs(1000);
        PTD->PDOR &= ~0x1;
        PTD->PDOR &= ~0x20;
    }
}