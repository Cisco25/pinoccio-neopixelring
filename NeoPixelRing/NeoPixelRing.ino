#define _debug_

#ifndef _debug_
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
#endif

#include <Adafruit_NeoPixel.h>

#include "version.h"

#define PIN 2

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

#define LEFT  0
#define RIGHT 1

uint8_t  snake_head, snake_tail;
uint8_t   nbPixels;

struct nsnake {
  uint8_t direction;  // LEFT or RIGHT
  uint8_t length;     // from 1 to max number of pixels (16)
  uint8_t speed;      // from 1 to 10
  uint8_t brightness; // from 0 to 255
  uint8_t vRed;       // from 0 to 255
  uint8_t vGreen;     // from 0 to 255
  uint8_t vBlue;      // from 0 to 255
};
typedef struct nsnake neoSnake;

neoSnake snake0 = {   .direction = RIGHT, \
                      .length = 5,  \
                      .speed = 1, \
                      .brightness = 50, \
                      .vRed = 0,    \
                      .vGreen = 255, \
                      .vBlue = 0 };
neoSnake snake1 = {   .direction = LEFT, \
                      .length = 10,  \
                      .speed = 1, \
                      .brightness = 50, \
                      .vRed = 50,    \
                      .vGreen = 150, \
                      .vBlue = 255 };

void setup() {
#ifndef _debug_
  Scout.setup(SKETCH_NAME, SKETCH_REVISION, SKETCH_BUILD);
  addBitlashFunction("neopix.on", (bitlash_function)turnOn);
  addBitlashFunction("neopix.off", (bitlash_function)turnOff);
  addBitlashFunction("neopix.lsnake", (bitlash_function)leftSnake);
  addBitlashFunction("neopix.rsnake", (bitlash_function)rightSnake);
  addBitlashFunction("neopix.flash", (bitlash_function)flash5c);
  addBitlashFunction("neopix.red", (bitlash_function)colorRed);
  addBitlashFunction("neopix.green", (bitlash_function)colorGreen);
  addBitlashFunction("neopix.blue", (bitlash_function)colorBlue);
  addBitlashFunction("neopix.white", (bitlash_function)colorWhite);
  addBitlashFunction("neopix.bonjour", (bitlash_function)bonjour);
#endif

  Serial.begin(115200);
  
  strip.begin();
  nbPixels = strip.numPixels();
}

void loop() {
#ifndef _debug_
  Scout.loop();
#endif

  runNeoSnake(snake0, 3);
  runNeoSnake(snake1, 2);

  //flash(5, 200);
  //delay(500);
}

#ifdef _debug_
void turnOn(neoSnake snake) {
  for(uint16_t i=0; i<nbPixels; i++) {
    strip.setPixelColor(i, strip.Color(snake.vRed, snake.vGreen, snake.vBlue));
    strip.show();
  }
}

void turnOff() {
  for(uint16_t i=0; i<nbPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}
#else
numvar turnOn() {
  for(uint16_t i=0; i<nbPixels; i++) {
    strip.setPixelColor(i, strip.Color(snake.vRed, snake.vGreen, snake.vBlue));
    strip.show();
  }
}

numvar turnOff() {
  for(uint16_t i=0; i<nbPixels; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}

numvar leftSnake() {
  runSnake(LEFT, 5, 70);
}

numvar rightSnake() {
  runSnake(RIGHT, 5, 70);
}

numvar flash5c() {
  flash(5, 200);
}

numvar colorRed() {
  snake.vRed = 255;
  snake.vGreen = 0;
  snake.vBlue = 0;
  Blink();
}

numvar colorGreen() {
  snake.vRed = 0;
  snake.vGreen = 255;
  snake.vBlue = 0;
  Blink();
}

numvar colorBlue() {
  snake.vRed = 0;
  snake.vGreen = 0;
  snake.vBlue = 255;
  Blink();
}

numvar colorWhite() {
  snake.vRed = 255;
  snake.vGreen = 255;
  snake.vBlue = 255;
  Blink();
}

numvar bonjour() {
  uint8_t sRed, sGreen, sBlue;
  // Save original colors
  sRed = snake.vRed;
  sGreen = snake.vGreen;
  sBlue = snake.vBlue;
  colorRed();
  runSnake(LEFT, 1, 70);
  colorGreen();
  runSnake(RIGHT, 1, 70);
  colorBlue();
  runSnake(LEFT, 1, 70);
  colorWhite();
  runSnake(RIGHT, 1, 70);
  // Load original colors
  snake.vRed = sRed;
  snake.vGreen = sGreen;
  snake.vBlue = sBlue;
}
#endif

void initNeoSnake(neoSnake snake) {
  uint32_t cRed, cGreen, cBlue;
  uint32_t sRed, sGreen, sBlue;

  cRed = snake.vRed / snake.length * snake.brightness / 255;
  cGreen = snake.vGreen / snake.length * snake.brightness / 255;
  cBlue = snake.vBlue / snake.length * snake.brightness / 255;

#ifdef _debug_
  Serial.println();
#endif

  if (!snake.direction) {
    snake_head = snake.length-1;
    snake_tail = 0;
  
    for(uint16_t i = 0; i < snake.length; i++) {
      sRed = (i+1)*cRed;
      sGreen = (i+1)*cGreen;
      sBlue = (i+1)*cBlue;
      strip.setPixelColor(i, strip.Color(sRed, sGreen, sBlue));
      strip.show();
      
#ifdef _debug_
      Serial.print("p[");
      Serial.print(i);
      Serial.print("] = [");
      Serial.print(sRed);
      Serial.print("\t");
      Serial.print(sGreen);
      Serial.print("\t");
      Serial.print(sBlue);
      Serial.println("]");
#endif
    }

    for(uint16_t i = snake.length; i < nbPixels; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show(); 
    }
  }
  else {
    snake_head = nbPixels-1 - snake.length;
    snake_tail = nbPixels-1;
    
    for(uint16_t i = nbPixels-1; i > snake_head; i--) {
      sRed = (nbPixels-i) * cRed;
      sGreen = (nbPixels-i) * cGreen;
      sBlue = (nbPixels-i) * cBlue;
      strip.setPixelColor(i, strip.Color(sRed, sGreen, sBlue));
      strip.show();

#ifdef _debug_
      Serial.print("p[");
      Serial.print(i);
      Serial.print("] = [");
      Serial.print(sRed);
      Serial.print("\t");
      Serial.print(sGreen);
      Serial.print("\t");
      Serial.print(sBlue);
      Serial.println("]");
#endif
    }

    for(uint16_t i = 0; i < snake_head; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    }
  }

#ifdef _debug_
  Serial.print("Head = ");
  Serial.print(snake_head);  
  Serial.print("\tTail = ");
  Serial.println(snake_tail);
#endif
}

void runNeoSnake (neoSnake snake, uint8_t nbTurns) {
  uint16_t  turnCount = 0;

  // Init snake
  initNeoSnake(snake);

  while(turnCount < nbTurns + 1) {
    // DIRECTION = LEFT
    if(!snake.direction) {
      // Head and tail are on the same part of the ring
      if(snake_head > snake_tail) {
        // Snake head is back on origin
        if(snake_head == snake.length) {
          uint32_t color = strip.getPixelColor(nbPixels-1);
          strip.setPixelColor(0, color);
          strip.show();
          
          // Increment turn counter
          turnCount++;

#ifdef _debug_
          Serial.print("\nTurn# ");
          Serial.println(turnCount);
          Serial.println("");
#endif
        }  
        
        // Move snake
        for(uint16_t i = 0; i <= snake_head+1; i++) {
          if(i == snake.length) {
            strip.setPixelColor(snake_tail, strip.Color(0, 0, 0));
            strip.show();
            break;
          } else {
            uint32_t color = strip.getPixelColor(snake_head-i);
            strip.setPixelColor(snake_head+1-i, color);
            strip.show();
          }
        }
      }
      // Head and tail are on separate part of the ring
      else {
        // Move head part of the snake
        for(uint16_t i = 0; i <= snake_head+1; i++) {
          uint32_t color = strip.getPixelColor(snake_head-i);
          strip.setPixelColor(snake_head+1-i, color);
          strip.show();
        }
        
        // Move last pixel to first
        uint32_t color = strip.getPixelColor(nbPixels-1);
        strip.setPixelColor(0, color);
        strip.show();
        
        // Move tail part of the snake
        uint8_t tail_len = ((nbPixels-1)-snake_tail);
        for(uint16_t i = 0; i <= tail_len; i++) {
          if(i == tail_len) {
            strip.setPixelColor(snake_tail, strip.Color(0, 0, 0));
            strip.show();
            break;
          } else {
            uint32_t color = strip.getPixelColor(nbPixels-2-i);
            strip.setPixelColor(nbPixels-1-i, color);
            strip.show();
          }
        }
      }
      
      // Increment head
      if(snake_head < nbPixels-1)
        snake_head++;
      else
        snake_head = 0;
      
      // Increment tail
      if(snake_tail < nbPixels-1)
        snake_tail++;
      else
        snake_tail = 0;
    }
    // DIRECTION = RIGHT
    else {
      // Head and tail are on the same part of the ring
      if(snake_head < snake_tail) {
        // Snake head is back on origin
        if(snake_head == (nbPixels-1 - snake.length)) {
          uint32_t color = strip.getPixelColor(0);
          strip.setPixelColor(nbPixels-1, color);
          strip.show();
          
          // Increment turn counter
          turnCount++;
          
#ifdef _debug_
          Serial.print("\nTurn# ");
          Serial.println(turnCount);
          Serial.println("");
#endif
        }
        
        // Move snake
        for(uint16_t i = snake_head; i <= snake_tail+1; i++) {
          if(i == snake_tail+1) {
            strip.setPixelColor(snake_tail, strip.Color(0, 0, 0));
            strip.show();
            break;
          } else {
            uint32_t color = strip.getPixelColor(i);
            if (i-1 >= 0)
              strip.setPixelColor(i-1, color);
            strip.show();
          }
        }
      }
      // Head and tail are on separate part of the ring
      else {
        // Move head part of the snake
        for(uint16_t i = snake_head; i < nbPixels-1; i++) {
          uint32_t color = strip.getPixelColor(i+1);
          strip.setPixelColor(i, color);
          strip.show();
        }
        
        // Move first pixel to last
        uint32_t color = strip.getPixelColor(0);
        strip.setPixelColor(nbPixels-1, color);
        strip.show();
        
        // Move tail part of the snake
        for(uint16_t i = 0; i <= snake_tail+1; i++) {
          if(i == snake_tail+1) {
            strip.setPixelColor(snake_tail, strip.Color(0, 0, 0));
            strip.show();
            break;
          } else {
            uint32_t color = strip.getPixelColor(i);
            if (i-1 >= 0)
              strip.setPixelColor(i-1, color);
            strip.show();
          }
        }
      }
      
      // Decrement head
      if(snake_head > 0)
        snake_head--;
      else
        snake_head = nbPixels-1;
      
      // Decrement tail
      if(snake_tail > 0)
        snake_tail--;
      else
        snake_tail = nbPixels-1;
    }  
      
  // Wait
  uint16_t wait = 110 - (snake.speed * 10);
  delay(wait);
  }
  
  // Turn off when done
  turnOff();

#ifdef _debug_
  Serial.print("Head = ");
  Serial.print(snake_head);  
  Serial.print("\tTail = ");
  Serial.println(snake_tail);
#endif
}

void Blink () {
  for(int i=0; i<2; i++) {
    turnOn(snake0);
    delay(250);
    turnOff();
    delay(250);
  }
}

void flash(neoSnake snake, uint8_t nbFlash, uint16_t wait) {
  uint16_t  flashCount = 0;
  
  while(flashCount < nbFlash) {
    for(uint16_t i=0; i<nbPixels; i++) {
      strip.setPixelColor(i, strip.Color(snake.vRed, snake.vGreen, snake.vBlue));
      strip.show();
    } 
    delay(wait);
    for(uint16_t i=0; i<nbPixels; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    } 
    delay(wait);
    flashCount++;
  } 
  
  // Turn off when done
  turnOff();
}

