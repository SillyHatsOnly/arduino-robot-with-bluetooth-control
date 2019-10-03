#include <SoftwareSerial.h>                                                      //  Подключаем библиотеку SoftwareSerial для общения с модулем по программной шине UART
#include <iarduino_Bluetooth_HC05.h>                                             //  Подключаем библиотеку iarduino_Bluetooth_HC05 для работы с Trema Bluetooth модулем HC-05
SoftwareSerial          softSerial(9, 10);                                       //  Создаём объект softSerial указывая выводы RX, TX (можно указывать любые выводы Arduino UNO). Вывод 2 Arduino подключается к выводу TX модуля, вывод 3 Arduino подключается к выводу RX модуля
iarduino_Bluetooth_HC05 hc05(13);                                                //  Создаём объект hc05 указывая любой вывод Arduino, который подключается к выводу K модуля
                                                                                 //  
uint8_t  pinShield_H2 = 4;                                                       //  Вывод, подключенный к драйверу, для задания направления вращения левым мотором
uint8_t  pinShield_E2 = 5;                                                       //  Вывод ШИМ, подключенный к драйверу, для задания скорости левого мотора
uint8_t  pinShield_E1 = 6;                                                       //  Вывод ШИМ, подключенный к драйверу, для задания скорости правого мотора
uint8_t  pinShield_H1 = 7;                                                       //  Вывод, подключенный к драйверу, для задания направления вращения правым мотором
uint8_t  pinLED_RED   = 12;                                                      //  Вывод с красным светодиодом
uint8_t  pinLED_BLUE  = 11;                                                      //  Вывод с синим светодиодом
uint16_t time_period  = 200;                                                     //  Частота мигания светодиодов (в миллисекундах)
uint8_t  valSpeed     = 255;                                                     //  Максимальная скорость ШИМ (число от 0 до 255)
bool     arrRoute[2]  = {1, 1};                                                  //  Направление движения для каждого мотора ([0]- правый мотор, [1] - левый мотор)
uint16_t arrSpeed[2];                                                            //  Скорость для каждого мотора ([0]- правый мотор, [1] - левый мотор)
uint32_t tmrLED;                                                                 //  Время  последнего включения светодиодов
uint32_t flgTime;                                                                //  Флаг для задания времени принятия пакетов от Bluetooth телефона
uint8_t  flg;                                                                    //  Флаг кнопок
uint32_t tmrWait;                                                                //  Время до начала сопряжения с новыми устройствами
bool     flg_LED;                                                                //  Флаг включения светодиодов
                                                                                 //  </iarduino_bluetooth_hc05.h></softwareserial.h>
void setup() {                                                                   //  
    // BLUETOOTH МОДУЛЬ                                                          //  
    Serial.begin  (9600);                                                        //  Инициируем передачу данных по аппаратной шине UART для вывода результата в монитор последовательного порта
    Serial.print  ("begin: ");                                                   //  Выводим текст "begin: " в монитор последовательного порта
    if (hc05.begin(softSerial))     {Serial.println("Ok");}                      //  Инициируем работу с Trema модулем hc05, указывая объект softSerial через который осуществляется связь по шине UART
    else                            {Serial.println("Error");}                   //  Если работа с модулем не инициирована, то выводим сообщение об ошибке
    tmrWait = millis();                                                          //  Устанавливаем таймер ожидания сопряжения
    while (!hc05.checkConnect() && millis()<tmrWait+60000) {;}                   //  Ждём в течении 60 секунд сопряжения с последним устройством из памяти
    if (millis()<tmrWait+60000)     {Serial.println("Connect with last ADR");}   //  Если сопряжение произошло, то выдаём в монитор порта сообщение об этом
    else {                                                                       //  Если сопряжение не произошло, то
    if (hc05.createSlave("BT_CAR", "1234"))                                      //  Создаем ведомую роль модулю, указывая его имя и pin-код (в примере имя = "BT_CAR", pin-код = "1234")
                                    {Serial.println("Slave create");}            //  Если ведомая роль была создана, выводим сообщение об успехе в монитор порта,
    else                            {Serial.println("Slave not create");}        //  а если не была создана - выводим сообщение об ошибке в монитор порта.
      }                                                                          //  
    // МОТОРЫ                                                                    //  
    pinMode(pinShield_H2, OUTPUT);                                               //  Конфигурируем вывод pinShield_H2 как выход (направление вращения левого мотора)
    pinMode(pinShield_E2, OUTPUT);                                               //  Конфигурируем вывод pinShield_E2 как выход (скорость вращения левого мотора, ШИМ)
    pinMode(pinShield_E1, OUTPUT);                                               //  Конфигурируем вывод pinShield_E1 как выход (скорость вращения правого мотора, ШИМ)
    pinMode(pinShield_H1, OUTPUT);                                               //  Конфигурируем вывод pinShield_H1 как выход (направление вращения правого мотора)
    // СВЕТОДИОДЫ                                                                //  
    pinMode(pinLED_RED,OUTPUT);                                                  //  Конфигурируем вывод pinLED_RED как выход 
    pinMode(pinLED_BLUE,OUTPUT);                                                 //  Конфигурируем вывод pinLED_BLUE как выход
    tmrLED = millis();                                                           //  Устанавливаем таймер светодиодов равным millis()
    flg_LED = 0;                                                                 //  Сбрасываем флаг светодиодов
}                                                                                //  
void loop() {                                                                    //  
  if (softSerial.available()) {                                                  //  Если есть принятые данные, то ...
    String str;                                                                  //  Создаём строку str
    while (softSerial.available()) {                                             //  Выполняем цикл пока есть что читать ...
      str += char(softSerial.read());                                            //  Читаем очередной принятый символ из UART в строку str
      delay(5);                                                                  //  Задержка на 5 мс на случай медленного приёма
    }                                                                            //  Цикл завершён, значит читать больше нечего
                                            // КНОПКИ ДВИЖЕНИЯ                   //  
                               // Флаг времени            Флаг кнопки            //  
    if (str == "II") {    flgTime = millis();     flg = 1;    }                  //  Кнопка "стрелка вверх-влево"
    if (str == "FF") {    flgTime = millis();     flg = 2;    }                  //  Кнопка "стрелка вверх"
    if (str == "GG") {    flgTime = millis();     flg = 3;    }                  //  Кнопка "стрелка вверх-вправо"
    if (str == "RR") {    flgTime = millis();     flg = 4;    }                  //  Кнопка "стрелка влево"
    if (str == "SS") {    flgTime = millis();     flg = 5;    }                  //  СТОП
    if (str == "LL") {    flgTime = millis();     flg = 6;    }                  //  Кнопка "стрелка вправо"
    if (str == "JJ") {    flgTime = millis();     flg = 7;    }                  //  Кнопка "стрелка вниз-влево"
    if (str == "BB") {    flgTime = millis();     flg = 8;    }                  //  Кнопка "стрелка вниз"
    if (str == "HH") {    flgTime = millis();     flg = 9;    }                  //  Кнопка "стрелка вниз-вправо"
                                        // КНОПКИ ДОПОЛНИТЕЛЬНЫХ ФУНКЦИЙ         //  
                          // Если кнопка нажата        меняем флаг               //  
    if (str == "SWS" || str == "SwS") {flg_LED = !flg_LED;}                      //  Кнопка включения светодиодов
    if (str == "S0S") {flg     = 10;      }                                      //  Ползунок скорости в положении 0
    if (str == "S1S") {flg     = 11;      }                                      //  Ползунок скорости в положении 1
    if (str == "S2S") {flg     = 12;      }                                      //  Ползунок скорости в положении 2
    if (str == "S3S") {flg     = 13;      }                                      //  Ползунок скорости в положении 3
    if (str == "S4S") {flg     = 14;      }                                      //  Ползунок скорости в положении 4
    if (str == "S5S") {flg     = 15;      }                                      //  Ползунок скорости в положении 5
    if (str == "S6S") {flg     = 16;      }                                      //  Ползунок скорости в положении 6
    if (str == "S7S") {flg     = 17;      }                                      //  Ползунок скорости в положении 7
    if (str == "S8S") {flg     = 18;      }                                      //  Ползунок скорости в положении 8
    if (str == "S9S") {flg     = 19;      }                                      //  Ползунок скорости в положении 9
    if (str == "SqS") {flg     = 20;      }                                      //  Ползунок скорости в положении 10
    //  ======================================================================================================================================================
    switch (flg) {//Направление левого мотора    Направление правого мотора      Скорость левого мотора             Скорость правого мотора
      case 1:          arrRoute[1] = 1;             arrRoute[0] = 1;            arrSpeed[1] = (valSpeed / 2);       arrSpeed[0] = valSpeed;             break;       //  С-З
      case 2:          arrRoute[1] = 1;             arrRoute[0] = 1;            arrSpeed[1] = valSpeed;             arrSpeed[0] = valSpeed;             break;       //  С
      case 3:          arrRoute[1] = 1;             arrRoute[0] = 1;            arrSpeed[1] = valSpeed;             arrSpeed[0] = (valSpeed / 2);       break;       //  С-В
      case 4:          arrRoute[1] = 1;             arrRoute[0] = 1;            arrSpeed[1] = 0;                    arrSpeed[0] = valSpeed;             break;       //  З
      case 5:          arrRoute[1] = 1;             arrRoute[0] = 1;            arrSpeed[1] = 0;                    arrSpeed[0] = 0;                    break;       //  Стоп
      case 6:          arrRoute[1] = 1;             arrRoute[0] = 1;            arrSpeed[1] = valSpeed;             arrSpeed[0] = 0;                    break;       //  В
      case 7:          arrRoute[1] = 0;             arrRoute[0] = 0;            arrSpeed[1] = (valSpeed / 2);       arrSpeed[0] = valSpeed;             break;       //  Ю-З
      case 8:          arrRoute[1] = 0;             arrRoute[0] = 0;            arrSpeed[1] = valSpeed;             arrSpeed[0] = valSpeed;             break;       //  Ю
      case 9:          arrRoute[1] = 0;             arrRoute[0] = 0;            arrSpeed[1] = valSpeed;             arrSpeed[0] = (valSpeed / 2);       break;       //  Ю-В
      }                                                                          //  
  } // =======================================================================================================================================================  
  if     (flg == 10){valSpeed = 5;}                                              //  0 режим скорости
  else if(flg == 11){valSpeed = 30;}                                             //  1 режим скорости
  else if(flg == 12){valSpeed = 55;}                                             //  2 режим скорости
  else if(flg == 13){valSpeed = 80;}                                             //  3 режим скорости
  else if(flg == 14){valSpeed = 105;}                                            //  4 режим скорости
  else if(flg == 15){valSpeed = 130;}                                            //  5 режим скорости
  else if(flg == 16){valSpeed = 155;}                                            //  6 режим скорости
  else if(flg == 17){valSpeed = 180;}                                            //  7 режим скорости
  else if(flg == 18){valSpeed = 205;}                                            //  8 режим скорости
  else if(flg == 19){valSpeed = 230;}                                            //  9 режим скорости
  else if(flg == 20){valSpeed = 255;}                                            //  10 режим скорости
                                                                                 //  
  if (flg_LED) {                                                                 //  Если флаг установлен (была нажата кнопка включения фары)
    if (millis() - tmrLED > time_period) {                                       //  мигаем светодиодами с заданной частотой
      tmrLED = millis();                                                         //  сохраняем время
      digitalWrite(pinLED_RED, digitalRead(pinLED_BLUE));                        //  управляем питанием красного светодиода
      digitalWrite(pinLED_BLUE, !digitalRead(pinLED_BLUE));                      //  управляем питанием синего светодиода
      }                                                                          //  
    } else {                                                                     //  если флаг сброшен, то
      digitalWrite(pinLED_RED, LOW);                                             //  гасим светодиоды
      digitalWrite(pinLED_BLUE, LOW);                                            //  
      }                                                                          //  
  if (flgTime > millis()) {                                                      //  Если millis() переполнен, то 
    flgTime = 0;                                                                 //  сбрасываем флаг в ноль
    }                                                                            //  
// ПОДАЧА ЗНАЧЕНИЙ СКОРОСТИ И НАПРАВЛЕНИЯ ВРАЩЕНИЯ НА ВЫВОДЫ                     //  
  if (flgTime > (millis() - 500)) {                                              //  Если сигналы с телефона приходят (в течении 50 мс)
    digitalWrite(pinShield_H2, arrRoute[1]);                                     //  тогда задаем направление вращения правого мотора
    digitalWrite(pinShield_H1, arrRoute[0]);                                     //  и левого мотора
    analogWrite(pinShield_E2, arrSpeed[1]);                                      //  Задаём скорость вращения для правого мотора
    analogWrite(pinShield_E1, arrSpeed[0]);                                      //  и для левого мотора
    } else {                                                                     //  Если пакеты не приходят
    analogWrite(pinShield_E2, 0);                                                //  Останавливаем работу моторов
    analogWrite(pinShield_E1, 0);                                                //  
    }                                                                            //  
}                                                                                //  
