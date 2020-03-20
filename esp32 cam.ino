#include <WiFi.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "Arduino.h"
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include "Base64.h"
#include <SPIFFS.h>
#include <FS.h>
#include <WiFiClientSecure.h>

// Photo File Name to save in SPIFFS
#define FILE_PHOTO "/photo.txt"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// Replace with your network credentials
const char* ssid = "OPPO A83";
const char* password = "12345678";

String myScript = "/macros/s/AKfycbxfhBuXyQGJhshpaWUzWDGUE5ZeYwk8ypJocShp6EwedFba4Cd0/exec";    //Create your Google Apps Script and replace the "myScript" path.
String myLineNotifyToken = "myToken=";    //Line Notify Token. You can set the value of xxxxxxxxxx empty if you don't want to send picture to Linenotify.
String myFoldername = "&myFoldername=ESP32-CAM";
String myFilename = "&myFilename=ESP32-CAM.jpg";
String myImage = "&myFile=";
boolean takeNewPhoto = false;
int count =0;
unsigned int pic_sz =0;
unsigned int pre_pic_sz;
int waitT = 10000;// timeout 10 seconds
long startT;
    
    boolean stad = false;
static void IRAM_ATTR detectsMovement(void * arg) { 
  Serial.println("ISR");  
  takeNewPhoto=true;
  Serial.println("ISR_finish");  
}
void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("SPIFFS mounted successfully");
  }

  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());

  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
    ESP.restart();
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {
      ledcWrite(3,10);
      delay(200);
      ledcWrite(3,0);
      delay(200);    
    }
    ledcDetachPin(3);      
  }
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA*/
  err = gpio_isr_handler_add(GPIO_NUM_13, &detectsMovement, (void *) 13);
  if (err != ESP_OK) {
Serial.printf("handler add failed with error 0x%x \r\n", err);
}
err = gpio_set_intr_type(GPIO_NUM_13, GPIO_INTR_POSEDGE);
if (err != ESP_OK) {
Serial.printf("set intr type failed with error 0x%x \r\n", err);
}
File file = SPIFFS.open(FILE_PHOTO, FILE_WRITE);
 if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.print("");
  }
  
  startT = millis();}
  void loop() {ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
        
   
  if (takeNewPhoto) {
    ledcWrite(3,10);
    capturePhotoSaveSpiffs();
    takeNewPhoto = false;
     ledcWrite(3,0);
    startT = millis();
  }
  if((startT+waitT<millis())&&count>0){
    ledcWrite(3,10);
    Upload();
    ledcWrite(3,0);
  }
   ledcDetachPin(3);
  delay(1);
}

bool checkPhoto( fs::FS &fs ) {
  File f_pic = fs.open( FILE_PHOTO );
  
  pre_pic_sz = pic_sz;
  pic_sz= f_pic.size();
  
  return ( pic_sz > pre_pic_sz +5000 );
}

// Capture Photo and Save it to SPIFFS
void capturePhotoSaveSpiffs( void ) {
  camera_fb_t * fb = NULL; // pointer
  bool ok = 0; // Boolean indicating if the picture has been taken correctly

  do {
    // Take a photo with the camera
    Serial.println("Taking a photo...");  

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
      }
    // Photo file name
    Serial.printf("Picture file name: %s\n", FILE_PHOTO);
    File file = SPIFFS.open(FILE_PHOTO, FILE_APPEND);

    // Insert the data in the photo file
    if (!file) {
      Serial.println("Failed to open file in writing mode");
    }
    else {
      file.print(imageFile+'\n'); // payload (image), payload length
      //Serial.println(imageFile);
      Serial.print("The picture has been saved in ");
      Serial.print(FILE_PHOTO);
      Serial.print(" - Size: ");
      Serial.print(file.size());
      Serial.println(" bytes");
    }
    // Close the file
    file.close();
    esp_camera_fb_return(fb);

    // check if file has been correctly saved in SPIFFS
    ok = checkPhoto(SPIFFS);
  } while ( !ok );
  count+=1;
  Serial.println(count);
}
//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
void Upload() {
File file = SPIFFS.open(FILE_PHOTO, FILE_READ);
if (!file) {
      Serial.println("Failed to open file in reading mode");
    }
    else {
      String imageFile;
      char append; 
      String getAll="", getBody = "";
      WiFiClientSecure client_tcp;
      const char* myDomain = "script.google.com";
      for(int i=0;i<count;i++){
        imageFile= file.readStringUntil('\n');
        //Serial.println(imageFile);
        Serial.println("Connect to " + String(myDomain));
        if (client_tcp.connect(myDomain, 443)) {
          Serial.println("Connection successful");
           String Data = myLineNotifyToken+myFoldername+myFilename+myImage;
    
    client_tcp.println("POST " + myScript + " HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(Data.length()+imageFile.length()));
    Serial.println(String(Data.length()+imageFile.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println();
    
    client_tcp.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1000) {
      client_tcp.print(imageFile.substring(Index, Index+1000));
    }

    
    int waitTime = 15000;   // timeout 15 seconds
    long startTime = millis();
    boolean state = false;
    
    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          if (state==true) getBody += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
        }
        else{
          getBody="Connected to " + String(myDomain) + " failed.";
          Serial.println("Connected to " + String(myDomain) + " failed.");
      }
    }
      file.close();
      SPIFFS.remove(FILE_PHOTO);
      count=0;
      pre_pic_sz =0 ;
      pic_sz=0;
      Serial.println(count);
      startT = millis();
      
}
}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
