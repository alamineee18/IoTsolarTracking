#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6Zoa-hzPb"
#define BLYNK_TEMPLATE_NAME "IoT Solar"
#define BLYNK_AUTH_TOKEN "XDXdhK280We2laD3DGaJv7_ukC4aIAsg"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WifiName";
char pass[] = "eeeVU2023";


//INA219 
#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina219;
 
//Servo Control
#include <ESP32Servo.h> //includes the servo library
Servo myservo;
#define ldr1 12 // set ldr 1 Analog input pin of East ldr as an integer
#define ldr2 14 // set ldr 2 Analog input pin of West ldr as an integer
int pos = 90; // initial position of the Horizontal movement controlling servo motor
int tolerance = 20; // allowable tolerance setting - so solar servo motor isn't constantly in motion
 

//Load Control
#define Load 4
BLYNK_WRITE(V0)
{
  if(param.asInt()==1){
    digitalWrite(Load, HIGH);
  }
  else{
    digitalWrite(Load, LOW);
  }
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);  
}

void setup()
{

  //Servo Traking
  myservo.attach(18); // attaches the servo on digital pin 2 to the horizontal movement servo motor 
  pinMode(ldr1, INPUT); //set East ldr pin as an input
  pinMode(ldr2, INPUT); //set West ldr pin as an input
  myservo.write(pos); // write the starting position of the horizontal movement servo motor 
      
  delay(1000); // 1 second delay to allow the solar panel to move to its staring position before comencing solar tracking
  
  //Debug console
  Serial.begin(9600);
  pinMode(Load, OUTPUT); // Initialise digital pin as an output pin
 
  delay(100);

  //INA219 Stary
  while (!Serial) 
  {
    delay(1);
  }
 
  // Initialize the INA219.
  if (! ina219.begin())
  {
    Serial.println("Failed to find INA219 chip");
    while (1) 
    {
      delay(10);
    }
  } 
  Serial.println("Measuring voltage, current, and power with INA219 ...");

  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
   
  //Servo Control
  int val1 = analogRead(ldr1); // read the value of ldr 1
  int val2 = analogRead(ldr2); // read the value of ldr 2
      
  if((abs(val1 - val2) <= tolerance) || (abs(val2 - val1) <= tolerance)) {
    //no servo motor horizontal movement will take place if the ldr value is within the allowable tolerance
  }
  else {
    if(val1 > val2){ 
      // if ldr1 senses more light than ldr2 
      pos = pos + 1; 
      // decrement the 90 degree poistion of the horizontal servo motor - this will move the panel position Eastward
    }
    if(val1 < val2){ 
      // if ldr2 senses more light than ldr1
      pos = pos - 1; 
      // increment the 90 degree position of the horizontal motor - this will move the panel position Westward
    }
  }
     
  if(pos > 170) {
    pos = 170;
  } // reset the horizontal postion of the motor to 180 if it tries to move past this point
  if(pos < 105) {
    pos = 105;
  } // reset the horizontal position of the motor to 0 if it tries to move past this point
  
  myservo.write(pos); // write the starting position to the horizontal motor
  
  delay(50);

  // ina219

  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;
 
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  float power_W = power_mW / 1000; 

  Serial.print("Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Current:   "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:     "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");
  Blynk.virtualWrite(V1, current_mA);
  Blynk.virtualWrite(V2, busvoltage);
  Blynk.virtualWrite(V3, power_W);


  Blynk.run();
}
