#include <SimbleeForMobile.h>
#include "FastLED.h"

#define DATA_PIN    5
#define CLOCK_PIN   4
#define LED_TYPE    APA102
#define COLOR_ORDER GBR
#define NUM_LEDS    14

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

uint8_t brightness = 128;

CRGB solidColor = CRGB(0, 0, 255);

// List of patterns to cycle through. Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

SimplePatternList patterns = {
  rainbow,
  rainbowWithGlitter,
  confetti,
  sinelon,
  juggle,
  bpm,
  showSolidColor,
  off,
};

uint8_t currentPatternIndex = 0;
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
uint8_t patternCount = ARRAY_SIZE(patterns);

uint8_t hue = 0; // rotating "base color" used by many of the patterns

// pattern screen controls
uint8_t ui_buttonOff;
uint8_t ui_buttonColor;
uint8_t ui_buttonRainbow;
uint8_t ui_buttonRainbowWithGlitter;
uint8_t ui_buttonConfetti;
uint8_t ui_buttonSinelon;
uint8_t ui_buttonBeat;
uint8_t ui_buttonJuggle;
uint8_t ui_sliderBrightness;

// color screen controls
uint8_t ui_buttonPattern;
uint8_t ui_textfieldRed;
uint8_t ui_sliderRed;
uint8_t ui_textfieldGreen;
uint8_t ui_sliderGreen;
uint8_t ui_textfieldBlue;
uint8_t ui_sliderBlue;
uint8_t ui_rectSwatch;
uint8_t ui_imageColorWheel;

void setup() {
  Serial.begin(57600);
  delay(500); // 3 second delay for recovery

  SimbleeForMobile.deviceName = "LumoStuff";
  SimbleeForMobile.advertisementData = "Strip";
  SimbleeForMobile.domain = "Simblee.com";
  SimbleeForMobile.begin();

  FastLED.addLeds<LED_TYPE, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(brightness);
}

void loop() {
  SimbleeForMobile.process();

  patterns[currentPatternIndex]();

  FastLED.show();

  EVERY_N_MILLIS(30) {
    hue++;
  }
}

void showSolidColor() {
  fill_solid(leds, NUM_LEDS, solidColor);
}

void off() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, hue, 7);
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter) {
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( hue + random8(64), 200, 255);
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( hue, 255, 192);
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, hue + (i * 2), beat - hue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void patternSelectorScreen() {
  SimbleeForMobile.beginScreen(WHITE);

  SimbleeForMobile.drawText(85, 30, "Choose a pattern....", BLUE);

  ui_buttonColor = SimbleeForMobile.drawButton(20, 320, 280, "Color");
  ui_buttonOff = SimbleeForMobile.drawButton(20, 360, 280, "Off");
  ui_buttonRainbow = SimbleeForMobile.drawButton(20, 80, 280, "Rainbow");
  ui_buttonRainbowWithGlitter = SimbleeForMobile.drawButton(20, 120, 280, "Rainbow with Glitter");
  ui_buttonConfetti = SimbleeForMobile.drawButton(20, 160, 280, "Confetti");
  ui_buttonSinelon = SimbleeForMobile.drawButton(20, 200, 280, "Sinelon");
  ui_buttonBeat = SimbleeForMobile.drawButton(20, 240, 280, "Beat");
  ui_buttonJuggle = SimbleeForMobile.drawButton(20, 280, 280, "Juggle");

  ui_sliderBrightness = SimbleeForMobile.drawSlider(20, 420, 280, 0, 255);
  SimbleeForMobile.updateValue(ui_sliderBrightness, brightness);

  // we need a momentary button (the default is a push button)
  //  SimbleeForMobile.setEvents(ui_button1, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button2, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button3, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button4, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button5, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button6, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button7, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_button8, EVENT_PRESS | EVENT_RELEASE);
  //  SimbleeForMobile.setEvents(ui_slider, EVENT_DRAG | EVENT_POSITION | EVENT_PRESS | EVENT_RELEASE);

  SimbleeForMobile.endScreen();
}

void colorSelectorScreen() {
  color_t darkgray = rgb(85, 85, 85);

  SimbleeForMobile.beginScreen(darkgray);

  // put the controls at the top, so the keyboard doesn't cover them up

  SimbleeForMobile.drawText(25, 71, "R:", WHITE);
  ui_sliderRed = SimbleeForMobile.drawSlider(55, 65, 175, 0, 255);
  ui_textfieldRed = SimbleeForMobile.drawTextField(245, 65, 50, 255, "", WHITE, darkgray);

  SimbleeForMobile.drawText(25, 116, "G:", WHITE);
  ui_sliderGreen = SimbleeForMobile.drawSlider(55, 110, 175, 0, 255);
  ui_textfieldGreen = SimbleeForMobile.drawTextField(245, 110, 50, 255, "", WHITE, darkgray);

  SimbleeForMobile.drawText(25, 161, "B:", WHITE);
  ui_sliderBlue = SimbleeForMobile.drawSlider(55, 155, 175, 0, 255);
  ui_textfieldBlue = SimbleeForMobile.drawTextField(245, 155, 50, 255, "", WHITE, darkgray);

  // border
  SimbleeForMobile.drawRect(25, 200, 270, 40, WHITE);
  ui_rectSwatch = SimbleeForMobile.drawRect(26, 201, 268, 38, WHITE);

  ui_imageColorWheel = SimbleeForMobile.drawImage(COLOR_WHEEL, 10, 250);

  SimbleeForMobile.setEvents(ui_imageColorWheel, EVENT_COLOR);

  SimbleeForMobile.endScreen();
}

void ui() {
  if (SimbleeForMobile.screen == 1) {
    patternSelectorScreen();
  }
  else if (SimbleeForMobile.screen == 2) {
    colorSelectorScreen();
  }
}

void ui_event(event_t &event) {
  if (event.id == ui_buttonColor) {
    currentPatternIndex = patternCount - 2; // off is always the second to last 'pattern'
    SimbleeForMobile.showScreen(2);
  }
  else if (event.id == ui_buttonPattern) {
    SimbleeForMobile.showScreen(1);
  }
  else if (event.id == ui_buttonOff) {
    currentPatternIndex = patternCount - 1; // off is always the last 'pattern'
  }
  else if (event.id == ui_sliderBrightness) {
    brightness = event.value;
    FastLED.setBrightness(brightness);
  }
  else if (event.id == ui_buttonRainbow) {
    currentPatternIndex = 0;
  }
  else if (event.id == ui_buttonRainbowWithGlitter) {
    currentPatternIndex = 1;
  }
  else if (event.id == ui_buttonConfetti) {
    currentPatternIndex = 2;
  }
  else if (event.id == ui_buttonSinelon) {
    currentPatternIndex = 3;
  }
  else if (event.id == ui_buttonJuggle) {
    currentPatternIndex = 4;
  }
  else if (event.id == ui_buttonBeat) {
    currentPatternIndex = 5;
  }
  else if (event.id == ui_imageColorWheel) {
    solidColor.r = event.red;
    solidColor.g = event.green;
    solidColor.b = event.blue;
    SimbleeForMobile.updateValue(ui_sliderRed, solidColor.r);
    SimbleeForMobile.updateValue(ui_textfieldRed, solidColor.r);
    SimbleeForMobile.updateValue(ui_sliderGreen, solidColor.g);
    SimbleeForMobile.updateValue(ui_textfieldGreen, solidColor.g);
    SimbleeForMobile.updateValue(ui_sliderBlue, solidColor.b);
    SimbleeForMobile.updateValue(ui_textfieldBlue, solidColor.b);
  }
  else if (event.id == ui_sliderRed) {
    solidColor.r = event.value;
    SimbleeForMobile.updateValue(ui_textfieldRed, solidColor.r);
  }
  else if (event.id == ui_textfieldRed) {
    solidColor.r = event.value;
    SimbleeForMobile.updateValue(ui_sliderRed, solidColor.r);
  }
  else if (event.id == ui_sliderGreen) {
    solidColor.g = event.value;
    SimbleeForMobile.updateValue(ui_textfieldGreen, solidColor.g);
  }
  else if (event.id == ui_textfieldGreen) {
    solidColor.g = event.value;
    SimbleeForMobile.updateValue(ui_sliderGreen, solidColor.g);
  }
  else if (event.id == ui_sliderBlue) {
    solidColor.b = event.value;
    SimbleeForMobile.updateValue(ui_textfieldBlue, solidColor.b);
  }
  else if (event.id == ui_textfieldBlue) {
    solidColor.b = event.value;
    SimbleeForMobile.updateValue(ui_sliderBlue, solidColor.b);
  }

  SimbleeForMobile.updateColor(ui_rectSwatch, rgb(solidColor.r, solidColor.g, solidColor.b));
}

