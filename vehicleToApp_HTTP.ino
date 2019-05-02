/*
**********
This code sends HTTP packet at regular intervals (every 30sec).
If interrupts at pin 2 or pin 3 are triggered, then HTTP message is sent irrespective of the time interval.

Buttons 11 and 4 do not use interrupts
Buttons 2 and 3 use interrupts
Potentiometer at A0 changes the battery level
eg: if Potentiometer input is 645 then bettery level is set to 64, Last digit is truncated

Button at 2 ----> toggles Power (TRUE or FALSE)
Button at 3 ----> toggles Trunk (TRUE or FALSE)
Button at 4 ----> toggles Stauts (LOCK or UNLOCK)
Button at 11 ----> toggles MODE (SPORT or ECO)


Interrupt pins 2 and 3
pin 2 ----> interrupt 0
pin 3 ----> interrupt 1
**********
*/

/****
 * Implement 30sec HTTP send as an interrupt (software interrupt or timer interrupt)
****/


#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LDateTime.h>
datetimeInfo t;
unsigned int rtc;

// Constants won't change. They're used here to set pin numbers:
const int buttonPin1 = 11;    // pin number of the pushbutton pin
const int buttonPin2 = 4;    // pin number of the pushbutton pin
int ledState1 = HIGH;       // Initial state of pin 11
int ledState2 = HIGH;       // Initial state of pin 4
int sensorPin = A0;      // Connected to potentiometer

int sensorValue = 0;
int lastSensorValue = 0;
// Variables will change:
int buttonState1;             // the current reading from the input pin
int lastbuttonState1 = LOW;   // the previous reading from the input pin
int buttonState2;             // the current reading from the input pin
int lastbuttonState2 = LOW;   // the previous reading from the input pin
int flag_power = 0;
int flag_trunk = 0;

// For buttons at pin 11 and 4
// The following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay1 = 10;    // the debounce time; increase if the output flickers
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay2 = 10;    // the debounce time; increase if the output flickers

bool intpower = false;     // Initaial states of pin 2 and 3 which represent POWER and TRUNK
bool inttrunk = false;

// WiFi Credentials
#define WIFI_AP "GeekSynergy_HQ"
#define WIFI_PASSWORD "p!a@s#s$w%o^r&d*"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP.

// Web address of server
#define SITE_URL "13.233.172.227"

// Build JSON method

// Initial values of JSON object
String vehicleId = "\"5c61523b3d5bb35aecc108bc\"";    //Suvam's vehicle ID
String mode = "\"sport\"";
String battery = "55";
String stat = "\"unlock\"" ;
String trunk = "false" ;
String power = "false" ;
int batInt = 56;

String buildJson() {                  // Function which biulds a JSON object (a String in JSON format) and returns it
  String data = "{";
  data+="\n";
  data+="\"vehicleId\":";
  data+=vehicleId;
  data+= ",";
  data+="\n";
  data+="\"mode\":";
  data+=mode;
  data+= ",";
  data+="\n";
  data+="\"battery\":";
  data+=battery;
  data+= ",";
  data+="\n";
  data+="\"trunk\":";
  data+=trunk;
  data+= ",";
  data+="\n";
  data+="\"power\":";
  data+=power;
  data+= ",";
  data+="\n";
  data+="\"status\":";
  data+=stat;
  data+= ",";
  data+="\n";
  data+="\"diagnostics\":";
  data+="false";
  data+= ",";
  data+="\n";
  data+="\"duration\":";
  data+="6";
  data+= ",";
  data+="\n";
  data+="\"distance\":";
  data+="98";
  data+="\n";
  data+="}";
  return data;
}

LWiFiClient c;     //WiFi TCP Client

void setup()
{
  LWiFi.begin();
  Serial.begin(115200);                  // Begin serial communication at 115200 baud rate
  delay(10000);                          // Wait for 10sec in the beginning
  Serial.println("DelayDone");

  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  
  // Keep retrying until connected to AP
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP, LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    delay(1000);
  }
  // Attach interrupts
  attachInterrupt(0, intrrPower, RISING);           // Interrupt triggered when signal goes from 0 to 1
  attachInterrupt(1, intrrTrunk, RISING);           // Interrupt triggered when signal goes from 0 to 1
  
}

void loop()
{
  int reading1 = digitalRead(buttonPin1);  //Read state of the button
  int reading2 = digitalRead(buttonPin2);
  sensorValue = analogRead(sensorPin);     //Read value of the analog pin
  
  if((sensorValue>lastSensorValue+9)||(sensorValue<lastSensorValue-9))   // Print updated values from potentiometer only if there is a minimum of +10 or -10 from the previous value
  {
    Serial.print("Analog input: ");
    Serial.println(sensorValue); 
  }

  // Check if button1 is pressed and also for debounce
  if (reading1 != lastbuttonState1) {
    // reset the debouncing timer
    lastDebounceTime1 = millis();            // Only if the state changed from its previous state
  }
  if ((millis() - lastDebounceTime1) > debounceDelay1) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading1 != buttonState1) {
      buttonState1 = reading1;

      // only toggle the LED if the new button state is HIGH
      if (buttonState1 == HIGH) {
        ledState1 = !ledState1;
        if(ledState1)              // Toggle State
        {
          mode = "\"eco\"";
        }
        else
        {
          mode = "\"sport\"";
        }
        Serial.println("Button 1 pressed, Toggle MODE!!!!!!!!");
        Serial.println(mode);
        
      }
    }
  }

  // Check if button2 is pressed and also for debounce
  if (reading2 != lastbuttonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();            // Only if the state changed from its previous state
  }
  if ((millis() - lastDebounceTime2) > debounceDelay2) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading2 != buttonState2) {
      buttonState2 = reading2;

      // only toggle the LED if the new button state is HIGH
      if (buttonState2 == HIGH) {
        ledState2 = !ledState2;
        if(ledState2)              // Toggle State
        {
          stat = "\"unlock\"";
        }
        else
        {
          stat = "\"lock\"";
        }
        Serial.println("Button 2 pressed, Toggle status (LOCK or UNLOCK)*******");
        Serial.println(stat);
      }
    }
  }
  
  // Save the reading. Next time through the loop, it'll be the lastbuttonState1 :
  lastbuttonState1 = reading1;
  lastbuttonState2 = reading2;
  lastSensorValue = sensorValue;

  int toggle = 1;
  if(flag_trunk)      // If Flag has been set by interrupt, then send HTTP message
  {
            noInterrupts();        // Disable Interrupts
            flag_trunk = 0;
            inttrunk = !inttrunk;
            if(inttrunk)       // Toggle State
            {
              trunk = "true";
            }
            else
            {
              trunk = "false";
            }
            Serial.print("TRUNK : ");
            Serial.print(trunk);
            Serial.println("\nConnecting to WebSite");
            sendHttp();
            interrupts();        // Enable Interrupts
  }
  if(flag_power)      // If Flag has been set by interrupt, then send HTTP message
  {
            noInterrupts();        // Disable Interrupts
            flag_power = 0;
            intpower = !intpower;
            if(intpower)        // Toggle State
            {
              power = "true";
            }
            else
            {
              power = "false";
            }
            Serial.print("POWER : ");
            Serial.print(power);
            Serial.println("\nConnecting to WebSite");
            sendHttp();
            interrupts();          // Enable Interrupts
  }
  LDateTime.getTime(&t);  
  LDateTime.getRtc(&rtc);      // Check real time clock, returns in seconds elapsed
  noInterrupts();        // Disable Interrupts
  if(rtc % 30 == 0)      // Every 30sec, send HTTP message
  {
        sendHttp();    
  }
  interrupts();          // Enable Interrupts
  

        batInt = sensorValue/10;    //If potentiometer input is 645 then bettery level is set to 64, Last digit is truncated
        battery = String(batInt);   //Changing int to String
        delay(10); 
        delay(50);
}


void intrrPower()                // ISR for interrupt 0 at pin 2
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 200) 
 {
        flag_power = 1;
 }
 last_interrupt_time = interrupt_time;

}


void intrrTrunk()                // ISR for interrupt 1 at pin 3
{
 static unsigned long last_interrupt_time1 = 0;
 unsigned long interrupt_time1 = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time1 - last_interrupt_time1 > 1200) 
 {
        flag_trunk = 1;  
 }
 last_interrupt_time1 = interrupt_time1;
 
}


void sendHttp()                 // Sends HTTP packet with a JSON object(payload), waits for response and prints it on serial monitor
{
//        Serial.println("Every 30 Seconds....SENDING");
//        while (0 == c.connect(SITE_URL,3000))     // port 80 for testing websites
//        {
//          Serial.println("Re-Connecting to WebSite");
//          delay(1000);
//        }
//        
//        String PostData = buildJson();
//        Serial.println("send HTTP POST request");
//        //c.println("POST 13.233.172.227:3000/users/vehicle/updateLivedata HTTP/1.1");
//        c.println("POST /users/vehicle/updateLivedata HTTP/1.1");//modify the POST path
//        //c.println("POST /test HTTP/1.1");//modify the POST path
//        c.println("Host: " SITE_URL);
//        c.println("Content-Type: application/json");
//        //c.println("Content-Type: application/x-www-form-urlencoded");
//        c.println("Connection: close");                   //Try keep-alive
//        c.print("Content-Length: ");
//        c.println(PostData.length());
//        c.println();
//        c.println(PostData);  //send 
//        c.println();
//        
//        Serial.println("waiting HTTP response:");
//        while (!c.available())
//        {
//          delay(100);
//        }
//        // Make sure we are connected, and dump the response content to Serial
//        while (c)
//        {
//          int v = c.read();
//          if (v != -1)
//          {
//            Serial.print((char)v);
//          }
//          else
//          {
//            Serial.println("no more content, disconnect");
//            c.stop();
//          }
//        }
//        
//        delay(500);
}
