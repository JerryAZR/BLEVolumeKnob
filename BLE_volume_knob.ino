/**
 * @file BLE_volume_knob.ino
 * @author Zerui An (anzerui@126.com / jerryazr@gmail.com)
 * 
 * @brief This is an example of a BLE HID volume knob. An nRF52 based
 * Bluefruit LE modules is required for this example.
 * 
 * This file is based on the hid_keyboard example from the bluefruit library.
 * 
 * @version 0.1
 * @date 2022-03-31
 * 
 * @copyright Copyright (c) 2022
 * 
 */

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
#include <bluefruit.h>
#include <RotaryEncoderD.h>

const uint8_t KEY_MUTE        = 0x7f; // Keyboard Mute
const uint8_t KEY_VOLUMEUP    = 0x80; // Keyboard Volume Up
const uint8_t KEY_VOLUMEDOWN  = 0x81; // Keyboard Volume Down

const uint8_t CC_MUTE       = 0xe2; // Consumer Control Mute
const uint8_t CC_VOLUMEUP   = 0xe9; // Consumer Control Up
const uint8_t CC_VOLUMEDOWN = 0xea; // Consumer Control Down

const uint8_t CLK_PIN = 0;
const uint8_t DT_PIN  = 1;
const uint8_t SW_PIN  = 2;

void keyTap(BLEHidAdafruit& hid, uint8_t keyCode);
void CCTap(BLEHidAdafruit& hid, uint8_t usageCode);

BLEDis bledis;
BLEHidAdafruit blehid;
RotaryEncoderD<CLK_PIN, DT_PIN> encoder;

void setup() 
{
  Serial.begin(9600);

  Serial.println("Bluefruit52 HID Volume Control Example");
  Serial.println("--------------------------------\n");

  Serial.println();
  Serial.println("Go to your phone's Bluetooth settings to pair your device");

  // Initialize the encoder
  encoder.begin();
  
  // configure the encoder pins as input
  // pinMode(CLK_PIN, INPUT);
  // pinMode(DT_PIN, INPUT);
  pinMode(SW_PIN, INPUT);


  Bluefruit.begin(1, 0);      // one peripheral
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("Seeed Studio");
  bledis.setModel("Seeeduino XIAO BLE");
  bledis.begin();

  /* Start BLE HID
   * Note: Apple requires BLE device must have min connection interval >= 20m
   * ( The smaller the connection interval the faster we could send data).
   * However for HID and MIDI device, Apple could accept min connection interval 
   * up to 11.25 ms. Therefore BLEHidAdafruit::begin() will try to set the min and max
   * connection interval to 11.25  ms and 15 ms respectively for best performance.
   */
  blehid.begin();

  /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms 
   */
  /* Bluefruit.Periph.setConnInterval(9, 12); */

  // Set up and start advertising
  startAdv();
}

void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_GENERIC_HID);
  
  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.setName("BLE Volume Knob");
  Bluefruit.Advertising.addName();
  
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

void loop() 
{
  if (Bluefruit.Periph.connected()) {
    // read encoder input
    int option = encoder.read();
    if (option == encoder.FORWARD) {
      CCTap(blehid, CC_VOLUMEUP);
    } else if (option == encoder.BACKWARD) {
      CCTap(blehid, CC_VOLUMEDOWN);
    }
    // read switch input
    if (digitalRead(SW_PIN) == 0) {
      CCTap(blehid, CC_MUTE);

      // debounce
      delay(50);
      while (digitalRead(SW_PIN) == 0);
      delay(50);
    }
  }
}

void keyTap(BLEHidAdafruit& hid, uint8_t keyCode) {
  static uint8_t keycodes[6] = {0};
  keycodes[0] = keyCode;
  hid.keyboardReport(0, keycodes);
  hid.keyRelease();
}

void CCTap(BLEHidAdafruit& hid, uint8_t usageCode) {
  hid.consumerKeyPress(usageCode);
  hid.consumerKeyRelease();
}
