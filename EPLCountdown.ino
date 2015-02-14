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

char msg[19];
int letterCount = 0;
int fixFlag = 0;
int colonFlag = 0;
int quoteFlag = 0;
int datetimeFlag = 0;
int recordFlag = 0;
int countdownFlag = 0;
int cstopFlag = 0;

int len = sizeof(msg) / sizeof(msg[0]);
long days, hrs, mins, sec, rem1, rem2;

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
    client.print("GET http://api.football-data.org/teams/61/fixtures/?timeFrame=n5");
    Serial.print("GET http://api.football-data.org/teams/61/fixtures/?timeFrame=n5");
    client.println(" HTTP/1.1");
    Serial.println(" HTTP/1.1");
    client.println("Host: api.football-data.org");
    Serial.println("Host: api.football-data.org");
    client.println("X-Auth-Token: 4f02cc524412487989ee61aed27503d5");
    Serial.println("X-Auth-Token: 4f02cc524412487989ee61aed27503d5");  
    client.println("Connection: close");
    Serial.println("Connection: close");
    client.println();
  } else{
    Serial.println("connection failed");
  }

}

void loop(){
  
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
    
    if(datetimeFlag == 1 && letterCount < len){
      recordMessage(c);
      if(letterCount == len){
        recordFlag = 1;
      }
    }
    
    if(c == '\n' && !client.available() && recordFlag == 1){
      checkAction();
      countdownFlag = 1;
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
  
  if(!client.connected() && cstopFlag == 0){
    delay(100);
    client.flush();
    client.stop();
    Serial.println();
    Serial.println("Disconnected.");
    Serial.println();
    cstopFlag = 1;
    //while(1){}
  }
  
  if(countdownFlag == 1){
    countdown();
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
    Serial.println(timediff);
    
    days = 2;//timediff / 86400;
    rem1 = 0;//timediff % 86400;
    hrs = 1;//rem1 / 3600;
    rem2 = 0;//rem1 % 3600;
    mins = 1;//rem2 / 60;
    sec = 20;//rem2 % 60;
    
}

void countdown(){
  lcd.setCursor(0,0);
  if(days < 10){
    lcd.print("0");
  }
    
  lcd.print(days);
  lcd.print(":");
  lcd.print(hrs);
  lcd.print(":");
  lcd.print(mins);
  lcd.print(":");
  lcd.print(sec--);
  delay(1000);
  
  if(sec < 10){
    lcd.setCursor(10,0);
    lcd.print("-");
    lcd.setCursor(0,0);
  }
  
  if(sec == 0){
    lcd.print(mins--);
    sec = 59;
  } else if(mins == 0){
    lcd.print(hrs--);
    mins = 59;
  } else if(hrs == 0){
    lcd.print(days--);
    hrs = 23;
  }
}
