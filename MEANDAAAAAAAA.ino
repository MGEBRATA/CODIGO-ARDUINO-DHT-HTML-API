#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // Include the ArduinoJson library

// Configuración WiFi
const char* ssid = "IoTB";
const char* password = "inventaronelVAR";

// Crear un objeto de servidor web en el puerto 80
WebServer server(80);

// Pines para el sensor DHT
const int dhtPin = 5;
const int dhtType = DHT11; // Cambiar según el modelo de tu sensor
DHT dht(dhtPin, dhtType);

void setup() {
  Serial.begin(115200);
  
  // Inicializar el sensor DHT
  dht.begin();

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado a WiFi. Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configurar las rutas del servidor web
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  // Manejar solicitudes HTTP
  server.handleClient();
}

// Función para manejar la solicitud a la raíz ("/")
void handleRoot() {
  // Leer la temperatura y la humedad del sensor DHT
  float dhtTemperature = dht.readTemperature(); // Temperatura en °C
  float dhtHumidity = dht.readHumidity(); // Humedad en %
  
  // Manejar errores de lectura
  if (isnan(dhtTemperature) || isnan(dhtHumidity)) {
    dhtTemperature = 0;
    dhtHumidity = 0;
  }

  // Leer la temperatura de la API
  float apiTemperature = getApiTemperature();
  if (isnan(apiTemperature)) {
    apiTemperature = 0; // Manejar error de lectura
  }

  // Construir la respuesta HTML
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta http-equiv='refresh' content='10'/>";
  html += "<meta charset='UTF-8'>"; // Set the correct character set
  html += "<title>Temperaturas</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; background-color: #f0f8ff; text-align: center; padding: 50px;}";
  html += ".container { background-color: #ffffff; padding: 20px; border-radius: 10px; box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); max-width: 400px; margin: auto;}";
  html += "h1 { color: #333333; }";
  html += "</style></head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>Temperaturas y Humedad</h1>";
  html += "<p>Temperatura DHT: " + String(dhtTemperature) + " °C</p>";
  html += "<p>Humedad DHT: " + String(dhtHumidity) + " %</p>";
  html += "<p>Temperatura API: " + String(apiTemperature) + " °C</p>";
  html += "</div>";
  html += "</body></html>";

  server.send(200, "text/html; charset=UTF-8", html); // Specify charset in the response
}

// Función para obtener la temperatura de la API
float getApiTemperature() {
  HTTPClient http;
  http.begin("http://api.weatherapi.com/v1/current.json?key=7d072d859e1b434b9ab145357241909&q=Buenos%20Aires&aqi=no");
  int httpCode = http.GET();

  if (httpCode > 0) { // Verificar el código de respuesta
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    // Obtener la temperatura en °C
    float tempC = doc["current"]["temp_c"];
    http.end();
    return tempC;
  } else {
    http.end();
    return NAN; // Retornar NaN en caso de error
  }
}
