// VelociNote Air
// Bluetooth Low Energy Note Trigger Pedal
// @dandydanny
// June 18, 2018

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

// Create a new instance of the Arduino MIDI Library,
// and attach BluefruitLE MIDI as the transport.
MIDI_CREATE_BLE_INSTANCE(blemidi);

// Time and switch state variables
unsigned long timeBegin = 0;
unsigned long timeEnd = 0;
unsigned long timeDiff = 0;
int sw1state = 0;
int sw2state = 0;
int midiValue;
int midiNote = 38; //D1

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("VelociNote Air");

  // Config the peripheral connection with maximum bandwidth
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);
  Bluefruit.begin();
  Bluefruit.setName("VelociNote Air");
  Bluefruit.setTxPower(4);

  // Setup the on board blue LED to be enabled on CONNECT
  Bluefruit.autoConnLed(true);

  // Configure and Start Device Information Service
  bledis.setManufacturer("@DandyDanny");
  bledis.setModel("VNx`-1");
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
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  // Display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("VelociNote Air");
  display.display(); // actually display all of the above
  delay(1000);
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

void printSwitchState() {
//  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(digitalRead(2));
  display.print(" ");
  display.print(digitalRead(3));
  display.display();
}

void printTimeVars() {
  display.setCursor(0, 8);
  display.print("Begin: ");
  display.print(timeBegin);
  display.println();
  display.print("End  : ");
  display.print(timeEnd);
  display.display();
}

void printDiff() {
  midiValue = map(timeDiff, 60, 10, 0, 127);
  midiValue = constrain(midiValue, 20, 127);
  MIDI.sendNoteOn(midiNote, midiValue, 1);
  display.setCursor(0, 24);
  display.print("Diff: ");
  display.print(timeDiff);
  display.print(" Velocity: ");
  display.print(midiValue);
  display.display();
  
}

void loop() {
  display.clearDisplay();

  // If switch is at home position, do nothing
  while (! digitalRead(2)) {
//    printSwitchState();
//    printTimeVars();
    // Read potentiometer and set MIDI note to be triggered
    
    midiNote = map(analogRead(4), 40, 875, 21, 108);
    midiNote = constrain(midiNote, 21, 108);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("MIDI Note: ");
    display.print(midiNote);
    display.display();
  }
  
  // If switch leaves resting position, store starting time
  if (digitalRead(2)) {
    timeBegin = millis();
//    printSwitchState();
//    printTimeVars();
  }

  // Until switch has reach down position, do nothing
  while (digitalRead(3)) {
//    printSwitchState();
//    printTimeVars();
  }
  
  // When switch reach down position, store end time 
  if (! digitalRead(3)) {
    timeEnd = millis();
    timeDiff = timeEnd - timeBegin;
//    printSwitchState();
    printDiff();
  }

  // While sw1 is high (not in home position), halt program loop
  while (digitalRead(2)) {
//    printSwitchState();
//    printTimeVars();
  }
  // Send Note Off for previous note.
  MIDI.sendNoteOff(midiNote, 0, 1);
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
