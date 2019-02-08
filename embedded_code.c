#include <REG51.H>

/***FUNCTION DECLATIONS*/
void tankState(unsigned char);
void init_LCD(void);
void set_Settings(void);
void set_Data(void);
void s_Latch(unsigned char);
void write_LCDColumn1(void);
void write_LCDColumn2(void);
void buzzerBeep(void);
void delay_1ms(void);
void delay_100us(void);
void openValve(void);
void closeValve(void);
void clr(void);
void delay_50ms(unsigned int );

typedef enum{A_STATE,B_STATE,C_STATE,D_STATE,U_STATE} TANK_STATE;

/*PORT PINS FOR THE TRANSISTORS OUTPUT DIPPED IN WATER*/
sbit WATER_FULL=P3^4;
sbit WATER_ALMOST=P3^5;
sbit WATER_HALF=P3^6;
sbit WATER_QUARTER=P3^7;


/*DECIDING PINS TO AVOID CPU OVERHEAD*/
sbit PIN_A=P1^3;
sbit PIN_B=P1^4;
sbit PIN_C=P1^5;
sbit PIN_D=P1^6;
sbit PIN_E=P1^7;

/*LCD PORTS & PINS DECLARATION*/
#define LCD P2
sbit RW=P3^1;
sbit RS=P3^0;
sbit EN=P3^3;

/*MOTOR PINS*/
sbit MOTOR_1=P1^0;
sbit MOTOR_2=P1^1;

/*BUZZER PIN*/
sbit BUZZER=P1^2;



/*=============================================================================
=============================================================================*/
unsigned char ex0_isr_counter = 0,
	lv_Full[11]="Level: Full",
	lv_ThQuatz[15]="Level: 3/4 Full",
	lv_Half[11]="Level: Half",
	lv_Quatz[14]="Level: Quarter",
	lv_Ukwn[14]="Level: Unknown",
	lv_MotorUkwn[14]="MOTOR DISABLED",
	lv_MotorON[10]=	"Motor: ON ",
	lv_MotorOFF[10]="Motor: OFF",
	lv_MotorCLOSE[10]="Closing Vs";

void ex0_isr (void) interrupt 0
{
	ex0_isr_counter++;   // Increment the count
}

/*=============================================================================
=============================================================================*/
void main (void)
{
	/*-----------------------------------------------
	Configure INT0 (external interrupt 0) to generate
	an interrupt on the falling-edge of /INT0 (P3.2).
	Enable the EX0 interrupt and then enable the
	global interrupt flag.
	-----------------------------------------------*/
	IT0 = 1;   // Configure interrupt 0 for falling edge on /INT0 (P3.2)
	EX0 = 1;   // Enable EX0 Interrupt
	EA = 1;    // Enable Global Interrupt Flag
	/*-----------------------------------------------*/
	
	WATER_FULL=0;
	WATER_ALMOST=0;
	WATER_HALF=0;
	WATER_QUARTER=0;
	MOTOR_1=0;
	MOTOR_2=0;
	
	PIN_A=1;
	PIN_B=1;
	PIN_C=1;
	PIN_D=1;
	
	init_LCD();
	
/*-----------------------------------------------
Wait forever.
-----------------------------------------------*/
while (1)
  {
		if(WATER_FULL==1){
			if(PIN_A==1)
				tankState('A');
			continue;
		}
		else if(WATER_ALMOST==1){
			if(PIN_B==1)
				tankState('B');
			continue;
		}
		else if(WATER_HALF==1){
			if(PIN_C==1)
				tankState('C');
			continue;
		}
		else if(WATER_QUARTER==1){
			if(PIN_D==1)
				tankState('D');
			continue;
		}
		else{
			if(PIN_E==1)
				tankState('U');
			continue;
		}
  }
}

/*=============================================================================
=============================================================================*/
void tankState(unsigned char state){
	unsigned char c;
	
	switch(state){
		case 'A':
			buzzerBeep();
			clr();
			write_LCDColumn1();
			for(c=0;c<11;c++){
				LCD=lv_Full[c];
				s_Latch(1);
			}
			
			closeValve();
			
			write_LCDColumn2();
			for(c=0;c<10;c++){
				LCD=lv_MotorOFF[c];
				s_Latch(1);
			}
			
			PIN_A=0;
			PIN_B=1;
			PIN_C=1;
			PIN_D=1;
			PIN_E=1;
		break;
			
		case 'B':
			clr();
			write_LCDColumn1();
			for(c=0;c<15;c++){
				LCD=lv_ThQuatz[c];
				s_Latch(1);
			}
			
			write_LCDColumn2();
			for(c=0;c<10;c++){
				if(MOTOR_1==0)
					LCD=lv_MotorOFF[c];
				else
					LCD=lv_MotorON[c];
				s_Latch(1);
			}
			
			PIN_A=1;
			PIN_B=0;
			PIN_C=1;
			PIN_D=1;
			PIN_E=1;
		break;
			
		case 'C':
			clr();
			write_LCDColumn1();
			for(c=0;c<11;c++){
				LCD=lv_Half[c];
				s_Latch(1);
			}
			
			write_LCDColumn2();
			for(c=0;c<10;c++){
				if(MOTOR_1==0)
					LCD=lv_MotorOFF[c];
				else
					LCD=lv_MotorON[c];
				s_Latch(1);
			}
			
			PIN_A=1;
			PIN_B=1;
			PIN_C=0;
			PIN_D=1;
			PIN_E=1;
		break;
			
		case 'D':
			clr();
			write_LCDColumn1();
			for(c=0;c<14;c++){
				LCD=lv_Quatz[c];
				s_Latch(1);
			}
			
			write_LCDColumn2();
			for(c=0;c<10;c++){
				LCD=lv_MotorON[c];
				s_Latch(1);
			}
			openValve();
			
			PIN_A=1;
			PIN_B=1;
			PIN_C=1;
			PIN_D=0;
			PIN_E=1;
		break;
			
		default:
			write_LCDColumn1();
			for(c=0;c<14;c++){
				LCD=lv_Ukwn[c];
				s_Latch(1);
			}
			closeValve();
			write_LCDColumn2();
			for(c=0;c<14;c++){
				LCD=lv_MotorUkwn[c];
				s_Latch(1);
			}
			
			PIN_A=1;
			PIN_B=1;
			PIN_C=1;
			PIN_D=1;
			PIN_E=0;
			break;
	}
}

void init_LCD(void){
	RW=0;
	RS=0;
	EN=1;
	
	delay_1ms();
	
	set_Settings();
	LCD=0x38;
	s_Latch(4);
	
	LCD=0x0F;
	s_Latch(1);
	
}
void set_Settings(void){
	RW=0;
	RS=0;
}
void set_Data(void){
	RW=0;
	RS=1;
}
void s_Latch(unsigned char c){
	unsigned char z;
	for(z=0;z<c;z++){
		EN=1;
		delay_50ms(2);		//give LCD some time
		EN=0;
	}
}
void write_LCDColumn1(void){
	set_Settings();
	LCD=0x80;
	s_Latch(1);
	
	LCD=0x0;
	set_Data();
	s_Latch(1);
}
void write_LCDColumn2(void){
	set_Settings();
	LCD=0xC0;
	s_Latch(1);
	
	LCD=0x0;
	set_Data();
	s_Latch(1);
}
void clr(void){
	delay_1ms();
	set_Settings();
	LCD=0x1;
	s_Latch(1);
}

void buzzerBeep(void){
	BUZZER=0;
	delay_50ms(3);
	BUZZER=1;
}

 /*=============================================================================
	CIRCUIT DELAYS, FOR GENERATING DELAYS IN THE CIRCUIT, EACH FUNCTION DELAYS,
	ARE 1mS, 100uS, 50uS & 1uS RESPECTIVELY. MAX DELAY ACHIAVABLE FOR 8051 OPERATING,
	AT TMOD 1 16BIT IS 65mS AND MIN DELAY IS 1us
=============================================================================*/
void delay_1ms(void){
	
/*Timer 0, mode 1(16-bit mode no Auto-reload) Selected*/
	TMOD&=0x0F; //CLEAR all Timer 0(T0) bits (T1 Unchanged)
	TMOD|=0x01;	//SET required bits (i.e. 1111, en mode 1)
	
	/* Values for 1mS Delay*/
	TH0=0xFC;	//the high byte
	TL0=0x66; //the low byte
	
	TF0=0;	//initialise by clearing timer 0 flag
	TR0=1;	//start the timer 0
	
	while(TF0==0);	//Loop until timer 0 overflows/rolls over(TF0==1)
	TR0=0;	//stop timer 0
	TF0=0;	//clear timer 0 flag
}
void delay_100us(void){
	
	/*Timer 0, mode 1(16-bit mode no Auto-reload) Selected*/
	TMOD&=0x0F; //CLEAR all Timer 0(T0) bits (T1 Unchanged)
	TMOD|=0x01;	//SET required bits (i.e. 1111, en mode 1)
	
	/* Values for 100uS Delay*/
	TH0=0xFF;	//the high byte
	TL0=0xA4; //the low byte
	
	TF0=0;	//initialise by clearing timer 0 flag
	TR0=1;	//start the timer 0
	
	while(TF0==0);	//Loop until timer 0 overflows/rolls over(TF0==1)
	TR0=0;	//stop timer 0
	TF0=0;	//clear timer 0 flag
}

/*-----------------------------------------------
Function thats responsible for creating a 50 ms 
time delay in the MCU
-----------------------------------------------*/
void delay_50ms(unsigned int x){
  unsigned char c;
	 
	for(c=0;c<x;c++){
		TMOD&=0x0F;		//;Timer 0, mode 1(16-bit mode
		TMOD|=0x01;		//;Timer 0, mode 1(16-bit mode
		
		TL0=0xFD;			//the low byte
		TH0=0x4B;			//the high byt
		
		TR0=1;				//start the timer 0
		while (TF0==0);//monitor timer flag 0 until it rolls over
			TR0=0;			//stop timer 0
			TF0=0;			//clear timer 0 flag
	}
}
void openValve(void){
	MOTOR_1=1;
	MOTOR_2=0;
}
void closeValve(void){
	unsigned char c;
	MOTOR_1=0;
	MOTOR_2=1;
	
	write_LCDColumn2();
			for(c=0;c<10;c++){
				LCD=lv_MotorCLOSE[c];
				s_Latch(1);
			}
	
	delay_50ms(200);
	
	MOTOR_1=0;
	MOTOR_2=0;
}

