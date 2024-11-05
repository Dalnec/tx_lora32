//Librerias OLED Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//Librerias LoRa
#include <LoRa.h>
#include <SPI.h>

//pines que se utilizaran por el modulo LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// Frecuencia de Operacion LoRa
#define BAND 915E6

#define SCREEN_WIDTH 128  // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64  // Alto de la pantalla OLED

// Dirección I2C de la pantalla (puede variar, comúnmente 0x3C o 0x3D)
#define OLED_RESET -1  // Compartimiento de reset (-1 si no se usa un pin de reset)
#define SCREEN_ADDRESS 0x3C

// Sensor JSN-SR04T
#define TRIG_PIN 14
#define ECHO_PIN 13
// #define ALTURA_MAXIMA_TANQUE 400
int emptyTankDistance = 600 ;  //Distance when tank is empty
int fullTankDistance =  30 ;  //Distance when tank is full

float duration;
float distance;
float averageDistance;
float sum = 0;
int   percentage;
int numReadings = 50;
float alpha = 0.1;  // Factor de suavizado (entre 0 y 1)

float batteryLevel;
const int numBatteryReadings = 10;
int readings[numBatteryReadings];      // La matriz de lecturas
int readIndex = 0;              // El índice en la matriz
int total = 0;                  // El total acumulado
int average = 0;                // El promedio
const int BATTERY_LEVEL_PIN = 34; 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);  //inicia monitor serial
  init_screen();
  init_lora();

  init_battery_measuring();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}


void loop() {
  measure_battery_level();
  measure_distance();
  // Limitar la distancia al tamaño del tanque
  // if (distancia > ALTURA_MAXIMA_TANQUE) {
  //   distancia_calculada = ALTURA_MAXIMA_TANQUE;
  // } else {
  //   distancia_calculada = distancia;
  // }

  // if (porcentaje < 0) {
  //   porcentaje = 0;
  // }
  // if (porcentaje > 100) {
  //   porcentaje = 100;
  // }

  // Concatenar y enviar
  send_data_lora(String(averageDistance), String(percentage), String(batteryLevel));

  // Mostrar datos en pantalla
  // show_info_screen(String(distancia_calculada) + " cm", String(porcentaje) + "%");
  show_info_screen2(String((int)averageDistance) + " cm", String(percentage) + "%", String((int)batteryLevel) + "%");
}

void init_screen() {
  // Inicializa la pantalla con la dirección I2C
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Fallo en la asignación de SSD1306"));
    for (;;)
      ;  // Detiene la ejecución si la pantalla no se inicia correctamente
  }
  // Limpia la pantalla
  display.clearDisplay();
  // Configura el tamaño del texto
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
}

void init_lora() {
  Serial.println("INICIO TRANSMISOR LoRa");
  delay(2000);
  //Definimos pines SPI
  SPI.begin(SCK, MISO, MOSI, SS);
  //Configuramos LoRa para enviar
  LoRa.setPins(SS, RST, DIO0);

  //Intenta transmitir en la banda elegida
  if (!LoRa.begin(BAND)) {
    //Si no puede transmitir, marca error
    Serial.println("Error iniciando LoRa");
    while (1)
      ;
  }
  //Mensaje de todo bien en puerto serial
  Serial.println("Inicio exitoso LoRa!");
  display.setCursor(0, 10);
  //Mensaje de todo bien en pantalla OLED
  display.print("Inicio exitoso LoRa!");
  display.display();
  delay(2000);
}

void init_battery_measuring() {
  analogReadResolution(12);  // Configura el ADC a 12 bits
  analogSetWidth(12);        // Asegura que el ADC esté leyendo a 12 bits (0 - 4095)
  analogSetAttenuation(ADC_11db);  // Configura la atenuación para medir hasta 3.3V correctamente
  
  // Inicializar todas las lecturas a 0
  for (int i = 0; i < numBatteryReadings; i++) {
    readings[i] = 0;
  }
}

void send_data_lora(String distancia, String porcentaje, String battery) {
  //Para mandar paquete al LoRa receptor
  //Inicia protocolo
  LoRa.beginPacket();
  //cancatenar valores leidos a enviar
  LoRa.print(distancia);
  LoRa.print(",");
  LoRa.print(porcentaje);
  LoRa.print(",");
  LoRa.print(battery);
  //Fin de paquete enviado
  LoRa.endPacket();
}

void measure_distance() {
  sum = 0;
  averageDistance = 0;
  for (int i = 0; i < numReadings; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(20);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH, 35000);
    distance = ((duration / 2) * 0.343) / 10;

    // Aplicar suavizado exponencial
    averageDistance = alpha * distance + (1 - alpha) * averageDistance;

    delay(80);  // Breve retraso entre lecturas
  }

  percentage = map((int)averageDistance, emptyTankDistance, fullTankDistance, 0, 100);

  Serial.print("Distance: ");
  Serial.print(averageDistance);
  Serial.print(" cm");
  Serial.print("  >>  Porcentaje: ");
  Serial.print(percentage);
  Serial.println(" %");
}

void measure_battery_level() {
  // Restar la lectura más antigua
  total = total - readings[readIndex];
  // Leer el nuevo valor
  readings[readIndex] = analogRead(BATTERY_LEVEL_PIN);
  // Sumar el valor al total
  total = total + readings[readIndex];
  // Avanzar al siguiente valor
  readIndex = (readIndex + 1) % numBatteryReadings;
  // Calcular el promedio
  average = total / numBatteryReadings;
  // Convertir a porcentaje
  batteryLevel = map(average, 2047, 4095, 0, 100);
  // Mostrar el valor
  Serial.print("Promedio: ");
  Serial.println(batteryLevel);
}

void show_info_screen2(String distancia, String porcentaje, String batteryLevel) {
  //Limpia pantalla
  display.clearDisplay();
  //Posicionamos en siguiente renglon
  display.setTextSize(2);
  display.setCursor(45, 5);
  display.print(distancia);
  display.setCursor(20, 25);
  display.print("AGUA");
  display.setCursor(75, 25);
  display.print(porcentaje);
  display.setCursor(55, 45);
  display.print(batteryLevel);
  display.display();
}

void show_info_screen(String distancia, String porcentaje) {
  //Limpia pantalla
  display.clearDisplay();
  //Posicionamos en siguiente renglon
  display.setCursor(0, 0);
  //Tamaño de fuente a 1 punto
  display.setTextSize(1);
  display.print("TRANSMISOR LoRa");
  display.setCursor(0, 15);
  display.setTextSize(1.5);
  display.print("Paquete enviado:");
  display.setCursor(0, 30);
  display.print(" Distancia");
  display.setCursor(80, 30);
  display.print(distancia);
  display.setCursor(0, 45);
  display.print(" Porcentaje");
  display.setCursor(80, 45);
  display.print(porcentaje);
  display.display();
}

void show_info_battery(String valor) {
  //Limpia pantalla
  display.clearDisplay();
  //Posicionamos en siguiente renglon
  display.setTextSize(2);
  display.setCursor(30, 10);
  display.print(valor);
  display.display();
}