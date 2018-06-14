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
  https://learn.adafruit.com/wireless-untztrument-using-ble-midi/overview
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

// VR values
float x;
float y;
float prev_x;
float prev_y;
float jitterAllowance = 7;

// Variable that holds sustain on-off
bool sustainPedal = false;

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


  // OLED setup
  Serial.println("DandyDanny BLE MIDI Pedal Prototype");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");

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
  display.setCursor(0, 0);
  display.println("D A N D Y D A N N Y");
  display.println("-------------------");
  display.println("BLE MIDI PEDAL");
  display.println("READY TO CONNECT ->");
  display.setCursor(0, 0);
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
    - Enable auto advertising if disconnected
    - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
    - Timeout for fast mode is 30 seconds
    - Start(timeout) with timeout = 0 will advertise forever (until connected)

    For recommended advertising interval
    https://developer.apple.com/library/content/qa/qa1931/_index.html
  */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  // Log when a note is pressed.
  Serial.printf("Note on: channel = %d, pitch = %d, velocity - %d", channel, pitch, velocity);
  Serial.println();
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  // Log when a note is released.
  Serial.printf("Note off: channel = %d, pitch = %d, velocity - %d", channel, pitch, velocity);
  Serial.println();
}

void loop()
{
//  // Don't continue if we aren't connected.
//  if (! Bluefruit.connected()) {
//    display.clearDisplay();
//    display.setCursor(0, 0);
//    display.println("Ready to connect");
//    display.display();
//    return;
//  } else
//  // Don't continue if the connected device isn't ready to receive messages.
//  if (! blemidi.notifyEnabled()) {
//    display.clearDisplay();
//    display.setCursor(0, 0);
//    display.println("Host not ready");
//    display.display();
//    return;
//  } else {
//    display.clearDisplay();
//    display.setCursor(0, 0);
//    display.println("Ready to send");
//    display.display();
//  }

  // Read current value
  x = analogRead(2);
  y = analogRead(3);

  // If current value is changed above jitter allowance, send
  if (abs(x - prev_x) > jitterAllowance) {
    // x value 0-999, middle value ~465, mod wheel value 0-127
    MIDI.sendControlChange(1, (x-465) *.2953, 1);
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("X-VALUE: %f", x);
  display.println();
  display.printf("Y-VALUE: %.0f", y);
  display.display();

  // Remember current value for next comparison
  prev_x = x;
  prev_y = y;

//  delay(100);
  // Send sustain
  if (sustainPedal == false && ! digitalRead(BUTTON_C)) {
    sustainPedal = true;
    MIDI.sendControlChange(64, 127, 1);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MIDI sustain on ");
    display.display();
  } else if (sustainPedal == true && digitalRead(BUTTON_C)) {
    MIDI.sendControlChange(64, 0, 1);
    sustainPedal = false;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MIDI sustain off");
    display.display();
  }
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

