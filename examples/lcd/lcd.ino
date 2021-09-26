#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#include <Wire.h> 
//LCD library used in this [https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library]
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Fill in the ssid and password
const char ssid[]="";
const char wifi_pass[]="";

DeSoLib deso;

void setup()
{
  // put your setup code here, to run once:
  lcd.begin();
  lcd.backlight();
  lcd.print("Hello, DeSo !");
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  WiFi.enableSTA(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, wifi_pass);
  while (!WiFi.isConnected())
  {
    delay(1000);
    Serial.print(".");
  }
  deso.addNodePath("https://bitclout.com", bitclout_caRootCert);
  deso.addNodePath("https://nachoaverage.com", nachoaverage_caRootCert);
  deso.addNodePath("https://members.giftclout.com",giftclout_caRootCert);
  deso.selectDefaultNode(0);
}

void loop()
{
  static int server_index = 0;
  
  if (WiFi.isConnected())
  {
    do{
      deso.selectDefaultNode(server_index);
      deso.updateNodeHealthCheck();
      if(!deso.getSelectedNodeStatus()){
        server_index++;
        if(server_index >= deso.getMaxNodes()) server_index=0; 
      }
       
    }while (!deso.getSelectedNodeStatus());
    
    Serial.println();
    Serial.print("DeSo Node: ");
    Serial.println(deso.getSelectedNodeUrl());

    deso.updateExchangeRates();
    Serial.print("DeSo Coin Value: $");
    double temp = deso.USDCentsPerBitCloutExchangeRate / 100.0;
    Serial.println(temp);
    lcd.setCursor(0,0);
    lcd.print("                ");
    lcd.setCursor(0,0);
    lcd.print("DeSo: $");
    lcd.print(temp);
    //Serial.println("BTC (USD):");
    //Serial.println(deso.USDCentsPerBitcoinExchangeRate/100.0);
    Serial.println("=======Profile========");
    DeSoLib::Profile profile1;
    delay(1000);
    //deso.updateSingleProfile("ropolexi", "" ,&profile1);//search by username or public key
    deso.updateSingleProfile("", "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", &profile1);

    Serial.print("Username: ");
    Serial.println(profile1.Username);
    Serial.print("PublicKey: ");
    Serial.println(profile1.PublicKeyBase58Check);
    Serial.print("Creator Coin Price: $");
 
    double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
    temp =coinPriceUSDCents / 100.0;
    Serial.println(temp);
    lcd.setCursor(0,1);
    lcd.print("        ");
    lcd.setCursor(0,1);
    lcd.print("C:$");
    lcd.print(temp,1);
    deso.updateUsersStateless("BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", true, &profile1);
    Serial.print("Wallet Balance:");
    double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
    temp = balanceCents / 100.0;
    lcd.print("        ");
    lcd.setCursor(8,1);
    lcd.print("B:$");
    
    lcd.print(temp,1);
    Serial.println(balanceCents / 100.0);
    Serial.println("======================");
  }
  delay(10000UL);
  server_index++;//try different nodes
  if(server_index >= deso.getMaxNodes()) server_index=0; 
}