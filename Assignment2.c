#include "main.h"
#include "Time_Delays.h"
#include "Clk_Config.h"

#include "LCD_Display.h"

#include <stdio.h>
#include <string.h>

#include "Small_7.h"
#include "Arial_9.h"
#include "Arial_12.h"
#include "Arial_24.h"

#include "stm32f4xx_ll_crc.h"

/*
This program flashes an LED to test the nucleo board, while including the header files and '#defines' that
you will need for the later parts of the lab. Use this project as a template for the remaining tasks of the lab.
*/

//Temperature Sensor I2C Address
#define TEMPADR 0x90

//EEPROM I2C Address
#define EEPROMADR 0xA0

uint32_t joystick_centre(void);
uint32_t joystick_right(void);
uint32_t joystick_left(void);
uint32_t joystick_up(void);
uint32_t joystick_down(void);
uint16_t Read_Temperature_Sensor(void);
uint16_t Read_Temp_EEPROM_TO_Memory(void); 
void Write_To_EEPROM(uint16_t temperature);
void Write_to_EEPROM_Array(uint8_t * Packet_Array, void(*Ack_Polling)(void));	
void Read_Packet_From_EEPROM_DisplayLCD(uint8_t * Packet_Array);
void Write_Packet_to_EEPROM_ACK_Polling(void);
uint32_t CRC_Check(uint8_t * Packet_Array);

int main(void){
	//Initialization
  SystemClock_Config();/* Configure the system clock to 84.0 MHz */
	SysTick_Config_MCE2(us);	
	char outputString [18];
	uint16_t Temp_Read = 0;
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	uint32_t FCS = 0;	
	uint32_t FCS_Check=0;
	uint8_t Packet_Array[60];
	
	for (int i = 0; i<6; i++){
		Packet_Array[i] = 0xaa; //MAC Destinatiomn
	}
	for (int t = 6; t<12; t++){
		Packet_Array[t] = 0xbb; //MAC Source
	} 
	
	Packet_Array[12] = 0x00; //Length Lower Byte
	Packet_Array[13] = 0x2e; //Length Higher Byte
	
	for (int u = 14; u<60; u++){ //Payload
		Packet_Array[u] = 0;
	}
	


	
	//Configure LED: GPIO Port B, pin 4
	LL_GPIO_SetPinMode (GPIOB, LL_GPIO_PIN_4, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType (GPIOB, LL_GPIO_PIN_4, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinPull (GPIOB, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);
	LL_GPIO_SetPinSpeed (GPIOB, LL_GPIO_PIN_4, LL_GPIO_SPEED_FREQ_HIGH);
	
	
//JOYSTICK CONFIGURATION	
	
	//Configuring Center Joystick (PORTB,PIN5)
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB); //Enables peripheral clock for PORT B
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_5, LL_GPIO_MODE_INPUT); //Sets B5 as an Input
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_5, LL_GPIO_PULL_NO); //Sets B5 as NO pull
	
	//Configuring Left Joystick (PORTC, PIN1)
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC); //Enables peripheral clock for Port C
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_INPUT); //Configures C1 as Input (subject to user press)
	LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_1, LL_GPIO_PULL_NO); //Sets C1 as NO pull
	
	//Configuring Right Joystick (PORTC, PIN0)
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC); //Enables peripheral clock for Port C
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT); //Configures C0 as Input (subject to user press)
	LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_0, LL_GPIO_PULL_NO); //Sets C0 as NO pull
	
	//Configuring Down Joystick (PORTB, PIN0)
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB); //Enables peripheral clock for Port B
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT); //Configures B0 as Input (subject to user press)
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_0, LL_GPIO_PULL_NO); //Sets B0 as NO pull
	
	//Configuring Up Joystick (PORTA, PIN4)
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA); //Enables peripheral clock for Port A
	LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_INPUT); //Configures A4 as Input (subject to user press)
	LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_NO); //Sets A4 as NO pull
	
	
//CONFIGURING LCD
	Configure_LCD_Pins();
	Configure_SPI1();
	Activate_SPI1();
	Clear_Screen();
	Initialise_LCD_Controller();
	set_font((unsigned char*)Small_7);
	
	
	//Configuring SDA as : Alternate, Open Drain, Pull-Up; SDA = Data Line  
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_8_15(GPIOB, LL_GPIO_PIN_9, LL_GPIO_AF_4);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_9, LL_GPIO_PULL_UP);
	
	
	//Configuring SCL as Alternate function, Open Drainm, Pull Up; SCL = Clock 
	LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetAFPin_8_15(GPIOB, LL_GPIO_PIN_8, LL_GPIO_AF_4);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_8, LL_GPIO_PULL_UP);
	
	
	//Enable the I2C1 Peripheral for us; Sets up Mode and Clock 
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
	LL_I2C_Disable(I2C1); //disable I2C1 prior to configuration
	LL_I2C_SetMode(I2C1, LL_I2C_MODE_I2C);
	LL_I2C_ConfigSpeed(I2C1, 84000000, 100000, LL_I2C_DUTYCYCLE_2); //set speed at 100kHz
	LL_I2C_Enable(I2C1); // re-enable I2C1
	
	//Enable CRC Checking
	LL_AHB1_GRP1_EnableClock (LL_AHB1_GRP1_PERIPH_CRC);
	
	int Down_Pos = 0;	//Will drive Up/Down Joystick switch-statement
	
	int x = 0, y = 10;
		sprintf (outputString, "MAC Dest:");
		put_string(0,0,outputString);
		for (int i =0; i<6 ;i++){
			sprintf(outputString, "%x", Packet_Array[i]);
			put_string(x, y, outputString);
			x += 20;
		}	
		
	
	while(1){
		
		
		//Display MAC Destination
		
	//Centre Pressed -> Read Temperature from sensor over I2C, store in memory and display in on LCD screen	
		if(joystick_centre() == 1){	//Display value of Temperature Sensor on LCD.
			Clear_Screen();
			Temp_Read = Read_Temperature_Sensor();
			//Split Temperature in 2 Bytes.
			uint8_t Temp_Read_LSB = Temp_Read & 0x00ff; 
			uint8_t Temp_Read_MSB = (Temp_Read & 0xff00)>>8; 
			//Print temperature sensor value
			sprintf (outputString, "Temp: %f", Temp_Read*0.125);
			put_string(10,10, outputString);
			LL_mDelay(500000);
			//Store Temperature bytes in 2 first Payload elements
			Packet_Array[14] = Temp_Read_MSB;
			Packet_Array[15] = Temp_Read_LSB;
			Clear_Screen();
			sprintf (outputString, "Stored in Pld!");
			put_string(0,10, outputString);
			LL_mDelay(1000000);
			Clear_Screen();
			FCS = CRC_Check(Packet_Array);
			sprintf (outputString, "CRC CHECK:");
			put_string(0,0, outputString);
			sprintf (outputString, "%x", FCS);
			put_string(0,10, outputString);
			LL_mDelay(500000);
		}
			
		else if (joystick_right()) {			//Write packet to EEPROM
			Clear_Screen();
			sprintf (outputString, "Storing...");
			put_string(10,10, outputString);
			Write_to_EEPROM_Array(Packet_Array, Write_Packet_to_EEPROM_ACK_Polling);
			LL_mDelay(500000);
			Clear_Screen();
			sprintf (outputString, "Stored in EEPROM.");
			put_string(0,10, outputString);
			LL_mDelay(500000);
		}
		
		else if (joystick_left()) { //Read value from EEPROM and display it on the LCD screen.
			
			
			Clear_Screen();
			Read_Packet_From_EEPROM_DisplayLCD(Packet_Array);
			//Payload_Temp = (Packet_Array[14] + Packet_Array[15]);
			//sprintf (outputString, "%f", Payload_Temp*0.125);
			//put_string(10,10, outputString);
			LL_mDelay(500000);
		}
		
		else if (joystick_down()) {
			
			Clear_Screen();
			uint16_t Payload_Temp;	
			if (Down_Pos<5)
			    Down_Pos+=1;
			switch(Down_Pos){
					
					case 1:	//MAC Dest
						//Down_Pos++;
						Clear_Screen();
						x = 0, y = 10;
						//sprintf (outputString, "DP: 1");
						//put_string(0,10,outputString);
						
						sprintf (outputString, "MAC Dest:");
						put_string(0,0,outputString);
						for (int i =0; i<6 ;i++){
							sprintf(outputString, "%x", Packet_Array[i]);
							put_string(x, y, outputString);
							x += 20;
						}
					
						LL_mDelay(500000);
						break;
					case 2:	//MAC Dest-->MAC Src
						//Down_Pos++;
						Clear_Screen();
						x = 0, y = 10;
						//sprintf (outputString, "DP: 1");
						//put_string(0,10,outputString);
						
						sprintf (outputString, "MAC SRC:");
						put_string(0,0,outputString);
						for (int i =6; i<12 ;i++){
							sprintf(outputString, "%x", Packet_Array[i]);
							put_string(x, y, outputString);
							x += 20;
						}
						LL_mDelay(500000);
						break;
					case 3: //MAX Src --> Length
						Clear_Screen();
						x = 0, y = 10;
						sprintf (outputString, "Length");
						put_string(0,0,outputString);
						for (int i =12; i<14 ;i++){
							sprintf(outputString, "%x", Packet_Array[i]);
							put_string(x, y, outputString);
							x += 30;
						}
						LL_mDelay(500000);
						break;
					case 4: 		//Length --> Payload --> Payload
						Clear_Screen();
						Payload_Temp = (Packet_Array[14] + Packet_Array[15]);
						sprintf (outputString, "Temp C°:%f", Payload_Temp*0.125);
						put_string(10,10, outputString);
						LL_mDelay(500000);	
						break;
					case 5:
						Clear_Screen();
						FCS_Check = CRC_Check(Packet_Array);
						LL_mDelay(500000);
						if(FCS == FCS_Check){
							sprintf (outputString, "CRC Check good!");
							put_string(0,0, outputString);
							LL_mDelay(500000);
							Clear_Screen();
							sprintf (outputString, "%x", FCS_Check);
							put_string(0,10, outputString);
						}
						else
							sprintf (outputString, "CRC Check bad!");
							put_string(0,0, outputString);
				}
		//	}
				
		}
		
		else if(joystick_up()){
			uint16_t Payload_Temp;	
			if (Down_Pos > 1 ) 
					Down_Pos-=1;
			switch(Down_Pos) {
				case 1:  //Mac Src --> Mac Dest ---> Mac Dest
					Clear_Screen(); 
					x = 0, y = 10;
					sprintf (outputString, "MAC Dest:");
					put_string(0,0,outputString);
					for (int t =0; t<6 ;t++){
						sprintf(outputString, "%x", Packet_Array[t]);
						put_string(x, y, outputString);
						x += 20;
					}
					LL_mDelay(500000);
					break;
					
				case 2:  //Length --> Mac Src
					Clear_Screen();
					x = 0, y = 10;
					sprintf (outputString, "MAC SRC:");
					put_string(0,0,outputString);
					for (int i =6; i<12 ;i++){
						sprintf(outputString, "%x", Packet_Array[i]);
						put_string(x, y, outputString);
						x += 20;
					}
					LL_mDelay(500000);
					
					break;
			
				case 3: 	//Payload --> Length
					Clear_Screen();
					x = 0, y = 10;
					sprintf (outputString, "Length");
					put_string(0,0,outputString);
					for (int i =12; i<14 ;i++){
						sprintf(outputString, "%x", Packet_Array[i]);
						put_string(x, y, outputString);
						x += 20;
					}
					LL_mDelay(500000);
					break;
					
				case 4: //CRC Check --> Payload
					Clear_Screen();
					LL_mDelay(500000);
					Payload_Temp = (Packet_Array[14] + Packet_Array[15]);
					sprintf (outputString, "Temp C°:%f", Payload_Temp*0.125);
					put_string(10,10, outputString);
					LL_mDelay(500000);	
					
					break;
		}
		
	}
	/**	else {
			LL_mDelay(500000);
			Clear_Screen();
			int x = 0, y = 10;
			sprintf (outputString, "MAC Dest:");
			put_string(0,0,outputString);
			for (int i =0; i<6 ;i++){
				sprintf(outputString, "%x", Packet_Array[i]);
				put_string(x, y, outputString);
				x += 20;
		}	
		
		}**/
		
		
	
		
		
	}
	
	
	
}



uint32_t joystick_centre(void) {
	return (LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_5)); //Returns 1 if the joystick is pressed in the centre, 0 otherwise
}

uint32_t joystick_left(void){
	return (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_1));
}
	
uint32_t joystick_right(void){
	return (LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_0));
}

uint32_t joystick_up(void){
	return (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4));
}

uint32_t joystick_down(void){
	return (LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_0));
}

//void Write_Temp_To_EEPROM(uint16_t temperature) { 
	
//}

uint16_t Read_Temperature_Sensor(void) {

			uint16_t temperature = 0; //to store the temperature value
		
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
			
			LL_I2C_TransmitData8(I2C1, TEMPADR); //Read 1 byte From Temperature Sensor Address
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);
			
			LL_I2C_TransmitData8(I2C1, 0x00); //Set pointer register to the temperature register
			while(!LL_I2C_IsActiveFlag_TXE(I2C1)); //TXE flag confirms ACK
			
			LL_I2C_GenerateStartCondition(I2C1); //Re-Start if you want to read from an address; i.e. re-generate a Start Condition
			while(!LL_I2C_IsActiveFlag_SB(I2C1));//Ack of Restart condition
			
			LL_I2C_TransmitData8(I2C1, TEMPADR+1); //Read byte from the next pointer address
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Ack needed
			LL_I2C_ClearFlag_ADDR(I2C1);
			
			LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK); //Ack the incoming data
			while(!LL_I2C_IsActiveFlag_RXNE(I2C1)); //Wait until complete
			temperature = LL_I2C_ReceiveData8(I2C1);
			
			temperature = temperature << 8; // shift to upermost bits
		
		
			LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK); //ack next byte received
			while(!LL_I2C_IsActiveFlag_RXNE(I2C1)); //wait until complete
			temperature += LL_I2C_ReceiveData8(I2C1); //combine temperature values
			
			LL_I2C_GenerateStopCondition(I2C1); //end transmissions sequence
			temperature = temperature >>5; //Shift temperature back to the lower 5 bits
				
			return temperature;
			
}

void Write_Packet_to_EEPROM_ACK_Polling(void){
			
			LL_I2C_ClearFlag_AF (I2C1);
	
			while(1){ 
				LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
				while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
			
				LL_I2C_TransmitData8(I2C1, EEPROMADR); //Focus on EEPROM 
			//while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
				LL_I2C_ClearFlag_ADDR(I2C1);

				LL_mDelay(100);
				if (LL_I2C_IsActiveFlag_AF (I2C1)==0)
					break;
				LL_I2C_ClearFlag_AF (I2C1);
			}

}



void Write_to_EEPROM_Array(uint8_t * Packet_Array, void (*Ack_Polling)(void)){
			//Write the Packet Array to EEPROM
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
			
			LL_I2C_TransmitData8(I2C1, EEPROMADR); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);
			
			LL_I2C_TransmitData8(I2C1, 0x00); //Set pointer register to the EEPROM internal register
			while(!LL_I2C_IsActiveFlag_TXE(I2C1)); //TXE flag confirms ACK
			
			LL_I2C_TransmitData8(I2C1, 0x00);	
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));

			for (int i=0; i<32; i++){
			LL_I2C_TransmitData8(I2C1, Packet_Array[i]);
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
			}
			
			LL_I2C_GenerateStopCondition(I2C1);
			
			Ack_Polling();
			
			//LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			//while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
			
			//LL_I2C_TransmitData8(I2C1, EEPROMADR); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);
			
			LL_I2C_TransmitData8(I2C1, 0x00); //Set pointer register to the EEPROM internal register
			while(!LL_I2C_IsActiveFlag_TXE(I2C1)); //TXE flag confirms ACK
			
			LL_I2C_TransmitData8(I2C1, 0x20);	
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
			
			for (int i=32; i<60; i++){
			LL_I2C_TransmitData8(I2C1, Packet_Array[i]);
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
			}
			
			LL_I2C_GenerateStopCondition(I2C1);
	
}


void Write_To_EEPROM(uint16_t temperature){

			//WRITE FROM MEMORY TO EEPROM ADDRESS.
			
			uint8_t	temperatureLSB = (temperature & 0x00FF);
			uint16_t temperatureHighByte = (temperature & 0xFF00);
			uint8_t temperatureMSB = temperatureHighByte >> 8; 
	
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
			
			LL_I2C_TransmitData8(I2C1, EEPROMADR); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);
			
			LL_I2C_TransmitData8(I2C1, 0x00); //Set pointer register to the EEPROM internal register
			while(!LL_I2C_IsActiveFlag_TXE(I2C1)); //TXE flag confirms ACK
			
			LL_I2C_TransmitData8(I2C1, 0x00);	
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
			
			LL_I2C_TransmitData8(I2C1, temperatureMSB);
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
				
			LL_I2C_TransmitData8(I2C1, temperatureLSB);
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
			
			LL_I2C_GenerateStopCondition(I2C1);
			
} 

void Read_Packet_From_EEPROM_DisplayLCD(uint8_t * Packet_Array){
			
			char outputString[18];
	
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
	
			LL_I2C_TransmitData8(I2C1, EEPROMADR); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);

			LL_I2C_TransmitData8(I2C1, 0x00); //Set pointer register to the EEPROM internal register
			while(!LL_I2C_IsActiveFlag_TXE(I2C1)); //TXE flag confirms ACK
			
			LL_I2C_TransmitData8(I2C1, 0x00);	
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
	
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); 
	
			LL_I2C_TransmitData8(I2C1, EEPROMADR+1); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);

			for (int i=0; i<60; i++){
				LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK); //Ack the incoming data
				while(!LL_I2C_IsActiveFlag_RXNE(I2C1)); //Wait until complete
				Packet_Array[i] = LL_I2C_ReceiveData8(I2C1);
			}
			LL_I2C_GenerateStopCondition(I2C1); //end transmission
			
			uint16_t Payload_Temp;
			Payload_Temp = (Packet_Array[14] + Packet_Array[15]);
			sprintf (outputString, "EEPROM VAL:");
			put_string(0,0,outputString);
			sprintf (outputString, "%f", Payload_Temp*0.125);
			put_string(10,10, outputString);

}

uint32_t CRC_Check(uint8_t * Packet_Array) {
	
	
	LL_CRC_ResetCRCCalculationUnit (CRC);
	uint32_t FCS = 0;
	for (int i = 0; i<60; i++){
		FCS+=Packet_Array[i];
		if (i%4 == 0)
			LL_CRC_FeedData32(CRC, FCS);
			LL_mDelay(50);
	}
	
	uint32_t CRC_Result = LL_CRC_ReadData32(CRC);
	return CRC_Result;
	//sprintf (outputString, "CRC CHECK:");
	//put_string(0,0,outputString);
	//sprintf (outputString, "%x", CRC_Result);
	//put_string(10,10, outputString);
}

uint16_t Read_Temp_EEPROM_TO_Memory(void){
			
			uint16_t temperature = 0;
			
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); //ACK for Start Condition	
	
			LL_I2C_TransmitData8(I2C1, EEPROMADR); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);

			LL_I2C_TransmitData8(I2C1, 0x00); //Set pointer register to the EEPROM internal register
			while(!LL_I2C_IsActiveFlag_TXE(I2C1)); //TXE flag confirms ACK
			
			LL_I2C_TransmitData8(I2C1, 0x00);	
			while(!LL_I2C_IsActiveFlag_TXE(I2C1));
	
			LL_I2C_GenerateStartCondition(I2C1); //Generate Start condition for start sequence
			while(!LL_I2C_IsActiveFlag_SB(I2C1)); 
	
			LL_I2C_TransmitData8(I2C1, EEPROMADR+1); //Focus on EEPROM 
			while(!LL_I2C_IsActiveFlag_ADDR(I2C1)); //Wait for ADDR flag to confirm ACK
			LL_I2C_ClearFlag_ADDR(I2C1);
	
			LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK); //Ack the incoming data
			while(!LL_I2C_IsActiveFlag_RXNE(I2C1)); //Wait until complete
			uint16_t temperatureLSB = LL_I2C_ReceiveData8(I2C1) << 8;
			
			//temperature = temperature << 8; // shift to upermost bits
		
		
			LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK); //ack next byte received
			while(!LL_I2C_IsActiveFlag_RXNE(I2C1)); //wait until complete
			uint16_t temperatureMSB = LL_I2C_ReceiveData8(I2C1); //combine temperature values
			
			
			LL_I2C_GenerateStopCondition(I2C1); //end transmissions sequence
			//temperature = temperature >>5; //Shift temperature back to the lower 5 bits
			temperature = temperatureLSB + temperatureMSB;
			
			return temperature;			

}




