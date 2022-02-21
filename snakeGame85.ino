#include <oled85.h>

#define buttonLeftRight A3
#define buttonUpDown A2

#define threshold1 600
#define threshold2 470

uint8_t snakeLen;
uint8_t dot[2];
int nextMove[2];
uint8_t snake[100];


// func declarations
void reset();
void play();
void moveSnake();
void placeDot();
void changeSnakeSize();
void changeNextMove();
bool checkButtonStateChange();
bool gameOver();

// initialize the oled display
OLED85 oled85;


void setup() 
{
  pinMode(buttonLeftRight, INPUT);
  pinMode(buttonUpDown, INPUT);
}

void loop() 
{
  play();
}

void play()
{
  uint8_t state = 0;
  unsigned long before = millis();
  delay(100);
  
  oled85.drawImage(LOAD_SCREEN);
  while (checkButtonStateChange())
  {
    // blink the 'press any button' without delay
    unsigned long now = millis();
    if (now - before >= 500)
    {
      state = !state;
      oled85.drawImage(LOAD_SCREEN, state);
      before = now;
    }
  }

  // resest the screen and begin the game
  reset();
  delay(300);
  placeDot();
  
  while (!gameOver())
  {
    changeNextMove();
    moveSnake();

    // check if you hit the dot
    if ((snake[0] == dot[0]) && (snake[1] == dot[1]))
    {
      snakeLen++;
      placeDot();
    }
    changeNextMove();
    delay(100);
  }

  // blink the screen to indicate game over
  delay(300);
  oled85.blinkScreen(3);
  delay(100);

  // display the user's score
  oled85.drawImage(SCORE);
  oled85.displayScore(snakeLen - 3);

  // wait for a button click and then start everything over
  while (checkButtonStateChange())
  ;
  play();
}

void reset()
{
  // remove all blocks from the screen and redraw a fresh new grid
  oled85.fillScreen(0x00);
  delay(50);
  oled85.drawGrid();

  // initial snake size is 3
  snakeLen = 3;
  nextMove[0] = 1;
  nextMove[1] = 0;
  snake[0] = 1, snake[1] = 1, snake[2] = 1, snake[3] = 2, snake[4] = 1, snake[5] = 3;
}

// check if any of the buttons was pressed
bool checkButtonStateChange()
{
  if ((analogRead(buttonLeftRight) < threshold2) && (analogRead(buttonUpDown) < threshold2))
  {
    return true;
  }
  return false;
}

// read input from buttons and map it to a certain direction
void changeNextMove()
{
  int val1 = analogRead(buttonLeftRight);
  int val2 = analogRead(buttonUpDown);

  ///button 1 = left
  if (val1 > threshold1  && (nextMove[0] != -1))  
  {
    nextMove[0] = 1;
    nextMove[1] = 0;
    delay(150);
  }
  
  //button 2 = right
  else if (val2 > threshold1  && (nextMove[1] != -1)) 
  {
    nextMove[0] = 0;
    nextMove[1] = 1;
    delay(150);
  }

  //button 3 = up
  else if (val1 > threshold2 && (nextMove[0] != 1))  
  {
    nextMove[0] = -1;
    nextMove[1] = 0;
    delay(150);
  }

  //button 4 = down
  else if (val2 > threshold2  && (nextMove[1] != 1))  
  {
    nextMove[0] = 0;
    nextMove[1] = -1;
    delay(150);
  }
}

// put the dot on a random place on the board
void placeDot()
{
  // select a new random place for the dot 
  srand(millis());
  uint8_t x = (rand() % 14) + 1;
  uint8_t y = (rand() % 6) + 1;
  for (int i = 0; i < snakeLen; ++i)
  {
    // check if the randomly selected dot is not a snake block
    if ((snake[2 * i] == x) && (snake[(2 * i) + 1] == y))
    {
      placeDot();
      return;
    }

  }
  dot[0] = x; dot[1] = y;
  
  // place the dot on the screen
  oled85.drawPixel(dot[0], dot[1]);
}

bool gameOver()
{
  for (uint8_t i = 1; i < snakeLen; ++i)
  {
    // check if you cross yourself
    if (  (snake[2 * i] == snake[0] && snake[(2 * i)  + 1] == snake[1] ) ) 
    {
      return true;
    }
  }
  
  // check for border positions (left || right)
  if ((snake[0] <= 0) || (snake[0] >= 15)) 
  {
    return true;
  }
  
  // check for border positions (top || bottom)
  if ((snake[1] <= 0) || (snake[1] >= 7))
  {
    return true;
  }
  
  return false;
}

void moveSnake()
{
  // temp variables to store the block to remove
  uint8_t x = snake[2 * snakeLen - 2], y =  snake[2 * snakeLen - 1];

  // move everything one block ahead
  for (int i = snakeLen - 1; i > 0; i--)                 
  {
    snake[2 * i] = snake[2 * (i - 1)];
    snake[(2 * i) + 1] = snake[(2 * (i - 1)) + 1];
  }

  // move the head(first) block in the new direction
  snake[0] += nextMove[0];                                      
  snake[1] += nextMove[1];
  delay(150);
  
  // make the movement visible on the board
  for (uint8_t i = 0; i < snakeLen; i++)
  {
    oled85.drawPixel(snake[2 * i], snake[(2 * i) + 1]);
  }
   oled85.removePixel(x, y);
}
