#include <ESP8266WiFi.h>
#include <Wire.h>

#define BLYNK_TEMPLATE_ID "TMPL-zwc53YX"
#define BLYNK_DEVICE_NAME "IoT"
#define BLYNK_AUTH_TOKEN "HLJ-zGVSEdn84tUCEaKOvdqFKybnKr95"

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>
 
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);
 
const int sampleWindow = 50;                              // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;
 
#define SENSOR_PIN A0
#define PIN_QUIET D3
#define PIN_MODERATE D4
#define PIN_LOUD D5
 
String apiKey = "1TRPJHGLPS8528W1"; // Enter your Write API key from ThingSpeak
const char *ssid = "Redmi Note 10SD";     // replace with your wifi ssid and wpa2 key
const char *pass = "12345678";
const char* server = "api.thingspeak.com";
char auth[]=BLYNK_AUTH_TOKEN;
 
WiFiClient client;
BlynkTimer timer;
 
void setup ()  
{   
  Blynk.begin(auth,ssid,pass);
  pinMode (SENSOR_PIN, INPUT); // Set the signal pin as input  
  pinMode(PIN_QUIET, OUTPUT);
  pinMode(PIN_MODERATE, OUTPUT);
  pinMode(PIN_LOUD, OUTPUT); 
 
  digitalWrite(PIN_QUIET, LOW);
  digitalWrite(PIN_MODERATE, LOW);
  digitalWrite(PIN_LOUD, LOW);
  
  Serial.begin(115200);
  lcd.init();
 
  // Turn on the backlight.
  lcd.backlight();
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
  
  lcd.setCursor(0, 0);
  lcd.print("Connecting to...");
 
  lcd.setCursor(0, 1);
  lcd.print(ssid);
 
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("WiFi connected");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected...");
    delay(4000);
    lcd.clear();
}  
 
   
void loop ()  
{ 
  Blynk.run();
  timer.run();
   unsigned long startMillis= millis();                   // Start of sample window
   float peakToPeak = 0;                                  // peak-to-peak level
 
   unsigned int signalMax = 0;                            //minimum value
   unsigned int signalMin = 1024;                         //maximum value
 
                                                          // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(SENSOR_PIN);                    //get reading from microphone
      if (sample < 1024)                                  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;                           // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;                           // save just the min levels
         }
      }
   }
 
   peakToPeak = signalMax - signalMin;                    // max - min = peak-peak amplitude
   int db = map(peakToPeak,20,900,49.5,90);             //calibrate for deciBels
   int db2=db-49;
 Blynk.virtualWrite(V0,db2);
  lcd.setCursor(0, 0);
  lcd.print("Loudness: ");
  lcd.print(db-49 );
  lcd.print("dB");
  
  if (db <= 50)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: Quite");
    digitalWrite(PIN_QUIET, HIGH);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, LOW);
  }
  else if (db > 50 && db<93)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: Moderate");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, HIGH);
    digitalWrite(PIN_LOUD, LOW);
  }
  else if (db>=93)
  {
    lcd.setCursor(0, 1);
    lcd.print("Level: High");
    digitalWrite(PIN_QUIET, LOW);
    digitalWrite(PIN_MODERATE, LOW);
    digitalWrite(PIN_LOUD, HIGH);
 
  }
   
 
  if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(db2);
    postStr += "r\n";
    
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
   
  }
    client.stop();
 
   delay(200);      // thingspeak needs minimum 15 sec delay between updates.
   lcd.clear();
}
