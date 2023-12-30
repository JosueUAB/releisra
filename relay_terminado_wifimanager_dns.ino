/**
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for ondemand configportal, hold for 3 seconds for reset settings.
 */
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <ESP8266mDNS.h>
WiFiServer server(80);

#define TRIGGER_PIN 0
#define led_1 16
#define led_2 2


#define RELAY1 5
#define RELAY2 4
#define RELAY3 14
#define RELAY4 12
#define RELAY5 13
#define CONNECT_TIME 10000
#define TIMEOUTTIME 5000

Ticker ticker;

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false;  // change to true to use non blocking

WiFiManager wm;                     // global wm instance
WiFiManagerParameter custom_field;  // global param ( for non blocking w params )
WiFiManagerParameter static_ip_field("staticip", "Static IP", "192.168.0.200", 15);


String header;
String Relay1State = "off";
String Relay2State = "off";
String Relay3State = "off";
String Relay4State = "off";
String Relay5State = "off";
size_t currentTime = millis();
size_t previousTime = 0;


void setup() {
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  delay(3000);
  Serial.println("\n Starting");

  pinMode(TRIGGER_PIN, INPUT);
  pinMode(led_1, OUTPUT);
  pinMode(led_2, OUTPUT);
  digitalWrite(led_1, HIGH);
  digitalWrite(led_2, HIGH);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);
  pinMode(RELAY4, OUTPUT);
  pinMode(RELAY5, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
  digitalWrite(RELAY3, LOW);
  digitalWrite(RELAY4, LOW);
  digitalWrite(RELAY5, LOW);
  


  ticker.attach(0.2, parpadeoLedWIFI);

  // wm.resetSettings(); // wipe settings

  if (wm_nonblocking) wm.setConfigPortalBlocking(false);

  // add a custom input field
  int customFieldLength = 40;


  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");

  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type

  // test custom html(radio)
  const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str);  // custom html input

  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  //
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
  // wm.setMenu(menu,6);
  std::vector<const char*> menu = { "wifi", "info", "param", "sep", "restart", "exit" };
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");


  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always

  // wm.setConnectTimeout(20); // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(5000);  // auto close configportal after n seconds
  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons

  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

  bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  res = wm.autoConnect("ISRA_DEV", "");  // password protected ap
  wm.addParameter(&static_ip_field);




  if (!res) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } else {
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    ticker.detach();
    digitalWrite(led_1, LOW);
     if (!MDNS.begin("esp8266")) 
   {             
     Serial.println("Error iniciando mDNS");
   }
   Serial.println("mDNS iniciado");
  }
  wm.addParameter(&static_ip_field);

  Serial.println(res);
  /*
   while (wm.setConfigPortalBlocking(false)) {
        delay(500);
        Serial.print(".");
        digitalWrite(led_1, LOW);
        delay(1000);
        digitalWrite(led_1, HIGH);
        delay(1000);
        digitalWrite(led_1, LOW);
        delay(1000);
        digitalWrite(led_1, HIGH);
        delay(1000);
    } */
  checkWiFiConnection();
  server.begin();

}

void checkButton() {
  // check for button press
  if (digitalRead(TRIGGER_PIN) == LOW) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW) {
      Serial.println("Button de reset presionado");
      // borra las configuraciones si se preciona por 5 segundos
      delay(5000);
      for (int contador = 0; contador < 10; contador++) {
        int espera = 100;
        digitalWrite(led_1, HIGH);
        digitalWrite(led_2, LOW);
        delay(espera);
        digitalWrite(led_1, LOW);
        digitalWrite(led_2, HIGH);
        delay(espera);
      }
      for (int contador = 0; contador < 5; contador++) {
        int espera = 100;
        digitalWrite(led_1, HIGH);
        digitalWrite(led_2, HIGH);
        delay(espera);
        digitalWrite(led_1, LOW);
        digitalWrite(led_2, LOW);
        delay(espera);
      }

      if (digitalRead(TRIGGER_PIN) == LOW) {
        Serial.println("Button presiado exitosamente");
        Serial.println("borrando configuraciones, restablecido exitosamente");
        wm.resetSettings();
        ESP.restart();
      }

      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);




      if (!wm.startConfigPortal("Domotic_Isra", "123flores")) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        // ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
      }
    }
  }
}


String getParam(String name) {
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(led_1, LOW);  // Enciende el led_1 si no hay conexión a un AP
  } else {
    digitalWrite(led_1, HIGH);  // Apaga el led_1 si hay conexión a un AP
  }
}

void parpadeoLedWIFI(){
  //cambiando el estado del led
  byte estado= digitalRead(led_1);
  digitalWrite(led_1,!estado);

}

void saveParamCallback() {
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
  // Obtén y aplica la configuración de IP estática
  String static_ip = static_ip_field.getValue();
  Serial.println("Static IP: " + static_ip);
  // Aquí debes implementar la lógica para aplicar la configuración de IP estática a tu dispositivo
}




void loop() {
  if (wm_nonblocking) wm.process();  // avoid delays() in loop when non-blocking and other long running code
  checkButton();
  checkWiFiConnection();
  // put your main code here, to run repeatedly:


  WiFiClient client = server.available();
  if (client) {
      currentTime = millis();
      previousTime = currentTime;
      Serial.println("New Client.");
      String currentLine = "";
      while (client.connected() && currentTime - previousTime <= TIMEOUTTIME) {
        currentTime = millis();
        if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          if (header.indexOf("GET /RELAY1/on") >= 0) {
            Serial.println("RELAY 1 on");
            Relay1State = "on";
            digitalWrite(RELAY1, HIGH);
            } else if (header.indexOf("GET /RELAY1/off") >= 0) {
            Serial.println("RELAY1 off");
            Relay1State = "off";
            digitalWrite(RELAY1, LOW);
            } else if (header.indexOf("GET /RELAY2/on") >= 0) {
            Serial.println("RELAY2 on");
            Relay2State = "on";
            digitalWrite(RELAY2, HIGH);
            } else if (header.indexOf("GET /RELAY2/off") >= 0) {
            Serial.println("RELAY2 off");
            Relay2State = "off";
            digitalWrite(RELAY2, LOW);
            }

            else if (header.indexOf("GET /RELAY3/on") >= 0) {
            Serial.println("RELAY3 on");
            Relay3State = "on";
            digitalWrite(RELAY3, HIGH);
            } else if (header.indexOf("GET /RELAY3/off") >= 0) {
            Serial.println("RELAY3 off");
            Relay3State = "off";
            digitalWrite(RELAY3, LOW);
            }
            else if (header.indexOf("GET /RELAY4/on") >= 0) {
            Serial.println("RELAY4 on");
            Relay4State = "on";
            digitalWrite(RELAY4, HIGH);
            } else if (header.indexOf("GET /RELAY4/off") >= 0) {
            Serial.println("RELAY4 off");
            Relay4State = "off";
            digitalWrite(RELAY4, LOW);
            }
            else if (header.indexOf("GET /RELAY5/on") >= 0) {
            Serial.println("RELAY5 on");
            Relay5State = "on";
            digitalWrite(RELAY5, HIGH);
            } else if (header.indexOf("GET /RELAY5/off") >= 0) {
            Serial.println("RELAY5 off");
            Relay5State = "off";
            digitalWrite(RELAY5, LOW);
            }
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<meta charset=\"UTF-8\">");
          client.println("<meta http-equiv=\"refresh\" content=\"5\" >");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\" integrity=\"sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T\" crossorigin=\"anonymous\">");
          client.println("<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.6.3/css/all.css\" integrity=\"sha384-UHRtZLI+pbxtHCWp1t77Bi1L4ZtiqrqD80Kn4Z8NTSRyMA2Fd33n5dQ8lWUE00s/\" crossorigin=\"anonymous\">");
          client.println("<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>");
          client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js\" integrity=\"sha384-UO2eT0CpHqdSJQ6hJty5KVphtPhzWj9WO1clHTMGa3JDZwrnQq4sF86dIHNDz0W1\" crossorigin=\"anonymous\"></script>");
          client.println("<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js\" integrity=\"sha384-JjSmVgyd0p3pXB1rRibZUAYoIIy6OrQ6VrjIEaFf/nJGzIxFDsf4x0xIM+B07jRM\" crossorigin=\"anonymous\"></script>");
          client.println("<style>");
          client.println("html {font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
          client.println(".mybtn {padding: 16px 40px; font-size: 30px;} ");
          client.println(".par { font-size: 16px;}");
          client.println("p {text-align: center;}");
          client.println(".lm35 {text-align: center; border: none; margin: 2px; padding: 16px 40px; font-size: 30px;}");
          client.println("</style></head>");
          client.println("<body class=\"bg-light\">");
          client.println("<h1 style=\"text-align:center; \" class=\"display-4\">ESP32 Web Server </h1> ");
          client.println("<br />");
          client.println("<p class=\"par\">GPIO 26 - State " + Relay1State + " </p> ");
          if (Relay1State == "off") {
          client.println("<p><a href =\"/RELAY1/on\" role=\"button\" class=\"btn btn-success mybtn\" >ON</a></p>");
          } else {
          client.println("<p><a href=\"/RELAY1/off\" role=\"button\" class=\"btn btn-danger mybtn\">OFF</a></p>");
          }
          client.println("<br />");
          client.println("<p class=\"par\">RELAY2 - State " + Relay2State + "</p>");
          if (Relay2State == "off") {
          client.println("<p><a href=\"/RELAY2/on\" role=\"button\" class=\" mybtn btn btn-success mybtn\" >ON</a></p>");
          } else {
          client.println("<p><a href=\"/RELAY2/off\" role=\"button\" class=\" mybtn btn btn-danger mybtn\">OFF</a></p>");
          }
          client.println("<br />");
          client.println("<p class=\"par\">GPIO 26 - State " + Relay3State + " </p> ");
          if (Relay3State == "off") {
          client.println("<p><a href =\"/RELAY3/on\" role=\"button\" class=\"btn btn-success mybtn\" >ON</a></p>");
          } else {
          client.println("<p><a href=\"/RELAY3/off\" role=\"button\" class=\"btn btn-danger mybtn\">OFF</a></p>");
          }
          client.println("<br />");
          client.println("<p class=\"par\">GPIO 26 - State " + Relay4State + " </p> ");
          if (Relay4State == "off") {
          client.println("<p><a href =\"/RELAY4/on\" role=\"button\" class=\"btn btn-success mybtn\" >ON</a></p>");
          } else {
          client.println("<p><a href=\"/RELAY4/off\" role=\"button\" class=\"btn btn-danger mybtn\">OFF</a></p>");
          }
          client.println("<br />");
          client.println("<p class=\"par\">GPIO 26 - State " + Relay4State + " </p> ");
          if (Relay5State == "off") {
          client.println("<p><a href =\"/RELAY5/on\" role=\"button\" class=\"btn btn-success mybtn\" >ON</a></p>");
          } else {
          client.println("<p><a href=\"/RELAY5/off\" role=\"button\" class=\"btn btn-danger mybtn\">OFF</a></p>");
          }
          client.println("<br />");
          client.println("<footer class=\"bg-dark text-center text-white\">");
          client.println("<div class=\"container pt-4 \">");
          client.println("<section class=\"mb-4\">");
          client.println("<a class=\"btn btn-outline-light btn-floating m-1\" href=\"https://www.facebook.com/MiniProjectsOfficial\" target=\"_blank\" rel=\"noopener noreferrer\" role=\"button\">");
          client.println("<i class=\"fab fa-facebook-f\"></i>");
          client.println("</a>");
          client.println("<a class=\"btn btn-outline-light btn-floating m-1\" href=\"https://www.instagram.com/officialprojecto\" target=\"_blank\" rel=\"noopener noreferrer\" role=\"button\">");
          client.println("<i class=\"fab fa-instagram\"></i>");
          client.println("</a>");
          client.println("<a class=\"btn btn-outline-light btn-floating m-1\" href=\"https://youtube.com/c/ProjectoOfficial\" target=\"_blank\" rel=\"noopener noreferrer\" role=\"button\">");
          client.println("<i class=\"fab fa-youtube\"></i>");
          client.println("</a>");
          client.println("<a class=\"btn btn-outline-light btn-floating m-1\" href=\"https://github.com/ProjectoOfficial\" target=\"_blank\" rel=\"noopener noreferrer\" role=\"button\">");
          client.println("<i class=\"fab fa-github\"></i>");
          client.println("</a>");
          client.println("</section>");
          client.println("</div>");
          client.println("<div class=\"text-center p-3\" style=\"background-color: rgba(0, 0, 0, 0.2);\"> © 2021 Copyright: <a class=\"text-white\" href=\"https://officialprojecto.wordpress.com\">Projecto</a></div>");
          client.println("</footer>");
          client.println("</body></html>");
          client.println();
          break;
          } else {
          currentLine = "";
          }
          } else if (c != '\r') {
          currentLine += c;
          }
          }
          }
          header = "";
client.stop();
Serial.println("Client disconnected.");
Serial.println("");
}
delay(1);
}








