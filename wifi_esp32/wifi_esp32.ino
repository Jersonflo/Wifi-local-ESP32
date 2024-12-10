#include <WiFi.h>
#include <DNSServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <time.h>

// Configuración WiFi
const char* ssid = "Aula-STEM";
const char* password = "unalman123";

// Configuración del servidor
WebServer server(80);
DNSServer dnsServer;  // Servidor DNS para el portal cautivo
const byte DNS_PORT = 53;

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
      <input type='text' name='celular' placeholder='Celular' required><br>
      <input type='email' name='correo' placeholder='Correo electronico' required><br>
      <input type='text' name='proyecto' placeholder='Proyecto' required><br>
      <input type='number' name='horas' placeholder='Horas asistidas' required><br>
      <input type='hidden' name='fecha' id='fecha'>
      <input type='submit' value='Registrar'>
    </form>
    <script>
      document.getElementById('fecha').value = new Date().toISOString().split('T')[0];
    </script>
    <p><a href="/download">Descargar archivo CSV</a></p>
    <p><a href="/delete">Eliminar todos los registros</a></p>
  </body>
</html>
)rawliteral";

// Inicializar SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al inicializar SPIFFS");
    return;
  }
  Serial.println("SPIFFS inicializado");
}

// Redirigir al formulario (Captive Portal)
void handleCaptivePortal() {
  server.sendHeader("Location", "/formulario", true); // Redirección al formulario
  server.send(302, "text/plain", "Redirigiendo...");
}

// Manejar la página raíz
void handleRoot() {
  String redirectHtml = R"rawliteral(
    <!DOCTYPE html>
    <html>
      <head>
        <meta http-equiv="refresh" content="0;url=/formulario">
      </head>
      <body>
        <p>Redirigiendo al formulario de asistencia...</p>
        <a href="/formulario">Haz clic aquí si no redirige automáticamente.</a>
      </body>
    </html>
  )rawliteral";
  server.send(200, "text/html", redirectHtml);
}

// Manejar el formulario
void handleSubmit() {
  String nombre = server.arg("nombre");
  String celular = server.arg("celular");
  String correo = server.arg("correo");
  String proyecto = server.arg("proyecto");
  String horas = server.arg("horas");
  String fecha = server.arg("fecha");

  File file = SPIFFS.open("/asistencia.csv", FILE_APPEND);
  file.printf("%s,%s,%s,%s,%s,%s\n", nombre.c_str(), celular.c_str(), correo.c_str(), proyecto.c_str(), horas.c_str(), fecha.c_str());
  file.close();

  server.send(200, "text/html", "<h1>Asistencia registrada correctamente!</h1>");
}


// Manejar la descarga del archivo CSV
void handleDownload() {
  if (!SPIFFS.exists("/asistencia.csv")) {
    server.send(404, "text/plain", "Archivo no encontrado");
    return;
  }

  File file = SPIFFS.open("/asistencia.csv", "r");
  server.streamFile(file, "text/csv");
  file.close();
}


// Manejar la eliminación de todos los registros
void handleDelete() {
  if (SPIFFS.exists("/asistencia.csv")) {
    SPIFFS.remove("/asistencia.csv");
    server.send(200, "text/html", "<h1>Registros eliminados correctamente.</h1><a href='/formulario'>Regresar</a>");
  } else {
    server.send(200, "text/html", "<h1>No hay registros para eliminar.</h1><a href='/formulario'>Regresar</a>");
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializar SPIFFS
  initSPIFFS();

  // Configurar punto de acceso
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Configurar servidor DNS para capturar todas las solicitudes
  dnsServer.start(DNS_PORT, "*", IP);

  // Configurar tiempo NTP
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "COT5", 1);
  tzset();

  // Configurar rutas del servidor web
  server.on("/", handleRoot);
  server.on("/formulario", []() {
    server.send(200, "text/html", html);
  });
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/download", handleDownload);
  server.on("/delete", handleDelete);
  server.onNotFound(handleCaptivePortal);  // Redirigir todas las solicitudes desconocidas al portal

  server.begin();
}

void loop() {
  dnsServer.processNextRequest(); // Procesar solicitudes DNS
  server.handleClient();          // Procesar solicitudes HTTP
}
