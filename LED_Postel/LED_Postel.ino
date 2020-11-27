#include <math.h>                     // Knihovna matematických funkcí
#include <ESP8266WiFi.h>              // Knihovna pro práci s ESP8266
#include <ESP8266WebServer.h>         // Knihovna pro práci s WebServerem
#include <WiFiClient.h>               // Knihovna pro práci s Wifi klienty
#include <Adafruit_NeoPixel.h>        // Knihovna pro práci s NeoPixel led páskem

#define PIN       D1                  // Číslo pinu, který ovládá NeoPixel LED pásek
#define NUMPIXELS 47                   // Počet připojených NeoPixel LED

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);    // Definice objektu "pixels" pro práci s NeoPixel

const char* ssid = "Network2";         // Nastavení připojení k Wifi
const char* password = "ondrout5";

byte cisloEfektu = 0;                  // proměnná pro přepínání mezi efekty
String masterHexString = "";
unsigned long milis;                   // hodnota milisekund od spuštění - pro přerušení while cyklů

ESP8266WebServer server(80);           // Iniciallizace proměnné webserver na portu 80

//Arduino funkce, která se spustí jen při startu
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  initNeoPixel ();                        // Inicializace NeoPixel LED páska a nastavení na poloviční jas

  Serial.begin(115200);                   // Zahájení komunikace po seriové lince
  delay(10);
  Serial.print("Connecting to : ");
  Serial.println(ssid);

  IPAddress staticIP(192, 168, 15, 31);       //ESP static ip
  IPAddress gateway(192, 168, 15, 1);         //IP Address of your WiFi Router (Gateway)
  IPAddress subnet(255, 255, 255, 0);         //Subnet mask
  IPAddress dns(213, 46, 172, 37);            //DNS
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  milis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - milis > 10000) break;      // Aby se nepřipojoval donekonečna přerušit po 10 sekundách připojování
    delay(500);
    Serial.print(".");
  }

  server.begin();
  Serial.println("Server started");       // informace, že server naběhl
  Serial.print("Server IP: ");            // vypise pridelenou IP adresu
  Serial.println(WiFi.localIP());

  server.on("/", homePage);               // zobrazit ovládání definované na hlavní stránce
  server.on("/barva", nastavBarvu);       // 1 - nastaví všechny diody na zvolenou barvu
  server.on("/ohen", efektOhen);          // 2 - nastaví LED pásek na simulaci ohne
  server.on("/duha", efektDuha);          // 3 - nasteví LED pásek na posouvající se duhu
  server.on("/vanoce", efektVanoce);      // 4 - nastaví LED pásek na vánoční blikání
  server.on("/rider", efektRider);        // 5 - nastaví LED pásek na Knigth Rider efekt
  server.on("/vlocky", efektVlocky);      // 6 - nastaví LED pásek na efekt Vločky
  server.onNotFound(strankaNeExistuje);   // akce pokud je zadána neexistující volba
}

// Arduino funkce, která se neustále opakuje
void loop() {
  server.handleClient();                   // pravidelné volání detekce klienta
  yield();                                 // mělo by počkat až se provedou background operace (WiFi, TCP/IP)
  
  // Reconnect pokud spojení bylo přerušeno
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
    digitalWrite(LED_BUILTIN, LOW);                 // Zapnutí In-buils LED
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);                // Vypnutí In-buils LED
    Serial.println("Disconnected");
    WiFi.reconnect();                               // Pokusí se znovu připojit k síti
    milis = millis();                               // Uložení okamžiku, kdy se začal připojovat
    while (WiFi.status() != WL_CONNECTED) {         // Připojení chvíli trvá - Kontrola jestli už se podařilo připojit
      if (abs(millis() - milis > 10000)) break;     // Aby se nepřipojoval donekonečna přerušit po 10 sekundách připojování

      delay(500);
      Serial.print(".");
    }
  }

  switch (cisloEfektu) {                   // řízení zvoleného efektu
    case 1:                                // 1 - Barva
      homePage();
      break;
    case 2:                                // 2 - Oheň
      efektOhen();
      homePage();
      break;
    case 3:                                // 3 - Duha
      efektDuha();
      homePage();
      break;
    case 4:                                // 4 - Vánoce
      efektVanoce();
      homePage();
      break;
    case 5:                                // 5 - Knigth Rider
      efektRider();
      homePage();
      break;
    case 6:                                // 6 - Vločky
      efektVlocky();
      homePage();
      break;
    default:                               // pokud nic výše nenalezeno
      break;
  }
}

// Zinicializuje NeoPixel LED pásek a nastaví ho na maximální jas
void initNeoPixel() {
  pixels.begin();                           // Inicializace objektu "pixels"
  pixels.clear();                           // Vypne všechny NeoPixel LED
  pixels.show();                            // odešle požadavek do LED pásku

  for (int i = 0; i < NUMPIXELS; i++) {                       // Cyklus, který postupně nastaví jednotlivé LED
    pixels.setPixelColor(i, pixels.Color(20, 20, 20));        // Nastavení jednotlivé LED na bílou poloviční jas (RGB)
    pixels.show();                                            // Odeslání nastavení do NeoPixel pásku
  }
}

// nastavení akce pro připojení na hlavní stránku (URI : /).
void homePage() {
  server.send(200, "text/html", HTMLStranka ());
}

//nastaví akce pro případ, kdy stránka neexistuje
void strankaNeExistuje () {
  String  zprava = "<!DOCTYPE html><html><head><title>Stránka nenalezena</title></head><body>požadovaná stránka nemá na Arduino nastavenu funkci</body></html>";
}

// funkce pro nastavení barvy
void nastavBarvu () {
  cisloEfektu = 1;

  String masterHexString = server.arg("barva");

  String hexRed = masterHexString.substring(1, 3);
  String hexGreen = masterHexString.substring(3, 5);
  String hexBlue = masterHexString.substring(5, 7);

  for (int i = 0; i < NUMPIXELS; i++) {                                                                 // Cyklus, který postupně nastaví jednotlivé LED
    pixels.setPixelColor(i, pixels.Color(hexToInt(hexRed), hexToInt(hexGreen), hexToInt(hexBlue)));     // Nastavení jednotlivé LED
    pixels.show();                                                                                      // Odeslání nastavení do NeoPixel pásku
  }
  delay(10);
}


// fukce pro převod HEX -> INT
int hexToInt (String hexString) {
  int nextInt = 0;
  int intValue = 0;
  for (int i = 0; i < 2; i++) {
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    intValue = (intValue * 16) + nextInt;
  }
  return intValue;
}

// nastaví LED pásek na simulaci ohně
void efektOhen () {

  static int red, green, blue;
  cisloEfektu = 2;

  if (server.arg("barva") != "") {
    masterHexString = server.arg("barva");
  }
  String hexRed = masterHexString.substring(1, 3);
  String hexGreen = masterHexString.substring(3, 5);
  String hexBlue = masterHexString.substring(5, 7);

  red = hexToInt(hexRed);
  green = hexToInt(hexGreen);
  blue = hexToInt(hexBlue);

  for (int i = 0; i < NUMPIXELS; i++) {
    int flicker = random(0, 100);
    int r1 = red - flicker;
    int g1 = green - flicker;
    int b1 = blue - flicker;
    if (g1 < 0) g1 = 0;
    if (r1 < 0) r1 = 0;
    if (b1 < 0) b1 = 0;
    pixels.setPixelColor(i, r1, g1, b1);
  }
  pixels.show();
  delay(random(10, 113));                           // počkat před dalším překreslením
}

// Nastaví LED pásek na simulaci duhy
void efektDuha() {
  cisloEfektu = 3;

  for (long firstPixelHue = 0; firstPixelHue < 65536; firstPixelHue += 256) {
    for (int i = 0; i < pixels.numPixels(); i++) {
      int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue, 255, 255)));
    }
    pixels.show();
    delay(10);
  }
}

// Nastaví LED pásek na vánoční blikání
void efektVanoce() {
  cisloEfektu = 4;

  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(random(0, 10) * 6553, 255, 255)));
  }
  pixels.show();
  delay(1000);
}

// Knigth Rider efekt
void efektRider() {
  cisloEfektu = 5;

  byte width = 20;
  byte hodnota = 255 / width;;
  int hue = 0;
  byte saturation = 255;

  for (int i = 0; i < NUMPIXELS; i++) {
    for (int j = 0; j < width; j++) {
      if (i - j >= 0) {
        pixels.setPixelColor(i - j, pixels.gamma32(pixels.ColorHSV(hue, saturation, hodnota * (width - j))));
      }
    }
    pixels.show();
    delay(20);
    pixels.clear();
  }

  for (int i = NUMPIXELS - 1; i > 0; i--) {
    for (int j = 0; j < width; j++) {
      if (j + i < NUMPIXELS) {
        pixels.setPixelColor(i + j, pixels.gamma32(pixels.ColorHSV(hue, saturation, hodnota * (width - j))));
      }
    }

    pixels.show();
    delay(20);
    pixels.clear();
  }
}

// efekt Vlocky
void efektVlocky() {
  cisloEfektu = 6;

  byte cisloPixelu = 0;
  byte saturation = 255;
  int hue = 43000;

  pixels.clear();
  hue = random(0, 65535);
  cisloPixelu = random(0, NUMPIXELS);
  for (int i = 0; i < 255; i++) {
    pixels.setPixelColor(cisloPixelu, pixels.gamma32(pixels.ColorHSV(hue, saturation, i)));
    pixels.show();
    delay(10);
  }

  for (int i = 255; i > 0; i--) {
    pixels.setPixelColor(cisloPixelu, pixels.gamma32(pixels.ColorHSV(hue, saturation, i)));
    pixels.show();
    delay(10);
  }
}

// Sestavení HTML stránky
String HTMLStranka () {
  String Text = "";

  Text += "<!DOCTYPE html><html><head><meta charset=\"iso-8859-2\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  Text += "<style>body {font-family: Arial;} .tab {overflow: hidden;border: 1px solid #ccc;background-color: #f1f1f1;} .tab button {background-color: inherit;float: left;border: none;outline: none;cursor: pointer;padding: 10px 8px;transition: 0.3s;font-size: 14px;} .tab button:hover {background-color: #ddd;} .tab button.active {background-color: #00A000;color: white} .tabcontent {padding: 6px 12px;border: 1px solid #ccc;border-top: none;background-color: white;} Fieldset {border: 1px solid #00A000;color: #00A000;position: relative;} input {box-sizing: border-box;margin: 5px;float: left;height: 50px;width: 100px;}</style>";
  Text += "</head><body><div class=\"tab\"><button class=\"tablinks\" onClick=\"window.location.href = 'http://192.168.15.30';\">LED TV</button><button class=\"active\" >LED Postel</button><button class=\"tablinks\" onClick=\"window.location.href = 'http://192.168.15.32';\">LED Maska</button><button class=\"tablinks\" onClick=\"window.location.href = 'http://192.168.15.33';\">Termostat</button><button class=\"tablinks\" onClick=\"window.location.href = 'http://192.168.15.34';\">Kamera</button></div>";
  Text += "<div class=\"tabcontent\"><fieldset><legend>Svetlo</legend><form action=\"/barva\" method=\"get\"><input type=\"hidden\" name=\"barva\" value=\"#ffffff\"><input type=\"submit\" value=\"Maximum\"></form><form action=\"/barva\" method=\"get\"><input type=\"hidden\" name=\"barva\" value=\"#7D7D7D\"><input type=\"submit\" value=\"50%\"></form><form action=\"/barva\" method=\"get\"><input type=\"hidden\" name=\"barva\" value=\"#0A0A0A\"><input type=\"submit\" value=\"Minimum\"></form><form action=\"/barva\" method=\"get\"><input type=\"hidden\" name=\"barva\" value=\"#000000\"><input type=\"submit\" value=\"Vypnout\"></form></fieldset><br>";
  Text += "<Fieldset><legend>Barva</legend><form action=\"/barva\" method=\"get\"><input type=\"color\" name=\"barva\" value=\"#0000FF\" ><input type=\"submit\" value=\"Nastavit\" ></form></Fieldset><br>";
  Text += "<Fieldset><legend>Ohen</legend><form action=\"/ohen\" method=\"get\"><input type=\"color\" name=\"barva\" value=\"#ff7d00\"><input type=\"submit\" value=\"Nastavit\"></form></Fieldset><br>";
  Text += "<Fieldset><legend>Efekty</legend><form action=\"/duha\" method=\"post\"><input type=\"submit\" value=\"Duha\" ></form><form action=\"/vanoce\" method=\"post\"><input type=\"submit\" value=\"Vanoce\" ></form><form action=\"/rider\" method=\"post\"><input type=\"submit\" value=\"Knight Rider\" ></form><form action=\"/vlocky\" method=\"post\"><input type=\"submit\" value=\"Vlocky\"></form></Fieldset><br></div></body></html>";

  return Text;
}
