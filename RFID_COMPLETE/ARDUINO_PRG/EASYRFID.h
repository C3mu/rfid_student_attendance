/*
 * this library write by cemu
 * to easyer readdata from rfid module only PICC_TYPE_MIFARE_1K
 * for more infomation contact "cemucume@gmail.com"
 * made in vietnam :D
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
#ifndef EASYRFID_H
#define EASYRFID_H

#include "Arduino.h"
#include "String.h"
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN         9       
#define SS_PIN          10 

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::MIFARE_Key Newkey;
MFRC522::StatusCode status;					// for error report
byte buffer[18];							// for faster
byte size = sizeof(buffer);
byte cardID[4];
boolean READMODE;// dox bg key moi hay key cu

class ESYRFID
{
  public:

  RFID_Begin(){
     SPI.begin();
     mfrc522.PCD_Init();
     mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_38dB);
     return;
    }
    
  void setpoweroutput(byte num){
    mfrc522.PCD_Init();
    switch(num){
           case 1: mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_18dB);
                    break;// 010b - 18 dB, it seems 010b is a duplicate for 000b
           case 2: mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_23dB);
                    break;// 011b - 23 dB, it seems 011b is a duplicate for 001b
           case 3: mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_33dB); 
                    break;// 100b - 33 dB, average, and typical default
           case 4: mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_38dB); 
                    break;// 101b - 38 dB
           case 5: mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_43dB);  
                    break;// 110b - 43 dB
           case 6: mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_48dB);   
                    break;// 111b - 48 dB, maximum    
    }
    return;
  }
  
  byte wait_for_new_card(void){
    if ( !mfrc522.PICC_IsNewCardPresent()) return false;
    if ( !mfrc522.PICC_ReadCardSerial()) return false; 
    if ( Islastcard()) {Stop(); lotone(); return false;} 
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak); 
    if (piccType != MFRC522::PICC_TYPE_MIFARE_1K){ Stop();lotone(); return false;}
    Serial.print(F("UID:"));  Serial.print(dump_byte_array(mfrc522.uid.uidByte,mfrc522.uid.size));
    return true;
    }

  String Read(byte sector){
    String data = "Data:";  
    byte trailerBlock = sector*4 + 3;
    setusekey(); 
    if(!READMODE){ if(!Authenticate(trailerBlock)){Stop();return "FALSE_AUTHENTICATE";}}
    else {if(!Authenticate_oldkey(trailerBlock)){Stop();return "FALSE_AUTHENTICATE";}}
    for (byte i=1;i<4;i++){
		status = mfrc522.MIFARE_Read(trailerBlock-i,buffer,&size);
			if (status != MFRC522::STATUS_OK) {readerror();lotone();return "FALSE_READ";}
		data = data + char_byte_array(buffer, 16);
    } 
    status = mfrc522.MIFARE_Read(trailerBlock,buffer,&size);
      if (status != MFRC522::STATUS_OK) {readerror();lotone();return "FALSE_READ";}
    if(buffer[9]  != 45 ||
       buffer[10] != 69 ||
       buffer[11] != 219||
       buffer[12] != 9||
       buffer[13] != 125||
       buffer[14] != 146||
       buffer[15] != 207
       ) Serial.print("FormatWrong"); //check_the_data_resuft 
    Stop();
    return data;
  }
  
  boolean Writedata(byte sector,String &Data,boolean newkey,boolean OLDkey){// sector   data to write key after write||key before write   
    resetIDmem();
    if(Data.length() > 48)return false;
    byte dataBlock[48];					//Max data lenght is 96 word (the buffer of serial is 128b) 
    stb(Data,dataBlock);
    byte trailerBlock = sector*4 + 3;
	  byte trailerBuffer[18];
	     trailerBuffer[9]  = 45;//
       trailerBuffer[10] = 69;
       trailerBuffer[11] = 219;
       trailerBuffer[12] = 9;///<<<----to sure that is my card :D
       trailerBuffer[13] = 125;
       trailerBuffer[14] = 146;
       trailerBuffer[15] = 207;
										/* ******o***** DANGER o*********o** */
	    setusekey();							/*******o*o*************o*******o*****/
	    setkey(trailerBuffer, newkey);				/******o***o******xD*****o*****o******/
										/*****o*****o***Hahaha****o***o*******/
									/****o*******o*************o*o********/
											/***o*********o*************o*********/
	mfrc522.MIFARE_SetAccessBits(&trailerBuffer[6], 0, 0, 0, 1); // safety
  
    if(OLDkey){ if(!Authenticate(trailerBlock)) return false;}
    else {if(!Authenticate_oldkey(trailerBlock))return false;}
    
	status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(trailerBlock, trailerBuffer, 16);
	if (status != MFRC522::STATUS_OK) {
		Serial.print(F("WRITE_1_ERROR"));
		Serial.println(mfrc522.GetStatusCodeName(status));
    lotone();
		return false;
	}
    byte datawrite[16];
    for (byte blockAddr = 1; blockAddr <= 3; blockAddr++) {
        for (byte i = 0;i <= 15;i++) {                 ///transmite data to datawrite[]
        datawrite[i] = dataBlock[16*(blockAddr-1)+i];// Zzzz...
      }
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(trailerBlock-blockAddr,datawrite,16);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("WRITE_2_ERROR"));
        Serial.println(mfrc522.GetStatusCodeName(status));
        lotone();
        return false;//resetFunc();
      }
    }
    
    Serial.print("WRITE_done...");
    
    if(newkey){ if(!Authenticate(trailerBlock))return false;}
    else {if(!Authenticate_oldkey(trailerBlock))return false; } 
    String readdata = "";
    for (byte blockAddr = 1; blockAddr <= 3; blockAddr++) {
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(trailerBlock-blockAddr,buffer,&size); //error here
      if (status != MFRC522::STATUS_OK) {readerror();}
      readdata = readdata + char_byte_array(buffer, 16);
    }
    //CHECK DATA RESUST 
    byte count = 0;
    for (byte i = 0; i < 48; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (dataBlock[i] == readdata[i])
        count++; // c++ :D
    }
    if (count == 48) {
      Serial.println(F("Success"));
    }
    else { Serial.println(F("Failure")); }
    Stop();
    return true;
  }

  boolean Locksector(byte sector,boolean key) { //using old key
    byte trailerBlock = sector*4 + 3;
	  byte trailerBuffer[18];
	  trailerBuffer[9] = 33;
	  mfrc522.MIFARE_SetAccessBits(&trailerBuffer[6], 0b010, 0b010, 0b010, 0b010); // lock all key & data
	  setusekey();  
	  setkey(trailerBuffer,key);
	  if(!Authenticate(trailerBlock)) return false;
	  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(trailerBlock, trailerBuffer, 16);
	  if (status != MFRC522::STATUS_OK) {
		  Serial.print(F("LOCK_ERROR:"));
		  Serial.println(mfrc522.GetStatusCodeName(status));
      lotone();
		  return;
	  }
   Serial.print(F("LOCK_COMPLETE"));
   Stop();
   resetIDmem();
  }

  boolean checksys(void){
    bool result = mfrc522.PCD_PerformSelfTest();
    if (result) return true; else return false;
  }

  void Stop(){ 
    for(byte i=0;i<4;i++){
      cardID[i]= mfrc522.uid.uidByte[i];
    }
    mfrc522.PICC_HaltA();// Halt PICC
    mfrc522.PCD_StopCrypto1();// Stop encryption on PCD
  } 
  
  void setnowmalkey(){
    for(byte i = 0;i<=5;i++){
    key.keyByte[i] = 0xFF;}
  }

  void lotone(){
    tone(8,733,70);
  }

  void(* resetFunc) (void) = 0;//cài đặt hàm reset
  
  private: 
  
  void setusekey(){
    for(byte i=0;i<4;i++){
      Newkey.keyByte[i] = mfrc522.uid.uidByte[i];
    }
      Newkey.keyByte[4] = mfrc522.uid.uidByte[1];
      Newkey.keyByte[5] = mfrc522.uid.uidByte[2];
  }

  void resetIDmem(){
    for(byte i=0;i<4;i++){
      cardID[i]= 0xFF;
    }
  }
  
  boolean Authenticate(byte trailer){
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,trailer, &Newkey, &(mfrc522.uid));     //63 is trailer Block
    if (status != MFRC522::STATUS_OK) { 
    Serial.print(F("Auth_newkey_ERROR: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    Stop(); 
    lotone();
    return false;   
    }
    return true;
  }
  
  boolean Authenticate_oldkey(byte trailer){
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,trailer, &key, &(mfrc522.uid));     //63 is trailer Block
    if (status != MFRC522::STATUS_OK) { 
    Serial.print(F("Auth_oldkey_ERROR: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    Stop(); 
    return false;   
    }
    return true;
  }
  
  boolean Islastcard(){
          if(mfrc522.uid.uidByte[0] != cardID[0] || 
             mfrc522.uid.uidByte[1] != cardID[1] || 
             mfrc522.uid.uidByte[2] != cardID[2] || 
             mfrc522.uid.uidByte[3] != cardID[3] ) {
      return false; // IT NOT LAST CARD ...
    } else{
    Serial.println(F("LASTCARD"));
    return true;
    }
  }

  void readerror(){
        Serial.print(F("READ_ERROR:"));
        Serial.println(mfrc522.GetStatusCodeName(status));
        Serial.println(F("Reset_SYS"));
        Stop();
        resetFunc(); // reset
  }

          /*convert data*/
          
  void setkey(byte *trailer,boolean chose){
	if(chose) for (byte i = 0;i <= 5;i++) {
		trailer[i] = Newkey.keyByte[i];
	} else for (byte i = 0;i <= 5;i++) {
		trailer[i] = key.keyByte[i];
	}
}

  void stb(String data, byte *output) {
  if (data.length() > 48){
    for (int a = 0; a < 48;a++) {
    output[a] = data[a];
    }
  }else{
  for (unsigned int a = 0; a < data.length() ;a++) {
    output[a] = data[a];
  }
  for (unsigned int a = data.length();a <= 48;a++){
    output[a] = char(' '); 
  }
  }
}

  String char_byte_array(byte *buffer, byte bufferSize) {
  String dout;
  for (byte i = 0; i < bufferSize; i++) {
    dout += (char(buffer[i]));
    //dout +=String(buffer[i],HEX)+" ";
  }
  return dout;
}

  String dump_byte_array(byte *buffer, byte bufferSize) {
  String dataa;
  for (byte i = 0; i < bufferSize; i++) {
    dataa += String(buffer[i], HEX);
    dataa += String(F(" "));
  }
  return dataa;
}

};
#endif
























