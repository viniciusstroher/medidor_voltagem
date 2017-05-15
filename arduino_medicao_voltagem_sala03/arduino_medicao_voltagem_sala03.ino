// EmonLibrary examples openenergymonitor.org, Licence GNU GPL V3
#include "SoftwareSerial.h"
#include <LiquidCrystal.h>
#include "EmonLib.h"                  

//instancias do sensor e do display
EnergyMonitor emon1;                   
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#include "ESP8266.h"
//CONFIGURAR WIFI
#define SSID        "COBRE"
#define PASSWORD    "robotica"
#define HOST_NAME   "192.168.25.2"

//utilizar a porta 8091 para a sala 03 e a porta 8092 para a sala 02
#define HOST_PORT   (8091)


static uint8_t mux_id = 0;
ESP8266 wifi(Serial1);

//FUNÇÂO DE RESETA - as vezes o wifi esp8266 trava e precisa ser reiniciado
void(* resetFunc) (void) = 0; 
int rede = 220;

void setup()
{  
    //INICIA LCD
    lcd.begin(16, 2);
    lcd.clear();
    //INICIA PORTA DO WIFI Em 115200 baudrate RX TX
    Serial1.begin(115200);
    Serial.begin(9600);
    
    //TENTA CONECTAR NA WIFI
    if (wifi.joinAP(SSID, PASSWORD)) {     
        //PRINT IP
        Serial.println(wifi.getLocalIP().c_str());
        //DESABILITA MUX
        if(!wifi.enableMUX()){
           resetFunc();
        }
    } else {
        resetFunc();
    }

    //calibra a corrente em A1
    emon1.current(A1, 1.1);

    //Configura o display do lcd com potencia e corrente
    lcd.setCursor(0,0);
    lcd.print("Corr.(A):");
    lcd.setCursor(0,1);
    lcd.print("Pot. (W):");
    
}

//declaracao de variaveis do escopo
char* params;    
String paramsArduino = "";
uint8_t buffer[128];

void loop()
{
  //calcula corrente
  double Irms = emon1.calcIrms(1480);
  
  //mostra no lcd a corrente calculada
  lcd.setCursor(10,0);
  lcd.print(Irms);
   
  //Calcula e mostra o valor da potencia
  lcd.setCursor(10,1);
  lcd.print("      ");
  lcd.setCursor(10,1);
  lcd.print(Irms*rede,1);


  //tenta conectar no servidor node 
  if (wifi.createTCP(mux_id, HOST_NAME, HOST_PORT)) {
      //cria json para enviar ao servidor os dados da conexao
      buffer[128]  = {0};
      Serial.println("I: "+String(Irms));
      paramsArduino = "{ \"corrente\" : \""+String(Irms)+"\"  }";
      params = new char[paramsArduino.length()+1];
      
      strncpy(params, paramsArduino.c_str(), paramsArduino.length()+1);
      
      //envia dados para o servidor
      if (wifi.send(mux_id, (const uint8_t*)params, strlen(params))) {
         Serial.println("Enviando: "+paramsArduino);
               
      }else{
         resetFunc();
      }

      //desconecta do servidor
      if (!wifi.releaseTCP(mux_id)) {
         Serial.println("release tcp");
         resetFunc();
      }
      
  } else {
     // se nao conectou ao servidor reinicia a placa
     Serial.println(" tcp not create");
     resetFunc();    
  }

  
}
