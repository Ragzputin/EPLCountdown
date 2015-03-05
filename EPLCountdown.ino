/*merged results*/

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
int lpcount = 0;
void(* resetFunc)(void) = 0; //pointer to reset function @ address 0

int len = sizeof(msg) / sizeof(msg[0]);
long days, hrs, mins, sec, rem1, rem2;
long gametime, timediff, new_timediff;

WiFlyClient client("api.football-data.org", 80);
long current_time;

void setup(){
  
  //Begin LCD
  lcd.begin (16,2);
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                   // go home
  lcd.print("Loading...");
  
  //Begin WiFly and Serial
  WiFly.begin();
  Serial.begin(9600);
  Serial.println("Serial begun :D");
  
  current_time = WiFly.getTime();
  Serial.print("connecting to server...");
  if(client.connect()){
    Serial.println("connected");
    client.print("GET http://api.football-data.org/teams/61/fixtures/?timeFrame=n20");
    Serial.print("GET http://api.football-data.org/teams/61/fixtures/?timeFrame=n20");
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

  }
  
  if(countdownFlag == 1){
    lcd.setCursor(0,0);
    lcd.print("dd:hh:mm:ss");
    lcd.setBacklight(LOW);
    countdown();
  }
  
}

void recordMessage(char message){
  msg[letterCount] = message;
  letterCount++;
  delay(10);
}

void checkAction(){
    
    if(lpcount < 60){
      gametime_calc();
      timediff = gametime - current_time;
      
      days = timediff / 86400;
      rem1 = timediff % 86400;
      hrs = rem1 / 3600;
      rem2 = rem1 % 3600;
      mins = rem2 / 60;
      sec = rem2 % 60;
    } else if(lpcount == 60){
      lpcount = 0;
      current_time = WiFly.getTime();
      Serial.println();
      Serial.print("game time = ");
      Serial.println(gametime);
      Serial.print("current time = ");
      Serial.println(current_time);
      new_timediff = gametime - current_time;
      if((new_timediff - timediff) != 0){
        lcd.setBacklight(HIGH);
        lcd.clear();
        lcd.print("Resetting...");
        delay(1500);
        
        days = new_timediff / 86400;
        rem1 = new_timediff % 86400;
        hrs = rem1 / 3600;
        rem2 = rem1 % 3600;
        mins = rem2 / 60;
        sec = rem2 % 60;
        
        lcd.clear();
      }
    }
}

void countdown(){
  lcd.setCursor(0,1);
  
  lcd_print(days,0); //print days
  lcd_print(hrs,3); //print hours
  lcd_print(mins,6); //print minutes
  lcd_print(sec,9); //print seconds
  
  if(sec == 0 && mins == 0 && hrs == 0 && days == 0){
    lcd.clear();
    delay(100);
    lcd.setBacklight(HIGH);
    lcd.setCursor(0,0);
    lcd.print("Game begins now!");
    delay(60000);
    resetFunc(); //call reset
  } else if(sec == 0 && mins != 0){
    mins--;
    lpcount++;
    sec = 60;
  } else if(sec == 0 && mins == 0 && hrs != 0){
    hrs--;
    mins = 59;
    sec = 60;
  } else if(sec == 0 && mins == 0 && hrs == 0 && days != 0){
    days--;
    hrs = 23;
    mins = 59;
    sec = 60;
  }
  
  if(lpcount == 60){
    checkAction();
    lcd.setCursor(0,1);
    lcd_print(days,0); //print days
    lcd_print(hrs,3); //print hours
    lcd_print(mins,6); //print minutes
    lcd_print(sec,9); //print seconds
  }
  
  delay(1000);
  sec--;
}

void lcd_print(long period, int col){
  if(period < 10){
    lcd.setCursor(col,1);
    lcd.print("0");
    lcd.setCursor(col+1,1);
    lcd.print(period);
  } else{
    lcd.print(period);
  }
  if(col != 9){
    lcd.print(":");
  }
}

void gametime_calc(){
  TimeElements tm; //create a TimeElements variable
                      //for conversion to time_t variable
    
  //Fill in the elements of tm with those from the msg array
  tm.Second = (msg[17] - '0')*10 + (msg[18] - '0');
  tm.Minute = (msg[14] - '0')*10 + (msg[15] - '0');
  tm.Hour = (msg[11] - '0')*10 + (msg[12] - '0');
  tm.Day = (msg[8] - '0')*10 + (msg[9] - '0');
  tm.Month = (msg[5] - '0')*10 + (msg[6] - '0');
  tm.Year = ((msg[0] - '0')*1000 + (msg[1] - '0')*100 + (msg[2] - '0')*10 + (msg[3] - '0')) - 1970;
  
  gametime = makeTime(tm); //game time as time_t variable
}
