// To use ArduinoGraphics APIs, include BEFORE Arduino_LED_Matrix
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include <WiFiS3.h>

// ---------- WiFi setup ----------
const char* ssid     = "change me";       // <-- replace with your Wi-Fi name
const char* password = "change me";   // <-- replace with your Wi-Fi password

WiFiServer server(80);
ArduinoLEDMatrix matrix;

String displayText = "Ready to Go";
bool scrolling = true;

// ---------- URL decode helper ----------
String urlDecode(const String& input) {
  String decoded = "";
  char temp[] = "00";
  unsigned int len = input.length();

  for (unsigned int i = 0; i < len; i++) {
    if (input[i] == '+') {
      decoded += ' ';
    } else if (input[i] == '%' && i + 2 < len) {
      temp[0] = input[i + 1];
      temp[1] = input[i + 2];
      char decodedChar = strtol(temp, NULL, 16);
      decoded += decodedChar;
      i += 2;
    } else {
      decoded += input[i];
    }
  }
  return decoded;
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  matrix.begin();
}

// ---------- LOOP ----------
void loop() {
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    // --- Handle different actions ---
    if (req.indexOf("/display?text=") != -1) {
      int start = req.indexOf("text=") + 5;
      int end   = req.indexOf(' ', start);
      displayText = req.substring(start, end);
      displayText = urlDecode(displayText);
      scrolling = true;
      Serial.print("New message: ");
      Serial.println(displayText);
    } 
    else if (req.indexOf("/stop") != -1) {
      scrolling = false;
      Serial.println("Scrolling stopped.");
    } 
    else if (req.indexOf("/clear") != -1) {
      scrolling = false;
      displayText = "";
      matrix.beginDraw();
      matrix.clear();
      matrix.endDraw();
      Serial.println("Matrix cleared.");
    }

    // ---------- Web Page ----------
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>UNO R4 WiFi LED Matrix</title>");
    client.println("<style>");
    client.println("body {font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; margin: 0; padding: 0;}");
    client.println(".container {background: #fff; max-width: 400px; width: 90%; margin: 40px auto; padding: 20px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.1);}");
    client.println("h2 {margin-top: 0; color: #333;}");
    client.println("form {margin-top: 10px;}");
    client.println("input[type=text] {width: 90%; padding: 10px; margin-bottom: 10px; border-radius: 6px; border: 1px solid #ccc; font-size: 16px;}");
    client.println("input[type=submit], .button {display: inline-block; padding: 10px 20px; margin: 6px; font-size: 16px; border: none; border-radius: 6px; cursor: pointer; color: #fff; transition: opacity 0.2s ease;}");
    client.println("input[type=submit] {background-color: #0078d7;}");
    client.println(".button.stop {background-color: #f39c12;}");
    client.println(".button.clear {background-color: #e74c3c;}");
    client.println(".button:hover, input[type=submit]:hover {opacity: 0.85;}");
    client.println("p {font-size: 16px; color: #333; word-wrap: break-word;}");
    client.println("</style></head><body>");
    client.println("<div class='container'>");
    client.println("<h2>UNO R4 WiFi LED Matrix</h2>");
    client.println("<form action=\"/display\" method=\"get\">");
    client.println("<input type=\"text\" name=\"text\" placeholder=\"Enter message here...\">");
    client.println("<input type=\"submit\" value=\"Show\">");
    client.println("</form>");
    client.println("<p><b>Currently showing:</b><br>" + displayText + "</p>");
    client.println("<div>");
    client.println("<a href=\"/stop\"><button class='button stop'>⏸ Stop Scroll</button></a>");
    client.println("<a href=\"/clear\"><button class='button clear'>🧹 Clear Display</button></a>");
    client.println("</div>");
    client.println("</div></body></html>");
    client.stop();
  }

  // ---------- Scroll the current text ----------
  if (scrolling && displayText.length() > 0) {
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);          // white text
    matrix.textScrollSpeed(70);         // lower = faster scroll
    matrix.textFont(Font_5x7);
    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(displayText);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
    delay(100);
  }
}
