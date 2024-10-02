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
#define ALTURA_MAXIMA_TANQUE 400

// int  Contador = 0;//Haremos un contador de paquetes enviados
long porcentaje = 0;
long distancia_calculada = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);//inicia monitor serial
  init_screen();
  init_lora();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  long duracion, distancia;
  
  // Enviar pulso de trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Leer el tiempo del pulso de echo
  duracion = pulseIn(ECHO_PIN, HIGH, 35000);
  
  // Calcular la distancia en cm
  distancia = duracion * 0.0343 / 2;
  // Serial.println();

  // distancia = pulseIn(ECHO_PIN, HIGH, 35000);
  // distancia = duracion / 58;

  Serial.println(String(distancia) + " mesuared");
  // Serial.println("********************");
  
  // if (distancia > 30) {
  // Limitar la distancia al tamaño del tanque
  if (distancia > ALTURA_MAXIMA_TANQUE) {
    distancia_calculada = ALTURA_MAXIMA_TANQUE;
  } else {
    distancia_calculada = distancia;
  }
  // Calcular el porcentaje de llenado utilizando la función map()
  porcentaje = map(distancia_calculada, ALTURA_MAXIMA_TANQUE, 25, 0, 100);
  // }

  if (porcentaje < 0) {
    porcentaje = 0;
  }
  if (porcentaje > 100) {
    porcentaje = 100;
  }
  
  // Mostrar los resultados
  // Serial.print("Distancia medida: ");
  // Serial.print(distancia_calculada);
  // Serial.println(" cm");
  
  // Serial.print("Porcentaje de llenado: ");
  // Serial.print(porcentaje);
  // Serial.println(" %");
   
  // Serial.print("Enviando paquete: ");//Muestra mensaje
  // Serial.println( Contador);
  // Concatenar y enviar
  // send_data_lora(String(distancia_calculada), String(porcentaje));
  // Mostrar datos en pantalla
  // show_info_screen(String(distancia_calculada) + " cm", String(porcentaje) + "%");

  // Contador++;
  // delay(2000);
  // delay(100);
  // show_info_screen2(String(distancia_calculada) + " cm", String(porcentaje) + "%");
  delay(50);
  // delay(1000);
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

void send_data_lora(String distancia, String porcentaje) {
  //Para mandar paquete al LoRa receptor
  //Inicia protocolo
  LoRa.beginPacket();
  //cancatenar valores leidos a enviar
  LoRa.print(distancia);
  LoRa.print(",");
  LoRa.print(porcentaje);
  //Fin de paquete enviado
  LoRa.endPacket();
}

void show_info_screen2(String distancia, String porcentaje) {
  //Limpia pantalla
  display.clearDisplay();
  //Posicionamos en siguiente renglon
  display.setTextSize(2);
  display.setCursor(40, 10);
  display.print(distancia);
  display.setTextSize(2);
  display.setCursor(20, 40);
  display.print("AGUA");
  display.setCursor(75, 40);
  display.print(porcentaje);
  display.display();
}

void show_info_screen(String distancia, String porcentaje) {
  //Limpia pantalla
  display.clearDisplay();
  //Posicionamos en siguiente renglon
  display.setCursor(0,0);
  //Tamaño de fuente a 1 punto
  display.setTextSize(1);
  display.print("TRANSMISOR LoRa");
  display.setCursor(0,15);
  display.setTextSize(1.5);
  display.print("Paquete enviado:");
  display.setCursor(0,30);
  display.print(" Distancia");
  display.setCursor(80,30);
  display.print(distancia);
  display.setCursor(0,45);
  display.print(" Porcentaje");
  display.setCursor(80,45);
  display.print(porcentaje);
  display.display();
}