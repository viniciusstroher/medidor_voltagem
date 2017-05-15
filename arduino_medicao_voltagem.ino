// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
#include "SoftwareSerial.h"
#include <LiquidCrystal.h>
#include "EmonLib.h"                   // Include Emon Library

EnergyMonitor emon1;                   // Create an instance
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#include "ESP8266.h"

#define SSID        "COBRE"
#define PASSWORD    "robotica"
#define HOST_NAME   "192.168.25.2"
#define HOST_PORT   (8091)


static uint8_t mux_id = 0;
ESP8266 wifi(Serial1);

void(* resetFunc) (void) = 0; 
int rede = 220;

void setup()
{  

    lcd.begin(16, 2);
    lcd.clear();
    
    Serial1.begin(115200);
    Serial.begin(9600);

    if (wifi.joinAP(SSID, PASSWORD)) {     
        Serial.println(wifi.getLocalIP().c_str());
        if(!wifi.enableMUX()){
           resetFunc();
        }
    } else {
        resetFunc();
    }
    
    emon1.current(A1, 1.1);

    lcd.setCursor(0,0);
    lcd.print("Corr.(A):");
    lcd.setCursor(0,1);
    lcd.print("Pot. (W):");
    
}

char* params;    
String paramsArduino = "";
uint8_t buffer[128];

void loop()
{
  
  double Irms = emon1.calcIrms(1480);

  lcd.setCursor(10,0);
  lcd.print(Irms);
   
  //Calcula e mostra o valor da potencia

  lcd.setCursor(10,1);
  lcd.print("      ");
  lcd.setCursor(10,1);
  lcd.print(Irms*rede,1);
  
  if (wifi.createTCP(mux_id, HOST_NAME, HOST_PORT)) {
      
      buffer[128]  = {0};
      Serial.println("I: "+String(Irms));
      paramsArduino = "{ \"corrente\" : \""+String(Irms)+"\"  }";
      params = new char[paramsArduino.length()+1];
      
      strncpy(params, paramsArduino.c_str(), paramsArduino.length()+1);
      if (wifi.send(mux_id, (const uint8_t*)params, strlen(params))) {
         Serial.println("Enviando: "+paramsArduino);
               
      }else{
         resetFunc();
      }
         
      if (!wifi.releaseTCP(mux_id)) {
         Serial.println("release tcp");
         resetFunc();
      }
      
  } else {
     Serial.println(" tcp not create");
     resetFunc();    
  }

  
}
