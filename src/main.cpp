/**
 * Program      CLI Menu
 * Author       2024-10-24 Charles Geiser (https://www.dodeka.ch)
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
 * Board        ESP32
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
// followed by another carriage return to reposition the cursor on line beginning
#define CLEAR_LINE     Serial.printf("\r%*c\r", 80, ' ')

// Definition of the action and the menuitem
using Action   = void(&)(const char*);
using MenuItem = struct mi{ const char key; const char* txt; const char* arg; Action action; };


bool heartbeatEnabled = true;

// Forward declaration of menu actions
void enterFloat(const char*);
void enterInteger(const char*);
void enterString(const char*);
void playRadio(const char* url);
void setDateTime(const char*);
void sayHello(const char*);
void showDateTime(const char*);
void showMenu(const char*);
void toggleHeartbeat(const char*);


// Menu definition
// Each menuitem is composed of a key, a text, an actionargument and an action
MenuItem menu[] = 
{
  { '0', "[0] Klassik Radio",    "http://stream.klassikradio.de/live/mp3-128/stream.klassikradio.de", playRadio },
  { '1', "[1] SRF1 AG-SO",       "http://stream.srg-ssr.ch/m/regi_ag_so/mp3_128", playRadio },
  { '2', "[2] SRF2",             "http://stream.srg-ssr.ch/m/drs2/mp3_128", playRadio },
  { '3', "[3] SRF3",             "http://stream.srg-ssr.ch/m/drs3/mp3_128", playRadio },  
  { 'h', "[h] Say Hello",        "Guten Tag", sayHello },
  { 'd', "[d] Set date and time as: yyyy mm dd hh mm ss", "", setDateTime },
  { 'D', "[D] Show date and time", "", showDateTime },
  { 'i', "[i] Enter an integer",   "", enterInteger },
  { 'f', "[f] Enter a float",      "", enterFloat },
  { 's', "[s] Enter a string",     "", enterString },
  { 't', "[t] Toggle heartbeat",   "", toggleHeartbeat },
  { 'S', "[S] Show menu",          "", showMenu },
};
constexpr uint8_t nbrMenuItems = sizeof(menu) / sizeof(menu[0]);


void setDateTime(const char* txt)
{
 // enter time as: yyyy mo dd hh mm ss
  tm time;
  int y, m, mday;
  timeval sec_musec;
  String str;
  uint32_t timeout = Serial.getTimeout(); // get the default timeout

  delay(2000);
  Serial.setTimeout(3 * timeout); // extend timeout to have more time for entering numbers
  while (Serial.available())
  {
    str = Serial.readString();
  }
  Serial.setTimeout(timeout); // restore default timeout
  sscanf(str.c_str(), "%4d%*c%2d%*c%2d%*c%2d%*c%2d%*c%2d", &time.tm_year, &time.tm_mon, &time.tm_mday, 
                                                           &time.tm_hour, &time.tm_min, &time.tm_sec);
  time.tm_mon  -= 1;
  time.tm_year -= 1900;

  sec_musec.tv_sec = mktime(&time);
  sec_musec.tv_usec= 0;
  settimeofday(&sec_musec, NULL); // Set the internal RTC of the ESP32
  showDateTime("");
}

void showDateTime(const char* txt)
{
  tm   rtcTime;
  char buf[60];
  int  bufSize = sizeof(buf);

  getLocalTime(&rtcTime);
  strftime(buf, bufSize, "%B %d %Y %T (%A)",  &rtcTime);
  Serial.printf("%s", buf);
}


void playRadio(const char* url)
{
  Serial.printf("Playing: %s", url);
}


/**
 * Greet the user
 */
void sayHello(const char* txt)
{
  Serial.print(txt);
}


/**
 * Ask a integer from user
 */
void enterInteger(const char* txt)
{
  char buf[32];
  int32_t value = 0;

  delay(2000);
  while (Serial.available())
  {
    value = Serial.parseInt();
  }
  snprintf(buf, sizeof(buf), "%d was entered ", value);
  Serial.print(buf);
}


/**
 * Ask a float from user
 */
void enterFloat(const char* txt)
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
void enterString(const char* txt)
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
void toggleHeartbeat(const char* txt)
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
void showMenu(const char* txt)
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
  CLEAR_LINE;

  for (int i = 0; i < nbrMenuItems; i++)
  {
    if (key == menu[i].key)
    {
      menu[i].action(menu[i].arg);
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
  showMenu("");
}


void loop() 
{
  // handle the menu
  if(Serial.available()) doMenu();
  
  // illustrates the blocking behavior when entering numbers and text
  if (heartbeatEnabled) heartbeat(LED_BUILTIN, 1000, 20);
}