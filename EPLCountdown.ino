#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Wire.h>
#include "Time.h"
#include "WiFly.h" //include the WiFly experimental library
#include "Credentials.h"

#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

char msg[100];
int letterCount = 0;
int fixFlag = 0;
int colonFlag = 0;
int quoteFlag = 0;
int datetimeFlag = 0;
int lcdprintFlag = 0;
int n = 1;

WiFlyClient client("api.football-data.org", 80);
long current_time;

void setup(){

  
  //Begin LCD
  lcd.begin (16,2);
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                   // go home
  
  //Begin WiFly and Serial
  WiFly.begin();
  Serial.begin(9600);
  Serial.println("Serial begun :D");
  
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    while (1) {
      // Hang on failure.
    }
  } 
  
  current_time = WiFly.getTime();
  
  Serial.print("connecting to server...");
  if(client.connect()){
    Serial.println("connected");
    client.print("GET http://api.football-data.org/teams/61/fixtures/?timeFrame=n7");
    Serial.print("GET http://api.football-data.org/teams/61/fixtures/?timeFrame=n7");
    client.println(" HTTP/1.1");
    Serial.println(" HTTP/1.1");
    client.println("Host: api.football-data.org");
    Serial.println("Host: api.football-data.org");
    client.println("X-Auth-Token: 4f02cc524412487989ee61aed27503d5");
    Serial.println("X-Auth-Token: 4f02cc524412487989ee61aed27503d5");  
    client.println("Connection: close");
    Serial.println("Connection: close");
    client.println();
    delay(100);
    client_ops();
  } else{
    Serial.println("connection failed");
  }
  
}

void loop(){
  Serial.println("Entered the loop");
  delay(1000);
  //lcd.home();
  lcd.print (n++, DEC);
  delay(200);
}

void client_ops(){
    while(client.connected()){
    if (client.available()) {
    char c = client.read();
    Serial.print(c);
    
    if(datetimeFlag == 1 && c != 'Z'){
      recordMessage(c);
    }
    
    if(c == '\n' && !client.available() && datetimeFlag == 1){
      checkAction();
      Serial.println("Exited checkAction");
    }
    
    if(c == '['){
      fixFlag = 1;
    }
    
    if(fixFlag == 1){
      if(c == 'e'){
        colonFlag = 1;
      }
    }
    
    if(colonFlag == 1){
      if(c == ':'){
        quoteFlag = 1;
      }
    }
    
    if(quoteFlag == 1){
      if(c == '"'){
        datetimeFlag = 1;
      }
    }

  }
  }
  
  if(!client.available()){
    client.flush();
    client.stop();
    Serial.println("Disconnected");
  }
}

void recordMessage(char message){
  msg[letterCount] = message;
  letterCount++;
  delay(10);
}

void checkAction(){
   
    TimeElements tm; //create a TimeElements variable
                    //for conversion to time_t variable
    
    //Fill in the elements of tm with those from the msg array
    tm.Second = (msg[17] - '0')*10 + (msg[18] - '0');
    tm.Minute = (msg[14] - '0')*10 + (msg[15] - '0');
    tm.Hour = (msg[11] - '0')*10 + (msg[12] - '0');
    tm.Day = (msg[8] - '0')*10 + (msg[9] - '0');
    tm.Month = (msg[5] - '0')*10 + (msg[6] - '0');
    tm.Year = ((msg[0] - '0')*1000 + (msg[1] - '0')*100 + (msg[2] - '0')*10 + (msg[3] - '0')) - 1970;
    
    long gametime = makeTime(tm); //game time as time_t variable

    long timediff; //time diff between current time and game time in the future
    timediff = gametime - current_time;
    
    float tdiff_hrs;
    tdiff_hrs = float(timediff) / 3600;
    
    Serial.println(timediff);
}
