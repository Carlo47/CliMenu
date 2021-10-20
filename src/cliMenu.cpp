/**
 * Program      cliMenu.cpp
 * Author       2021-06-05 Charles Geiser (https://www.dodeka.ch)
 * 
 * Purpose      A simple, but well structured menu for the command line
 *              The user can trigger commands and enter 
 *                - integers
 *                - floats
 *                - text
 *              Numbers are parsed into variables of type integer or float. 
 *              Reading of keystrokes ends when the time between keystrokes is
 *              greater than the time set in milliseconds with Serial.setTimeout(ms).
 *              The default timeout is 1000 ms
 * 
 * Board        Arduino Uno, Wemos D1 R2, ESP32
 *
 * Caveats      To use snprintf() with floats on Arduino we have to define build flags (see platformio.ini):
 *              build_flags = -Wl,-u,vfprintf -lprintf_flt -lm
 *
 * Remarks      Pros: + Simple and well structured
 *                    + Easy to understand
 *                    + Input of integers, floats and text
 *                    + Execut user defined actions
 *              Cons: - The input is hidden, which means that the entered value can  
 *                      only be displayed after the timeout of the serial interface.
 *                    - The main loop is blocked as long as numbers or text are entered.      
 *
 * References   https://www.arduino.cc/reference/en/language/functions/communication/serial/
 */

#include <Arduino.h>

// Clear the current line with a carriage return, then printing 80 blanks 
// followed by another carriage return to position the cursor on line beginning
#define CLR_LINE    "\r                                                                                \r"
//#define CLR_LINE "\r%*c\r", 80, ' '   // possible for printf() on ESP32

// Definition of a menuitem
typedef struct { const char key; const char *txt; void (&action)(); } MenuItem;

bool heartbeatEnabled = true;

// Forward declaration of menu actions
void sayHello();
void enterInteger();
void enterFloat();
void enterString();
void toggleHeartbeat();
void showMenu();

// Menu definition
// Each menuitem is composed of a key, a text and an action
MenuItem menu[] = 
{
  { 'h', "[h] Say Hello",        sayHello },
  { 'i', "[i] Enter an integer", enterInteger },
  { 'f', "[f] Enter a float",    enterFloat },
  { 's', "[s] Enter a string",   enterString },
  { 't', "[t] Toggle heartbeat", toggleHeartbeat },
  { 'S', "[S] Show menu",        showMenu },
};

constexpr uint8_t nbrMenuItems = sizeof(menu) / sizeof(menu[0]);

/**
 * Greet the user
 */
void sayHello()
{
  Serial.print("Hello World ");
}

/**
 * Ask a integer from user
 */
void enterInteger()
{
  char buf[32];
  int32_t value = 0;

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseInt();
  }
  snprintf(buf, sizeof(buf), "%ld was entered ", value);
  Serial.print(buf);
}

/**
 * Ask a float from user
 */
void enterFloat()
{
  char buf[32];
  double value = 0;

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseFloat();
  }
  snprintf(buf, sizeof(buf), "%f was entered ", value);
  Serial.print(buf);  
}

/**
 * Ask a string from user
 */
void enterString()
{
  String str;
  
  delay(2000);
  while (Serial.available())
  {
    str = Serial.readString();
  }
  Serial.print(str);
}

/**
 * Turn on or off flashing led
 */
void toggleHeartbeat()
{
  heartbeatEnabled = !heartbeatEnabled;
  if (heartbeatEnabled)
    Serial.print("Heartbeat on ");
  else
    Serial.print("Heartbeat off ");
}

/**
 * Display menu on monitor
 */
void showMenu()
{
  // title is packed into a raw string
  Serial.print(
  R"TITLE(
---------------
 CLI Menu Demo 
---------------
)TITLE");

  for (int i = 0; i < nbrMenuItems; i++)
  {
    Serial.println(menu[i].txt);
  }
  Serial.print("\nPress a key: ");
}

/**
 * Execute the action assigned to the key
 */
void doMenu()
{
  char key = Serial.read();
  Serial.print(CLR_LINE);
  for (int i = 0; i < nbrMenuItems; i++)
  {
    if (key == menu[i].key)
    {
      menu[i].action();
      break;
    }
  } 
}

/**
 * Flash the led on pin with period and pulse width
 */
void heartbeat(uint8_t pin, uint32_t period, uint32_t pulseWidth)
{
  digitalWrite(pin, millis() % period < pulseWidth ? HIGH : LOW);
}

void setup() 
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  showMenu();
}

void loop() 
{
  // handle the menu
  if(Serial.available()) doMenu();
  
  // illustrates the blocking behavior when entering numbers and text
  if (heartbeatEnabled) heartbeat(LED_BUILTIN, 1000, 20);
}