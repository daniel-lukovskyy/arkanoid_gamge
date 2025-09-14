#define PIN_MAX7219_LOAD              25
#define PIN_MAX7219_DATA              26
#define PIN_MAX7219_CLK               27
#define PIN_JOYSTICK                  32

#define BOARD_WIDTH                   8
#define BOARD_HEIGHT                  8
#define BEEP_DURATION_MS              10
#define BEEP_FERQ                     440
#define DISPLAY_INTENSITY             3

#define MAX7219_ADDRESS_DECODE        0x09
#define MAX7219_ADDRESS_INTENSITY     0x0A
#define MAX7219_ADDRESS_SCAN_LIMIT    0x0B
#define MAX7219_ADDRESS_SHUTDOWN      0x0C
#define MAX7219_ADDRESS_DISPLAY_TEST  0x0F

#define MOVE_DURATION_MS              200
#define NUMBER_OF_LIVES               3

#define paddel_size                   3

bool buffer[BOARD_HEIGHT][BOARD_WIDTH];
bool blocks[BOARD_HEIGHT][BOARD_WIDTH];
uint8_t lives;
uint8_t score;
int8_t pos_player;

struct {
  int8_t pos_x;
  int8_t pos_y;
  int8_t step_x;
  int8_t step_y;
} ball;

void send_data(byte address, byte data){
  // this function send a single packet to the MAX7219. see it's datasheet for more info.
  digitalWrite(PIN_MAX7219_LOAD, LOW);
  shiftOut(PIN_MAX7219_DATA, PIN_MAX7219_CLK, MSBFIRST, address);
  shiftOut(PIN_MAX7219_DATA, PIN_MAX7219_CLK, MSBFIRST, data);
  digitalWrite(PIN_MAX7219_LOAD, HIGH);
}

int8_t read_joystick(){
  // TODO: return 0 if no joystick input, -1 to move player left, and 1 to move player right.
  if (analogRead(PIN_JOYSTICK) > 612) {return 1;}
  else if (analogRead(PIN_JOYSTICK) < 412) {return -1;}
  return 0;
}

void clear_buffer(){
  // TODO: clear buffer - set everything to 0.
  for (int y = 0; y < BOARD_HEIGHT; y++){
    for (int x = 0; x < BOARD_WIDTH; x++){
      buffer[y][x] = false;
    }
  }
}

bool is_blocks_empty(){
  // TODO: return true if there are no more blocks left
  for (int y = 0; y < BOARD_HEIGHT; y++){
    for (int x = 0; x < BOARD_WIDTH; x++){
      if (blocks[y][x]) {return false;} 
    }
  }
  return true;
}

void display_buffer(){
  // this function sets the display to be like the global buffer variable.
  for(int y = 0; y < BOARD_HEIGHT ; y++){
    uint8_t data = 0;
    for(int x = 0; x < BOARD_WIDTH - 1; x++){
      if(buffer[y][x]) data |= 1 << (6-x);
    }
    if(buffer[y][BOARD_WIDTH - 1]) data |= 1 << 7;
    send_data(BOARD_HEIGHT - y, data);
  }
}

void display_game(){
  // TODO:
  //   - clear buffer
  clear_buffer();
  //   - draw blocks to buffer
  for (int y = 0; y < BOARD_HEIGHT; y++){
    for(int x = 0; x < BOARD_WIDTH; x++){
      buffer[y][x] |= blocks[y][x];
    }
  }
  //   - draw ball to buffer
  buffer[ball.pos_y][ball.pos_x] |= true;
  //   - draw player to buffer
    for (int x = 0; x < paddel_size; x++){
      if ((pos_player + x) >= 0 && (pos_player + x) < BOARD_WIDTH){
        buffer[0][pos_player + x] = true;
      }
  }
  display_buffer();
}


void display_score(){
  // TODO: display score
}

void life_lost(){
  // TODO: do a small animation to indicate a life was lost.
  // for that you can write directly to buffer (buffer[y][x] = 1), and use display_buffer() to show it.
  clear_buffer();
  for (int y = 0; y < BOARD_HEIGHT; y++){
    for(int x = 0; x < BOARD_WIDTH; x++){
      buffer[y][x] = 1;
      delay(5);
    }
  }
  display_buffer();
}

void game_won(){
  // TODO: do a small animation to indicate victory.
  // for that you can write directly to buffer (buffer[y][x] = 1), and use display_buffer() to show it.
    for (int y = 0; y < BOARD_HEIGHT / 2; y++){
    for(int x = 0; x < BOARD_WIDTH; x++){
      buffer[y][x] = 1;
      delay(5);
    }
  }
  display_buffer();
}

void move_player(){
  // TODO: read value from joystick and move player accordingly. make sure the player doesn't go off board.
  int direction = read_joystick();
  if (direction != 0){
    pos_player += direction;
    if (pos_player < 0) { pos_player = 0; }
    if (pos_player > BOARD_WIDTH - paddel_size) { pos_player = BOARD_WIDTH - paddel_size;}
  }
}

bool move_ball(){
  // TODO: move the ball one step.
  // if the ball hits a wall - flip it direction.
  // calculate ball next cell
  // if the next cell is in the last row and it's colliding with the player, set it's x direction accordingly.
  // if the next cell has a block in it - remove that block, do a short beep, update score, and set direction of ball back to where it came from.
  //                               else - move ball to next cell. to prevent difficult to debug bugs - make sure the ball isn't out of the board.
  // if the ball reached the last row (meaning it didn't collide with the player) - return false - life lost.
  //                                                                         else - return true - keep moving.

  if (ball.pos_x == 0 || ball.pos_x == BOARD_WIDTH - 1){
    ball.step_x *= -1;
  }
  if (ball.pos_y == 0 || ball.pos_y == BOARD_HEIGHT - 1){
    ball.step_y *= -1;
  }

  int new_x = ball.pos_x + ball.step_x;
  int new_y = ball.pos_y + ball.step_y;

  if (new_y == 0){
    if (abs(new_x - pos_player) <= 1){
      ball.step_y *= -1;
      ball.step_x = new_x - pos_player;
    }
  }

  if (blocks[new_y][new_x]){
    blocks[new_y][new_x] = 0;
    score+= 10;
    ball.step_x *= -1;
    ball.step_y *= -1;
  }else{
    ball.pos_x += ball.step_x;
    ball.pos_y += ball.step_y;
    if (ball.pos_x < 0){
      ball.pos_x = 0;
    }
    if (ball.pos_x > BOARD_WIDTH - 1){
      ball.pos_x = BOARD_WIDTH - 1;
    }
    if (ball.pos_y < 0){
      ball.pos_y = 0;
    }
    if (ball.pos_y > BOARD_HEIGHT - 1){
      ball.pos_y = BOARD_HEIGHT - 1;
    }
  }
  if (ball.pos_y == 0) {
    return false;
  }else{return true;}
}

void test(){
  clear_buffer();
  display_buffer();
  delay(1000);
  
  for(int y = 0; y < BOARD_HEIGHT; y++){
    for(int x = 0; x < BOARD_WIDTH; x++){
      delay(500);
      buffer[y][x] = 1;
      display_buffer();
    }
  }
  unsigned long int start = millis();
  while(millis() - start < 5000){
    tone(PIN_BUZZER, map(analogRead(PIN_JOYSTICK), 0, 1023, 220, 880));
  }
  noTone(PIN_BUZZER);
}


void setup(){
  // set pin modes:
  pinMode(PIN_JOYSTICK, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  
  digitalWrite(PIN_MAX7219_LOAD, HIGH);
  digitalWrite(PIN_MAX7219_CLK, LOW);

  pinMode(PIN_MAX7219_LOAD, OUTPUT);
  pinMode(PIN_MAX7219_DATA, OUTPUT);
  pinMode(PIN_MAX7219_CLK, OUTPUT);

  // initialize max7219:
  send_data(MAX7219_ADDRESS_SCAN_LIMIT, 7);
  send_data(MAX7219_ADDRESS_DECODE, 0);
  send_data(MAX7219_ADDRESS_INTENSITY, DISPLAY_INTENSITY);
  send_data(MAX7219_ADDRESS_SHUTDOWN, 1);
  send_data(MAX7219_ADDRESS_DISPLAY_TEST, 0);

  test();
}

void loop(){
  // TODO: start a new game.
  // initialize lives, score, and blocks. this is done for every new game.
  lives = NUMBER_OF_LIVES;
  score = 0;
  bool win = false;
  for (int y = BOARD_HEIGHT / 2; y < BOARD_HEIGHT; y++){
    for (int x = 0; x < BOARD_WIDTH; x++){
      blocks[y][x] = true;
    }
  }
  
  // - while there are still remaining lives: (this is for every new life - multiple time per game).
  while(!win && lives != 0){
    // - initialize player position, and ball position and direction.
    pos_player = 4;
    ball.pos_x = 4;
    ball.pos_y = 1;
    ball.step_x = 1;
    ball.step_y = 1;
    //    - show game
    display_game();
    //    - wait for user input to start game
    while (read_joystick() == 0);
    while (!win){
      move_player();
      if (!move_ball()){
        life_lost();
        lives--;
        display_score();
        break;
      }
      display_game();
      delay(MOVE_DURATION_MS);
      if (is_blocks_empty()) {
        score += (100 + lives * 10);
        delay(2000);
        game_won();
        win = true;
      }
    }
  }
  delay(5000);
}
