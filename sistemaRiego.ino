/*Pantalla LCD*/
#include <Wire.h>                 // libreria de comunicacion por I2C
#include <LCD.h>                  // libreria para funciones de LCD
#include <LiquidCrystal_I2C.h>

/*DHT sensor Temperatura y Humedad*/
#include <DHT.h>
#include <DHT_U.h>

/*RTC3231 Reloj y Fecha*/
//#include <Wire.h>    // incluye libreria para interfaz I2C
#include <RTClib.h>   // incluye libreria para el manejo del modulo RTC

//Keypad - Teclado
#include <Keypad.h>     // importa libreria Keypad

/*----------------Coneccion----------------
  DHT11
  DATA -> PIN 4

  RTC3231
  SCL -> PIN A5
  SDA -> PIN A4

  LCD I2C
  SCL -> PIN A5
  SDA -> PIN A4

  LED Rojo  -> PIN 2
  LED Verde -> PIN 3

  Keypad
  PINES 5 a 12

  Sensor Tierra
  AC -> PIN ANALOG A0

  Rele Activar Electrovalvula
  IN -> PIN 13
-------------------------------------------
*/

#define pinLedVerde 3
#define pinLedRojo 2

#define pinDht 4

#define pinRele 13

#define pinSensorTierra A0

const byte FILAS = 4;
const byte COLUMNAS = 4;
byte pinesFilas[FILAS] = {12, 11, 10, 9};     // pines correspondientes a las filas
byte pinesColumnas[COLUMNAS] = {8, 7, 6, 5};  // pines correspondientes a las columnas
char keys[FILAS][COLUMNAS] = {
  {'D', 'C', 'B', 'A'},
  {'#', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};

//LCD
LiquidCrystal_I2C lcd (0x27, 2, 1, 0, 4, 5, 6, 7); // DIR, E, RW, RS, D4, D5, D6, D7

//Sensor Temp Humd DHT11
DHT dht(pinDht, DHT11);

//Reloj
RTC_DS3231 rtc;

//Keypad Teclado
Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);  // crea objeto

//Variables Globales
int humd = 0;
int temp = 0;

//millis()
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 1000;

//Keypad
char TECLA;       // almacena la tecla presionada
char index = 'A';       //
char indexSub;    //
char CLAVE[2];        // almacena en un array 6 digitos ingresados
byte INDICE = 0;      // indice del array

//Variables Alarma
int alarma[] = {18, 38}; //{hour,minutes}
int existsAlarm = false;
int indexAlarma = 0;

//Variables Humedad Tierra
int humedadTierra[] = {80, 30}; // {empieza, termina}

//Variables de configuracion
bool modificarHumedad = false;
bool modificarAlarma = false;

bool modificarStartHumedad = false;
bool modificarEndHumedad = false;

bool modificarMinutos = false;
bool modificarHora = false;

void setup() {
  Serial.begin(9600);

  //Inicializar sensor DHT11 y Reloj RTC3231
  dht.begin();
  rtc.begin();

  //Establecer fecha y horario RTC3231
  rtc.adjust(DateTime(__DATE__, __TIME__));

  //Definimos funciones del LCD I2C
  lcd.setBacklightPin(3, POSITIVE); // puerto P3 de PCF8574 como positivo
  lcd.setBacklight(HIGH);   // habilita iluminacion posterior de LCD
  lcd.begin(16, 2);     // 16 columnas por 2 lineas para LCD 1602A
  lcd.clear();      // limpia pantalla

  //Establecer los pines como salida
  pinMode(pinLedVerde, OUTPUT);
  pinMode(pinLedRojo, OUTPUT);
  pinMode(pinRele, OUTPUT);

  startMillis = millis();
}

void loop() {
  // Obtiene tecla presionada y asigna a variable
  TECLA = teclado.getKey();   
  
  //Obtenemos los datos del DHT11
  temp = dht.readTemperature();
  humd = dht.readHumidity();

  //Si existe una TECLA presionada
  if (TECLA) {
    if (!modificarHumedad && !modificarAlarma) {
      if (index != 'D' && index != 'C') {
        index = TECLA;
        Serial.print("INDEX: ");
        Serial.println(index);
      }
      else {
        indexSub = TECLA;
        Serial.print("INDEX SUB: ");
        Serial.println(indexSub);
      }
    } else {
      CLAVE[INDICE] = TECLA;    // almacena en array la tecla presionada
      INDICE++;       // incrementa indice en uno
    }

  }
  if (modificarHumedad) {
    lcd.setCursor(0, 0);
    lcd.print("Str Humedad: ");

    lcd.setCursor(0, 1);
    lcd.print("End Humedad: ");

    modificarStartHumedad = true;
  }
  if (modificarAlarma) {
    lcd.setCursor(0, 0);
    lcd.print("Hora: ");

    lcd.setCursor(0, 1);
    lcd.print("Minutos: ");

    modificarHora = true;
  }
  if (INDICE == 2) {
    if (modificarHumedad) {
      if (modificarEndHumedad) {

        lcd.setCursor(13, 1);
        lcd.print(CLAVE[0]);
        lcd.print(CLAVE[1]);

        humedadTierra[1] = getValue();
        delay(2000);

        //humedadTierra[1] = 20;
        modificarEndHumedad = false;
        modificarStartHumedad = false;
        modificarHumedad = false;
        index = 'A';
        indexSub = '0';

      }
      if (modificarStartHumedad) {
        lcd.setCursor(13, 0);
        lcd.print(CLAVE[0]);
        lcd.print(CLAVE[1]);
        
        humedadTierra[0] = getValue();
        modificarStartHumedad = false;
        modificarEndHumedad = true;
      }
    }

    if (modificarAlarma) {
      if (modificarMinutos) {

        lcd.setCursor(9, 1);
        lcd.print(CLAVE[0]);
        lcd.print(CLAVE[1]);
        alarma[1] = getValue();
        delay(2000);
        
        modificarHora = false;
        modificarMinutos = false;
        modificarAlarma = false;
        index = 'A';
        indexSub = '0';
      }
      if (modificarHora) {
        lcd.setCursor(6, 0);
        lcd.print(CLAVE[0]);
        lcd.print(CLAVE[1]);
        alarma[0] = getValue();
        
        modificarHora = false;
        modificarMinutos = true;
      }
    }
    INDICE = 0;
  }

  currentMillis = millis();  //get the current time
  if ( (currentMillis - startMillis) >= period && !modificarHumedad && !modificarAlarma) { //test whether the period has elapsed
    if (index == 'A') {
      lcd.clear();      // limpia pantalla
      mostrarTiempoLcd();
      mostrarTemperaturaLcd();
      mostrarHumedadTierra();
    }

    if (index == 'B') {
      lcd.clear();      // limpia pantalla
      lcd.setCursor(0, 0);
      lcd.print("Regando: ");
      lcd.setCursor(8, 0);

      isRegando() ? lcd.print("SI") : lcd.print("NO");
    }

    //Modificar el horario del sistema de riego
    if (index == 'C') {
      lcd.clear();      // limpia pantalla

      lcd.setCursor(0, 0);
      lcd.print("1. Ver hr. Riego");

      lcd.setCursor(0, 1);
      lcd.print("2. Mod hr. Riego");

      if (indexSub == '1') {
        lcd.clear();      // limpia pantalla

        lcd.setCursor(0, 0);
        lcd.print("Hora: ");
        lcd.print(alarma[0]);

        lcd.setCursor(0, 1);
        lcd.print("Minutos: ");
        lcd.print(alarma[1]);
      }
      if (indexSub == '2') {
        lcd.clear();      // limpia pantalla
        modificarAlarma = true;
      }
      if (indexSub == 'A') {
        index = 'A';
        indexSub = '0';
      }
      if (indexSub == 'B') {
        index = 'B';
        indexSub = '0';
      }
      if (indexSub == 'D') {
        index = 'D';
        indexSub = '0';
      }
      if (indexSub == '*') {
        index = 'A';
        indexSub = '0';
      }
    }

    //Modificar de que humedad empezar y de que humedad terminar el sistema de riego
    if (index == 'D') {
      lcd.clear();      // limpia pantalla

      lcd.setCursor(0, 0);
      lcd.print("1. humedad Riego");

      lcd.setCursor(0, 1);
      lcd.print("2. Mod h. Riego");

      if (indexSub == '1') {
        lcd.clear();      // limpia pantalla

        lcd.setCursor(0, 0);
        lcd.print("Str Humedad: ");
        lcd.print(humedadTierra[0]);

        lcd.setCursor(0, 1);
        lcd.print("End Humedad: ");
        lcd.print(humedadTierra[1]);

      }
      if (indexSub == '2') {
        lcd.clear();      // limpia pantalla
        modificarHumedad = true;
      }
      if (indexSub == 'A') {
        index = 'A';
        indexSub = '0';
      }
      if (indexSub == 'B') {
        index = 'B';
        indexSub = '0';
      }
      if (indexSub == 'C') {
        index = 'C';
        indexSub = '0';
      }
      if (indexSub == '*') {
        index = 'A';
        indexSub = '0';
      }
    }

    if (index == '#') {
      digitalWrite(pinRele, !digitalRead(pinRele));
      index = 'B';
    }

    startMillis = currentMillis;
  }

  if (rtc.now().hour() == alarma[0] && rtc.now().minute() == alarma[1]) {
    existsAlarm = true;
  }
  if (existsAlarm) {
    activarLedRojo(LOW);
    activarLedVerde(HIGH);
  } else {
    activarLedVerde(LOW);
    activarLedRojo(HIGH);
  }

  if (humedadTierra[1] > getSensorTierra()) {
    existsAlarm = false;
  }
}

bool isRegando() {
  return digitalRead(pinRele) == HIGH;
}

void activarRele(int estado) {
  digitalWrite(pinRele, estado == HIGH ? HIGH : LOW);
}

void activarLedVerde(int estado) {
  digitalWrite(pinLedVerde, estado == HIGH ? HIGH : LOW);
}

void activarLedRojo(int estado) {
  digitalWrite(pinLedRojo, estado == HIGH ? HIGH : LOW);
}

void mostrarTemperaturaLcd() {
  //Mostrar en el LCD
  lcd.setCursor(0, 1);
  lcd.print("TA:");

  lcd.setCursor(4, 1);
  lcd.print(temp);

  lcd.setCursor(9, 1);
  lcd.print("HA:");

  lcd.setCursor(12, 1);
  lcd.print(humd);
  lcd.print("%");

  /*
    Serial.println("-----------------");
    Serial.print(temp);
    Serial.print(" C");
    Serial.println();
    Serial.print(humd);
    Serial.print(" %");
    Serial.println();
    Serial.println("-----------------");
  */
}

void mostrarTiempoLcd() {
  DateTime fecha = rtc.now(); // funcion que devuelve fecha y horario en formato

  lcd.setCursor(0, 0);
  lcd.print(fecha.hour());

  lcd.setCursor(2, 0);
  lcd.print(":");

  lcd.setCursor(3, 0);
  lcd.print(fecha.minute());

  lcd.setCursor(5, 0);
  lcd.print(":");

  lcd.setCursor(6, 0);
  lcd.print(fecha.second());
}

void mostrarHumedadTierra() {
  int humedad = analogRead(pinSensorTierra) / 10;
  lcd.setCursor(9, 0);
  lcd.print("HT: ");

  lcd.setCursor(12, 0);
  lcd.print(humedad);
}

int getSensorTierra() {
  return (analogRead(pinSensorTierra) / 10);
}

int getValue() {
  String strNumero;
  for(int i=0 ; i<2; i++){
    strNumero += CLAVE[i];
  }
  return strNumero.toInt();
}
