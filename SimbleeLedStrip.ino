#define FASTLED_FORCE_SOFTWARE_SPI

#include <SimbleeForMobile.h>
#include <FastLED.h>
#include "GradientPalettes.h"

#define DATA_PIN    5
#define CLOCK_PIN   4
#define LED_TYPE    APA102
#define COLOR_ORDER GBR // RGB
#define NUM_LEDS    16

// ten seconds per color palette makes a good demo
// 20-120 is better for deployment
#define SECONDS_PER_PALETTE 10

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

uint8_t brightness = 32;

CRGB solidColor = CRGB(0, 0, 255);

// List of patterns to cycle through. Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

SimplePatternList patterns = {
  colorwaves,
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

// Current palette number from the 'playlist' of color palettes
uint8_t currentPaletteIndex = 0;

CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(gradientPalettes[0]);

// pattern screen controls
uint8_t ui_buttonOff;
uint8_t ui_buttonColor;
uint8_t ui_buttonColorWaves;
uint8_t ui_buttonRainbow;
uint8_t ui_buttonRainbowWithGlitter;
uint8_t ui_buttonConfetti;
uint8_t ui_buttonSinelon;
uint8_t ui_buttonBeat;
uint8_t ui_buttonJuggle;
uint8_t ui_sliderBrightness;

// color screen controls
uint8_t ui_buttonPattern;
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

  EVERY_N_SECONDS(SECONDS_PER_PALETTE) {
    currentPaletteIndex = addmod8(currentPaletteIndex, 1, gradientPaletteCount);
    targetPalette = gradientPalettes[currentPaletteIndex];
  }

  EVERY_N_MILLIS(30) {
    hue++;
  }

  EVERY_N_MILLISECONDS(40) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, 16);
  }

  patterns[currentPatternIndex]();

  FastLED.show();
}

void showSolidColor() {
  fill_solid(leds, NUM_LEDS, solidColor);
}

void off() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
}

// From ColorWavesWithPalettes by Mark Kriegsman: https://gist.github.com/kriegsman/8281905786e8b2632aeb
// This function draws color waves with an ever-changing,
// widely-varying set of parameters, using a color palette.
void colorwaves() {
  static uint16_t sPseudotime = 0;
  static uint16_t sLastMillis = 0;
  static uint16_t sHue16 = 0;

  uint8_t sat8 = beatsin88(87, 220, 250);
  uint8_t brightdepth = beatsin88(341, 96, 224);
  uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
  uint8_t msmultiplier = beatsin88(147, 23, 60);

  uint16_t hue16 = sHue16;//gHue * 256;
  uint16_t hueinc16 = beatsin88(113, 300, 1500);

  uint16_t ms = millis();
  uint16_t deltams = ms - sLastMillis ;
  sLastMillis  = ms;
  sPseudotime += deltams * msmultiplier;
  sHue16 += deltams * beatsin88(400, 5, 9);
  uint16_t brightnesstheta16 = sPseudotime;

  for (uint16_t i = 0 ; i < NUM_LEDS; i++) {
    hue16 += hueinc16;
    uint8_t hue8 = hue16 / 256;
    uint16_t h16_128 = hue16 >> 7;
    if (h16_128 & 0x100) {
      hue8 = 255 - (h16_128 >> 1);
    } else {
      hue8 = h16_128 >> 1;
    }

    brightnesstheta16  += brightnessthetainc16;
    uint16_t b16 = sin16(brightnesstheta16) + 32768;

    uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
    uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
    bri8 += (255 - brightdepth);

    uint8_t index = hue8;
    //index = triwave8(index);
    index = scale8(index, 240);

    CRGB newcolor = ColorFromPalette(currentPalette, index, bri8);

    uint16_t pixelnumber = i;
    pixelnumber = (NUM_LEDS - 1) - pixelnumber;

    nblend(leds[pixelnumber], newcolor, 128);
  }
}

// patterns from DemoReel100 FastLED example: https://github.com/FastLED/FastLED/blob/master/examples/DemoReel100/DemoReel100.ino

void rainbow() {
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, hue, 7);
}

void rainbowWithGlitter() {
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter) {
  if (random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(hue + random8(64), 200, 255);
}

// sinelon with no visual gaps at any speed or pixel count
// by Mark Kriegsman: https://gist.github.com/kriegsman/261beecba85519f4f934
void sinelon() {
  // a colored dot sweeping
  // back and forth, with
  // fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  static int prevpos = 0;
  if ( pos < prevpos ) {
    fill_solid(leds + pos, (prevpos - pos) + 1, CHSV(hue, 220, 255));
  } else {
    fill_solid(leds + prevpos, (pos - prevpos) + 1, CHSV(hue, 220, 255));
  }
  prevpos = pos;
}

void bpm() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette(palette, hue + (i * 2), beat - hue + (i * 10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void patternSelectorScreen() {
  SimbleeForMobile.beginScreen(WHITE);

  SimbleeForMobile.drawText(85, 30, "Choose a pattern....", BLUE);

  uint16_t margin = 10;
  uint16_t height = 40;
  uint16_t y = 70;
  uint16_t width = SimbleeForMobile.screenWidth / 2;
  width -= margin * 1.5;

  ui_buttonOff                = SimbleeForMobile.drawButton(margin, y, width, "Off");
  ui_buttonColor              = SimbleeForMobile.drawButton(margin * 2 + width, y, width, "Color");

  y += height;
  ui_buttonColorWaves         = SimbleeForMobile.drawButton(margin, y, width, "ColorWaves");
  ui_buttonRainbow            = SimbleeForMobile.drawButton(margin * 2 + width, y, width, "Rainbow");

  y += height;
  ui_buttonRainbowWithGlitter = SimbleeForMobile.drawButton(margin, y, width, "Rainbow w Glitter");
  ui_buttonConfetti           = SimbleeForMobile.drawButton(margin * 2 + width, y, width, "Confetti");

  y += height;
  ui_buttonSinelon            = SimbleeForMobile.drawButton(margin, y, width, "Sinelon");
  ui_buttonJuggle             = SimbleeForMobile.drawButton(margin * 2 + width, y, width, "Juggle");

  y += height;
  ui_buttonBeat               = SimbleeForMobile.drawButton(margin, y, width, "Beat");
  SimbleeForMobile.drawButton(margin * 2 + width, y, width, "");

  y += height;
  SimbleeForMobile.drawButton(margin, y, width, "");
  SimbleeForMobile.drawButton(margin * 2 + width, y, width, "");

  y += height;
  SimbleeForMobile.drawButton(margin, y, width, "");
  SimbleeForMobile.drawButton(margin * 2 + width, y, width, "");

  y += height;
  SimbleeForMobile.drawButton(margin, y, width, "");
  SimbleeForMobile.drawButton(margin * 2 + width, y, width, "");

  y += height;
  SimbleeForMobile.drawText(margin, y, "Brightness:", BLUE);

  y += height / 2;
  width = SimbleeForMobile.screenWidth - margin * 2;
  ui_sliderBrightness = SimbleeForMobile.drawSlider(margin, y, width, 0, 255);
  SimbleeForMobile.setEvents(ui_sliderBrightness, EVENT_DRAG | EVENT_RELEASE);
  SimbleeForMobile.updateValue(ui_sliderBrightness, brightness);

  SimbleeForMobile.endScreen();
}

void colorSelectorScreen() {
  SimbleeForMobile.beginScreen(WHITE);

  // put the controls at the top, so the keyboard doesn't cover them up

  uint16_t margin = 10;
  uint16_t height = 40;
  uint16_t y = 70;
  uint16_t width = SimbleeForMobile.screenWidth / 2;
  width -= margin * 1.5;

  ui_buttonPattern = SimbleeForMobile.drawButton(10, y, width, "Back");

  width = SimbleeForMobile.screenWidth;
  width -= margin * 2;
  
  // border
  SimbleeForMobile.drawRect(margin, 120, width, height, BLACK);
  ui_rectSwatch = SimbleeForMobile.drawRect(margin + 1, 121, width - 2, height - 2, WHITE);

  ui_imageColorWheel = SimbleeForMobile.drawImage(COLOR_WHEEL, margin, 170);

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
  Serial.print("screen: ");
  Serial.println(SimbleeForMobile.screen);
  Serial.print("event.id: ");
  Serial.println(event.id);

  if (SimbleeForMobile.screen == 1) {
    if (event.id == ui_buttonColor) {
      currentPatternIndex = patternCount - 2; // solid color is always the second to last 'pattern'
      SimbleeForMobile.showScreen(2);
    }
    else if (event.id == ui_buttonOff) {
      currentPatternIndex = patternCount - 1; // off is always the last 'pattern'
    }
    else if (event.id == ui_sliderBrightness) {
      brightness = event.value;
      FastLED.setBrightness(brightness);
    }
    else if (event.id == ui_buttonColorWaves) {
      currentPatternIndex = 0;
    }
    else if (event.id == ui_buttonRainbow) {
      currentPatternIndex = 1;
    }
    else if (event.id == ui_buttonRainbowWithGlitter) {
      currentPatternIndex = 2;
    }
    else if (event.id == ui_buttonConfetti) {
      currentPatternIndex = 3;
    }
    else if (event.id == ui_buttonSinelon) {
      currentPatternIndex = 4;
    }
    else if (event.id == ui_buttonJuggle) {
      currentPatternIndex = 5;
    }
    else if (event.id == ui_buttonBeat) {
      currentPatternIndex = 6;
    }
  }
  else if (SimbleeForMobile.screen == 2) {
    if (event.id == ui_buttonPattern) {
      SimbleeForMobile.showScreen(1);
    }
    else if (event.id == ui_imageColorWheel) {
      Serial.print("R,G,B: ");
      Serial.print(event.red);
      Serial.print(",");
      Serial.print(event.green);
      Serial.print(",");
      Serial.println(event.blue);

      solidColor.r = event.red;
      solidColor.g = event.green;
      solidColor.b = event.blue;
      SimbleeForMobile.updateColor(ui_rectSwatch, rgb(solidColor.r, solidColor.g, solidColor.b));
    }
  }
}

