#ifndef __RIG_H__
#define __RIG_H__
#include "IO.h"
#include "DateTime.h"

#ifdef RIG_DEBUG
OutputField gMesState;
#endif

class Rig {
	public:
		Rig(): _paused(true){};

		float lastPH()
		{
			return _lastPh;
		}

		float lastEC()
		{
			return _lastEc;
		}

		void start()
		{
			_paused = false;
		}

		void halt()
		{
			_paused = true;
			_led = false;
			_passvent = false;
			_vent = false;
			_pumpison = false;
			_mainpump = false;
			io.haltAll();
			//Serial.println("halted...");
		}

		void update()
		{
			if (_paused)
				return;

			_updateLight();
			_updateVent();
			_updatePassVent();
			_updateSolutions();
			_updateRigType();
		}

		bool getLed()
		{
			return _led;
		}

		bool getVent()
		{
			return _vent;
		}

		bool getPassVent()
		{
			return _passvent;
		}

		bool getPump()
		{
			return _mainpump;
		}

		void setMeasureWindow(bool window=true)
		{
			_measureWindow = window;
		}

		void measureTime()
		{
			_measuretime = true;
		}

		bool measureDone()
		{
			bool measuredone = _measuredone;
			_measuredone = false;
			return measuredone;
		}

	private:
		bool _measuredone = false;
		bool _measuretime = false;
		bool _paused;
		bool _window_opened = false;
		bool _window_energized = false;
		unsigned long _window_timer = 0;
		static constexpr unsigned long WINDOW_INT = 60000;
		bool _led = false;
		bool _vent = false;
		bool _passvent = false;
		bool _measureWindow = false;

		void _updateLight()
		{
			_led = false;

			if (!g_data.getInt(LIGHT_ON)) {
				io.haltPWMout(PWR_PG_LIGHT);
				return;
			}

			// check day
			if (datetime.getDays() < g_data.getInt(LIGHT_DAY)) {
				io.haltPWMout(PWR_PG_LIGHT);
				return;
			}

			// check hour
			int tmp = datetime.getHour();
			if (g_data.getInt(LIGHT_FROM) <= tmp && tmp < g_data.getInt(LIGHT_TO)) {

				// set led brightness
				uint8_t brightness = clamp(uint8_t(g_data.getFloat(ADD_LED_BRIGHT)/100 * 255), 1, 255);
				io.drivePWMout(PWR_PG_LIGHT, brightness);
				_led = true;
			}
			else {
				io.haltPWMout(PWR_PG_LIGHT);
			}
		}


		void _updateVent()
		{
			_vent = false;
			if (!g_data.getInt(VENT_ON)) {
				io.driveOut(PWR_PG_FAN, false);
				return;
			}

			bool a = g_data.getInt(VENT_TIME_LIM);
			bool b = g_data.getInt(VENT_TEMP_LIM);
			bool c = g_data.getInt(VENT_HUM_LIM);
			int tmp = datetime.getHour();
			bool x = (g_data.getInt(VENT_TIME_FROM) <= tmp) && (tmp < g_data.getInt(VENT_TIME_TO));
			bool y = int(io.getTem()) > g_data.getInt(VENT_TEMP_THRES);
			bool z = int(io.getHum()) > g_data.getInt(VENT_HUM_THRES);

			if (!a)
				x = false;
			if (!b || io.noSht())
				y = false;
			if (!c || io.noSht())
				z = false;

			bool Q = x || y || z;

			if (Q) {
				io.driveOut(PWR_PG_FAN, true);
				_vent = true;
			}
			else
				io.driveOut(PWR_PG_FAN, false);
		}

		void _openWindow()
		{
			io.driveOut(PWR_PG_UP, true);
			_window_energized = true;
			_window_timer = millis();
		}

		void _closeWindow()
		{
			io.driveOut(PWR_PG_DOWN, true);
			_window_energized = true;
			_window_timer = millis();
		}

		void _updatePassVent()
		{
			if (_window_energized || _window_opened) {
				_passvent = true;
			}
			else {
				_passvent = false;
			}

			if (!g_data.getInt(PASSVENT)) {
				return;
			}

			if (!_window_energized) {
				io.driveOut(PWR_PG_UP, false);
			}

			if (_window_energized) {
				if (millis() - _window_timer > WINDOW_INT) {
					_window_energized = false;
					_window_opened = !_window_opened;
				}
				return;
			}

			bool a = g_data.getInt(PASSVENT_TIME_LIM);
			bool b = g_data.getInt(PASSVENT_TEMP_LIM);
			bool c = g_data.getInt(PASSVENT_HUM_LIM);
			int tmp = datetime.getHour();
			bool x = (g_data.getInt(PASSVENT_TIME_FROM) <= tmp)
				&& (tmp < g_data.getInt(PASSVENT_TIME_TO));
			bool y = int(io.getTem()) > g_data.getInt(PASSVENT_TEMP_THRES);
			bool z = int(io.getHum()) > g_data.getInt(PASSVENT_HUM_THRES);

			if (!a)
				x = false;
			if (!b || io.noSht())
				y = false;
			if (!c || io.noSht())
				z = false;

			bool Q = x || y || z;

			if (Q && !_window_opened) {
				_openWindow();
			}
			else if (!Q && _window_opened)
				_closeWindow();
		}

		bool _commenceABC = false;
		bool _prepareABC = false;
		bool _commencePH = false;
		bool _preparePH = false;
		float _pumpAseconds = 0;
		float _pumpBseconds = 0;
		float _pumpCseconds = 0;
		int _pumpPHseconds = 0;

		static constexpr unsigned long _begin_interval = 10000;
		unsigned long _begin_mils = 0;
		bool _metersBusy = false;
		float _lastPh;
		float _lastEc;

		bool _localMeas = false;
		bool _localMeas2 = false;
		unsigned long _meas_mils = 0;
		static constexpr unsigned long _meas_interval = 15000;

		// read from settings EC_ON, ACID_ON
		bool _acid = false;
		bool _ec = false;

		void _resetMeasure()
		{
			_measuretime = false;
			_localMeas = false;
			_metersBusy = false;
			_localMeas2 = false;
			_measuredone = true;
			_mainpumpwindow = false;
			_aeropumpwindow = false;
			// single shoot flag after reboot
			_rigInitDone = true;
		}

		void _measure()
		{
			_acid = g_data.getInt(ACID_ON);
			_ec = g_data.getInt(EC_ON);

			if (!_acid && !_ec) {
#ifdef RIG_DEBUG
				Serial.println("Нормализация не установлена пользователем");
#endif
				_resetMeasure();
				return;
			}

			if (!_metersBusy) {
#ifdef RIG_DEBUG
				gMesState.setValue(1);
				Serial.println("Инициализация датчиков ph и tds");
				Serial.print("День посадки: ");
				Serial.println(datetime.getDays());
#endif
				io.initMeters();
				_begin_mils = millis();
				_metersBusy = true;

				// reset all if both sensors absent;
				if (io.noTds() && io.noPh()) {
#ifdef RIG_DEBUG
					Serial.println("Датчиков не найдено");
					gMesState.setValue(0);
#endif
					_resetMeasure();
					return;
				}

#ifdef RIG_DEBUG
				else if (!_acid) {
					Serial.println("Нормализация pH не установлена пользователем");
				}
				else if (!_ec) {
					Serial.println("Нормализация EC не установлена пользователем");
				}
#endif
			}

			if (millis() - _begin_mils > _begin_interval && !_localMeas) {
#ifdef RIG_DEBUG
				Serial.println("Замер показаний");
#endif

				for (int i = 0; i < 3; i++) {
					_lastPh = io.getPH();
					_lastEc = io.getEC();
				}

				_meas_mils = millis();
				_localMeas = true;
				_localMeas2 = true;
			}

			if (millis() - _meas_mils > _meas_interval && _localMeas2) {
				_lastPh = io.getPH();
				_lastEc = io.getEC();

				//io.reinit();
#ifdef RIG_DEBUG
				Serial.println("Показания сняты");
				Serial.println();
				gMesState.setValue(2);
#endif
				_resetMeasure();

				if (!io.noTds() && _ec) {
					_prepareABC = true;

				}
				else if (!io.noPh() && _acid) {
					_preparePH = true;
				}
#ifdef RIG_DEBUG
				else
					gMesState.setValue(0);
#endif
			}
		}

		// between a, b, c and ph pumping
		int _sol_interval = 0;
		bool _intervalFlag = false;

		void _setSolInterval(int seconds)
		{
			_sol_interval = seconds * 1000;
		}

		void _updateSolutions()
		{
			int hour = datetime.getHour();
			_setSolInterval(g_data.getInt(SOLUTIONS_INT));

			// allowed time interval
			if (g_data.getInt(NORM_AL_TM_LO) <= hour && hour < g_data.getInt(NORM_AL_TM_HI)) {

				// measure pH and EC
				if (_measuretime) {
					_measure();
					return;
				}

				// normalize
				_updateABC();
				_squirtABC();
				_updatePH();
				_squirtPH();
			}
		}


		void _updateABC()
		{
			if (!_prepareABC)
				return;

			if (_commenceABC)
				return;

			if (!_ec)
				return;

			// return if one of the tanks is empty
			bool* dig = io.getDigitalValues();
			bool a = dig[DIG_KEY10];
			bool b = dig[DIG_KEY9];
			bool c = dig[DIG_KEY8];

			if (a || b || c) {
				Serial.print("A: ");
				Serial.println(a);
				Serial.print("B: ");
				Serial.println(b);
				Serial.print("C: ");
				Serial.println(c);
				_prepareABC = false;
				_preparePH = true;
#ifdef RIG_DEBUG
				Serial.println("Один из баков пуст. Нормализация запрещена");
#endif
				return;
			}

			rig_settings_t ec_set, ec_a, ec_b, ec_c;
			int pumptime = g_data.getInt(EC_PUMPS);
			int today = datetime.getDays();

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
#ifdef RIG_DEBUG
				Serial.println("Первая стадия EC");
#endif
				ec_set = EC_CYCL1;
				ec_a = EC_A1;
				ec_b = EC_B1;
				ec_c = EC_C1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL2;
				ec_a = EC_A2;
				ec_b = EC_B2;
				ec_c = EC_C2;
#ifdef RIG_DEBUG
				Serial.println("Вторая стадия EC");
#endif
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL3;
				ec_a = EC_A3;
				ec_b = EC_B3;
				ec_c = EC_C3;
#ifdef RIG_DEBUG
				Serial.println("Третья стадия EC");
#endif
			}

			float set_ec = g_data.getFloat(ec_set);
			float hyst_ec = g_data.getFloat(EC_HYST);
			float target_ec = set_ec - hyst_ec;

#ifdef RIG_DEBUG
			Serial.print("Целевой ЕС: ");
			Serial.println(set_ec);
			Serial.print("Гистерезис ЕС: ");
			Serial.println(hyst_ec);
			Serial.print("Целевой ЕС - гистерезис: ");
			Serial.println(set_ec - hyst_ec);
			Serial.print("Текущий: ");
			Serial.println(_lastEc);
			Serial.println();
#endif

			// if EC more than what's set go to PH
			if (_lastEc >= target_ec) {
#ifdef RIG_DEBUG
				Serial.print("Текущие показания EC в пределах нормы");
#endif
				_prepareABC = false;
				_preparePH = true;
				return;
			}

			_commenceABC = true;;

			//squirt amount set in settings
			_pumpAseconds = g_data.getFloat(ec_a) * pumptime;
			_pumpBseconds = g_data.getFloat(ec_b) * pumptime;
			_pumpCseconds = g_data.getFloat(ec_c) * pumptime;
			_pump_mils = millis();
			_prepareABC = false;
		}

		unsigned long _pump_mils = 0;
		enum {
			A_ON,
			WAIT_A,
			B_ON,
			WAIT_B,
			C_ON,
			WAIT_C
		} pumps = A_ON;

		void _squirtABC()
		{
			if (!_commenceABC)
				return;

			if (pumps == A_ON) {
				if (!io.getOut(pumpA)) {
#ifdef RIG_DEBUG
					Serial.println("Нормализация EC");
					Serial.println("Резервуар A");
					gMesState.setValue(3);
#endif
					io.driveOut(pumpA, true);
				}
				if (millis() - _pump_mils > _pumpAseconds*1000) {
#ifdef RIG_DEBUG
					unsigned long timeA = millis() - _pump_mils;
					Serial.print("Установка времени работы насоса A, сек:");
					Serial.println(_pumpAseconds, 1);
					Serial.print("Время работы насоса A, мс: ");
					Serial.println(timeA);
#endif
					io.driveOut(pumpA, false);
					pumps = WAIT_A;
					_pump_mils = millis();
				}
			}

			if (pumps == WAIT_A) {
				if (millis() - _pump_mils > _sol_interval) {
					pumps = B_ON;
					_pump_mils = millis();
				}
			}

			if (pumps == B_ON) {
				if (!io.getOut(pumpB)) {
#ifdef RIG_DEBUG
					Serial.println("Резервуар B");
#endif
					io.driveOut(pumpB, true);
				}
				if (millis() - _pump_mils > _pumpBseconds*1000) {
#ifdef RIG_DEBUG
					unsigned long timeB = millis() - _pump_mils;
					Serial.print("Установка времени работы насоса B, сек:");
					Serial.println(_pumpBseconds, 1);
					Serial.print("Время работы насоса B, мс: ");
					Serial.println(timeB);
#endif
					io.driveOut(pumpB, false);
					pumps = WAIT_B;
					_pump_mils = millis();
				}
			}

			if (pumps == WAIT_B) {
				if (millis() - _pump_mils > _sol_interval) {
					pumps = C_ON;
					_pump_mils = millis();
				}
			}

			if (pumps == C_ON) {
				if (!io.getOut(pumpC)) {
#ifdef RIG_DEBUG
					Serial.println("Резервуар C");
#endif
					io.driveOut(pumpC, true);
				}
				if (millis() - _pump_mils> _pumpCseconds*1000) {
#ifdef RIG_DEBUG
					unsigned long timeC = millis() - _pump_mils;
					Serial.print("Установка времени работы насоса C, сек:");
					Serial.println(_pumpCseconds, 1);
					Serial.print("Время работы насоса C, мс: ");
					Serial.println(timeC);
#endif
					io.driveOut(pumpC, false);
					pumps = WAIT_C;
				}
			}

			if (pumps == WAIT_C) {
				if (millis() - _pump_mils > _sol_interval) {
					pumps = A_ON;
					_pump_mils = millis();
					_commenceABC = false;
					_preparePH = true;
				}
			}
		}

		enum {
			NO_PH,
			PH_UP,
			PH_DW,
		} _ph_state;

		void _updatePH()
		{

			if (!_preparePH || _commencePH)
				return;

			if (!_acid)
				return;


			rig_settings_t ph_set;
			int today = datetime.getDays();
#ifdef RIG_DEBUG
			Serial.println();
#endif

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
#ifdef RIG_DEBUG
				Serial.println("Первая стадия PH");
#endif
				ph_set = ACID_1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {

#ifdef RIG_DEBUG
				Serial.println("Вторая стадия PH");
#endif
				ph_set = ACID_2;
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {

#ifdef RIG_DEBUG
				Serial.println("Третья стадия PH");
#endif
				ph_set = ACID_3;
			}

			float ph = g_data.getFloat(ph_set);
			float ph_hyst = g_data.getFloat(PH_HYST);

			// if PH is less
			float loLim = ph - ph_hyst;
			float hiLim = ph + ph_hyst;
			if (loLim > _lastPh) {
#ifdef RIG_DEBUG
				Serial.print("pH низкий: ");
				Serial.println(_lastPh);
				Serial.print("Целевой: ");
				Serial.println(ph);
				Serial.print("Целевой - гистерезис: ");
				Serial.println(loLim, 1);
#endif
				_ph_state = PH_UP;
				_commencePH = true;
			}
			else if (ph + ph_hyst < _lastPh) {
#ifdef RIG_DEBUG
				Serial.print("pH высокий: ");
				Serial.println(_lastPh);
				Serial.print("Целевой pH: ");
				Serial.println(ph);
				Serial.print("Целевой pH + гистерезис: ");
				Serial.println(hiLim, 1);
#endif
				_ph_state = PH_DW;
				_commencePH = true;
			}
			else {
#ifdef RIG_DEBUG
				Serial.println("pH в пределах установок");
				Serial.print("Целевой pH: ");
				Serial.println(ph);
				Serial.print("Целевой pH - гистерезис: ");
				Serial.println(loLim, 1);
				Serial.print("Целевой pH + гистерезис: ");
				Serial.println(hiLim, 1);
				gMesState.setValue(0);
#endif
				_ph_state = NO_PH;
				_commencePH = false;
			}

			_preparePH = false;
			_pump_mils = millis();
		}


		void _squirtPH()
		{
			if (!_commencePH)
				return;

			// tank states
			bool* dig = io.getDigitalValues();
			bool up_empty = dig[DIG_KEY7];
			bool down_empty = dig[DIG_KEY6];

			_pumpPHseconds = g_data.getInt(ACID_PUMPS);

			if (_ph_state == PH_UP) {
				if (up_empty) {
#ifdef RIG_DEBUG
					Serial.println("Резервуар pH UP пуст");
					gMesState.setValue(0);
#endif
					_commencePH = false;
					return;
				}
				if (!io.getOut(phUpPump)) {
#ifdef RIG_DEBUG
					gMesState.setValue(4);
					Serial.println("Нормализуем pH");
#endif

					io.driveOut(phUpPump, true);
				}
				if (millis() - _pump_mils > _pumpPHseconds*1000) {

#ifdef RIG_DEBUG
					unsigned long timePH = millis() - _pump_mils;
					Serial.print("Установка времени работы насоса PH_UP, сек:");
					Serial.println(_pumpPHseconds);
					Serial.print("Время работы насоса PH_UP, мс: ");
					Serial.println(timePH);
#endif
					io.driveOut(PWR_PG_PORT_D, false);
					_ph_state = NO_PH;
#ifdef RIG_DEBUG
					gMesState.setValue(0);
#endif
					_commencePH = false;
				}
			}
			else if (_ph_state == PH_DW) {
				if (down_empty) {
#ifdef RIG_DEBUG
					Serial.println("Резервуар pH DOWN пуст");
					gMesState.setValue(0);
#endif
					_commencePH = false;
					return;
				}
				if (!io.getOut(phDwPump)) {
#ifdef RIG_DEBUG
					gMesState.setValue(4);
					Serial.println("Нормализуем pH");
#endif

					io.driveOut(phDwPump, true);
				}
				if (millis() - _pump_mils > _pumpPHseconds*1000) {
#ifdef RIG_DEBUG
					Serial.print("Установка времени работы насоса PH_DOWN, сек:");
					Serial.println(_pumpPHseconds);
					Serial.print("Время работы насоса PH_DOWN, мс: ");
					Serial.println(millis() - _pump_mils);
					Serial.println();
#endif
					io.driveOut(phDwPump, false);
					_ph_state = NO_PH;
#ifdef RIG_DEBUG
					gMesState.setValue(0);
#endif
					_commencePH = false;
				}
			}
		}

		void _updateRigType()
		{
			switch (g_rig_type) {
				default: break;
				case RIG_DEEPWATER: _deepwater(); break;
				case RIG_LAYER: _layer(); break;
				case RIG_FLOOD: _flood(); break;
				case RIG_AERO: _aero(); break;
				case RIG_DRIP: _drip(); break;
				case RIG_OPENG: _openg(); break;
				case RIG_GREENH: _greenh(); break;
				case RIG_MIXSOL: _mixsol(); break;
			}
		}

		bool _mainpumpwindow = false;
		bool _aeropumpwindow = false;
		bool _rigInitDone = false;
	public:
		void setMainPumpWindow()
		{
			_mainpumpwindow = true;
		}

		void setAeroPumpWindow()
		{
			_aeropumpwindow = true;
		}

	private:
		int pumpA, pumpB, pumpC, phUpPump, phDwPump, waterPump, mainPump, aeroPump;

		bool _pumpison = false;
		bool _mainpump = false;
		bool _haltpump = false;
		unsigned long _pumpMils = 0;
		unsigned long _pump_timeout = 60000;
#ifdef RIG_DEBUG
		unsigned long _aero_window_mils = 0;
		unsigned long _main_window_mils = 0;
#endif

		// h2o pump hyst
		bool _pumpHystFlag = false;
		unsigned long _pumpHystMils = 0;
		static constexpr unsigned long _pumpHyst = 5000;

		void _setPumpTimeOut(int minutes)
		{
			_pump_timeout = minutes * 60 * 1000;
		}

		void _driveH2Opump()
		{
			io.driveOut(PWR_PG_PORT_F, !io.getDigital(DIG_KEY5));
		}

		void _stopH2Opump()
		{
			if (_pumpison && millis() - _pumpMils > _pumpHyst)
				io.driveOut(PWR_PG_PORT_F, false);
		}

		void _haltH2Opump()
		{
			io.driveOut(PWR_PG_PORT_F, false);
		}

		void _deepwater()
		{
			pumpA = PWR_PG_PORT_A;
			pumpB = PWR_PG_PORT_B;
			pumpC = PWR_PG_PORT_C;
			phUpPump = PWR_PG_PORT_D;
			phDwPump = PWR_PG_PORT_E;
			waterPump = PWR_PG_PORT_F;
			mainPump = PWR_PG_PORT_G;
			aeroPump = PWR_PG_PORT_H;

			if (!_rigInitDone)
				return;
			// H2O pump
			if (!_haltpump) {
				if (!io.getDigital(DIG_KEY3) && !_measuretime) {
					_driveH2Opump();
				}
				else if (io.getDigital(DIG_KEY3) || io.getDigital(DIG_KEY4) || _measuretime) {
					_stopH2Opump();
				}
			}
			else {
				// TODO: indicate that pump is halted
				//g_Tap->setBlink();
				_haltH2Opump();
			}

			// reset flag if pump is off
			if (!io.getOut(waterPump)) {
				_pumpison = false;
			}

			// raise flag if pump is on
			if (!_pumpison && io.getOut(waterPump)) {
				_pumpison = true;
				// remember when
				_pumpMils = millis();
			}

			_setPumpTimeOut(g_data.getInt(PUMP_TIMEOUT));

			if (!_haltpump && _pumpison && millis() - _pumpMils > _pump_timeout) {
				_haltpump = true;
#ifdef RIG_DEBUG
				Serial.println("Насос H2O аварийно выключен");
#endif
			}

			// aero pump
			if (_aeropumpwindow && g_data.getInt(AERO_PUMP)) {
#ifdef RIG_DEBUG
				_aero_window_mils = millis();
#endif
				io.driveOut(aeroPump, false);
			}
			else if (g_data.getInt(AERO_ON)){
				io.driveOut(aeroPump, true);
			}

			// main pump
			if (_mainpumpwindow && g_data.getInt(PUMP_OFF)) {
#ifdef RIG_DEBUG
				_main_window_mils = millis();
#endif
				io.driveOut(mainPump, false);
			}
			else {
				io.driveOut(mainPump, io.getDigital(DIG_KEY2));
			}

			_mainpump = io.getOut(mainPump);
		}

		void _layer()
		{
		}

		void _flood()
		{
		}

		void _aero()
		{
		}

		void _drip()
		{
		}

		void _openg()
		{
		}

		void _greenh()
		{
		}

		void _mixsol()
		{
		}

} g_rig;

#endif
