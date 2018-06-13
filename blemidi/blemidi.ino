/*********************************************************************
This is an example for our nRF52 based Bluefruit LE modules

Pick one up today in the adafruit shop!

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in
any redistribution
*********************************************************************/

/* For BLE MIDI Setup
* https://learn.adafruit.com/wireless-untztrument-using-ble-midi/overview
*/


#include <bluefruit.h>
#include <MIDI.h>

// OLED shield headers
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

BLEDis bledis;
BLEMidi blemidi;
Adafruit_SSD1306 display = Adafruit_SSD1306();

#if defined(ARDUINO_FEATHER52)
#define BUTTON_A 31
#define BUTTON_B 30
#define BUTTON_C 27
#define LED 17
#endif

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


// Create a new instance of the Arduino MIDI Library,
// and attach BluefruitLE MIDI as the transport.
MIDI_CREATE_BLE_INSTANCE(blemidi);

// Variable that holds the current position in the sequence.
int position = 0;

// Variable that holds sustain on-off
bool sustainPedal = false;

// Store example melody as an array of note values
//byte note_sequence[] = {
//  74,78,81,86,90,93,98,102,57,61,66,69,73,78,81,85,88,92,97,100,97,92,88,85,81,78,
//  74,69,66,62,57,62,66,69,74,78,81,86,90,93,97,102,97,93,90,85,81,78,73,68,64,61,
//  56,61,64,68,74,78,81,86,90,93,98,102
//};

// Note sequence for Stranger Things theme
byte note_sequence[] = {
  48, 52, 55, 59, 60, 59, 55, 52
};

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 200;           // interval at which to blink (milliseconds)


void setup()
{
  Serial.begin(115200);
  Serial.println("Adafruit Bluefruit52 MIDI over Bluetooth LE Example");

// Config the peripheral connection with maximum bandwidth 
// more SRAM required by SoftDevice
// Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);  

  Bluefruit.begin();
  Bluefruit.setName("DandyDanny BLE MIDI Pedal");
  Bluefruit.setTxPower(4);

// Setup the on board blue LED to be enabled on CONNECT
  Bluefruit.autoConnLed(true);

// Configure and Start Device Information Service
//  bledis.setManufacturer("Adafruit Industries");
//  bledis.setModel("Bluefruit Feather52");
  bledis.setManufacturer("DandyDanny");
  bledis.setModel("DandyDanny Prototype");
  bledis.begin();

// Initialize MIDI, and listen to all MIDI channels
// This will also call blemidi service's begin()
  MIDI.begin(MIDI_CHANNEL_OMNI);

// Attach the handleNoteOn function to the MIDI Library. It will
// be called whenever the Bluefruit receives MIDI Note On messages.
  MIDI.setHandleNoteOn(handleNoteOn);

// Do the same for MIDI Note Off messages.
  MIDI.setHandleNoteOff(handleNoteOff);

// Set up and start advertising
  startAdv();

// Start MIDI read loop
  Scheduler.startLoop(midiRead);
//  Scheduler.startLoop(activityLED);


// OLED setup

  Serial.println("OLED FeatherWing test");
// by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
// init done
Serial.println("OLED begun");

// Show image buffer on the display hardware.
// Since the buffer is intialized with an Adafruit splashscreen
// internally, this will display the splashscreen.
//  display.display();
//  delay(1000);

// Clear the buffer.
display.clearDisplay();
display.display();

Serial.println("IO test");

pinMode(BUTTON_A, INPUT_PULLUP);
pinMode(BUTTON_B, INPUT_PULLUP);
pinMode(BUTTON_C, INPUT_PULLUP);

// text display tests
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(0,0);
display.println("D A N D Y D A N N Y");
display.println("-------------------");
display.println("BLE MIDI PEDAL");
display.println("READY TO CONNECT ->");
display.setCursor(0,0);
display.display(); // actually display all of the above
}

void startAdv(void)
{
// Set General Discoverable Mode flag
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);

// Advertise TX Power
  Bluefruit.Advertising.addTxPower();

// Advertise BLE MIDI Service
  Bluefruit.Advertising.addService(blemidi);

// Secondary Scan Response packet (optional)
// Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();

/* Start Advertising
* - Enable auto advertising if disconnected
* - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
* - Timeout for fast mode is 30 seconds
* - Start(timeout) with timeout = 0 will advertise forever (until connected)
*
* For recommended advertising interval
* https://developer.apple.com/library/content/qa/qa1931/_index.html   
*/
  Bluefruit.Advertising.restartOnDisconnect(true);
Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}
void activityLED()
{
// here is where you'd put code that needs to be running all the time.

// check to see if it's time to blink the LED; that is, if the difference
// between the current time and last time you blinked the LED is bigger than
// the interval at which you want to blink the LED.
//  unsigned long currentMillis = millis();

//  if (currentMillis - previousMillis >= interval) {
//      Serial.printf("LED time exceeded: currentMillis %d, previousMillis %d", currentMillis, previousMillis);
//      Serial.println();
//    // save the last time you blinked the LED
//    previousMillis = currentMillis;
//    
//    // set the LED with the ledState of the variable:
//    digitalWrite(LED_BUILTIN, LOW); 
//  }
//  else {
//      Serial.printf("LED on: currentMillis %d, previousMillis %d", currentMillis, previousMillis);
//      Serial.println();
//    digitalWrite(LED_BUILTIN, HIGH); 
//  }
//
//    digitalWrite(LED_BUILTIN, HIGH); 
//    digitalWrite(LED_BUILTIN, LOW); 
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
// Log when a note is pressed.
  Serial.printf("Note on: channel = %d, pitch = %d, velocity - %d", channel, pitch, velocity);
  Serial.println();
//  activityLED();
//  display.clearDisplay();
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
// Log when a note is released.
  Serial.printf("Note off: channel = %d, pitch = %d, velocity - %d", channel, pitch, velocity);
  Serial.println();
//  activityLED();
//  display.clearDisplay();
}



void loop()
{
// Don't continue if we aren't connected.
  if (! Bluefruit.connected()) {
    return;
  }

// Don't continue if the connected device isn't ready to receive messages.
  if (! blemidi.notifyEnabled()) {
    return;
  }

// Setup variables for the current and previous
// positions in the note sequence.
//  int current = position;
//  int previous  = position - 1;

// If we currently are at position 0, set the
// previous position to the last note in the sequence.
//  if (previous < 0) {
//    previous = sizeof(note_sequence) - 1;
//  }

// Send Note On for current position at full velocity (127) on channel 1.
//  MIDI.sendNoteOn(note_sequence[current], 80, 1);
//  delay(50);

// Send Note Off for previous note.
//  MIDI.sendNoteOff(note_sequence[previous], 0, 1);

// Turn off current note
//  MIDI.sendNoteOff(note_sequence[current], 0, 1);
//  delay(200);



// Increment position
//  position++;

// If we are at the end of the sequence, start over.
//  if (position >= sizeof(note_sequence)) {
//    position = 0;
//    delay(1000);
// Toggle sustain at end of song
    if (sustainPedal == true ) {
//      sustainPedal = false;
//      MIDI.sendControlChange(64, 0, 1);
//      display.clearDisplay();
//      display.display();
//      display.setCursor(0,0);
//      display.println("MIDI sustain off");
//      display.display();
    }
//    else {
//      sustainPedal = true;
//      MIDI.sendControlChange(64, 127, 1);
//      display.clearDisplay();
//      display.display();
//      display.setCursor(0,0);
//      display.println("MIDI sustain on");
//      display.display();
//    }
//  }




// Send sustain
// Usage: send(ControlChange, inControlNumber, inControlValue, inChannel);
    if (sustainPedal == false && ! digitalRead(BUTTON_C)) {
      sustainPedal = true;
//      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      MIDI.sendControlChange(64, 127, 1);
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("MIDI sustain on ");
      display.display();


    } else  if (sustainPedal == true && digitalRead(BUTTON_C)) {
// digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW 
      MIDI.sendControlChange(64, 0, 1);
// turn off all notes
// MIDI.sendControlChange(123, 0, 1);
      sustainPedal = false;
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("MIDI sustain off");
      display.display();
    }


//  if (! digitalRead(BUTTON_A)) display.print("A");
//  if (! digitalRead(BUTTON_B)) display.print("B");
//  if (! digitalRead(BUTTON_C)) display.print("C");
//  delay(10);
//  yield();
//  display.display();

//  delay(220);
}

void midiRead()
{
// Don't continue if we aren't connected.
  if (! Bluefruit.connected()) {
    return;
  }

// Don't continue if the connected device isn't ready to receive messages.
  if (! blemidi.notifyEnabled()) {
    return;
  }

// read any new MIDI messages
  MIDI.read();
}

