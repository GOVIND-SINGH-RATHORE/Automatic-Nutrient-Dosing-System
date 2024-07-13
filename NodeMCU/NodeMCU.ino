// ArduinoJson - Version: Latest
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "thingProperties.h"
#include <SoftwareSerial.h>      
LiquidCrystal_I2C lcd(0x27,20,4);
// if lcd is not print then use this 0x27.. 0x3F



// SoftwareSerial for esp8266 - Version: Latest


/*
  Sketch generated by the Arduino IoT Cloud Thing 
  https://create.arduino.cc/cloud/things/1a637fda-bf94-4d3f-a6bc-defda2015772

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  CloudElectricCurrent eC;
  float pH;
  int tDS;
  CloudTemperature temperature;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/




SoftwareSerial nodemcu(D6,D5);  //Rx,tX PINS



////////////////////////PH CODE START HERE////////////////
#define SensorPin A0           //pH meter Analog output to Arduino Analog Input 0
#define Offset 41.555         //deviation compensate
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  10    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex = 0;
////////////////////////PH CODE END HERE///////////////////

/////PUMP DECLARATION PH//////
const byte Pump_UP=D3;     ////D0    ///// 10 pin
const byte Pump_DOWN=D4;   ////D1    ///// 11 pin
//////////////////////////////

static float pHValue=0,voltage=0;
float Set_ph= 6; // change to your desired ppm//set default 6 to 7

void setup() {
  // Initialize serial and wait for port to open:
// Open serial communications and wait for port to open:  
Serial.begin(9600);
nodemcu.begin(9600);
while (!Serial) continue;

  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1000);
  //////////////////////////////////////////////////////////////
 
  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
 
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  /*    PH       */
////////////////PUMP UP//////////////////
  pinMode(Pump_UP,OUTPUT);                      //////////10 PIN////////////
  digitalWrite(Pump_UP,HIGH);
////////////////PUMP DOWN//////////////////
  pinMode(Pump_DOWN,OUTPUT);                    //////////11 PIN///////////
  digitalWrite(Pump_DOWN,HIGH);

   lcd.init();      
   lcd.backlight();
   lcd.clear();

}

void loop() {

 digitalWrite(Pump_UP,HIGH);
 digitalWrite(Pump_DOWN,HIGH);
//////////////////////////PH///////////////////////////
Ph_sensor();
///////////////////////////////////////////////////////
if(pHValue<=Set_ph)
{
Pump_PH_UP();
Serial.print("pH UP");
}
else if(pHValue>=Set_ph+1)
     {
   Pump_PH_DOWN();
Serial.println("pH DOWN");
         }
//////////////////////////////////////////////////////
ArduinoCloud.update();

StaticJsonDocument<1000> doc;



DeserializationError error = deserializeJson(doc,nodemcu);

//Test parsing
if(error)
{
  Serial.println("CONNECTED");
  return;  
  }


/////////////////////object received from arduino///////////////
Serial.println("Data Recieved");
Serial.println("Received temperature:");
float Temperature = doc["temperature"];
Serial.print("Temperature:");
Serial.println(Temperature,2);
float EC_at_25 = doc["EC"];
Serial.print("EC:");
Serial.println(EC_at_25,2);
float ppm = doc["TDS"];
Serial.print("TDS:");
Serial.println(ppm,2);
  if(pHValue<=10 && pHValue>=0)
    {
Serial.print("Voltage:");
Serial.println(voltage, 2);
Serial.print("pH:");
Serial.println(pHValue,2);
pH=pHValue;
    }
Serial.println("_ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _");
////////////////
eC=EC_at_25;
tDS=ppm;
temperature=Temperature;


  ////PRINT STATEMENTS OF EC////
 lcd.setCursor(0, 0);
  lcd.print("T:");
 lcd.print(temperature);
 lcd.print("*C");
// lcd.print(sensors.getTempFByIndex(0));
 //lcd.println("*F");
 ////////////////////
 lcd.setCursor(0, 1);
 lcd.print("EC:");
 lcd.print(eC);
 lcd.print("|");
 lcd.print("TDS:");
 lcd.print(tDS);
 lcd.print(" ppm ");
 ///////////////////
   if(pHValue<=10 && pHValue>=0)
   {
 lcd.setCursor(0,2);
 lcd.print("V:");
 lcd.print(voltage);
 lcd.print("|pH:");
 lcd.print(pHValue);
   }

}


void Ph_sensor()
{
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
        /*pHValue*/
  if (millis() - samplingTime > samplingInterval)
  {
    pHArray[pHArrayIndex++] = analogRead(SensorPin);
    if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
    voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
    pHValue = -11.77272727 * voltage + Offset;
    samplingTime = millis();
  }
  if (millis() - printTime > printInterval)  //Every 800 milliseconds, print a numerical, convert the state of the LED indicator
 {


    //Serial.print("Voltage:");
   // Serial.print(voltage, 2);
   // Serial.print("    pH value: ");
   // Serial.println(pHValue, 2);
    printTime = millis();
 }
//    if(pHValue<10)
//      {
  //     pH=pHValue;
      //print phValue
// lcd.setCursor(0,2);
// lcd.print("voltage:");
// lcd.print(voltage);
// lcd.print("pH:");
// lcd.println(pHValue);
//
//      }
//    else
//    {
//      Serial.print("calculating ph");
//      lcd.print("calculating pH");
//      
//      }  
    //}
  }
double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if (number <= 0) {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}

////////////////////////////////////////////////////////////

void Pump_PH_UP()
{

int Change_per_dose = 100; // determine this experimentally (see video for clarity)
int Pump_on_time = 5; // set pump dose time in seconds
int Wait_time = 4; // set time to wait between dosing
//if (Set_ph<=(Set_ph-Change_per_dose))
//{
digitalWrite(Pump_UP,HIGH);
//delay(Pump_on_time*1000);
digitalWrite(Pump_UP,LOW);
//delay(Wait_time*1000);
//}
}

void Pump_PH_DOWN()
{

int Change_per_dose = 100; // determine this experimentally (see video for clarity)
int Pump_on_time = 5; // set pump dose time in seconds
int Wait_time = 4; // set time to wait between dosing
//if (Set_ph<=(Set_ph-Change_per_dose))
//{
digitalWrite(Pump_DOWN,HIGH);
//delay(Pump_on_time*1000);
digitalWrite(Pump_DOWN,LOW);
//delay(Wait_time*1000);
//}
}
