#include <WiFi.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Configuración de Red
const char* ssid = "TESTBED";
const char* password = "cepia2022";
const char* udpAddress = "255.255.255.255"; 
const int udpPort = 12345;
 
// Configuración DS18B20
const int oneWireBus = 4; // Pin de datos de los sensores
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
 
WiFiUDP udp;
 
// Estructura de datos (empaquetada para evitar bytes de relleno/padding)
struct __attribute__((packed)) SensorData {
  uint32_t timestamp_ms; // 4 bytes
  float temp_ambiente;   // 4 bytes
  float temp_antena;     // 4 bytes
};
SensorData dataPacket;
 
// Variables de control de tiempo
unsigned long lastRequest = 0;
const int conversionTime = 95; // ~94ms necesarios para 9 bits, redondeado a 95ms
bool requestPending = false;
 
void setup() {
  Serial.begin(115200);
 
  // Iniciar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
 
  // Iniciar sensores
  sensors.begin();
  sensors.setResolution(9); // Resolución de 9 bits (0.5°C), permite lecturas cada ~94ms
 
  // CRÍTICO: Desactiva el bloqueo. requestTemperatures() retornará instantáneamente.
  sensors.setWaitForConversion(false); 
}
 
void loop() {
  unsigned long currentMillis = millis();
 
  // Paso 1: Si no hay una petición pendiente, ordenamos a los sensores que midan
  if (!requestPending) {
    sensors.requestTemperatures();
    lastRequest = currentMillis;
    requestPending = true;
  } 
  // Paso 2: Si ya pedimos la medición, revisamos si ya pasó el tiempo necesario (95ms)
  else {
    if (currentMillis - lastRequest >= conversionTime) {
      // Poblar la estructura
      dataPacket.timestamp_ms = currentMillis;
      dataPacket.temp_ambiente = sensors.getTempCByIndex(0);
      dataPacket.temp_antena = sensors.getTempCByIndex(1);
 
      // Enviar por UDP
      udp.beginPacket(udpAddress, udpPort);
      udp.write((uint8_t*)&dataPacket, sizeof(dataPacket));
      udp.endPacket();
 
      // Reiniciamos la bandera para pedir una nueva medición en el próximo ciclo
      requestPending = false;
    }
  }
 
}