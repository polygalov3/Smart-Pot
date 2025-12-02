#define HUMIDITY_PIN    A0
#define PH_PIN    A1

#define WATER50_PIN 5//2
#define WATER_EMPTY_PIN 4//3
#define PHCORR50_PIN 3//4
#define PHCORR_EMPTY_PIN 2//5


#define OPERATING_VOLTAGE   5.0
#define ZERO_SHIFT          0
#define CALIBRATION_FACTOR  3.5

// минимальный порог влажности почвы
#define HUMIDITY_MIN    20
// максимальный порог влажности почвы
#define HUMIDITY_MAX    320

// минимальный порог pH почвы
#define PH_MIN    6
// максимальный порог pH почвы
#define PH_MAX    7




// интервал между проверкой на полив растения
#define INTERVAL1        10000  
#define INTERVAL2        5000

#include <iarduino_Modbus.h>          // Подключаем библиотеку для работы по протоколу Modbus.
#include <iarduino_MB_Pump.h>         // Подключаем библиотеку для работы с блоком перистальтическиx насосов.
#include <SoftwareSerial.h>



#define RS485_1_DE_PIN  13
#define SOFT_UART1_TX_PIN 9
#define SOFT_UART1_RX_PIN 10




SoftwareSerial rs485_1(SOFT_UART1_RX_PIN, SOFT_UART1_TX_PIN);
ModbusClient     modbus(rs485_1, RS485_1_DE_PIN);   // Создаём объект для работы по протоколу Modbus указав класс Serial и номер вывода DE конвертера UART-RS485.
iarduino_MB_Pump pump(modbus);        // Создаём объект для работы с блоком перистальтическиx насосов, указав объект протокола Modbus.

int f; 

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;

void setup() {
  // put your setup code here, to run once:


  Serial.begin(9600);
  pinMode(WATER50_PIN, INPUT_PULLUP);
  pinMode(WATER_EMPTY_PIN, INPUT_PULLUP);
  pinMode(PHCORR50_PIN, INPUT_PULLUP);
  pinMode(PHCORR_EMPTY_PIN, INPUT_PULLUP);


  rs485_1.begin(9600);
  modbus.begin();
  while(!rs485_1);                   //
  pump.begin(2);                   // Инициируем работу с блоком перистальтическиx насосов, указав его адрес.

  // f = pump.begin(2);                     // Инициируем работу с блоком перистальтических насосов, указав его адрес.
  // if( !f ){                              // Если функция begin() вернула false, значит блок перистальтическиx насосов с адресом 2 не найден.
  //     Serial.print("not found");    //
  //     while(1);                          // Запрещаем дальнейшее выполнение скетча.
  // }

 // pump.setState(PUMP_A, true);
                                      //

//Serial.print("Humidity: ");

}

int humidityADC = 0;
int pHADC = 0;
float pHvoltage = 0;
float pH = 0;

int water50_n = 0;
int water_empty_n = 0;
int phcorr50_n = 0;
int phcorr_empty_n = 0;

int pumpOn=0;



void read_sensors()
{
  humidityADC = analogRead(HUMIDITY_PIN);
  pHADC = analogRead(PH_PIN);

  pHvoltage = pHADC * OPERATING_VOLTAGE / 1023;
  pH = CALIBRATION_FACTOR * (pHvoltage + ZERO_SHIFT); 
  water50_n = digitalRead(WATER50_PIN);
  water_empty_n =  digitalRead(WATER_EMPTY_PIN);
  phcorr50_n = digitalRead(PHCORR50_PIN);
  phcorr_empty_n =  digitalRead(PHCORR_EMPTY_PIN);
}



void send_web_data()
{
  String s;
  s+=humidityADC;
  s+=";";
  s+=pH;
  s+=";";
  s+=water50_n;
  s+=";";
  s+=water_empty_n;
  s+=";";
  s+=phcorr50_n;
  s+=";";
  s+=phcorr_empty_n;
  s+=";";
  s+=pumpOn;
 // Serial.println(humidityADC+ ";"+pH+ ";"+ water50_n+ ";"+ water_empty_n);
  Serial.println(s);
}

void waterOn()
{
  unsigned long currentMillis = millis();
  pumpOn=1;
  pump.setTimeOn(PUMP_A, 2);
  while (humidityADC<HUMIDITY_MAX)
  {
    //if (currentMillis - previousMillis2 >= INTERVAL2) {
      previousMillis2 = currentMillis;
      read_sensors();  
      pump.setTimeOn(PUMP_A, 2);
      delay(INTERVAL2);
      read_sensors();

      if(pH>2 && phcorr_empty_n==0)
      {
        if(pH<=PH_MIN)
        {
          while(pH<PH_MAX)
          {
            pump.setTimeOn(PUMP_B, 2);
            delay(INTERVAL2);
            read_sensors();
            int a;
            while(Serial.available())
            {
              a= Serial.read();
              if (a == 0xAA) send_web_data();
            }  
            a="";
          }
        }
        
        
      }
  //  }
    int a;
    while(Serial.available())
    {
      a= Serial.read();
      if (a == 0xAA) send_web_data();
    }  
    a="";
  }
  
}
void phcorrOn()
{
  unsigned long currentMillis = millis();
  pumpOn=1;
  pump.setTimeOn(PUMP_B, 2);
  while (humidityADC<HUMIDITY_MAX)
  {
    //if (currentMillis - previousMillis2 >= INTERVAL2) {
      previousMillis2 = currentMillis;
      read_sensors();  
      pump.setTimeOn(PUMP_B, 2);
      delay(INTERVAL2);
  //  }
    int a;
    while(Serial.available())
    {
      a= Serial.read();
      if (a == 0xAA) send_web_data();
    }  
    a="";
  }
  
}



void loop() {
  // put your main code here, to run repeatedly:
  read_sensors();
 

  int a;
  while(Serial.available())
  {
     a= Serial.read();
     if (a == 0xAA) send_web_data();
  }
  
  a="";
  //if (a == 0xAA) send_web_data();
 
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis1 >= INTERVAL1) {
    previousMillis1 = currentMillis;

    if (humidityADC<= HUMIDITY_MIN)
    { 
        if (pH<= PH_MIN)
        {
          if(phcorr_empty_n==0)
          {
            phcorrOn();
          }      
        }
        else
        {
          if(water_empty_n==0)
          {
            waterOn();
          }
        }
      pumpOn=0;
    }
   }

}
