#define BLINKER_WIFI
#define BLINKER_MIOT_OUTLET

#include <Arduino.h>
#include <Blinker.h>
#include <Stepper.h>
#include <SoftwareSerial.h>



#define MAX_BUFFER_SIZE 50 // 定义缓冲区大小

char auth[] = "e736e69f24bd";
char ssid[] = "test";
char pswd[] = "123456789";

bool oState = false;

const int dirPin = 13;
const int stepperPin = 12;
const int EN = 14;
const int buttonPin = 2;
const int stepsPerRevolution = 360;

byte buffer[MAX_BUFFER_SIZE]; // 创建一个存储接收数据的缓冲区
int bufferSize = 0;           // 缓冲区中已接收字节数
String hexData = "";

SoftwareSerial mySerial(4, 5); // RX, TX
Stepper myStepper(stepsPerRevolution, stepperPin, dirPin);

BlinkerButton Button1("btn-abd");

// 函数声明
void rotate360();
void step(bool dir, int steps);
void miotPowerState(const String &state);
void dataRead(const String &data);

// 按钮回调函数
void button1_callback(const String &state) {
    if (Serial.available() > 0)
        BLINKER_LOG("get button state: ", state);
    digitalWrite(EN, LOW);
    step(false, 360); // 设置步数
}

// 步进电机控制函数
void step(bool dir, int steps) {
    digitalWrite(dirPin, dir);
    delay(50);

    for (int i = 0; i < steps; i++) {
        digitalWrite(stepperPin, HIGH);
        delayMicroseconds(1600);
        digitalWrite(stepperPin, LOW);
        delayMicroseconds(1600);
    }
}

// MIOT电源状态处理函数
void miotPowerState(const String &state) {
    BLINKER_LOG("Power state: ", state);

    if (state == BLINKER_CMD_ON) {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(EN, LOW);
        step(false, 360);
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();

        oState = true;
    }
}

// Blinker数据读取处理函数
void dataRead(const String &data) {
    BLINKER_LOG("Blinker readString: ", data);

    Blinker.vibrate();

    uint32_t BlinkerTime = millis();

    Blinker.print("millis", BlinkerTime);
}

void setup() {
    Serial.begin(9600);      // 初始化串口通信
    mySerial.begin(9600);    // 初始化虚拟串口通信
    BLINKER_DEBUG.stream(Serial);
    BLINKER_DEBUG.debugAll();

    Button1.attach(button1_callback);

    Blinker.begin(auth, ssid, pswd);

    pinMode(dirPin, OUTPUT);
    pinMode(stepperPin, OUTPUT);
    pinMode(EN, OUTPUT);
    Blinker.attachData(dataRead);
    BlinkerMIOT.attachPowerState(miotPowerState);

    pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
    Blinker.run();

    // 按钮触发旋转
    if (digitalRead(buttonPin) == LOW) {
        rotate360();
        delay(1000); // 按钮防抖延迟
    }

    // 接收虚拟串口数据
    while (mySerial.available()) {
        byte data = mySerial.read();        // 使用 byte 类型来读取十六进制数据
        hexData += String(data, HEX);       // 将十六进制数据添加到积累的字符串中
        if (hexData.length() == 2) {        // 检测是否接收到完整的十六进制数
            char character = char(strtol(hexData.c_str(), NULL, 16)); // 转换为字符并显示
            Serial.print(character);

            if (character == 'A') {
                digitalWrite(EN, LOW);  // 使能步进电机
                digitalWrite(dirPin, LOW); // 设置旋转方向为顺时针
                for (int i = 0; i < stepsPerRevolution; i++) { // 发送脉冲来驱动步进电机旋转
                    digitalWrite(stepperPin, HIGH);
                    delayMicroseconds(1600); // 设置脉冲宽度
                    digitalWrite(stepperPin, LOW);
                    delayMicroseconds(1600); // 设置脉冲间隔
                }
            } else if (character == '0') {
                digitalWrite(stepperPin, HIGH);  // 禁用步进电机
            }
            hexData = ""; // 重置积累的字符串，准备接收下一个十六进制数
        }
    }
}

// 顺时针旋转360度
void rotate360() {
    const int steps = 360; // 根据你的步进电机调整
    for (int i = 0; i < steps; i++) {
        digitalWrite(stepperPin, HIGH);
        delayMicroseconds(1600);
        digitalWrite(stepperPin, LOW);
        delayMicroseconds(1600);
    }
}
