  #include "pitches.h"
#define resolution 8
#define mains 50 // 60: north america, japan; 50: most other places

#define refresh 2*1000000/mains

typedef long long ll;

int lockPin=A5,emergencyPin=A4,activityPin=A3,speakerPin=A1,ledPin=A0;
int pins[3]={9, 10, 11};

int melody[]={NOTE_B4, 0, NOTE_B4, 0, NOTE_B4, 0, NOTE_B4, 0};
int noteDurations[]={4, 4, 4, 4, 4, 4, 4, 4};

bool hAlarm;

struct Coordinate
{
  long x,y,z;
  ll sumX,sumY,sumZ,counts;
  void Initialize()
  {
    sumX=sumY=sumZ=counts=0;
    return ;
  }
  void TakeValues(long xx,long yy,long zz)
  {
    sumX+=xx;sumY+=yy;sumZ+=zz;
    counts++;
    x=sumX/counts;y=sumY/counts;z=sumZ/counts;
    return ;
  }
}positions;

void setup()
{
  Serial.begin(115200);

  // unused pins are fairly insignificant,
  // but pulled low to reduce unknown variables
  for (int i = 2; i < 20; i++)
  {
    pinMode(i,OUTPUT);
    digitalWrite(i,LOW);
  }

  for (int i = 0; i < 3; i++) pinMode(pins[i],INPUT);
  
  pinMode(activityPin,INPUT);
  pinMode(emergencyPin,INPUT);

  startTimer();

  hAlarm=false;
  
  positions.Initialize();
  
  delay(500);
}

void Alarm(int pin,int notes)
{
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < notes; thisNote++)
  {
    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    digitalWrite(ledPin,HIGH);
    tone(pin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    int pauseBetweenNotes = noteDuration;
    delay(pauseBetweenNotes/2);
    digitalWrite(ledPin,LOW);
    delay(pauseBetweenNotes/2);
  }
  return ;
}

bool InRange(long a,long b,long c)
{
  return a<=b&&b<=c;
}

void RunAlarm()
{
  Alarm(speakerPin,8);
  return ;
}

void GettingPosition()
{
  long x=time(pins[0],B00000010);//Serial.print("");Serial.print(x);
  long y=time(pins[1],B00000100);//Serial.print(" ");Serial.print(y);
  long z=time(pins[2],B00001000);//Serial.print(" ");Serial.println(z);
  positions.TakeValues(x,y,z);
  return ;
}

void SetLockSignal()
{
  hAlarm=true;
  digitalWrite(lockPin,HIGH);
  return ;
}

int ePinState=0,ePinCount=0,nchecks=10;

void CheckEmergencyPin()
{
  ePinState+=digitalRead(emergencyPin);
  ePinCount++;
  return ;
}

bool IsEmergency()
{
  bool state=ePinState==ePinCount&&ePinCount>=nchecks-1;
  if(ePinCount==nchecks)
  {
    ePinState=ePinCount=0;
  }
  return state;
}

bool IsActive()
{
  return digitalRead(activityPin)==LOW;
}

void loop()
{
  Up:
  if(hAlarm)
  {
    RunAlarm();
    Serial.println("Alarm!");
  }
  else
  {
    CheckEmergencyPin();
    if(IsEmergency())
    {
      SetLockSignal();
      goto Up;
    }
    if(IsActive())
    {
      GettingPosition();
      if(positions.counts==5)
      {
        Serial.print("");Serial.print(positions.x);
        Serial.print(" ");Serial.print(positions.y);
        Serial.print(" ");Serial.println(positions.z);
        if(InRange(5600,positions.x,5800)&&InRange(6550,positions.y,6650)&&InRange(6850,positions.z,100000)) SetLockSignal();
        positions.Initialize();
      }
    }
    else Serial.println("***Inactive***");
  }
}

long time(int pin, byte mask)
{
  unsigned long count = 0, total = 0;
  startTimer();
  while (checkTimer() < refresh)
  {
    // pinMode is about 6 times slower than assigning
    // DDRB directly, but that pause is important
    pinMode(pin, OUTPUT);
    PORTB = 0;
    pinMode(pin, INPUT);
    while((PINB & mask) == 0)
    {
      count++;
    }
    total++;
  }
  //Serial.println();Serial.print("count = ");Serial.print(count);
  //Serial.print(", total = ");Serial.println(total);
  return (count << resolution)/total;
}

extern volatile unsigned long timer0_overflow_count;

void startTimer()
{
  timer0_overflow_count = 0;
  TCNT0 = 0;
}

unsigned long checkTimer()
{
  return ((timer0_overflow_count << 8) + TCNT0) << 2;
}
