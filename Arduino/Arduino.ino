#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include<SoftwareSerial.h>
#include<ArduinoJson.h>

#define ONE_WIRE_BUS A3

///////////////
///////////////
SoftwareSerial nodemcu(5,6); /////Rx,Tx/////////////
//////////////
//////////////

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

LiquidCrystal_I2C lcd(0x27, 20, 4); //The LCD address and size. You can change according you yours

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

int R1= 1000; // Value of resistor for EC probe
int EC_Read = A0;
int ECPower = A1;
float Temp_C; // Do not change
float Temp_F; // Do not change
float Temp1_Value = 0;
float Temp_Coef = 0.019; // You can leave as it is
/////////////////This part needs your attention during calibration only///////////////
float Calibration_PPM =287 ; //Change to PPM reading measured with a separate meter
float K=5.50; //You must change this constant once for your probe(see video)
float PPM_Con=0.5; //You must change this only if your meter uses a different factor
/////////////////////////////////////////////////////////////////////////////////////
float CalibrationEC= (Calibration_PPM*2)/1000;
//////////////////
      ///////
float Temperature;
      ///////
//////////////////
float EC;
//////////////////
   ////////////
float EC_at_25;
  /////////////
    //////////
int ppm;
  /////////////////
float A_to_D= 0;
float Vin= 5;
float Vdrop= 0;
float R_Water;
float Value=0;
///////////////////////////////////////////////////////////////////////////////


/////PUMP DECLARATION EC///////
const byte Pump_A=8;
const byte Pump_B=9;
///////////////////////////////

int Set_ppm = 100; // change to your desired ppm

/////////////////////////////////////////////MAIN FROM HERE///////////////////////////////////
void setup()
{  
  
Serial.begin(9600);

////////////////////////////////////////
nodemcu.begin(9600);
delay(1000);
Serial.println("program started");
////////////////////////////////////////




  // Start up the library
sensors.begin();//temp sensor
pinMode(EC_Read,INPUT);
pinMode(ECPower,OUTPUT);
///////////////////////////////////////////////////////////////////////////////
pinMode(Pump_A,OUTPUT);
pinMode(Pump_B,OUTPUT);

/*    EC       */
////////////////PUMP A//////////////////
  pinMode(Pump_A,OUTPUT);                       //////////8 PIN/////////////
  digitalWrite(Pump_A,HIGH);
////////////////PUMP 2/////////////////
  pinMode(Pump_B,OUTPUT);                       //////////9 PIN////////////
  digitalWrite(Pump_B,HIGH);
  //////////////////////////////////////


/////////////////////////////////////////////////////////////
//Calibrate (); // After calibration put two forward slashes before this line of code
//////////////////////////////////////////////////////////////////////////////////////////
 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
//  lcd.setCursor( 5, 0);
//  lcd.print("....STARTING....");

  lcd.clear();
}





//////////////////////////////LOOP START FROM HERE/////////////////////////////////////////////////


void loop()
{

digitalWrite(Pump_A,HIGH);
digitalWrite(Pump_B,HIGH); 
// Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
sensors.requestTemperatures(); 
////////////////////////////////////////////////////////
GetEC(); //Calls GetEC()
delay(6000); //Do not make this less than 6 sec (6000)
if(ppm<=Set_ppm)
{
Pump_EC();
}
//////////////serial commmunication//////////////////////////////////
StaticJsonDocument<1000> doc;


//Assign collected data to JSON object
doc["temperature"]=Temperature;
doc["EC"]=EC_at_25;
doc["TDS"]=ppm;


//send data to nodemcu
serializeJson(doc,nodemcu);
delay(500);
//




}
//////////////////////////////////////LOOP END FROM HERE//////////////////////////////////////////////




void GetEC()
{
Temp_C = sensors.getTempCByIndex(0); // Celsius
Temp_F = sensors.getTempFByIndex(0); // Celsius to Fahrenheit
Temp1_Value = Temp_C;
Temperature = Temp_C;
digitalWrite(ECPower,HIGH);
A_to_D= analogRead(EC_Read);
A_to_D= analogRead(EC_Read);
digitalWrite(ECPower,LOW);
Vdrop= (Vin*A_to_D) / 1024.0;
R_Water = (Vdrop*R1) / (Vin-Vdrop);
EC = 1000/ (R_Water*K);
EC_at_25 = EC / (1+ Temp_Coef*(Temperature-25.0));
ppm=(EC_at_25)*(PPM_Con*1000);
Serial.print(" EC: ");
Serial.print(EC_at_25);
Serial.print("(mS/cm) ");
Serial.print(ppm);
Serial.print(" ppm ");
Serial.print(sensors.getTempCByIndex(0));
Serial.print(" *C | ");
Serial.print(sensors.getTempFByIndex(0));
Serial.println(" *F ");

//  ////PRINT STATEMENTS OF EC////
// lcd.setCursor(0, 0);
//  lcd.print("T:");
// lcd.print(sensors.getTempCByIndex(0));
// lcd.print("*C|");
// lcd.print(sensors.getTempFByIndex(0));
// lcd.println("*F");
// ////////////////////
// lcd.setCursor(0, 1);
// lcd.print("EC:");
// lcd.print(EC_at_25);
// lcd.print("|");
// lcd.print("TDS:");
// lcd.print(ppm);
// lcd.print(" ppm ");
}


///////////////////CALIBRATION//////////////////////////////////




void Calibrate ()
{
Serial.println("Calibration routine started");
float Temperature_end=0;
float Temperature_begin=0;
Temp_C = sensors.getTempCByIndex(0);
Temp_F = sensors.getTempFByIndex(0); // Celsius to Fahrenheit
Temp1_Value = Temp_C;
Temperature_begin=Temp_C;
Value = 0;
int i=1;
while(i<=10){
digitalWrite(ECPower,HIGH);
A_to_D= analogRead(EC_Read);
A_to_D= analogRead(EC_Read);
digitalWrite(ECPower,LOW);
Value=Value+A_to_D;
i++;
delay(6000);
};
A_to_D=(Value/10);
Temp_C = sensors.getTempCByIndex(0); // Kelvin to Celsius
Temp_F = sensors.getTempFByIndex(0); // Celsius to Fahrenheit
Temp1_Value = Temp_C;
Temperature_end=Temp_C;
EC =CalibrationEC*(1+(Temp_Coef*(Temperature_end-25.0)));
Vdrop= (((Vin)*(A_to_D))/1024.0);
R_Water=(Vdrop*R1)/(Vin-Vdrop);
float K_cal= 1000/(R_Water*EC);
Serial.print("Replace K in line 23 of code with K = ");
Serial.println(K_cal);
Serial.print("Temperature difference start to end were = ");
Temp_C=Temperature_end-Temperature_begin;
Serial.print(Temp_C);
Serial.println("*C");
Serial.println("Temperature difference start to end must be smaller than 0.15*C");
Serial.println("");
}
///////////////////////////////CALIBRATION//////////////////////////////////


void Pump_EC()
{

int Change_per_dose = 100; // determine this experimentally (see video for clarity)
//int Pump_on_time = 5; // set pump dose time in seconds
//int Wait_time = 4; // set time to wait between dosing
//if (ppm <=(Set_ppm-Change_per_dose))
//{
digitalWrite(Pump_A,HIGH);
digitalWrite(Pump_B,HIGH);
//delay(Pump_on_time*1000);
digitalWrite(Pump_A,LOW);
digitalWrite(Pump_B,LOW);
//delay(Wait_time*1000);
//}
}
