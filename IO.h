#ifndef __IO_H__
#define __IO_H__

#include <iarduino_RTC.h>
#include <iarduino_I2C_pH.h>
#include <iarduino_I2C_TDS.h>
#include <iarduino_PCA9555.h>

#define PH_METER_ADDRESS 0x0a
#define TDS_METER_ADDRESS 0x0b
#define RTC_CLOCK_MODEL RTC_RX8025

iarduino_PCA9555 gpio[2]{0x20, 0x21};
iarduino_RTC rtc(RTC_CLOCK_MODEL);
iarduino_I2C_pH ph_meter(PH_METER_ADDRESS);
iarduino_I2C_TDS tds_meter(TDS_METER_ADDRESS);

// outs second_expander
#define BIT_PUMP_A 	0
#define BIT_PUMP_B 	1
#define BIT_PUMP_C 	2
#define BIT_PUMP_D 	3
#define BIT_PUMP_E 	4
#define BIT_M_DIR	5
#define BIT_MOTOR	6
#define BIT_TDS_EN	7

// outs esp32
#define PUMP_F 	17
#define PUMP_G 	18
#define PUMP_H 	19
#define FAN	33
#define LED	32

// esp analog in
#define ANALOG_C 34
#define ANALOG_D 35
#define ANALOG_A 36
#define ANALOG_B 39

// uint16_t pinsIN = buttons.portRead(2);
#define BTN_PREV 	0b00000001
#define BTN_MIN 	0b00000010
#define BTN_OK 		0b00000100
#define BTN_HOME 	0b00001000
#define BTN_NEXT 	0b00010000
#define BTN_PLU 	0b00100000

#define BIT_KEY1	6
#define BIT_KEY2	7
#define BIT_KEY3	8
#define BIT_KEY4	9
#define BIT_KEY5	10
#define BIT_KEY6	11
#define BIT_KEY7	12
#define BIT_KEY8	13
#define BIT_KEY9	14
#define BIT_KEY10	15

class InputOutput {
	public:
		void update()
		{
		}

	private:
		void _readExpanders()
		{
		}

		void _writeExpanders()
		{
		}

		void _readRTC()
		{
		}

		void _setRTC()
		{
		}

		void _readPH()
		{
		}

		void _readTDS()
		{
		}

	private:
		bool _lock = false;
		float _last_ph = 0.0;
		int _last_tds = 0;
		uint16_t _buttons_buffer[10];
} io;

#endif
