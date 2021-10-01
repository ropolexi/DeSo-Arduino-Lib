#include <Arduino.h>
#include "DeSoLib.h"
#include <WiFi.h>
#include "cert.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "customchars.h"

// Set the LCD address to 0x27 for a 16 chars and 4 line display
#define I2C_SDA 14 // SDA Connected to GPIO 14
#define I2C_SCL 15 // SCL Connected to GPIO 15

LiquidCrystal_I2C lcd(0x27, 16, 4);

//Fill in the ssid and password
const char ssid[] = "";
const char wifi_pass[] = "";
DeSoLib deso;
int server_index = 0;

void setup()
{
  // put your setup code here, to run once:
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, deso_logo);
  lcd.createChar(1, diamond);
  lcd.createChar(2, heart);
  lcd.clear();
  lcd.print("DeSo Dashbaord");
  lcd.setCursor(0, 1);
  lcd.print("(Bitclout)");
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
  deso.addNodePath("https://members.giftclout.com", giftclout_caRootCert);
  deso.selectDefaultNode(0);
  lcd.clear();
}
void nextServer()
{
  server_index++; //try different nodes
  if (server_index >= deso.getMaxNodes())
    server_index = 0;
}
void loop()
{
  while (true)
  {
    if (WiFi.isConnected())
    {
      do
      {
        deso.selectDefaultNode(server_index);
        if (!deso.updateNodeHealthCheck())
        {
          nextServer();
        }
      } while (!deso.getSelectedNodeStatus());

      Serial.println();
      Serial.print("DeSo Node: ");
      Serial.println(deso.getSelectedNodeUrl());

      if (!deso.updateExchangeRates())
      {
        Serial.println("exchange error!");
        nextServer();
        continue;
      }
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print(&deso.getSelectedNodeUrl()[0]+8);

      Serial.print("DeSo Coin Value: $");
      double temp = deso.USDCentsPerBitCloutExchangeRate / 100.0;
      Serial.println(temp);
      lcd.setCursor(0, 1);
      lcd.print("        ");
      lcd.setCursor(0, 1);
      lcd.print("D:$");
      lcd.print(temp, 0);
      //Serial.println("BTC (USD):");
      //Serial.println(deso.USDCentsPerBitcoinExchangeRate/100.0);
      Serial.println("=======Profile========");
      DeSoLib::Profile profile1;
      delay(1000);
      //deso.updateSingleProfile("ropolexi", "" ,&profile1);//search by username or public key
      int status = deso.updateSingleProfile("", "BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", &profile1);
      if (!status)
      {
        Serial.println("single profile error!");
        nextServer();
        continue;
      }
      Serial.print("Username: ");
      Serial.println(profile1.Username);
      Serial.print("PublicKey: ");
      Serial.println(profile1.PublicKeyBase58Check);
      Serial.print("Creator Coin Price: $");

      double coinPriceUSDCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.CoinPriceBitCloutNanos / 1000000000.0);
      temp = coinPriceUSDCents / 100.0;
      Serial.println(temp);
      
      
      lcd.setCursor(0-4, 2);
      lcd.print("        ");
      lcd.setCursor(0-4, 2);
      lcd.print("C:$");
      lcd.print(temp, 1);
      status = deso.updateUsersStateless("BC1YLfghVqEg2igrpA36eS87pPEGiZ65iXYb8BosKGGHz7JWNF3s2H8", false, &profile1);
      if (!status)
      {
        Serial.println("user stateless error!");
        nextServer();
        continue;
      }

      Serial.print("Wallet Balance:");
      double balanceCents = deso.USDCentsPerBitCloutExchangeRate * (profile1.BalanceNanos / 1000000000.0);
      temp = balanceCents / 100.0;
      lcd.setCursor(8-4, 2);
      lcd.print("        ");
      lcd.setCursor(8-4, 2);
      lcd.print("B:$");

      lcd.print(temp, 1);
      Serial.println(balanceCents / 100.0);
      Serial.print("Total HODLE assets : ");
      Serial.println(profile1.TotalHodleNum);
      Serial.print("Total HODLE Asset Balance: $");
      double assetsValue = (profile1.TotalHODLBalanceClout * deso.USDCentsPerBitCloutExchangeRate) / 100.0;
      Serial.println(assetsValue);
      lcd.setCursor(8, 1);
      lcd.print("        ");
      lcd.setCursor(8, 1);
      temp = assetsValue;
      lcd.print("H:$");
      lcd.print(temp, 1);
      deso.updateLastNumPostsForPublicKey(profile1.PublicKeyBase58Check,5, &profile1);
      Serial.print("Last Post Likes: ");
      Serial.println(profile1.lastPostLikes);
      Serial.print("Last Post Diamonds: ");
      Serial.println(profile1.lastNPostDiamonds);
      lcd.setCursor(-4,3);
      lcd.print("        ");
      lcd.setCursor(-4,3);
      lcd.write(2);
      lcd.print(" ");
      lcd.print(profile1.lastNPostLikes);

      lcd.setCursor(-4+8,3);
      lcd.print("        ");
      lcd.setCursor(-4+8,3);
      lcd.write(1);
      lcd.print(" ");
      lcd.print(profile1.lastNPostDiamonds);
      Serial.println("======================");
    }
    delay(10000UL);
    nextServer();
  }
}