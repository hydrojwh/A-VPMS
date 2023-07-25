#include <Wire.h>
#include "RTClib.h" //RTC library, RTClib by Adafruit 2.1.1 install


// ------- RGB LED
int RED = 12;
int GREEN = 11;
int BLUE = 10;

//stepper input
int PUL=7; //define Pulse pin
int DIR=8; //define Direction pin
int ENA=9; //define Enable Pin

//endstop
const int Endstop = 5;

//depth measurment
long DEPTH = 0;

bool isDown = false; //false = down, true = up [default = true]
bool isWaiting = true; //wait until the next measurement [default = true]
bool isTime = false;

int relay = 6;

// = = = = = = = = = = = = = = = = = = = = = = = = = 

int cmd = 0;
// ==========================================
long upmdil = 1000;           //speed control
long downmdil = 1000;
int t_correction = 1;       //calibration (sec)

long DIVER_DOWN = 5000000;         //rotation control
long DIVER_UP = 100000; //pulling length
long DIVER_ZERO = 1000; //distance down after stop

int DELAY_DIVER = 3000; //Set waiting time after going down to the bottom of the well
int OPERATION_TIME = 0;

int updown = 500;           //updown control, 500=10cm
// = = = = = = = = = = = = = = = = = = = = = = = = = 

int N = 0; //1 minute

RTC_DS3231 rtc;//Real Time Clock
int year =2022;
int Month = 12;
int Day = 20;
int Hour = 15;
int minute = 00;
int second = 00;
DateTime alarm;

void setup() 
{
  pinMode (relay, OUTPUT);
  pinMode (PUL, OUTPUT);
  pinMode (DIR, OUTPUT);
  pinMode (ENA, OUTPUT);
  pinMode (Endstop, INPUT_PULLUP);

  pinMode (RED, OUTPUT);
  pinMode (GREEN, OUTPUT);
  pinMode (BLUE, OUTPUT);

  Serial.begin(9600);
  

  rtc.begin();
  Serial.print("\n\n\n# # # # # # # # # # # #\n");
  Serial.print("HI-res SIM\n");
  Serial.print("# # # # # # # # # # # # # # # # # # # # # # # # # # #\n");
  Serial.print("# # # # # Total Integration CODE. 20221219 # # # # #\n \n \n");
  Serial.print("# # # # # # # # # # # # # # # # # # # # # # # # # # #\n");


  Serial.print("= = = = = = = = = = = = = = = = = = = = = = = \n");
  Serial.print("Initializing Hi-res SIM \n");
  //Serial.print("1. getting time data from connected pc... \n");

  
  delay(300); 
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //computer time synchronization
  DisplayTime();

}

// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = motor function
void goingDown() {
  digitalWrite(ENA,HIGH);
  digitalWrite(relay, HIGH);
  Serial.println("\n CRANK DOWN START ....");
  DateTime snow = rtc.now();
  DisplayTime();
  
  //delay(5000);  
  for (unsigned long i=0; i<=DIVER_DOWN; i++)   
  {

    digitalWrite(DIR,HIGH); 
    digitalWrite(PUL,HIGH);
    delayMicroseconds(downmdil); 
    digitalWrite(PUL,LOW);
    delayMicroseconds(downmdil); 

    DEPTH++;

    if (digitalRead(Endstop) == true)    
    {
      //DEPTH ==0;
      DateTime enow = rtc.now();
      int mtime = subtractTime(snow,enow);
      Serial.println("\nEndstop depth: "+ String(DEPTH)+" ,Time: "+String(mtime));
      //Serial.print("\nEndstop depth: ");
      //Serial.print(DEPTH);
      //Serial.print("\n");
      DisplayTime();
      cmd = 7;      
      break;
    } 
    
    if(cmd>2 && Serial.available())
    {
      DateTime enow = rtc.now();
      int mtime = subtractTime(snow,enow);
      Serial.println("\nNow depth: "+ String(DEPTH)+" ,Time: "+String(mtime));
      DisplayTime();
      break;
    }
  }
  delay(200);  
  digitalWrite(relay, LOW);
  digitalWrite(ENA,LOW);
}

//////////////////////////////// time function
void DisplayTime(){
  Serial.print("\n Now RTC Time: ");
  DateTime now = rtc.now();
  DateTime correct (now + TimeSpan(0,0,0,0)); 
  Serial.print(correct.hour());
            Serial.print(":");
            Serial.print(correct.minute());
            Serial.print(":");
            Serial.print(correct.second());
         
            Serial.print("\n");
}

int subtractTime(DateTime startT, DateTime endT) {

  int hourS = startT.hour();
  int minuteS = startT.minute(); 
  int secondS = startT.second();  
  int hourE = endT.hour();
  int minuteE = endT.minute(); 
  int secondE = endT.second();  
  return (hourE * 3600 + minuteE * 60 + secondE)-(hourS * 3600 + minuteS * 60 + secondS);
}

void goingUp() {
  digitalWrite(ENA,HIGH);
  digitalWrite(relay, HIGH);
  Serial.println("\n Crank UP START");
  DateTime snow = rtc.now();
  DisplayTime();

  
  //delay(200);  
  for (unsigned long i=0; i<=DIVER_UP; i++)   
  {

    digitalWrite(DIR,LOW); //LOW is up, HIGH is down
    digitalWrite(PUL,HIGH);
    delayMicroseconds(upmdil); //low --> fast
    digitalWrite(PUL,LOW);
    delayMicroseconds(upmdil); //low --> fast
    DEPTH--;

    if (digitalRead(Endstop) == true)    
    {
      //DEPTH ==0;
      DateTime enow = rtc.now();
      int mtime = subtractTime(snow,enow);
      Serial.println("\nEndstop depth: "+ String(DEPTH)+" ,Time: "+String(mtime));
      //Serial.print("\nEndstop depth: ");
      //Serial.print(DEPTH);
      //Serial.print("\n");      
      break;
    }   
    if(cmd>2 && Serial.available()){

      DateTime enow = rtc.now();
      int mtime = subtractTime(snow,enow);
      Serial.println("\nNow depth: "+ String(DEPTH)+" ,Time: "+String(mtime));
      digitalWrite(relay, LOW);
      digitalWrite(ENA,LOW);
      return;      
    }
  }
  
  delay(200);
  //정지 후 내려갈 양
  for (unsigned long i=0; i<=DIVER_ZERO; i++)   
  {
    digitalWrite(DIR,HIGH); //LOW is up, HIGH is down
    digitalWrite(PUL,HIGH);
    delayMicroseconds(downmdil); //low --> fast
    digitalWrite(PUL,LOW);
    delayMicroseconds(downmdil); //low --> fast
  }
  
  delay(200);  
  digitalWrite(relay, LOW);
  digitalWrite(ENA,LOW);
  DEPTH == 0;
  Serial.print("\n Move to Start point \n");
}
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 

void ParsingData(String message) {
  String inString(message);
  int first = inString.indexOf(",");// first comma
  int two = inString.indexOf(",",first+1); 
  int third = inString.indexOf(",",two+1); 
  int fourth = inString.indexOf(",",third+1); 
  int fifth = inString.indexOf(",",fourth+1); 
  int sixth = inString.indexOf(",",fifth+1); 
  int seventh = inString.indexOf(",",sixth+1);
  int length = inString.length(); // 문자열 길이
 
  String str1 = inString.substring(0, first); // first token(0, 3)
  String str2 = inString.substring(first+1, two); // second token (4, 7)
  String str3 = inString.substring(two+1,third); // third token(8, 10)
  String str4 = inString.substring(third+1,fourth); // fourth token(8, 10)
  String str5 = inString.substring(fourth+1,fifth); // fifth token(8, 10)
  String str6 = inString.substring(fifth+1,sixth); // sixth token(8, 10)
  String str7 = inString.substring(sixth+1,seventh); // seventh token(8, 10)
  String str8 = inString.substring(seventh+1,length); // end token(8, 10)
  cmd = str1.toInt();
  if(str1 !="0"){
    DIVER_DOWN = str2.toInt();         
    DIVER_UP = str3.toInt(); 
    DIVER_ZERO = str4.toInt(); 
    DELAY_DIVER = str5.toInt(); 
    OPERATION_TIME = str6.toInt(); 
    downmdil = str7.toInt();
    upmdil = str8.toInt();
  }else{
    year = str2.toInt();
    Month = str3.toInt();
    Day = str4.toInt();
    Hour = str5.toInt();
    minute =str6.toInt();
    second =str7.toInt();
  }
  
  if(str1 == "1" || str1=="2" || str1=="8"){
    Serial.println("Command: "+str1+"\n");   
    Serial.println("Diver_Down: "+String(DIVER_DOWN)+"\n");
    Serial.println("Diver_Up: "+String(DIVER_UP)+"\n");
    Serial.println("Diver_Zero: "+String(DIVER_ZERO)+"\n");
    Serial.println("Diver_Delay: "+String(DELAY_DIVER)+"\n");
    Serial.println("Diver_Time: "+String(OPERATION_TIME)+"\n");
    DisplayTime();
  }   
}

void SetInit(){
  DEPTH = 0;
}

void loop()
{
  DateTime now = rtc.now();
  int cTime = 60 - now.minute();
  DateTime correct (now + TimeSpan(0,0,cTime,0)); 

  String serialReadData;  

  if(cmd ==1 || cmd == 8){// mesurement
    if(now.minute() == alarm.minute() && alarm.hour() == now.hour() && alarm.day() == now.day()){
        DisplayTime();
        if(cmd == 8)goingUp();
        DateTime Tempalarm (now+TimeSpan(0,0,OPERATION_TIME,0));
        alarm = Tempalarm;
        goingDown();  
        delay(DELAY_DIVER);
        if(cmd==1 || cmd == 8)goingUp();
        DEPTH= 0;
    }
  }

  
  if(Serial.available()){
    serialReadData = Serial.readString();
    ParsingData(serialReadData);
    
    switch(cmd){
      case 0://Time Synchronize
        
        rtc.adjust(DateTime(year,Month,Day,Hour,minute,second));
        Serial.print("\n");
        DisplayTime();
        //Serial.println("Command: "+String(cmd)+"  innnnnn\n");
        break;
      case 1:// mesurement
        //alarm = now;
        alarm = correct;
        break;
      case 2:// Test.
        goingDown();  
        delay(DELAY_DIVER);
        if(cmd==2) goingUp();
        break;
      case 3:       
        Serial.println("Command: "+String(cmd)+"  innnnnn\n");
        DisplayTime();
        break;
      case 4:        
        goingDown();
        break;
      case 5:        
        Serial.println("\n Command: Diver STOP \n ");
        DisplayTime();
        break;
      case 6:        
        goingUp();
        break;
      case 7: /// Syste Init.        
        SetInit();
        Serial.println("\nInitialize.....\n");
        DisplayTime();
        break;
      case 8:   
        alarm = correct;
        break;
      default:        
        Serial.println("Command: "+String(cmd)+"  default\n");
        DisplayTime();
        break;
        
    }
  }
  
}
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = End
