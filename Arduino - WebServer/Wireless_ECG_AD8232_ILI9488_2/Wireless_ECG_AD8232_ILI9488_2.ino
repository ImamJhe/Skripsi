#include <WiFiManager.h>
#include <SPI.h>
#include "Logo.h"
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFT_eSPI.h>

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;
#define Sensor 36
#define UpperThreshold 2120
#define LowerThreshold 1920


int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0.0;


TFT_eSPI tft = TFT_eSPI();
SPIClass hspi(HSPI);
AsyncWebServer server(80);

uint16_t LastTime=0;
uint16_t ThisTime;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;
int lastBPM=0;

uint16_t a=130;
uint16_t t1=131;
uint16_t lasta=130;
uint16_t lastb=40;

/*void wifi(void) {
  WiFiManager wifiManager;
  wifiManager.autoConnect("AD8232_2","");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
 
}*/

void Home(void) {
tft.pushImage(105,90,118,120,Logo);
tft.setTextDatum(TL_DATUM);
tft.drawString("WIRELESS", 230, 110,4);
tft.drawString("ECG", 230, 135,4);
tft.drawString("MedikUH", 230, 160,4);
WiFiManager wifiManager;
  wifiManager.autoConnect("AD8232_1","");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
 tft.drawString("IP :" + WiFi.localIP().toString(),135,265,4);
for (int t=60; t<420; t += 1) {
tft.fillRect(t,225,2,24,TFT_SILVER);
delay (10);
 }
}

void Layar() {
int value = analogRead(Sensor);

if(a>480)
{
t1=131;
a=130;
lasta=a;
}
tft.drawRect(130,0,1,320,TFT_BLACK); 
tft.setTextColor(TFT_WHITE, TFT_BLACK);
int b=420-(value/8); // AD8232
if (b<0)
b = 0;
Serial.println(value);


tft.drawLine(lasta, lastb, a, b, TFT_GREEN);
tft.fillRect(t1,0,4,320,TFT_BLACK);

lastb=b;
lasta=a;

ThisTime=millis();
  if(value>UpperThreshold) {
    if(BeatComplete) {
    BPM= ThisTime-LastTime;
    BPM= int(60/(float(BPM)/1000));
    BPMTiming=false;
    BeatComplete=false; 
}
  if(BPMTiming==false) {
    LastTime=millis();
    BPMTiming=true; 
  }
}
if((value<LowerThreshold)&(BPMTiming))
    BeatComplete=true;

tft.setTextColor(TFT_GREEN, TFT_BLACK);
int padding = tft.textWidth("9999", 6);
tft.setTextPadding(padding);
tft.setTextDatum(TC_DATUM);


if ((BPM<40||BPM>140)) {
BPM = lastBPM; }


tft.drawNumber(BPM,55,120,6);
lastBPM = BPM;
int padding1 = tft.textWidth("999.99", 4);
tft.setTextPadding(padding1);
tft.setTextDatum(TR_DATUM);
tft.drawFloat(temperature,2,80,290,4);
tft.setTextPadding(0);
tft.drawString("o",90,285,2);
tft.drawString("C",110, 290, 4); 

a+=2;
t1+=2;
}

void Suhu(void) {
  // this part keeps the var temperature up to date as possible.
  if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
  {
    temperature = sensors.getTempCByIndex(0);
    sensors.requestTemperatures(); 
    lastTempRequest = millis(); 
  }
}

String Data() {
  float D = analogRead(Sensor);
  if (isnan(D)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    
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
    
    return String(B);
    //delay(9);
  }
}

String Suhu1() {
  float S = sensors.getTempCByIndex(0);
  if (isnan(S)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    
    return String(S);
    //delay(9);
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
}, 300 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Suhu").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Suhu", true);
  xhttp.send();
}, 300 ) ;

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

  Serial.begin(115200);
  SPIFFS.begin (true);
    tft.setSwapBytes(true);
    tft.setRotation(1);     //Lanscape
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    Home();
    //wifi();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillRect(120,0,10,320,TFT_BLUE);
    tft.fillRect(0,275,120,5,TFT_BLUE); 
 
    sensors.begin();
    sensors.getAddress(tempDeviceAddress, 0);
    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - resolution)); 
    lastTempRequest = millis();
    
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
    
  tft.setTextColor(TFT_BLACK);
}

void loop() {
  Layar();
  Suhu();
  tft.fillRect(130,0,5,320,TFT_BLACK);
  delay(9);
}
