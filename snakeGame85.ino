#include "oled85.h"

#define LEFT_RIGHT A3
#define UP_DOWN A2
#define BUZZER 1
#define THRESHOLD1 600
#define THRESHOLD2 470

uint8_t snakeLen;
uint8_t dot[2];
int nextMove[2];
uint8_t snake[100];

/* initialize the oled display */
OLED85 oled85;


void setup() 
{
  pinMode(LEFT_RIGHT, INPUT);
  pinMode(UP_DOWN, INPUT);
  pinMode(BUZZER, OUTPUT);
}

/* emulates the arduino tone() function */
void tinyTune(uint8_t down, uint8_t up, uint8_t times){
  for (int i = 0; i < times; i++){
    digitalWrite(BUZZER, HIGH);
    delay(up);
    digitalWrite(BUZZER, LOW);
    delay(down);
  }
}

/* clean up for a new game */
void reset()
{
  /* remove all blocks from the screen and redraw a fresh new grid */
  oled85.fillScreen(0x00);
  delay(50);
  oled85.drawGrid();

  snakeLen = 3; /* initial snake size is 3 */
  nextMove[0] = 1; /* set og movement to left */
  nextMove[1] = 0;
  snake[0] = 1, snake[1] = 1, snake[2] = 1, snake[3] = 2, snake[4] = 1, snake[5] = 3; /* set initial snake position */
}

/* check if any of the buttons was pressed */
bool checkButtonStateChange()
{
  if ((analogRead(LEFT_RIGHT) < THRESHOLD2) && (analogRead(UP_DOWN) < THRESHOLD2))
  {
    return true;
  }
  return false;
}

/* evaluate state of the current game*/
bool gameOver()
{
  for (uint8_t i = 1; i < snakeLen; ++i)
  {
    /* check if you crossed yourself */
    if (  (snake[2 * i] == snake[0] && snake[(2 * i)  + 1] == snake[1] ) ) 
    {
      return true;
    }
  }
  
  /* check for border positions (left || right || top || bottom) */
  if ((snake[0] <= 0) || (snake[0] >= 15) || (snake[1] <= 0) || (snake[1] >= 7)) 
  {
    return true;
  }
  
  return false;
}

/* put the dot on a random place on the board */
void placeDot()
{
  /* select a new random place for the dot */
  srand(millis());
  uint8_t x = (rand() % 14) + 1;
  uint8_t y = (rand() % 6) + 1;
  for (int i = 0; i < snakeLen; ++i)
  {
    /* check if the randomly selected dot is not a snake block */
    if ((snake[2 * i] == x) && (snake[(2 * i) + 1] == y))
    {
      placeDot();
      return;
    }

  }
  dot[0] = x; dot[1] = y;
  
  /* place the dot on the screen */
  oled85.drawBlock(dot[0], dot[1], 1, 1, 0x7e);
}

/* snake movement animation */
void moveSnake()
{
  /* temp variables to store the block to remove */
  uint8_t x = snake[2 * snakeLen - 2], y =  snake[2 * snakeLen - 1];

  /* move everything one block ahead */
  for (int i = snakeLen - 1; i > 0; i--)                 
  {
    snake[2 * i] = snake[2 * (i - 1)];
    snake[(2 * i) + 1] = snake[(2 * (i - 1)) + 1];
  }

  /* move the head(first) block in the new direction */
  snake[0] += nextMove[0];                                      
  snake[1] += nextMove[1];
  delay(150);
  
  /* make the movement visible on the board */
  for (uint8_t i = 0; i < snakeLen; i++)
  {
    oled85.drawBlock(snake[2 * i], snake[(2 * i) + 1], 1, 1, 0x7e);
  }
   oled85.removeBlock(x, y);
}

/* read input from buttons and map it to a certain direction */
void changeNextMove()
{
  int val1 = analogRead(LEFT_RIGHT);
  int val2 = analogRead(UP_DOWN);

  /* button 1 = left */
  if (val1 > THRESHOLD1  && (nextMove[0] != -1))  
  {
    nextMove[0] = 1;
    nextMove[1] = 0;
    delay(150);
  }
  
  /* button 2 = right */
  else if (val2 > THRESHOLD1  && (nextMove[1] != -1)) 
  {
    nextMove[0] = 0;
    nextMove[1] = 1;
    delay(150);
  }

  /* button 3 = up */
  else if (val1 > THRESHOLD2 && (nextMove[0] != 1))  
  {
    nextMove[0] = -1;
    nextMove[1] = 0;
    delay(150);
  }

  /* button 4 = down */
  else if (val2 > THRESHOLD2  && (nextMove[1] != 1))  
  {
    nextMove[0] = 0;
    nextMove[1] = -1;
    delay(150);
  }
}

/* flow of the game */
void play()
{
  uint8_t state = 0; /* initial state - used for half - display blinking animation*/
  unsigned long before = millis();
  delay(100);

  /* display the loading screen and wait for user to press any button */
  oled85.drawImage(LOAD_SCREEN);
  while (checkButtonStateChange())
  {
    /* blink the 'press any button' without delay */
    unsigned long now = millis();
    if (now - before >= 500)
    {
      state = !state;
      oled85.drawImage(LOAD_SCREEN, state);
      before = now;
    }
  }

  /* play game begin sound */
  for (int i = 4; i >= 1; i--){
    tinyTune(i, 1, 25);
  }

  /* reset the screen and begin the game */
  reset();
  delay(300);
  placeDot();

  /* keep repeating until gameover */
  while (!gameOver())
  {
    changeNextMove();
    moveSnake();

    /* check if you catch the dot */
    if ((snake[0] == dot[0]) && (snake[1] == dot[1]))
    {
      tinyTune(2, 2, 10);
      snakeLen++;
      placeDot();
    }
    changeNextMove();
    delay(100);
  }

  /* blink the screen to indicate game over and play a sound on the buzzer */
  delay(300);
  for (int i = 5; i < 9; i++){
    tinyTune(i, 1, 25); /* game over sound */
  }
  oled85.blinkScreen(3);
  delay(100);

  /* display the user's score */
  oled85.drawImage(SCORE);
  oled85.displayScore(snakeLen - 3);

  /* wait for a button click and then start a new game */
  while (checkButtonStateChange())
  ;
  play();
}



void loop() 
{
  play();
}
