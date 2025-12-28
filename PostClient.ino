#include "M5CoreS3.h" 
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <DHT.h> 
#include <WebServer.h>

// --- Configuration Reseau ---
const char* ssid = "MyHouseOS";
const char* password = "12345678";
const char* authUrl  = "http://192.168.4.1/link";
const char* tempUrl  = "http://192.168.4.2:3000/temp";
const char* ledUrl   = "http://192.168.4.2:3000/toggle/light";
const char* doorUrl  = "http://192.168.4.2:3000/toggle/door";
const char* heatUrl  = "http://192.168.4.2:3000/toggle/heat";

// --- Design UI (Importé du Server) ---
#define COLOR_BG      CoreS3.Display.color565(15, 23, 42)
#define COLOR_CARD    CoreS3.Display.color565(30, 41, 59)
#define COLOR_ACCENT  CoreS3.Display.color565(99, 102, 241)
#define COLOR_TEXT    CoreS3.Display.color565(248, 250, 252)
#define COLOR_SUBTEXT CoreS3.Display.color565(148, 163, 184)
#define COLOR_SUCCESS CoreS3.Display.color565(34, 197, 94)
#define COLOR_WARNING CoreS3.Display.color565(251, 146, 60)

// --- Variables Globales ---
String Token = "";
String DeviceID = "307D68F23A08";
int currentMenu = 1; 
bool autoSendMeteo = false;
unsigned long lastMeteoTime = 0;
WebServer server(80);

#define DHTPIN 17
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); 

// --- Fonctions WebServer ---
void handleRoot() {
    server.send(200, "text/html", 
        "<html><head><title>M5 Screen</title>"
        "<script>setInterval(() => { document.querySelector('img').src = '/capture?t=' + Date.now(); }, 3000);</script>"
        "<style>body{background:#0f172a;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;}"
        "img{border:4px solid #6366f1;border-radius:10px;box-shadow:0 0 20px rgba(99,102,241,0.5);}"
        "</style></head>"
        "<body><img src='/capture' /></body></html>");
}

void handleCapture() {
    int w = CoreS3.Display.width();
    int h = CoreS3.Display.height();
    
    uint32_t imageSize = w * h * 3;
    uint32_t fileSize = 54 + imageSize;
    
    uint8_t header[54] = {
        0x42, 0x4D, 
        (uint8_t)(fileSize), (uint8_t)(fileSize >> 8), (uint8_t)(fileSize >> 16), (uint8_t)(fileSize >> 24),
        0, 0, 0, 0, 
        54, 0, 0, 0,
        40, 0, 0, 0,
        (uint8_t)(w), (uint8_t)(w >> 8), (uint8_t)(w >> 16), (uint8_t)(w >> 24),
        (uint8_t)(-h), (uint8_t)(-h >> 8), (uint8_t)(-h >> 16), (uint8_t)(-h >> 24),
        1, 0, 
        24, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0, 
        0, 0, 0, 0
    };

    WiFiClient client = server.client();
    client.write("HTTP/1.1 200 OK\r\nContent-Type: image/bmp\r\nConnection: close\r\n\r\n");
    client.write(header, 54);

    uint16_t* lineBuffer = (uint16_t*)malloc(w * 2);
    uint8_t* rgbBuffer = (uint8_t*)malloc(w * 3);

    if (lineBuffer && rgbBuffer) {
        for (int y = 0; y < h; y++) {
            CoreS3.Display.readRect(0, y, w, 1, lineBuffer);
            for (int x = 0; x < w; x++) {
                uint16_t p = lineBuffer[x];
                p = (p >> 8) | (p << 8); // Byte Swap
                
                rgbBuffer[x*3] = (p & 0x001F) << 3;       // Blue
                rgbBuffer[x*3+1] = (p & 0x07E0) >> 3;     // Green
                rgbBuffer[x*3+2] = (p & 0xF800) >> 8;     // Red
            }
            client.write(rgbBuffer, w * 3);
        }
    }
    
    if(lineBuffer) free(lineBuffer);
    if(rgbBuffer) free(rgbBuffer);
}

// --- Fonctions de Dessin ---
void drawCard(int x, int y, int w, int h) {
    CoreS3.Display.fillRoundRect(x, y, w, h, 12, COLOR_CARD);
}

void drawHeader(String title) {
    CoreS3.Display.fillScreen(COLOR_BG);
    drawCard(10, 10, 300, 40);
    CoreS3.Display.setTextColor(COLOR_ACCENT);
    CoreS3.Display.setTextDatum(MC_DATUM);
    CoreS3.Display.setTextFont(2);
    CoreS3.Display.drawString(title, 160, 30);
}

void drawMenu() {
    CoreS3.Display.startWrite();
    
    if (currentMenu == 1) {
        drawHeader("SYSTEM & AUTH");
        
        drawCard(10, 60, 145, 70); // Bouton A
        CoreS3.Display.setTextColor(COLOR_SUBTEXT);
        CoreS3.Display.drawString("BTN A", 82, 75);
        CoreS3.Display.setTextColor(COLOR_TEXT);
        CoreS3.Display.drawString("AUTHENTICATE", 82, 105);

        drawCard(165, 60, 145, 70); // Bouton B
        CoreS3.Display.setTextColor(COLOR_SUBTEXT);
        CoreS3.Display.drawString("BTN B", 237, 75);
        CoreS3.Display.setTextColor(COLOR_ACCENT);
        CoreS3.Display.drawString("NEXT MENU >>", 237, 105);

        drawCard(10, 140, 300, 60); // Bouton C
        CoreS3.Display.setTextColor(COLOR_SUCCESS);
        CoreS3.Display.drawString("BTN C: STATUS CHECK", 160, 170);
    } 
    else if (currentMenu == 2) {
        drawHeader("CLIMATE & LIGHTS");
        
        drawCard(10, 60, 145, 70); // Bouton A
        CoreS3.Display.setTextColor(COLOR_SUBTEXT);
        CoreS3.Display.drawString("BTN A", 82, 75);
        CoreS3.Display.setTextColor(autoSendMeteo ? COLOR_SUCCESS : COLOR_TEXT);
        CoreS3.Display.drawString(autoSendMeteo ? "AUTO: ON" : "AUTO: OFF", 82, 105);

        drawCard(165, 60, 145, 70); // Bouton B
        CoreS3.Display.setTextColor(COLOR_SUBTEXT);
        CoreS3.Display.drawString("BTN B", 237, 75);
        CoreS3.Display.setTextColor(COLOR_ACCENT);
        CoreS3.Display.drawString("NEXT MENU >>", 237, 105);

        drawCard(10, 140, 300, 60); // Bouton C
        CoreS3.Display.setTextColor(COLOR_WARNING);
        CoreS3.Display.drawString("BTN C: TOGGLE LED", 160, 170);
    }
    else {
        drawHeader("SECURITY & HEAT");
        
        drawCard(10, 60, 145, 70); // Bouton A
        CoreS3.Display.setTextColor(COLOR_SUBTEXT);
        CoreS3.Display.drawString("BTN A", 82, 75);
        CoreS3.Display.setTextColor(COLOR_TEXT);
        CoreS3.Display.drawString("DOOR LOCK", 82, 105);

        drawCard(165, 60, 145, 70); // Bouton B
        CoreS3.Display.setTextColor(COLOR_SUBTEXT);
        CoreS3.Display.drawString("BTN B", 237, 75);
        CoreS3.Display.setTextColor(COLOR_ACCENT);
        CoreS3.Display.drawString("BACK TO M1 >>", 237, 105);

        drawCard(10, 140, 300, 60); // Bouton C
        CoreS3.Display.setTextColor(COLOR_ACCENT);
        CoreS3.Display.drawString("BTN C: HEAT CONTROL", 160, 170);
    }

    // Info barre en bas
    CoreS3.Display.setTextFont(1);
    CoreS3.Display.setTextColor(Token == "" ? COLOR_WARNING : COLOR_SUCCESS);
    CoreS3.Display.drawString(Token == "" ? "NOT LINKED" : "LINKED OK", 160, 220);
    
    CoreS3.Display.endWrite();
}

// --- Logique de Feedback ---
void showStatus(String msg, uint16_t color) {
    drawCard(40, 80, 240, 80);
    CoreS3.Display.setTextColor(color);
    CoreS3.Display.setTextFont(2);
    CoreS3.Display.drawString(msg, 160, 120);
    delay(1500);
    drawMenu();
}

void setup() {
    auto cfg = M5.config();
    CoreS3.begin(cfg);
    Serial.begin(115200);
    dht.begin(); 

    CoreS3.Display.fillScreen(COLOR_BG);
    CoreS3.Display.setTextColor(COLOR_SUBTEXT);
    CoreS3.Display.setTextDatum(MC_DATUM);
    CoreS3.Display.drawString("CONNECTING WIFI...", 160, 120);

    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    server.on("/", handleRoot);
    server.on("/capture", handleCapture);
    server.begin();

    Serial.println("");
    Serial.print("Live Screen available at: http://");
    Serial.println(WiFi.localIP());

    drawMenu();
}

void authenticate() {
    if(WiFi.status() != WL_CONNECTED) return;
    
    showStatus("PAIRING...", COLOR_ACCENT);

    HTTPClient http;
    http.begin(authUrl);
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    doc["id"] = DeviceID;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode == 200) {
        String response = http.getString();
        JsonDocument responseDoc; 
        deserializeJson(responseDoc, response);
        Token = responseDoc["token"].as<String>(); 
        showStatus("LINKED !", COLOR_SUCCESS);
    } else {
        showStatus("FAILED: " + String(httpResponseCode), COLOR_WARNING);
    }
    http.end();
}

void postMeteo(bool silent = false) {
    if (Token == "") { 
        if(!silent) showStatus("AUTH REQUIRED", COLOR_WARNING); 
        return; 
    }
    
    float h = dht.readHumidity();
    float t = dht.readTemperature(); 

    if (isnan(h) || isnan(t)) {
        if(!silent) showStatus("SENSOR ERROR", COLOR_WARNING);
        return;
    }

    JsonDocument doc;
    doc["temp"] = String(t);
    String jsonString;
    serializeJson(doc, jsonString);

    HTTPClient http;
    http.begin(tempUrl);
    http.addHeader("Authorization", DeviceID + ":" + Token);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(jsonString);
    if (!silent) {
        if(code == 200) showStatus("SENT: " + String(t) + "C", COLOR_SUCCESS);
        else showStatus("HTTP ERR: " + String(code), COLOR_WARNING);
    }
    
    http.end();
}

void genericPost(String url, String label) {
    if (Token == "") { showStatus("AUTH REQUIRED", COLOR_WARNING); return; }
    
    HTTPClient http;
    http.begin(url);
    http.addHeader("Authorization", DeviceID + ":" + Token);
    http.addHeader("Content-Type", "application/json");
    
    JsonDocument doc;
    doc["id"] = DeviceID;
    doc["token"] = Token;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    int code = http.POST(jsonString);
    if(code == 200) showStatus(label + " OK", COLOR_SUCCESS);
    else showStatus("ERR: " + String(code), COLOR_WARNING);
    http.end();
}

void loop() {
    M5.update();
    server.handleClient();

    if (autoSendMeteo && (millis() - lastMeteoTime > 5000)) {
        lastMeteoTime = millis();
        postMeteo(true);
    }

    if (M5.BtnA.wasPressed()) {
        if (currentMenu == 1) authenticate();
        else if (currentMenu == 2) {
            autoSendMeteo = !autoSendMeteo;
            drawMenu();
        }
        else if (currentMenu == 3) genericPost(doorUrl, "DOOR");
    }

    if (M5.BtnB.wasPressed()) {
        currentMenu++;
        if (currentMenu > 3) currentMenu = 1;
        drawMenu();
    }

    if (M5.BtnC.wasPressed()) {
        if (currentMenu == 1) showStatus("IP: " + WiFi.localIP().toString(), COLOR_TEXT);
        else if (currentMenu == 2) genericPost(ledUrl, "LED");
        else if (currentMenu == 3) genericPost(heatUrl, "HEAT");
    }

    delay(10);
}