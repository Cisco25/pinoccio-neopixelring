#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
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

uint8_t   snake_len = 8;
uint32_t  snake_head, snake_tail;
uint8_t   nbPixels;
uint8_t   vRed, vGreen, vBlue;

void setup() {
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
  //Serial.begin(115200);
  
  strip.begin();
  nbPixels = strip.numPixels();
  
  vRed = 255;
  vGreen = 0;
  vBlue = 0;
  bonjour();
}

void loop() {
  Scout.loop();
  //runSnake(LEFT, 5, 70);
  //runSnake(RIGHT, 5, 70);
  //flash(5, 200);
  //delay(500);
}

numvar turnOn() {
  for(uint16_t i=0; i<nbPixels; i++) {
    strip.setPixelColor(i, strip.Color(vRed, vGreen, vBlue));
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
  vRed = 255;
  vGreen = 0;
  vBlue = 0;
  Blink();
}

numvar colorGreen() {
  vRed = 0;
  vGreen = 255;
  vBlue = 0;
  Blink();
}

numvar colorBlue() {
  vRed = 0;
  vGreen = 0;
  vBlue = 255;
  Blink();
}

numvar colorWhite() {
  vRed = 255;
  vGreen = 255;
  vBlue = 255;
  Blink();
}

numvar bonjour() {
  uint8_t sRed, sGreen, sBlue;
  // Save original colors
  sRed = vRed;
  sGreen = vGreen;
  sBlue = vBlue;
  colorRed();
  runSnake(LEFT, 1, 70);
  colorGreen();
  runSnake(RIGHT, 1, 70);
  colorBlue();
  runSnake(LEFT, 1, 70);
  colorWhite();
  runSnake(RIGHT, 1, 70);
  // Load original colors
  vRed = sRed;
  vGreen = sGreen;
  vBlue = sBlue;
}

void Blink () {
  for(int i=0; i<2; i++) {
    turnOn();
    delay(250);
    turnOff();
    delay(250);
  }
}

void initSnake(uint8_t direction) {
  uint8_t sRed, sGreen, sBlue;
  
  if(!direction) {
    snake_head = snake_len-1;
    snake_tail = 0;
  
    for(uint16_t i=0; i<snake_len; i++) {
      if(i==0) {
        sRed = (256/snake_len)*vRed;
        sGreen = (256/snake_len)*vGreen;
        sBlue = (256/snake_len)*vBlue;
        strip.setPixelColor(i, strip.Color(sRed, sGreen, sBlue));
      } else if(i==snake_len-1) {
        strip.setPixelColor(i, strip.Color(vRed, vGreen, vBlue));
      } else {
        sRed = (i+1)*(256/snake_len)*vRed;
        sGreen = (i+1)*(256/snake_len)*vGreen;
        sBlue = (i+1)*(256/snake_len)*vBlue;
        strip.setPixelColor(i, strip.Color(sRed, sGreen, sBlue));
      }
      strip.show();
    }
  
    for(uint16_t i=snake_len; i<nbPixels; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    }
  }
  else {
    snake_head = nbPixels-1-snake_len;
    snake_tail = nbPixels-1;
    
    for(uint16_t i=nbPixels-1; i>snake_head; i--) {
      if(i==nbPixels-1) {
        sRed = (256/snake_len)*vRed;
        sGreen = (256/snake_len)*vGreen;
        sBlue = (256/snake_len)*vBlue;
        strip.setPixelColor(i, strip.Color(sRed, sGreen, sBlue));
      } else if(i==snake_head+1) {
        strip.setPixelColor(i, strip.Color(vRed, vGreen, vBlue));
      } else {
        sRed = (nbPixels-i)*(256/snake_len)*vRed;
        sGreen = (nbPixels-i)*(256/snake_len)*vGreen;
        sBlue = (nbPixels-i)*(256/snake_len)*vBlue;
        strip.setPixelColor(i, strip.Color(sRed, sGreen, sBlue));
      }
      strip.show();
    }
  
    for(uint16_t i=0; i<snake_head; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
      strip.show();
    }
  }
  
  // Debug
  /*Serial.print("Head = ");
  Serial.print(snake_head);  
  Serial.print("\tTail = ");
  Serial.println(snake_tail);*/
}

void runSnake(uint8_t direction, uint8_t nbTurns, uint16_t wait) {
  uint16_t  turnCount = 0;
  
  // Init snake
  initSnake(direction);
  
  while(turnCount < nbTurns) {
    // DIRECTION = LEFT
    if(!direction) {
      // Head and tail are on the same part of the ring
      if(snake_head > snake_tail) {
        // Snake head is on last pixel
        if(snake_head == nbPixels-1) {
          uint32_t color = strip.getPixelColor(nbPixels-1);
          strip.setPixelColor(0, color);
          strip.show();
          
          // Increment turn counter
          turnCount++;
          
          // Debug
          //Serial.print("\nTurn# ");
          //Serial.println(turnCount);
          //Serial.println("");
        }  
        
        // Move snake
        for(uint16_t i=0; i<=snake_head+1; i++) {
          if(i == snake_len) {
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
        for(uint16_t i=0; i<=snake_head+1; i++) {
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
        for(uint16_t i=0; i<=tail_len; i++) {
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
        // Snake head is on last pixel
        if(snake_head == 0) {
          uint32_t color = strip.getPixelColor(0);
          strip.setPixelColor(nbPixels-1, color);
          strip.show();
          // Increment turn counter
          turnCount++;
          
          // Debug
          //Serial.print("\nTurn# ");
          //Serial.println(turnCount);
          //Serial.println("");
        }
        
        // Move snake
        for(uint16_t i=snake_head; i<=snake_tail+1; i++) {
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
        for(uint16_t i=snake_head; i<nbPixels-1; i++) {
          uint32_t color = strip.getPixelColor(i+1);
          strip.setPixelColor(i, color);
          strip.show();
        }
        
        // Move first pixel to last
        uint32_t color = strip.getPixelColor(0);
        strip.setPixelColor(nbPixels-1, color);
        strip.show();
        
        // Move tail part of the snake
        for(uint16_t i=0; i<=snake_tail+1; i++) {
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
  // Debug
  /*Serial.print("Head = ");
  Serial.print(snake_head);  
  Serial.print("\tTail = ");
  Serial.println(snake_tail);*/
      
  // Wait
  delay(wait);
  }
  
  // Turn off when done
  turnOff();
}

void flash(uint8_t nbFlash, uint16_t wait) {
  uint16_t  flashCount = 0;
  
  while(flashCount < nbFlash) {
    for(uint16_t i=0; i<nbPixels; i++) {
      strip.setPixelColor(i, strip.Color(vRed, vGreen, vBlue));
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

