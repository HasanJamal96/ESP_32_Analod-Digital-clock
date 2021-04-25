#include <WiFi.h>
#include <time.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(128, 64, &Wire, -1);

#define change_pin 4
#define wifi_reset 5

bool digital = true;
bool analog = false;

WiFiManager wifiManager;
bool isconnected = false;

int timezone = 5 * 3600;
int dst = 0;
unsigned long current_time = 0;
unsigned long previous_time = 0;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup_wifi(){
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect("Clock", "12345678");
}

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.display();
  
  pinMode(change_pin, INPUT_PULLUP);
  pinMode(wifi_reset, INPUT_PULLUP);

  setup_wifi();
}

void loop() {
  while(WiFi.status() != WL_CONNECTED){
    isconnected = false;
    wifiManager.process();
  }
  if(!isconnected){
    isconnected = true;
    digital = true;
    configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
    while(!time(nullptr)){
    delay(1000);
    }
  }
  if(digitalRead(change_pin) == LOW){
    if(digital){
      digital = false;
      analog = true;
    }
    else{
      digital = true;
      analog = false;
    }
    delay(1000);
    display.clearDisplay();
  }

  if(digitalRead(wifi_reset) == LOW){
    wifiManager.resetSettings();
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor((display.width()/2)-20,0);
    display.print("WIFI");
    display.setCursor((display.width()/2)-20,20);
    display.print("Reset");
    setup_wifi();
    delay(1000);
    digital = false;
    analog = false;
  }
  
  if(isconnected){
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    if(digital){
      current_time = millis();
      if(current_time-previous_time > 990){
        // Clear the buffer.
        display.clearDisplay();
        display.setTextSize(3);
        display.setTextColor(WHITE);
         
        display.setCursor(0,0);
        display.print(p_tm->tm_hour);
        display.print(":");
        if( p_tm->tm_min <10)
        display.print("0");
        display.print(p_tm->tm_min);
         
        display.setTextSize(2);
        display.setCursor(90,5);
        display.print(".");
        if( p_tm->tm_sec <10)
        display.print("0");
        display.print(p_tm->tm_sec);
         
        display.setTextSize(2);
        display.setCursor(0,40);
        display.print(p_tm->tm_mday);
        display.print("/");
        display.print(p_tm->tm_mon + 1);
        display.print("/");
        display.print(p_tm->tm_year + 1900);
         
        display.display();
        previous_time = current_time;
      }
    }
    else if(analog){
      int r = 33;
      display.drawCircle((display.width()/2)-10, display.height()/2, 2, WHITE);
      
      //hour ticks
      for( int z=0; z < 360;z= z + 30 ){
        float angle = z ;
         
        angle=(angle/57.29577951) ; //Convert degrees to radians
        int x2=(54+(sin(angle)*r));
        int y2=(32-(cos(angle)*r));
        int x3=(54+(sin(angle)*(r-5)));
        int y3=(32-(cos(angle)*(r-5)));
        display.drawLine(x2,y2,x3,y3,WHITE);
      }
      // display second hand
      float angle = p_tm->tm_sec*6 ;
      angle=(angle/57.29577951) ; //Convert degrees to radians
      int x3=(54+(sin(angle)*(r)));
      int y3=(32-(cos(angle)*(r)));
      display.drawLine(54,32,x3,y3,WHITE);
    
      // display minute hand
      angle = p_tm->tm_min * 6 ;
      angle=(angle/57.29577951) ; //Convert degrees to radians
      x3=(54+(sin(angle)*(r-3)));
      y3=(32-(cos(angle)*(r-3)));
      display.drawLine(54,32,x3,y3,WHITE);
    
      // display hour hand
      angle = p_tm->tm_hour * 30 + int((p_tm->tm_min / 12) * 6 );
      angle=(angle/57.29577951) ; //Convert degrees to radians
      x3=(54+(sin(angle)*(r-11)));
      y3=(32-(cos(angle)*(r-11)));
      display.drawLine(54,32,x3,y3,WHITE);
       
      display.setTextSize(1);
      display.setCursor((display.width())-30,3);
      display.print(p_tm->tm_mday);
      display.setCursor((display.width())-30,15);
      display.print(p_tm->tm_mon + 1);
      display.setCursor((display.width())-30,27);
      display.print(p_tm->tm_year + 1900);
       
      // update display with all data
      display.display();
      delay(100);
      display.clearDisplay();
    }
  }
}
