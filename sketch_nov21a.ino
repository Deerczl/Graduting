/*实际面积公式
  RS=(0.4542 * H - 0.2375 ) / S * 9
  S为OpenMV3测量所得面积
  H为VL53L0X测量所得高度
*/

/*
   高度与激光测距的函数关系
  y = -2E-06x3 + 0.0009x2 - 0.1673x + 25.704

  高度与显示面积的关系
  y = 0.7004x2 - 2.5726x + 17.679

  显示面积与实际面积的关系
  y = 0.4746x + 3.0124
*/

#include <Arduino.h>
#include <stdlib.h>
#include <math.h>
#include "TINY5110.h"

#define PIN_LCD_SCE   43
#define PIN_LCD_RESET 41
#define PIN_LCD_DC    45
#define PIN_LCD_SDIN  47
#define PIN_LCD_SCLK  49

TINY5110 lcd(PIN_LCD_SCE, PIN_LCD_RESET, PIN_LCD_DC, PIN_LCD_SDIN, PIN_LCD_SCLK);

String inString = " ";
int S, Light;
char  c[] = "";
int a, b, i, n, flag;
double dis, dis_sum, RS, H, V;
double S_avr, S_final;



void setup() {
  Serial.begin(9600);
  delay(50);
  
  Serial3.begin(9600);//与Openmv通信波特率
  
  VL53L0X_set();//封装函数
  
  lcd.begin();//lcd初始化
  lcd.setContrast(60);//lcd字深设置

  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);//lcd背光设置
}

void loop() {
  void clear();
  Light_measure();
  H_measure();
  S_measure();
  V_count();
  lcd_display();
}

void lcd_display() {
  dtostrf(dis_sum, 2, 2, c);//转数字为字符串
  lcd.gotoXY(0, 0);//坐标移动
  lcd.writeString("                                         ");//清除显示
  lcd.gotoXY(0, 0);
  lcd.writeString(c);
  lcd.gotoXY(60, 0);
  lcd.writeString("cm");
  delay(10);

  dtostrf(S_final, 2, 2, c);
  lcd.gotoXY(0, 1);
  lcd.writeString("                                         ");
  lcd.gotoXY(0, 1);
  lcd.writeString(c);
  lcd.gotoXY(60, 1);
  lcd.writeString("cm2");
  delay(10);

  dtostrf(V, 2, 2, c);
  lcd.gotoXY(0, 2);
  lcd.writeString("                                         ");
  lcd.gotoXY(0, 2);
  lcd.writeString(c);
  lcd.gotoXY(60, 2);
  lcd.writeString("cm3");
  delay(10);
}

void VL53L0X_set() {
  Serial2.begin(9600);//默认波特率9600
  Serial2.write(0xD0);
  delay(50);
  //模块比特率探测
  if (Serial2.read() == 0xD0) {
    Serial.println("Bpm 9600");
  } else
  {
    Serial.println("Bpm Error!");
    while (1);
  }
  Serial2.write(0xC1);
  delay(50);
  //  设置高速测量模式
}

void H_measure() {

  dis_sum = 0;
  for (i = 0; i < 50; i++)
  {
    Serial2.write(0xA0);//单次输出模式
    delay(50);
    a = Serial2.read();
    b = Serial2.read();
    dis = a * 256 + b;//输出距离为（a<< 8+ b）,单位mm
    dis_sum += dis;
  }

  dis_sum = dis_sum / 50;

  dis_sum = + (-2) * pow(10, -8 ) * pow(dis_sum, 4)
            + ( 8) * pow(10, -6 ) * pow(dis_sum, 3)
            - 0.0009 * pow(dis_sum, 2)
            - 0.0221 * dis_sum
            + 22.053;

  //高度与激光测距的函数关系
  //y = -2E-08x4 + 8E-06x3 - 0.0009x2 - 0.0221x + 22.053

  Serial.print(dis_sum);
  Serial.println("  cm");

}

void S_measure() {
  while (Serial3.available() > 0) {
    int inChar = Serial3.read();
    if (isDigit(inChar)) {
      inString += (char)inChar;
    }
    if (inChar == '\n') {
      S = inString.toInt();
      inString = "";
    }
  }

  //高度与显示面积的关系
  //y = 0.7925x4 - 19.255x3 + 165.93x2 - 309.68x + 1084.4
  // y = 8E-11x3 - 1E-06x2 + 0.0139x - 4.3494
  S = S - 9.2737 * pow(dis_sum, 3)
      + 150.94 * pow(dis_sum, 2)
      - 772.37 * dis_sum
      + 754.24;

  S_final = 0.0088 * S + 0.7812;

  if (S_final < 0)  S_final = (-S_final);

  Serial.print(S_final);
  Serial.println("  cm2");

}

void V_count() {
  V = S_final *  dis_sum ;
  Serial.print(V);
  Serial.println("  cm3");
  Serial.println();
}

void Light_measure() {
  Light = analogRead(A0);
  //电压计算方法 电压=读取值*5V/1024
  if (Light > 800)
    Serial.println("Too Dark!");
  else if (Light < 200)
    Serial.println("Too Bright!");
  else
    Serial.println("Light is fine.");
}

