/*!
 * @file UI_button.ino
 * @brief Create a button control on the screen, and users can customize the parameters of the button.
 * There are three buttons created in this demo: on, off, and clr. Press button on and off, the character "ON" and "OFF" will be 
 * @n displayed in the text box. Press the clr button to delete a character in the text box.
 * @n Click the text box to move the cursor into it when you need a text box to display characters.
 * @n The demo supports Mega2560, FireBeetle-ESP32, FireBeetle-ESP8266, and FireBeetle-M0.
 *
 * @copyright Copyright (c) 2010 DFRobot Co. Ltd (http://www.dfrobot.com)
 * @license The MIT License (MIT)
 * @author [fengli] (li.feng@dfrobot.com)
 * @version V1.0
 * @date 2019-12-6
 * @get from https://www.dfrobot.com
 * @url https://github.com/DFRobot/DFRobot_GDL/src/DFRpbot_UI
*/
#include "DFRobot_UI.h"
#include "Arduino.h"
#include "DFRobot_GDL.h"
#include "DFRobot_Touch.h"
/*M0*/
#if defined ARDUINO_SAM_ZERO
#define TFT_DC  7
#define TFT_CS  5
#define TFT_RST 6
/*ESP32 and ESP8266*/
#elif defined(ESP32) || defined(ESP8266)
#define TFT_DC  D2
#define TFT_CS  D6
#define TFT_RST D3
/* AVR series mainboard */
#else
#define TFT_DC  2
#define TFT_CS  3
#define TFT_RST 4
#endif

/**
   @brief Constructor  When the touch uses the gt series chip, you can call this constructor
*/
DFRobot_Touch_GT911 touch;

/**
   @brief Constructor When the screen uses hardware SPI communication, the driver IC is ILI9488, and the screen resolution is 320x480, this constructor can be called
   @param dc Command/data line pin for SPI communication
   @param cs Chip select pin for SPI communication
   @param rst Reset pin of the screen
*/
DFRobot_ILI9488_320x480_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);
/* M0 mainboard DMA transfer */
//DFRobot_ILI9488_320x480_HW_SPI screen(/*dc=*/TFT_DC,/*cs=*/TFT_CS,/*rst=*/TFT_RST);

/**
   @brief Constructor
   @param gdl Screen object
   @param touch Touch object
*/
DFRobot_UI ui(&screen, &touch);
//Callback function for three buttons
void btnCallback(DFRobot_UI::sButton_t &btn,DFRobot_UI::sTextBox_t &obj) {
   String text((char *)btn.text);
   if(text == "ON"){
    obj.setText("you have touch button on");
    }
   else if(text == "OFF"){
    obj.setText("you have touch button off");
    }
   else if(text == "clr"){
    obj.deleteChar();
    }
    
}

void setup()
{

  Serial.begin(115200);
  //Initialize UI 
  ui.begin();
  //Set the UI theme, there are two themes to choose from: CLASSIC and MODERN.
  ui.setTheme(DFRobot_UI::MODERN);
  //Create a text box control
  DFRobot_UI::sTextBox_t & tb = ui.creatText();
  tb.bgColor = 0xe6B6;
  ui.draw(&tb);
  //Create a button control on the screen
  DFRobot_UI::sButton_t & btn1 = ui.creatButton();
  //Set the name of the button
  btn1.setText("ON");
  btn1.bgColor = COLOR_RGB565_RED;
  btn1.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn1.setOutput(&tb);
  ui.draw(&btn1,/**x=*/screen.width()/10,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
  
  DFRobot_UI::sButton_t & btn2 = ui.creatButton();
  btn2.setText("OFF");
  btn2.bgColor = COLOR_RGB565_GREEN;
  btn2.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  btn2.setOutput(&tb);
  ui.draw(&btn2,/**x=*/(screen.width()/10)*4,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
 
  DFRobot_UI::sButton_t & btn3 = ui.creatButton();
  btn3.bgColor = COLOR_RGB565_BLUE;
  btn3.setText("clr");

  //Set the callback function of the button
  btn3.setCallback(btnCallback);
  //Each button has a text box, its parameter needs to be set by yourself.
  
  btn3.setOutput(&tb);
  ui.draw(&btn3,/**x=*/(screen.width()/10)*7,/**y=*/screen.height()/2,/*width*/screen.width()/10*2,/*height*/screen.width()/10*2);
}
void loop()
{
  //refresh
  ui.refresh();
}
