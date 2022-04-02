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
 * @copyright Copyright (c) 2022 Zerui An
 * 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License,or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>. 
 * 
 */

#include <bluefruit.h>
#include "RotaryEncoderD.h"

const uint8_t KEY_MUTE        = 0x7f; // Keyboard Mute
const uint8_t KEY_VOLUMEUP    = 0x80; // Keyboard Volume Up
const uint8_t KEY_VOLUMEDOWN  = 0x81; // Keyboard Volume Down

const uint8_t CC_MUTE       = 0xe2; // Consumer Control Mute
const uint8_t CC_VOLUMEUP   = 0xe9; // Consumer Control Up
const uint8_t CC_VOLUMEDOWN = 0xea; // Consumer Control Down

const uint8_t CLK_PIN = 0;
const uint8_t DT_PIN  = 1;
const uint8_t SW_PIN  = 2;

// As far as I know, either one of these functions would work.
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
  Serial.println("Go to your PC or phone's Bluetooth settings to pair your device");

  // Initialize the encoder
  encoder.begin();
  
  // configure the switch as input
  pinMode(SW_PIN, INPUT);


  Bluefruit.begin(1, 0);      // one peripheral
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  // Configure and Start Device Information Service
  bledis.setManufacturer("Seeed Studio");
  bledis.setModel("Seeeduino XIAO BLE");
  bledis.begin();

  // Start BLE HID
  blehid.begin();

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
   * - Start(timeout) with timeout = 0 will advertise forever (until connected).
   *   Otherwise it will advertise for (x * 10) ms, then stop advertising
   *   and invoke the stop callback function (if set);
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);
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
