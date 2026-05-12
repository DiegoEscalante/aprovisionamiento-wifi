#include <WiFi.h> // Manejo de wifi en ESP32
#include <WebServer.h> // Servidor HTTP básico 
#include <DNSServer.h> // Permite crear un portal cautivo (página al conectarse a la red)
#include <Preferences.h> // Manejo de memoria no volátil (NVS)

Preferences preferences;
// Escucha en el puerto 80
WebServer server(80); 
DNSServer dnsServer;

String ssid;
String password;

// Puerto estándar usado por DNS
const byte DNS_PORT = 53;



bool connectToWiFi() {
  
  /// Intenta conectarse a una red wifi usando las credenciales guardadas previamente.
  /// Retorna:
  /// true -> conexión exitosa
  /// false -> no hay credenciales o falló la conexión

    preferences.begin("wifi", true);
    

    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");

    preferences.end();

    // Si el SSID está vacío significa que nunca se configuró el wifi, retorna false
    if (ssid == "") {
        return false;
    }

    // Configura ESP32 en modo estación (cliente WiFi)
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.println("Conectando a WiFi...");

    int attempts = 0;

    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado!");
        // Muestra IP local asignada por el router
        Serial.println(WiFi.localIP());
        return true;
    }

    return false;
}

void startAccessPoint() {
  
  ///Inicia el ESP32 como punto de acceso y crea un portal web para configurar wifi.

    // Configura el ESP32 en modo AP
    WiFi.mode(WIFI_AP);

    // Crea la red WiFi con el nombre especificado
    WiFi.softAP("ESP32-Setup", "123456789");

    // Obtiene la IP del ESP32 en modo AP
    IPAddress IP = WiFi.softAPIP();

    Serial.println("AP iniciado");
    Serial.println(IP);

    // Configura el servidor DNS para redirigir cualquier dominio solicitado hacia el ESP32
    dnsServer.start(DNS_PORT, "*", IP);


    /// Endpoint: GET /
    /// Muestra la página HTML de configuración
    server.on("/", HTTP_GET, []() {

        String html = R"rawliteral(
        <html>
        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1">
        </head>
        <body>
            <h2>Configuracion WiFi ESP32</h2>
            <form action="/save" method="POST">
                SSID:<br>
                <input type="text" name="ssid"><br>
                Password:<br>
                <input type="password" name="password"><br><br>
                <input type="submit" value="Guardar">
            </form>
        </body>
        </html>
        )rawliteral";

        server.send(200, "text/html", html);
    });

    
    /// Endpoint: POST /save
    /// Guarda las credenciales wifi enviadas desde el formulario HTML
    server.on("/save", HTTP_POST, []() {

        String newSSID = server.arg("ssid");
        String newPassword = server.arg("password");

        preferences.begin("wifi", false);

        preferences.putString("ssid", newSSID);
        preferences.putString("password", newPassword);

        preferences.end();

        server.send(200, "text/html",
                    "<h1>Credenciales guardadas. Reiniciando...</h1>");

        delay(2000);

        ESP.restart();
    });
    
    /// Endpoint: POST /reset
    /// Elimina las credenciales wifi almacenadas.
    server.on("/reset", HTTP_POST, []() {

        preferences.begin("wifi", false);

        preferences.clear();

        preferences.end();

        server.send(200, "application/json",
                    "{\"message\":\"Configuracion eliminada\"}");

        delay(1000);

        ESP.restart();
    });

    // --- LA SOLUCIÓN AQUÍ ---
    // Atrapa las peticiones de comprobación de red de Android/iOS/Windows
    server.onNotFound([]() {
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server.send(302, "text/plain", "");
    });

    server.begin();
}
void setup() {

    Serial.begin(115200);

    bool connected = connectToWiFi();

    // Si no se logró conectar, inicia el portal de configuración
    if (!connected) {
        startAccessPoint();
    } else {
        
        /// Endpoint: GET /
        /// Endpoint para confirmar que el dispositivo está conectado
        server.on("/", HTTP_GET, []() {
            server.send(200, "application/json",
                        "{\"status\":\"connected\"}");
        });

        server.begin();
    }
}

void loop() {

    dnsServer.processNextRequest();
    server.handleClient();
}