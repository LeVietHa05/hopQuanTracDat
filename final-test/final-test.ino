#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// Insert your network credentials
#define WIFI_SSID "AmericanStudy T1"
#define WIFI_PASSWORD "66668888"
// Insert Firebase project API Key
#define API_KEY "AIzaSyBCqLto76Wdu4gCojSE9XoF1idZYXL6vYM"
// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://quan-trac-dat-prj-default-rtdb.asia-southeast1.firebasedatabase.app/"
//Define Firebase Data object
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
float ph = 20.0;
float Nitrogen = 20.0;
float Phosphorus = 20.0;
float Potassium = 20.0;
float soilmoisturepercent = 20.0;
float temp = 20.0;
char timestamp[20];



int phRX = 12;
int phTX = 14;
int npkRX = 4;
int npkTX = 13;

SoftwareSerial phUART(phRX, phTX);
SoftwareSerial npkUART(npkRX, npkTX);

// Khởi tạo đối tượng ModbusMaster cho 2 node
ModbusMaster node1;
ModbusMaster node2;

bool receivedDataPH = false;
bool receivedDataNPK = false;

const int AirValue = 750;  //you need to replace this value with Value_1
const int WaterValue = 430;
int sensorpin = A0;  //you need to replace this value with Value_2
int soilMoistureValue = 0;


void setup() {
  // Khởi tạo baudrate cho Serial Monitor và RS485 to TTL UART
  Serial.begin(4800);
  phUART.begin(4800);
  node2.begin(2, phUART);
  npkUART.begin(4800);
  node1.begin(3, npkUART);
  sensors.begin();
  //wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  timeClient.begin();
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  //firebase
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


  // Khởi tạo Modbus RTU với slave ID là 2 và sử dụng RS485 to TTL UART
}

void loop() {
  if (phUART.isListening()) {
    Serial.println("nghe dc ph");
  } else {
    Serial.println("ph bi diec");
  }
  if (npkUART.isListening()) {
    Serial.println("nghe dc npk");
  } else {
    Serial.println("npk bi diec");
  }
  // Khai báo biến và mảng lưu dữ liệu đọc được từ 2 node
  uint8_t result1, result2;
  uint16_t data1[5], data2[1];
  Serial.println("Các chỉ số của đất");


  result2 = node2.readHoldingRegisters(0, 3);

  // Đọc dữ liệu từ node ES-PH-SOIL

  // In thông báo trên Serial Monitor và cập nhật biến boolean khi đọc dữ liệu thành công
  if (result2 == node2.ku8MBSuccess) {

    receivedDataPH = true;
  } else {
    Serial.println("sai ph");
    Serial.println(result2);
    Serial.println(node2.ku8MBSuccess);
  }
  // In dữ liệu đọc được từ node ES-PH-SOIL
  if (receivedDataPH) {

    ph = (float)node2.getResponseBuffer(0) / 10.0;
    Serial.print("Soil pH: ");
    Serial.println(ph);

    // Reset biến boolean để chuẩn bị đọc dữ liệu từ node ES-NPK-SOIL
    receivedDataPH = false;
  }



  // Đọc dữ liệu từ node ES-NPK-SOIL
  result1 = node1.readHoldingRegisters(0, 3);
  Serial.println(result1);
  Serial.println(node1.ku8MBSuccess);
  // In thông báo trên Serial Monitor và cập nhật biến boolean khi đọc dữ liệu thành công
  if (result1 == node1.ku8MBSuccess) {
    receivedDataNPK = true;
  } else {
    Serial.println("sai npk");
  }
  // In dữ liệu đọc được từ node ES-NPK-SOIL
  if (receivedDataNPK) {
    Nitrogen = (float)node1.getResponseBuffer(0);
    Phosphorus = (float)node1.getResponseBuffer(1);
    Potassium = (float)node1.getResponseBuffer(2);
    Serial.print("Nitrogen: ");
    Serial.print(Nitrogen);
    Serial.println(" mg/Kg");
    Serial.print("Phosphorus: ");
    Serial.print(Phosphorus);
    Serial.println(" mg/Kg");
    Serial.print("Potassium: ");
    Serial.print(Potassium);
    Serial.println(" mg/Kg");

    // Reset biến boolean để chuẩn bị đọc dữ liệu từ node ES-PH-SOIL
    receivedDataNPK = false;
  }

  soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if (soilmoisturepercent >= 100) {
    Serial.println("100 %");
  } else if (soilmoisturepercent <= 0) {
    Serial.println("0 %");
  } else if (soilmoisturepercent > 0 && soilmoisturepercent < 100) {
    Serial.print("Độ ẩm hiện tại: ");
    Serial.print(soilmoisturepercent);
    Serial.println("%");
  }
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);
  Serial.print("Độ ẩm hiện tại: ");
  Serial.print(temp);
  Serial.println("do C");

  time_t t = timeClient.getEpochTime(); // Lấy thời gian hiện tại
  Serial.print("Thoi gian: ");
  Serial.print(day(t));
  Serial.print("/");
  Serial.print(month(t));
  Serial.print("/");
  Serial.print(year(t));
  Serial.print(" ");
  Serial.print(hour(t));
  Serial.print(":");
  Serial.print(minute(t));
  Serial.print(":");
  Serial.println(second(t));
  delay(1000);
  sprintf(timestamp, "%02d/%02d/%04d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));


  Firebase.RTDB.setFloat(&fbdo, "Soil parameter/Ph", ph);
  Firebase.RTDB.setFloat(&fbdo, "Soil parameter/Nitrogen", Nitrogen);
  Firebase.RTDB.setFloat(&fbdo, "Soil parameter/Phosphorus", Phosphorus);
  Firebase.RTDB.setFloat(&fbdo, "Soil parameter/Potassium", Potassium);
  Firebase.RTDB.setFloat(&fbdo, "Soil parameter/Humidity", soilmoisturepercent);
  Firebase.RTDB.setFloat(&fbdo, "Soil parameter/Temperature", temp);
  Firebase.RTDB.setString(&fbdo, "Soil parameter/Timestamp", timestamp );
  delay(300);
}
