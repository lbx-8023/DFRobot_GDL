#include "DFRobot_Touch.h"
#include <DFRobot_Type.h>
#include "Wire.h"
#define ADDR 0x5D

GDL_IF_PB_DEV(gdl_Dev_GTXXX_TOUCH_HW_IIC, NULL, DEV_TYPE_TOUCH, IF_COM_HW_IIC)
GDL_IF_PB_DEV(gdl_Dev_XPT2046_TOUCH_HW_SPI, NULL, DEV_TYPE_TOUCH, IF_COM_HW_SPI)

DFRobot_Touch::DFRobot_Touch(sGdlIFDev_t *dev, uint8_t addr, uint8_t rst, uint8_t irq)
  :DFRobot_IF(dev, addr, rst, irq){}
DFRobot_Touch::DFRobot_Touch(sGdlIFDev_t *dev, uint8_t cs, uint8_t rst, uint8_t irq, uint8_t dc)
  :DFRobot_IF(dev, dc, cs, rst, irq){}

DFRobot_Touch::~DFRobot_Touch(){}
void DFRobot_Touch::initTouch(){
  _points = "";
  _pNum = 0;
  memset(&_size, 0, sizeof(_size));
  memset(&_point, 0, sizeof(sPoints_t));
  initInterface();
}
void DFRobot_Touch::touchConfig(uint8_t *addr){
  uint8_t regByte = pgm_read_byte(addr++);
  uint8_t regValByte = pgm_read_byte(addr++);
  uint8_t regBuf[regByte];
  uint32_t reg = 0;
  for(uint8_t i = 0; i < regByte; i++){
      regBuf[i] = pgm_read_byte(addr++);
  }
  for(uint8_t j = 0; j < regByte; j++){
      reg |= (uint32_t)(regBuf[regByte -1 -j] << 8*j);
  }

  uint16_t length =  0;//(pgm_read_byte(addr++)<<8) | (pgm_read_byte(addr++));
  //int8_t flag = regByte - 1;
  uint8_t buf[regByte+1+regValByte];
  buf[0] = regByte;
  for(uint8_t i = 0; i < length; i++){
      memcpy(buf+1, regBuf, regByte);
      for(uint8_t j = 0; j < regValByte; j++){
          buf[regByte+1+j] = pgm_read_byte(addr++);
      }
      //Serial.println(buf[regByte+1]);
      _if.dev->talk(&_if, IF_COM_WRITE_RAM_INC, buf, regValByte);
      reg += 1;
      for(uint8_t i = 0; i < regByte; i++){
          regBuf[regByte - 1- i] = (uint8_t)(reg >> 8*i);
      }
  }
}

void DFRobot_Touch::setRotation(uint8_t rotate)
{
   direction = rotate;
}

String DFRobot_Touch::pointRemap(uint16_t &x,uint16_t &y,uint16_t _width,uint16_t _height){

   uint16_t tempX = x;
   uint16_t tempY = y;
   switch(direction){
     case 0:{
       
     break;
     }
     case 1:{
       x = tempY;
       y = _width -tempX;
     break;
    }
     case 2:{
       x = _width -tempX;
       y = _height -tempY;
     break;
     }
     case 3:{
       x = _height -tempY;
       y = tempX;
     break;
    }
     
   }
  
   return "ok";

}

DFRobot_Touch_GT911::DFRobot_Touch_GT911(uint8_t addr, uint8_t rst, uint8_t irq)
  :DFRobot_Touch(&gdl_Dev_GTXXX_TOUCH_HW_IIC, addr, rst, irq){
  id = "";
  memset(_p, 0, sizeof(_p));
}
DFRobot_Touch_GT911::~DFRobot_Touch_GT911(){
  
}
void DFRobot_Touch_GT911::begin(uint32_t freq){
  freq = freq;
  initTouch();
  char temp[4]={0};//Get chip id
  uint16_t sizeReg = 0;
  readReg(0x8140,temp,4);
  id += temp;
  
  if(id == "5688"){
      _if.dev->addr = (uint8_t *)touchGt5688ConfigTable;
      sizeReg = 0x8051;
  }else if(id == "911"){
   IC = GT911;
   //Serial.println(id);
   _if.dev->addr = (uint8_t *)touchGT911ConfigTable;
   sizeReg = 0x8048;
  }else{
   IC = FT5436;
   _if.pinList[IF_PIN_ADDR] = 0x38;
   return;
  }
  
  uint8_t *addr = _if.dev->addr;
  touchConfig(addr);
  readReg(sizeReg,temp,4);

  _size.xw = ((uint8_t)temp[1] << 8) | (uint8_t)temp[0];
  _size.yh = ((uint8_t)temp[3] << 8) | (uint8_t)temp[2];
  
  // Serial.println("_size.xw = ");Serial.println(_size.xw);
  // Serial.println("_size.yh = ");Serial.println(_size.yh);

  PIN_IN(_if.pinList[IF_PIN_BL]);
}
String DFRobot_Touch_GT911::scan(){
   
   if(IC == FT5436){
     return ft5436Scan();
   }else if(IC == GT911){
     return gt911Scan();
   }else{
     return "255,0,0,0,0 ";
   }
}
String DFRobot_Touch_GT911::gt911Scan()
{
  uint8_t flag = 0;
  uint8_t val = 0x00;
  String s = "";
  memset(_p, 0, sizeof(_p));
  readReg(0x814E, &flag, 1);
  if((flag & 0x80) ||((flag&0x0F)<6)){
      writeBuf(0x814E, &val, 1);
  }
  if((flag & 0x80) &&((flag&0x0F)<6)){
      readReg(0x814F, &_p, sizeof(_p));
      _pNum = flag&0x0F;
      for(uint8_t i = 0; i < _pNum; i++){
          _point.id = _p[i].id;
          if(id == "5688")
              _point.id &= 0x0F;
          _point.x =  _p[i].xl + (_p[i].xh << 8);
          _point.y =  _p[i].yl + (_p[i].yh << 8);
          _point.wSize = _p[i].wSize;
          _point.hSize = _p[i].hSize;
          if((_point.x <= _size.xw) && (_point.y <= _size.yh)){
              pointRemap(_point.x,_point.y,319,479);
              s += String(_point.id) + "," + String(_point.x) + "," + String(_point.y) + "," + String(_point.wSize) + ","+ String(_point.hSize) + " ";
          }
      }
  }
  //Serial.println(s);
  if(s.length() == 0){
     s = "255,0,0,0,0 ";
  }
  delay(10);
  _points = s;
  return s;
}
String DFRobot_Touch_GT911::ft5436Scan()
{
  uint8_t FT5216_Touch_Buf[43];
  uint8_t point = 0;
  String s = "";
   //I2C_read_reg(0,FT5216_Touch_Buf,33);
   readCommand(0,FT5216_Touch_Buf,33);
  if((FT5216_Touch_Buf[2] == 0x00) || (FT5216_Touch_Buf[2] == 0xFF)){
    point = FT5216_Touch_Buf[2];
  }else if(FT5216_Touch_Buf[2] > 5){
  }else{
    point = FT5216_Touch_Buf[2];
    if(point > 0x05){
      point = 0x05;
    }
    for(uint8_t i = 0; i < point; i++){
      uint16_t x = (uint16_t)(FT5216_Touch_Buf[3+6*i] & 0x0F)<<8 | (uint16_t) FT5216_Touch_Buf[4+6*i];
      uint16_t y = (uint16_t)(FT5216_Touch_Buf[5+6*i] & 0x0F) << 8 | (uint16_t) FT5216_Touch_Buf[6+6*i];
      //uint8_t event = FT5216_Touch_Buf[0x3+6*i] >> 6;
      uint8_t id = FT5216_Touch_Buf[5+6*i]>>4;
      if(x > 319){ x = 319; }
      if(y > 479){ y = 479; }
      x = 319 - x;
      y = 479 - y;
      pointRemap(x,y,319,479);
      s += String(id) + "," + String(x) + "," + String(y) + "," + String(0) + ","+ String(0) + " ";
      
	  
    }
	return s;
  }
  return "255,0,0,0,0 ";

}

DFRobot_Touch_GT911_IPS::DFRobot_Touch_GT911_IPS(uint8_t addr, uint8_t rst, uint8_t irq)
  :DFRobot_Touch_GT911(addr, rst, irq){
  id = "";

  PIN_OUT(rst);
  PIN_OUT(irq);
  memset(_p, 0, sizeof(_p));
}
DFRobot_Touch_GT911_IPS::~DFRobot_Touch_GT911_IPS(){
}
String DFRobot_Touch_GT911_IPS::pointRemap(uint16_t &x,uint16_t &y,uint16_t _width,uint16_t _height){

   uint16_t tempX = x;
   uint16_t tempY = y;
   switch(direction){
     case 0:{
        x = _width -tempX;  //X-axis mirroring
     break;
     }
     case 1:{
       x = tempY;
       y = _height -tempX;
     break;
    }
     case 2:{
       x = tempX;
       y = _height -tempY;
     break;
     }
     case 3:{
       x = _height -tempY;
       y = _width -tempX;
     break;
    }
     
   }
  
   return "ok";

}

DFRobot_Touch_XPT2046::DFRobot_Touch_XPT2046(uint8_t cs, uint8_t rst , uint8_t irq )
:DFRobot_Touch(&gdl_Dev_XPT2046_TOUCH_HW_SPI, cs, rst, irq, GDL_PIN_NC){}
DFRobot_Touch_XPT2046::~DFRobot_Touch_XPT2046(){}

void DFRobot_Touch_XPT2046::begin(uint32_t freq){
  setFrequency(freq);
  initTouch();
}
String DFRobot_Touch_XPT2046::scan(){
    uint16_t x,y,x1,y1,x2,y2;
      String s = "";
      x1 = readxy(0xD0);
      y1 = readxy(0x90);
      x2 = readxy(0xD0);
      y2 = readxy(0x90);
    if(((x2<=x1&&x1<x2+50)||(x1<=x2&&x2<x1+50))//Before and after the two samples are within +- ERR_RANGE.
    &&((y2<=y1&&y1<y2+50)||(y1<=y2&&y2<y1+50)))
    {
        x=(x1+x2)/2;
        y=(y1+y2)/2;
    }
    else
    {
      return "255,0,0,0,0 ";
    }        
    x=((long)XPT2046_XFAC_240x320*x)/10000+XPT2046_XOFFSET_240x320;
    y=((long)XPT2046_YFAC_240x320*y)/10000+XPT2046_YOFFSET_240x320;
             
    if(x > 240 || x <1 || y >320 || y < 1){
      x = 0;
      y = 0;
    }
    else {
     x = x;
     y = 320 - y;
    }
    
    pointRemap(x,y,240,320);
    //delay(10);
    s += String(1) + "," + String(x) + "," + String(y) + "," + String(10) + ","+ String(10) + " ";
    return s;


}
uint16_t DFRobot_Touch_XPT2046::readxy(uint8_t cmd){
    uint16_t i, j;
    uint16_t buf[5];
    uint16_t sum=0;
    uint16_t temp;

    for(i=0;i<5;i++)
    {
        uint16_t num=0;
        uint8_t buf1[2];
        memset(buf1, 0, sizeof(buf1));
        readCommand(cmd, buf1, 2);
        num = (buf1[0] << 8) | buf1[1];
        num >>= 3;
        buf[i]=num;
    }
    for(i=0;i<5-1; i++)//Sort in ascending order
    {
        for(j=i+1;j<5;j++)
        {
            if(buf[i]>buf[j])
            {
                temp=buf[i];
                buf[i]=buf[j];
                buf[j]=temp;
            }
        }
    }         

    for(i=1;i<5-1;i++) //Remove maximum and minimum values
    {
        sum+=buf[i];
    }
    temp=sum/(5-2*1);
    return temp;   
}

DFRobot_Touch_FT5436::DFRobot_Touch_FT5436(uint8_t addr , uint8_t rst, uint8_t irq)
  :DFRobot_Touch(&gdl_Dev_GTXXX_TOUCH_HW_IIC, addr, rst, irq){

}

DFRobot_Touch_FT5436::~DFRobot_Touch_FT5436(){

}
void DFRobot_Touch_FT5436::begin(uint32_t freq){
  initTouch();
  freq= freq;
}
String DFRobot_Touch_FT5436::scan(){
  uint8_t FT5216_Touch_Buf[43];
  uint8_t point = 0;
  String s = "";
   //I2C_read_reg(0,FT5216_Touch_Buf,33);
   readCommand(0,FT5216_Touch_Buf,33);
  if((FT5216_Touch_Buf[2] == 0x00) || (FT5216_Touch_Buf[2] == 0xFF)){
    point = FT5216_Touch_Buf[2];
  }else if(FT5216_Touch_Buf[2] > 5){
  }else{
    point = FT5216_Touch_Buf[2];
    if(point > 0x05){
      point = 0x05;
    }
    for(uint8_t i = 0; i < point; i++){
      uint16_t x = (uint16_t)(FT5216_Touch_Buf[3+6*i] & 0x0F)<<8 | (uint16_t) FT5216_Touch_Buf[4+6*i];
      uint16_t y = (uint16_t)(FT5216_Touch_Buf[5+6*i] & 0x0F) << 8 | (uint16_t) FT5216_Touch_Buf[6+6*i];
      //uint8_t event = FT5216_Touch_Buf[0x3+6*i] >> 6;
      uint8_t id = FT5216_Touch_Buf[5+6*i]>>4;
      if(x > 319){ x = 319; }
      if(y > 479){ y = 479; }
      x = 319 - x;
      y = 479 - y;
	  pointRemap(x,y,319,479);
      s += String(id) + "," + String(x) + "," + String(y) + "," + String(0) + ","+ String(0) + " ";
    }
	return s;
  }
  return "255,0,0,0,0 ";
}


DFRobot_Touch_FT3267::DFRobot_Touch_FT3267(uint8_t addr , uint8_t rst, uint8_t irq)
  :DFRobot_Touch(&gdl_Dev_GTXXX_TOUCH_HW_IIC, addr, rst, irq){

}

DFRobot_Touch_FT3267::~DFRobot_Touch_FT3267(){

}

void DFRobot_Touch_FT3267::begin(uint32_t freq){
  initTouch();
  freq= freq;
}

String DFRobot_Touch_FT3267::scan(){
  uint8_t FT3267_Touch_Buf[10];
  uint8_t point = 0;
  String s = "";
  readCommand(0,FT3267_Touch_Buf,10);
  if((FT3267_Touch_Buf[2] == 0x00) || (FT3267_Touch_Buf[2] == 0xFF)){
    point = FT3267_Touch_Buf[2];
  }else if(FT3267_Touch_Buf[2] > 5){
  }else{
    point = FT3267_Touch_Buf[2];
    if(point > 0x05){
      point = 0x05;
    }
    for(uint8_t i = 0; i < point; i++){
      uint16_t x = (uint16_t)(FT3267_Touch_Buf[3+6*i] & 0x0F)<<8 | (uint16_t) FT3267_Touch_Buf[4+6*i];
      uint16_t y = (uint16_t)(FT3267_Touch_Buf[5+6*i] & 0x0F) << 8 | (uint16_t) FT3267_Touch_Buf[6+6*i];
      uint8_t id = FT3267_Touch_Buf[5+6*i]>>4; 
      if(x > 240){ x = 240; }
      if(y > 204){ y = 204; }
      pointRemap(x,y,240,204);
      s += String(id) + "," + String(x) + "," + String(y) + "," + String(0) + ","+ String(0) + " ";
    }
	return s;
  }
  return "255,0,0,0,0 ";
}