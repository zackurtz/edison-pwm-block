#include "mraa.hpp"

#include <iostream>
#include <unistd.h>

#include "pwm_block.h"

const int us_per_ms = 1000;


int main() {
	pwmblock::pwmblock servo_ctrl;
	servo_ctrl.setFreq(100);

	float pulse_width_ms = 1.65;
	float lower_threshold = 1.1;
	float upper_threshold = 2.1;

	bool up_dir = true;
	
	for(;;) {
		// Update 
		servo_ctrl.setPW(0, pulse_width_ms * us_per_ms);
		usleep(50000);

		// Increment or decrement depending on direction
		if(up_dir) {
			pulse_width_ms += 0.02;
		} else {
			pulse_width_ms -= 0.02;
		}

		// Change direction at thresholds
		if(pulse_width_ms > upper_threshold) {
			up_dir = false;
		} else if(pulse_width_ms < lower_threshold) {
			up_dir = true;
		}
	}

	return MRAA_SUCCESS;
}
