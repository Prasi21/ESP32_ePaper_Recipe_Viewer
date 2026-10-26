#include "WiFi.h"
#include "LittleFS.h"
#include "ESPAsyncWebServer.h"
#include "credentials.h"
#include "display.h"
 
AsyncWebServer server(80);
AsyncWebSocket ws("/test");

bool displayInisiated = false;
void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client,
                AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if(type == WS_EVT_CONNECT){
    Serial.println("Websocket client connection received");
  }
  else if(type == WS_EVT_DISCONNECT){
    Serial.println("Websocket client disconnected");
  }
  // else if(type == WS_EVT_DATA){
  //   AwsFrameInfo * info = (AwsFrameInfo*)arg;
  //   if(info->opcode  == WS_TEXT){  // Got text data (the client checks if the display is busy)
  //       client->text(responseToNewImage());
  //   }
  //   else{                         // Got binary data (the client sent the image)
  //     // Send the received data to the display buffer.
  //     loadImage((const char*)data, len, info->index);
  //     if((info->index + len) == info->len){   // if this data was the end of the file
  //       //client->text("IMAGE_LOADED");       // tell the client that the server got the image
  //       Serial.printf("Updating Display: %d bytes total\n", info->len);
  //       updateDisplay_withoutIdle();          // Display the image that received
  //     }
  //   }
    

  // }
  //Temp debug code
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;

    if (info->opcode == WS_TEXT) {
        client->text(responseToNewImage());
    } else {
        Serial.printf(
            "Chunk: index=%u len=%u total=%u final=%d\n",
            (unsigned int)info->index,
            (unsigned int)len,
            (unsigned int)info->len,
            info->final
        );

        // Print the first few received bytes of the message.
        if (info->index == 0) {
            Serial.print("First received bytes: ");
            for (size_t i = 0; i < 10 && i < len; i++) {
                Serial.printf("%02X ", data[i]);
            }
            Serial.println();
        }

        loadImage((const char*)data, len, info->index);
        if (info->index == 0) {
            Serial.print("Buffer after first chunk: ");
            for (int i = 0; i < 10; i++) {
                Serial.printf("%02X ", imageBuffer[i]);
            }
            Serial.println();
        }
        
        if ((info->index + len) == info->len) {
            Serial.printf("Received %u bytes total\n",
                          (unsigned int)info->len);

            Serial.print("Received buffer, first 10 bytes: ");
            for (int i = 0; i < 10; i++) {
                Serial.printf("%02X ", imageBuffer[i]);
            }
            Serial.println();

            Serial.print("Received buffer, row 0 bytes 0-9: ");
            for (int i = 0; i < 10; i++) {
                Serial.printf("%02X ", imageBuffer[i]);
            }
            Serial.println();

            printFramebufferSamples();
            updateDisplay_withoutIdle();
        }
    }
}
}

char* responseToNewImage(){
  if(isDisplayBusy()){
    return "BUSY";
  }
  else{
    displayStartTransmission();
    return "OK";
  }
}
void setup(){
  Serial.begin(115200);

  Serial.println("Test 1 Pass");
  // Inisialize LittleFS
  if(!LittleFS.begin()){             
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
  }

  Serial.println("Test 2 Pass");
  // Inisialize display
if (!displayInisiated) {
    if (!initDisplay()) {
        Serial.println("Stopping: display init failed");
        return;
    }

    displayInisiated = true;
}

  Serial.println("Test 3 Pass");
  // Inisialize WiFi
  WiFi.begin(wifi_credentials.ssid, wifi_credentials.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  Serial.println("Test 4 Pass");

  delay(2000);

  // setup handles for websockets connetions
  ws.onEvent(onWsEvent);  
  server.addHandler(&ws);

  // handle GET requests to route /index.html
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request recived: GET /index.html");
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  // handle GET requests to route /
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request recived: GET /");
    request->send(LittleFS, "/index.html", "text/html");
  });
  
  // handle GET requests to route /hello - echo "Hello Word" (for debugging)
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request recived: GET /hello");
    request->send(200, "text/plain", "Hello World");
  });
  
  server.begin(); //start listening to incoming HTTP requests
}
 
void loop(){
  delay(1000);
}
