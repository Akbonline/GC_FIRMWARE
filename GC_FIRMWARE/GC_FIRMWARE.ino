#include "Wire.h"
#include<avr/wdt.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <Keypad.h>//header for keypad commands enabling
#include <SoftwareSerial.h>
#include<LiquidCrystal.h>

#define BUZZER_ON HIGH
#define BUZZER_OFF LOW
#define RELAY_ON HIGH
#define RELAY_OFF LOW

#define ON HIGH
#define OFF LOW

LiquidCrystal lcd(13,12,8,9,10,11);
 /*The circuit:
 * LCD RS pin to digital pin 13
 * LCD Enable pin to digital pin 12
 * LCD D4 pin to digital pin 8
 * LCD D5 pin to digital pin 9
 * LCD D6 pin to digital pin 10
 * LCD D7 pin to digital pin 11
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * VEE contrast to pin 46(PWM)
*/

#define M41T00_I2C_ADDRESS 0x68
#define EEPROM_I2C_BLOCK0_ADDRESS 0x50
#define EEPROM_I2C_BLOCK1_ADDRESS 0x54


String GRID_ID="300300"; 
//G310001 D240318 T1350 AL00 SV00.00 SC00.00 BV00.00 BC00.00 LV00.00 LC00.00 LP00.00 ta27.3 tb35.9 L3500  Ca G#*

 

const byte ROWS = 4; // Four rows
const byte COLS = 4; // FOUR columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 23, 25, 27, 29 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 31, 33, 35, 37 };
//  Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );


// A array of the weekday
char* days[] = { "NA","Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

char* SMS_RECIPIENT[] = { "9313407332" , "9816686443" };


char* shortcut_code[]={"*D000#","*D100#","*D101#","*D102#","*D103#","*D104#","*D105#","*C991#","*C022#","*C333#","*C981#","*C121#","*C029#","*C156#",};
/*******************************KEYBOARD SHORTCUT CODE*******************************
*D000#----------Clear Display
*D100#----------Show Grid id
*D101#----------Array Parameters
*D102#----------Battery Parameters
*D103#----------LOAD parameters
*D104#----------Show Date & time
*D105#----------SP VOLTAGE AND BATTERY VOLTAGE
*C991#---------BATTERY LOW LEVEL
*C022#---------HIGH BATTERY CURRENT
*C333#---------V_AT_HIGH_BATTERY_CURRENT
*C981#---------LOAD_SC_RISE
*C121#--------LOAD_HIGH_CURRENT
*C029#--------BATTERY_CHG_VOLTAGE
*C156#--------MINIMUM_SPV
 */
/*******************************KEYBOARD SHORTCUT CODE*******************************/




/*******************************Memory MAP*******************************
0     :  First time pointer set indicator
1-2   :  empty
3-4   :  empty
5-8   :  Battery low level
9-12  :  High Battery Current
13-16 :  Voltage at High battery current
17-20 :  Load Sc Rise
21-24 :  Load High Current
25-28 :  Battery charging Voltage
29-32 :  Minimum Solar Panel voltage
33    :  Fault_logg_overwrite indicator
34    :  data logg overwrite indicator
35-38 :  Fault logg pointer
39-42 :  Data logg pointer
/*******************************Memory Map*******************************/



/******************************ALL comparable value for fault detection******************************/
float BATTERY_LOW_LEVEL=10.5;
float HIGH_BATTERY_CURRENT=13.0;
float V_AT_HIGH_BATTERY_CURRENT=20.0;
float LOAD_SC_RISE=5.0;//5amp/10ms 
float LOAD_HIGH_CURRENT=24.0;
float BATTERY_CHG_VOLTAGE=24.0;
float MINIMUM_SPV=24.0;
//These all values are Changeable by keypad
/******************************ALL comparable value for fault detection******************************/



int  SPV_POSITIVE_PORT=A0;
int  SPV_NEGATIVE_PORT=A1;

int BATTERY_VOLTAGE_PORT=A3;
int LOAD_VOLTAGE_PORT=A5;

int ARRAY_CURRENT_PORT=A2;
int BATTERY_CURRENT_PORT=A4;
int LOAD_CURRENT_PORT=A6;
int GC_TEMPERATURE_PORT=A7;
int AMB_TEMPERATURE_PORT=A8;

/***********************************FAULT INDICATORS***********************************/
int ARRAY_FAULT=2;
int BATTERY_FAULT=3;
int LOAD_FAULT=4;
//int HIGH_LOAD_CURRENT=5;
/***********************************FAULT INDICATORS***********************************/

int SIM900A_ON_OFF = 24;


/*************************APN for Different Service Providers*************************/
String AIRTEL_APN="airtelgprs.com";
String IDEA_APN="internet";
String VODAFONE_APN="www";
String BSNL_APN="bsnlnet";

String APN="AT+SAPBR=3,1,\"APN\",\"";
String E_APN="\"";
String SET_APN="";

//SET_APN=APN + AIRTEL_APN + E_APN;
//"AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\""

/*************************APN for Different Service Providers*************************/


//Operate Alarm and relay simultaneously
int BUZZER_PORT=6;
int RELAY_PORT=7;


//Multipliers for analogg input
float V_BATT_MULT=0.03200955;

float SPV_POSITIVE_MULT=0.0537109375;
float SPV_NEGATIVE_MULT=0.009765625;

float V_LOAD_MULT=0.03200955;

float I_BATT_MULT=0.0739820076 ;
float I_ARRAY_MULT=0.0739820076 ;
float I_LOAD_MULT=0.0739820076 ;

float TEMP_MULT=0.48828125;


String s="";
String Serialport1_rx="";

char x=0;
byte cursor_count=0;
byte retry=0;
byte time_cmd=0;
byte send_data_cmd=0;

//ADC values are read into these variables
int v0[10];//array positive
int v00[10];//array negative
int v1[10];//battery
int v2[10];//load

int i0[10];//array
int i1[10];//battery
int i2[10];//load
int TEMP_AMB[10];
int TEMP_GC[10];

String SP_CURRENT="";
String LOW_BATTERY_VOLTAGE="";
String S_HIGH_BATTERY_CURRENT="";
String LOAD_SHORTCIRCUIT="";
String S_LOAD_HIGH_CURRENT="";
String LOAD_OPEN="";
String _15min_string1="";
String _15min_string2="";
String _15min_string3="";
String _15min_string4="";
String _15min_string5="";
String _15min_string6="";

byte c=0,c1=0;
long int p=0;
String Final="";
String _Final="";
String LOAD_CURRENT_STRING="";
String TRANSMIT_BUFFER="";

String ALARM_CODE="00";
String LOAD_ALARM_CODE="00";
String Key_Shortcut="";
String temp="";
String temp1="";
char* countdown[]={"10","09","08","07","06","05","04","03","02","01","00"};
byte second, minute, hour,dayOfWeek, date, month, year;
byte _second, _minute, _hour,_dayOfWeek, _date, _month, _year;
byte ss,mm,hh,day,dd,mo,yy;
byte invalid_shortcut=0;
byte SIM_inserted=0;
byte xyz=0;
byte _10ms=0;
int _1sec=0;
float previous_value=0.0;
byte count=0;
byte k=0;
byte battery_v_fault=0;
byte battery_i_fault=0;
byte transfer_busy_flag=0;
byte STEP=0;
byte LOAD_OPEN_FAULT=0;
byte LOAD_HIGH_CURRENT_FAULT=0;
byte LOAD_SC_FAULT=0;
byte i=0;
byte cnt=0,String_to_transmit=0;
byte p_init=0;
byte  send_start=0;

int sp_timer=0,lbv_timer=0,hbc_timer=0,lhc_timer=0,lo_timer=0,lsc_timer=0;
byte any_fault=0,_15min_flag=0;
byte display=0;
byte set_comp_flag=0;
byte set_edit_cursor=0,value_completed=0,display_save_exit=0;
byte W_EEPROM_PARA=0;
byte user_wants_save=0;
int free_timer=0,free_timer2=0,timeout_timer=0;
int request_timeout=0,request_timeout1=0;
byte clear_display_flag=0;
String pqr="";
float para_temp=0;
byte read_fault_logg=0;
byte read_data_logg=0;
byte fault_logg_overwrite_indicator=0;
byte data_logg_overwrite_indicator=0;
byte network_registration=0;
byte _network_time_timer=0;
byte hourly_data_logg=0;
byte time_command_sent=0;
String zigbee_string="";
String zigbee_buffer="";
String dummy_string="";
char y;
byte first_current_read=0;
int s_length=0;
byte wait_time_update=1;
float SPV_NEG_IN=5.0;
int LUX_VALUE=1300;

//0-1023------------variables
//1024-21503---------fault logg
//21504-131071--------data logg

long int data_logg_ptr=21504;
long int fault_logg_ptr=1024;
long int fault_logg_overwrite=21490;
long int data_logg_overwrite=131003;


////////////////////////////////
float SPV_FINAL= 0;
float SPV_POSITIVE=0;
float SPV_NEGATIVE=0;

float BATTERY_VOLTAGE = 0;
float LOAD_VOLTAGE=0;

float ARRAY_CURRENT= 0;
float BATTERY_CURRENT = 0;
float LOAD_CURRENT=0;
float LOAD_POWER=0;
float LOAD_CURRENT1=0;
////////////////////////////////////

//Read values are processed and stored here
float _SPV_FINAL= 0;
float _BATTERY_VOLTAGE = 0;
float _LOAD_VOLTAGE=0;


float _ARRAY_CURRENT= 0;
float _BATTERY_CURRENT = 0;
float _LOAD_CURRENT=0;
float _LOAD_POWER=0;
float GC_TEMPERATURE=0;
float AMB_TEMPERATURE=0;

float LOAD_CURRENT_OFFSET=0.325;
float SP_CURRENT_OFFSET=0.325;
float BATT_CURRENT_OFFSET=0.420;


float _v0=0, _v1 =0,_v2 =0, _i0 = 0,_i1 = 0,_i2=0,_pow=0;

byte prev_minute=99,flag=0;
byte retry_time_read=0;

/************************************************FUNCTION HEADER************************************************/
byte decToBcd(byte val);
byte bcdToDec(byte val);
void _1st_time_pointer_set(void);
void Read_pointer(void);
void Read_comparable_parameters(void);
void Store_comparable_parameters(void);
void STORE_DEFAULT_PARAMETER_VALUES(void);
void READ_FAULT_LOGG(void);
void READ_DATA_LOGG(void);
void CHECK_FAULT_DATA_LOGG_READ_CMD(void);
void adc_add(void);
void KEYPAD_OPERATION(void);
void READ_SERIAL_PORT(void);
void READ_SERIAL_PORT1(void);
void READ_ZIGBEE_DATA(void);

void SEND_SIM900A_COMMAND(String x,String y);
void SEND_SMS(String x);
void setM41T00time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year);
void readM41T00time(byte *second,byte *minute,byte *hour,byte *dayOfWeek,byte *dayOfMonth,byte *month,byte *year);
void displayTime();
void displayTime_lcd();
void _DISPLAY(void);
void ReadTime(void);
void GET_Network_TIME(void);
void SEND_SIM900A_TIME(void);
void RECEIVE_SIM900A_TIME(void);
void SYNC_DATE_TIME(void);
int dayofweek(int d, int m, int y);

void SPV_POSITIVE_READ(void);
void SPV_NEGATIVE_READ(void);
void SPV_FINAL_READ(void);

void ARRAY_CURRENT_READ(void);
void BATTERY_VOLTAGE_READ(void);
void BATTERY_CURRENT_READ(void);
void LOAD_VOLTAGE_READ(void);
void LOAD_CURRENT_READ(void);
void GC_TEMPERATURE_READ(void);

void AVG_ALL_PARAMETERS(void);
void ALL_AVG_PARA(void);
void CHECK_ALL_FAULT(void);
void CHECK_15MIN_FAULT(void);
void CHECK_LOAD_CURRENT_FAULT(void);
void CHECK_SHORTCIRCUIT_FAULT(void);
void CHECK_FAULT_TIME(void);
void all_timeout_check(void);
void Timeout(void);
void LOAD_CAL_CHECKSUM(void);
char CAL_CHECKSUM(String u);
void Reset_hourly_data_flag(void);
void FINAL_STRING(String temp);
void _FINAL_STRING(String temp);

void LOAD_PARA_STRING(String t);
void CHECK_INTERNET(void);
void SIM_STATUS(void);
void ShowSerialData();
void SIM900A_RESPONSE(void);
void STORE_DATA_LOG(String y);
void STORE_FAULT_LOG(String y);
void BLOCK_CHANGE_1(void);
void BLOCK_CHANGE_2(void);
void SEND_DATA_SERVER(String x);
void _15_min_data();
void SEND_PENDING_STRING(void);
/************************************************FUNCTION HEADER************************************************/







byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}


void _1st_time_pointer_set(void)
{
  byte kk=0;
  byte *f_ptr=(byte *)&fault_logg_ptr;      
  byte *d_ptr=(byte *)&data_logg_ptr;
    
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(0 >> 8);     // MSB
  Wire.write(0 & 0xFF); // LSB
  
  Wire.endTransmission();

 
  Wire.requestFrom(EEPROM_I2C_BLOCK0_ADDRESS,1);
  if (Wire.available())
  {
  kk = Wire.read();
  if(kk!=1)
  {
    
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(0 >> 8);   // MSB
  Wire.write(0 & 0xFF);   // LSB
  Wire.write(1);
  Wire.endTransmission();   
  delay(5);


Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(35 >> 8);   // MSB
  Wire.write(35 & 0xFF);   // LSB
  Wire.write(*f_ptr++);
  Wire.write(*f_ptr++);
  Wire.write(*f_ptr++);
  Wire.write(*f_ptr++);
 
  Wire.write(*d_ptr++);
  Wire.write(*d_ptr++);
  Wire.write(*d_ptr++);
  Wire.write(*d_ptr++);
 
 
  Wire.endTransmission();   
  delay(5);


 
  
  STORE_DEFAULT_PARAMETER_VALUES();

Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(33 >> 8);   // MSB
  Wire.write(33 & 0xFF);   // LSB
  Wire.write(0);//fault logg overwrite
  Wire.write(0);//data logg overwrite
  Wire.endTransmission();   
  delay(5);
  }
  
  }
  
  
  }

void Read_pointer(void)
{
  byte *f_ptr=(byte *)&fault_logg_ptr;  
  byte *d_ptr=(byte *)&data_logg_ptr;  
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(35 >> 8);   // MSB
  Wire.write(35 & 0xFF);   // LSB
  Wire.endTransmission();
  
  Wire.requestFrom(EEPROM_I2C_BLOCK0_ADDRESS,8);
 if (Wire.available())
 {
  *f_ptr++ = Wire.read(); 
  *f_ptr++ = Wire.read();
  *f_ptr++ = Wire.read(); 
  *f_ptr++ = Wire.read();

  *d_ptr++ = Wire.read();
  *d_ptr++ = Wire.read(); 
  *d_ptr++ = Wire.read();
  *d_ptr++ = Wire.read(); 
 }
  
Serial.print("FAULT LOGG POINTER : ");
Serial.println(fault_logg_ptr);
Serial.print("DATA LOGG POINTER : ");
Serial.print(data_logg_ptr);
Serial.println();
}


void Read_comparable_parameters(void)
{
byte *R_BATTERY_LOW_LEVEL=(byte *)&BATTERY_LOW_LEVEL;      
byte *R_HIGH_BATTERY_CURRENT=(byte *)&HIGH_BATTERY_CURRENT;      
byte *R_V_AT_HIGH_BATTERY_CURRENT=(byte *)&V_AT_HIGH_BATTERY_CURRENT;      
byte *R_LOAD_SC_RISE=(byte *)&LOAD_SC_RISE;      
byte *R_LOAD_HIGH_CURRENT=(byte *)&LOAD_HIGH_CURRENT;      
byte *R_BATTERY_CHG_VOLTAGE=(byte *)&BATTERY_CHG_VOLTAGE;      
byte *R_MINIMUM_SPV=(byte *)&MINIMUM_SPV;      

  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(5 >> 8);   // MSB
  Wire.write(5 & 0xFF);   // LSB
  Wire.endTransmission();
  
  Wire.requestFrom(EEPROM_I2C_BLOCK0_ADDRESS,30);
 if (Wire.available())
 {
  for(c=0;c<4;c++)
  {
  *R_BATTERY_LOW_LEVEL++ = Wire.read(); 
  }
  for(c=0;c<4;c++)
  {
  *R_HIGH_BATTERY_CURRENT++ = Wire.read(); 
  }
  for(c=0;c<4;c++)
  {
  *R_V_AT_HIGH_BATTERY_CURRENT++ = Wire.read(); 
  }
  for(c=0;c<4;c++)
  {
  *R_LOAD_SC_RISE++ = Wire.read(); 
  }
  for(c=0;c<4;c++)
  {
  *R_LOAD_HIGH_CURRENT++ = Wire.read(); 
  }
  for(c=0;c<4;c++)
  {
  *R_BATTERY_CHG_VOLTAGE++ = Wire.read(); 
  }
  for(c=0;c<4;c++)
  {
  *R_MINIMUM_SPV++ = Wire.read(); 
  }
fault_logg_overwrite_indicator=Wire.read();
data_logg_overwrite_indicator=Wire.read();
  
 }//wire.available

Serial.println();
Serial.print("BATTERY LOW LEVEL : ");
if(BATTERY_LOW_LEVEL<10)
{
Serial.print("0");  
}
Serial.print(BATTERY_LOW_LEVEL);
Serial.println();

Serial.print("HIGH_BATTERY_CURRENT : ");
if(HIGH_BATTERY_CURRENT<10)
{
Serial.print("0");  
}
Serial.print(HIGH_BATTERY_CURRENT);
Serial.println();

Serial.print("V_AT_HIGH_BATTERY_CURRENT : ");
if(V_AT_HIGH_BATTERY_CURRENT<10)
{
Serial.print("0");  
}
Serial.print(V_AT_HIGH_BATTERY_CURRENT);
Serial.println();


Serial.print("LOAD_SC_RISE : ");
if(LOAD_SC_RISE<10)
{
Serial.print("0");  
}
Serial.print(LOAD_SC_RISE);
Serial.println();


Serial.print("LOAD_HIGH_CURRENT : ");
if(LOAD_HIGH_CURRENT<10)
{
Serial.print("0");  
}
Serial.print(LOAD_HIGH_CURRENT);
Serial.println();

Serial.print("BATTERY_CHG_VOLTAGE : ");
if(BATTERY_CHG_VOLTAGE<10)
{
Serial.print("0");  
}

Serial.print(BATTERY_CHG_VOLTAGE);
Serial.println();


Serial.print("MINIMUM_SPV : ");
if(MINIMUM_SPV<10)
{
Serial.print("0");  
}
Serial.print(MINIMUM_SPV);
Serial.println();

Serial.print("FAULT LOGG OVERWRITE : ");
Serial.println(fault_logg_overwrite_indicator);
Serial.print("DATA LOGG OVERWRITE : ");
Serial.print(data_logg_overwrite_indicator);
Serial.println();

}


void Store_comparable_parameters(void)
{
if(W_EEPROM_PARA==1)  
{
BATTERY_LOW_LEVEL=para_temp;
if(user_wants_save==1)  
{  
byte *W_BATTERY_LOW_LEVEL=(byte *)&BATTERY_LOW_LEVEL;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(5 >> 8);   // MSB
  Wire.write(5 & 0xFF);   // LSB
  Wire.write(*W_BATTERY_LOW_LEVEL++);
  Wire.write(*W_BATTERY_LOW_LEVEL++);
  Wire.write(*W_BATTERY_LOW_LEVEL++);
  Wire.write(*W_BATTERY_LOW_LEVEL++);
  Wire.endTransmission();   
}}


if(W_EEPROM_PARA==2)  
{
HIGH_BATTERY_CURRENT=para_temp;
if(user_wants_save==1)  
{  
byte *W_HIGH_BATTERY_CURRENT=(byte *)&HIGH_BATTERY_CURRENT;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(9 >> 8);   // MSB
  Wire.write(9 & 0xFF);   // LSB
  Wire.write(*W_HIGH_BATTERY_CURRENT++);
  Wire.write(*W_HIGH_BATTERY_CURRENT++);
  Wire.write(*W_HIGH_BATTERY_CURRENT++);
  Wire.write(*W_HIGH_BATTERY_CURRENT++);
  Wire.endTransmission();   
}}

if(W_EEPROM_PARA==3) 
{
V_AT_HIGH_BATTERY_CURRENT=para_temp;
if(user_wants_save==1)  
{  
byte *W_V_AT_HIGH_BATTERY_CURRENT=(byte *)&V_AT_HIGH_BATTERY_CURRENT;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(13 >> 8);   // MSB
  Wire.write(13 & 0xFF);   // LSB
  Wire.write(*W_V_AT_HIGH_BATTERY_CURRENT++);
  Wire.write(*W_V_AT_HIGH_BATTERY_CURRENT++);
  Wire.write(*W_V_AT_HIGH_BATTERY_CURRENT++);
  Wire.write(*W_V_AT_HIGH_BATTERY_CURRENT++);
  Wire.endTransmission();   
}}

if(W_EEPROM_PARA==4)  
{
LOAD_SC_RISE=para_temp;
if(user_wants_save==1)  
{  
byte *W_LOAD_SC_RISE=(byte *)&LOAD_SC_RISE;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(17 >> 8);   // MSB
  Wire.write(17 & 0xFF);   // LSB
  Wire.write(*W_LOAD_SC_RISE++);
  Wire.write(*W_LOAD_SC_RISE++);
  Wire.write(*W_LOAD_SC_RISE++);
  Wire.write(*W_LOAD_SC_RISE++);
  Wire.endTransmission();   
}}

if(W_EEPROM_PARA==5) 
{
LOAD_HIGH_CURRENT=para_temp;
if(user_wants_save==1)  
{  
byte *W_LOAD_HIGH_CURRENT=(byte *)&LOAD_HIGH_CURRENT;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(21 >> 8);   // MSB
  Wire.write(21 & 0xFF);   // LSB
  Wire.write(*W_LOAD_HIGH_CURRENT++);
  Wire.write(*W_LOAD_HIGH_CURRENT++);
  Wire.write(*W_LOAD_HIGH_CURRENT++);
  Wire.write(*W_LOAD_HIGH_CURRENT++);
  Wire.endTransmission();   
}}

if(W_EEPROM_PARA==6)  
{
 BATTERY_CHG_VOLTAGE=para_temp;
if(user_wants_save==1)  
{  
byte *W_BATTERY_CHG_VOLTAGE=(byte *)& BATTERY_CHG_VOLTAGE;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(25 >> 8);   // MSB
  Wire.write(25 & 0xFF);   // LSB
  Wire.write(*W_BATTERY_CHG_VOLTAGE++);
  Wire.write(*W_BATTERY_CHG_VOLTAGE++);
  Wire.write(*W_BATTERY_CHG_VOLTAGE++);
  Wire.write(*W_BATTERY_CHG_VOLTAGE++);
  Wire.endTransmission();   
}}

if(W_EEPROM_PARA==7)  
{
MINIMUM_SPV=para_temp;
if(user_wants_save==1)  
{  
byte *W_MINIMUM_SPV=(byte *)&MINIMUM_SPV;      
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(29 >> 8);   // MSB
  Wire.write(29 & 0xFF);   // LSB
  Wire.write(*W_MINIMUM_SPV++);
  Wire.write(*W_MINIMUM_SPV++);
  Wire.write(*W_MINIMUM_SPV++);
  Wire.write(*W_MINIMUM_SPV++);
  Wire.endTransmission();   
}}


if(user_wants_save==1)  
{  
delay(5);
lcd.clear();
lcd.print("SAVED : OK");    
W_EEPROM_PARA=0;
user_wants_save=0;
set_comp_flag=0;
free_timer=1;
para_temp=0;
}
  
}



void STORE_DEFAULT_PARAMETER_VALUES(void)
{
byte *W_BATTERY_LOW_LEVEL=(byte *)&BATTERY_LOW_LEVEL;      
byte *W_HIGH_BATTERY_CURRENT=(byte *)&HIGH_BATTERY_CURRENT;      
byte *W_V_AT_HIGH_BATTERY_CURRENT=(byte *)&V_AT_HIGH_BATTERY_CURRENT;      
byte *W_LOAD_SC_RISE=(byte *)&LOAD_SC_RISE;      
byte *W_LOAD_HIGH_CURRENT=(byte *)&LOAD_HIGH_CURRENT;      
byte *W_BATTERY_CHG_VOLTAGE=(byte *)&BATTERY_CHG_VOLTAGE;      
byte *W_MINIMUM_SPV=(byte *)&MINIMUM_SPV;      

  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(5 >> 8);   // MSB
  Wire.write(5 & 0xFF);   // LSB
  for(c=0;c<4;c++)
  {
  Wire.write(*W_BATTERY_LOW_LEVEL++);
  }

  for(c=0;c<4;c++)
  {
  Wire.write(*W_HIGH_BATTERY_CURRENT++);
  }

  for(c=0;c<4;c++)
  {
  Wire.write(*W_V_AT_HIGH_BATTERY_CURRENT++);
  }

  for(c=0;c<4;c++)
  {
  Wire.write(*W_LOAD_SC_RISE++);
  }

  for(c=0;c<4;c++)
  {
  Wire.write(*W_LOAD_HIGH_CURRENT++);
  }

  for(c=0;c<4;c++)
  {
  Wire.write(*W_BATTERY_CHG_VOLTAGE++);
  }
  
  for(c=0;c<4;c++)
  {
  Wire.write(*W_MINIMUM_SPV++);
  }
  
  Wire.endTransmission();   
  delay(5);

}





void READ_FAULT_LOGG(void)
{
long int t=0;  
byte g=0,counter=0;

if(fault_logg_ptr==1024 && fault_logg_overwrite_indicator==0)
{
Serial.println("LOGG IS EMPTY");
Serial.println();  
}

if(fault_logg_ptr > 1024 && fault_logg_overwrite_indicator==0)
{
p= 1024;
t= fault_logg_ptr;
}

if(fault_logg_overwrite_indicator==1)
{
p=1024;
t=21490;
}
  
while(p<t)
{
wdt_reset();

Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
Wire.write(p >> 8);   // MSB
Wire.write(p & 0xFF);   // LSB
Wire.endTransmission();
  if(t-p < 32)
  {
g=t-p;
  }
else
{
 g=32; 
}

Wire.requestFrom(EEPROM_I2C_BLOCK0_ADDRESS,g);
if (Wire.available())
{
for(c=0;c<g;c++)
{
Serial.print((char)Wire.read()); 
++counter;
if(counter>=97)
{
Serial.println();
counter=0;  
}

}
}
p+=32;

}


read_fault_logg=0;  

}


void READ_DATA_LOGG(void)
{
  
long int t=0;  
byte g=0,counter=0;

if(data_logg_ptr==21504 && data_logg_overwrite_indicator==0)
{
Serial.println("LOGG IS EMPTY");
Serial.println();  
}

if(data_logg_ptr > 21504 && data_logg_overwrite_indicator==0)
{
p= 21504;
t= data_logg_ptr;
}

 if(data_logg_overwrite_indicator==1)
{
p=21504;
t=131003;
}
  
while(p<t)
{
wdt_reset();

if(p<=65535)
{  
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
Wire.write(p >> 8);   // MSB
Wire.write(p & 0xFF);   // LSB
Wire.endTransmission();
}
else if(p>65535)
{
Wire.beginTransmission(EEPROM_I2C_BLOCK1_ADDRESS);
Wire.write((p-65536) >> 8);   // MSB
Wire.write((p-65536) & 0xFF);   // LSB
Wire.endTransmission();
}


 if(p+97 < 65536)
  {

  if(t-p < 32)
  {
g=t-p;
  }
else
{
 g=32; 
}


Wire.requestFrom(EEPROM_I2C_BLOCK0_ADDRESS,g);
if (Wire.available())
{
for(c=0;c<g;c++)
{
Serial.print((char)Wire.read()); 
++counter;
if(counter>=97)
{
Serial.println();
counter=0;  
}

}
}
p+=32;

}


else if(p >= 65536)
  {


  if(t-p < 32)
  {
g=t-p;
  }
else
{
 g=32; 
}


Wire.requestFrom(EEPROM_I2C_BLOCK1_ADDRESS,g);
if (Wire.available())
{
for(c=0;c<g;c++)
{
Serial.print((char)Wire.read()); 
++counter;
if(counter>=97)
{
Serial.println();
counter=0;  
}

}
}
p+=32;

}


else if((p+32 > 65535) && (p< 65536))
  {
 int n= 65536-p;



Wire.requestFrom(EEPROM_I2C_BLOCK0_ADDRESS,n);

if (Wire.available())
{
for(c=0;c<n;c++)
{
Serial.print((char)Wire.read()); 
++counter;
if(counter>=97)
{
Serial.println();
counter=0;  
}

}
}
p+=n;

Wire.beginTransmission(EEPROM_I2C_BLOCK1_ADDRESS);
Wire.write((p-65536) >> 8);   // MSB
Wire.write((p-65536) & 0xFF);   // LSB
Wire.endTransmission();

n=32-n;
Wire.requestFrom(EEPROM_I2C_BLOCK1_ADDRESS,n);

if (Wire.available())
{
for(c=0;c<n;c++)
{
Serial.print((char)Wire.read()); 
++counter;
if(counter>=97)
{
Serial.println();
counter=0;  
}

}
}
p+=n;



//Serial.println();
  }


}


read_data_logg=0;  
  
}


void CHECK_FAULT_DATA_LOGG_READ_CMD(void)
{

if(read_fault_logg==1)
{
READ_FAULT_LOGG();
read_fault_logg=0; 
}

if(read_data_logg==1)
{
READ_DATA_LOGG();
read_data_logg=0;
}

}




void adc_add()
{
  _v0+=SPV_FINAL;
  _v1+= BATTERY_VOLTAGE;
  _v2+=LOAD_VOLTAGE;
  _i0+=ARRAY_CURRENT;
  _i1+=BATTERY_CURRENT;
  _i2+=LOAD_CURRENT;
 _pow+=LOAD_POWER;
}

void KEYPAD_OPERATION(void)
{
char key = kpd.getKey(); //storing pressed key value in a char
  if (key != NO_KEY)
{
 // Serial.println(key);  
  //lcd.print(key);
if(set_comp_flag==0)
{
  

if(Key_Shortcut[0]=='*')
{
Key_Shortcut+=key ;       

if(Key_Shortcut.length()==6)
{
if((Key_Shortcut[0]=='*') && (Key_Shortcut[5]=='#') )
{
invalid_shortcut=1;
cursor_count=0;
for(c=0;c<=13;c++)
{
if(Key_Shortcut==shortcut_code[c])
{  
invalid_shortcut=0;      
}
}

if(Key_Shortcut==shortcut_code[0])
{  
lcd.clear();
Key_Shortcut="";  
}

if(Key_Shortcut==shortcut_code[1])
{  
display=1;
lcd.clear();
Key_Shortcut="";  
}

if(Key_Shortcut==shortcut_code[2])
{  
display=2;
lcd.clear();
Key_Shortcut="";  
}

if(Key_Shortcut==shortcut_code[3])
{  
display=3;
lcd.clear();
Key_Shortcut="";  
}
if(Key_Shortcut==shortcut_code[4])
{  
display=4;
lcd.clear();
Key_Shortcut="";  
}

if(Key_Shortcut==shortcut_code[5])
{  
display=5;
lcd.clear();
Key_Shortcut="";  
}

if(Key_Shortcut==shortcut_code[6])
{  
display=0;
lcd.clear();
Key_Shortcut="";  
}


if(Key_Shortcut==shortcut_code[7])
{  
display=6;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=1;
}

if(Key_Shortcut==shortcut_code[8])
{  
display=7;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=2;
}


if(Key_Shortcut==shortcut_code[9])
{  
display=8;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=3;
}


if(Key_Shortcut==shortcut_code[10])
{  
display=9;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=4;
}


if(Key_Shortcut==shortcut_code[11])
{  
display=10;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=5;
}



if(Key_Shortcut==shortcut_code[12])
{  
display=11;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=6;
}

if(Key_Shortcut==shortcut_code[13])
{  
display=12;
lcd.clear();
Key_Shortcut="";  
set_comp_flag=1;
//key='\0';
W_EEPROM_PARA=7;
}

if(set_comp_flag==1)
{
free_timer2=1;
}

if(invalid_shortcut==1)
{
lcd.clear();  
lcd.print("INVALID SHORTCUT");  
 Key_Shortcut="";   
}

}//if((Key_Shortcut[0]=='*') && (Key_Shortcut[5]=='#') )
}//Key_Shortcut.length()==6

}//key_shortcut[0]==*

if(key=='*')
{
Key_Shortcut=""; 
Key_Shortcut+=key;
}

}//if(set_comp_flag==0)



if(set_comp_flag==1)
{

if(Key_Shortcut.length()==5)
{    
lcd.noBlink();

if(key== '#')
{ 
para_temp=Key_Shortcut.toFloat();
Key_Shortcut="";
user_wants_save=1;   
//value_completed=0;
display_save_exit=0;
Store_comparable_parameters();
timeout_timer=0;
free_timer2=0;
}

if(key== '*')
{  
display=0;
//value_completed=0;
display_save_exit=0;
set_comp_flag=0;
lcd.clear();
set_edit_cursor=0;
timeout_timer=0;
Key_Shortcut="";
free_timer2=0;
}


}
else
{
 //lcd.clear();
//lcd.print("EXIT : INVALID VALUE");  
}

if(Key_Shortcut.length()<5)
{ 
   
if(Key_Shortcut.length()!=2)
{
if(key>='0' && key <='9' )
{ 
Key_Shortcut+=key;
lcd.print(key);
if(Key_Shortcut.length()==5)
{ 
value_completed=1;
lcd.noBlink();
timeout_timer=1;
}
}}

if(Key_Shortcut.length()==2 && key=='*')
{
Key_Shortcut+='.'; 
lcd.print(".");
}
}


}
  
/*++ cursor_count;
if(cursor_count>=16)
{
//lcd.setCursor(0,0);  
cursor_count=0;
}*/


}//if key!=no key

} 



void READ_SERIAL_PORT(void)
{
  //s=Serial1.readStringUntil('\n');
if(Serial.available()!=0)
{
x=(char)(Serial.read());
if(x!='\n')
{
s+=x;
}

if(x=='\n')
{
//Serial.println(s);

if(s=="*#RFL#\r")
{
  Serial.println();
Serial.println("Please Wait....");
Serial.println("FAULT LOGG : ");
read_fault_logg=1;
s="";
}
else if(s=="*#RDL#\r")
{
Serial.println();  
Serial.println("Please Wait....");
Serial.println("DATA LOGG : ");
read_data_logg=1;
s="";
}
else
{
s="";
}
}

}

}



void READ_SERIAL_PORT1(void)
{
if(Serial1.available()>0)
{
Serialport1_rx+=(char)Serial1.read();
}
  
}


void READ_ZIGBEE_DATA(void)
{
if(Serial2.available()!=0)
{
y=(char)(Serial2.read());
if(y!='\n' && y!='\r')
{
zigbee_string +=y;
}

if(y=='\n')
{
//zigbee_string +=y;  
if(zigbee_string.length()==85)
{  

//if(_second<=5)
//{
//if(_minute ==1 || _minute ==6 || _minute ==11 || _minute ==16 || _minute ==21 || _minute ==26 || _minute ==31 || _minute ==36 || _minute ==41 || _minute ==46 || _minute ==51 || _minute ==56)
//{  
zigbee_buffer="{\"Description\":\""+zigbee_string + "\"}";

//}

//}

}
//Serial.print(zigbee_string);
//Serial.println(zigbee_string.length());
zigbee_string="";
}
  
}
}








void SEND_SIM900A_COMMAND(String x,String y)
{
//byte count=0;  
retry=0;
temp="";
  
m0 : Serial1.println(x); /* Check Communication */

while(temp.indexOf(y)==-1 && retry <3)
{
SIM900A_RESPONSE();
if((temp.indexOf("ERROR")!=-1 )) 
{
++retry;
Serial.println("entered");
//Serial.print(temp);
if(retry<3)
{
temp="";
goto m0;
}
}
//++count;
}

}




void SEND_SMS(String x)
{

  /*
   
   AT+CMGF=1

OK
 
    AT+CMGS="+919816686443"

> 
   "HI Vinod"

>  

+CMGS: 5

OK

   */
Serial1.println("AT+CMGF=1");    
delay(1000);
 
//Replace XXXXXXXXXX to 10 digit mobile number &  ZZ to 2 digit country code
Serial1.println("AT+CMGS=\"+919816686443\"");    
delay(1000);

//The text of the message to be sent.
Serial1.println("HI Vinod");   
delay(1000);

Serial1.println((char)26); 
delay(1000);

}


void setM41T00time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year)
{
  // sets time and date data to M41T00
  Wire.beginTransmission(M41T00_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(0x80)); //Stop osc
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.write(0x07);
  Wire.endTransmission();
  delay(5);
  Wire.beginTransmission(M41T00_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // // set seconds,start osc
  Wire.endTransmission();
  delay(1000);
}


void readM41T00time(byte *second,
byte *minute,
byte *hour,
byte *dayOfWeek,
byte *date,
byte *month,
byte *year)
{
  Wire.beginTransmission(M41T00_I2C_ADDRESS);
  Wire.write(0); // set M41T00 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(M41T00_I2C_ADDRESS, 7);
  // request seven bytes of data from M41T00 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *date = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}


void displayTime()
{
/*  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from M41T00
  readM41T00time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);
 */
 
  // send it to the serial monitor
  Serial.print(_hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (_minute<10)
  {
    Serial.print("0");
  }
  Serial.print(_minute, DEC);
  Serial.print(":");
  if (_second<10)
  {
    Serial.print("0");
  }
  Serial.print(_second, DEC);
  Serial.print(" ");
  Serial.print(_date, DEC);
  Serial.print("/");
  Serial.print(_month, DEC);
  Serial.print("/");
  Serial.print(_year, DEC);
  Serial.print(" Day of week: ");
  switch(_dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }
}



void displayTime_lcd()
{
/*  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from M41T00
  readM41T00time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
  &year);*/

   lcd.clear();


 // Formatting and displaying time
    lcd.setCursor(4,0);
      if (_hour < 10) lcd.print("0");
        lcd.print(_hour); lcd.print(":"); 
      if (_minute < 10) lcd.print("0");
        lcd.print(_minute); lcd.print(":"); 
      if (_second < 10) lcd.print("0");
        lcd.print(_second);
    lcd.setCursor(2,1);

  _dayOfWeek=dayofweek( _date,  _month,  _year);
    // Formatting and displaying date
    lcd.print(days[_dayOfWeek]); lcd.print(" ");
      if (_date < 10) lcd.print("0");
        lcd.print(_date); lcd.print(".");
      if (_month < 10) lcd.print("0");
        lcd.print(_month); lcd.print(".");
        lcd.print(_year);  
  

}



void _DISPLAY(void)
{
  if(display<5)
  {
lcd.setCursor(0,0);  
  }

if(clear_display_flag==1)
{
  LiquidCrystal lcd(13,12,8,9,10,11);

lcd.begin(16,2);//lcd begin 16*2
//lcd.setCursor(0,0);//set cursor to 1st row and 1st column
lcd.clear();
clear_display_flag=0;
}


if(invalid_shortcut==1)
{
lcd.clear();  
invalid_shortcut=0;
}

  
//SP VOLTAGE AND BATTERY VOLTAGE  
if(display==0)
{
lcd.print("SPV : "); 
if(SPV_FINAL<10)
{
lcd.print("0");  
}
lcd.print(SPV_FINAL);
lcd.print("v");  
lcd.setCursor(0,1);  
 
lcd.print("BV  : "); 
if(BATTERY_VOLTAGE<10)
{
lcd.print("0");  
}
lcd.print(BATTERY_VOLTAGE);
lcd.print("v");  
 

}

//SHOW GRID ID
if(display==1)
{
lcd.print("G_ID : ");  
lcd.print(GRID_ID); 
  
}



//SP VOLTAGE AND CURRENT
if(display==2)
{
lcd.print("SPV : "); 
if(SPV_FINAL<10)
{
lcd.print("0");  
}
lcd.print(SPV_FINAL);
lcd.print("v");  
lcd.setCursor(0,1);  
 
lcd.print("SPC : "); 
if(ARRAY_CURRENT<10)
{
lcd.print("0");  
}
lcd.print(ARRAY_CURRENT);
lcd.print("A");  
 
  
}

//BATTERY VOLTAGE AND CURRENT
if(display==3)
{
lcd.print("BV : "); 
if(BATTERY_VOLTAGE<10)
{
lcd.print("0");  
}
lcd.print(BATTERY_VOLTAGE);
lcd.print("v");  
lcd.setCursor(0,1);  

lcd.print("BC : "); 
if(BATTERY_CURRENT<10)
{
lcd.print("0");  
}
lcd.print(BATTERY_CURRENT);
lcd.print("A");   
}

//LOAD VOLTAGE AND CURRENT
if(display==4)
{
lcd.print("LV : "); 
if(LOAD_VOLTAGE<10)
{
lcd.print("0");  
}
lcd.print(LOAD_VOLTAGE);
lcd.print("v");  
lcd.setCursor(0,1);  

lcd.print("LC : "); 
if(LOAD_CURRENT<10)
{
lcd.print("0");  
}
lcd.print(LOAD_CURRENT);
lcd.print("A");   
}

//SHOW DATE AND TIME
if(display==5)
{
  displayTime_lcd();
}


//Set battery low level
if(display==6)
{
 if( set_edit_cursor==0)
 { 
lcd.print("LOW_BATT : ");
if(BATTERY_LOW_LEVEL<10)
{
 lcd.print("0"); 
}
lcd.print(BATTERY_LOW_LEVEL);
lcd.setCursor(11,0);
lcd.blink();
set_edit_cursor=1;
 }
 
}




//Set HIGH_BATTERY_CURRENT
if(display==7)
{
 if( set_edit_cursor==0)
 { 
lcd.print("H BATT C : ");
if(HIGH_BATTERY_CURRENT<10)
{
 lcd.print("0"); 
}

lcd.print(HIGH_BATTERY_CURRENT);
lcd.setCursor(11,0);
lcd.blink();
set_edit_cursor=1;
 }

}

//Set  V_AT_HIGH_BATTERY_CURRENT
if(display==8)
{
 if( set_edit_cursor==0)
 { 
lcd.print("V @ HBC : ");
if(V_AT_HIGH_BATTERY_CURRENT<10)
{
 lcd.print("0"); 
}
lcd.print( V_AT_HIGH_BATTERY_CURRENT);
lcd.setCursor(10,0);
lcd.blink();
set_edit_cursor=1;
 }
 
}




//Set LOAD_SC_RISE
if(display==9)
{
 if( set_edit_cursor==0)
 { 
lcd.print("L SC R : ");
if(LOAD_SC_RISE<10)
{
 lcd.print("0"); 
}
lcd.print(LOAD_SC_RISE);

lcd.setCursor(9,0);
lcd.blink();
set_edit_cursor=1;
 }
 }



//Set LOAD_HIGH_CURRENT
if(display==10)
{
 if( set_edit_cursor==0)
 { 
lcd.print("LOAD HC : ");
if(LOAD_HIGH_CURRENT<10)
{
 lcd.print("0"); 
}
lcd.print(LOAD_HIGH_CURRENT);
lcd.setCursor(10,0);
lcd.blink();
set_edit_cursor=1;
 }
 
}


//BATTERY_CHG_VOLTAGE
if(display==11)
{
 if( set_edit_cursor==0)
 { 
lcd.print("BCHG V : ");
if(BATTERY_CHG_VOLTAGE<10)
{
 lcd.print("0"); 
}
lcd.print(BATTERY_CHG_VOLTAGE);
lcd.setCursor(9,0);
lcd.blink();
set_edit_cursor=1;
 }
 
}




//MINIMUM_SPV
if(display==12)
{
 if( set_edit_cursor==0)
 { 
lcd.print("MIN SPV : ");
if(MINIMUM_SPV<10)
{
 lcd.print("0"); 
}
lcd.print(MINIMUM_SPV);
lcd.setCursor(10,0);
lcd.blink();
set_edit_cursor=1;
 }
}



if(display>=6)
{
if(display_save_exit==2)
{
lcd.setCursor(0,0); 
lcd.print("Press * to exit");  
display_save_exit=1;
}
else
{
if(display_save_exit==1)
{
lcd.setCursor(0,0);  
lcd.print("Press # to save");  
display_save_exit=2;
}
}

}

if(value_completed==1)
{
lcd.clear();
lcd.print("Press # to save");  
lcd.setCursor(0,1);

if(display==6)
{
lcd.print("LOW_BATT : ");
}

if(display==7)
{
lcd.print("H BATT C : ");
}

if(display==8)
{
lcd.print("V @ HBC : ");
}

if(display==9)
{
lcd.print("L SC R : ");
}

if(display==10)
{
lcd.print("LOAD HC : ");
}

if(display==11)
{
lcd.print("BCHG V : ");
}

if(display==12)
{
lcd.print("MIN SPV : ");
}
lcd.print(Key_Shortcut);
value_completed=0;
display_save_exit=2;
}

  
}



void ReadTime()
{
  
  // retrieve data from M41T00
  readM41T00time(&second, &minute, &hour,&dayOfWeek ,&date, &month,
  &year);
}

/*

AT+CLTS=1

OK
AT+CTZU=1

OK
AT+CCLK?

+CCLK: "18/04/10,19:48:59+22"

OK
 */

void GET_Network_TIME(void)
{
lcd.print("Sync D&T...");  

/*Serial1.println("AT+CLTS=1"); /* Check Communication */
/*delay(2000);
ShowSerialData();*/

SEND_SIM900A_COMMAND("AT+CLTS=1","AT+CLTS=1\r\n\r\nOK\r\n");
/*Serial1.println("AT+CTZU=1"); /* Check Communication */
/*delay(2000);
ShowSerialData();*/
SEND_SIM900A_COMMAND("AT+CTZU=1","AT+CTZU=1\r\n\r\nOK\r\n");
/*Serial1.println("AT+CCLK?"); /* Check Communication */
//delay(1000);
//ShowSerialData();
//SIM900A_RESPONSE();
//Serial.println(temp);


SEND_SIM900A_COMMAND("AT+CCLK?","OK\r\n");
Serial.println(temp);

p=temp.indexOf("+CCLK:");
//Serial.println(p);

ss=bcdToDec(temp[p+23]<<4 | temp[p+24] & 0x0f);
mm=bcdToDec(temp[p+20]<<4 | temp[p+21] & 0x0f);
hh=bcdToDec(temp[p+17]<<4 | temp[p+18] & 0x0f);
dd=bcdToDec(temp[p+14]<<4 | temp[p+15] & 0x0f);
mo=bcdToDec(temp[p+11]<<4 | temp[p+12] & 0x0f);
yy=bcdToDec(temp[p+8]<<4 | temp[p+9] & 0x0f);

_second=ss;
_minute=mm;
_hour=hh;
_date=dd;
_month=mo;
_year=yy;



if(ss<59)
{
ss+=1;  
}
day=dayofweek( dd,  mo,  yy);
_dayOfWeek=day;
}



void SEND_SIM900A_TIME(void)
{
if(time_cmd==0 && send_data_cmd==0)
{
Serial1.println("AT+CCLK?"); /* Check Communication */
time_cmd=1;
request_timeout=1;
//if(STEP!=0)
//{
 // Serial.print("Step no. from transmit ");

// Serial.println(STEP); 
//}

} 
}


void RECEIVE_SIM900A_TIME(void)
{
if((Serialport1_rx.indexOf("OK\r\n")!= -1) && (Serialport1_rx.indexOf("+CCLK:")!= -1) && time_cmd==1)
{
//Serial.println(Serialport1_rx);  
//Serial.print("Step no. from receive");
//Serial.println(STEP);
p=Serialport1_rx.indexOf("+CCLK:");
_second=bcdToDec(Serialport1_rx[p+23]<<4 | Serialport1_rx[p+24] & 0x0f);
_minute=bcdToDec(Serialport1_rx[p+20]<<4 | Serialport1_rx[p+21] & 0x0f);
_hour=bcdToDec(Serialport1_rx[p+17]<<4 | Serialport1_rx[p+18] & 0x0f);
_date=bcdToDec(Serialport1_rx[p+14]<<4 | Serialport1_rx[p+15] & 0x0f);
_month=bcdToDec(Serialport1_rx[p+11]<<4 | Serialport1_rx[p+12] & 0x0f);
_year=bcdToDec(Serialport1_rx[p+8]<<4 | Serialport1_rx[p+9] & 0x0f);
time_cmd=0;  
Serialport1_rx="";
retry_time_read=0;
request_timeout=0;

}


}




void SYNC_DATE_TIME(void)
{
GET_Network_TIME();  

// set the initial time here:
// M41T00 seconds, minutes, hours, day, date, month, year
setM41T00time(ss,mm,hh,day,dd,mo,yy);

displayTime();
Serial.println();
}


int dayofweek(int d, int m, int y)
{
    static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
   y -= m < 3;
    return ((( y + y/4 - y/100 + y/400 + t[m-1] + d) % 7)+1);
}
 


void SPV_POSITIVE_READ(void)
{
v0[k] = analogRead( SPV_POSITIVE_PORT);   
}



void SPV_NEGATIVE_READ(void)
{
v00[k] = analogRead( SPV_NEGATIVE_PORT);     
}





void SPV_FINAL_READ(void)
{
/* if(SPV_NEGATIVE-SPV_NEG_IN > 0)
 {
SPV_FINAL = SPV_POSITIVE - ( SPV_NEGATIVE - SPV_NEG_IN );
}
 else
 {
SPV_FINAL = SPV_POSITIVE + (SPV_NEG_IN - SPV_NEGATIVE);
 }*/

SPV_FINAL = SPV_POSITIVE - SPV_NEGATIVE;

 if(SPV_FINAL < 0)
 {
  SPV_FINAL=0;
 }

}


void ARRAY_CURRENT_READ(void)
{
 i0[k] = analogRead(ARRAY_CURRENT_PORT);
 
 i0[k]-=512;
}




void BATTERY_VOLTAGE_READ(void)
{
 v1[k] = analogRead(BATTERY_VOLTAGE_PORT);
}

void BATTERY_CURRENT_READ(void)
{
i1[k] = analogRead(BATTERY_CURRENT_PORT);
 i1[k]-=512;
}


void LOAD_VOLTAGE_READ(void)
{
 v2[k] = analogRead(LOAD_VOLTAGE_PORT); 

}


void LOAD_CURRENT_READ(void)
{
 i2[k] = analogRead(LOAD_CURRENT_PORT);
 i2[k]-=512;
  LOAD_CURRENT1=i2[k]*I_LOAD_MULT;
  if(LOAD_CURRENT1>0)
  {
  LOAD_CURRENT1+=LOAD_CURRENT_OFFSET;
  }
}




void GC_TEMPERATURE_READ(void)
{

TEMP_GC[k]= analogRead(GC_TEMPERATURE_PORT);
 }

void AMB_TEMPERATURE_READ()
{
TEMP_AMB[k]= analogRead(AMB_TEMPERATURE_PORT);
}



void AVG_ALL_PARAMETERS(void)
{

if(k>=9)
{
SPV_POSITIVE = (((v0[0]+v0[1]+v0[2]+v0[3]+v0[4]+v0[5]+v0[6]+v0[7]+v0[8]+v0[9])/10) * SPV_POSITIVE_MULT); 
SPV_NEGATIVE  = (( ( ( ( (v00[0]+v00[1]+v00[2]+v00[3]+v00[4]+v00[5]+v00[6]+v00[7]+v00[8]+v00[9])/10) * 5.0) / 1024)-2.5) * 12); 

SPV_FINAL_READ();

ARRAY_CURRENT = (((i0[0]+i0[1]+i0[2]+i0[3]+i0[4]+i0[5]+i0[6]+i0[7]+i0[8]+i0[9])/10) * I_ARRAY_MULT); 
 if( ARRAY_CURRENT<0 )
 {
 ARRAY_CURRENT=0;
 }
 else
 {
  ARRAY_CURRENT+=SP_CURRENT_OFFSET;
 }


BATTERY_VOLTAGE = (((v1[0]+v1[1]+v1[2]+v1[3]+v1[4]+v1[5]+v1[6]+v1[7]+v1[8]+v1[9])/10) * V_BATT_MULT); 
BATTERY_CURRENT = (((i1[0]+i1[1]+i1[2]+i1[3]+i1[4]+i1[5]+i1[6]+i1[7]+i1[8]+i1[9])/10) * I_ARRAY_MULT); 

 if( BATTERY_CURRENT<0 )
 {
 BATTERY_CURRENT=0;
 }
 else
 {
   BATTERY_CURRENT+=BATT_CURRENT_OFFSET;
 }


LOAD_VOLTAGE = (((v2[0]+v2[1]+v2[2]+v2[3]+v2[4]+v2[5]+v2[6]+v2[7]+v2[8]+v2[9])/10) * V_LOAD_MULT); 
LOAD_CURRENT = (((i2[0]+i2[1]+i2[2]+i2[3]+i2[4]+i2[5]+i2[6]+i2[7]+i2[8]+i2[9])/10) * I_LOAD_MULT); 


 if( LOAD_CURRENT<=0 )
 {
LOAD_CURRENT=0;
 }
 else
 {
  LOAD_CURRENT+=LOAD_CURRENT_OFFSET; 
 }

LOAD_POWER = LOAD_VOLTAGE * LOAD_CURRENT;

GC_TEMPERATURE = (((TEMP_GC[0]+TEMP_GC[1]+TEMP_GC[2]+TEMP_GC[3]+TEMP_GC[4]+TEMP_GC[5]+TEMP_GC[6]+TEMP_GC[7]+TEMP_GC[8]+TEMP_GC[9])/10) * TEMP_MULT); 

if(GC_TEMPERATURE>99)
{
  GC_TEMPERATURE=0;
}

AMB_TEMPERATURE = (((TEMP_AMB[0]+TEMP_AMB[1]+TEMP_AMB[2]+TEMP_AMB[3]+TEMP_AMB[4]+TEMP_AMB[5]+TEMP_AMB[6]+TEMP_AMB[7]+TEMP_AMB[8]+TEMP_AMB[9])/10) * TEMP_MULT); 
//  AMB_TEMPERATURE=35.69;

if(AMB_TEMPERATURE>99)
{
 AMB_TEMPERATURE=0; 
}


p_init=1;
}


if(k<9)
{
++k;
}
else
{
k=0;
}

  
}
        


void ALL_AVG_PARA(void)
{
_SPV_FINAL=_v0/count;  
_BATTERY_VOLTAGE=_v1/count;  
_LOAD_VOLTAGE=_v2/count;
_ARRAY_CURRENT=_i0/count;  
_BATTERY_CURRENT=_i1/count; 
 
_LOAD_CURRENT=_i2/count;
/*if(_LOAD_CURRENT>0)
{
_LOAD_CURRENT+=LOAD_CURRENT_OFFSET;
}*/

_LOAD_POWER= _pow/count;
}


void CHECK_ALL_FAULT(void)
{ 
//Check battery LOW LEVEL
if(BATTERY_VOLTAGE<=BATTERY_LOW_LEVEL)
{  
digitalWrite(BATTERY_FAULT,HIGH); 
digitalWrite(BUZZER_PORT,BUZZER_ON); 
digitalWrite(RELAY_PORT,RELAY_ON); 
battery_v_fault=1; 
FINAL_STRING("20");

if(Final.length()==115 && lbv_timer==0 ) 
{
 
LOW_BATTERY_VOLTAGE=Final;
}
else
{
 Final=""; 
}

}
else
{
battery_v_fault=0;
lbv_timer=0;
if(battery_i_fault==0)  
{
digitalWrite(BATTERY_FAULT,LOW);   
//digitalWrite(BUZZER_PORT,BUZZER_ON); 
digitalWrite(RELAY_PORT,RELAY_OFF);  
}
}


//Check battery Current & VOLTAGE
if(BATTERY_VOLTAGE>=V_AT_HIGH_BATTERY_CURRENT)
{
if(BATTERY_CURRENT>HIGH_BATTERY_CURRENT)
{  
digitalWrite(BATTERY_FAULT,HIGH); 
battery_i_fault=1;
FINAL_STRING("21");
if(Final.length()==115 && hbc_timer==0)
{
S_HIGH_BATTERY_CURRENT=Final;
}
else{
  Final="";
}
}
else
{   
  battery_i_fault=0;
  hbc_timer=0;
 if(battery_v_fault==0)  
{  
digitalWrite(BATTERY_FAULT,LOW);   
}}

}


//Check SP Fault
if((ARRAY_CURRENT==0.0) && (_hour<19) && (_hour>=7)  && (BATTERY_VOLTAGE<=BATTERY_CHG_VOLTAGE) && (SPV_FINAL<MINIMUM_SPV) )
{
digitalWrite(ARRAY_FAULT,HIGH); 
FINAL_STRING("10");

 if(Final.length()==115  && sp_timer==0)
{
 SP_CURRENT=Final;
}
else
{
  Final="";
}
}
else
{
 sp_timer=0; 
digitalWrite(ARRAY_FAULT,LOW);
}

//Check load high current
if(LOAD_CURRENT>LOAD_HIGH_CURRENT)
{
digitalWrite(LOAD_FAULT,HIGH); 
LOAD_HIGH_CURRENT_FAULT=1;
FINAL_STRING("31");
if(Final.length()==115 && lhc_timer==0)
{
S_LOAD_HIGH_CURRENT=Final;
}
else
{
  LOAD_CURRENT_STRING="";
}
}
else
{
 if(LOAD_HIGH_CURRENT_FAULT==1)
 {   
digitalWrite(LOAD_FAULT,LOW); 
LOAD_HIGH_CURRENT_FAULT=0;
lhc_timer=0;
}}

//check load is open?
if(LOAD_CURRENT==0)
{
digitalWrite(LOAD_FAULT,HIGH); 
digitalWrite(BUZZER_PORT,BUZZER_ON); 
digitalWrite(RELAY_PORT,RELAY_ON); 
LOAD_OPEN_FAULT=1; 
FINAL_STRING("32");
//Serial.println(Final.length());

if(Final.length()==115 && lo_timer==0)
{
LOAD_OPEN=Final;
}
else
{
 LOAD_CURRENT_STRING=""; 
}

}
else
{
 if(LOAD_OPEN_FAULT==1)
 {   
digitalWrite(LOAD_FAULT,LOW); 
//digitalWrite(BUZZER_PORT,BUZZER_OFF); 
digitalWrite(RELAY_PORT,RELAY_OFF);  
LOAD_OPEN_FAULT=0;  
lo_timer=0;
}}

}





void CHECK_15MIN_FAULT(void)
{

any_fault=0; 
//Check battery LOW LEVEL
if(_BATTERY_VOLTAGE<=BATTERY_LOW_LEVEL)
{  
any_fault=1;
if(_15min_flag==1)
{
_FINAL_STRING("20");
if(_Final.length()==115)
{ 
_15min_string1=_Final;
}
else
{
  _Final="";
}

}}



//Check battery Current & VOLTAGE
if(_BATTERY_VOLTAGE>=V_AT_HIGH_BATTERY_CURRENT)
{
if(_BATTERY_CURRENT>HIGH_BATTERY_CURRENT)
{  
any_fault=1;
if(_15min_flag==1)
{
_FINAL_STRING("21");
if(_Final.length()==115)
{ 
_15min_string2=_Final;
}
else
{
  _Final="";
}}}}



//Check SP Fault
if((_ARRAY_CURRENT==0.0) && (_hour<19) && (_hour>=7)  && (_BATTERY_VOLTAGE<=BATTERY_CHG_VOLTAGE) && (_SPV_FINAL<MINIMUM_SPV) )
{
any_fault=1;
if(_15min_flag==1)
{
_FINAL_STRING("10");
if(_Final.length()==115)
{ 
_15min_string3=_Final;
}
else
{
  _Final="";
}
}
}

//Check load high current
if(_LOAD_CURRENT>LOAD_HIGH_CURRENT)
{
any_fault=1;
if(_15min_flag==1)
{
_FINAL_STRING("31");
if(_Final.length()==115)
{ 
_15min_string4=_Final;
}
else
{
  _Final="";
}

}}


//check load is open?
if(_LOAD_CURRENT==0)
{
any_fault=1;
if(_15min_flag==1)
{
_FINAL_STRING("32");
if(_Final.length()==115)
{ 
_15min_string5=_Final;
}
else
{
  _Final="";
}

}}

/*if(_15min_flag==1)
{
CHECK_SHORTCIRCUIT_FAULT();  
}*/


if(any_fault==0)
{
_FINAL_STRING("00");
if(_15min_flag==1 && _Final.length()==115)
{
_15min_string1=_Final;
}}
  
}


void CHECK_SHORTCIRCUIT_FAULT(void)
{
if(first_current_read==0)
{
first_current_read=1;
previous_value =  LOAD_CURRENT1;
}
  
 if(LOAD_CURRENT1-previous_value> LOAD_SC_RISE)
{
digitalWrite(LOAD_FAULT,HIGH); 
LOAD_ALARM_CODE="30";
digitalWrite(BUZZER_PORT,BUZZER_ON); 
digitalWrite(RELAY_PORT,RELAY_ON); 

//Serial.println(LOAD_CURRENT1-previous_value);
//ReadTime();  
//any_fault=1;
LOAD_PARA_STRING(LOAD_ALARM_CODE);

/*if(_15min_flag==1)
{
_FINAL_STRING("30");
if(_Final.length()==115)
{ 
_15min_string6=_Final;
}
else
{
  _Final="";
}

}*/



if(LOAD_CURRENT_STRING.length()==115  && lsc_timer==0)
{
LOAD_SHORTCIRCUIT=LOAD_CURRENT_STRING;
}
else
{
 LOAD_CURRENT_STRING="";
}
LOAD_SC_FAULT=1;
}
else
{
if(LOAD_SC_FAULT==1)
{ 
digitalWrite(LOAD_FAULT,LOW); 
//digitalWrite(BUZZER_PORT,BUZZER_OFF); 
digitalWrite(RELAY_PORT,RELAY_OFF);  
LOAD_ALARM_CODE="00";
LOAD_SC_FAULT=0;
lsc_timer=0;
}}
previous_value=LOAD_CURRENT1;   
}



void CHECK_FAULT_TIME(void)
{
  if(lbv_timer>0)
 {
 ++lbv_timer;  

if(lbv_timer>=10801)
{
 lbv_timer=0; 
}}

if(hbc_timer>0)
 {
 ++hbc_timer;  

if(hbc_timer>=10801)
{
 hbc_timer=0; 
}}

if(sp_timer>0)
 {
 ++sp_timer;  

if(sp_timer>=10801)
{
 sp_timer=0; 
}}

if(lhc_timer>0)
 {
 ++lhc_timer;  

if(lhc_timer>=10801)
{
 lhc_timer=0; 
}}


if(lo_timer>0)
 {
 ++lo_timer;  

if(lo_timer>=10801)
{
 lo_timer=0; 
}}


if(lsc_timer>0)
 {
 ++lsc_timer;  

if(lsc_timer>=10801)
{
 lsc_timer=0; 
}}

 
}


void all_timeout_check(void)
{

if(free_timer>0)
{
++free_timer; 

if(free_timer>=101)
{
clear_display_flag=1;
free_timer=0;
display=0;  
set_edit_cursor=0;
}}


if(timeout_timer>0)
{
 ++ timeout_timer;
if(timeout_timer>=3001)
{

clear_display_flag=1;
timeout_timer=0;
display=0;  
set_comp_flag=0;
W_EEPROM_PARA=0;

}  

}
  
}


void Timeout(void)
{
if(free_timer2>0)
{
++free_timer2; 
if(free_timer2>120)
{
free_timer2=0;
Key_Shortcut="";
clear_display_flag=1;
timeout_timer=0;
display=0;  
lcd.noBlink();
set_comp_flag=0;
W_EEPROM_PARA=0;
set_edit_cursor=0;
}}

}

void FINAL_STRING(String temp)
{
Final="";  
Final="{\"Description\":\"G"+GRID_ID; 

  Final+="D";
  if (_date<10)
  {
  Final+="0";
  }
Final+=_date;

  if (_month<10)
  {
  Final+="0";
  }
  Final+=_month;

  if (_year<10)
  {
  Final+="0";
  }

  Final+=_year;
  
  Final+="T";
 if (_hour<10)
  {
    Final+="0";
  }
  Final+=_hour;
 
  if (_minute<10)
  {
  Final+="0";
  }
  Final+=_minute;
  Final+="FC";

 //G310001 D240318 T1350 AL00 SV00.00 SC00.00 BV00.00 BC00.00 LV00.00 LC00.00 LP00.00 ta27.3 tb35.9 L3500  Ca G#*
  Final+=temp;
  
  
Final+="SV";
if(SPV_FINAL<10)
{
  Final+="0"; 
}
Final+=SPV_FINAL;

Final+="SC";
if(ARRAY_CURRENT<10)
{
  Final+="0"; 
}
Final+=ARRAY_CURRENT;

Final+="BV";
if(BATTERY_VOLTAGE<10)
{
  Final+="0"; 
}
Final+=BATTERY_VOLTAGE;

Final+="BC";
if(BATTERY_CURRENT<10)
{
  Final+="0"; 
}
Final+=BATTERY_CURRENT;

Final+="LV";
if(LOAD_VOLTAGE<10)
{
  Final+="0"; 
}
Final+=LOAD_VOLTAGE;

Final+="LC";
if(LOAD_CURRENT<10)
{
  Final+="0"; 
}
Final+=LOAD_CURRENT;

Final+="LP"; 
if(LOAD_POWER < 100 && LOAD_POWER >= 10)
{
Final+="0";  
}
else if(LOAD_POWER < 10)
{
 Final+="00"; 
}

Final+=LOAD_POWER;
Final+= "ta";

if(AMB_TEMPERATURE<10)
{
  Final+="0"; 
}
Final+=AMB_TEMPERATURE;



Final+="tb";
if(GC_TEMPERATURE<10)
{
  Final+="0"; 
}
Final+=GC_TEMPERATURE;

Final+="L" + (String)LUX_VALUE + "C";

Final+="X";
//LV11.84LC00.44t29.79
//Final+=CAL_CHECKSUM(Final);
  
  Final+="G#*\"}";
//Final="";
}





void _FINAL_STRING(String temp)
{
_Final="";  
_Final="{\"Description\":\"G"+GRID_ID; 

  _Final+="D";
  if (_date<10)
  {
  _Final+="0";
  }
_Final+=_date;

  if (_month<10)
  {
  _Final+="0";
  }
  _Final+=_month;

  if (_year<10)
  {
  _Final+="0";
  }

  _Final+=_year;
  
  _Final+="T";
 if (_hour<10)
  {
    _Final+="0";
  }
  _Final+=_hour;
 
  if (_minute<10)
  {
  _Final+="0";
  }
  _Final+=_minute;
  _Final+="FC";
  _Final+=temp;
 //G310001 D240318 T1350 AL00 SV00.00 SC00.00 BV00.00 BC00.00 LV00.00 LC00.00 LP00.00 ta27.3 tb35.9 L3500  Ca G#*
  
_Final+="SV";
if(_SPV_FINAL<10)
{
  _Final+="0"; 
}
_Final+=_SPV_FINAL;

_Final+="SC";
if(_ARRAY_CURRENT<10)
{
  _Final+="0"; 
}
_Final+=_ARRAY_CURRENT;

_Final+="BV";
if(_BATTERY_VOLTAGE<10)
{
  _Final+="0"; 
}
_Final+=_BATTERY_VOLTAGE;

_Final+="BC";
if(_BATTERY_CURRENT<10)
{
  _Final+="0"; 
}
_Final+=_BATTERY_CURRENT;

_Final+="LV";
if(_LOAD_VOLTAGE<10)
{
  _Final+="0"; 
}
_Final+=_LOAD_VOLTAGE;

_Final+="LC";
if(_LOAD_CURRENT<10)
{
  _Final+="0"; 
}
_Final+=_LOAD_CURRENT;

_Final+="LP"; 
if(_LOAD_POWER < 100 && _LOAD_POWER >= 10)
{
_Final+="0";  
}
else if(_LOAD_POWER < 10)
{
 _Final+="00"; 
}

_Final+=_LOAD_POWER ;
_Final+="ta";

  if(AMB_TEMPERATURE<10)
{
  _Final+="0"; 
}
_Final+=AMB_TEMPERATURE;
  


_Final+="tb";
if(GC_TEMPERATURE<10)
{
  _Final+="0"; 
}
_Final+=(String)GC_TEMPERATURE + "L1300C";
_Final+="X";
//LV11.84LC00.44t29.79
//_Final+=CAL_CHECKSUM(_Final);
  
  _Final+="G#*\"}";


//_Final="";
  
}









void LOAD_PARA_STRING(String t)
{
LOAD_CURRENT_STRING="";  
LOAD_CURRENT_STRING="{\"Description\":\"G"+GRID_ID; 

  LOAD_CURRENT_STRING+="D";
  if (_date<10)
  {
  LOAD_CURRENT_STRING+="0";
  }
LOAD_CURRENT_STRING+=_date;

  if (_month<10)
  {
  LOAD_CURRENT_STRING+="0";
  }
  LOAD_CURRENT_STRING+=_month;

  if (_year<10)
  {
  LOAD_CURRENT_STRING+="0";
  }

  LOAD_CURRENT_STRING+=_year;
  
  LOAD_CURRENT_STRING+="T";
 if (_hour<10)
  {
    LOAD_CURRENT_STRING+="0";
  }
  LOAD_CURRENT_STRING+=_hour;
 
  if (_minute<10)
  {
  LOAD_CURRENT_STRING+="0";
  }
  LOAD_CURRENT_STRING+=_minute;

  LOAD_CURRENT_STRING+="FC";
  LOAD_CURRENT_STRING+=t;
  
LOAD_CURRENT_STRING+="SV";
if(SPV_FINAL<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=SPV_FINAL;
LOAD_CURRENT_STRING+="SC";
if(ARRAY_CURRENT<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=ARRAY_CURRENT;

LOAD_CURRENT_STRING+="BV";
if(BATTERY_VOLTAGE<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=BATTERY_VOLTAGE;

LOAD_CURRENT_STRING+="BC";
if(BATTERY_CURRENT<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=BATTERY_CURRENT;

LOAD_CURRENT_STRING+="LV";
if(LOAD_VOLTAGE<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=LOAD_VOLTAGE;

LOAD_CURRENT_STRING+="LC";
if(LOAD_CURRENT<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=LOAD_CURRENT;


LOAD_CURRENT_STRING+="LP"; 
if(LOAD_POWER < 100 && LOAD_POWER >= 10)
{
LOAD_CURRENT_STRING+="0";  
}
else if(LOAD_POWER < 10)
{
 LOAD_CURRENT_STRING+="00"; 
}

LOAD_CURRENT_STRING+=(String)LOAD_POWER + "ta";

//G310001 D240318 T1350 AL00 SV00.00 SC00.00 BV00.00 BC00.00 LV00.00 LC00.00 LP00.00 ta27.3 tb35.9 L3500  Ca G#*

  
  if(AMB_TEMPERATURE<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=AMB_TEMPERATURE;
 


LOAD_CURRENT_STRING+="tb";
if(GC_TEMPERATURE<10)
{
  LOAD_CURRENT_STRING+="0"; 
}
LOAD_CURRENT_STRING+=GC_TEMPERATURE;
LOAD_CURRENT_STRING+="L3500C";

//LV11.84LC00.44t29.79
//Calculate checksum
// LOAD_CAL_CHECKSUM();
 LOAD_CURRENT_STRING+="X";
LOAD_CURRENT_STRING+="G#*\"}"; 

//Serial.println(LOAD_CURRENT_STRING);
}


void LOAD_CAL_CHECKSUM(void)
{
char b_ck=16;
int a_ck=0;
while(b_ck!=103)
{
a_ck+=LOAD_CURRENT_STRING[b_ck] ;  
if((a_ck & 0x0100)==0x0100)
{
a_ck=a_ck & 0x00ff;  
a_ck=a_ck+1;    
}
++b_ck;
}
b_ck=a_ck;
b_ck=~b_ck;
LOAD_CURRENT_STRING+=b_ck;
}






char CAL_CHECKSUM(String u)
{
char b_ck=16;
int a_ck=0;
while(b_ck!=103)
{
a_ck+=u[b_ck] ;  
if((a_ck & 0x0100)==0x0100)
{
a_ck=a_ck & 0x00ff;  
a_ck=a_ck+1;    
}
++b_ck;
}
b_ck=a_ck;
b_ck=~b_ck;
return b_ck;
}


void Reset_hourly_data_flag(void)
{
if(hourly_data_logg==1)
{

if( _15min_string1=="" && _15min_string2=="" && _15min_string3=="" && _15min_string4=="" && _15min_string5=="")
{
hourly_data_logg=0;
}

}
}


void CHECK_INTERNET(void)
{
  
lcd.clear();
p=0;
lcd.print("Setting APN...");

SEND_SIM900A_COMMAND("AT+CSPN?","OK\r\n");
/*
Serial1.println("AT+CSPN?");  /* Start Get session */
/*delay(2000);
//ShowSerialData();

SIM900A_RESPONSE();*/

p=temp.indexOf("+CSPN:");
p+=8;
temp1="";
while(temp[p]!='"' && p<=50)
{
temp1+=temp[p]; 
++p; 
}

if(temp1=="airtel")
{ 
 SET_APN=APN + AIRTEL_APN + E_APN;
lcd.setCursor(0,1);
lcd.print(temp1);
temp1="";
}
else if(temp1=="internet")
{
 SET_APN=APN + IDEA_APN + E_APN;
}
else if(temp1=="www")
{
 SET_APN=APN + VODAFONE_APN + E_APN;
}
else if(temp1=="bsnlnet")
{
 SET_APN=APN + BSNL_APN + E_APN;
}
else
{

}
wdt_reset();

delay(1000);
lcd.clear();

 p=0;
 temp1="INTERNET....";
 while(temp1[p]!='\0')
 {
 lcd.print(temp1[p]);
 ++p;
 delay(125);
wdt_reset();

 }
 Serial.println("HTTP post method :");
 
 SEND_SIM900A_COMMAND("AT+CGATT=1","AT+CGATT=1\r\n\r\nOK\r\n");//Attaching to GPRS
Serial.println(temp);
wdt_reset();
 
  /*Serial1.println("AT+CGATT=1");  /* Initialize HTTP service */
  /*delay(2000); 
  ShowSerialData();*/

  /* Configure bearer profile 1 */
SEND_SIM900A_COMMAND("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"" , "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n\r\nOK\r\n");  /* Connection type GPRS */
Serial.println(temp);
wdt_reset();

  /*Serial1.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  /* Connection type GPRS */
  /*delay(2000);
  ShowSerialData();*/
  
  
 // Serial1.println("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");  /* APN of the provider */
//  Serial1.println("AT+SAPBR=3,1,\"APN\",\"internet\"");  /* APN of the provider */

SEND_SIM900A_COMMAND(SET_APN , SET_APN + "\r\n\r\nOK\r\n");  /* Connection type GPRS */
Serial.println(temp);
wdt_reset();

 /*Serial1.println(SET_APN);
delay(2000);
ShowSerialData();*/

SEND_SIM900A_COMMAND("AT+SAPBR=2,1","OK\r\n"); 
Serial.println(temp);
wdt_reset();

if(temp.indexOf("0.0.0.0")!= -1)
{
SEND_SIM900A_COMMAND("AT+SAPBR=1,1","AT+SAPBR=1,1\r\n\r\nOK\r\n"); // Open bearer
Serial.println(temp);
/* Serial1.println("AT+SAPBR=1,1"); // Enable GPRS
  delay(5000);
  ShowSerialData();*/
}


 
/*  Serial1.println("AT+SAPBR=2,1"); 
 delay(2000);
  ShowSerialData();*/
 
  /*Serial1.println("AT+HTTPINIT");  /* Initialize HTTP service */
  /*delay(5000); 
  ShowSerialData();*/

SEND_SIM900A_COMMAND("AT+HTTPINIT","AT+HTTPINIT\r\n\r\nOK\r\n"); /* Initialize HTTP service */
Serial.println(temp);
wdt_reset();

if(temp.indexOf("ERROR")!= -1)
{
SEND_SIM900A_COMMAND("AT+HTTPTERM","AT+HTTPTERM\r\n\r\nOK\r\n"); /* Terminate HTTP service */
Serial.println(temp);

SEND_SIM900A_COMMAND("AT+HTTPINIT","AT+HTTPINIT\r\n\r\nOK\r\n"); /* Initialize HTTP service */
Serial.println(temp);
}



/*  Serial1.println("AT+HTTPPARA=\"CID\",1");  /* Set parameters for HTTP session */
 /* delay(5000);
  ShowSerialData();*/


SEND_SIM900A_COMMAND("AT+HTTPPARA=\"CID\",1","AT+HTTPPARA=\"CID\",1\r\n\r\nOK\r\n"); /* Set parameters for HTTP session */
Serial.println(temp);
wdt_reset();

/* Serial1.println("AT+HTTPPARA=\"URL\",\"http://www.google.co.in\"");  /* Set parameters for HTTP session */
 // Serial1.println("AT+HTTPPARA=\"URL\",\"http://www.google.com\"");  /* Set parameters for HTTP session */
/*  delay(2000);
  ShowSerialData();*/

SEND_SIM900A_COMMAND("AT+HTTPPARA=\"URL\",\"www.google.com\"","AT+HTTPPARA=\"URL\",\"www.google.com\"\r\n\r\nOK\r\n"); /* Set URL parameters for HTTP session */
Serial.println(temp);
wdt_reset();

  
/*  Serial1.println("AT+HTTPACTION=0");  /* Start Get session */
/*  delay(8000);
//  ShowSerialData();*/


SEND_SIM900A_COMMAND("AT+HTTPACTION=0","AT+HTTPACTION=0\r\n\r\nOK\r\n\r\n+HTTPACTION:"); /* Start Get session */
Serial.println(temp);
wdt_reset();


/*//SIM900A_RESPONSE();
//Serial.println(temp);
p=temp.indexOf("+HTTPACTION:\r\n\r\n0,200");
p+=14;
temp1="";
while(temp[p]!=',' && p<=50)
{
temp1+=temp[p]; 
++p; 
}

Serial.println(temp1);
*/



SEND_SIM900A_COMMAND("AT+HTTPTERM","AT+HTTPTERM\r\n\r\nOK\r\n"); /* Terminate HTTP service */
Serial.println(temp);

lcd.setCursor(0,1);
if(temp.indexOf("200")!=-1)
{
lcd.print("OK");
} 
else if(temp.indexOf("601")!=-1)
{
lcd.print("Network Error");  
}
else if(temp.indexOf("603")!=-1)
{
 lcd.print("DNS Error");     
}
else
{
//  lcd.print("Error");  
}






SEND_SIM900A_COMMAND("AT+SAPBR=0,1","AT+SAPBR=0,1\r\n\r\nOK\r\n"); // Close bearer
Serial.println(temp);
wdt_reset();

}


void SIM_STATUS(void)
{
 temp="GSM INIT..."; 
 c=0;
 Serial.println();
while(temp[c]!='\0')
{
lcd.print(temp[c]);  
Serial.print(temp[c]);
delay(100);
++c;
}

lcd.setCursor(0,1);
lcd.print("Pls wait : ");
Serial.println();
Serial.println("Pls wait : ");


lcd.setCursor(11,1);
lcd.print(countdown[0]); 
for(c=1;c<=10;c++)
{
wdt_reset();  
delay(1000); 
lcd.setCursor(11,1);
lcd.print(countdown[c]); 
lcd.setCursor(9,1);

if(c%2==0)
{
lcd.print(":");
}
else
{
lcd.print(" "); 
}
}

temp="";
//Serial1.println((char)(26));

ShowSerialData();


Serial1.println("AT"); /* Check Communication */
delay(1000);
SIM900A_RESPONSE();
Serial.println(temp);
wdt_reset();
if(temp.indexOf("OK")== -1)
{
  lcd.clear();
  lcd.print("SIM900 Not Ready");
Serial.print("SIM900 Not Ready");
temp="";
while(1);
}

//temp=""; 


//asm volatile ("  jmp 0"); 
lcd.clear();
lcd.print("Checking SIM"); 
Serial.println("Checking SIM");
for(c=0;c<=4;c++)
{
lcd.setCursor(12,0);
for(c1=0;c1<=2;c1++)
{
lcd.print(".");
Serial.print(".");
delay(100);
}
lcd.setCursor(12,0);
lcd.print("   ");
Serial.print("   ");
if(c<=3)
{
delay(500); 
wdt_reset();

}
}
lcd.setCursor(0,1);

//SIM900A_RESPONSE();
//Serial1.println("AT+CSMINS?"); /* Check SIM Card inserted or not */
//delay(1000);


SEND_SIM900A_COMMAND("AT+CSMINS?","AT+CSMINS?\r\n\r\n+CSMINS: 0,1\r\n\r\nOK\r\n");

//ShowSerialData(); /* Print response on the serial monitor */ 
//SIM900A_RESPONSE();
//Serial.println(temp);
p=temp.indexOf("+CSMINS:");
p+=11;

if(temp[p]=='1')
{
  
lcd.print("SIM CARD OK");  
Serial.println("SIM CARD OK");
SIM_inserted=1;
delay(1000);
wdt_reset();

lcd.clear();
lcd.print("Network Reg...");
Serial.print("Network Reg...");

while(network_registration!=1)
{
wdt_reset();

SEND_SIM900A_COMMAND("AT+CREG?","\r\n\r\nOK\r\n");

p=temp.indexOf("+CREG:");
p+=9;
if(temp[p]=='1')
{
pqr="H";  
network_registration=1;
lcd.setCursor(0,1);
lcd.print("OK           ");  
Serial.print(" OK");
}

if(temp[p]=='5')
{
pqr="R";  
network_registration=1;
lcd.setCursor(0,1);
lcd.print("OK          ");  
}

if(temp[p]=='2')
{
lcd.setCursor(0,1);
lcd.print("Searching...");  
}

if(temp[p]=='0')
{
lcd.setCursor(0,1);
lcd.print("No          ");  
}

if(temp[p]=='3')
{
lcd.setCursor(0,1);
lcd.print("Reg Denied   ");  
}

if(temp[p]=='4')
{
lcd.setCursor(0,1);
lcd.print("Unknown      ");  
}

delay(1000);
wdt_reset();
}

SEND_SIM900A_COMMAND("AT+COPS?","OK\r\n");

/*Serial1.println("AT+COPS?"); /* Check Communication */
/*delay(1000);
SIM900A_RESPONSE();*/

p=temp.indexOf("+COPS:");
p+=12;
while(temp[p]!='"' && p<=50)
{
temp1+=temp[p]; 
++p; 
}

lcd.clear();
lcd.print("Network Operator");
Serial.println();
Serial.print("Network Operator");

lcd.setCursor(0,1);
lcd.print(temp1);
Serial.print(" : ");  
Serial.print(temp1);  


lcd.setCursor(15,1);
lcd.print(pqr);
Serial.print(" - ");  
Serial.print(pqr);  
  
pqr="";
wdt_reset();
delay(2000);

}
Serial.println();

if(temp[p]=='0')
{
Serial.println();
Serial.print("SIM CARD ERROR");  
Serial.println();
Serial.print("Please Insert Sim Card First");  

lcd.print("SIM CARD ERROR");  
wdt_reset();
delay(1000);

while(1);
}






}

void ShowSerialData()
{
 while(Serial1.available()!=0)  /* If data is available on serial port */
 Serial.write(char (Serial1.read())); /* Print character received on to the serial monitor */
}


void SIM900A_RESPONSE(void)
{
while(Serial1.available()!=0)  /* If data is available on serial port */
{
temp+=(char)(Serial1.read());
}

}




void STORE_DATA_LOG(String y)
{
  byte *ptr_w_data_logg_ptr=(byte *)&data_logg_ptr;    //writing value of data_logg_ptr

if(y.length()==115)
 {
    c=16;

if( (data_logg_ptr < 65536) && (data_logg_ptr + 115  < 65536) )
{
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(data_logg_ptr>>8);   // MSB
  Wire.write(data_logg_ptr & 0xFF);   // LSB

 
while(c!=113)
{
 Wire.write(y[c]);
 ++data_logg_ptr;
 BLOCK_CHANGE_1();
 ++c;
 }
 Wire.endTransmission();    
delay(5);
}

else if(data_logg_ptr > 65535)
{
Wire.beginTransmission(EEPROM_I2C_BLOCK1_ADDRESS);
Wire.write((data_logg_ptr - 65536) >>8);   // MSB
Wire.write((data_logg_ptr - 65536) & 0xFF);   // LSB

 
while(c!=113)
{
 Wire.write(y[c]);
 ++data_logg_ptr;
 BLOCK_CHANGE_1();
 ++c;
 }
 Wire.endTransmission();    
delay(5);
} 


else if( (data_logg_ptr < 65536) && (data_logg_ptr + 115  >= 65536) )
{
  int n = 65536 - data_logg_ptr;
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(data_logg_ptr>>8);   // MSB
  Wire.write(data_logg_ptr & 0xFF);   // LSB

 
while(c!=n)
{
 Wire.write(y[c]);
 ++data_logg_ptr;
 BLOCK_CHANGE_1();
 ++c;
 }
 Wire.endTransmission();    
delay(5);

c=0;

n=115-n; 


Wire.beginTransmission(EEPROM_I2C_BLOCK1_ADDRESS);
  Wire.write((data_logg_ptr - 65536) >>8);   // MSB
  Wire.write((data_logg_ptr - 65536) & 0xFF);   // LSB

 
while(c!=n)
{
 Wire.write(y[c]);
 ++data_logg_ptr;
 BLOCK_CHANGE_1();
 ++c;
 }
 Wire.endTransmission();    
delay(5);


}





//Store data logg pointer
if(data_logg_ptr>data_logg_overwrite)
{
data_logg_ptr=21504;  
data_logg_overwrite_indicator=1;
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);                                //write data_logg overwrite
Wire.write((34>>8));  
Wire.write((34 & 0xff));
Wire.write(1);//data logg overwrite
Wire.endTransmission();    
delay(5);
}
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);                                //write data_logg_ptr
Wire.write((39>>8));  
Wire.write((39 & 0xff));
Wire.write(*ptr_w_data_logg_ptr++);
Wire.write(*ptr_w_data_logg_ptr++);
Wire.write(*ptr_w_data_logg_ptr++);
Wire.write(*ptr_w_data_logg_ptr++);
Wire.endTransmission();    

delay(5);

Serial.println("data logg entered");
Serial.println(data_logg_ptr);
 }


}


void STORE_FAULT_LOG(String y)
{
  byte *ptr_w_fault_logg_ptr=(byte *)&fault_logg_ptr;    //writing value of fault_logg_ptr
  
  Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
  Wire.write(fault_logg_ptr >>8);   // MSB
  Wire.write(fault_logg_ptr& 0xFF);   // LSB

if(y.length()==115)
 {
    c=16;
 }
 else
 {
  c=113;
 }


while(c!=113)
{
  Wire.write(y[c]);
 ++fault_logg_ptr;
 BLOCK_CHANGE_2();
 ++c;
 }
 Wire.endTransmission();    

 delay(5); 

//Store Fault logg pointer
if(fault_logg_ptr>fault_logg_overwrite)
{
fault_logg_ptr=1024; 
fault_logg_overwrite_indicator=1;
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);                                //write data_logg_ptr
Wire.write((33>>8));  
Wire.write((33 & 0xff));
Wire.write(1);//fault logg overwrite
Wire.endTransmission();    
delay(5);

}
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);                                //write fault_logg_ptr
Wire.write((35>>8));  
Wire.write((35 & 0xff));
Wire.write(*ptr_w_fault_logg_ptr++);
Wire.write(*ptr_w_fault_logg_ptr++);
Wire.write(*ptr_w_fault_logg_ptr++);
Wire.write(*ptr_w_fault_logg_ptr++);

Wire.endTransmission();    
delay(5);
Serial.println("fault logg entered");
Serial.println(fault_logg_ptr);
//Serial.println(y);

}



void BLOCK_CHANGE_1(void)
{

if(data_logg_ptr % 64==0)
{  
Wire.endTransmission();  
delay(5);
if( data_logg_ptr < 65536)
{
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
Wire.write(data_logg_ptr>>8);   // MSB
Wire.write(data_logg_ptr & 0xFF);   // LSB
} 

if( data_logg_ptr >= 65536)
{
Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
Wire.write((data_logg_ptr - 65536) >>8);   // MSB
Wire.write((data_logg_ptr - 65536) & 0xFF);   // LSB
} 

}
  
}


void BLOCK_CHANGE_2(void)
{
 if(fault_logg_ptr % 64==0)
{  
Wire.endTransmission();
delay(5);

Wire.beginTransmission(EEPROM_I2C_BLOCK0_ADDRESS);
Wire.write(fault_logg_ptr>>8);   // MSB
Wire.write(fault_logg_ptr & 0xFF);   // LSB
}
  
}



void SEND_DATA_SERVER()
{

s1 : if(time_cmd==0 && STEP==0 && send_start==1)
{
send_data_cmd=1;
Serial1.println("AT+CGATT=1"); 
send_start=2;
STEP=1;
request_timeout1=1;
}


if(STEP==1 )
{
if((Serialport1_rx.indexOf("AT+CGATT=1\r\n\r\nOK\r\n")!=-1)  ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=2;
}
}




if(time_cmd==0 && STEP==2)
{
send_data_cmd=1;
Serial1.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  /* Connection type GPRS */
STEP=3;
}


if(STEP==3 )
{
if((Serialport1_rx.indexOf("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n\r\nOK\r\n")!=-1)  ||   (Serialport1_rx.indexOf("ERROR")!=-1)  ) 
{
Serial.println(Serialport1_rx);
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=4;
}
}


if(time_cmd==0 && STEP==4)
{
send_data_cmd=1;
Serial1.println(SET_APN);  /* Access Point Name of the service provider */
STEP=5;
}


if(STEP==5 )
{
//Serial.println(Serialport1_rx);

if((Serialport1_rx.indexOf( SET_APN + "\r\n\r\nOK\r\n" )!=-1)  ||   (Serialport1_rx.indexOf("ERROR")!=-1)  ) 
{
Serial.println(Serialport1_rx);
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=6;
}
}


if(time_cmd==0 && STEP==6)
{
send_data_cmd=1;
//Serial1.println();
Serial1.println("AT+SAPBR=1,1"); 
STEP=7;
}


if(STEP==7)
{
if((Serialport1_rx.indexOf("AT+SAPBR=1,1\r\n\r\nOK\r\n")!=-1) || (Serialport1_rx.indexOf("ERROR")!=-1) ) 
{
Serial.println(Serialport1_rx);
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=8;
}

}


if(time_cmd==0 && STEP==8)
{
send_data_cmd=1;
//Serial1.println();
Serial1.println("AT+SAPBR=2,1"); 
STEP=9;
}



if(STEP==9 )
{
if( (Serialport1_rx.indexOf("0.0.0.0\r\n\r\nOK\r\n")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx); 
Serialport1_rx=""; 
Serial1.println("AT+SAPBR=0,1"); 
send_data_cmd=1;
STEP=50;
}
else if (Serialport1_rx.indexOf("OK\r\n")!=-1)
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=10;  
}

}



if(STEP==50)
{
if((Serialport1_rx.indexOf("AT+SAPBR=0,1\r\n\r\nOK\r\n")!=-1) || (Serialport1_rx.indexOf("ERROR")!=-1) ) 
{
Serial.println(Serialport1_rx);
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=6;
}

}


if(time_cmd==0 && STEP==10)
{
send_data_cmd=1;
Serial1.println("AT+HTTPINIT"); 
STEP=11;
}



if(STEP==11 )
{
if((Serialport1_rx.indexOf("AT+HTTPINIT\r\n\r\nOK\r\n")!=-1)  ||   (Serialport1_rx.indexOf("ERROR")!=-1))
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=12;
}
}




if(time_cmd==0 && STEP==12)
{
send_data_cmd=1;
Serial1.println("AT+HTTPPARA=\"CID\",1"); 
STEP=13;
}



if(STEP==13 )
{
if( (Serialport1_rx.indexOf("AT+HTTPPARA=\"CID\",1\r\n\r\nOK\r\n")!=-1) || (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
STEP=14;
}

/*
if(Serialport1_rx.indexOf("ERROR")!=-1)
{
time_cmd=0;
STEP=0;
send_start=1;
goto s1;
}*/

}



if(time_cmd==0 && STEP==14)
{
send_data_cmd=1;
Serial1.println("AT+HTTPPARA=\"URL\",\"139.59.95.153/gcdataupload/api/home/description\""); 
//Serial1.println("AT+HTTPPARA=\"URL\",\"182.71.94.67/GCDataUpload/API/home/Description\"");  
STEP=15;
}

///GCDataUpload/API/home/Description\

if(STEP==15 )
{
if( (Serialport1_rx.indexOf("AT+HTTPPARA=\"URL\",\"139.59.95.153/gcdataupload/api/home/description\"\r\n\r\nOK\r\n")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);    
Serialport1_rx="";
send_data_cmd=0;

SEND_SIM900A_TIME();
time_cmd=1;

STEP=16;
}
}



if(time_cmd==0 && STEP==16)
{
send_data_cmd=1;
Serial1.println("AT+HTTPPARA=\"CONTENT\",\"application/json\""); 
STEP=17;
}



if(STEP==17 )
{
if( (Serialport1_rx.indexOf("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n\r\nOK\r\n")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;

SEND_SIM900A_TIME();
time_cmd=1;

STEP=18;
}
}

if(time_cmd==0 && STEP==18)
{
send_data_cmd=1;
s_length = TRANSMIT_BUFFER.length();
dummy_string ="AT+HTTPDATA="  + String(s_length) + ",10000" ;
Serial1.println(dummy_string); /* POST data of size 33 Bytes with maximum latency time of 10seconds for inputting the data*/ 
STEP=19;
}


if(STEP==19)
{
if( (Serialport1_rx.indexOf( dummy_string + "\r\n\r\nDOWNLOAD")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) ) 
{
Serial.println(Serialport1_rx);
Serialport1_rx="";
//send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=20;
}
}



if(time_cmd==0 && STEP==20)
{  
send_data_cmd=1;

Serial1.println(TRANSMIT_BUFFER); 
//Serial.println(TRANSMIT_BUFFER); 
STEP=21;
}



if(STEP==21)
{
//Serial.println(Serialport1_rx); 
if( (Serialport1_rx.indexOf("OK")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
/*
Serial.println("AT+HTTPSSL=1");    
delay(5000);
while(Serial1.available()>0)
{
Serial.print((char)Serial1.read()); 
}*/

STEP=22;
}
}



if(time_cmd==0 && STEP==22)
{
send_data_cmd=1;
//Serial1.println();
Serial1.println("AT+HTTPACTION=1");  /* Start POST session */
STEP=23;
}



if(STEP==23)
{
if( (Serialport1_rx.indexOf("AT+HTTPACTION=1\r\n\r\nOK\r\n\r\n+HTTPACTION:")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=24;
}
}



if(time_cmd==0 && STEP==24)
{
send_data_cmd=1;
//Serial1.println("");
Serial1.println("AT+HTTPTERM");  /* Start POST session */
STEP=25;
}



if(STEP==25)
{
if( (Serialport1_rx.indexOf("AT+HTTPTERM\r\n\r\nOK\r\n")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
temp=  Serialport1_rx;
Serial.println(Serialport1_rx);

send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
if(Serialport1_rx.indexOf("200")==-1 )
{
STEP=26;
}
else if(Serialport1_rx.indexOf("200")!=-1 )
{
  STEP=34;
}

Serialport1_rx="";
}
}




if(time_cmd==0 && STEP==26)
{
send_data_cmd=1;
Serial1.println("AT+CMGF=1");    
STEP=27;
}




if(STEP==27)
{
if( (Serialport1_rx.indexOf("AT+CMGF=1\r\n\r\nOK")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=28;
}
}



if(time_cmd==0 && STEP==28)
{
send_data_cmd=1;
Serial1.println("AT+CMGS=\"+919816686443\"");    
STEP=29;
}



if(STEP==29)
{
if( (Serialport1_rx.indexOf("AT+CMGS=\"+919816686443\"\r\n\r\n>")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=30;
}
}


if(time_cmd==0 && STEP==30)
{
send_data_cmd=1;
Serial1.println("SENDER_ID - " + (String)TRANSMIT_BUFFER[17] + (String)TRANSMIT_BUFFER[18] +  (String)TRANSMIT_BUFFER[19] + (String)TRANSMIT_BUFFER[20] + (String)TRANSMIT_BUFFER[21] + (String)TRANSMIT_BUFFER[22]  + "\n\n" + temp );    
STEP=31;
}



if(STEP==31)
{
if( (Serialport1_rx.indexOf("SENDER_ID - " + (String)TRANSMIT_BUFFER[17] + (String)TRANSMIT_BUFFER[18] +  (String)TRANSMIT_BUFFER[19] + (String)TRANSMIT_BUFFER[20] + (String)TRANSMIT_BUFFER[21] + (String)TRANSMIT_BUFFER[22]  + "\n\n" + temp)!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=32;
}
}



if(time_cmd==0 && STEP==32)
{
send_data_cmd=1;
Serial1.println((char)26);    
STEP=33;
}

/*
+CMGS: 5

OK

   */


if(STEP==33)
{
if( (Serialport1_rx.indexOf("+CMGS:")!=-1) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
//SEND_SIM900A_TIME();
time_cmd=0;
STEP=34;
}
}




if(time_cmd==0 && STEP==34)
{
send_data_cmd=1;
Serial1.println("AT+SAPBR=0,1");  
STEP=35;
}



if(STEP==35)
{
if( (Serialport1_rx.indexOf("AT+SAPBR=0,1\r\n\r\nOK\r\n")!=-1 ) ||   (Serialport1_rx.indexOf("ERROR")!=-1) )
{
Serial.println(Serialport1_rx);  
Serialport1_rx="";
send_data_cmd=0;
SEND_SIM900A_TIME();
time_cmd=1;
send_start=0;
transfer_busy_flag=0;
TRANSMIT_BUFFER="";
STEP=0;
request_timeout1=0;
}
}



}




void _15_min_data()
{
if(_minute != prev_minute)
{
flag=0;
}
else
{
flag=1 ;
}



if(flag==0)
{
adc_add();
++count;
prev_minute = _minute ;

//Send data logg to server in every 15 minutes.
if((_minute==0) || (_minute==15) || (_minute==30) || (_minute==45))
{
ALL_AVG_PARA(); 

if(_minute==0)
{
hourly_data_logg=1;  
}
//Serial.println(_Final.length());
//SEND_DATA_SERVER(_Final);


_15min_flag=1;
Serial.println("15 min entered");
//Serial.print(_minute);
//Serial.print(":");
//Serial.println(_second);

CHECK_15MIN_FAULT();


_15min_flag=0;

count=0;
_SPV_FINAL=_BATTERY_VOLTAGE=_LOAD_VOLTAGE=_ARRAY_CURRENT=_BATTERY_CURRENT=_LOAD_CURRENT=_LOAD_POWER=0;
_v0=_v1=_v2=_i0=_i1=_i2=_pow=0;

}}


}





void SEND_PENDING_STRING(void)
{

if(LOW_BATTERY_VOLTAGE!=""  && transfer_busy_flag==0)
{
  lbv_timer=1;
 transfer_busy_flag=1;
TRANSMIT_BUFFER=LOW_BATTERY_VOLTAGE; 
Serial.println(TRANSMIT_BUFFER);
STORE_FAULT_LOG(LOW_BATTERY_VOLTAGE);
LOW_BATTERY_VOLTAGE="";
}




if(SP_CURRENT!="" && transfer_busy_flag==0)
{
sp_timer=1;   
transfer_busy_flag=1;
TRANSMIT_BUFFER=SP_CURRENT;
Serial.println(TRANSMIT_BUFFER);
STORE_FAULT_LOG(SP_CURRENT);
SP_CURRENT="";
}



if(S_HIGH_BATTERY_CURRENT!=""  && transfer_busy_flag==0)
{
  hbc_timer=1; 
  transfer_busy_flag=1;
TRANSMIT_BUFFER=S_HIGH_BATTERY_CURRENT; 
STORE_FAULT_LOG(S_HIGH_BATTERY_CURRENT);
S_HIGH_BATTERY_CURRENT="";
}

if(S_LOAD_HIGH_CURRENT!=""  && transfer_busy_flag==0)
{ 
  lhc_timer=1;
  transfer_busy_flag=1;
  TRANSMIT_BUFFER=S_LOAD_HIGH_CURRENT; 
STORE_FAULT_LOG(S_LOAD_HIGH_CURRENT);
S_LOAD_HIGH_CURRENT="";
}

if(LOAD_OPEN!=""  && transfer_busy_flag==0)
{
  lo_timer=1;
  transfer_busy_flag=1;
TRANSMIT_BUFFER=LOAD_OPEN;  
STORE_FAULT_LOG(LOAD_OPEN);
LOAD_OPEN="";
}

if(LOAD_SHORTCIRCUIT!=""  && transfer_busy_flag==0)
{ 
  lsc_timer=1;
  transfer_busy_flag=1;
  TRANSMIT_BUFFER=LOAD_SHORTCIRCUIT; 
  Serial.println(TRANSMIT_BUFFER);
STORE_FAULT_LOG(LOAD_SHORTCIRCUIT);
LOAD_SHORTCIRCUIT="";
}

if( _15min_string1!=""  && transfer_busy_flag==0)
{ 
  transfer_busy_flag=1;
TRANSMIT_BUFFER=_15min_string1; 
//Store data logg locally
if(hourly_data_logg==1)
{
STORE_DATA_LOG(_15min_string1); 
}
_15min_string1="";
}

if( _15min_string2!=""  && transfer_busy_flag==0)
{ 
  transfer_busy_flag=1;
TRANSMIT_BUFFER=_15min_string2; 
//Store data logg locally
if(hourly_data_logg==1)
{
STORE_DATA_LOG(_15min_string2); 
}
_15min_string2="";
}

if( _15min_string3!=""  && transfer_busy_flag==0)
{ 
  transfer_busy_flag=1;
TRANSMIT_BUFFER=_15min_string3; 
//Store data logg locally
if(hourly_data_logg==1)
{
STORE_DATA_LOG(_15min_string3); 
}

_15min_string3="";
}

if( _15min_string4!=""  && transfer_busy_flag==0)
{ 
  transfer_busy_flag=1;
TRANSMIT_BUFFER=_15min_string4; 
//Store data logg locally
if(hourly_data_logg==1)
{  
STORE_DATA_LOG(_15min_string4); 
}
_15min_string4="";
}

if( _15min_string5!=""  && transfer_busy_flag==0)
{ 
transfer_busy_flag=1;
TRANSMIT_BUFFER=_15min_string5; 
//Store data logg locally
 if(hourly_data_logg==1)
{
STORE_DATA_LOG(_15min_string5); 
}
_15min_string5="";
}

if( zigbee_buffer!=""  && transfer_busy_flag==0)
{ 
transfer_busy_flag=1;
TRANSMIT_BUFFER=zigbee_buffer; 
zigbee_buffer="";
}



/*
if( _15min_string6!=""  && transfer_busy_flag==0)
{ 
  transfer_busy_flag=1;
TRANSMIT_BUFFER=_15min_string6; 
//Store data logg locally
 
STORE_DATA_LOG(_15min_string6); 
_15min_string6="";
}*/

if(TRANSMIT_BUFFER!="")
{
if(send_start==0)
{
  send_start=1;
}

SEND_DATA_SERVER();  

}



  
}







void setup() {
  // put your setup code here, to run once
Wire.begin();
Serial.begin(9600);
Serial1.begin(9600);
Serial2.begin(9600);

Serial.flush();
Serial1.flush();
Serial2.flush();

for(c=2;c<=7;c++)
{
pinMode(c,OUTPUT);
}


pinMode(SIM900A_ON_OFF,OUTPUT);
digitalWrite(SIM900A_ON_OFF,ON); 


lcd.begin(16,2);//lcd begin 16*2
lcd.setCursor(0,0);//set cursor to 1st row and 1st column
analogWrite(46,60);//set LCD contrast.

wdt_enable(WDTO_8S);
SIM_STATUS();
lcd.clear();

SYNC_DATE_TIME();
displayTime_lcd();
delay(2000);  
wdt_reset();

if(SIM_inserted==1)
{
CHECK_INTERNET();
}

ReadTime();
_1st_time_pointer_set();
Read_pointer();
Read_comparable_parameters();

//SEND_SMS("");

Timer1.initialize(10000);  //10 ms interrupt
Timer1.attachInterrupt(_10ms_interrupt);
lcd.clear();


//Timer3.initialize(1000000);  //10 ms interrupt
//Timer3.attachInterrupt(abc);
}



void loop() {
// put your main code here, to run repeatedly: 
//Serial.println(Final);
//Serial.println(Final.length());
//READ_SERIAL_PORT();

KEYPAD_OPERATION();
READ_SERIAL_PORT();
READ_SERIAL_PORT1();
READ_ZIGBEE_DATA();
CHECK_FAULT_DATA_LOGG_READ_CMD();
RECEIVE_SIM900A_TIME();



if(_10ms==1)
{
_10ms=0;

if(k<=9)
{
SPV_POSITIVE_READ();
SPV_NEGATIVE_READ();  
ARRAY_CURRENT_READ();
BATTERY_VOLTAGE_READ();
BATTERY_CURRENT_READ();
LOAD_VOLTAGE_READ();
LOAD_CURRENT_READ();
GC_TEMPERATURE_READ();
AMB_TEMPERATURE_READ(); 
}
CHECK_SHORTCIRCUIT_FAULT();
AVG_ALL_PARAMETERS();
all_timeout_check();
//GET_Network_TIME2();
}


if(p_init==1)
{
CHECK_ALL_FAULT();
p_init=0;
}


if(_1sec>=100)
{
_1sec=0;
//ReadTime();
if(STEP==0)
{
SEND_SIM900A_TIME();
}
CHECK_FAULT_TIME();
_DISPLAY();
_15_min_data(); 
Reset_hourly_data_flag();
Timeout();
wdt_reset();

}

SEND_PENDING_STRING();
}


void _10ms_interrupt()
{
_10ms=1;
++_1sec;

if(request_timeout>0)
{
  ++request_timeout;
if(request_timeout>=1001)
{

time_cmd=0;
//send_data_cmd=0;
request_timeout=0;
SEND_SIM900A_TIME();
time_cmd=1;


//Serial1.println((char)(26));
Serial.println(STEP);

Serial.println("Read Time timeout");

++retry_time_read;
if(retry_time_read>=3)
{
digitalWrite(SIM900A_ON_OFF,OFF); 
while(1);   
}


}

}

if(request_timeout1>0)
{
  ++request_timeout1;
if(request_timeout1>=30001)
{
 /* 
time_cmd=0;
send_data_cmd=0;
send_start=1;
STEP=0;
request_timeout1=0;*/
Serial.println("Send Data timeout");


digitalWrite(SIM900A_ON_OFF,OFF); 
while(1); 
}

}


}

