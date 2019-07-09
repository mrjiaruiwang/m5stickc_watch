#include <M5StickC.h>
#include <EEPROM.h>
#define COLOR_BACKGROUND BLACK

// Set time and date
int set_hours = 14;
int set_minutes = 56;
int set_seconds = 0; // 22 second upload delay
float set_lunar_day = 7.44;
int set_weekday = 2;
int set_date = 9;
int set_month = 7;
int set_year = 2019;
int set_enable = 0;

// for setting RTC date time
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

// global variables
int current_hour;
int current_minute;
//int current_date;
int led_count = 8;
float chrono_sec;
int chrono_min;
int chrono_hrs;
int chrono_state;
int alarm_hrs;
int alarm_min;
int alarm_state;
int alarm_state_prior;
int alarm_active;
int loop_delay_ms = 1000;

// SH200I gyro & acc chip
int16_t accX = 0;
int16_t accY = 0;
int16_t accZ = 0;
int16_t gyroX = 0;
int16_t gyroY = 0;
int16_t gyroZ = 0;
//float elapsed_day;
float lunar_period = 1447/49; // days, 1 day deviation per 3 millenia
//float lunar_day;

// game
int state_game = 0;

void setup() {
  M5.begin();
  
  // Time set
  if (set_enable > 0) {
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = set_hours;
    TimeStruct.Minutes = set_minutes;
    TimeStruct.Seconds = set_seconds;
    M5.Rtc.SetTime(&TimeStruct);
    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = set_weekday;
    DateStruct.Month = set_month;
    DateStruct.Date = set_date;
    DateStruct.Year = set_year;
    M5.Rtc.SetData(&DateStruct);
    //lunar_day = set_lunar_day; // days
    set_enable = 0;
  }
  
  // put your setup code here, to run once:
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  
  // Gyro & accelerometer init
  M5.IMU.Init();

  // Screen background brightness
  M5.Axp.ScreenBreath(led_count);

  // Battery init
  M5.Axp.EnableCoulombcounter();

  // Sonnerie init
  M5.Rtc.GetBm8563Time();
  current_minute = M5.Rtc.Minute;
  current_hour = M5.Rtc.Hour;

  // Alarm init
  alarm_hrs = M5.Rtc.Hour;
  alarm_min = M5.Rtc.Minute;

  // I/O
  pinMode(M5_LED, OUTPUT);
  digitalWrite(M5_LED, HIGH);
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_RST, INPUT);

  // Flappy bird
  resetMaxScore();
}

int menu_len = 5;
int menu_idx = 0;

void loop() {
  // Handle home button click
  if(digitalRead(M5_BUTTON_HOME) == LOW){
    //digitalWrite(M5_LED, LOW);
    //delay(100);
    //digitalWrite(M5_LED, HIGH);
    if (menu_idx == 0) {
      minute_repeat();
    } else if (menu_idx == 1) {
      toggle_chronograph();
    } else if (menu_idx == 2) {
      toggle_alarm();
    } else if (menu_idx == 3) {
      toggle_game();      
    } else if (menu_idx == 4) {
      advance_backlight();
    }
  }

  // Handle second button click
  if(digitalRead(M5_BUTTON_RST) == LOW){
    menu_idx = menu_idx + 1;
    if (menu_idx % menu_len == 0) {
      menu_idx = 0;
    }
    M5.Lcd.fillScreen(COLOR_BACKGROUND);
    digitalWrite(M5_LED, LOW);
    delay(100);
    digitalWrite(M5_LED, HIGH);
  }

  if (menu_idx == 0) {
    display_clock();
  } else if (menu_idx == 1) {
    display_chronograph();
  } else if (menu_idx == 2) {
    display_alarm();
  } else if (menu_idx == 3) {
    display_game();
  } else if (menu_idx == 4) {
    display_battery();
  }
  // put your main code here, to run repeatedly:
  //M5.Rtc.GetBm8563Time();
  //M5.Lcd.setCursor(0, 20, 2); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
  //M5.Lcd.printf("%02d : %02d : %02d\n", M5.Rtc.Hour, M5.Rtc.Minute, M5.Rtc.Second);
  delay(loop_delay_ms);
}


void display_clock() {
  //M5.Lcd.setTextColor(RED, WHITE);
  M5.Lcd.setTextSize(1);
  M5.Rtc.GetBm8563Time();
  M5.Rtc.GetData(&RTC_DateStruct);
  M5.Lcd.setCursor(6, 28, 4); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
  int hr1;
  int hr2;
  char ampm1;
  char ampm2;
  int hr = M5.Rtc.Hour;
  char ampm = 'A';
  if (hr == 12) {
   ampm = 'P';
  }
  if (hr > 12) {
    hr = hr - 12;
    ampm = 'P';
  }
  M5.Lcd.printf("%02d:%02d:%02d %cM\n", hr, M5.Rtc.Minute, M5.Rtc.Second, ampm);

  // Second time zone
  //M5.Lcd.setCursor(34, 3, 2); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
  hr1 = M5.Rtc.Hour - 3;
  if (hr1 < 0 ) {
    hr1 = hr1 + 24;
  }
  ampm1 = 'A';
  if (hr1 > 12) {
    hr1 = hr1 - 12;
    ampm1 = 'P';
  }
  //M5.Lcd.printf("%02d:%02d:%02d %cM\n", hr1, M5.Rtc.Minute, M5.Rtc.Second, ampm1);

  // Third time zone
  hr2 = M5.Rtc.Hour + 6;
  if (hr2 > 24 ) {
    hr2 = hr2 - 24;
  }
  ampm2 = 'A';
  if (hr2 > 12) {
    hr2 = hr2 - 12;
    ampm2 = 'P';
  }
  M5.Lcd.setCursor(8, 58, 2); // 34, 58, 2 //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
  M5.Lcd.printf("CA %02d %cM | DE %02d %cM\n", hr1, ampm1, hr2, ampm2);

  // Date
  M5.Lcd.setCursor(8, 3, 2);
  char* weekstr;
  int wkday = RTC_DateStruct.WeekDay;
  switch (wkday) {
    case 1: {
      weekstr = "Mon"; //"Mon"; Mo
      break;
    }
    case 2: {
      weekstr = "Tue"; //"Tue"; Di
      break;
    }
    case 3: {
      weekstr = "Wed"; //"Wed"; Mi
      break;
    }
    case 4: {
      weekstr = "Thu"; //"Thu"; Do
      break;
    }
    case 5: {
      weekstr = "Fri"; //"Fri"; Fri
      break;
    }
    case 6: {
      weekstr = "Sat"; //"Sat"; Sa
      break;
    }
    case 7: {
      weekstr = "Sun"; //"Sun"; So
      break;
    }
  }
  M5.Lcd.printf("%s %i-%02d-%02d", weekstr, RTC_DateStruct.Year, RTC_DateStruct.Month,RTC_DateStruct.Date);

  // Moon phase
  //int current_date
  float current_lunar_day;
  float moon_phase;
  //float lunar_period;
  //float lunar_day;
  current_lunar_day = (RTC_DateStruct.Date - set_date) + set_lunar_day;
  moon_phase = current_lunar_day / lunar_period;
  moon_phase = moon_phase - floor(moon_phase);
  M5.Lcd.setCursor(122, 3, 2);
  M5.Lcd.printf("%.0f%%\n",100*(moon_phase));

  // trigger hours sonnerie
  if (M5.Rtc.Hour != current_hour) {
    current_hour = M5.Rtc.Hour;
    int current_hour_s = current_hour;
    if (current_hour_s > 12) {
      current_hour_s = current_hour_s - 12;
    }
    for (int i = 0; i < current_hour_s; i++) {
      digitalWrite(M5_LED, LOW);
      delay(500);
      digitalWrite(M5_LED, HIGH);
      delay(500);
    }
  }
  
  // trigger quarter and minute sonnerie
  if (M5.Rtc.Minute != current_minute) {
    current_minute = M5.Rtc.Minute;

    // Strike the quarters
    if (current_minute % 15 == 0) {
      for (int i = 0; i < floor(M5.Rtc.Minute/15); i++) {
        digitalWrite(M5_LED, LOW);
        delay(100);
        digitalWrite(M5_LED, HIGH);
        delay(100);
        digitalWrite(M5_LED, LOW);
        delay(100);
        digitalWrite(M5_LED, HIGH);
        delay(700);
      }
    }
    
    // Strike the minutes
    // Working code below - uncomment to include
    /*
    for (int i = 0; i < (current_minute % 15); i++) {
      digitalWrite(M5_LED, LOW);
      delay(100);
      digitalWrite(M5_LED, HIGH);
      delay(700);
    }
    */
    // End minute striking
  }
}

void minute_repeat() {
  // strike hours
  int current_hour_s = M5.Rtc.Hour;
  if (current_hour_s > 12) {
    current_hour_s = current_hour_s - 12;
  }
  for (int i = 0; i < current_hour_s; i++) {
    digitalWrite(M5_LED, LOW);
    delay(500);
    digitalWrite(M5_LED, HIGH);
    delay(500);
  }
  // strike quarters
  for (int i = 0; i < floor(M5.Rtc.Minute/15); i++) {
    digitalWrite(M5_LED, LOW);
    delay(100);
    digitalWrite(M5_LED, HIGH);
    delay(100);
    digitalWrite(M5_LED, LOW);
    delay(100);
    digitalWrite(M5_LED, HIGH);
    delay(700);
  }
  // strike minutes
  for (int i = 0; i < (M5.Rtc.Minute % 15); i++) {
    digitalWrite(M5_LED, LOW);
    delay(100);
    digitalWrite(M5_LED, HIGH);
    delay(700);
  }
  // refractory period
  delay(5000);
}


void display_chronograph() {
  //M5.Lcd.setTextColor(RED, WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(26, 28, 4); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160

  if (chrono_state == 1) {
    chrono_sec = chrono_sec + (loop_delay_ms * 0.001);
    if (chrono_sec > 60) {
      chrono_sec = 0;
      chrono_min = chrono_min + 1;
    }
    if (chrono_min > 60) {
      chrono_min = 0;
      chrono_hrs = chrono_hrs + 1;
    }
  }
  M5.Lcd.printf("%02d:%02d:%02.0f\n", chrono_hrs, chrono_min, chrono_sec);
}

void toggle_chronograph() {
  if (chrono_state == 2) {
    chrono_sec = 0;
    chrono_min = 0;
    chrono_hrs = 0;
    M5.Lcd.fillScreen(COLOR_BACKGROUND);
    /*
    M5.Lcd.setCursor(26, 3, 2);
    M5.Lcd.printf("Chrono cleared\n");
    */
  }

  // change state
  chrono_state = chrono_state + 1;
  if (chrono_state > 2) {
    chrono_state = 0;
  }

  digitalWrite(M5_LED, LOW);
  delay(100);
  digitalWrite(M5_LED, HIGH);
}


void display_alarm() {

  M5.Lcd.setTextSize(1);
  M5.Rtc.GetBm8563Time();
  M5.Lcd.setCursor(26, 62, 1); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
  int hr = M5.Rtc.Hour;
  char ampm = 'A';
  if (hr == 12) {
   ampm = 'P';
  }
  if (hr > 12) {
    hr = hr - 12;
    ampm = 'P';
  }
  M5.Lcd.printf("%02d:%02d:%02d %cM\n", hr, M5.Rtc.Minute, M5.Rtc.Second, ampm);
  
  // set alarm
  // Note gyro and acc are swapped in library
  M5.IMU.getGyroAdc(&gyroX,&gyroY,&gyroZ);
  M5.IMU.getAccelAdc(&accX,&accY,&accZ);
  // X direction is rotation about the wrist axis
  float acc_x = ((float) accX) * M5.IMU.aRes; // milligrams
  float gyr_x = ((float) gyroX) * M5.IMU.gRes; // 0 is level, 0.5 is 90 degrees
  

  // set hours by tilting wrist
  //if ((alarm_state == 2) && (gyr_x > 50) && (abs(acc_x) > 0.3)) {
  if ((alarm_state == 2) && (acc_x < (-0.4))) {
    alarm_hrs = alarm_hrs + 6;
  } else if ((alarm_state == 2) && (acc_x > (0.4))) {
    alarm_hrs = alarm_hrs - 6;
  } else if ((alarm_state == 2) && (acc_x < (-0.2))) {
    alarm_hrs = alarm_hrs + 1;
  } else if ((alarm_state == 2) && (acc_x > (0.2))) {
    alarm_hrs = alarm_hrs - 1;
  } else if ((alarm_state == 3) && (acc_x < (-0.4))) {
    alarm_min = alarm_min + 15;
  } else if ((alarm_state == 3) && (acc_x > (0.4))) {
    alarm_min = alarm_min - 15;
  } else if ((alarm_state == 3) && (acc_x < (-0.2))) {
    alarm_min = alarm_min + 1;
  } else if ((alarm_state == 3) && (acc_x > (0.2))) {
    alarm_min = alarm_min - 1;
  }

  // overflow protect
  if (alarm_hrs < 0) {
    alarm_hrs = 0;
  }
  if (alarm_hrs > 23) {
    alarm_hrs = 23;
  }
  if (alarm_min < 0) {
    alarm_min = 0;
  }
  if (alarm_min > 59) {
    alarm_min = 59;
  }

  // show alarm
  if (alarm_state_prior != alarm_state) {
    M5.Lcd.fillScreen(COLOR_BACKGROUND);
  }
  
  //M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(26, 28, 4); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
  int alarm_hrs_disp = alarm_hrs;
  char ampm_a = 'A';
  if (alarm_hrs_disp == 12) {
   ampm_a = 'P';
  }
  if (alarm_hrs_disp > 12) {
    alarm_hrs_disp = alarm_hrs_disp - 12;
    ampm_a = 'P';
  }
  M5.Lcd.printf("%02d:%02d %cM\n", alarm_hrs_disp, alarm_min, ampm_a);
  if (alarm_state == 0) {
    M5.Lcd.setCursor(26, 3, 2);
    M5.Lcd.printf("Alarm OFF\n");
  } else if (alarm_state == 1) {
    M5.Lcd.setCursor(26, 3, 2);
    M5.Lcd.printf("Alarm ON\n");
  } else if (alarm_state == 2) {
    M5.Lcd.setCursor(26, 3, 2);
    M5.Lcd.printf("Alarm set HRS\n");
  } else if (alarm_state == 3) {
    M5.Lcd.setCursor(26, 3, 2);
    M5.Lcd.printf("Alarm set MIN\n");
  }

  alarm_state_prior = alarm_state;

  // Activate alarm
  if (((alarm_hrs == M5.Rtc.Hour) && (alarm_min == M5.Rtc.Minute)) && (alarm_active == 0)) {
    alarm_active = 1;
  } else {
    alarm_active == 0;
  }
  if ((alarm_state == 1) && (alarm_active == 1)) {
    digitalWrite(M5_LED, LOW);
    delay(20);
    digitalWrite(M5_LED, HIGH);
    delay(20);
    digitalWrite(M5_LED, LOW);
    delay(20);
    digitalWrite(M5_LED, HIGH);
    delay(20);
    digitalWrite(M5_LED, LOW);
    delay(20);
    digitalWrite(M5_LED, HIGH);
  }
}

void toggle_alarm() {

  // stop alarm
  if (alarm_active == 1) {
    alarm_active = 0;
    alarm_state = 0;
  } else {
  /*
  if (alarm_state == 0) {
    alarm_active = 0;
  }
  */

  // change state
    alarm_state = alarm_state + 1;
    if (alarm_state > 3) {
      alarm_state = 0;
    }
  }

  digitalWrite(M5_LED, LOW);
  delay(100);
  digitalWrite(M5_LED, HIGH);
}

void display_battery() {
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(2, 2, 1);
  M5.Lcd.printf("--- Battery --------------\r\n");
  M5.Lcd.printf("  Time to empty: %.2f hr\r\n",(M5.Axp.GetCoulombData())/(M5.Axp.GetIdischargeData() / 2));
  M5.Lcd.printf("  Capacity: %.2f mAh\r\n",M5.Axp.GetCoulombData());
  M5.Lcd.printf("  Charge: %dmA\r\n",M5.Axp.GetIchargeData()/2 );
  M5.Lcd.printf("  Discharge: %dmA\r\n",M5.Axp.GetIdischargeData()/2 );
  M5.Lcd.printf("  Voltage: %.3fV\r\n",M5.Axp.GetVbatData() * 1.1 / 1000);
  M5.Lcd.printf("  Temp: %.1fC\r\n",-144.7 + M5.Axp.GetTempData() * 0.1);
  M5.Lcd.printf("  Backlight: %i/3\r\n",led_count - 7);
}

void advance_backlight() {
  if(digitalRead(M5_BUTTON_HOME) == LOW){
    led_count++;
    if(led_count >= 11)
      led_count = 7;
    while(digitalRead(M5_BUTTON_HOME) == LOW);
    M5.Axp.ScreenBreath(led_count);
  }
}

void toggle_game() {
  digitalWrite(M5_LED, LOW);
  delay(100);
  digitalWrite(M5_LED, HIGH);
  if (state_game == 0) {
    state_game = 1;
  } else {
    state_game = 0;
  }
}

void display_game() {
  if (state_game == 0) {
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(14, 20, 4); //int16_t x0, int16_t y0, uint8_t font, Resolution: 80 * 160
    M5.Lcd.printf("Flappy Bird\n");
    M5.Lcd.setCursor(16, 48, 1);
    M5.Lcd.printf("Press home to launch.\n");
  } else {
    M5.Axp.ScreenBreath(11);
    M5.Lcd.setRotation(0);
    game_start();
    game_loop();
    game_over();
    state_game = 0;
    M5.Lcd.setRotation(3);
    //M5.begin();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE,BLACK);
    M5.Axp.ScreenBreath(led_count);
  }
}


// === Flappy Bird ========================================================================
#define TFTW            80     // screen width
#define TFTH            160     // screen height
#define TFTW2           40     // half screen width
#define TFTH2           80     // half screen height
// game constant
#define SPEED            1
#define GRAVITY         9.8
#define JUMP_FORCE      3.6
#define SKIP_TICKS     75.0     // 1000 / 50fps
#define MAX_FRAMESKIP     5
// bird size
#define BIRDW             8     // bird width
#define BIRDH             8     // bird height
#define BIRDW2            4     // half width
#define BIRDH2            4     // half height
// pipe size
#define PIPEW            15     // pipe width
#define GAPHEIGHT        35     // pipe gap height
// floor size
#define FLOORH           20     // floor height (from bottom of the screen)
// grass size
#define GRASSH            4     // grass height (inside floor, starts at floor y)

int maxScore = 0;
const int buttonPin = 2;     
// background
const unsigned int BCKGRDCOL = M5.Lcd.color565(138,235,244);
// bird
const unsigned int BIRDCOL = M5.Lcd.color565(255,254,174);
// pipe
const unsigned int PIPECOL  = M5.Lcd.color565(99,255,78);
// pipe highlight
const unsigned int PIPEHIGHCOL  = M5.Lcd.color565(250,255,250);
// pipe seam
const unsigned int PIPESEAMCOL  = M5.Lcd.color565(0,0,0);
// floor
const unsigned int FLOORCOL = M5.Lcd.color565(246,240,163);
// grass (col2 is the stripe color)
const unsigned int GRASSCOL  = M5.Lcd.color565(141,225,87);
const unsigned int GRASSCOL2 = M5.Lcd.color565(156,239,88);

// bird sprite
// bird sprite colors (Cx name for values to keep the array readable)
#define C0 BCKGRDCOL
#define C1 M5.Lcd.color565(195,165,75)
#define C2 BIRDCOL
#define C3 TFT_WHITE
#define C4 TFT_RED
#define C5 M5.Lcd.color565(251,216,114)

static unsigned int birdcol[] =
{ C0, C0, C1, C1, C1, C1, C1, C0, C0, C0, C1, C1, C1, C1, C1, C0,
  C0, C1, C2, C2, C2, C1, C3, C1, C0, C1, C2, C2, C2, C1, C3, C1,
  C0, C2, C2, C2, C2, C1, C3, C1, C0, C2, C2, C2, C2, C1, C3, C1,
  C1, C1, C1, C2, C2, C3, C1, C1, C1, C1, C1, C2, C2, C3, C1, C1,
  C1, C2, C2, C2, C2, C2, C4, C4, C1, C2, C2, C2, C2, C2, C4, C4,
  C1, C2, C2, C2, C1, C5, C4, C0, C1, C2, C2, C2, C1, C5, C4, C0,
  C0, C1, C2, C1, C5, C5, C5, C0, C0, C1, C2, C1, C5, C5, C5, C0,
  C0, C0, C1, C5, C5, C5, C0, C0, C0, C0, C1, C5, C5, C5, C0, C0};

// bird structure
static struct BIRD {
  long x, y, old_y;
  long col;
  float vel_y;
} bird;

// pipe structure
static struct PIPES {
  long x, gap_y;
  long col;
} pipes;

// score
int score;
// temporary x and y var
static short tmpx, tmpy;

// ---------------
// draw pixel
// ---------------
// faster drawPixel method by inlining calls and using setAddrWindow and pushColor
// using macro to force inlining
#define drawPixel(a, b, c) M5.Lcd.setAddrWindow(a, b, a, b); M5.Lcd.pushColor(c)
// ---------------
// game loop
// ---------------
void game_loop() {
  // ===============
  // prepare game variables
  // draw floor
  // ===============
  // instead of calculating the distance of the floor from the screen height each time store it in a variable
  unsigned char GAMEH = TFTH - FLOORH;
  // draw the floor once, we will not overwrite on this area in-game
  // black line
  M5.Lcd.drawFastHLine(0, GAMEH, TFTW, TFT_BLACK);
  // grass and stripe
  M5.Lcd.fillRect(0, GAMEH+1, TFTW2, GRASSH, GRASSCOL);
  M5.Lcd.fillRect(TFTW2, GAMEH+1, TFTW2, GRASSH, GRASSCOL2);
  // black line
  M5.Lcd.drawFastHLine(0, GAMEH+GRASSH, TFTW, TFT_BLACK);
  // mud
  M5.Lcd.fillRect(0, GAMEH+GRASSH+1, TFTW, FLOORH-GRASSH, FLOORCOL);
  // grass x position (for stripe animation)
  long grassx = TFTW;
  // game loop time variables
  double delta, old_time, next_game_tick, current_time;
  next_game_tick = current_time = millis();
  int loops;
  // passed pipe flag to count score
  bool passed_pipe = false;
  // temp var for setAddrWindow
  unsigned char px;
  unsigned char bpx;

  while (1) {
    loops = 0;
    while( millis() > next_game_tick && loops < MAX_FRAMESKIP) {
      if(digitalRead(M5_BUTTON_HOME) == LOW){
        //while(digitalRead(M5_BUTTON_HOME) == LOW);
         if (bird.y > BIRDH2*0.5) bird.vel_y = -JUMP_FORCE;
          // else zero velocity
          else bird.vel_y = 0;
      }
      
      // ===============
      // update
      // ===============
      // calculate delta time
      // ---------------
      old_time = current_time;
      current_time = millis();
      delta = (current_time-old_time)/1000;

      // bird
      // ---------------
      bird.vel_y += GRAVITY * delta;
      bird.y += bird.vel_y;

      // pipe
      // ---------------
      
      pipes.x -= SPEED;
      // if pipe reached edge of the screen reset its position and gap
      if (pipes.x < -PIPEW) {
        pipes.x = TFTW;
        pipes.gap_y = random(10, GAMEH-(10+GAPHEIGHT));
      }

      // ---------------
      next_game_tick += SKIP_TICKS;
      loops++;
    }

    // ===============
    // draw
    // ===============
    // pipe
    // ---------------
    // we save cycles if we avoid drawing the pipe when outside the screen

    if (pipes.x >= 0 && pipes.x < TFTW) {
      // pipe color
      M5.Lcd.drawFastVLine(pipes.x+3, 0, pipes.gap_y, PIPECOL);
      M5.Lcd.drawFastVLine(pipes.x+3, pipes.gap_y+GAPHEIGHT+1, GAMEH-(pipes.gap_y+GAPHEIGHT+1), PIPECOL);
      // highlight
      M5.Lcd.drawFastVLine(pipes.x, 0, pipes.gap_y, PIPEHIGHCOL);
      M5.Lcd.drawFastVLine(pipes.x, pipes.gap_y+GAPHEIGHT+1, GAMEH-(pipes.gap_y+GAPHEIGHT+1), PIPEHIGHCOL);
      // bottom and top border of pipe
      drawPixel(pipes.x, pipes.gap_y, PIPESEAMCOL);
      drawPixel(pipes.x, pipes.gap_y+GAPHEIGHT, PIPESEAMCOL);
      // pipe seam
      drawPixel(pipes.x, pipes.gap_y-6, PIPESEAMCOL);
      drawPixel(pipes.x, pipes.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
      drawPixel(pipes.x+3, pipes.gap_y-6, PIPESEAMCOL);
      drawPixel(pipes.x+3, pipes.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
    }
#if 1
    // erase behind pipe
    if (pipes.x <= TFTW)
     M5.Lcd.drawFastVLine(pipes.x+PIPEW, 0, GAMEH, BCKGRDCOL);
     //M5.Lcd.drawFastVLine(pipes.x, 0, GAMEH, BCKGRDCOL);
    // PIPECOL
#endif
    // bird
    // ---------------
    tmpx = BIRDW-1;
    do {
          px = bird.x+tmpx+BIRDW;
          // clear bird at previous position stored in old_y
          // we can't just erase the pixels before and after current position
          // because of the non-linear bird movement (it would leave 'dirty' pixels)
          tmpy = BIRDH - 1;
          do {
            drawPixel(px, bird.old_y + tmpy, BCKGRDCOL);
          } while (tmpy--);
          // draw bird sprite at new position
          tmpy = BIRDH - 1;
          do {
            drawPixel(px, bird.y + tmpy, birdcol[tmpx + (tmpy * BIRDW)]);
          } while (tmpy--);
    } while (tmpx--);
    // save position to erase bird on next draw
    bird.old_y = bird.y;

    // grass stripes
    // ---------------
    grassx -= SPEED;
    if (grassx < 0) grassx = TFTW;
    M5.Lcd.drawFastVLine( grassx    %TFTW, GAMEH+1, GRASSH-1, GRASSCOL);
    M5.Lcd.drawFastVLine((grassx+64)%TFTW, GAMEH+1, GRASSH-1, GRASSCOL2);

    // ===============
    // collision
    // ===============
    // if the bird hit the ground game over
    if (bird.y > GAMEH-BIRDH) break;
    // checking for bird collision with pipe
    if (bird.x+BIRDW >= pipes.x-BIRDW2 && bird.x <= pipes.x+PIPEW-BIRDW) {
      // bird entered a pipe, check for collision
      if (bird.y < pipes.gap_y || bird.y+BIRDH > pipes.gap_y+GAPHEIGHT) break;
      else passed_pipe = true;
    }
    // if bird has passed the pipe increase score
    else if (bird.x > pipes.x+PIPEW-BIRDW && passed_pipe) {
      passed_pipe = false;
      // erase score with background color
      M5.Lcd.setTextColor(BCKGRDCOL);
      M5.Lcd.setCursor( TFTW2, 4);
      M5.Lcd.print(score);
      // set text color back to white for new score
      M5.Lcd.setTextColor(TFT_WHITE);
      // increase score since we successfully passed a pipe
      score++;
    }

    // update score
    // ---------------
    M5.Lcd.setCursor( 2, 4);
    M5.Lcd.print(score);
  }
  
  // add a small delay to show how the player lost
  delay(1200);
}


// ---------------
// game start
// ---------------
void game_start() {
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.fillRect(0, TFTH2 - 10, TFTW, 1, TFT_WHITE);
  M5.Lcd.fillRect(0, TFTH2 + 15, TFTW, 1, TFT_WHITE);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(1);
  // half width - num char * char width in pixels
  M5.Lcd.setCursor( TFTW2-15, TFTH2 - 6);
  M5.Lcd.println("FLAPPY");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor( TFTW2-15, TFTH2 + 6);
  M5.Lcd.println("-BIRD-");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor( 15, TFTH2 - 21);
  M5.Lcd.println("M5StickC");
  M5.Lcd.setCursor( TFTW2 - 40, TFTH2 + 21);
  M5.Lcd.println("please press home");
  while (1) {
    // wait for push button
     if(digitalRead(M5_BUTTON_HOME) == LOW){
      while(digitalRead(M5_BUTTON_HOME) == LOW);
      break;
    }
        
    }
      // init game settings
      game_init();
}

void game_init() {
  // clear screen
  M5.Lcd.fillScreen(BCKGRDCOL);
  // reset score
  score = 0;
  // init bird
  bird.x = 30;
  bird.y = bird.old_y = TFTH2 - BIRDH;
  bird.vel_y = -JUMP_FORCE;
  tmpx = tmpy = 0;
  // generate new random seed for the pipe gape
  randomSeed(analogRead(0));
  // init pipe
  pipes.x = 0;
  pipes.gap_y = random(20, TFTH-60);
}


// ---------------
// game over
// ---------------
void game_over() {
  M5.Lcd.fillScreen(TFT_BLACK);
  EEPROM_Read(&maxScore,0);
  
  if(score>maxScore)
  {
    EEPROM_Write(&score,0);
    maxScore = score;
    M5.Lcd.setTextColor(TFT_RED);
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setCursor( 0, TFTH2 - 16);
    M5.Lcd.println("NEW HIGHSCORE");
  }
  
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(1);
  // half width - num char * char width in pixels
  M5.Lcd.setCursor( TFTW2 - 25, TFTH2 - 6);
  M5.Lcd.println("GAME OVER");
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor( 1, 10);
  M5.Lcd.print("score: ");
  M5.Lcd.print(score);
  M5.Lcd.setCursor( 5, TFTH2 + 6);
  M5.Lcd.println("press button");
  M5.Lcd.setCursor( 1, 21);
  M5.Lcd.print("Max Score:");
  M5.Lcd.print(maxScore);
  while(1) {
    // wait for push button
    if(digitalRead(M5_BUTTON_HOME) == LOW){
      while(digitalRead(M5_BUTTON_HOME) == LOW);
      break;
    }
  }
}

void resetMaxScore()
{
  EEPROM_Write(&maxScore,0);
}



void EEPROM_Write(int *num, int MemPos)
{
 byte ByteArray[2];
 memcpy(ByteArray, num, 2);
 for(int x = 0; x < 2; x++)
 {
   EEPROM.write((MemPos * 2) + x, ByteArray[x]);
 }  
}



void EEPROM_Read(int *num, int MemPos)
{
 byte ByteArray[2];
 for(int x = 0; x < 2; x++)
 {
   ByteArray[x] = EEPROM.read((MemPos * 2) + x);    
 }
 memcpy(num, ByteArray, 2);
}
// === Flappy Bird ========================================================================
