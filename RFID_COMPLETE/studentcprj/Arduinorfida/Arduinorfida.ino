#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9       
#define SS_PIN          10 

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status; // for error report
byte buffer[18];
byte size = sizeof(buffer);


void tonee(){
  tone(8,3000,30);
}
void Authenticate(){
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,63, &key, &(mfrc522.uid));//63 is trailer Block
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("ERROR:1 "));
    Serial.println(mfrc522.GetStatusCodeName(status));
//    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B,63, &key, &(mfrc522.uid));//63 is trailer Block
//    if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("ERROR:1"));
//    Serial.pritln(mfrc522.GetStatusCodeName(status));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();  
    mfrc522.PCD_Init(); 
    return;
  }
}
void stb(String data, byte *output) {
  if (data.length() > 47){
    for (int a = 0; a < 47 ;a++) {
    output[a] = data[a];
    }
  }else{
  for (int a = 0; a < data.length() ;a++) {
    output[a] = data[a];
  }
  for (int a = data.length();a < 49;a++){
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
void ReadSerial() {
  String Sdata = Serial.readStringUntil('\r');
  Serial.flush();

  if (Sdata.indexOf("buzz") != -1)tone(8, 3333, 33);//just for fun

  if (Sdata.indexOf("READY?") != -1) {
    bool result = mfrc522.PCD_PerformSelfTest();
    if (result)
      Serial.println(F("OK"));
    else
      Serial.println(F("UNKNOWN"));
  }
  if (Sdata.indexOf("write:") != -1) {
    String writes;
    Serial.println(F("inputcard..."));
    for (int i = Sdata.indexOf("write:") + 6; i < Sdata.indexOf("#"); i++) {
      writes = writes + Sdata[i];
    }
    writes = writes+"#";
//    Serial.println(writes);
    /// write model
    byte dataBlock[48];
    stb(writes, dataBlock);
  //  Serial.println(dump_byte_array(dataBlock,48));//test
    while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      if (Serial.available()) { 
         String outdatas = Serial.readStringUntil('\r');
         if (outdatas.indexOf("cancle") != -1) {
        Serial.print("EXIT"); return; }
    }
    }
    tonee();
    Authenticate();
    //write data
    for (byte blockAddr = 0; blockAddr <= 2; blockAddr++) {
      byte datawrite[15];
        for (int i = 0;i <= 15;i++) {
        datawrite[i] = dataBlock[16*blockAddr+i];//sorry i wana tto Zzzz...
      }
     // Serial.println(char_byte_array(datawrite,16));//testr
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr+60, datawrite, 16);

      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("ERROR1"));
        Serial.println(mfrc522.GetStatusCodeName(status));
      }
    }

    String readdata;
    for (byte blockAddr = 60; blockAddr <= 62; blockAddr++) {
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("ERROR:1"));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
      }
      readdata = readdata + char_byte_array(buffer, 16);
    }
    Serial.println();
    Serial.println(readdata);
    Serial.println(writes);
    // Check that data in block is what we have written
    // by counting the number of bytes that are equal

    byte count;
    for (byte i = 0; i < 48; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (dataBlock[i] == readdata[i])
        count++;
    }
    if (count == 48) {
      Serial.println(F("Success"));
    }
    else { Serial.println(F("Failure")); }
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
//    mfrc522.PCD_Init();//dager
    delay(100);//for save
    tonee();
  }
}
void(* resetFunc) (void) = 0;
void setup()
{
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_33dB);
  Serial.println("READY");
  tonee();
  for (byte i = 0; i < 6; i++) { //nowmal key like 0xFFFFFFFFFFFF in 6 byte
    key.keyByte[i] = 0xFF;
  }
}

void loop()
{ 
  if (Serial.available()) {
    ReadSerial();
  }
  if (!mfrc522.PICC_IsNewCardPresent()) { // look for new card
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {  // seclect card\
    return;
  }
  Serial.print(F("UID:"));Serial.print(dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size));
  Serial.print("#");
  ///print data in 3 block 60 61 62 on sector 15 
  // Authenticate using key A
  Authenticate();
  //read data
  String data = "Data:";
  for (byte blockAddr = 60; blockAddr <= 62; blockAddr++) {
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
    Serial.print(F("ERROR:1"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    resetFunc();
    return;
    }
  data = data + char_byte_array(buffer, 16);
  }
  //take data after "*"
  if (data.indexOf("#") != -1) {
    String out;
    for (long i = 0; i < data.indexOf("#"); i++) {
      out = out + data[i];
    }
    out = out + "*";
   Serial.println(out);
  }else{
    data = data + "*";//n
    Serial.println(data);
  }
  // print UID
 
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1(); delay(100);
   // for safe too
   tonee();
}



