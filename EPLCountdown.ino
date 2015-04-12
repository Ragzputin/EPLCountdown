#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
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

char keymap[4][4] =
{
  {'1', '2', '3', 'A'  },
  {'4', '5', '6', 'B'  },
  {'7', '8', '9', 'C'  },
  {'*', '0', '#', 'D'  }
};

byte rowPins[4] = {9,8,7,6};
byte colPins[4] = {5,4,3,2};

//initialize myKeypad object with above variables
Keypad myKeypad = Keypad(makeKeymap(keymap),rowPins,colPins,4,4);

WiFlyClient client("api.football-data.org", 80);

int key_num = 0; //this is the team number acquired from keypad
boolean sc_flag = false;
int lpcnt = 0;
boolean client_connect = false;
boolean kpad_flag = false;

boolean flag1 = false;
boolean flag2 = false;
boolean flag3 = false;
boolean flag4 = false;
boolean flag5 = false;
boolean homeTeamflag = false;
boolean awayTeamflag = false;
boolean cstopFlag = false;
int lpcount = 0;
boolean countdownFlag = false;

int letterCount = 0;
int not_EPL = 0;
int game_state = 0;
char msg[500];
char * ptr = &msg[0];
char * ptr2; //declare a second pointer to change reference from start of msg string to start of date i.e. "d"

char hmTeam[25];
char awTeam[25];
char dateTime[25];

//initialize pointers to the start of homeTeam and awayTeam strings
//and the lengths of these two strings respectively
char * tmNameh = &hmTeam[0];
char * tmNamea = &awTeam[0];
int lenh = sizeof(hmTeam)/sizeof(hmTeam[0]);
int lena = sizeof(awTeam)/sizeof(awTeam[0]);

long rem1, rem2, days, hrs, mins, sec;
long gametime, timediff, new_timediff, current_time;

//void(* resetFunc)(void) = 0; //pointer to reset function @ address 0


void setup(){
  WiFly.begin();
  
  Serial.begin(9600);
  Serial.println("Serial begun");
  delay(1000);
  
  //client_connect = true; 
  lcd.begin (16,2);

  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                   // go home
  lcd.print("Loading...");
  delay(1000);

  kpad_flag = true;
  sc_flag = true;
  
}

void loop(){
  if(kpad_flag && !client_connect){
    if(sc_flag){
      lcd.clear();
      lcd.print("enter team #:");
      lcd.setCursor(0,1);
      sc_flag = false;
    }
    
  char key = myKeypad.getKey();
  if(key != NO_KEY && lpcnt < 3 && key != '#'){
    lcd.print(key);
    lpcnt++;

    if(key != '*' || key != '#' || key != 'A' || key != 'B' || key != 'C' || key != 'D'){
      if(key_num == 0){
        key_num += (key - '0');
      } 
      else {
        key_num = (key_num*10) + (key - '0');
      }
    }

  }

  if((key == '#') && ((lpcnt == 2) || (lpcnt == 3))){
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(key_num);
    lcd.print(" ");
    client_connect = true;
    kpad_flag = false;
    switch(key_num){
    case 57:
      lcd.print("ARS");
      break;
    case 58:
      lcd.print("AVFC");
      break;
    case 61:
      lcd.print("CHE");
      break;
    case 354:
      lcd.print("CRP");
      break;      
    case 62:
      lcd.print("EVR");
      break;           
    case 322:
      lcd.print("HUL");
      break;
    case 338:
      lcd.print("LEIC");
      break;           
    case 64:
      lcd.print("LIV");
      break;
    case 65:
      lcd.print("MCITY");
      break;
    case 66:
      lcd.print("MANU");
      break;
    case 67:
      lcd.print("NEW");
      break;
    case 69:
      lcd.print("QPR");
      break;
    case 340:
      lcd.print("SOU");
      break;
    case 73:
      lcd.print("SPURS");
      break;           
    case 70:
      lcd.print("STK");
      break;
    case 71:
      lcd.print("SUND");
      break;
    case 72:
      lcd.print("SWA");
      break;
    case 74:
      lcd.print("WBA");
      break;          
    case 563:
      lcd.print("WHU");
      break;          
    default:
      lcd.print("No match found");
      break;        
    }
  }
  }

  if(client_connect){
    current_time = WiFly.getTime();
    Serial.print("connecting to server...");
    if(client.connect()){
      Serial.println("connected");
      client.print("GET http://api.football-data.org/teams/");
      client.print(key_num); 
      client.print("/fixtures/?timeFrame=n7");
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

  if(!client_connect && !kpad_flag){
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
      
      if(flag3 && letterCount < 501){
        recordMessage(c);
        flag4 = true;
      }
      
      if(c == '\n' && !client.available() && flag4){
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
      delay(100);
      client.flush();
      client.stop();
      Serial.println();
      Serial.println("Disconnected.");
      delay(1000);
      Serial.println();
      cstopFlag = true;
      //flag5 = true;
      /*
      Serial.println(dateTime);
      delay(1000);
      Serial.println(hmTeam);
      delay(1000);
      Serial.println(awTeam);
      delay(3000);
      Serial.println();
      Serial.println(current_time);
      delay(3000);
      */
      //computeTimes();
    }
    
    /*
    if(flag5 == true && !client.available()){ 
      //current_time = WiFly.getTime();
      if(current_time < 1000){ //to remove glitch which shows huge value for dd in dd:hh:mm:ss
        //resetFunc(); //call reset
        delay(10000);
      }
      computeTimes();
      if(current_time > (gametime + 8000)){ //if the current game has already passed, we need to look for the next game.
        //7800-8000 seconds is the average length of a football game (2h10m)
        game_state = 1;
      } 
      else {
        countdownFlag = true;
      }
      flag5 = false;
    }
    
    
    if(countdownFlag == true && not_EPL != 1){
      lcd.setCursor(0,1);
      lcd.setBacklight(LOW);
      countdown();
      if(lpcount == 60){
        computeTimes();
      }
    }

    //if countdown has been completed, attempt countdown
    //to next game.
    if(game_state == 1){
      //clear arrays to all 0s
      clearArrays(dateTime);
      clearArrays(hmTeam);
      clearArrays(awTeam);
      HttpResponseParsing();
      flag5 = true;

      if(dateTime[0] == '0'){
        lcd.print("No games for");
        lcd.setCursor(0,1);
        lcd.print("next 15 days");
      }
      game_state = 0;
    }
    */
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
      lcd.clear();
      lcd.setCursor(0,0);
      homeTeamflag = true;
      teamNamePrint(tmNameh, lenh);
      homeTeamflag = false; 
    }
    count = 0;
    if(*(ptr+i) == 'a' && *(ptr+i+3) == 'y' && *(ptr+i+4) == 'T' && *(ptr+i+7) == 'm'){
      ptr2 = ptr+i+11;
      while(*ptr2 != '"'){
        awTeam[count] = *ptr2;
        ptr2++;
        count++;  
      }
      if(not_EPL != 1){
        awayTeamflag = true; 
        teamNamePrint(tmNamea, lenh);
        awayTeamflag = false;
      }
      break;
    }
  }

}
/*
void clearArrays(char * array_val){
  //used to clear array to all 0s
  while(*array_val != '\0'){
    *array_val = '0';
    array_val++;
  }

}
*/


void teamNamePrint(char * teamName, int length){
  int i;

  for(i = 0; i < length; i++){
    //Here we look up the team's full name given in the HTTP response and print out the short name to the LCD screen
    if(*(teamName + i) == 'F' && *(teamName + i + 1) == 'C' && *(teamName + i + 3) == 'A' && *(teamName + i + 4) == 'r'){
      lcd.print("ARS");
      break;
    }
    else if(*(teamName + i) == 'A' && *(teamName + i + 1) == 's' && *(teamName + i + 2) == 't'){
      lcd.print("AVFC");
      break;
    }
    else if(*(teamName + i) == 'F' && *(teamName + i + 1) == 'C' && *(teamName + i + 3) == 'B' && *(teamName + i + 4) == 'u'){
      lcd.print("BUR");
      break;
    }
    else if(*(teamName + i) == 'C' && *(teamName + i + 1) == 'r' && *(teamName + i + 2) == 'y'){
      lcd.print("CRP");
      break;
    }
    else if(*(teamName + i) == 'C' && *(teamName + i + 1) == 'h' && *(teamName + i + 2) == 'e'){
      lcd.print("CHE");
      break;
    }
    else if(*(teamName + i) == 'E' && *(teamName + i + 1) == 'v' && *(teamName + i + 2) == 'e'){
      lcd.print("EVR");
      break;
    }
    else if(*(teamName + i) == 'H' && *(teamName + i + 1) == 'u' && *(teamName + i + 2) == 'l'){
      lcd.print("HUL");
      break;
    }
    else if(*(teamName + i) == 'L' && *(teamName + i + 1) == 'e' && *(teamName + i + 2) == 'i'){
      lcd.print("LEIC");
      break;
    }
    else if(*(teamName + i) == 'L' && *(teamName + i + 1) == 'i' && *(teamName + i + 2) == 'v'){
      lcd.print("LIV");
      break;
    }
    else if(*(teamName + i) == 'M' && *(teamName + i + 1) == 'a' && *(teamName + i + 2) == 'n' && *(teamName + i + 11) == 'C'){
      lcd.print("MCITY");
      break;
    }
    else if(*(teamName + i) == 'M' && *(teamName + i + 1) == 'a' && *(teamName + i + 2) == 'n' && *(teamName + i + 11) == 'U'){
      lcd.print("MANU");
      break;
    }
    else if(*(teamName + i) == 'N' && *(teamName + i + 1) == 'e' && *(teamName + i + 2) == 'w'){
      lcd.print("NEW");
      break;
    }
    else if(*(teamName + i) == 'Q' && *(teamName + i + 1) == 'u' && *(teamName + i + 2) == 'e'){
      lcd.print("QPR");
      break;
    }
    else if (*(teamName + i) == 'F' && *(teamName + i + 1) == 'C' && *(teamName + i + 3) == 'S' && *(teamName + i + 4) == 'o'){
      lcd.print("SOU");
      break;
    }
    else if(*(teamName + i) == 'S' && *(teamName + i + 1) == 't' && *(teamName + i + 2) == 'o'){
      lcd.print("STK");
      break;
    }
    else if(*(teamName + i) == 'S' && *(teamName + i + 1) == 'u' && *(teamName + i + 2) == 'n'){
      lcd.print("SUND");
      break;
    }
    else if(*(teamName + i) == 'S' && *(teamName + i + 1) == 'w' && *(teamName + i + 2) == 'a'){
      lcd.print("SWA");
      break;
    }
    else if(*(teamName + i) == 'T' && *(teamName + i + 1) == 'o' && *(teamName + i + 2) == 't'){
      lcd.print("SPURS");
      break;
    }
    else if(*(teamName + i) == 'W' && *(teamName + i + 1) == 'e' && *(teamName + i + 2) == 's' && *(teamName + i + 5) == 'B'){
      lcd.print("WBA");
      break;
    }
    else if(*(teamName + i) == 'W' && *(teamName + i + 1) == 'e' && *(teamName + i + 2) == 's' && *(teamName + i + 5) == 'H'){
      lcd.print("WHU");
      break;
    } 
    else{
      lcd.print("N/A");
      not_EPL = 1;
      break;
    }
  }

  if(not_EPL != 1 && homeTeamflag == true){
    lcd.print(" v ");
  }
}

void computeTimes(){
  if(lpcount < 60){
    gametime_calc();
    timediff = gametime - current_time;
    days = timediff / 86400;
    rem1 = timediff % 86400;
    hrs = rem1 / 3600;
    rem2 = rem1 % 3600;
    mins = rem2 / 60;
    sec = rem2 % 60;
  } 
  else if(lpcount == 60){
    lpcount = 0;
    current_time = WiFly.getTime();
    new_timediff = gametime - current_time;
    if((new_timediff - timediff) != 0){
      lcd.setBacklight(HIGH);
      lcd.clear();
      lcd.print("Resetting...");
      delay(1500);
      HttpResponseParsing();

      days = new_timediff / 86400;
      rem1 = new_timediff % 86400;
      hrs = rem1 / 3600;
      rem2 = rem1 % 3600;
      mins = rem2 / 60;
      sec = rem2 % 60;
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
    lcd.clear();
    countdownFlag = false;
    game_state = 1;

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
    computeTimes();
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
  } 
  else{
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

