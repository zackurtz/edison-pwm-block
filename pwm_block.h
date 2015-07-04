#ifndef PWM_BLOCK_H__

#include "mraa.hpp"

#include <iostream>
#include <unistd.h>
#include <cmath>


namespace pwmblock {

namespace reg {

// Registers
static const uint8_t MODE1 = 0x00;
static const uint8_t MODE2 = 0x01;
static const uint8_t SUBADR1 = 0x02;
static const uint8_t UBADR2 = 0x03;
static const uint8_t SUBADR3 = 0x04;
static const uint8_t PRESCALE = 0xFE;
static const uint8_t LED0_ON_L = 0x06;
static const uint8_t LED0_ON_H = 0x07;
static const uint8_t LED0_OFF_L = 0x08;
static const uint8_t LED0_OFF_H = 0x09;
static const uint8_t ALL_LED_ON_L = 0xFA;
static const uint8_t ALL_LED_ON_H = 0xFB;
static const uint8_t ALL_LED_OFF_L = 0xFC;
static const uint8_t ALL_LED_OFF_H = 0xFD;

uint8_t LED_ON_L(uint8_t channel) {
	return LED0_ON_L + channel*4;
}

uint8_t LED_ON_H(uint8_t channel) {
	return LED0_ON_H + channel*4;
}

uint8_t LED_OFF_L(uint8_t channel) {
	return LED0_OFF_L + channel*4;
}

uint8_t LED_OFF_H(uint8_t channel) {
	return LED0_OFF_H + channel*4;
}

};

// Bits
namespace bit {
static const uint8_t RESTART = 0x80;
static const uint8_t SLEEP = 0x10;
static const uint8_t ALLCALL = 0x01;
static const uint8_t INV = 0x10;
static const uint8_t OUTDRV = 0x04;
};

class pwmblock {
private:
	mraa::I2c* i2c_;
	uint8_t address_;
	double freq_hz_;
	double period_us_;

	uint16_t on_;
	uint16_t off_;


	void write_byte(uint8_t reg, uint8_t byte) {
		uint8_t buf[2];
		buf[0] = reg;
		buf[1] = byte;

		i2c_->address(address_);
		i2c_->write(buf, 2);
	}

	uint8_t read_byte(uint8_t reg) {
		i2c_->address(address_);
		uint8_t val = i2c_->readReg(reg);
		return val;
	}

public:
	static const int OscClock = 25000000.0;
	static const float DefaultFreqHz = 100;
	static const int EdisonBus = 1;

	pwmblock(uint8_t address=0x40) {
		i2c_ = new mraa::I2c(EdisonBus);
		address_ = address;
		freq_hz_ = DefaultFreqHz;

		clear();

		// Configures for controlling RC Servos (From VCC power).
		write_byte(reg::MODE2, 0 & ~bit::OUTDRV);
		usleep(5000);

		set_sleep(false);

		setFreq(freq_hz_);
	}

	~pwmblock() {
		clear();
		set_sleep(true);
		delete i2c_;
	}

	void set_sleep(bool sleep_on) {
		uint8_t mode1 = read_byte(reg::MODE1);

		if (sleep_on) {
			mode1 = mode1 | bit::SLEEP;
		} else {
			mode1 = mode1 & ~bit::SLEEP;
		}
		write_byte(reg::MODE1, mode1);
		usleep(5000);
	}

	void clear() {
		write_byte(reg::ALL_LED_OFF_H, 0x10);
	}

	// Set the pulse frequency
	void setFreq(double freq_hz) {
		freq_hz_ = freq_hz;
		period_us_= 1000000.0/freq_hz_;

		float prescalf = (OscClock)/(4096.0f * freq_hz_) - 1;
		uint8_t prescale = (prescalf + 0.5);

		// Must sleep before changing frequency
		uint8_t mode1 = read_byte(reg::MODE1);
		write_byte(reg::MODE1, mode1 | bit::SLEEP);

		write_byte(reg::PRESCALE, prescale);

		// Clear sleep bit
		write_byte(reg::MODE1, mode1 | ~bit::SLEEP);
		usleep(5000);

		write_byte(reg::MODE1, mode1 | bit::RESTART);
	}

	// Sets the pulse rising edge position and falling edge position in counts
	void setPWOnOff(uint8_t channel, uint16_t on_counts, uint16_t off_count) {
		write_byte(reg::LED_ON_L(channel), on_counts & 0xFF);
		write_byte(reg::LED_ON_H(channel), on_counts >> 8);
		write_byte(reg::LED_OFF_L(channel), off_count & 0xFF);
		write_byte(reg::LED_OFF_H(channel), off_count >> 8);
	}

	// Sets the pulse width in microseconds
	void setPW(uint8_t channel, double on_us) {
		uint16_t counts_len = 4096.0*(on_us/period_us_);;
		setPWOnOff(channel, 0, counts_len);
	}

	// Sets the pulse percentage high each cycle
	void setPercentOn(int channel, double percent) {
		setPW(channel, percent * period_us_);
	}
};

};

#endif
