/*
 * SPDX-License-Identifier: MIT
 * Author: Jorik Cronenberg
 *
 * ESP32 Logitech Shifter
 * Has 3 modes: H-Mode, Sequential and Handbrake
 * Modes can be changed by quickly pressing the shifter down twice.
 *
 * H-Mode behaves like the standard logitech shifter.
 * Gears 1-6 are assigned buttons 1-6.
 * Reverse gear can be activated by pressing down and selecting 6th gear.
 * Reverse gear is mapped to button 7
 *
 * Sequential only differentiates between up and down state.
 * Left and right movement has no effect.
 * Gear up is assigned button 8, Gear down is assigned button 9.
 *
 * Handbrake mode only checks for the down state.
 * If the shifter is down, button 10 is pressed.
 */

#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
#else

#include <Arduino.h>
#include "USB.h"
#include "USBHIDGamepad.h"

#define DEBUG 0

USBHIDGamepad gamepad;

/*
 * Values I measured:
 * neutral:
 * horiz: ~1725
 * vert: ~1850
 * button: 1
 *
 * vert down: ~3670
 * vert up: ~300
 *
 * horiz left: ~2540
 * horiz right: ~1060
 */

// Thresholds
const int THRESH_DOWN = 3000;
const int THRESH_UP = 1000;
const int THRESH_LEFT = 2000;
const int THRESH_RIGHT = 1300;

// Modes
typedef enum {
	H_MODE,
	SEQUENTIAL,
	HANDBRAKE,
} modes_t;

modes_t next_mode(modes_t m) {
	return (modes_t)((m + 1) % 3);
}

// Button constants
const int SEQ_UP_BUTTON = 7;
const int SEQ_DOWN_BUTTON = 8;
const int HANDBRAKE_BUTTON = 9;

// GPIO pins
const int VERT_PIN = 6; // Brown wire
const int HORIZ_PIN = 4; // Red wire
const int BUTTON_PIN = 15; // Orange wire

// Time window for double press mode change
const unsigned long DOUBLE_PRESS_WINDOW = 400;

void setup() {
	pinMode(BUTTON_PIN, INPUT_PULLUP);

	// Set device name
	USB.productName("ESP32 Shifter");
	USB.manufacturerName("Jorik");

	// Set up usb gamepad
	gamepad.begin();
	USB.begin();

#if DEBUG
	Serial.begin(115200);
#endif
}

// Potentiometer values
int horiz_value = 0;
int vert_value = 0;

// Button value and double press handlers
bool button_value = LOW;
bool last_button_state = LOW;
unsigned long last_press_time = 0;
bool waiting_for_second_press = false;

// State variables
int cur_button = -1;
modes_t cur_mode = H_MODE;

void change_button(int new_button) {
	if (new_button == cur_button) {
		return;
	}
	if (cur_button != -1) {
		gamepad.releaseButton(cur_button);
	}
	cur_button = new_button;
	if (new_button != -1) {
		gamepad.pressButton(new_button);
	}
}

void loop() {
	horiz_value = analogRead(HORIZ_PIN);
	vert_value = analogRead(VERT_PIN);
	button_value = digitalRead(BUTTON_PIN);

#if DEBUG
	Serial.printf("Horizontal: %d, Vertical: %d, Button: %d\n", horiz_value, vert_value, button_value);
#endif

	// Mode change handling
	// On button press (up, so it hopefully doesn't happen by accident)
	if (button_value != last_button_state && button_value == HIGH) {
		if (waiting_for_second_press) {
			unsigned long now = millis();

			if ((now - last_press_time) <= DOUBLE_PRESS_WINDOW) {
				cur_mode = next_mode(cur_mode);
			}
		} else {
			last_press_time = millis();
			waiting_for_second_press = true;
		}
	}
	// Clear flag if time window has passed
	if (waiting_for_second_press && (millis() - last_press_time > DOUBLE_PRESS_WINDOW)) {
		waiting_for_second_press = false;
	}
	last_button_state = button_value;

	// Handle gears
	if (vert_value < THRESH_UP) {
		if (cur_mode == H_MODE) {
			if (horiz_value > THRESH_LEFT) {
				change_button(0);
			} else if (horiz_value < THRESH_RIGHT) {
				change_button(4);
			} else {
				change_button(2);
			}
		} else if (cur_mode == SEQUENTIAL) {
			change_button(SEQ_UP_BUTTON);
		}
	} else if (vert_value > THRESH_DOWN) {
		if (cur_mode == H_MODE) {
			if (horiz_value > THRESH_LEFT) {
				change_button(1);
			} else if (horiz_value < THRESH_RIGHT) {
				if (button_value == LOW) {
					change_button(6);
				} else {
					change_button(5);
				}
			} else {
				change_button(3);
			}
		} else if (cur_mode == SEQUENTIAL) {
			change_button(SEQ_DOWN_BUTTON);
		} else {
			change_button(HANDBRAKE_BUTTON);
		}
	} else {
		// Neutral
		change_button(-1);
	}
	delay(25);
}
#endif /* ARDUINO_USB_MODE */
