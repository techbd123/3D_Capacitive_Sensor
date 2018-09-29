#include <Keypad.h>
#include <LiquidCrystal.h>
#include <string.h>
#define MAX 15

using namespace std;

int signalPin=4,activityPin=3,emergencyPin=2;

LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);

const byte ROW=4;
const byte COL=4;
byte rowPin[ROW]={13,12,11,10};
byte colPin[COL]={9,8,7,6};

char keys[ROW][COL]={
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'},
};

Keypad keypad=Keypad(makeKeymap(keys),rowPin,colPin,ROW,COL);

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

PassWord pw,p1,p2;

bool hlocked,emergencyState;

void setup()
{
  Serial.begin(115200);
  lcd.begin(16,2);
  
  hlocked=true;
  emergencyState=false;
  
  pinMode(signalPin,OUTPUT);digitalWrite(signalPin,LOW);pinMode(signalPin,INPUT);  
  pinMode(activityPin,OUTPUT);digitalWrite(activityPin,LOW);
  pinMode(emergencyPin,OUTPUT);digitalWrite(emergencyPin,LOW);
  
  
  p1.SetPassWord("123456"); //Password #1.
  
  pw.SetPassWord(p1.password);
}

char TakeInput()
{
  char key;
  while((key=keypad.getKey())==NO_KEY);
  return key;
}

int TakeString(char *a)
{
  int i=0;char key;lcd.setCursor(0,0);
  while((key=TakeInput())!='A')
  {
    if(key=='#')
    {
      if(i>0)
      {
        i--;
        lcd.rightToLeft();lcd.print(" ");lcd.leftToRight();lcd.print(" ");
        lcd.rightToLeft();lcd.print(" ");lcd.leftToRight();
      }
      continue;
    }
    lcd.print(key);
    a[i++]=key;
  }
  a[i]=0;
  return i;
}

void ShowShortLength(int msec)
{
  lcd.setCursor(0,1);lcd.print("SHORT PASSWORD! ");delay(msec);
  lcd.setCursor(0,1);lcd.print("LENGTH >= 4     ");delay((msec*3)/2);
  lcd.clear();
}

void ShowWrongPassword(int msec)
{
  lcd.setCursor(0,1);lcd.print("WRONG PASSWORD! ");delay(msec);lcd.clear();
}

void ShowActive()
{
  lcd.clear();lcd.setCursor(0,1);lcd.print("     ACTIVE     ");
}

void ShowInactive()
{
  lcd.clear();lcd.setCursor(0,1);lcd.print("   INACTIVE     ");
}

void ShowDone(int msec)
{
  lcd.clear();lcd.setCursor(0,1);lcd.print("     DONE!     ");delay(msec);lcd.clear();
}


void BlinkingText()
{
  lcd.display();delay(500);lcd.noDisplay();delay(500);
}

void ShowEmergency()
{
  lcd.setCursor(0,1);lcd.print("   EMERGENCY!   ");
  while(true)
  {
    BlinkingText();
  } 
}

void MakeInactive()
{
  digitalWrite(activityPin,HIGH);
  return ;
}

void MakeActive()
{
  digitalWrite(activityPin,LOW);
  return ;
}


void MakeEmergency()
{
  digitalWrite(emergencyPin,HIGH);
  emergencyState=true;
  return ;
}

void Checking()
{
  while(digitalRead(signalPin)!=HIGH&&keypad.getKey()==NO_KEY);
  return;
}

char str[MAX];
int WrongPasswords;

void loop()
{
  int len;
  
  if(emergencyState==true) ShowEmergency();
  
  if(hlocked==true)
  {
    ShowActive();
    int len=TakeString(str);
    if(len<4)
    {
      ShowShortLength(1000);
    }
    else
    {
      if(pw.hMatched(str)==true)
      {
        hlocked=false;
        MakeInactive();
        WrongPasswords=0;
        ShowDone(1000);
      }
      else
      {
        WrongPasswords++;
        if(WrongPasswords>2) //Consecutive 3 wrong passwords will generate emergency.
        {
          MakeEmergency();
        }
        ShowWrongPassword(1000);
      }
    }
  }
  else
  {
    ShowInactive();
    Checking();
    pw.hset=true;
    hlocked=true;
    MakeActive();
  } 
}
