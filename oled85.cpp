#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include "oled85.h"


// class constructor - initialize display registers and clean all pixels
OLED85::OLED85()
{
  TinyWireM.begin();
  commandList(sizeof(init_commands_list), init_commands_list);
  fillScreen(0x00);
  sendCommand(INVERT_DISPLAY);
}

void OLED85::commandList(uint8_t len, const uint8_t init_commands_list [])
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  uint8_t comm;
  for (uint8_t i = 0; i < len; i++)
  {
    comm = pgm_read_byte(&init_commands_list[i]);
    if (TinyWireM.write(comm) == 0)
    {
      TinyWireM.endTransmission();
      TinyWireM.beginTransmission(I2C_ADDR);
      TinyWireM.write(COMMANDBYTE);
      TinyWireM.write(comm);
    }
  }
  TinyWireM.endTransmission();
}

void OLED85::sendCommand(uint8_t command)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  TinyWireM.write(command);
  TinyWireM.endTransmission();
}

void OLED85::sendData(uint8_t data)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(DATABYTE);
  TinyWireM.write(data);
  TinyWireM.endTransmission();
}

void OLED85::setColAddr(uint8_t addr_start, uint8_t addr_stop)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  TinyWireM.write(COLUMN_ADDRESS);
  TinyWireM.write(addr_start);
  TinyWireM.write(addr_stop);
  TinyWireM.endTransmission();
}

void OLED85::setPageAddr(uint8_t addr_start, uint8_t addr_stop)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  TinyWireM.write(PAGE_ADDRESS);
  TinyWireM.write(addr_start);
  TinyWireM.write(addr_stop);
  TinyWireM.endTransmission();
}

void OLED85::fillScreen(uint8_t data)
{
  setPageAddr(0, NUM_PAGES - 1);
  setColAddr(0, WIDTH - 1);
  for (uint8_t i = 0; i < NUM_PAGES; i++) {
    TinyWireM.beginTransmission(I2C_ADDR);
    TinyWireM.write(DATABYTE);
    for (uint8_t j = 0; j < WIDTH ; j++)
    {
      if (TinyWireM.write(data) == 0)
      {
        TinyWireM.endTransmission();
        TinyWireM.beginTransmission(I2C_ADDR);
        TinyWireM.write(DATABYTE);
        TinyWireM.write(data);
      }
    }
    TinyWireM.endTransmission();
  }
}

/// x in range <0, 16), y in range <0, 8) - fills a 8x8 pixel block on the specified index
void OLED85::drawBlock(uint8_t x, uint8_t y, uint8_t offset_start, uint8_t offset_stop, uint8_t pattern)
{
  x *= NUM_PAGES;
  setPageAddr(y, y);
  setColAddr(x + offset_start, x + 8 - offset_stop);
  for (uint8_t j = 0; j < 8; j++)
  {
    sendData(pattern);
  }
}

// x in range <0, 16), y in range <0, 8) - removes a 8x8 pixel block from the specified index
void OLED85::removeBlock(uint8_t x, uint8_t y)
{
  x *= NUM_PAGES;
  setPageAddr(y, y);
  setColAddr(x, x + NUM_PAGES);
  for (uint8_t i = 0; i < NUM_PAGES; i++)
  {
    sendData(0x00);
  }
  setColAddr(x + 4, x + 4);

  // redraw the grid on the removed block (draw a black dot in the middle of the block)
  sendData(0x10);
}

// draw background grid with dots
void OLED85::drawGrid()
{
  //setColAddr(0, WIDTH - 1);
  //setPageAddr(0, NUM_PAGES - 1);
  for (uint8_t i = 0; i < NUM_PAGES; i++){
    for (uint8_t j = 0; j < WIDTH/ NUM_PAGES; j++) {
      drawBlock(j, i, 4, 4, 0x10);
    }
  }
}

// create blinking effect by inverting and reverting colors of the display (n times)
void OLED85::blinkScreen(uint8_t times)
{
  for (uint8_t i = 0; i < times; i++)
  {
    sendCommand(NORMAL_DISPLAY);
    delay(200);
    sendCommand(INVERT_DISPLAY);
    delay(200);
  }
}

// render a bitmap
void OLED85::drawImage(const uint8_t img[], uint8_t blank_half = 0)
{
  uint32_t counter = 0;
  setPageAddr(0, NUM_PAGES - 1);
  setColAddr(0, WIDTH - 1);
  for (uint8_t i = 0; i < NUM_PAGES; i++)
  {
    //setColAddr(0, WIDTH - 1);
    //setPageAddr(i, i);
    TinyWireM.beginTransmission(I2C_ADDR);
    TinyWireM.write(DATABYTE);
    for (uint8_t j = 0; j < WIDTH ; j++)
    {
      uint8_t h = pgm_read_byte(&img[counter++]);
      if (counter < 265 && blank_half) h = 0x00;
      if (TinyWireM.write(h) == 0)
      {
        TinyWireM.endTransmission();
        TinyWireM.beginTransmission(I2C_ADDR);
        TinyWireM.write(DATABYTE);
        TinyWireM.write(h);
      }
    }
    TinyWireM.endTransmission();
  }

}

void OLED85::displayScore(uint8_t score)
{
  uint8_t full[][2] = {{10, 1}, {11, 1}, {12, 1}, {10, 2}, {12, 2}, {10, 3}, {11, 3}, {12, 3}, {10, 4}, {12, 4}, {10, 5}, {11, 5}, {12, 5}};
  uint8_t numbers[][NUM_SEGMENTS] = {
    {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1}, // 0
    {0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1}, // 1
    {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1}, // 2
    {1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1}, // 3
    {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1}, // 4
    {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1}, // 5
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1}, // 7
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1} // 9
  };

  for (uint8_t i = 0; i < NUM_SEGMENTS; i++)
  {
    if (numbers[(score / 10) % 10][i])
    {
      drawBlock(16 - full[i][0] + 1, NUM_PAGES - full[i][1] - 2, 0, 0, 0xff);
    }
    if (numbers[score % 10][i])
    {
      drawBlock(16 - full[i][0] + 1 - 4, NUM_PAGES - full[i][1] - 2, 0, 0, 0xff);
    }

  }
}
