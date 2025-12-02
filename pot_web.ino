#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


/* Установите здесь свои SSID и пароль */
const char* ssid = "Pot_WEB";       // SSID
const char* password = "12345678";  // пароль

/* Настройки IP адреса */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
    WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  server.begin();
  //Serial.println("HTTP server started");

  server.on("/", handle_OnConnect);
}

void handle_OnConnect() 
{  

  Serial.write(0xAA);
  //Serial.println("");
  delay(200);
  String data[7];
  int i = 0;

  while(Serial.available())
  {
    data[i]=Serial.readStringUntil(';');
    i++;
  }
  i=0;
  
  String str2,str3;
  String pumpState;
  if (data[2]=="0")
  {
    str2 = "Water over 50%";
  }  
  else if (data[3]=="1")
  {
    str2 = "Water less than 20%";
  }
  else if (data[2]=="1")
  {
    str2 = "Water less than 50%";
  }

  if (data[4]=="0")
  {
    str3 = "pH corrector over 50%";
  }  
  else if (data[5]=="1")
  {
    str3 = "pH corrector less than 20%";
  }
  else if (data[4]=="1")
  {
    str3 = "pH corrector less than 50%";
  }


  if (data[6]=="1")
  {
    pumpState = "Pump is On";
  }
  else
  {
    pumpState = "Pump is Off";
  }

  String webout = "Roman's WEB Pot data:\r\n\r\n";
  webout+="Humidity: ";
  webout+=data[0];
  webout+="\r\npH: ";
  webout+=data[1];
  webout+="\r\n";
  webout+=str2;
  webout+="\r\n";
  webout+=str3;
  webout+="\r\n";
  webout+=pumpState;
  server.send(200, "text/plain", webout); 
   
}


void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();

}
