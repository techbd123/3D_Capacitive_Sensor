#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <string.h>
#define MAX 15

using namespace std;


int lockPin=9,signalPin=8,servoPin=5,servo1Pin=4;
int initial_angle=15;

Servo servo,servo1;

LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);
LiquidCrystal lcd1(A8,A9,A10,A11,A12,A13);

const byte ROW=4;
const byte COL=4;
byte rowPin[ROW]={36,34,32,30};
byte colPin[COL]={28,26,24,22};

byte rowPin1[ROW]={37,35,33,31};
byte colPin1[COL]={29,27,25,23};

char keys[ROW][COL]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};

Keypad keypad=Keypad(makeKeymap(keys),rowPin,colPin,ROW,COL);
Keypad keypad1=Keypad(makeKeymap(keys),rowPin1,colPin1,ROW,COL);

struct PassWord
{
  char password[MAX];
  int len;bool hset;
  PassWord()
  {
    hset=false;
  }
  void SetPassWord(char *a)
  {
    for(len=0;a[len];len++) password[len]=a[len];
    hset=true;
  }
  bool hMatched(char *a)
  {
    int i;
    for(i=0;i<len;i++)
    {
      if(password[i]==a[i]);else return false;
    }
    if(a[i]) return false;
    return true;
  }
  void RemovePassword()
  {
    hset=false;
  }
};

PassWord pw,pw1,p1,p2,p3,p4;

bool hlocked;


void InitializeServo(Servo ser,int pin,int angle)
{
  ser.attach(pin);
  ser.write(angle);
  return ;
}

void setup()
{
  Serial.begin(115200);
  lcd.begin(16,2);lcd.clear();
  lcd1.begin(16,2);lcd1.clear();
  for(int i=2;i<70;i++)
  {
    if(13<i&&i<22) continue;
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }
  
  pinMode(lockPin,INPUT);
  
  InitializeServo(servo,servoPin,initial_angle);
  InitializeServo(servo1,servo1Pin,initial_angle);
  
  hlocked=true;
  p1.SetPassWord("111111"); //Password #1
  p2.SetPassWord("222222"); //Password #2
  p3.SetPassWord("333333"); //Password #3
  p4.SetPassWord("444444"); //Password #4
  pw.SetPassWord(p1.password);
  pw1.SetPassWord(p2.password);
}

struct CharValue
{
  char a,b;
};

struct CharValue TakeInput()
{
  char key,key1;CharValue inChar;
  while((key=keypad.getKey())==NO_KEY&&(key1=keypad1.getKey())==NO_KEY);
  inChar.a=key;
  inChar.b=key1;
  return inChar;
}

int alen=0,blen=0;

int ProcessTakenChar(char *str,int pos,char ch,LiquidCrystal lcd)
{
  if(ch=='#')
  {
    if(pos>0)
    {
      pos--;
      lcd.rightToLeft();lcd.print(" ");lcd.leftToRight();lcd.print(" ");
      lcd.rightToLeft();lcd.print(" ");lcd.leftToRight();
    }
    return pos;
  }
  lcd.print(ch);
  str[pos++]=ch;
  return pos;
}

int TakeString(char *a,char *b)
{
  char key;CharValue inChar;
  int state=0;lcd.setCursor(0,0);lcd1.setCursor(0,0);
  while(!state)
  {
    inChar=TakeInput();
    if(inChar.a!=NO_KEY)
    {
      if(inChar.a=='A')
      {
        a[alen]=0;
        alen=0;
        state=1;
      }
      else alen=ProcessTakenChar(a,alen,inChar.a,lcd);
    }
    if(inChar.b!=NO_KEY)
    {
      if(inChar.b=='A')
      {
        b[blen]=0;
        blen=0;
        if(state) state=3;else state=2;
      }
      else blen=ProcessTakenChar(b,blen,inChar.b,lcd1);
    }
  }
  return state;
}


void ShowShortLength(LiquidCrystal lcd,int msec)
{
  lcd.setCursor(0,1);lcd.print(" Short Password!");delay(msec);
  lcd.setCursor(0,1);lcd.print("   Length >= 4  ");delay((msec*3)/2);
  lcd.clear();
}

void ShowWrongPassword(LiquidCrystal lcd,int msec)
{
  lcd.setCursor(0,1);lcd.print(" Wrong Password!");delay(msec);lcd.clear();
}

void ShowLocked(LiquidCrystal lcd)
{
  lcd.setCursor(0,1);lcd.print(" Enter Password ");
}

void ShowUnlocked(LiquidCrystal lcd)
{
  lcd.clear();
  lcd.setCursor(0,1);lcd.print("   UnLocked!    ");delay(500);
  lcd.setCursor(0,1);lcd.print("   Opening...   ");
}


void MakeSignalToActivate()
{
  digitalWrite(signalPin,HIGH);delay(100);
  digitalWrite(signalPin,LOW);
  return ;
}

void RunServo(Servo ser)
{
  ser.write(min(initial_angle+120,170));
  delay(3000);
  ser.write(initial_angle);
  delay(1000);
}


bool CheckLockPin()
{
  int i;bool state;
  for(i=0,state=true;i<5;i++) state=state&&digitalRead(lockPin);
  return state;
}

char insideStr[MAX],outsideStr[MAX];

bool CheckPassword(char *str,struct PassWord pw,LiquidCrystal lcd)
{
  int len=strlen(str);
  if(len<4)
  {
    if(!len&&pw.len&&hlocked==false)
    {
      pw.hset=true;
      hlocked=true;
    }
    else ShowShortLength(lcd,1000);
    return false;
  }
  if(pw.hset==false)
  {
    pw.SetPassWord(p1.password);
    hlocked=true;
  }
  if(pw.hMatched(str)==true) return true;
  ShowWrongPassword(lcd,1000);
  return false;
}

void loop()
{
  if(CheckLockPin()) pw.SetPassWord(p3.password), pw1.SetPassWord(p4.password);
  
  lcd.clear();
  if(hlocked==true)
  {
    ShowLocked(lcd);
    ShowLocked(lcd1);
  }
  else
  {
    ShowUnlocked(lcd);
    ShowUnlocked(lcd1);
  }
  int state=TakeString(outsideStr,insideStr);
  
  if(CheckLockPin()) pw.SetPassWord(p3.password), pw1.SetPassWord(p4.password);
  
  if(state==1)
  {
    if(CheckPassword(outsideStr,pw,lcd))
    {
      ShowUnlocked(lcd);
      RunServo(servo);
      alen=0;
    }
  }
  else if(state==2)
  {
    if(CheckPassword(insideStr,pw1,lcd1))
    {
      ShowUnlocked(lcd1);
      RunServo(servo1);
      blen=0;
      MakeSignalToActivate();
    }
  }
  else
  {
    if(CheckPassword(outsideStr,pw,lcd))
    {
      ShowUnlocked(lcd);
      RunServo(servo);
      alen=0;
    }
    if(CheckPassword(insideStr,pw1,lcd1))
    {
      ShowUnlocked(lcd1);
      RunServo(servo1);
      blen=0;
      MakeSignalToActivate();
    }
  }
}
