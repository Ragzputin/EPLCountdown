#include <Keypad.h>
#include <SPI.h>
#include <Wire.h>
#include "WiFly.h"
#include "Time.h"
#include "Credentials.h"
#include <LiquidCrystal_I2C.h>
#include <CalcTime.h>
#include <EPLTeams.h>

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

char keymap[4][4] =
{
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[4] = {9,8,7,6};
byte colPins[4] = {5,4,3,2};

//initialize myKeypad object with above variables
Keypad myKeypad = Keypad(makeKeymap(keymap),rowPins,colPins,4,4);

uint16_t key_num = 0;
boolean sc_flag = false;
int lpcnt = 0;
boolean client_connect = false;
boolean kpad_setup_flag = false;

boolean flag1 = false;
boolean flag2 = false;
boolean flag3 = false;
boolean flag4 = false;
boolean flag5 = false;
boolean homeTeamflag = false;
boolean awayTeamflag = false;
boolean cstopFlag = false;
int lpcount = 0;

int letterCount = 0;
int not_EPL = 0;
int game_state = 0;
char msg[500];
char * ptr = &msg[0];
char * ptr2;
int tmp = 0;

char hmTeam[25];
char awTeam[25];
char dateTime[25];

char * tmNameh = &hmTeam[0];
char * tmNamea = &awTeam[0];
int lenh = sizeof(hmTeam)/sizeof(hmTeam[0]);
int lena = sizeof(awTeam)/sizeof(awTeam[0]);

long current_time;
char * dTptr = &dateTime[0];
CalcTime calc(dTptr);

uint16_t days, hrs, mins, sec;

EPLTeams team;
char * teamcodeptr;

WiFlyClient client("api.football-data.org", 80);

void setup(){
  WiFly.begin();

  Serial.begin(9600);

  //client_connect = true; 
  lcd.begin (16,2);

  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                   // go home
  lcd.print("Loading...");
  delay(2000);

  kpad_setup_flag = true;
  sc_flag = true;
}

void loop(){

  if(!client_connect && kpad_setup_flag){
    if (sc_flag){
      lcd.clear();
      lcd.print("enter team #:");
      lcd.setCursor(0,1);
      sc_flag = false;
    }

    char key = myKeypad.getKey();
    delay(10);
    if(int(key) != 0 && lpcnt < 4){
      if(int(key) >= 48 && int(key) <= 57){
        lcd.print(key);
        lpcnt++;

        if(key_num == 0){
          key_num += (key - '0');
        } 
        else {
          key_num = (key_num*10) + (key - '0');
        }
      } /*else if(int(key) != 35) {
        lcd.setCursor(0,1);
        lpcnt = 0;
        key_num = 0;
      }*/
    }
    
    if(int(key) == 35 && (lpcnt == 2 || lpcnt == 3)){
     delay(1000);
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print(key_num);
     lcd.print(" ");
     client_connect = true;
     kpad_setup_flag = false;
     team.setTeamCode(key_num);
     teamcodeptr = team.getTeamCode();
     int i;
     char * tmp = teamcodeptr;
     for(i = 0; i < team.getTeamCodeLength(); i++)
     {
       lcd.print(*tmp);
       tmp++;
     }
    }
  }

  if(client_connect && !kpad_setup_flag){
    delay(100);
    current_time = WiFly.getTime();
    delay(500);
    Serial.print("connecting to server...");
    if(client.connect()){
      Serial.println("connected");
      client.print("GET http://api.football-data.org/teams/");
      client.print(key_num);
      client.print("/fixtures/?timeFrame=n8");
      client.println(" HTTP/1.1");
      client.println("Host: api.football-data.org");
      client.println("X-Auth-Token: 4f02cc524412487989ee61aed27503d5"); 
      client.println("Connection: close");
      client.println();
    } 
    else{
      Serial.println("connection failed");
    }
    client_connect = false;
  }

  if(!client_connect && !kpad_setup_flag){
    if(client.available()){
      char c = client.read();
      Serial.print(c);

      if(flag3 && letterCount < 501){
        recordMessage(c);
        flag4 = true;
      }

      if(c == '\n' && flag4 && !client.available()){
        HttpResponseParsing();
      }

      if(c == '_'){
        flag1 = true;
      }

      if(flag1 && c == 'l'){
        flag2 = true;
      }

      if(flag2 && c == 'i'){
        flag3 = true;
      }
    }

    if(!client.connected() && !cstopFlag){
      client.flush();
      client.stop();
      Serial.println("Disconnected");
      delay(100);
      compute();
      cstopFlag = true;
      flag5 = true;
    }
    
    if(!client.connected() && flag5){
      print_countdown();
    }
    
  }

}

void compute(){
  
  calc.gametime_calc();
  calc.compute_times(current_time);
  
  days = calc.days();
  hrs = calc.hours();
  mins = calc.minutes();
  sec = calc.seconds();
  
}

void print_countdown(){
  
  
  lcd.setBacklight(LOW);
  
  lcd_print(days,0); //print days
  lcd_print(hrs,3); //print hours
  lcd_print(mins,6); //print minutes
  lcd_print(sec,9); //print seconds
  lcd.print("  ");
  
  if(sec == 0 && mins == 0 && hrs == 0 && days == 0){
    lcd.clear();
    delay(100);
    lcd.setBacklight(HIGH);
    lcd.setCursor(0,0);
    lcd.print("Game begins now!");
    //delay(60000);
    //lcd.clear();
    flag5 = false;
    //game_state = 1;
  } 
  else if(sec == 0 && mins != 0){
    mins--;
    lpcount++;
    sec = 60;
  } 
  else if(sec == 0 && mins == 0 && hrs != 0){
    hrs--;
    mins = 59;
    sec = 60;
  } 
  else if(sec == 0 && mins == 0 && hrs == 0 && days != 0){
    days--;
    hrs = 23;
    mins = 59;
    sec = 60;
  }
  
  if(lpcount == 60){
    lcd.setBacklight(HIGH);
    lcd.setCursor(0,1);
    lcd.print("Resetting...");
    current_time = WiFly.getTime();
    compute();
    lpcount = 0;
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
  } 
  else{
    lcd.print(period);
  }
  if(col != 9){
    lcd.print(":");
  }
}

void recordMessage(char message){
  msg[letterCount] = message;
  letterCount++;
  delay(1);
}


void HttpResponseParsing(){
  if(not_EPL == 1 || game_state == 1){
    ptr = ptr2;
    not_EPL = 0;
    game_state = 0;
    pointerLogic();
  } 
  else {
    pointerLogic();
  }
  
  if(not_EPL != 1){
    lcd.clear();
    lcd.setCursor(0,0);
    //get information of the Home Team
    teamSetup(tmNameh, false);
    //get information of the Away Team
    teamSetup(tmNamea, true);
  }
}

void teamSetup(char * teamptr, boolean teamflag){
  team.setKeynum(teamptr, teamflag);
  int keynum = team.getKeynum(teamflag);
  team.setTeamCode(keynum);
  teamcodeptr = team.getTeamCode();

  int i;
  char * tmp = teamcodeptr;
  for(i = 0; i < team.getTeamCodeLength(); i++)
  {
    lcd.print(*tmp);
    tmp++;
  }
  
  if(!teamflag){
    lcd.print(" v ");
  }
}

void pointerLogic(){
  int count = 0; //this is to count for hmTeam character array
  int i;
  for(i = 0; i < 501; i++){
    if(*(ptr+i) == 'd' && *(ptr+i+1) == 'a' && *(ptr+i+2) == 't' && *(ptr+i+3) == 'e'){
      ptr2 = ptr+i+7;
      while(*ptr2 != '"'){
        dateTime[count] = *ptr2;
        ptr2++;
        count++;  
      } 
    }
    count = 0;
    if(*(ptr+i) == 'h' && *(ptr+i+4) == 'T' && *(ptr+i+7) == 'm'){ //once the sequence "h T m" is found, we know we have reached the word "homeTeam" in the string
      ptr2 = ptr+i+11;
      while(*ptr2 != '"'){
        hmTeam[count] = *ptr2;
        ptr2++;
        count++;  
      }
      /*lcd.clear();
      lcd.setCursor(0,0);
      homeTeamflag = true;
      teamNamePrint(tmNameh, lenh);
      homeTeamflag = false; */
      homeTeamflag = true;
    }
    count = 0;
    if(*(ptr+i) == 'a' && *(ptr+i+3) == 'y' && *(ptr+i+4) == 'T' && *(ptr+i+7) == 'm'){
      ptr2 = ptr+i+11;
      while(*ptr2 != '"'){
        awTeam[count] = *ptr2;
        ptr2++;
        count++;  
      }
      /*if(not_EPL != 1){
        awayTeamflag = true; 
        teamNamePrint(tmNamea, lenh);
        awayTeamflag = false;
      }*/
      awayTeamflag = true;
      break;
    }
  }

}

void lcd_print(uint16_t period, int col){

  if(period < 10){
    lcd.setCursor(col,1);
    lcd.print("0");
    lcd.setCursor(col+1,1);
    lcd.print(period);
  } 
  else{
    lcd.print(period);
  }
  if(col != 9){
    lcd.print(":");
  }
}

