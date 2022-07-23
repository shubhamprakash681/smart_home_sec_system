// Smart Home Security System
// by:
// Faisal Rahman
// Shimaila Khan
// Shubham Prakash
// Bikram Kumar Vivek
// Stanzin Dolkar


#include <math.h>
#include <Servo.h>
#include <LiquidCrystal.h>  //ilbrary for LCD
#include <SoftwareSerial.h>   //library to create software serial port
int rs=8;
int en=9;
int d4=10;
int d5=11;
int d6=12;
int d7=13;
LiquidCrystal lcd(rs,en,d4,d5,d6,d7);  //lcd definition
int rxd=6;
int txd=7;
SoftwareSerial mySerial(rxd,txd);  //serial port definition
Servo myServo; //Servo definition
Servo extServo;  //extinguisherServo definition
int tempPin=A0;
int gasPin=A1;
int firePin=A2;
int ldrPin=A3;
int pir1Pin=A4;
int pir2Pin=A5;

int fanPin=2;
int extPin=3;
int lightPin=4;
int servoPin=5;


//reference vslues:
float tMax=40.0;
float pir1Pos=60.0, pir2Pos=120.0;  //positions of pir sensors
//int lVal=10;  //eqvlnt. to logic LOW
//int uVal=1000;  //eqvlnt. to logic HIGH
int gMax=150,fMax=50,lMin=510,lMax=750,p1Max=100,p2Max=100;  //VVI change these accdng. to hardware

int faultDetected=0;
float tempData,tempC,tempF, servoPos=10.0, extPos=10.0;
int gasV,fireV,ldrV,pir1V,pir2V;

void sendMessage(); //gsm msg definitions (VVI insert all)
void fireAlertSms();
void gasAlertSms();
void motionAlertSms();

void setup() {
  mySerial.begin(9600);  //serial port started
  Serial.begin(9600);  //arduino serial monitor started
  lcd.begin(16,2);  //lcd started
  pinMode(tempPin, INPUT);
  pinMode(gasPin, INPUT);
  pinMode(firePin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(pir1Pin, INPUT);
  pinMode(pir2Pin, INPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(extPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(servoPin, OUTPUT);
  
  myServo.attach(servoPin);
  extServo.attach(extPin);
  digitalWrite(fanPin, LOW);
  //digitalWrite(lightPin, LOW);
  myServo.write(servoPos);
  delay(150);
  extServo.write(extPos);
  delay(150);

  lcd.setCursor(1,0);
  lcd.print("HOME SECUIRITY");
  lcd.setCursor(5,1);
  lcd.print("SYSTEM");
  delay(3000);
  lcd.clear();  //to print project name on LCD
}

void loop() { //scannig values of all parameters
  tempData = analogRead(tempPin);
  tempC = thermistor(tempData);
  tempF = (tempC*9.0)/5.0+32.0;
  delay(1000);
  
  gasV = analogRead(gasPin);
  fireV = analogRead(firePin);
  ldrV = analogRead(ldrPin);
  pir1V = analogRead(pir1Pin);
  pir2V = analogRead(pir2Pin);
  
  //for temperature sensor
  if (tempC>tMax) {
    faultDetected=1;
    //no fire
    digitalWrite(fanPin, HIGH);
    lcd.setCursor(0,0);
    lcd.print("Room Temp. is");
    lcd.setCursor(0,1);
    lcd.print("HIGH");
    delay(4000);
    lcd.clear();
    
    if (fireV<fMax) {  //fire
      fireAlertSms(); //uncomment it later
      lcd.setCursor(0,0);
      lcd.print("FIRE!! FIRE!!");
      lcd.setCursor(3,1);
      lcd.print("FIRE!!!");
      delay(5000);
      gasV=gMax+10;
      
      extPos = 90.0;
      extServo.write(extPos);
      delay(150);
    }
  }

  //for fire sensor
  if (fireV<fMax) {
    faultDetected=1;
    fireAlertSms(); //uncomment it later
      digitalWrite(fanPin, HIGH);
      extPos = 90.0;
      extServo.write(extPos);
      delay(150);
      lcd.setCursor(0,0);
      lcd.print("FIRE!! FIRE!!");
      lcd.setCursor(3,1);
      lcd.print("FIRE!!!");
      delay(5000);
      lcd.clear();
      gasV=gMax+10;
  }

  //for GAS sensor
  if (gasV>gMax) {
    faultDetected=1;
    gasAlertSms(); //uncommment it later
    lcd.setCursor(0,0);
    lcd.print("Potential");
    lcd.setCursor(0,1);
    lcd.print("Gas Leak!!!");
    delay(4000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("EVACUATE");
    lcd.setCursor(0,1);
    lcd.print("The Rooom!");
    delay(4000);
    lcd.clear();
    
  }

  //for PIR sensors
  if (pir1V<p1Max || pir2V<p2Max) {
    faultDetected=1;
    motionAlertSms(); //uncommment it later
    lcd.setCursor(0,0);
    lcd.print("MOTION");
    lcd.setCursor(0,1);
    lcd.print("DETECTED");
    if (pir1V<p1Max) {
      servoPos = pir1Pos;
      myServo.write(servoPos);
      delay(5000);
      myServo.write(0);
    }
    if (pir2V<p2Max) {
      servoPos = pir2Pos;
      myServo.write(servoPos);
      delay(5000);
      myServo.write(0);
    }
  }
  

  if (faultDetected == 0) { //no fault so LET PRINTING values
    digitalWrite(fanPin, LOW);
    digitalWrite(extPin, LOW);
    digitalWrite(servoPin, LOW);
    lcd.setCursor(0,0); //temp.
    lcd.print("Temperature is:");
    lcd.setCursor(0,1);
    lcd.print(tempC);
    lcd.print("C  ");
    lcd.print(tempF);
    lcd.print("F");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);  //gas conc.
    lcd.print("Smoke Conc. is:");
    lcd.setCursor(6,1);
    lcd.print("GOOD");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);  //fire status
    lcd.print("Fire is NOT");
    lcd.setCursor(0,1);
    lcd.print("detected");
    delay(2000);
    lcd.clear();
    if (ldrV>lMax) {    //light intensity
      lcd.setCursor(0,0);
      lcd.print("DAY Time");
      digitalWrite(lightPin, LOW);
      delay(2000);
      lcd.clear();
    }
    else if (ldrV<lMin) {
      lcd.setCursor(0,0);
      lcd.print("NIGHT Time");
      digitalWrite(lightPin, HIGH);
      delay(2000);
      lcd.clear();
    }
    lcd.setCursor(0,0);  //motion status
    lcd.print("NO Motion");
    lcd.setCursor(0,1);
    lcd.print("Detected");
    delay(2000);
    lcd.clear();
    
  }

  faultDetected=0;


}

float thermistor(float tempData) {
  float temp, tempC;
  temp=log(10000.0*((1024.0/tempData-1)));
  temp=1/(0.001129148+(0.000234125+(0.0000000876741*temp*temp))*temp);
  temp=temp-273.15;
  tempC = temp;
  return tempC;
}


void sendMessage()
{
Serial.println("I am in send");
mySerial.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
delay(1000); // Delay of 1sec to let the module setup
mySerial.println("AT+CMGS=\"+916299783192\"\r"); //Mobile no.
delay(1000);
mySerial.println("Excess Gas Detected. Open Windows");// SMS
delay(100);
mySerial.println((char)26);// ASCII code of CTRL+Z
delay(1000);
}

void fireAlertSms() {
  Serial.println("I am in fireAlert send");
  mySerial.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000); // Delay of 1sec to let the module setup
  mySerial.println("AT+CMGS=\"+916299783192\"\r"); //Mobile no.
  delay(1000);
  mySerial.println("FIRE!! FIRE!! FIRE!!");// SMS
  mySerial.println("  !!Attention!!  ");
  mySerial.println("FIRE in the Room");
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}

void gasAlertSms() {
  Serial.println("I am in gasAlert send");
  mySerial.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000); // Delay of 1sec to let the module setup
  mySerial.println("AT+CMGS=\"+916299783192\"\r"); //Mobile no.
  delay(1000);
  mySerial.println("!!POTENTIAL GAS LEAK FOUND!!");// SMS
  mySerial.println("  !!Attention!!  ");
  mySerial.println("Evacuate the Room");
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}

void motionAlertSms() {
  Serial.println("I am in motionAlert send");
  mySerial.println("AT+CMGF=1"); //Sets the GSM Module in Text Mode
  delay(1000); // Delay of 1sec to let the module setup
  mySerial.println("AT+CMGS=\"+916299783192\"\r"); //Mobile no.
  delay(1000);
  mySerial.println("!!Attention !! Attention!!");
  mySerial.println("!!UNAUTHORISED MOTION DETECTED!!");// SMS
  delay(100);
  mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}
