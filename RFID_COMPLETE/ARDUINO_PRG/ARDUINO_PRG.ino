/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to setup blocks on a MIFARE Classic PICC (= card/tag)
 * to be in "Value Block" mode: in this mode the operations Increment/Decrement,
 * Restore and Transfer can be used.
 * 
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 * 
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 */

#include "EASYRFID.h"

#define NUMSECTOR         15  

unsigned long times,timechange;
byte numtone;
//const byte pininterrupt = 2;
//const byte psi1 = 7;
//const byte psi2 = 6;
//const byte outin = 2;
const byte ENABLECHANGE = 4;
ESYRFID rfid;
int lastpate = 600;
int mms;
int maxb = 694;

void setup() {
//pinMode(outin,INPUT);
//pinMode(psi1,OUTPUT);
analogReference(INTERNAL);
pinMode(ENABLECHANGE,OUTPUT);
digitalWrite(ENABLECHANGE,HIGH);
Serial.begin(115200);
rfid.RFID_Begin();
rfid.setnowmalkey();
hitone();
rfid.lotone();
Serial.println("READY");
}

void checkbaterychange(){
  int a = analogRead(A0);
  int b = analogRead(A1);  
  lastpate = (lastpate*10+a)/11;// cho zui
  if(lastpate!=mms){
    timechange = millis();
   // Serial.println("change\n\n\n");
  }
  mms = a = lastpate;
  Serial.print("btr:");
  int a1 = map(a,530,maxb,0,100);
  Serial.print(a1);
  if(digitalRead(ENABLECHANGE) == HIGH){
    if(b>500){
    Serial.print("+");
    }
  }  
  Serial.print("%");
  Serial.println(a);
  if(a>670){ //stopchangebatery ~4.1v
  if((unsigned long)(millis() - timechange) > 600000){ //~5m
  maxb = lastpate;
  digitalWrite(ENABLECHANGE, LOW);
  }
  }   
  if(a<665){
  digitalWrite(ENABLECHANGE, HIGH);
  Serial.println(F("LOWbatery"));
  }
  if(b<500){
  Serial.println(F("USINGbatery"));
  }else{
   Serial.println(F("USINGUSB")); 
  }
}

void loop() {
  if ((unsigned long)(millis() - times) > 10000){
    times = millis();
    checkbaterychange();
  }
  if(!rfid.wait_for_new_card())return;
  Serial.println(rfid.Read(NUMSECTOR));
  hitone();
}

void hitone(){ // not hit one
  tone(8,2333,70);
}

void serialEvent(){

 String Sdata = Serial.readStringUntil('\r');
  Serial.flush();

  if(Sdata.indexOf("SETPOWEROUTPUT")!= -1){
    byte num = byte(Sdata[Sdata.indexOf("SETPOWEROUTPUT:")+15])-48;
    Serial.print("set power out put to ");
    Serial.println(num);
    rfid.setpoweroutput(num);
    hitone();
  }

  if (Sdata.indexOf("CHECKSYS") != -1){
      if(rfid.checksys()){Serial.println("OK");} else{ rfid.lotone(); Serial.print("FALSE");}
      hitone();
  } 

  if (Sdata.indexOf("WRITE:") != -1) {/// NEWCARDreWRITE:12394612879461278689#
    boolean NEWkey;
    boolean OLDkey;
    if(Sdata.indexOf("reWRITE") != -1){NEWkey=true;} else NEWkey = false; //chose key output nowmal is true 
    if(Sdata.indexOf("OLDCARD") != -1){OLDkey=true;} else OLDkey = false;
    WRITEDATA(Sdata,NEWkey,OLDkey);
  }

  if (Sdata.indexOf("CHANGEREADMODE:") != -1) {
    if(Sdata.indexOf("1") != -1){READMODE=true;}
    if(Sdata.indexOf("0") != -1){READMODE=false;}
    Serial.print("readmode_change_to:");
    Serial.print(READMODE);
    hitone();
  }

  if (Sdata.indexOf("!!LOCKCARD!!") != -1){
    boolean key;
    if(Sdata.indexOf("SECURITY") != -1){key=true;}else{key=false;}
    LOCK(key);
  }

}

void WRITEDATA(String &Sdata,boolean NEWkey,boolean OLDkey){
  String writes;
  hitone(); 
      for (int i = Sdata.indexOf("WRITE:") + 6; i < Sdata.indexOf("#"); i++) {
      writes = writes + Sdata[i];
      }
      writes = writes+"#";
      Serial.println(F("inputcard..."));
      while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) { // waitting for new card
      if (Serial.available()) { 
         String outdatas = Serial.readStringUntil('\r');
         if (outdatas.indexOf("cancle") != -1) {
        Serial.print("EXIT");
        rfid.Stop();
        return;
        }
      }
      }
      if(!rfid.Writedata(NUMSECTOR,writes,NEWkey,OLDkey)){rfid.Stop();rfid.lotone(); Serial.print("WRITE_DATA_FALSE");return;}
      hitone(); 
}

void LOCK(boolean key){
  hitone();
  Serial.println(F("inputcard..."));
  while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      if (Serial.available()) { 
        String outdatas = Serial.readStringUntil('\r');
        if (outdatas.indexOf("cancle") != -1) {
        Serial.print("EXIT"); 
        rfid.Stop(); 
        return; 
        }
      } 
      }
  if(!rfid.Locksector(NUMSECTOR,key)){Serial.print("LOCK_FALSE");rfid.lotone();return;}
  hitone();
}
    
/**
 * Ensure that a given block is formatted as a Value Block.
 */


