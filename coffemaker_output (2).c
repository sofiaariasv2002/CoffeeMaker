/*
Coffemaker microcontroller outputs
*/

//Librarys
#include <MKL25Z4.H>
#include <stdio.h>

//Definitions
#define RS 0x04 // PTA2 mask
#define RW 0x10 // PTA4 mask
#define EN 0x20 // PTA5 mask

int flag = 0; 
unsigned char cup1[] = {  
    0x09,
    0x1B,
    0x12,
    0x00,
    0x1F,
    0x1F,
    0x1F,
    0x0E
};
unsigned char cup2[] ={  
    0x00,
    0x00,
    0x00,
    0x00,
    0x1C,
    0x04,
    0x1C,
    0x00
};

//Functions
void delayMs(int n);
void init(void);
void LED_set(int value);
char keypad_getkey(void);
void LCD_ready(void);
void LCD_command(unsigned char command);
void LCD_command_noWait(unsigned char command);
void LCD_custom(unsigned char ch[],int pos);
int  bomb_IO(char bomb,int time,int squares,const char *drink);
int pwm(int valor);

void delayMs(int n) {
    int i;
    SysTick->LOAD = 41940 - 1;
    SysTick->CTRL = 0x5; // Enable the timer and choose sysclk as the clock source
    for(i = 0; i < n; i++) {
        while((SysTick->CTRL & 0x10000) == 0)/* wait until the COUNT flag is set */{ }
    }
}
void init(void){
//CLks
    SIM->SCGC5 |= 0x0200; // enable clock to Port A
    SIM->SCGC5 |= 0x0400; // enable clock to Port B
    SIM->SCGC5 |= 0x0800; // enable clock to Port C
    SIM->SCGC5 |= 0x1000; // enable clock to Port D
    SIM->SCGC5 |= 0x2000; // enable clock to Port E
//IRQs
    __disable_irq(); // disable all IRQs
//LCD Initialization
    PORTD->PCR[0] = 0x100;/* make PTD0 pin as GPIO */
    PORTD->PCR[1] = 0x100;/* make PTD1 pin as GPIO */
    PORTD->PCR[2] = 0x100;/* make PTD2 pin as GPIO */
    PORTD->PCR[3] = 0x100;/* make PTD3 pin as GPIO */
    PORTD->PCR[4] = 0x100;/* make PTD4 pin as GPIO */
    PORTD->PCR[5] = 0x100;/* make PTD5 pin as GPIO */
    PORTD->PCR[6] = 0x100;/* make PTD6 pin as GPIO */
    PORTD->PCR[7] = 0x100;/* make PTD7 pin as GPIO */
    PTD->PDDR = 0xFF;  /* make PTD7-0 as output pins */   
    SIM->SCGC5 |= 0x0200; /* enable clock to Port A */
    PORTA->PCR[2] = 0x100;/* make PTA2 pin as GPIO */
    PORTA->PCR[4] = 0x100;/* make PTA4 pin as GPIO */
    PORTA->PCR[5] = 0x100;/* make PTA5 pin as GPIO */
    PTA->PDDR |= 0x34; /make PTA5, 4, 2 as output pins/
    delayMs(20); /* initialization sequence */
    /* LCD does not respond to status poll */
    LCD_command_noWait(0x30); 
    delayMs(5);
    LCD_command_noWait(0x30);
    delayMs(1);
    LCD_command_noWait(0x30);
    /* set 8-bit data, 2-line, 5x7 font */
    LCD_command(0x38); 
    /* move cursor right */
    LCD_command(0x06); 
    /* clear screen, move cursor to home */
    LCD_command(0x01);
    /* turn on display, cursor blinking */
    LCD_command(0x0F);
    //lcd custom
    LCD_custom(cup1,0);
    LCD_custom(cup2,1);
///PinPad Initialization
    PORTC->PCR[0] = 0x103; /* PTD0, GPIO, enable pullup*/
    PORTC->PCR[1] = 0x103; /* PTD1, GPIO, enable pullup*/
    PORTC->PCR[2] = 0x103; /* PTD2, GPIO, enable pullup*/
    PORTC->PCR[3] = 0x103; /* PTD3, GPIO, enable pullup*/
    PORTC->PCR[4] = 0x103; /* PTD4, GPIO, enable pullup*/
    PORTC->PCR[5] = 0x103; /* PTD5, GPIO, enable pullup*/
    PORTC->PCR[6] = 0x103; /* PTD6, GPIO, enable pullup*/
    PORTC->PCR[7] = 0x103; /* PTD7, GPIO, enable pullup*/
    PTC->PDDR = 0x0F; /* make PTD7-0 as input pins */
//Interruption intiation
    //configure PTA12 for interrupt
    PORTA->PCR[12] |= 0x00100; // make it GPIO
    PORTA->PCR[12] |= 0x00003; // enable pull-up
    PTA->PDDR &= ~0x1000; // make pin input
    PORTA->PCR[12] &= ~0xF0000; // clear interrupt selection
    PORTA->PCR[12] |= 0xA0000; // enable falling edge INT
    // configure PTA13 for interrupt
    PORTA->PCR[13] |= 0x00100; // make it GPIO
    PORTA->PCR[13] |= 0x00003; // enable pull-up
    PTA->PDDR &= ~0x2000; // make pin input
    PORTA->PCR[13] &= ~0xF0000; // clear interrupt selection
    PORTA->PCR[13] |= 0xA0000; // enable falling edge INT

    NVIC->ISER[0] |= 0x40000000; /* enable INT30 (bit 30 of ISER[0]) */
    __enable_irq(); /* global enable IRQs */
    
// Bomba chocolate
    //PTB8
    PORTB->PCR[8] |= 0x100;  //make pin as GPIO
    PTB->PDDR |= 0x100; // make pin as output
// Bomba agua
    //PTB9
    PORTB->PCR[9] |= 0x100;  //make pin as GPIO
    PTB->PDDR |= 0x200; // make pin as output
// Bomba leche
    //PTE0
    PORTE->PCR[0] |= 0x100;  //make pin as GPIO
    PTE->PDDR |= 0x1; // make pin as output
// Bomba ...
    //PTE1
    PORTE->PCR[1] |= 0x100;  //make pin as GPIO
    PTE->PDDR |= 0x2; // make pin as output
};
char keypad_getkey(void){
//
    int row, col;
    const char row_select[] = {0x01, 0x02, 0x04, 0x08}; 
    /* one row is active */
/* check to see any key pressed */
    PTC->PDDR |= 0x0F; /* enable all rows */
    PTC->PCOR = 0x0F;
    delayMs(2); /* wait for signal return */
    col = PTC-> PDIR & 0xF0; /* read all columns */
    PTC->PDDR = 0; /* disable all rows */
    if (col == 0xF0)
        return 99; /* no key pressed */
/* If a key is pressed, we need find out which key.*/ 
    for (row = 0; row < 4; row++){ 
        PTC->PDDR = 0; /* disable all rows */
        PTC->PDDR |= row_select[row]; /* enable one row */
        PTC->PCOR = row_select[row]; /* drive active row low*/
        delayMs(2); /* wait for signal to settle */
        col = PTC->PDIR & 0xF0; /* read all columns */
        if (col != 0xF0) break; 
        /* if one of the input is low, some key is pressed. */
    }
    PTC->PDDR = 0; /* disable all rows */
    if (row == 4)  return 99; /* if we get here, no key is pressed */
/* gets here when one of the rows has key pressed*/ 
/check which column it is/
    if (col == 0xE0) return row*4+ 1; /* key in column 0 */
    if (col == 0xD0) return row*4+ 2; /* key in column 1 */
    if (col == 0xB0) return row*4+ 3; /* key in column 2 */
    if (col == 0x70) return row*4+ 4; /* key in column 3 */
    return 99; /* just to be safe */
}
void LCD_ready(void){
    /* This function waits until LCD controller is ready to accept a new command/data before returns. */
    char status;
    PTD->PDDR = 0; /* PortD input */
    PTA->PCOR = RS; /* RS = 0 for status */
    PTA->PSOR = RW; /* R/W = 1, LCD output */
    do { /* stay in the loop until it is not busy */
        PTA->PSOR = EN; /* raise E */
        delayMs(0);
        status = PTD->PDIR; /* read status register */
        PTA->PCOR = EN;
        delayMs(0); /* clear E */
    } while (status & 0x80); /* check busy bit */
    PTA->PCOR = RW; /* R/W = 0, LCD input */
    PTD->PDDR = 0xFF; /* PortD output */
}
void LCD_command(unsigned char command){
    LCD_ready(); /* wait until LCD is ready */
    PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
    PTD->PDOR = command;
    PTA->PSOR = EN; /* pulse E */
    delayMs(0);
    PTA->PCOR = EN;
}
void LCD_command_noWait(unsigned char command){
    PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
    PTD->PDOR = command;
    PTA->PSOR = EN; /* pulse E */
    delayMs(0);
    PTA->PCOR = EN; 
}
void LCD_data(unsigned char data){
    LCD_ready(); /* wait until LCD is ready */
    PTA->PSOR = RS; /* RS = 1, R/W = 0 */
    PTA->PCOR = RW;
    PTD->PDOR = data;
    PTA->PSOR = EN; /* pulse E */ 
    delayMs(0);
    PTA->PCOR = EN;
}
void LCD_message(const char* message){
   while (*message != '\0') {
        LCD_data(*message);
        message++;
    }
}  
void LCD_custom(unsigned char ch[],int pos){
    delayMs(1);
    pos &= 0x7; 
    LCD_command(0x40 | (pos << 3));
    for (int i = 0; i<8; i++) {
        LCD_data(ch[i]);
    }
    LCD_command(1);
}

void print_fl( const char *_drink){
    LCD_command(1);
    LCD_command(2);
    LCD_message(_drink);
}

void printProgress(int _squares){
/*  Cargando animation   */
    LCD_command(0xC0);
    for (int i = 0; i < _squares; i++){
        LCD_data(0);
        LCD_data(1);
    }
}

int bomb_IO(char bomb,int time,int squares,const char *drink){
    while(time > 0){
                if(flag == 0){ 
                    squares++;
                    print_fl(drink);
                    delayMs(50);
                    printProgress(squares);
                    if ( bomb == 1){
                        PTB->PDOR |= 0x200;
                    }   else if ( bomb == 2) {
                        PTE->PDOR |= 0x1;
                    }   else if ( bomb == 3) {
                        PTB->PDOR |= 0x100;
                    }else if ( bomb == 4 ) {
                        pwm(775);
                        delayMs(2000);
                        pwm(100);
                    }
                    time--;     
                } else if (flag == 1){
                    LCD_command(2);
                    LCD_message("Pause...    ");
                    PTB->PDOR &= ~0x100;
                    PTB->PDOR &= ~0x200;
                    PTE->PDOR &= ~0x1;
                }
                delayMs(1000);
            }
    PTB->PDOR &= ~0x100;
    PTB->PDOR &= ~0x200;
    PTE->PDOR &= ~0x1;
 return squares;
}
int pwm(int valor){
    int pulseWidth = 0;
    SIM->SCGC5 |= 0x2000; /* enable clock to Port E */
    PORTE->PCR[20] = 0x0300; /* PTD1 used by TPM1 */
    SIM->SCGC6 |= 0x02000000; /* enable clock to TPM1 */
    SIM->SOPT2 |= 0x01000000; /* use MCGFLLCLK as timer counter clock */
    TPM1->SC = 0; /* disable timer */
    /* edge-aligned, pulse high */
    TPM1->CONTROLS[0].CnSC =  0x20|0x08;  /* Set up modulo register for 60 Hz */
    TPM1->MOD = 7499; 
    TPM1->CONTROLS[0].CnV = valor; /* Set up channel value for 50% dutycycle */
    TPM1->SC = 0X0F; /* enable TPM0 with prescaler /128 */
return 0;
}


//Interruptions
extern "C" void PORTA_IRQHandler(void) {
    while (PORTA->ISFR & 0x3000) {
        if (PORTA->ISFR & 0x2000) {
            flag = 1; 
            PORTA->ISFR = 0x2000; /* clear interrupt flag */
        }
        if (PORTA->ISFR & 0x1000) {
            flag = 0;
            PORTA->ISFR = 0x1000; /* clear interrupt flag */
        } 
    } 
    PORTA->ISFR = 0x00000000; /* clear interrupt flag */    
}

//Main programm
int main(void){
//Setup    
    unsigned char key = 0;
    const char *drink;
    int squares;
    int water = 0;
    int coffee = 0;
    int milk = 0;
    int chocolate = 0;
    init();
    delayMs(500);
//Loop    
    for(;;) {
        squares = 0;
        print_fl("Select coffee");
        LCD_command(0xC0);
        LCD_message("  1:M 2:C 3:A");
        delayMs(500);

        
            key = keypad_getkey();
            if (key == 1)
            {
                drink     =  "Mokka";
                print_fl(drink);
                delayMs(1000);
                water     =  0;
                milk      =  0;
                chocolate =  0;
                coffee    =  2;
            } else if (key == 2)
            {
                drink     =  "Capuchino";
                print_fl(drink);
                delayMs(1000);
                water     =  10;                
                milk      =  5;
                chocolate =  0;
                coffee    =  0;
            } else if (key == 3)
            { 
                drink     =  "Americano";
                print_fl(drink);
                delayMs(1000);
                water     =  5;
                milk      =  10;
                chocolate =  0;
                coffee    =  0;
            }
        if (key == 13){
        squares = bomb_IO(1, water, squares, drink);
        squares = bomb_IO(2, milk, squares, drink);      
        squares = bomb_IO(3, chocolate, squares, drink); 
        squares = bomb_IO(4, coffee, squares, drink);
        print_fl("Done !");
        delayMs(3000);
        }
    }
}