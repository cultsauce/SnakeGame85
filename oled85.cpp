#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include "oled85.h"


/**
 * class constructor - initialize the display with the required init sequence and clear the display's memory
 */
OLED85::OLED85()
{
  TinyWireM.begin();
  commandList(sizeof(init_commands_list), init_commands_list);
  fillScreen(0x00);
  sendCommand(INVERT_DISPLAY);
}

/**
 * send a list of commands to the display
 * @param len length of the command list
 * @param commands_list list of commands to send
 */
void OLED85::commandList(uint8_t len, const uint8_t commands_list [])
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  uint8_t comm;
  for (uint8_t i = 0; i < len; i++)
  {
    comm = pgm_read_byte(&commands_list[i]);
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

/**
 * send a single command to the display
 * @param command commands to be sent to display
 */
void OLED85::sendCommand(uint8_t command)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  TinyWireM.write(command);
  TinyWireM.endTransmission();
}

/**
 * send a data byte to the display
 * @param data data byte to write to the display's memory
 */
void OLED85::sendData(uint8_t data)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(DATABYTE);
  TinyWireM.write(data);
  TinyWireM.endTransmission();
}

/**
 * set column address to write to - in range < 0, 127 >
 * column address gets incremented after every write operation from start_addr to stop_addr
 * @param addr_start starting column
 * @param addr_stop last column
 */
void OLED85::setColAddr(uint8_t addr_start, uint8_t addr_stop)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  TinyWireM.write(COLUMN_ADDRESS);
  TinyWireM.write(addr_start);
  TinyWireM.write(addr_stop);
  TinyWireM.endTransmission();
}

/**
 * set page address to write to - in range < 0, 7 >
 * page address gets incremented after every write operation from start_addr to stop_addr
 * @param addr_start starting page
 * @param addr_stop last page
 */
void OLED85::setPageAddr(uint8_t addr_start, uint8_t addr_stop)
{
  TinyWireM.beginTransmission(I2C_ADDR);
  TinyWireM.write(COMMANDBYTE);
  TinyWireM.write(PAGE_ADDRESS);
  TinyWireM.write(addr_start);
  TinyWireM.write(addr_stop);
  TinyWireM.endTransmission();
}

/**
 * fill entire screen with the data byte specified
 * @param data data byte to fill the screen with
 */
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

/**
 * draw a 8x8 block on the specified index - in range < 0, 15 >
 * @param x horizontal index of the block to draw
 * @param y vertical index of the block to draw
 * @param offset_start horizontal offset from the start of the block
 * @param offset_stop horizontal offset from the end of the block
 * @param pattern data byte to fill the block with
 */
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

/**
 * remove a 8x8 block from the specified index - in range < 0, 15 >
 * @param x horizontal index of the block to draw
 * @param y vertical index of the block to draw
 */
void OLED85::removeBlock(uint8_t x, uint8_t y) 
{
  x *= NUM_PAGES;
  setPageAddr(y, y);
  setColAddr(x, x + NUM_PAGES);
  for (uint8_t i = 0; i < NUM_PAGES; i++)
  {
    sendData(0x00);
  }
  /* redraw the grid on the removed block (draw a black dot in the middle of the block) */
  setColAddr(x + 4, x + 4);
  sendData(0x10);
}

/**
 * draw a dot grid across the entire screen 
 */
void OLED85::drawGrid()
{
  for (uint8_t i = 0; i < NUM_PAGES; i++){
    for (uint8_t j = 0; j < WIDTH/ NUM_PAGES; j++) {
      drawBlock(j, i, 4, 4, 0x10);
    }
  }
}

/**
 * create blinking effect by inverting and reverting colors of the display 
 * @param times number of times to blink the display
 */
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

/**
 * render a 128 x 64 px bitmap on the screen
 * @param img bitmap data
 * @param blank_half optional, used for creating the blinking effect of half of the screen
 */
void OLED85::drawImage(const uint8_t img[], uint8_t blank_half = 0)
{
  uint32_t counter = 0;
  setPageAddr(0, NUM_PAGES - 1);
  setColAddr(0, WIDTH - 1);
  for (uint8_t i = 0; i < NUM_PAGES; i++)
  {
    TinyWireM.beginTransmission(I2C_ADDR);
    TinyWireM.write(DATABYTE);
    for (uint8_t j = 0; j < WIDTH ; j++)
    {
      uint8_t data = pgm_read_byte(&img[counter++]);
      if (counter < 265 && blank_half) data = 0x00;
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

/**
 * display the user's score in a 7-segment format
 * @param score the user's score to display
 */
void OLED85::displayScore(uint8_t score)
{
  static const uint8_t full[][2] = {{10, 1}, {11, 1}, {12, 1}, {10, 2}, {12, 2}, {10, 3}, {11, 3}, {12, 3}, {10, 4}, {12, 4}, {10, 5}, {11, 5}, {12, 5}};
  static const uint8_t numbers[][NUM_SEGMENTS] = {
    {1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1}, /* 0 */
    {0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1}, /* 1 */
    {1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1}, /* 2 */
    {1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1}, /* 3 */
    {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1}, /* 4 */
    {1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1}, /* 5 */
    {1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}, /* 6 */
    {1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1}, /* 7 */
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, /* 8 */
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1}  /* 9 */
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
