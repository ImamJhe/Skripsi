
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SPI.h>
#include "Logo.h"
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFT_eSPI.h>

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempDeviceAddress;

int  resolution = 12;
unsigned long lastTempRequest = 0;
int  delayInMillis = 0;
float temperature = 0.0;

TFT_eSPI tft = TFT_eSPI();
SPIClass hspi(HSPI);
AsyncWebServer server(80);

static const int spiClk = 10000000; // 1 MHz

const int pin_DRDYB = 4;  // data ready
const int pin_ALARMB = 19; // alarm
const int pin_MISO = 12;  // MISO
const int pin_MOSI = 13;  // MOSI
const int pin_SCLK = 25;  // SCLK
const int pin_SS = 15;    // CSB

uint16_t LastTime=0;
uint16_t ThisTime;
bool BPMTiming=false;
bool BeatComplete=false;
int BPM=0;
int lastBPM=0;

int D, S, B;
int32_t c1;
int32_t c2;
int32_t c3;

int maxLife_11 = 0;
int minLife_11 = 10000000;
int maxLife_1 = 0;
int minLife_1 = 10000000;
int maxLife_2 = 0;
int minLife_2 = 10000000;
int maxLife_3 = 0;
int minLife_3 = 10000000;

int graphData_11[180];
int graphData_1[180];
int graphData_2[180]; 
int graphData_3[180]; 
float scaling_1 = 1;
float scaling_2 = 1;
float scaling_3 = 1;

int32_t getValFromChannel(int channel)
{
  byte x1;
  byte x2;
  byte x3;

  switch (channel)
  {
  case 1:
    x1 = 0x37;
    x2 = 0x38;
    x3 = 0x39;
    break;
  case 2:
    x1 = 0x3A;
    x2 = 0x3B;
    x3 = 0x3C;
    break;
  case 3:
    x1 = 0x3D;
    x2 = 0x3E;
    x3 = 0x3F;
    break;
  }
  int32_t val;

  // 3 8-bit registers combination on a 24 bit number
  val = readRegister(x1);
  val = (val << 8) | readRegister(x2);
  val = (val << 8) | readRegister(x3);

  return val;
}

void setup_ECG_2_channel()
{
  writeRegister(0x01, 0x11);
  writeRegister(0x02, 0x19);
  writeRegister(0x0A, 0x07);
  writeRegister(0x0C, 0x04);
  writeRegister(0x12, 0x04);
  writeRegister(0x14, 0x24);
  writeRegister(0x21, 0x02);
  writeRegister(0x22, 0x02);
  writeRegister(0x23, 0x02);
  writeRegister(0x27, 0x08);
  writeRegister(0x2F, 0x30);
  writeRegister(0x00, 0x01);
}

void setup_ECG_3_channel()
{
  writeRegister(0x01, 0x11);
  writeRegister(0x02, 0x19);
  writeRegister(0x03, 0x2E); //diff
  writeRegister(0x0A, 0x07);
  writeRegister(0x0C, 0x04);
  writeRegister(0x0D, 0x01); //diff
  writeRegister(0x0E, 0x02); //diff
  writeRegister(0x0F, 0x03); //diff
  writeRegister(0x10, 0x01); //diff
  writeRegister(0x12, 0x04);
  writeRegister(0x21, 0x02);
  writeRegister(0x22, 0x02);
  writeRegister(0x23, 0x02);
  writeRegister(0x24, 0x02); //diff
  writeRegister(0x27, 0x08);
  writeRegister(0x2F, 0x70); //diff
  writeRegister(0x00, 0x01);
}

//===========SPECIALIZED SPI OPTION 1
byte readRegister(byte reg)
{
  byte data;
  reg |= 1 << 7;
  hspi.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(pin_SS, LOW);
  hspi.transfer(reg);
  data = hspi.transfer(0);
  digitalWrite(pin_SS, HIGH);
  hspi.endTransaction();
  return data;
}

void wifi(void) {
  WiFiManager wifiManager;
  wifiManager.autoConnect("ADS1293_2","");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
 
}

void writeRegister(byte reg, byte data)
{
  reg &= ~(1 << 7);
  hspi.beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(pin_SS, LOW);
  hspi.transfer(reg);
  hspi.transfer(data);
  digitalWrite(pin_SS, HIGH);
  hspi.endTransaction();
}

void drawgraph_1(){
c1 = getValFromChannel(1);
graphData_1[89] = int(c1);///5.0);  //150 data points initially all 0.
Serial.print(graphData_1[89]);
Serial.print(" ");

  for(int i = 0; i <= 88; i++){
    tft.drawLine(130+i*4, 90+(minLife_1-graphData_1[i-1])/scaling_1, 130+i*4+4, 90+(minLife_1-graphData_1[i])/scaling_1,TFT_BLACK);
  }
   maxLife_1 = 0;
   minLife_1 = 10000000;  
  for(int i = 0; i <= 88; i++){
      if(graphData_1[i] > maxLife_1)
      maxLife_1 = graphData_1[i];
    if(graphData_1[i] < minLife_1)
      minLife_1 = graphData_1[i]; 
  }
  for(int i = 0; i <= 88; i++){//shift graph
  graphData_1[i] = graphData_1[i+1]; 
  }
  int maxPixelsAllowed = 70; //will scale the graph if it's over this...
  if(  (maxLife_1 - minLife_1) >= maxPixelsAllowed){
    scaling_1 = (maxLife_1 - minLife_1)/(int(maxPixelsAllowed));
  }
    for(int i = 0; i <= 88; i++){
    tft.drawLine(130+i*4, 90+(minLife_1-graphData_1[i-1])/scaling_1, 130+i*4+4, 90+(minLife_1-graphData_1[i])/scaling_1,TFT_GREEN);
  }

c2 = getValFromChannel(2);
graphData_2[89] = int(c2);///5.0);  //150 data points initially all 0.
Serial.print(graphData_2[89]);
Serial.print(" ");

  for(int i = 0; i <= 88; i++){
    tft.drawLine(130+i*4, 195+(minLife_2-graphData_2[i-1])/scaling_2, 130+i*4+4, 195+(minLife_2-graphData_2[i])/scaling_2,TFT_BLACK);
  }
   maxLife_2 = 0;
   minLife_2 = 10000000;  
  for(int i = 0; i <= 88; i++){
      if(graphData_2[i] > maxLife_2)
      maxLife_2 = graphData_2[i];
    if(graphData_2[i] < minLife_2)
      minLife_2 = graphData_2[i]; 
  }
  for(int i = 0; i <= 88; i++){//shift graph
  graphData_2[i] = graphData_2[i+1]; 
  }

  if(  (maxLife_2 - minLife_2) >= maxPixelsAllowed){
    scaling_2 = (maxLife_2 - minLife_2)/(int(maxPixelsAllowed));
  }
  for(int i = 0; i <= 88; i++){
    tft.drawLine(130+i*4, 195+(minLife_2-graphData_2[i-1])/scaling_2, 130+i*4+4, 195+(minLife_2-graphData_2[i])/scaling_2,TFT_GREEN);
  } 

c3 = getValFromChannel(3);
graphData_3[89] = int(c3);///5.0);  //150 data points initially all 0.
Serial.println(graphData_3[89]);

  for(int i = 0; i <= 88; i++){
    tft.drawLine(130+i*4, 300+(minLife_3-graphData_3[i-1])/scaling_3, 130+i*4+4, 300+(minLife_3-graphData_3[i])/scaling_3,TFT_BLACK);
  }
maxLife_3 = 0;
minLife_3 = 10000000;  
  for(int i = 0; i <= 88; i++){
      if(graphData_3[i] > maxLife_3)
      maxLife_3 = graphData_3[i];
    if(graphData_3[i] < minLife_3)
      minLife_3 = graphData_3[i]; 
  }
  for(int i = 0; i <= 88; i++){//shift graph
  graphData_3[i] = graphData_3[i+1]; 
  }

  if(  (maxLife_3 - minLife_3) >= maxPixelsAllowed){
    scaling_3 = (maxLife_3 - minLife_3)/(int(maxPixelsAllowed));
  }
  for(int i = 0; i <= 88; i++){
    tft.drawLine(130+i*4, 300+(minLife_3-graphData_3[i-1])/scaling_3, 130+i*4+4, 300+(minLife_3-graphData_3[i])/scaling_3,TFT_GREEN);
  }
}

void Suhu(void) {
  // this part keeps the var temperature up to date as possible.
  if (millis() - lastTempRequest >= delayInMillis) // waited long enough??
  {
    sensors.requestTemperatures(); 
    temperature = sensors.getTempCByIndex(0);
    lastTempRequest = millis(); 
  }
}

void Layar() {
ThisTime = millis();
int c11 = getValFromChannel(2);
graphData_11[177] = int (c11);  
maxLife_11 = 0;
minLife_11 = 10000000;  
  for(int i = 0; i <= 88; i++){
    if(graphData_11[i] > maxLife_11)
      maxLife_11 = graphData_11[i];
    if(graphData_11[i] < minLife_11)
      minLife_11 = graphData_11[i]; 
  }
  int x11 = maxLife_11 - 40000;
  int x12 = maxLife_11 - 60000;
  for(int i = 0; i <= 88; i++){//shift graph
  graphData_11[i] = graphData_11[i+1]; 
  }
  
  if(c11>x11) {
    if(BeatComplete) {
    BPM=ThisTime-LastTime;
    BPM=int(60/(float(BPM)/1000));
    BPMTiming=false;
    BeatComplete=false; 
}
  if(BPMTiming==false) {
    LastTime=millis();
    BPMTiming=true; 
  }
}
if((c11<x12)&(BPMTiming))
    BeatComplete=true;

tft.setTextColor(TFT_GREEN, TFT_BLACK);
int padding = tft.textWidth("999", 6);
tft.setTextPadding(padding);
tft.setTextDatum(TC_DATUM);
if (BPM > 140)
BPM = lastBPM;
if (BPM < 50)
BPM = lastBPM;
tft.drawNumber(BPM,55,120,6);
lastBPM = BPM;
int padding1 = tft.textWidth("999.99", 4);
tft.setTextPadding(padding1);
tft.setTextDatum(TR_DATUM);
tft.drawFloat(temperature,2,80,290,4);
tft.setTextPadding(0);
tft.drawString("o",90,285,2);
tft.drawString("C",110, 290, 4); 

}

void Home(void) {
tft.pushImage(105,90,118,120,Logo);
tft.setTextDatum(TL_DATUM);
tft.drawString("WIRELESS", 230, 110,4);
tft.drawString("ECG", 230, 135,4);
tft.drawString("MedikUH", 230, 160,4);
for (int t=60; t<420; t += 1) {
tft.fillRect(t,225,2,24,TFT_SILVER);
delay (10);
 }
}

String Data() {
  int D = graphData_1[89];
  if (isnan(D)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(D);
    return String(D);
  }
}

String Data1() {
  int D1 = graphData_2[89];
  if (isnan(D1)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(D1);
    return String(D1);
  }
}

String Data2() {
  int D2 = graphData_3[89];
  if (isnan(D2)) {    
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    Serial.println(D2);
    return String(D2);
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
  chart:{ renderTo : 'chart-temperature' },
  title: { text: 'Sinyal Aktivitas Jantung' },
  series: [{
    showInLegend: false,
    data: [] 
}, {
    showInLegend: false,
    data: [] 
}, {
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
      if(chartT.series[0].data.length > 180) {
        chartT.series[0].addPoint([x, y], false, true, false);
      } else {
        chartT.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/Data", true);
  xhttp.send();
}, 34 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartT.series[1].data.length > 180) {
        chartT.series[1].addPoint([x, y], false, true, false);
      } else {
        chartT.series[1].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/Data1", true);
  xhttp.send();
}, 34 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var x = (new Date()).getTime(),
          y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if(chartT.series[2].data.length > 180) {
        chartT.series[2].addPoint([x, y], false, true, false);
      } else {
        chartT.series[2].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/Data2", true);
  xhttp.send();
}, 34 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Bpm").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Bpm", true);
  xhttp.send();
}, 100 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("Suhu").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/Suhu", true);
  xhttp.send();
}, 100 ) ;

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

  pinMode(pin_DRDYB, INPUT);
  pinMode(pin_ALARMB, INPUT);
  pinMode(pin_SS, OUTPUT);

  Serial.begin(115200);
  SPIFFS.begin (true);
  tft.setSwapBytes(true);
    tft.setRotation(1);     //Lanscape
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    Home();
    wifi();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.fillRect(120,0,10,320,TFT_BLUE);
    tft.fillRect(0,275,120,5,TFT_BLUE); 
    
  hspi.begin(pin_SCLK, pin_MISO, pin_MOSI, pin_SS);
  
  setup_ECG_3_channel();
    sensors.begin();
    sensors.getAddress(tempDeviceAddress, 0);
    sensors.setResolution(tempDeviceAddress, resolution);
    sensors.setWaitForConversion(false);
    sensors.requestTemperatures();
    delayInMillis = 750 / (1 << (12 - resolution)); 
    lastTempRequest = millis(); 
    
  for(int i = 0; i < 88; i++) {
    graphData_1[i] = 0; }
  for(int i = 0; i < 88; i++) {
    graphData_2[i] = 0; }
  for(int i = 0; i < 88; i++) {
    graphData_3[i] = 0; }
    
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
  server.on("/Data1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Data1().c_str());
  });
  server.on("/Data2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Data2().c_str());
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
  drawgraph_1();
  Suhu();
  tft.fillRect(130,0,5,320,TFT_BLACK); 
}
