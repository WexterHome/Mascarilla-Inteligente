  //basado en el proyecto de AlexGyver
#define ADC_PIN 0
#define FREQ_PIN 1
#define VOL_THR 20
#define VOL_MAX 200
#define BRIGHTNESS 100 //brillo 0-255
#define EMOJI_THR 700

#define M_PIN 7       // pin LEDs
#define BUTTON_PIN 2
#define M_WIDTH 8       
#define M_HEIGHT 8   
#define ORDER_GRB
#define COLOR_DEBTH 2
#include <microLED.h>
#define NUM_LEDS M_WIDTH * M_HEIGHT
LEDdata leds[NUM_LEDS];
microLED matrix(leds, M_PIN, M_WIDTH, M_HEIGHT, ZIGZAG, LEFT_TOP, DIR_RIGHT);

#define FHT_N 32   
#define LOG_OUT 1
#include <FHT.h>    

#define WINDOW_SIZE 250
#define ARRAY_SIZE 10
int window[ARRAY_SIZE];
uint32_t tmr, tmr2;
bool emojiState = false;
bool emoji;
int ESTADO;
const int threshold = 150;
volatile int lastTime = 0;

const uint8_t mouthHappy[8][8] = {
  {0x00, 0x00, 0x81, 0x42, 0x3C, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x7E, 0x81, 0x7E, 0x00, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x7E, 0x00, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x42, 0x3C, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00},
  {0x7E, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00},
  {0x7E, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C},
};

const uint8_t mouthSad[8][8] = {
  {0x00, 0x00, 0x00, 0x3C, 0x42, 0x81, 0x00, 0x00},
  {0x00, 0x00, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x7E, 0x81, 0x7E, 0x00, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x7E, 0x00, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x42, 0x3C, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00},
  {0x7E, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00},
  {0x7E, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C},
};
const uint8_t mouthNormal[8][8] = {
  {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00},
  {0x00, 0x00, 0x7E, 0x81, 0x7E, 0x00, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x7E, 0x00, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x42, 0x3C, 0x00, 0x00},
  {0x00, 0x7E, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00},
  {0x7E, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C, 0x00},
  {0x7E, 0x81, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3C},
};

void setup() {
  Serial.begin(9600);
  matrix.setBrightness(BRIGHTNESS);
  ESTADO = 0; //Feliz
  pinMode(BUTTON_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), cambioEstado, RISING);
}

void loop() {
  byte mouthSize = filter(getSoundLength());
  //Serial.print(mouthSize);
  Serial.println(mouthSize);

  analyzeAudio();
  
  int freqVal = 0;
  for (int i = 2; i < 16; i++) {
    freqVal += (max(20, fht_log_out[i]) - 20) * i;
  }
  static float freqFil = 0;
  freqFil += (freqVal - freqFil) * 0.3;

  if (millis() - tmr >= 50) {
    tmr = millis();
    static int counter = 0;
    window[counter] = freqFil;
    int thisMin = 10000, thisMax = 0;
    for (byte i = 0; i < ARRAY_SIZE; i++) {
      if (window[i] > thisMax) thisMax = window[i];
      if (window[i] < thisMin) thisMin = window[i];
    }
    counter++;
    if (counter >= ARRAY_SIZE) counter = 0;
    if (thisMin > 150 && thisMax - thisMin < WINDOW_SIZE) {
      tmr2 = millis();
      emojiState = true;
      emoji = (thisMax + thisMin) / 2 < EMOJI_THR;
    }
  }
  if (millis() - tmr2 >= 1000) {
    emojiState = false;
    tmr2 = millis();
  }

  if(ESTADO >=3) ESTADO = 0;
  if (ESTADO == 0) {    //HAPPY
    LEDdata thisColor = mHSV( 120, 255, 255);
    for (byte y = 0; y < 8; y++) {
      for (byte x = 0; x < 8; x++) {
        matrix.setPix(x, y, bitRead(mouthHappy[mouthSize][y], x) ? thisColor : 0);
      }
    }
  } 
  else if(ESTADO == 1) {              // SAD
    LEDdata thisColor = mHSV(0, 255, 255);
    for (byte y = 0; y < 8; y++) {
      for (byte x = 0; x < 8; x++) {
        matrix.setPix(x, y, bitRead(mouthSad[mouthSize][y], x) ? thisColor : 0);
      }
    } 
  }
  else{              
    LEDdata thisColor = mHSV(190, 255, 255);
    for (byte y = 0; y < 8; y++) {
      for (byte x = 0; x < 8; x++) {
        matrix.setPix(x, y, bitRead(mouthNormal[mouthSize][y], x) ? thisColor : 0);   // рисуем
      }
    } 
  }
  matrix.show();
  delay(10);
}

int filter(int value) { // возвращает фильтрованное значение
  static int buff[3];
  static byte _counter = 0;
  int middle;
  buff[_counter] = value;
  if (++_counter > 2) _counter = 0;

  if ((buff[0] <= buff[1]) && (buff[0] <= buff[2])) {
    middle = (buff[1] <= buff[2]) ? buff[1] : buff[2];
  } else {
    if ((buff[1] <= buff[0]) && (buff[1] <= buff[2])) {
      middle = (buff[0] <= buff[2]) ? buff[0] : buff[2];
    } else {
      middle = (buff[0] <= buff[1]) ? buff[0] : buff[1];
    }
  }
  return middle;
}

void cambioEstado(){
  if((millis()-lastTime)>threshold){
    ESTADO++;
    lastTime = millis();
  }
}
