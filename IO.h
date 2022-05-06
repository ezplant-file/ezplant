/*******************************************************************************
  All io of the device should be here. TODO: move all i2c stuff here
*******************************************************************************/
#ifndef __IO_H__
#define __IO_H__

#include <iarduino_RTC.h>
#include <iarduino_I2C_pH.h>
#include <iarduino_I2C_TDS.h>
#include <iarduino_I2C_SHT.h>
#include <iarduino_PCA9555.h>
#include <FunctionalInterrupt.h>

#define PH_METER_ADDRESS 0x0a
#define TDS_METER_ADDRESS 0x0b
#define SHT_METER_ADDRESS 0x0c
#define RTC_CLOCK_MODEL RTC_RX8025

// interrupt pin
#define EXPANDER_INT	27

#define MOTOR_SW_DELAY 25

iarduino_PCA9555 gpio[2]{0x20, 0x21};
iarduino_RTC rtc(RTC_CLOCK_MODEL);
iarduino_I2C_pH ph_meter(PH_METER_ADDRESS);
iarduino_I2C_TDS tds_meter(TDS_METER_ADDRESS);
iarduino_I2C_SHT sht_meter(SHT_METER_ADDRESS);

// analog inputs
#define N_ADC 4

// outs second_expander
#define BIT_PORT_A 	0
#define BIT_PORT_B 	1
#define BIT_PORT_C 	2
#define BIT_PORT_D 	3
#define BIT_PORT_E 	4
#define BIT_M_DIR	5
#define BIT_MOTOR	6
#define BIT_TDS_EN	7

// outs esp32
#define PORT_F 	17
#define PORT_G 	18
#define PORT_H 	19
#define FAN	33
#define LED	32

// esp analog in pins
#define PIN_ANALOG_A 36
#define PIN_ANALOG_B 39
#define PIN_ANALOG_C 34
#define PIN_ANALOG_D 35

// uint16_t pinsIN = buttons.portRead(2);
// bitmasks (TODO: rename)
#define BTN_PREV 	0b00000001
#define BTN_MIN 	0b00000010
#define BTN_OK 		0b00000100
#define BTN_HOME 	0b00001000
#define BTN_NEXT 	0b00010000
#define BTN_PLU 	0b00100000

// keys start bit
#define KEYS_START_BIT 6

// second half of first expander
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

enum {
	FIRST_EXPANDER,
	SECOND_EXPANDER
};

enum {
	FIRST_PORT,
	SECOND_PORT,
	BOTH_PORTS
};

enum {
	ADC_1,
	ADC_2,
	ADC_3,
	ADC_4
};

// digital inputs bits (position matter!)
enum {
	DIG_KEY1,
	DIG_KEY2,
	DIG_KEY3,
	DIG_KEY4,
	DIG_KEY5,
	DIG_KEY6,
	DIG_KEY7,
	DIG_KEY8,
	DIG_KEY9,
	DIG_KEY10,
	DIG_NKEYS
};

// power diag page items bits (position matter!)
enum {
	PWR_PG_PORT_A,
	PWR_PG_PORT_B,
	PWR_PG_PORT_C,
	PWR_PG_PORT_D,
	PWR_PG_PORT_E,
	PWR_PG_PORT_F,
	PWR_PG_PORT_G,
	PWR_PG_PORT_H,
	PWR_PG_FAN,
	PWR_PG_LIGHT,
	PWR_PG_UP,
	PWR_PG_DOWN,
	PWR_PG_NITEMS
};

// user keys bits (position matter!)
enum {
	UI_BACK,
	UI_MINUS,
	UI_OK,
	UI_HOME,
	UI_FORW,
	UI_PLUS,
	UI_NKEYS
};


class InputOutput {
	public:
		bool update()
		{
			if (_pressed) {
				readDigital();
				_pressed = false;
			}

			for (auto& i:_ui_keys) {
				if (i) {
					return true;
				}
			}

			return false;
		}

		void ARDUINO_ISR_ATTR isr()
		{
			_pressed = true;
		}

		void init()
		{
			// sht
			sht_meter.begin();
			// expander stuff
			gpio[FIRST_EXPANDER].begin();
			gpio[SECOND_EXPANDER].begin();
			gpio[FIRST_EXPANDER].portMode(BOTH_PORTS, pinsAll(INPUT));
			gpio[SECOND_EXPANDER].portMode(SECOND_PORT, pinsAll(INPUT));
			gpio[SECOND_EXPANDER].portMode(FIRST_PORT, pinsAll(OUTPUT));
			gpio[SECOND_EXPANDER].portWrite(FIRST_PORT, pinsAll(LOW));
			pinMode(EXPANDER_INT, INPUT_PULLUP);
			attachInterrupt(EXPANDER_INT, std::bind(&InputOutput::isr, this), FALLING);
			pinMode(PORT_F, OUTPUT);
			pinMode(PORT_G, OUTPUT);
			pinMode(PORT_H, OUTPUT);
			pinMode(LED, OUTPUT);
			pinMode(FAN, OUTPUT);
		}

		float getTem()
		{
			return sht_meter.getTem();
		}

		float getHum() {
			return sht_meter.getHum();
		}

		void drivePWMout(int id, uint8_t pwm)
		{
			if (id < PWR_PG_PORT_F || id > PWR_PG_LIGHT)
				return;

			switch (id) {
				default: break;
				case PWR_PG_PORT_F: analogWrite(PORT_F, pwm); break;
				case PWR_PG_PORT_G: analogWrite(PORT_G, pwm); break;
				case PWR_PG_PORT_H: analogWrite(PORT_H, pwm); break;
				case PWR_PG_FAN: analogWrite(FAN, pwm);
				case PWR_PG_LIGHT: analogWrite(LED, pwm);
			}
		}

		void haltPWMout(int id)
		{
			if (id < PWR_PG_PORT_F || id > PWR_PG_LIGHT)
				return;

			switch (id) {
				default: break;
				case PWR_PG_PORT_F: digitalWrite(PORT_F, LOW); break;
				case PWR_PG_PORT_G: digitalWrite(PORT_G, LOW); break;
				case PWR_PG_PORT_H: digitalWrite(PORT_H, LOW); break;
				case PWR_PG_FAN: digitalWrite(FAN, LOW); break;
				case PWR_PG_LIGHT: digitalWrite(LED, LOW); break;
			}
		}

		void haltAll()
		{
			for (int i = 0; i < PWR_PG_NITEMS; i++) {
				driveOut(i, LOW);
			}
		}

		void driveOut(int id, bool state)
		{
			if (id < 0)
				return;

			switch (id) {
				default: break;
				case PWR_PG_PORT_A:
				case PWR_PG_PORT_B:
				case PWR_PG_PORT_C:
				case PWR_PG_PORT_D:
				case PWR_PG_PORT_E:
					gpio[SECOND_EXPANDER].digitalWrite(id, state);
					break;
				case PWR_PG_PORT_F:
					digitalWrite(PORT_F, state);
					break;
				case PWR_PG_PORT_G:
					digitalWrite(PORT_G, state);
					break;
				case PWR_PG_PORT_H:
					digitalWrite(PORT_H, state);
					break;
				case PWR_PG_FAN:
					digitalWrite(FAN, state);
					break;
				case PWR_PG_LIGHT:
					digitalWrite(LED, state);
					break;
				case PWR_PG_UP:
					gpio[SECOND_EXPANDER].digitalWrite(BIT_M_DIR, LOW);
					delay(MOTOR_SW_DELAY);
					gpio[SECOND_EXPANDER].digitalWrite(BIT_MOTOR, LOW);
					delay(MOTOR_SW_DELAY);
					gpio[SECOND_EXPANDER].digitalWrite(BIT_MOTOR, state);
					break;
				case PWR_PG_DOWN:
					gpio[SECOND_EXPANDER].digitalWrite(BIT_MOTOR, LOW);
					delay(MOTOR_SW_DELAY);
					gpio[SECOND_EXPANDER].digitalWrite(BIT_M_DIR, state);
					delay(MOTOR_SW_DELAY);
					gpio[SECOND_EXPANDER].digitalWrite(BIT_MOTOR, state);
					break;
			}
		}

		uint16_t* getAnalogValues()
		{
			return _adc;
		}

		// TODO: make private
		void readADC()
		{
			_adc[ADC_1] = analogRead(PIN_ANALOG_A);
			_adc[ADC_2] = analogRead(PIN_ANALOG_B);
			_adc[ADC_3] = analogRead(PIN_ANALOG_C);
			_adc[ADC_4] = analogRead(PIN_ANALOG_D);
		}

		bool* getDigitalValues()
		{
			return _dig_keys;
		}

		// TODO: make private, operate on interrupt
		void readDigital()
		{
			uint16_t keys = _readExpanders();

			for (int i = 0; i < UI_NKEYS; i++) {
				if (keys & (0x0001 << i)) {
					_ui_keys[i] = false;
				}
				else {
					_ui_keys[i] = true;
				}
			}

			keys >>= KEYS_START_BIT;

			for (int i = 0; i < DIG_NKEYS; i++) {
				if (keys & (0x0001 << i)) {
					_dig_keys[i] = false;
				}
				else {
					_dig_keys[i] = true;
				}
			}
		}

		bool userPlus()
		{
			return _ui_keys[UI_PLUS];
		}

		bool userMinus()
		{
			return _ui_keys[UI_MINUS];
		}

		bool userBack()
		{
			return _ui_keys[UI_BACK];
		}

		bool userForw()
		{
			return _ui_keys[UI_FORW];
		}

		bool userOK()
		{
			return _ui_keys[UI_OK];
		}

		bool userHome()
		{
			return _ui_keys[UI_HOME];
		}

	private:
		uint16_t _readExpanders()
		{
			return gpio[FIRST_EXPANDER].portRead(BOTH_PORTS);
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
		bool _pressed = false;
		bool _lock = false;
		float _last_ph = 0.0;
		int _last_tds = 0;
		//uint16_t _buttons_buffer[10];
		uint16_t _adc[N_ADC];
		bool _dig_keys[DIG_NKEYS];
		bool _ui_keys[UI_NKEYS];
} io;

#endif
