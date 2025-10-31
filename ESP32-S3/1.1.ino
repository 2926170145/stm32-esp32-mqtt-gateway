#include <ModbusMaster.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// ============ RS485 & Modbus 部分 ============
ModbusMaster node;
#define RS485_DIR  4   // TTL转485模块的DIR引脚
const uint8_t SLAVE_ID = 4;            // 从机地址
const uint16_t START_REGISTER = 0;     // 起始寄存器
const uint8_t NUM_REGISTERS = 4;       // 读取数量
const uint32_t READ_INTERVAL = 5000;   // 增加到5秒读取间隔

// ============ WiFi & MQTT 配置 ============
const char* ssid = "TP-LINK_1FFB";
const char* password = "123123123";
const char* mqtt_server = "tb7a114a.ala.cn-hangzhou.emqxsl.cn";
const int mqtt_port = 8883;
const char* mqtt_username = "esp-32";
const char* mqtt_password = "admin";
const char* mqtt_clientId = "esp32_modbus_mqtt_001";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// ============ 函数声明 ============
void connectWiFi();
void setupMQTT();
void connectMQTT();
void readModbusAndPublish();
void preTransmission();
void postTransmission();
void resetSlaveCommunication();

// ============ RS485方向控制 ============
void preTransmission() {
  delay(2);
  digitalWrite(RS485_DIR, HIGH);
  delayMicroseconds(50);
}

void postTransmission() {
  digitalWrite(RS485_DIR, LOW);
  delayMicroseconds(100);
  // 清空可能残留的数据
  while(Serial1.available()) {
    Serial1.read();
  }
}

// ============ 从机恢复函数 ============
void resetSlaveCommunication() {
  Serial.println(" 恢复从机通信...");
  
  // 彻底清空缓冲区
  while(Serial1.available()) {
    Serial1.read();
  }
  
  // 重新初始化Modbus
  postTransmission();
  delay(200);
  
  node.begin(SLAVE_ID, Serial1);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);
  
  Serial.println(" 通信已重置");
}

// ============ 初始化 ============
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== ESP32-S3 Modbus + MQTT 综合测试 ===");

  pinMode(RS485_DIR, OUTPUT);
  postTransmission();

  // 初始化 Modbus 通信
  Serial1.begin(9600, SERIAL_8N1, 18, 17); // RX=18, TX=17
  node.begin(SLAVE_ID, Serial1);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  // WiFi + MQTT 初始化
  connectWiFi();
  setupMQTT();
  Serial.println("初始化完成，开始循环...");
}

// ============ WiFi连接 ============
void connectWiFi() {
  Serial.print("连接WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n WiFi连接成功");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi连接失败");
  }
}

// ============ MQTT设置 ============
void setupMQTT() {
  espClient.setInsecure();  // 跳过证书验证
  mqttClient.setServer(mqtt_server, mqtt_port);
}

// ============ MQTT连接 ============
void connectMQTT() {
  Serial.print("连接MQTT服务器...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(mqtt_clientId, mqtt_username, mqtt_password)) {
      Serial.println(" 成功连接MQTT");
      
      // 订阅多个主题
      bool sub1 = mqttClient.subscribe("test/sensor");
      bool sub2 = mqttClient.subscribe("sensor/data");
      bool sub3 = mqttClient.subscribe("test/control");
      
      Serial.printf("订阅状态: test/sensor=%s, sensor/data=%s, test/control=%s\n",
                   sub1 ? "成功" : "失败", 
                   sub2 ? "成功" : "失败",
                   sub3 ? "成功" : "失败");
                   
    } else {
      Serial.print(" 失败, 状态码: ");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

// ============ Modbus读取 + MQTT发布 ============
void readModbusAndPublish() {
  static uint32_t lastReadTime = 0;
  static uint8_t errorCount = 0;
  const uint8_t MAX_ERRORS = 3;
  
  if (millis() - lastReadTime < READ_INTERVAL) return;
  lastReadTime = millis();

  Serial.println("\n📡 读取Modbus数据...");
  
  // 清空接收缓冲区
  while(Serial1.available()) {
    Serial1.read();
  }
  
  uint8_t result = node.readHoldingRegisters(START_REGISTER, NUM_REGISTERS);

  if (result == node.ku8MBSuccess) {
    errorCount = 0; // 成功则重置错误计数
    
    // 直接读取各个部分的数据（已经是8位数据存储在16位寄存器中）
    uint8_t temp_int = node.getResponseBuffer(0);  // 寄存器0: 温度整数
    uint8_t temp_dec = node.getResponseBuffer(1);  // 寄存器1: 温度小数
    uint8_t hum_int  = node.getResponseBuffer(2);  // 寄存器2: 湿度整数
    uint8_t hum_dec  = node.getResponseBuffer(3);  // 寄存器3: 湿度小数

    // 组合成实际的温湿度值
    float temperature = temp_int + (temp_dec / 100.0);
    float humidity = hum_int + (hum_dec / 100.0);

    Serial.printf("原始数据: temp_int=%d, temp_dec=%d, hum_int=%d, hum_dec=%d\n", 
                  temp_int, temp_dec, hum_int, hum_dec);
    Serial.printf("温度: %.2f °C, 湿度: %.2f %%RH\n", temperature, humidity);

    // 生成 JSON 并发布到多个主题
    String jsonMsg = "{\"temperature\":" + String(temperature, 2) +
                     ",\"humidity\":" + String(humidity, 2) +
                     ",\"time\":" + String(millis()) + "}";

    bool pub1 = mqttClient.publish("test/sensor", jsonMsg.c_str());
    bool pub2 = mqttClient.publish("sensor/data", jsonMsg.c_str());
    bool pub3 = mqttClient.publish("test/hello", "温湿度数据更新");
    
    Serial.printf("发布状态: test/sensor=%s, sensor/data=%s, test/hello=%s\n",
                 pub1 ? "成功" : "失败", 
                 pub2 ? "成功" : "失败",
                 pub3 ? "成功" : "失败");
                 
    if (pub1 || pub2 || pub3) {
      Serial.println(" 消息内容: " + jsonMsg);
    } else {
      Serial.println(" 所有主题发布失败");
    }

  } else {
    errorCount++;
    Serial.printf(" Modbus读取失败, 错误码: %d, 连续错误: %d/%d\n", result, errorCount, MAX_ERRORS);
    
    // 清空缓冲区
    while(Serial1.available()) {
      Serial1.read();
    }
    
    // 错误太多时重置通信
    if (errorCount >= MAX_ERRORS) {
      resetSlaveCommunication();
      errorCount = 0;
    }
    
    delay(200);
  }
}

// ============ 主循环 ============
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" WiFi掉线,重连中...");
    connectWiFi();
  }

  if (!mqttClient.connected()) {
    connectMQTT();
  }

  mqttClient.loop();
  readModbusAndPublish();
  delay(100);
}