#include <Wire.h>
#include <DS3231.h>
DS3231 clock;

#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

const int numberOfInputs = 3;
//const int inputPins[numberOfInputs] = {8, 9};
const int inputPins[numberOfInputs] = {14, 15, 16};

//Input Logic
int inputState[numberOfInputs];
int lastInputState[numberOfInputs] = {LOW, LOW};
bool inputFlags[numberOfInputs] = {LOW, LOW};
long lastDebounceTime[numberOfInputs] = {0,0};
long debounceDelay = 10;
bool inputInit = true;

//Menu Logic
const int numberOfScreens = 6;
int currentScreen = 0;
String screens[numberOfScreens][2] = 
{
  {"Exit",""}, 
  {"Set Minutes",""}, 
  {"Set Hours",""}, 
  {"Set Day",""}, 
  {"Set Month",""}, 
  {"Set Year",""}
};

int parameters[numberOfScreens];

//Menu Options
const int menu_exit = 0;
const int menu_set_minutes = 1;
const int menu_set_hours = 2;
const int menu_set_day = 3;
const int menu_set_month = 4;
const int menu_set_year = 5;

//Modes
int mode = 1;
const int mode_main = 0;
const int mode_menu = 1;
const int mode_set_year = 2;
const int mode_set_month = 3;
const int mode_set_day = 4;
const int mode_set_hours = 5;
const int mode_set_minutes = 6;

bool screenUpdateRequired = false;

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Setup"));

  clock.begin();
  
  for(int i=0; i < numberOfInputs; i++)
  {
    Serial.print(F("setup: input: "));
    Serial.print(i); 
    Serial.print(F(" Setting to PIN: "));
    Serial.println(inputPins[i]); 

    pinMode(inputPins[i], INPUT);
    digitalWrite(inputPins[i], HIGH);
  }

  lcd.begin(16,2);
  lcd.print("SETUP");
  lcd.setCursor(0,1);
  lcd.print("setup");
}

void loop()
{ 
  setInputFlags();
  resolveInputFlags();
}

void setInputFlags()
{
  for(int i=0;i<numberOfInputs;i++)
  {
    int reading = digitalRead(inputPins[i]);
//    Serial.print(F("setInputFlags: Input "));
//    Serial.print(i);
//    Serial.print(F(" reading "));
//    Serial.println(reading);
    if(reading != lastInputState[i])
    {
      Serial.println(F("setInputFlags: Setting last debounce"));
      lastDebounceTime[i] = millis();
    }
  
    if((millis() - lastDebounceTime[i]) > debounceDelay)
    {
      if(reading != inputState[i])
      {
        Serial.print(F("setInputFlags: Setting input state for input: "));
        Serial.print(i);
        Serial.print(F(" to "));
        Serial.println(reading);
        inputState[i] = reading;
      
        if(inputState[i] == HIGH)
        {
          inputFlags[i] = HIGH;
        }
      }
    }
  
    lastInputState[i] = reading;
  }
}

void resolveInputFlags()
{
  //Serial.print(F("resolveInputFlags: MODE="));
  //Serial.println(mode);
  
  if(mode == mode_main)
  {
    screenUpdateRequired = true;
  }
  
  for(int i = 0;i<numberOfInputs;i++)
  {
    if(inputFlags[i] == HIGH)
    {
      Serial.print(F("resolveInputFlags: input "));
      Serial.print(i);
      Serial.println(F(" is HIGH"));

      Serial.println(F("resolveInputFlags: Processing input action"));
      inputAction(i);

      Serial.println(F("resolveInputFlags: Setting input flag low."));
      inputFlags[i] = LOW;

      Serial.println(F("resolveInputFlags: Screen update set to required."));
      screenUpdateRequired = true;
    }
  }

  updateScreenIfRequired();
}

void inputAction(int input)
{
  Serial.print(F("inputAction: input:"));
  Serial.print(input);
  Serial.print(F(" mode: "));
  Serial.println(mode);
  
  if(input == 0)
  {
    if(inputInit)
    {
      Serial.println(F("inputInit is set, ignoring input 0"));
      return;
    }else if(mode == mode_menu)
    {
      showNextMenu();
    }else if(mode == mode_set_year || mode == mode_set_month || mode == mode_set_day || mode == mode_set_hours || mode == mode_set_minutes)
    {
      mode = mode_menu;
    }
    else if(mode == mode_main)
    {
      mode = mode_menu;
    }
  }
  else if(input == 1)
  {
    if(inputInit)
    {
      Serial.println(F("inputInit is set, ignoring input 1"));
      return;
    }
    else if(mode == mode_menu)
    {
      showMenuItem();
    }
    else if(mode == mode_set_year || mode == mode_set_month || mode == mode_set_day || mode == mode_set_hours || mode == mode_set_minutes)
    {
      incrementDate(mode);
    }
  }
  else if (input == 2)
  {
    if(inputInit)
    {
      Serial.println(F("inputInit is set, ignoring input 2"));
      inputInit = false;
      return;
    }
    else if(mode == mode_set_year || mode == mode_set_month || mode == mode_set_day || mode == mode_set_hours || mode == mode_set_minutes)
    {
      decrementDate(mode);
    }
  }
}

void incrementDate(int mode)
{
  RTCDateTime rtc = clock.getDateTime();
  
  int year = mode == mode_set_year ? rtc.year + 1 : rtc.year;
  int month = mode == mode_set_month ?  rtc.month + 1 : rtc.month;
  int day = mode == mode_set_day ? rtc.day + 1 : rtc.day;
  int hours = mode == mode_set_hours ? rtc.hour + 1 : rtc.hour;
  int minutes = mode == mode_set_minutes ? rtc.minute + 1 : rtc.minute;
  int seconds = rtc.second;
  
  clock.setDateTime(year, month, day, hours, minutes, seconds);
}

void decrementDate(int mode)
{
  RTCDateTime rtc = clock.getDateTime();
  
  int year = mode == mode_set_year ? rtc.year - 1 : rtc.year;
  int month = mode == mode_set_month ?  rtc.month - 1 : rtc.month;
  int day = mode == mode_set_day ? rtc.day - 1 : rtc.day;
  int hours = mode == mode_set_hours ? rtc.hour - 1 : rtc.hour;
  int minutes = mode == mode_set_minutes ? rtc.minute -1 : rtc.minute;
  int seconds = rtc.second;
  
  clock.setDateTime(year, month, day, hours, minutes, seconds);
}

void showNextMenu()
{
  if(currentScreen == menu_exit)
  {
    currentScreen = numberOfScreens-1;
  }
  else
  {
    currentScreen--;
  }
}

void showMenuItem()
{
  if(currentScreen == menu_exit)
  {
    mode = mode_main;
  }
  else if(currentScreen == menu_set_year)
  {
    mode = mode_set_year;
  }
  else if(currentScreen == menu_set_month)
  {
    mode = mode_set_month;
  }
  else if (currentScreen == menu_set_day)
  {
    mode = mode_set_day;
  }
  else if (currentScreen == menu_set_hours)
  {
    mode = mode_set_hours;
  }
  else if (currentScreen == menu_set_minutes)
  {
    mode = mode_set_minutes;
  }
}

void updateScreenIfRequired()
{
  if(screenUpdateRequired == true)
  {
    Serial.print(F("updateScreenIfRequired: MODE: "));
    Serial.println(mode);
    if(mode == mode_main)
    {
      Serial.println(F("updateScreenIfRequired: MAIN MODE: "));
      //https://github.com/jarzebski/Arduino-DS3231
      String line1 = clock.dateFormat("d/m/y H:i:s",  clock.getDateTime());
      //String line1 = "CLOCK";
      String line2 = F("Hours Remaining:");

      printScreen(line1, line2);
      delay(1000);
    }
    else if(mode == mode_menu)
    {
      Serial.println(F("updateScreenIfRequired: MENU MODE"));
      Serial.println(F("updateScreenIfRequired: Updating Screen"));
      String line1 = screens[currentScreen][0];
      Serial.print(F("updateScreenIfRequired: Line1: "));
      Serial.println(line1);
      String line2 = screens[currentScreen][1];
      Serial.print(F("updateScreenIfRequired: Line2: "));
      Serial.println(line2);
      Serial.println(F("Printing screen"));
      lcd.clear();
      printScreen(line1, line2);
    }
    else if(mode == mode_set_year || mode == mode_set_month || mode == mode_set_day || mode == mode_set_hours || mode == mode_set_minutes)
    {
      displayDateEditingComponent(mode);
    }
    
    screenUpdateRequired = false;
  }
}

void displayDateEditingComponent(int mode)
{
  lcd.clear();
  RTCDateTime rtc = clock.getDateTime();
  if(mode == mode_set_year)
  {
    printScreen(String(rtc.year),"");
  }
  else if(mode == mode_set_month)
  {
    printScreen(String(rtc.month),"");
  }
  else if(mode == mode_set_day)
  {
    printScreen(String(rtc.day),"");
  }
  else if(mode == mode_set_hours)
  {
    printScreen(String(rtc.hour),"");
  }
  else if(mode == mode_set_minutes)  
  {
    printScreen(String(rtc.minute),"");
  }
  
}

void printScreen(String line1, String line2)
{ 
  lcd.setCursor(0,0);  
  lcd.print(line1);
  lcd.setCursor(0,1);
  lcd.print(line2);
}
