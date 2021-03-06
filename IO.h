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
typedef enum {
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
} dig_inputs_t;

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

class Toggle;

Toggle* toggles[PWR_PG_NITEMS];

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
			if (_pressed || !digitalRead(EXPANDER_INT)) {
				readDigital();
				_pressed = false;
#ifdef EXPANDERS_DEBUG
				Serial.println("???????????????????? ???? ?????????????????????? ??????????????");
#endif
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
			/*
			try
			{
				readDigital();
			}
			catch(...)
			{
				Serial.print("wire end returned: ");
				Serial.println(_wireError);
				_wireError = 0;
			}
			*/
		}

		void initTdsPh()
		{
			if (!ph_meter.begin())
				_no_ph = true;
			else
				_no_ph = false;

			if (!tds_meter.begin())
				_no_tds = true;
			else
				_no_tds = false;
		}

		void initMeters()
		{
			if (!ph_meter.begin())
				_no_ph = true;
			else
				_no_ph = false;
			if (!tds_meter.begin())
				_no_tds = true;
			else
				_no_tds = false;

			if (!sht_meter.begin())
				_no_sht = true;
			else
				_no_sht = false;
		}

		bool noTds()
		{
			return _no_tds;
		}

		bool noPh()
		{
			return _no_ph;
		}

		bool noSht()
		{
			return _no_sht;
		}

		void reinit()
		{
#ifdef EXPANDERS_DEBUG
			Serial.println("?????????????????????????????? ??????????????????????");
#endif
			// expander stuff
			gpio[FIRST_EXPANDER].begin();
			gpio[SECOND_EXPANDER].begin();
			gpio[FIRST_EXPANDER].portMode(BOTH_PORTS, pinsAll(INPUT));
			gpio[SECOND_EXPANDER].portMode(SECOND_PORT, pinsAll(INPUT));
			gpio[SECOND_EXPANDER].portMode(FIRST_PORT, pinsAll(OUTPUT));
			gpio[SECOND_EXPANDER].portWrite(FIRST_PORT, pinsAll(LOW));
		}

		void init()
		{
#ifdef EXPANDERS_DEBUG
			Serial.println("?????????????????????????? ??????????????????????");
#endif
			// expander stuff
			gpio[FIRST_EXPANDER].begin();
			gpio[SECOND_EXPANDER].begin();
			gpio[FIRST_EXPANDER].portMode(BOTH_PORTS, pinsAll(INPUT));
			gpio[SECOND_EXPANDER].portMode(SECOND_PORT, pinsAll(INPUT));
			gpio[SECOND_EXPANDER].portMode(FIRST_PORT, pinsAll(OUTPUT));
			gpio[SECOND_EXPANDER].portWrite(FIRST_PORT, pinsAll(LOW));

			// native esp32
			pinMode(EXPANDER_INT, INPUT_PULLUP);
			attachInterrupt(EXPANDER_INT, std::bind(&InputOutput::isr, this), FALLING);
			pinMode(PORT_F, OUTPUT);
			pinMode(PORT_G, OUTPUT);
			pinMode(PORT_H, OUTPUT);
			pinMode(LED, OUTPUT);
			pinMode(FAN, OUTPUT);

			// update data in esp memory
			readDigital();
		}

		float getEC()
		{
			//return 0.3;
			return (float)tds_meter.getEC()/1000;
		}

		float getPH()
		{
			//return 6.1;
			return ph_meter.getPH();
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

			_out_states[id] = true;

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

			_out_states[id] = false;

			switch (id) {
				default: break;
				case PWR_PG_PORT_F: analogWrite(PORT_F, 0); break;
				case PWR_PG_PORT_G: analogWrite(PORT_G, 0); break;
				case PWR_PG_PORT_H: analogWrite(PORT_H, 0); break;
				case PWR_PG_FAN: analogWrite(FAN, 0); break;
				case PWR_PG_LIGHT: analogWrite(LED, 0); break;
			}
		}

		void haltAll()
		{
			for (int i = 0; i < PWR_PG_NITEMS; i++) {
				toggles[i]->on(false);
				haltPWMout(i);
				driveOut(i, LOW);
			}
		}

		bool getOut(int id)
		{
			if (id < 0)
				return false;
			return _out_states[id];
		}

		void driveOut(int id, bool state)
		{
			if (id < 0) {
#ifdef EXPANDERS_DEBUG
				Serial.print("???????????????? ??????????");
#endif
				return;
			}

			if (state == _out_states[id]) {
				return;
			}
			else {
#ifdef EXPANDERS_DEBUG
				Serial.println();
				switch (id) {
					default: Serial.println("???? ?????????? ??????????????????????"); break;
					case PWR_PG_PORT_A: Serial.println("?????????? A."); break;
					case PWR_PG_PORT_B: Serial.println("?????????? B."); break;
					case PWR_PG_PORT_C: Serial.println("?????????? C."); break;
					case PWR_PG_PORT_D: Serial.println("?????????? D."); break;
					case PWR_PG_PORT_E: Serial.println("?????????? E."); break;
				}

				Serial.print("C????????????????: ");
				Serial.println(state);
#endif
			}

			_out_states[id] = state;

			switch (id) {
				default: break;
				case PWR_PG_PORT_A:
				case PWR_PG_PORT_B:
				case PWR_PG_PORT_C:
				case PWR_PG_PORT_D:
				case PWR_PG_PORT_E:
					gpio[SECOND_EXPANDER].digitalWrite(id, state);
#ifdef EXPANDERS_DEBUG
					Serial.println("???????????? ?????????????????????? ????????????????????");
#endif
					break;
				case PWR_PG_PORT_F:
					analogWrite(PORT_F, state?255:0);
					//digitalWrite(PORT_F, state);
					break;
				case PWR_PG_PORT_G:
					analogWrite(PORT_G, state?255:0);
					//digitalWrite(PORT_G, state);
					break;
				case PWR_PG_PORT_H:
					analogWrite(PORT_H, state?255:0);
					//digitalWrite(PORT_H, state);
					break;
				case PWR_PG_FAN:
					analogWrite(FAN, state?255:0);
					//digitalWrite(FAN, state);
					break;
				case PWR_PG_LIGHT:
					analogWrite(LED, state?255:0);
					//digitalWrite(LED, state);
					break;
				case PWR_PG_UP:
					Serial.println("Motor UP");
					gpio[SECOND_EXPANDER].digitalWrite(BIT_MOTOR, LOW);
					delay(MOTOR_SW_DELAY);
					gpio[SECOND_EXPANDER].digitalWrite(BIT_M_DIR, LOW);
					delay(MOTOR_SW_DELAY);
					gpio[SECOND_EXPANDER].digitalWrite(BIT_MOTOR, state);
					break;
				case PWR_PG_DOWN:
					Serial.println("Motor DOWN");
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

		bool readAsDig(int a)
		{
			return digitalRead(a);
		}

		bool* getDigitalValues()
		{
			return _dig_keys;
		}

		bool getDigital(dig_inputs_t input)
		{
			if (input > DIG_NKEYS)
				return false;

			return _dig_keys[input];
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
		int _wireError = 0;

		uint16_t _readExpanders()
		{
			/*
			static constexpr int DATASIZE = 2;
			char data[DATASIZE]{0};
			Wire.beginTransmission(0x20);
			Wire.requestFrom(0x20, 2);
			static int i = 0;
			while (Wire.available()) {
				data[i] = Wire.read();
				i++;
				i %= DATASIZE;
			}
			uint16_t firstExpData= *((uint16_t*)data);

			//Wire.beginTransmission(0x21);
			Wire.requestFrom(0x21, 2);

			while (Wire.available()) {
				data[i] = Wire.read();
				i++;
				i %= DATASIZE;
			}

			_wireError = Wire.endTransmission();


			if (!_wireError) {
				return firstExpData;
			}
			else {
				Serial.println("Wire error");
				throw std::runtime_error("wire error");
			}
			*/

			gpio[SECOND_EXPANDER].portRead(BOTH_PORTS);
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
		bool _out_states[PWR_PG_NITEMS];
		uint16_t _adc[N_ADC];
		bool _dig_keys[DIG_NKEYS];
		bool _ui_keys[UI_NKEYS];
		bool _no_ph = false;
		bool _no_tds = false;
		bool _no_sht = false;
} io;

#endif
