# ESP32 WIFI Provisioning System

## Descripción

Sistema IoT basado en ESP32 que permite el aprovisionamiento dinámico de redes WIFI sin necesidad de reprogramar el dispositivo.

El sistema implementa un portal cautivo mediante un Access Point local que permite al usuario configurar las credenciales WIFI desde cualquier dispositivo móvil o computador.

Las credenciales son almacenadas de forma persistente en memoria no volátil usando NVS mediante la librería Preferences.

---

# Características

* Configuración WIFI dinámica
* Portal cautivo
* Access Point automático
* Reconexión automática
* Almacenamiento persistente
* Reset de configuración
* API REST básica
* Interfaz web local
* Compatible con Arduino Framework

---

# Tecnologías utilizadas

* ESP32
* Arduino IDE
* WiFi.h
* WebServer.h
* DNSServer.h
* Preferences.h

---

# Arquitectura general

El sistema funciona mediante dos modos principales:

## Modo AP

Si el ESP32 no posee credenciales almacenadas:

1. Crea una red WIFI llamada:

```text
ESP32-Setup
```

2. El usuario se conecta desde celular o computador.

3. Accede al portal web:

```text
http://192.168.4.1
```

4. Ingresa SSID y contraseña.

5. El ESP32 almacena las credenciales y reinicia.

---

## Modo STA

Si existen credenciales almacenadas:

1. El ESP32 intenta conectarse automáticamente.
2. Si la conexión es exitosa:

   * inicia modo normal
3. Si falla:

   * vuelve automáticamente a modo AP

---

# Flujo del sistema

```text
Inicio
   |
¿Hay credenciales?
   |
 ├── NO
 |     |
 |     └── Iniciar AP
 |             |
 |      Portal cautivo
 |             |
 |      Guardar WIFI
 |             |
 |        Reiniciar
 |
 └── SI
        |
   Conectar WIFI
        |
   +----+----+
   |         |
 Éxito     Error
   |         |
 Normal    Volver AP
```

---

# Endpoints

---

## GET /

Retorna la interfaz web de configuración.

### Response

```http
200 OK
Content-Type: text/html
```

---

## POST /save

Guarda las credenciales WIFI.

### Payload

```text
ssid=MiRed
password=12345678
```

### Response

```http
200 OK
```

---

## POST /reset

Elimina las credenciales almacenadas y reinicia el dispositivo.

### Response

```json
{
  "message": "Configuracion eliminada"
}
```

---

# Memoria persistente

El almacenamiento de credenciales se realiza mediante NVS (Non Volatile Storage) utilizando la librería Preferences.

Esto permite:

* conservar credenciales tras reinicios
* evitar reprogramar el dispositivo
* almacenamiento seguro en flash

---

# Reconexión automática

Al iniciar, el ESP32:

1. Lee credenciales guardadas
2. Intenta conexión WIFI
3. Si falla:

   * vuelve automáticamente a modo AP

---

# Restablecimiento de configuración

El sistema permite borrar credenciales mediante:

* endpoint `/reset`
* botón físico de reset

Esto permite configurar nuevas redes WIFI sin reprogramar el ESP32.

---

# Validación funcional

## Caso 1 — Primer arranque

Resultado esperado:

* ESP32 crea AP
* Portal accesible
* Configuración exitosa

---

## Caso 2 — WIFI válida

Resultado esperado:

* conexión automática
* IP asignada
* modo normal

---

## Caso 3 — WIFI inválida

Resultado esperado:

* timeout de conexión
* regreso automático a modo AP

---

## Caso 4 — Reset

Resultado esperado:

* credenciales borradas
* reinicio automático
* nueva configuración posible

---

# Comparación con WiFiManager

| Característica     | Implementación propia | WiFiManager |
| ------------------ | --------------------- | ----------- |
| Dependencias       | Bajas                 | Altas       |
| Tamaño Flash       | Menor                 | Mayor       |
| Control del código | Completo              | Parcial     |
| Complejidad        | Baja                  | Media       |
| Portal cautivo     | Sí                    | Sí          |

---

# Seguridad Enterprise PEAP

El ESP32 soporta WPA2 Enterprise/PEAP mediante funciones incluidas en ESP-IDF.

Se requiere:

* SSID
* usuario
* contraseña
* identidad enterprise
* certificados opcionales

---

# Clientes simultáneos

La librería WebServer funciona de forma síncrona y soporta aproximadamente entre 4 y 5 clientes concurrentes ligeros.

Alternativas más escalables:

* ESPAsyncWebServer
* AsyncTCP
* ESP-IDF HTTP Server

---

# Ejecución

## Requisitos

* ESP32
* Arduino IDE
* Drivers USB instalados

---

## Compilación

1. Abrir proyecto
2. Seleccionar placa ESP32
3. Compilar
4. Subir firmware

---

# Uso

1. Encender ESP32
2. Conectarse a:

```text
ESP32-Setup
```

3. Abrir:

```text
http://192.168.4.1
```

4. Configurar WIFI

5. Esperar reinicio automático

