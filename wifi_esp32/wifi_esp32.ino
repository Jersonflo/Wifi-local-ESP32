#include <WiFi.h>
#include <FS.h>         // Para trabajar con el sistema de archivos
#include <SPIFFS.h>     // Sistema de archivos para ESP32
#include <WebServer.h>  // Servidor web

// Configuración WiFi
const char* ssid = "Aula-STEM";
const char* password = "unalman";

// Configuración del servidor
WebServer server(80);

// Página HTML con formulario
String html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <style>
      body {
        font-family: Arial, sans-serif;
        text-align: center;
        margin-top: 50px;
        background-color: #f4f4f4;
      }
      form {
        display: inline-block;
        padding: 20px;
        background: white;
        border-radius: 10px;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
      }
      input {
        margin: 10px 0;
        padding: 9px;
        width: 90%;
        max-width: 300px;
      }
      input[type="submit"] {
        background: #007bff;
        color: white;
        border: none;
        cursor: pointer;
      }
    </style>
  </head>
  <body>
    <h1>Registro de Asistencia</h1>
    <form action='/submit' method='POST'>
      <input type='text' name='nombre' placeholder='Ingresa tu nombre' required><br>
      <input type='number' name='horas' placeholder='Horas asistidas' required><br>
      <input type='date' name='fecha' required><br>
      <input type='submit' value='Registrar'>
    </form>
  </body>
</html>
)rawliteral";

// Función para inicializar SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al inicializar SPIFFS");
    return;
  }
  Serial.println("SPIFFS inicializado");
}

// Función para manejar la página principal
void handleRoot() {
  server.send(200, "text/html", html);
}

// Función para manejar el envío del formulario
void handleSubmit() {
  String nombre = server.arg("nombre");
  String horas = server.arg("horas");
  String fecha = server.arg("fecha");

  // Validar duplicados
  if (isDuplicate(nombre, fecha)) {
    server.send(200, "text/html", "<h1>Ya registraste tu asistencia para esta fecha!</h1>");
    return;
  }

  // Guardar datos en el archivo CSV
  File file = SPIFFS.open("/asistencia.csv", FILE_APPEND);
  if (!file) {
    Serial.println("Error al abrir el archivo");
    server.send(500, "text/plain", "Error al guardar la asistencia");
    return;
  }

  // Escribir los datos en formato CSV
  file.printf("%s,%s,%s\n", nombre.c_str(), horas.c_str(), fecha.c_str());
  file.close();

  server.send(200, "text/html", "<h1>Asistencia registrada correctamente!</h1>");
  Serial.printf("Datos guardados: %s, %s, %s\n", nombre.c_str(), horas.c_str(), fecha.c_str());
}

// Verificar duplicados
bool isDuplicate(String nombre, String fecha) {
  File file = SPIFFS.open("/asistencia.csv", FILE_READ);
  if (!file) {
    return false;  // No hay duplicados si el archivo no existe
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.indexOf(nombre + "," + fecha) != -1) {
      file.close();
      return true;  // Se encontró un duplicado
    }
  }
  file.close();
  return false;
}

// Función para leer el archivo CSV (opcional, para depuración)
void readCSV() {
  File file = SPIFFS.open("/asistencia.csv", FILE_READ);
  if (!file) {
    Serial.println("No hay datos registrados");
    return;
  }

  Serial.println("Datos registrados:");
  while (file.available()) {
    Serial.println(file.readStringUntil('\n'));
  }
  file.close();
}

void setup() {
  Serial.begin(115200);

  // Inicializar SPIFFS
  initSPIFFS();

  // Configurar WiFi
  WiFi.softAP(ssid, password);
  Serial.println("Punto de acceso iniciado");
  Serial.println("IP: " + WiFi.softAPIP().toString());

  // Configurar rutas del servidor
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);

  server.begin();
}

void loop() {
  server.handleClient();
}
