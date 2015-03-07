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

char msg[320];
char * ptr = &msg[0];
int letterCount = 0;
int countdownFlag = 0;
int cstopFlag = 0;
int lpcount = 0;
void(* resetFunc)(void) = 0; //pointer to reset function @ address 0

long days, hrs, mins, sec, rem1, rem2;
long gametime, timediff, new_timediff, current_time;

int flag1 = 0;
int flag2 = 0;
int flag3 = 0;
int flag4 = 0;
int flag5 = 0;
int flag6 = 0;
int flag7 = 0;
char hmTeam[25];
char awTeam[25];
char dateTime[25];

WiFlyClient client("api.football-data.org", 80);

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
    client.println(" HTTP/1.1");
    client.println("Host: api.football-data.org");
    client.println("X-Auth-Token: 4f02cc524412487989ee61aed27503d5"); 
    client.println("Connection: close");
    client.println();
  } else{
    Serial.println("connection failed");
  }

}

void loop(){
  
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
    
    if(flag4 == 1 && letterCount < 321){
      recordMessage(c);
      flag5 = 1;
    }
    
    if(c == '\n' && !client.available() && flag5 == 1){
      
      char * ptr2; //declare a second pointer to change reference from start of msg string to start of "homeTeam" - i.e. "h"
      int count = 0; //this is to count for hmTeam character array
      int i;
      for(i = 0; i < 321; i++){
        if(*(ptr+i) == 'h' && *(ptr+i+4) == 'T' && *(ptr+i+7) == 'm'){ //once the sequence "h T m" is found, we know we have reached the word "homeTeam" in the string
          ptr2 = ptr+i+11;
          while(*ptr2 != '"'){
            hmTeam[count] = *ptr2;
            ptr2++;
            count++;  
          } 
        }
        count = 0;
        if(*(ptr+i) == 'a' && *(ptr+i+3) == 'y' && *(ptr+i+4) == 'T' && *(ptr+i+7) == 'm'){
            ptr2 = ptr+i+11;
            while(*ptr2 != '"'){
              awTeam[count] = *ptr2;
              ptr2++;
              count++;  
            } 
        }
        count = 0;
        if(*(ptr+i) == 'd' && *(ptr+i+2) == 't' && *(ptr+i+3) == 'e'){
            ptr2 = ptr+i+7;
            while(*ptr2 != '"'){
              dateTime[count] = *ptr2;
              ptr2++;
              count++;  
            } 
        }
      }
  
    }
    
    if(c == '{'){
      flag1 = 1;
    }
    
    if(flag1 == 1){
      if(c == '"'){
        flag2 = 1;
      }
    }
    
    if(flag2 == 1){
      if(c == '_'){
        flag3 = 1;
      }
    }
    
    if(flag3 == 1){
      if(c == 'l'){
        flag4 = 1;
      }
    }
    
  }
  
  if(!client.connected() && cstopFlag == 0){
    delay(100);
    client.flush();
    client.stop();
    Serial.println();
    Serial.println("Disconnected.");
    delay(500);
    Serial.println();
    cstopFlag = 1;
    flag7 = 1;
  }
  
  if(flag7 == 1){
    checkAction();
    countdownFlag = 1;
  }
  
  if(countdownFlag == 1){
    //lcd.setCursor(0,0);
    //lcd.setBacklight(LOW);
    //teamNamePrint(hmTeam);
    //lcd.print(" vs ");
    //teamNamePrint(awTeam);
    countdown();
  }
  
}

void recordMessage(char message){
  msg[letterCount] = message;
  letterCount++;
}

/*
void teamNamePrint(){
    char team[25];
    strcpy(team,hmTeam);
    if(team == "Arsenal FC")
      lcd.print("Arsenal");
    else if (team == "Aston Villa FC")
      lcd.print("AVFC");
    else if (team =="Burnley FC")
      lcd.print("Burnley");
    else if (team =="Chelsea FC")
      lcd.print("Chelsea");
    else if (team =="Crystal Palace FC")
      lcd.print("CPFC");
    else if (team =="Everton FC")
      lcd.print("Everton");
    else if (team =="Hull City")
      lcd.print("Hull");
    else if (team =="Leiceter City FC")
      lcd.print("Lcity");
    else if (team =="Liverpool FC")
      lcd.print("Lpool");
    else if (team =="Manchester City FC")
      lcd.print("ManCity");
    else if (team =="Manchester United")
      lcd.print("ManU");
    else if (team =="Newcastle FC")
      lcd.print("NCFC");
    else if (team =="Queens Park Rangers")
      lcd.print("QPR");
    else if (team =="FC Southampton")
      lcd.print("Sthtn");
    else if (team =="Stoke FC")
      lcd.print("Stoke");
    else if (team =="Sunderland FC")
      lcd.print("Sndlnd");
    else if (team =="Swansea FC")
      lcd.print("SwnFC");
    else if (team =="Tottenham Hotspur")
      lcd.print("Spurs");
    else if (team =="West Bromwich Albion")
      lcd.print("WBrom");     
    else if (team =="West Ham Utd")
      lcd.print("WHU");

}
*/

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
    lcd.setBacklight(HIGH);
    lcd.setCursor(0,0);
    lcd.print("Game begins now!");
    lcd.setCursor(0,1);
    lcd.print("            ");
    delay(30000);
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
  tm.Second = (dateTime[17] - '0')*10 + (dateTime[18] - '0');
  tm.Minute = (dateTime[14] - '0')*10 + (dateTime[15] - '0');
  tm.Hour = (dateTime[11] - '0')*10 + (dateTime[12] - '0');
  tm.Day = (dateTime[8] - '0')*10 + (dateTime[9] - '0');
  tm.Month = (dateTime[5] - '0')*10 + (dateTime[6] - '0');
  tm.Year = ((dateTime[0] - '0')*1000 + (dateTime[1] - '0')*100 + (dateTime[2] - '0')*10 + (dateTime[3] - '0')) - 1970;
  
  gametime = makeTime(tm); //game time as time_t variable
}
