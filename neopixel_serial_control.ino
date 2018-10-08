//#define REDUCED_MODES // sketch is too big for Arduino w/32k flash, so invoke reduced modes

#include <WS2812FX.h>
#include <EveryTimer.h>

#define LED_COUNT 24
#define LED_PIN   12
#define PERIOD_MS 5000
#define USE_SERIAL

/*
  Parameter 1 = number of pixels in strip
  Parameter 2 = Arduino pin number (most are valid)
  Parameter 3 = pixel type flags, add together as needed:
   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
*/

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
EveryTimer timer;

String cmd = "";               // String to store incoming serial commands
boolean cmd_complete = false;  // whether the command string is complete

void setup() {
#ifdef USE_SERIAL
  Serial.begin(9600);
#endif
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  // ws2812fx.setSpeed(1200);
  ws2812fx.setColor(BLUE);
  // ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  ws2812fx.start();
#ifdef USE_SERIAL
  printModes();
  printUsage();
#endif
  timer.Every(PERIOD_MS, action);
  timer.Start();
}

void loop() {
  ws2812fx.service();

  // On Atmega32U4 based boards (leonardo, micro) serialEvent is not called automatically when data arrive on the serial RX. We need to do it ourself
#if defined(__AVR_ATmega32U4__) || defined(ESP8266)
  serialEvent();
#endif

  if (cmd_complete) process_command();
  timer.Update();
}

void action(void)
{
  int i = ws2812fx.getMode() + 1;
  i = i % ws2812fx.getModeCount();
  ws2812fx.setMode(i);
#ifdef USE_SERIAL
  Serial.print(F("Changing mode to: "));
  Serial.println(ws2812fx.getModeName(i));
#endif
}

/*
   Checks received command and calls corresponding functions.
*/
void process_command(void)
{
  if (cmd == F("b+")) {
    ws2812fx.increaseBrightness(25);
    Serial.print(F("Increased brightness by 25 to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  else if (cmd == F("b-")) {
    ws2812fx.decreaseBrightness(25);
    Serial.print(F("Decreased brightness by 25 to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  else if (cmd.startsWith(F("b "))) {
    uint8_t b = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setBrightness(b);
    Serial.print(F("Set brightness to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  else if (cmd == F("s+")) {
    //  ws2812fx.increaseSpeed(10);
    ws2812fx.setSpeed(ws2812fx.getSpeed() * 1.2);
    Serial.print(F("Increased speed by 20% to: "));
    Serial.println(ws2812fx.getSpeed());
  }
  else if (cmd == F("s-")) {
    //  ws2812fx.decreaseSpeed(10);
    ws2812fx.setSpeed(ws2812fx.getSpeed() * 0.8);
    Serial.print(F("Decreased speed by 20% to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  else if (cmd.startsWith(F("s "))) {
    uint16_t s = (uint16_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setSpeed(s);
    Serial.print(F("Set speed to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  else if (cmd.startsWith(F("m "))) {
    uint8_t m = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setMode(m);
    Serial.print(F("Set mode to: "));
    Serial.print(ws2812fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
  }

  else if (cmd.startsWith(F("c "))) {
    uint32_t c = (uint32_t) strtol(&cmd.substring(2, cmd.length())[0], NULL, 16);
    ws2812fx.setColor(c);
    Serial.print(F("Set color to: "));
    Serial.print(F("0x"));
    if (ws2812fx.getColor() < 0x100000) {
      Serial.print(F("0"));
    }
    if (ws2812fx.getColor() < 0x010000)
      Serial.print(F("0"));
    if (ws2812fx.getColor() < 0x001000)
      Serial.print(F("0"));
    if (ws2812fx.getColor() < 0x000100)
      Serial.print(F("0"));
    if (ws2812fx.getColor() < 0x000010)
      Serial.print(F("0"));
    Serial.println(ws2812fx.getColor(), HEX);
  }

  else if (cmd.startsWith(F("stop"))) {
    Serial.println(F("Stopping timer."));
    timer.Stop();
  }

  else if (cmd.startsWith(F("start"))) {
    Serial.println(F("Starting timer."));
    timer.Start();
  }

  cmd = "";              // reset the commandstring
  cmd_complete = false;  // reset command complete
}

/*
   Prints a usage menu.
*/
void printUsage() {
  Serial.println(F("Usage:"));
  Serial.println(F("m <n> \t : select mode <n>"));
  Serial.println();
  Serial.println(F("b+ \t : increase brightness"));
  Serial.println(F("b- \t : decrease brightness"));
  Serial.println(F("b <n> \t : set brightness to <n>"));
  Serial.println();
  Serial.println(F("s+ \t : increase speed"));
  Serial.println(F("s- \t : decrease speed"));
  Serial.println(F("s <n> \t : set speed to <n>"));
  Serial.println();
  Serial.println(F("c 0x007BFF \t : set color to 0x007BFF"));
  Serial.println();
  Serial.println(F("Have a nice day."));
  Serial.println(F("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
}


/*
   Prints all available WS2812FX blinken modes.
*/
void printModes(void)
{
  Serial.println(F("Supporting the following modes: "));
  Serial.println();
  for (int i = 0; i < ws2812fx.getModeCount(); i++) {
    Serial.print(i);
    Serial.print(F("\t"));
    Serial.println(ws2812fx.getModeName(i));
  }
  Serial.println();
}


/*
   Reads new input from serial to cmd string. Command is completed on \n
*/
void serialEvent()
{
#ifdef USE_SERIAL
  while (Serial.available())
  {
    char inChar = (char) Serial.read();
    if (inChar == '\n')
      cmd_complete = true;
    else
      cmd += inChar;
  }

  // ESP8266 doesn't terminate the command with "/n", so manually mark the command complete
#if defined(ESP8266)
  cmd_complete = true;
#endif
#endif
}
