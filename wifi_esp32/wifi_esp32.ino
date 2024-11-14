#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "Aula-STEM";    // Nombre de la red WiFi
const char* password = "unalman";  // Contraseña de la red

WebServer server(80);

// Página HTML
String html = "<html><body><h1>Registro de Asistencia</h1><form action='/submit' method='POST'><label>Nombre:</label><input type='text' name='nombre'><br><label>Horas:</label><input type='number' name='horas'><br><input type='submit' value='Enviar'></form></body></html>";

// Función para manejar la página principal
void handleRoot() {
  server.send(200, "text/html", html);
}

// Función para manejar la recepción del formulario
void handleSubmit() {
  String nombre = server.arg("nombre");
  String horas = server.arg("horas");
  
  // Aquí puedes guardar los datos o enviarlos a un servicio externo
  Serial.println("Asistencia recibida: Nombre = " + nombre + ", Horas = " + horas);

  server.send(200, "text/html", "<html><body><h1>Asistencia registrada!</h1></body></html>");
}

void setup() {
  Serial.begin(115200);
  
  // Configurar el ESP32 como Punto de Acceso con contraseña
  WiFi.softAP(ssid, password);

  // Iniciar servidor y manejar rutas
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  
  server.begin();
}

void loop() {
  server.handleClient();
}
