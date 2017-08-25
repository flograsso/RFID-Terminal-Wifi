#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

//needed for library
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>
#include "phrases.h"

#define DEBUG true
#define UID_LENGTH 12

String recibido, payload, UID;
bool WiFi_Status = false;
HTTPClient http;
WiFiManager wifiManager;
byte httpCode, aux, indice;
unsigned long cardUID;

/*
    Set de Comandos:

      Chequea el serial. Simil al comando "AT"
      Comando: ESP
      Respuesta: OK

      Chequea si esta conectado al WiFi o no.
      Comando: WIFI_STATUS
      Respuesta: WIFI_FAIL / WIFI_OK

      Chequea si hay conexión a internet o no.
      Comando: CHECK_CONNECTION
      Respuesta: CONNECTION_OK / CONNECTION_FAIL

      Reconecta al WiFi
      Comando: RECONNECT_WIFI
      Respuesta: WIFI_FAIL / WIFI_OK

      Borra SSID y password guardados en EEPROM.
      Comando: RESET_WIFI
      Respuesta: RESET_OK.Luego de este comando hay que utilizar el comando RECONNECT_WIFI

      Comando: CARD_UID. Formato: CARD_UID=(12 digitos); Ex: CARD_UID=123456789123; CARD_UID=000056789123;
               Si no recibe los 12 digitos funciona mal.
*/
void setup()
{
  Serial.begin(9600);

  /*Salida de debugging del portal de configuracion*/
  wifiManager.setDebugOutput(false);

  /*Tiempo de TimeOut en segundos*/
  wifiManager.setTimeout(120);

  /*Para resetear el SSID guardado en EEPROM*/
  //wifiManager.resetSettings();

  /*Intenta conectar al SSID guardado en la EEPROM y sino crea un AP con el SSID=WiFi_Terminal_RFID
    el cual, al estar conectado a él, me permite configurar la red wifi a la cual me quiero conectar
    Para ello debo entrar a la IP 192.168.4.1 o a algun sitio web HTTP (no HTTPS) para que actue el DNS
    del módulo y me rediriga hacia la anterior IP. La funcion es bloqueante, no devuelve el control
    hasta que se conecta a una red o pasa el tiempo de timeout.*/
  while (!wifiManager.autoConnect("WiFi_Terminal_RFID", "cespi"))
  {
    //Codigo que se ejecuta cada "wifiManager.setTimeout(120)" mientras no se conecta a un wifi
    if (Serial.available())
    {
      recibido = Serial.readString();
      if (recibido.indexOf(F("WIFI_STATUS")) != -1)
      {
        Serial.println(F("WIFI_FAIL"));
      }
    }
  }

  //Conectado
  WiFi_Status = true;
  Serial.println(F("WIFI_OK"));
}

void loop()
{

  /*Bloqueante. Se queda esperando que le llegue un comfando*/
  while (!Serial.available())
  {
  };
  recibido = Serial.readString();

  if (recibido.indexOf(F("WIFI_STATUS")) != -1)
  {
    //WIFI STATUS RUTINE
    if (WiFi_Status)
      Serial.println(F("WIFI_OK"));
    else
      Serial.println(F("WIFI_FAIL"));
  }
  else
  {
    if (recibido.indexOf(F("CHECK_CONNECTION")) != -1)
    {

      http.begin("http://jsonplaceholder.typicode.com/users/1");
      httpCode = http.GET();
      payload = http.getString();
      http.end();

      /*
        http.begin("http://api.thingspeak.com/update?api_key=W7ZBYXPR3FIY2SOF&field1=10");
        httpCode = http.GET();
        payload = http.getString();
        http.end();
      */
      if (httpCode > 0)
      {
        Serial.println(F("CONNECTION_OK"));
      }
      else
      {
        Serial.println(F("CONNECTION_FAIL"));
      }
    }
    else
    {
      if (recibido.indexOf(F("RECONNECT_WIFI")) != -1)
      {
        WiFi_Status = false;
        while (!wifiManager.autoConnect("WiFi_Terminal_RFID", "cespi"))
        {
          //Codigo que se ejecuta cada "wifiManager.setTimeout(120)" mientras no se conecta a un wifi
          if (Serial.available())
          {
            recibido = Serial.readString();
            if (recibido.indexOf(F("WIFI_STATUS")) != -1)
            {
              Serial.println(F("WIFI_FAIL"));
            }
          }
        }
        Serial.println(F("WIFI_OK"));
        WiFi_Status = true;
      }
      else
      {
        if (recibido.indexOf(F("RESET_WIFI")) != -1)
        {
          wifiManager.resetSettings();
          WiFi_Status = false;
          Serial.println(F("RESET_OK"));
        }
        else
        {
          if (recibido.indexOf(F("CARD_UID")) != -1)
          {
            //Procesar terjeta
            UID = "";
            aux = 0;
            indice = 0;
            while (recibido[indice] != '=')
            {
              indice++;
            }
            indice++;
            while (recibido[indice] != ';')
            {
              UID.concat(String(recibido[indice]));
              aux++;
              indice++;
            }

            cardUID = UID.toInt();
            Serial.println(cardUID);
            http.begin(serverURL);
            http.addHeader(header1,header2);
            httpCode = http.POST("arg1=value1&arg2=value2");
            payload = http.getString();
            if (httpCode > 0)
            {
              Serial.println(F("CONNECTION_OK"));
              Serial.println(payload);
            }
            else
            {
              Serial.println(F("CONNECTION_FAIL"));
            }
          }
          else
          {
            if (recibido.indexOf(F("ESP")) != -1)
            {
              Serial.println(F("OK"));
            }
            else
            {
              Serial.println(F("INCORRECT COMMAND"));
            }
          }
        }
      }
    }
  }

  /*
    HTTPClient http;
    http.begin("http://jsonplaceholder.typicode.com/users/1");
    int httpCode = http.GET();
    String payload = http.getString();
    Serial.println(payload);
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject &root = jsonBuffer.parseObject(payload);

    char a1[32];
    strcpy(a1, root["name"]);
    Serial.println(a1);

    http.end();
    delay(30000); //Send a request every 30 seconds
  */
}
