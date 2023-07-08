/****************************************************************************** 
Heart_Rate_Display over Bluetooth 
Publisher: https://www.circuitschools.com 
******************************************************************************/
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "BluetoothSerial.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
#define LED_BUILTIN 2 //pin with LED to turn on when BT connected
#define i2c_Address 0x3c //Address dari display
#define SCREEN_WIDTH 128 // Lebar display
#define SCREEN_HEIGHT 64 // Tinggi display
#define display_RESET -1   // Reset Poin
#define SENSOR 36 // Tetapkan pin36 ESP32 sebagai sensor
#define UpperThreshold 2000
#define LowerThreshold 1970
#define buzzer 4// Tetapkan buzzer di pin4
#define ONE_WIRE_BUS 15

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, display_RESET);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0.0;
int  idle = 0;

int D=0;
int B=0;
int S=0;
int t1=1;
int a=0;
int lasta=0;
int lastb=0;
int LastTime=0;
int ThisTime;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;
int lastBPM = 0;
int value=0;

BluetoothSerial ESP_BT; // Object for Bluetooth
AsyncWebServer server(80);

void suhu(void) {
  // this part keeps the var temperature up to date as possible.
  if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
  {
    temperature = sensors.getTempCByIndex(0);
    sensors.requestTemperatures(); 
    lastTempRequest = millis(); 
  }
}

void oled(void) {
/****************************************
 * Mengirim Data ke Oled 1.3"
 ****************************************/
if(a>127)
  {
  a=0;
  t1=1;
  lasta=a;
  }
/****************************************
 * Program untuk BPM
 ****************************************/ 
  ThisTime=millis();
  int value=analogRead(SENSOR);
  display.setTextColor(SH110X_WHITE);
  int b=654-(value/3.125); // Pulse Heart Sensor
     //int b=(982.5-(value/2));
  //int b=130.2-(value/20.8333); // AD8232
  Serial.println(value);
  display.writeLine(lasta,lastb,a,b,SH110X_WHITE);
  display.fillRect(t1,0,4,64,SH110X_BLACK);
  display.fillRect(0,0,1,64,SH110X_BLACK);
  lastb=b;
  lasta=a;

  if(value>UpperThreshold)
  {
    if(BeatComplete)
    {
    BPM=ThisTime-LastTime;
    BPM=int(60/(float(BPM)/1000));
    BPMTiming=false;
    BeatComplete=false;
    }
  if(BPMTiming==false)
  {
  LastTime=millis();
  BPMTiming=true;
/****************************************
 * Akhir Program
 ****************************************/
    }
  }
  if((value<LowerThreshold)&(BPMTiming))
  BeatComplete=true;
  display.writeFillRect(0,50,128,16,SH110X_BLACK);
  display.setCursor(0,50);
  if (BPM > 130) {
  BPM = 0; }
  if (BPM < 60) {
  BPM = lastBPM; }
  display.print(BPM);
  lastBPM = BPM;
  display.setCursor(65,50);
  display.print(temperature);
  display.display();
  a++;
  t1++;
  }

void Home(void) {
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(46,0);
  display.print("ECG");
  display.setCursor(15,20);
  display.print("WIRELESS");
  display.display();
   WiFiManager wifiManager;
  wifiManager.autoConnect("PulseHeart_1","");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  display.setTextSize(1);
  display.setCursor(11,40);
  display.println("IP :" + WiFi.localIP().toString());
  display.setTextSize(2);
  display.display();
  for (int16_t t=0; t<127; t += 1) {
  display.writeFillRect(t,50,1,12,SH110X_WHITE);
  display.display();
  delay (30);
  }
}

String Data() {
  float D = analogRead(SENSOR);
  if (isnan(D)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(D);
    return String(D);
    delay(33);
  }
}
String Bpm() {
  float B = BPM;
  if (isnan(B)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(B);
    return String(B);
  }
}

String Suhu1() {
  float S = sensors.getTempCByIndex(0);
  if (isnan(S)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(S);
    return String(S);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <script src="https://code.highcharts.com/highcharts.js"></script>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    body {
      min-width: 310px;
      max-width: 800px;
      height: 400px;
      margin: 0 auto;
    }
    h2 {
      font-family: Arial;
      font-size: 2.5rem;
      text-align: center;
    }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Monitoring Wireless ECG</h2>
  <div id="chart-temperature" class="container"></div>
  <p>
    <i class="fas fa-heartbeat" style="color:#c60c0c;"></i>
    <span class="dht-labels">Detak Jantung</span> 
    <span id="Bpm">%Detak Jantung%</span>
    <sup class="units">BPM</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Suhu</span> 
    <span id="Suhu">%Suhu%</span>
    <sup class="units">&deg;C</sup>
  </p>
</body>
<script>
var chartT = new Highcharts.Chart({
  chart:{ zoomType: 'x', renderTo : 'chart-temperature' },
  title: { text: 'Sinyal Aktivitas Jantung' },
  series: [{
    showInLegend: false,
    data: []
  }],
  plotOptions: {
    line: { animation: false,
      dataLabels: { enabled: false }
    },
    series: { color: '#059e8a' }
  },
 xAxis: { type: 'datetime',
    dateTimeLabelFormats: { second: '%H:%M:%S' }
  },
  yAxis: {
    title: { text: 'Value' }
  },
  credits: { enabled: false }
});

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartT.series[0].data.length > 1240) {
        chartT.series[0].addPoint([x, y], false, true, false);
      } else {
        chartT.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/Data", true);
  xhttp.send();
}, 1 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Bpm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Bpm", true);
  xhttp.send();
}, 3000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Suhu").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Suhu", true);
  xhttp.send();
}, 3000 ) ;

</script>
</html>)rawliteral";

String processor(const String& var){
  //Serial.println(var);
  if(var == "Suhu"){
    return Suhu1();
  }
  else if(var == "Detak Jantung"){
    return Bpm();
  }
  return String();
}

void setup() {
  // initialize digital pin 2 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT); // Sensor sebagai Input
  // initialize the serial communication:
  Serial.begin(115200);
  display.begin(i2c_Address, true); // Address 0x3C default dari display
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  sensors.begin();
  sensors.getAddress(tempDeviceAddress, 0);
  sensors.setResolution(tempDeviceAddress, resolution);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
  delayInMillis = 750 / (1 << (12 - resolution)); 
  lastTempRequest = millis(); 
  pinMode(4, OUTPUT); 
  display.display();
  display.clearDisplay();
  Home();
  display.display();
  display.clearDisplay();
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/Data", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Data().c_str());
  });
  server.on("/Bpm", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Bpm().c_str());
  });
  server.on("/Suhu", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Suhu1().c_str());
  });
  // Start server
  server.begin();
}
 
void loop() {
  suhu();
  oled();
  Serial.println(analogRead(SENSOR));
  //delay(27);
}
