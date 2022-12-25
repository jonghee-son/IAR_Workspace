////////////////////////////////////////////
// 과제명: Maze Runner
// 과제개요:  Joy-stick을 이용한 미로탈출 게임 작성
// 사용한 하드웨어(기능): GPIO, Joy-stick, GLCD, Buzzer, SW
// 제출일: 2023. 1. 13
// 학번: 2019130023
// 이름: 손종희
////////////////////////////////////////////

#include "stm32f4xx.h"
#include "GLCD.h"

#define SW0_PUSH        0xFE00  //PH8
#define SW1_PUSH        0xFD00  //PH9
#define SW2_PUSH        0xFB00  //PH10
#define SW3_PUSH        0xF700  //PH11
#define SW4_PUSH        0xEF00 //PH12
#define SW7_PUSH        0x7F00 //PH15

// NO Joy-Button : 0x03E0 : 0000 0011 1110 0000 
		        //Bit No      FEDC BA98 7654 3210
#define NAVI_PUSH	0x03C0  //PI5 0000 0011 1100 0000 
#define NAVI_UP		0x03A0  //PI6 0000 0011 1010 0000 
#define NAVI_DOWN       0x0360  //PI7 0000 0011 0110 0000 
#define NAVI_RIGHT	0x02E0  //PI8 0000 0010 1110 0000 
#define NAVI_LEFT	0x01E0  //PI9 0000 0001 1110 0000 

void _GPIO_Init(void);
uint16_t KEY_Scan(void);	// Switch 입력 함수
uint16_t JOY_Scan(void);	// Joystick 입력 함수

void BEEP(void);
void DisplayInitScreen(void);
void DelayMS(unsigned short wMS);
void DelayUS(unsigned short wUS);

uint8_t	SW0_Flag, SW1_Flag;	// SW0_Flag: Joystick UP 입력이 홀수번째인지 짝수번째인지를 기록한 변수 
				// SW1_Flag: Joystick DOWN 입력이 홀수번째인지 짝수번째인지를 기록한 변수 
int main(void) {
    _GPIO_Init(); 	// GPIO (LED,SW,Buzzer,Joy stick) 초기화
    LCD_Init();		// LCD 모듈 초기화
    DelayMS(10);       // Set time margin for GLCD init
    // Initial X & Y coordinate (Pixel)
    int x = 8;
    int y = 23;

    DisplayInitScreen();	// LCD 초기화면
    GPIOG->ODR &= ~0x00FF;	// LED 초기값: LED0~7 Off
        
    GPIOG->ODR |= 0x0001; // LED0 ON
    
    LCD_SetBrushColor(RGB_BLUE); // Set color for cursor
        
    LCD_DrawFillRect(x,y,6,6); // Initial cursor placement
        
    while(1) {
        // Initial X & Y coordinate (Pixel)
        x = 8;
        y = 23;
        // Variables counting key input
        int U = 0, D = 0, L = 0, R = 0;
        int LU = 0, LD = 0, LL = 0, LR = 0;
        // Scan for SW
        switch(KEY_Scan()) {
        case SW7_PUSH:
            // Start when SW7 pushed
            // Initialize STATUS as (0,0)
            LCD_DisplayChar(4,22,'0'); 
            LCD_DisplayChar(4,24,'0');
            BEEP();
            GPIOG->ODR |= 0x0080; // LED7 ON
            while(!(x == 80 && y == 95)) {
                if (KEY_Scan() == SW0_PUSH) {
                    // Reset
                    goto a;
                }
                // Display Up, Down, Left, Right count
                LCD_DisplayChar(10,2,(U+0x30));
                LCD_DisplayChar(10,7,(D+0x30));
                LCD_DisplayChar(10,12,(L+0x30));
                LCD_DisplayChar(10,17,(R+0x30));
                // Display current cursor placement
                LCD_DisplayChar(4,22,(((x-8)/18) + 0x30));
                LCD_DisplayChar(4,24,(((y-23)/18) + 0x30));
                // Scan for joystick input
                switch(JOY_Scan()) {
                    case NAVI_UP:
                        GPIOG->ODR &= ~0x007E; // Turn off all LED except LED0 & LED7
                        if(y > 23) {
                            GPIOG->ODR |= 0x0002; // LED1 ON
                            U+=1;
                            LU+=1;
                            LD-=1;
                            BEEP();
                            LCD_SetBrushColor(RGB_WHITE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Erase previous cursor
                            LCD_DrawHorLine(10, 25 + ((1 - LU) * 18), 72); // Draw horizontal line
                            LCD_DrawVerLine(10 + ((LR) * 18), 25, 72); // Draw vertical line
                            y-=18; // update y coordinate
                            LCD_SetBrushColor(RGB_BLUE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Draw cursor
                        }
                        else {
                            // When cursor goes beyond boundary
                            BEEP();
                            DelayMS(500); // Delay for 0.5 seconds
                            BEEP();
                        }
                        break;
                        
                    case NAVI_DOWN:
                        GPIOG->ODR &= ~0x007E; // Turn off all LED except LED0 & LED7
                        if(y < 95) {
                            GPIOG->ODR |= 0x0004; // LED2 ON
                            D+=1;
                            LD+=1;
                            LU-=1;
                            BEEP();
                            LCD_SetBrushColor(RGB_WHITE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Erase previous cursor
                            LCD_DrawHorLine(10, 25 + ((LD - 1) * 18), 72); // Draw horizontal line
                            LCD_DrawVerLine(10 + ((LR) * 18), 25, 72); // Draw vertical line
                            y+=18; // update y coordinate
                            LCD_SetBrushColor(RGB_BLUE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Draw cursor
                        }
                        else {
                            // Goes beyond boundary
                            BEEP();
                            DelayMS(500); // Delay for 0.5 seconds
                            BEEP();
                        }
                        break;
                    case NAVI_LEFT:
                        GPIOG->ODR &= ~0x007E; // Turn off all LED except LED0 & LED7
                        if(x >  8) {
                            GPIOG->ODR |= 0x0008; // LED3 ON
                            L+=1;
                            LL+=1;
                            LR-=1;
                            BEEP();
                            LCD_SetBrushColor(RGB_WHITE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Erase previous cursor
                            LCD_DrawHorLine(10, 25 + (LD * 18), 72);  // Draw horizontal line
                            LCD_DrawVerLine(10 + ((1 - LL) * 18), 25, 72); // Draw vertical line
                            x-=18; // update x coordinate
                            LCD_SetBrushColor(RGB_BLUE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Draw cursor
                        }
                        else {
                            // Goes beyond boundary
                            BEEP();
                            DelayMS(500); // Delay for 0.5 seconds
                            BEEP();
                        }
                        break;
                    case NAVI_RIGHT:
                        GPIOG->ODR &= ~0x007E; // Turn off all LED except LED0 & LED7
                        if(x < 80) {
                            GPIOG->ODR |= 0x0010; // LED4 ON
                            R+=1;
                            LL-=1;
                            LR+=1;
                            BEEP();
                            LCD_SetBrushColor(RGB_WHITE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Erase previous cursor
                            LCD_DrawHorLine(10, 25 + (LD * 18), 72); // Draw horizontal line
                            LCD_DrawVerLine(10 + ((LR - 1) * 18), 25, 72); // Draw vertical line
                            x+=18; // update x coordinate
                            LCD_SetBrushColor(RGB_BLUE); // Set brush color
                            LCD_DrawFillRect(x,y,6,6); // Draw cursor
                        }
                        else {
                            // Goes beyond boundary
                            BEEP();
                            DelayMS(500); // Delay for 0.5 seconds
                            BEEP();
                        }
                        break;
                    }
                }
                // Final variable display update before ending sequence
                LCD_DisplayChar(10,2,(U+0x30));
                LCD_DisplayChar(10,7,(D+0x30));
                LCD_DisplayChar(10,12,(L+0x30));
                LCD_DisplayChar(10,17,(R+0x30));
                LCD_DisplayChar(4,22,(((x-8)/18) + 0x30));
                LCD_DisplayChar(4,24,(((y-23)/18) + 0x30));
                // Indicate that the game ended
                DelayMS(500); // Delay for 0.5 seconds
                BEEP();
                DelayMS(500); // Delay for 0.5 seconds
                BEEP();
                DelayMS(500); // Delay for 0.5 seconds
                BEEP();
                // Block any input except SW0_PUSH
                while (KEY_Scan() != SW0_PUSH) {
                }
                // Reset
                goto a;
            break;
        case SW0_PUSH:
            // Reset sequence
            a:
            x = 8;
            y = 23;
            GPIOG->ODR &= ~0x00FE; // Turn off all LED except LED0
            DisplayInitScreen(); // Reset GLCD
            LCD_DrawFillRect(x,y,6,6); // Draw initial cursor
            break;
        }
    }
}

/* GPIO (GPIOG(LED), GPIOH(Switch), GPIOF(Buzzer), GPIOI(Joy stick)) 초기 설정	*/
void _GPIO_Init(void) {
	// LED (GPIO G) 설정
    	RCC->AHB1ENR	|=  0x00000040;	// RCC_AHB1ENR : GPIOG(bit#6) Enable							
	GPIOG->MODER 	|=  0x00005555;	// GPIOG 0~7 : Output mode (0b01)						
	GPIOG->OTYPER	&= ~0x00FF;	// GPIOG 0~7 : Push-pull  (GP8~15:reset state)	
 	GPIOG->OSPEEDR 	|=  0x00005555;	// GPIOG 0~7 : Output speed 25MHZ Medium speed 
    
	// SW (GPIO H) 설정 
	RCC->AHB1ENR    |=  0x00000080;	// RCC_AHB1ENR : GPIOH(bit#7) Enable							
	GPIOH->MODER 	&= ~0xFFFF0000;	// GPIOH 8~15 : Input mode (reset state)				
	GPIOH->PUPDR 	&= ~0xFFFF0000;	// GPIOH 8~15 : Floating input (No Pull-up, pull-down) :reset state

	// Buzzer (GPIO F) 설정 
    	RCC->AHB1ENR	|=  0x00000020; // RCC_AHB1ENR : GPIOF(bit#5) Enable							
	GPIOF->MODER 	|=  0x00040000;	// GPIOF 9 : Output mode (0b01)						
	GPIOF->OTYPER 	&= ~0x0200;	// GPIOF 9 : Push-pull  	
 	GPIOF->OSPEEDR 	|=  0x00040000;	// GPIOF 9 : Output speed 25MHZ Medium speed 

	//Joy Stick SW(PORT I) 설정
	RCC->AHB1ENR 	|= 0x00000100;	// RCC_AHB1ENR GPIOI Enable
	GPIOI->MODER 	&= ~0x000FFC00;	// GPIOI 5~9 : Input mode (reset state)
	GPIOI->PUPDR    &= ~0x000FFC00;	// GPIOI 5~9 : Floating input (No Pull-up, pull-down) (reset state)
}	

/* GLCD 초기화면 설정 */
void DisplayInitScreen(void) {
    LCD_Clear(RGB_WHITE);		// 화면 클리어
    LCD_SetFont(&Gulim7);		// 폰트 : 굴림 7
    LCD_SetBackColor(RGB_YELLOW);	// 글자배경색 : Yellow
    LCD_SetTextColor(RGB_BLACK);	// 글자색 : Black
    LCD_DisplayText(0,0,"Maze Runner");	// Title
    
    LCD_SetBackColor(RGB_WHITE); // Background Color : White
    
    // Subtitles
    LCD_DisplayText(1,0,"(0,0)");
    LCD_DisplayText(1,11,"(4,0)");
    LCD_DisplayText(9,0,"(0,4)");
    LCD_DisplayText(9,11,"(4,4)");
    LCD_DisplayText(10,0,"U:0 ,D:0 ,L:0 ,R:0");
        
    LCD_SetPenColor(RGB_BLACK);
    
    LCD_DisplayText(2,14,"RESET(SW0)");
    LCD_DisplayText(3,14,"START(SW7)");
    LCD_DisplayText(4,14,"STATUS:(X,Y)");
    
    // Draws maze
    for (int i = 0; i < 5; i++) {
        LCD_DrawHorLine(10, 25 + (i * 18), 72); 
        LCD_DrawVerLine(10 + (i * 18), 25, 72);
    }
}

/* Joystick switch가 입력되었는지를 여부와 어떤 Joystick switch가 입력되었는지의 정보를 return하는 함수  */ 
uint8_t joy_flag = 0;
uint16_t JOY_Scan(void)	// input joy stick NAVI_* 
{ 
	uint16_t key;
	key = GPIOI->IDR & 0x03E0;	// any key pressed ?
	if(key == 0x03E0)		// if no key, check key off
	{  	if(joy_flag == 0)
        		return key;
      		else
		{	DelayMS(10);
        		joy_flag = 0;
        		return key;
        	}
    	}
  	else				// if key input, check continuous key
	{	if(joy_flag != 0)	// if continuous key, treat as no key input
        		return 0x03E0;
      		else			// if new key,delay for debounce
		{	joy_flag = 1;
			DelayMS(10);
 			return key;
        	}
	}
}

/* Switch가 입력되었는지를 여부와 어떤 switch가 입력되었는지의 정보를 return하는 함수  */ 
uint8_t key_flag = 0;
uint16_t KEY_Scan(void)	// input key SW0 - SW7 
{ 
	uint16_t key;
	key = GPIOH->IDR & 0xFF00;	// any key pressed ?
	if(key == 0xFF00)		// if no key, check key off
	{  	if(key_flag == 0)
        		return key;
      		else
		{	DelayMS(10);
        		key_flag = 0;
        		return key;
        	}
    	}
  	else				// if key input, check continuous key
	{	if(key_flag != 0)	// if continuous key, treat as no key input
        		return 0xFF00;
      		else			// if new key,delay for debounce
		{	key_flag = 1;
			DelayMS(10);
 			return key;
        	}
	}
}

/* Buzzer: Beep for 30 ms */
void BEEP(void)			
{ 	
	GPIOF->ODR |=  0x0200;	// PF9 'H' Buzzer on
	DelayMS(30);		// Delay 30 ms
	GPIOF->ODR &= ~0x0200;	// PF9 'L' Buzzer off
}

void DelayMS(unsigned short wMS)
{
	register unsigned short i;
	for (i=0; i<wMS; i++)
		DelayUS(1000);	// 1000us => 1ms
}

void DelayUS(unsigned short wUS)
{
	volatile int Dly = (int)wUS*17;
	for(; Dly; Dly--);
}
