#include <SoftwareSerial.h>
#include <ModbusMaster.h>

SoftwareSerial mySerial(12, 13);  // Khai báo chân kết nối của RS485 to TTL UART
ModbusMaster node;

bool receivedData = false;  // Biến boolean để theo dõi trạng thái nhận dữ liệu từ ES-PH-SOIL-01

void setup() {
  Serial.begin(4800);       // Khởi tạo baudrate cho Serial Monitor
  mySerial.begin(4800);     // Khởi tạo baudrate cho RS485 to TTL UART
  node.begin(2, mySerial);  // Khởi tạo Modbus RTU với slave ID là 1 và sử dụng RS485 to TTL UART
}

void loop() {
  uint8_t result;
  uint16_t data[16];

  result = node.readHoldingRegisters(0, 5);  // Đọc 5 thanh ghi bắt đầu từ địa chỉ 0
  Serial.println(result);
  Serial.println(node.ku8MBSuccess);
  if (result == node.ku8MBSuccess) {                     // Nếu đọc dữ liệu thành công
    Serial.println("Data received from ES-PH-SOIL-01");  // In thông báo trên Serial Monitor
    receivedData = true;                                 // Cập nhật biến boolean về trạng thái true để cho biết đã nhận được dữ liệu từ ES-PH-SOIL-01
  }

  if (receivedData) {  // Nếu đã nhận được dữ liệu từ ES-PH-SOIL-01
    // Gửi dữ liệu đến Arduino Uno
    Serial.print("Data from ES-PH-SOIL-01: ");
    Serial.println(node.getResponseBuffer(0));
    receivedData = false;  // Cập nhật biến boolean về trạng thái false để sẵn sàng nhận dữ liệu từ ES-PH-SOIL-01 tiếp theo
  }
  delay(3000);
}
