#include <msp430.h> 
#include <stdint.h>

int onewire_reset();
void onewire_write_bit(int bit);
int onewire_read_bit();
void onewire_write_byte(uint8_t byte);
uint8_t onewire_read_byte();
void onewire_line_low();
void onewire_line_high();
void onewire_line_release();
float GetData(void);
void motor(void);
void ow_portsetup();
float ReadDS1820 ( void );
int onewire_reset();
void onewire_write_bit(int bit);
int onewire_read_bit();
void onewire_write_byte(uint8_t byte);
uint8_t onewire_read_byte();
void mostrar_temp(int,int);
void display (void);
void DELAY_MS(int);
void DELAY_US(int);
void DELAY_S(int);
void DELAY_M(int);

#define OWPORTDIR P2DIR
#define OWPORTOUT P2OUT
#define OWPORTIN P2IN
#define OWPORTREN P2REN
#define OWPORTPIN BIT6
#define OW_LO {	OWPORTDIR |= OWPORTPIN;	OWPORTREN &= ~OWPORTPIN; OWPORTOUT &= ~OWPORTPIN; }
#define OW_HI {	OWPORTDIR |= OWPORTPIN;	OWPORTREN &= ~OWPORTPIN; OWPORTOUT |= OWPORTPIN; }
#define OW_RLS { OWPORTDIR &= ~OWPORTPIN; OWPORTREN |= OWPORTPIN; OWPORTOUT |= OWPORTPIN; }
#define HISTERESIS	1
#define TIEMPO_ESPERA_MINUTOS 30
#define CANTIDAD_SEGUNDOS_PARA_MINUTOS 59
#define CANTIDAD_MILI_PARA_SEGUNDOS 999
#define DECENA_MAXIMA_DISPLAY 2
#define UNIDAD_MAXIMA_DISPLAY 5
#define DECENA_MINIMA_DISPLAY 0
#define UNIDAD_MINIMA_DISPLAY 6
#define UNIDAD_INICIO 3
#define DECENA_INICIO 1

#define DS1820_SKIP_ROM             0xCC
#define DS1820_READ_SCRATCHPAD      0xBE
#define DS1820_CONVERT_T            0x44


uint16_t tiempo_encendido=0;
int minutos;
int segundos;
int milisegundos;
int temp_unidad=UNIDAD_INICIO;
int temp_decena=DECENA_INICIO;
int temp_display;
float temperature=0; //TEMPERATURA TOMA SENSOR
int flag=0; //FLAG DELAY MOTOR

int main()
{
	WDTCTL = WDTPW + WDTHOLD;
	BCSCTL1 = CALBC1_16MHZ; 				//Pongo el clock interno del micro en 16MHz
	DCOCTL = CALDCO_16MHZ;				//Pongo el clock interno del micro en 16MHz
	P1OUT &= 0x00;               //Apago todo
	P1DIR &= 0x00;				//Apago todo
	P2DIR &= 0x00;				//Apago todo
	P2OUT &= 0x00;               //Apago todo
	P2DIR|=(BIT0+BIT1+BIT2+BIT3+BIT4);			//Configuro del 0 al 3 como salida por el display y el 4 por el motor
	P1DIR|= (BIT0+BIT1+BIT2+BIT3);            //Configuro del 0 al 3 como salida por el display
	P1REN |= (BIT4+BIT5);                   // Habilito pull-up/down resistencias
	P1OUT |= (BIT4+BIT5);                   //Selecciono pull-up las resistencias
	P1IE |= (BIT4+BIT5);                       // P1.3 interrupt enabled
	P1IES |= (BIT4+BIT5);                     // P1.3 Hi/lo edge
	P1IFG &= ~(BIT4+BIT5);                  // P1.3 IFG cleared
	P2SEL &= ~BIT6;
	P2SEL &= ~BIT7;
	_BIS_SR(GIE);          // Habilita las interrupciones globales
	P1OUT&=~(BIT0+BIT1+BIT2+BIT3); // Apago todas las salidas del micro que van al display
	P2OUT&=~(BIT0+BIT1+BIT2+BIT3); // Apago todas las salidas dbel micro que van al display
	while(1)
	{
		display();
		motor();
	}
}

void display (void) {
	temp_display=temp_unidad+temp_decena*10;
	mostrar_temp(temp_unidad,temp_decena);
}

/*Cuando se pulsa un boton salta una interrupción, busca cual de los botones fue y despues suma o resta dependiendo.
 * Si llega a un valor minimo o maximo no efectua cambios
 */

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if((P1IFG & BIT4)==BIT4)
		{
		if(temp_decena==DECENA_MAXIMA_DISPLAY&&temp_unidad==UNIDAD_MAXIMA_DISPLAY)   //Set de temperatura maxima, no realiza cambios si se cumple la condicion
						{
						}
						else
						{
							if(temp_unidad==9) //Caso de conflicto, aumenta decena y pone en cero unidad
							{
										temp_unidad=0;
										temp_decena=temp_decena+1;
							}
							else
							{
								temp_unidad=temp_unidad+1;
							}
						}
			P1IFG &= ~BIT4;
		}
	if((P1IFG & BIT5)==BIT5)
		{
		if(temp_decena==DECENA_MINIMA_DISPLAY&&temp_unidad==UNIDAD_MINIMA_DISPLAY) //Set de temperatura maxima, no realiza cambios si se cumple la condicion
						{
						}
						else
						{
							if(temp_unidad==0) //Caso de conflicto, disminuye decena y pone en nueve unidad
							{
								temp_unidad=9;
								temp_decena=temp_decena-1;
							}
							else
							{
								temp_unidad=temp_unidad-1;
							}
						}
			P1IFG &= ~BIT5;
		}
	P1IFG=0x00;
	display();
}


void mostrar_temp(int u,int d) //Hago las comparaciones y saco el binario para el BCD
{
	switch(d)
	{
		case 0:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			break;
		case 1:
			P1OUT&=~(BIT1+BIT2+BIT3);
			P1OUT|=BIT0;
			break;
		case 2:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT1;
			break;
		case 3:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT0+BIT1;
			break;
		case 4:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT2;
			break;
		case 5:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT2+BIT0;
			break;
		case 6:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT2+BIT1;
			break;
		case 7:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT2+BIT1+BIT0;
			break;
		case 8:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT3;
			break;
		case 9:
			P1OUT&=~(BIT0+BIT1+BIT2+BIT3);
			P1OUT|=BIT3+BIT0;
			break;
	}
	switch(u)
		{
			case 0:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				break;
			case 1:
				P2OUT&=~(BIT1+BIT2+BIT3);
				P2OUT|=BIT0;
				break;
			case 2:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT1;
				break;
			case 3:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT0+BIT1;
				break;
			case 4:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT2;
				break;
			case 5:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT2+BIT0;
				break;
			case 6:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT2+BIT1;
				break;
			case 7:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT2+BIT1+BIT0;
				break;
			case 8:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT3;
				break;
			case 9:
				P2OUT&=~(BIT0+BIT1+BIT2+BIT3);
				P2OUT|=BIT3+BIT0;
				break;
		}
}


/***************************************************************/

//Configuracion del puerto one wire para el sensado de temperatura.
void ow_portsetup() {
	OWPORTDIR |= OWPORTPIN;
	OWPORTOUT |= OWPORTPIN;
	OWPORTREN |= OWPORTPIN;
}

/***************************************************************/

//Función para leer un byte del sensor.

float ReadDS1820 ( void )
{
	unsigned int i;
	uint16_t byte = 0;
	for(i = 16; i > 0; i--){
		byte >>= 1;
		if (onewire_read_bit()) {
			byte |= 0x8000;
		}
  }
  return byte;
}

// Funcion para leer la temperatura del sensor

float GetData(void)
{
    uint16_t temp;
    ow_portsetup();
     onewire_reset();
	  onewire_write_byte(0xcc); // skip ROM command
	  onewire_write_byte(0x44); // convert T command
	  OW_HI
	  DELAY_MS(750); // at least 750 ms for the default 12-bit resolution
	  onewire_reset();
	  onewire_write_byte(0xcc); // skip ROM command
	  onewire_write_byte(0xbe); // read scratchpad command
	  temp = ReadDS1820();
	  //Esta calibrado con la mayor precision, por eso se multiplica por 0.0625.
    if(temp<0x8000){
        return(temp*0.0625);
    }
    else
    {
        temp=(~temp)+1;
        return(temp*0.0625);
    }
}

int onewire_reset()
{
	OW_LO
	DELAY_US(480); // 480us minimum
	OW_RLS
  DELAY_US(40); // slave waits 15-60us
  if (OWPORTIN & OWPORTPIN) return 1; // line should be pulled down by slave
  DELAY_US(300); // slave TX presence pulse 60-240us
  if (!(OWPORTIN & OWPORTPIN)) return 2; // line should be "released" by slave
  return 0;
}

//#####################################################################

void onewire_write_bit(int bit)
{
//  DELAY_US(1); // recovery, min 1us
  OW_HI
  if (bit) {
	OW_LO
    DELAY_US(5); // max 15us
	OW_RLS	// input
    DELAY_US(56);
  }
  else {
	  OW_LO
	  DELAY_US(60); // min 60us
	  OW_RLS	// input
	  DELAY_US(1);
  }
 }

//#####################################################################

int onewire_read_bit()
{
  int bit=0;
//  DELAY_US(1); // recovery, min 1us
  OW_LO
  DELAY_US(5); // hold min 1us
  OW_RLS
  DELAY_US(10); // 15us window
  if (OWPORTIN & OWPORTPIN) {
	  bit = 1;
  }
  DELAY_US(46); // rest of the read slot
  return bit;
}

//#####################################################################

void onewire_write_byte(uint8_t byte)
{
  int i;
  for(i = 0; i < 8; i++)
  {
    onewire_write_bit(byte & 1);
    byte >>= 1;
  }
}

//#####################################################################

uint8_t onewire_read_byte()
{
	unsigned int i;
  uint8_t byte = 0;
  for(i = 0; i < 8; i++)
  {
    byte >>= 1;
    if (onewire_read_bit()) byte |= 0x80;
  }
  return byte;
}




void motor(void)
{
	_BIS_SR(GIE);
    if(flag==0)//Me fijo si pasaron los 40 minutos para poder prender el motor.
    {
    	temperature=GetData();	//Leo la temperatura
    	if(temperature-HISTERESIS>((float)temp_display)) //Comparo temperatura de gabinete con temperatura display, sumo 2 grados de variacion termica
    	{
    		tiempo_encendido=18000;
    		P2OUT|=BIT4;	//Prendo el compresor
    		while(temperature+HISTERESIS>((float)temp_display)) // Comparo tempratura de gabinete con temperatura display, resto 2 grados de variacion termica
    		{
    			temperature=GetData();	//Leo la temperatura
    			tiempo_encendido--; //cada lectura de temperatura lleva 800ms aprox
    			if(tiempo_encendido==0) //18000 son los ciclos necesarios para que cumpla 4 horas funcionando
    			{
    				temperature=0;
    			}
    		}
    		P2OUT&=~BIT4; //Apago el compresor xq ya bajo la temperatura o la temperatura esta en los niveles adecuados
    		flag=1;  //Enciendo flag de que se prendio el compresor
    	}
    }
    if(flag==1) //Funcion para contar el tiempo de espera requerido para volver a encender el compresor
    {
    	for(minutos=1;minutos<=TIEMPO_ESPERA_MINUTOS;minutos++)
    	{
    		for(segundos=1;segundos<CANTIDAD_SEGUNDOS_PARA_MINUTOS;segundos++)
    		{
    			for(milisegundos=1;milisegundos<=CANTIDAD_MILI_PARA_SEGUNDOS;milisegundos++)
    			{
    				DELAY_MS(1);
    			}
    		}
    	}
    	flag=0;
    }

}


void DELAY_MS(int ms)
{
	while (ms--)
	{
		__delay_cycles(16000);
	}

}

void DELAY_US(int us)
{
	while (us--)
	{
		__delay_cycles(16);
	}

}

void DELAY_S(int s)
{
	while (s--)
	{
		__delay_cycles(16000000);
	}

}

void DELAY_M(int m)
{
	while (m--)
	{
		__delay_cycles(1600000000);
	}

}
