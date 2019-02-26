#include <SPI.h>
#include "LCD_Functions.h"

const byte menuDelay = 20;
const byte gameDelay = 4;

const byte buzzerPin = 10;
const byte upButtonPin = 2;
const byte downButtonPin = 3;
const byte leftButtonPin = 4;
const byte rightButtonPin = 8;

// display mode
// 0 - intro
// 1 - menu
// 2 - game
// 3 - end
// 4 - about
int mode = 0;
int time = 0;

boolean soundOn = true;

// button states
boolean upPressed = false;
boolean downPressed = false;
boolean leftPressed = false;
boolean rightPressed = false;

boolean isUpHigh = false;
boolean isDownHigh = false;
boolean isLeftHigh = false;
boolean isRightHigh = false;

void setup() {

  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);
  pinMode(leftButtonPin, INPUT);
  pinMode(rightButtonPin, INPUT);

  lcdBegin();
  setContrast(58);
  
  clearDisplay(WHITE);
  updateDisplay();
}

void loop() {
  
  randomSeed(analogRead(0));
  
  snake_buttons();
  
  switch (mode) {
  
    case 0: snake_intro(); break;
    case 1: snake_menu(); break;
    case 2: snake_game(); break;
    case 3: snake_end(); break;
    case 4: snake_about(); break;
  }
  
  updateDisplay();
  
  int delayInterval = (mode == 0 || mode == 1) ? menuDelay : gameDelay;
  delay(delayInterval);
  
  time++;
}

void snake_buttons() {

  int upButtonState = digitalRead(upButtonPin);
  if (upButtonState == HIGH) { isUpHigh = true; }
  else if (isUpHigh && upButtonState == LOW) { 
    isUpHigh = false; 
    upPressed = true; 
  }
  
  int downButtonState = digitalRead(downButtonPin);
  if (downButtonState == HIGH) { isDownHigh = true; }
  else if (isDownHigh && downButtonState == LOW) { 
    isDownHigh = false; 
    downPressed = true;
  }
  
  int leftButtonState = digitalRead(leftButtonPin);
  if (leftButtonState == HIGH) { isLeftHigh = true; }
  else if (isLeftHigh && leftButtonState == LOW) { 
    isLeftHigh = false; 
    leftPressed = true; 
  }
  
  int rightButtonState = digitalRead(rightButtonPin);
  if (rightButtonState == HIGH) { isRightHigh = true; }
  else if (isRightHigh && rightButtonState == LOW) { 
    isRightHigh = false; 
    rightPressed = true; 
  }
}

/********************************
 * INTRO
 */
int introTime = LCD_HEIGHT;
int introChar = 0;

void snake_intro() {

  switch (introChar) {

    case 0:
      setChar('S', 24, introTime, BLACK);
      if (introTime <= 14) {
        introTime = LCD_HEIGHT;
        introChar++;
      }
      break;

    case 1:
      setChar('n', 32, introTime, BLACK);
      if (introTime <= 16) {
        introTime = LCD_HEIGHT;
        introChar++;
      }
      break;

    case 2:
      setChar('a', 40, introTime, BLACK);
      if (introTime <= 18) {
        introTime = LCD_HEIGHT;
        introChar++;
      }
      break;

    case 3:
      setChar('k', 48, introTime, BLACK);
      if (introTime <= 20) {
        introTime = LCD_HEIGHT;
        introChar++;
      }
      break;

    case 4:
      setChar('e', 56, introTime, BLACK);
      if (introTime <= 22) {
        introTime = 50;
        introChar = -1;
      }
      break;
  }
  
  introTime -= 2;
  
  updateDisplay();
  
  if (introChar == -1)
    introTime -= 1;

  // after one second go to menu
  if (introTime < 0 && time >= 10)
    snake_goto_mode(1);
}

/********************************
 * GAME
 */
const int snakeSize = 1;
const int snakeWidth = 20;
const int rightSpace = 18;

int foodPosition = -1;
int score = 0;

int vectorX = +1;
int vectorY = 0;

int headX = -1;
int headY = -1;

boolean foodEaten = false;
boolean gameEnd = false;

void snake_game() {

  snake_border();
    
  if (upPressed && vectorY == 0) { vectorX = 0; vectorY = -1; }
  else if (downPressed && vectorY == 0) { vectorX = 0; vectorY = +1; }
  else if (leftPressed && vectorX == 0) { vectorX = -1; vectorY = 0; }
  else if (rightPressed && vectorX == 0) { vectorX = +1; vectorY = 0; }
  
  if (time % gameDelay == 0) {
  
    if (gameEnd) {
      
      snake_melody_end();
      snake_goto_mode(3);   
      snake_reset_buttons();   
      return;
    }
  
    // play melody
    if (upPressed || downPressed || leftPressed || rightPressed)
      snake_melody_vector();
    //else
    //  snake_melody_move();

    updateDisplay();
    
    // initialize snake
    if (headX == -1 || headY == -1) {
    
      // head start positions
      headX = 24;
      headY = 40;
      
      for (int i = 0; i < snakeWidth * snakeSize; i++) {
        
        setPixel(headX - i, headY);
        
        if (snakeSize == 2)
          setPixel(headX - i, headY + 1);
      }
      
      snake_game_info();
    }
    
    if (foodPosition == -1)
      snake_game_add_food();
    
    setPixel(headX, headY, WHITE);
    
    // check food
    int foodY = foodPosition / LCD_WIDTH;
    int foodX = foodPosition - foodY * LCD_WIDTH;
    if (headX == foodX && headY == foodY) {
      
      // food eaten
      foodPosition = -1;
      score++;
      
      foodEaten = true;
      
      snake_melody_food();
      snake_game_info();
    }
    
    // check game end and move
    snake_game_check();
    snake_game_move(headX, headY);
    
    if (vectorX != 0)
      headX += vectorX * snakeSize;
    else if (vectorY != 0)
      headY += vectorY * snakeSize;
      
    setPixel(headX, headY, BLACK);
    
    snake_reset_buttons();
  }
}

void snake_game_info() {

  char charBuf[3];
  String(score).toCharArray(charBuf, 3);

  int xPos = 70;
  
  if (score < 10)
    xPos = 76;
    
  setStr(charBuf, xPos, 4, BLACK);
}

void snake_game_check() {

  int x, y;

  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
    
      if (i != vectorX || j != vectorY)
        continue;
      
      x = headX + i;
      y = headY + j;
      
      if (snake_game_board_check(x, y)) {
        
        gameEnd = true;
        return;
      }
    }
}

void snake_game_move(int sx, int sy) {

  int tx = sx;
  int ty = sy;
  int x, y, foodX, foodY;
  
  for (int j = -1; j <= 1; j++)
    for (int i = -1; i <= 1; i++) {
    
      if (i == j || (i != 0 && j != 0))
        continue;
        
      x = tx + i;
      y = ty + j;
      
      if (x == sx && y == sy)
        continue;
      
      if (snake_game_board_check(x, y))
        continue;
      
      if (getPixel(x, y) != 1)
        continue;
      
      // skip food
      foodY = foodPosition / LCD_WIDTH;
      foodX = foodPosition - foodY * LCD_WIDTH;
      
      if (foodX == x && foodY == y)
        continue;
        
      setPixel(x, y, WHITE);
      snake_game_move(x, y);
      setPixel(x + -i, y + -j, BLACK);
      
      if (foodEaten) {
      
        setPixel(x, y, BLACK);
        foodEaten = false;
      }
      
      return;
    }
}

void snake_game_add_food() {

  int x, y, temp;

  while (true) {
  
    temp = (int)random(85, ((LCD_WIDTH - rightSpace) - 3) * (LCD_HEIGHT - 23) + 5);
    y = temp / LCD_WIDTH;
    x = temp - y * LCD_WIDTH;
    
    if (snake_game_board_check(x, y, 2))
      continue;
      
    if (getPixel(x, y) == 1)
      continue;
    
    foodPosition = temp;
    setPixel(x, y, BLACK);
    
    break;
  }
  
  setPixel(x, y);
}

void snake_game_reset() {

  gameEnd = false;
  vectorX = +1;
  vectorY = score = 0;
  foodPosition = headX = headY = -1;
}

boolean snake_game_board_check(int x, int y) {
  return snake_game_board_check(x, y, 1);
}

boolean snake_game_board_check(int x, int y, int offset) {

  return 
    x <= offset 
    || y <= offset 
    || x >= (LCD_WIDTH - rightSpace) - (offset + 1) 
    || y >= LCD_HEIGHT - (offset + 1);
}

/********************************
* MENU
*/
// menu options
// 0 - play
// 1 - sound on / off
// 2 - about
int menuOption = 0;

void snake_menu() {
  
  snake_rounded_border();
  
  if (headX != -1) { 
  
    // reset game
    snake_game_reset();
  }
  
  // 14 bytes
  String soundState = soundOn ? "on" : "off";
  String sound = "Sound " + soundState;
  
  char charBuf[10];
  sound.toCharArray(charBuf, 10);
  
  setStr("Play", 12, 10, BLACK);
  setStr(charBuf, 12, 20, BLACK);
  setStr("About", 12, 30, BLACK);
  
  int arrowPosition = 10 * (menuOption + 1);
  setChar('*', 4, arrowPosition, BLACK);
  
  if (upPressed || downPressed)
    setChar(' ', 4, arrowPosition, BLACK);
  
  snake_menu_action();
}

void snake_menu_action() {
      
  // sound on off
  if (rightPressed) {
    
    snake_melody_sound();
        
    switch (menuOption) {
    
      case 0: snake_goto_mode(2); break;  // start game
      
      case 1:
        soundOn = !soundOn;
        setChar(' ', 60, 20, BLACK); // clean 'f' letter
        break;
      
      case 2: snake_goto_mode(4); break; // show about
    }
  
    rightPressed = false;
  }
  
  if (upPressed) {
  
    snake_melody_menu();
    menuOption = menuOption - 1;
    
    if (menuOption == -1)
      menuOption = 2;
  }
  
  if (downPressed) {
  
    snake_melody_menu();
    menuOption = menuOption + 1;
    
    if (menuOption == 3)
      menuOption = 0;
  }
  
  snake_reset_buttons();
}

/********************************
 * END
 */
void snake_end() {
  
  char charBuf[3];
  String(score).toCharArray(charBuf, 3);

  snake_rounded_border();
  setStr("Your score: ", 10, 17, BLACK);
  setStr(charBuf, 10, 27, BLACK);
  snake_any_key_menu();
}

/********************************
 * ABOUT
 */
void snake_about() {

  snake_rounded_border();
  
  setStr("2014", 56, 36, BLACK);
  
  snake_any_key_menu();
}

/********************************
 * COMMON
 */
void snake_reset_buttons() {

  upPressed = false;
  downPressed = false;
  leftPressed = false;
  rightPressed = false;
}

void snake_any_key_menu() {

  // go to menu by pressing any key
  if (upPressed || downPressed || rightPressed || leftPressed)
    snake_goto_mode(1);
  
  snake_reset_buttons();
}

void snake_goto_mode(int m) {

  mode = m;
  
  snake_reset_buttons();
  
  clearDisplay(WHITE);
}

void snake_border() {

  for (int i = 1; i < (LCD_WIDTH - rightSpace) - 1; i++) {
    
    snake_border_part(i, 1);
    
    if (i % 2 == 0 || i % 3 == 0)
      snake_border_part(i, 0);
  }
}

void snake_border_part(int i, int offset) {

  setPixel(i, offset);
  setPixel(i, LCD_HEIGHT - (1 + offset));
  
  if (i < LCD_HEIGHT - offset) { 
  
    setPixel(offset, i);
    setPixel((LCD_WIDTH - rightSpace) - (1 + offset), i);
  }
}

#define CORNER 5
int ltCorner[CORNER] = { 88, 87, 170, 253, 337 };
int lbCorner[CORNER] = { 3613, 3697, 3782, 3867, 3868 };
int rbCorner[CORNER] = { 3943, 3944, 3861, 3778, 3694 };
int rtCorner[CORNER] = { 163, 164, 249, 334, 418 };

void snake_rounded_border() {

  // corners
  setPixels(ltCorner, CORNER, BLACK); // Left top
  setPixels(lbCorner, CORNER, BLACK); // Left bottom
  setPixels(rbCorner, CORNER, BLACK); // Right bottom
  setPixels(rtCorner, CORNER, BLACK); // Right top
  
  for (int i = CORNER; i < LCD_WIDTH - CORNER; i++) {
  
    setPixel(i, 0);
    setPixel(i, LCD_HEIGHT - 1);
    
    if (i < LCD_HEIGHT - CORNER) {
    
      setPixel(0, i);
      setPixel(LCD_WIDTH - 1, i);
    }
  }
}

/********************************
 * MELODY
 * http://arduino.cc/en/Tutorial/Tone
 */
void snake_melody_menu() {

  int melody[2] = { 2093, 880 };
  int noteDurations[2] = { 16, 8 };

  snake_melody(melody, noteDurations, 2);
}

void snake_melody_sound() {

  int melody[2] = { 2093, 494 };
  int noteDurations[2] = { 16, 8 };

  snake_melody(melody, noteDurations, 2);
}

void snake_melody_food() {

  int melody[2] = { 78, 93 };
  int noteDurations[2] = { 32, 16 };

  snake_melody(melody, noteDurations, 2);
}

void snake_melody_move() {

  int melody[1] = { 1976 };
  int noteDurations[1] = { 64 };

  snake_melody(melody, noteDurations, 1);
}

void snake_melody_vector() {

  int melody[1] = { 988 };
  int noteDurations[1] = { 64 };

  snake_melody(melody, noteDurations, 1);
}

void snake_melody_end() {

  int melody[4] = { 988, 349, 196, 98 };
  int noteDurations[4] = { 8, 16, 32, 64 };

  snake_melody(melody, noteDurations, 4);
}

void snake_melody(int* melody, int* durations, byte count) {

  if (!soundOn)
    return;

  for (int thisNote = 0; thisNote < count; thisNote++) {

    int noteDuration = 1000 / durations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    
    delay(pauseBetweenNotes);
    
    noTone(buzzerPin);
  }
}