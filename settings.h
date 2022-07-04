#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "json.hpp"
#include <SPIFFS.h>
//#define STR(A) (#A)

const char* data_file = "/data";

using json = nlohmann::json;

// x macro
#define ALL_SETTINGS \
	X(SETT_EMPTY) \
	/* stage 2 settings */ \
	X(LIGHT_ON) \
	X(LIGHT_FROM) \
	X(LIGHT_TO) \
	X(LIGHT_DAY) \
 \
	/* stage 3 settings */ \
	X(VENT_ON) \
	X(VENT_TIME_LIM) \
	X(VENT_TIME_FROM) \
	X(VENT_TIME_TO) \
	X(VENT_TEMP_LIM) \
	X(VENT_TEMP_THRES) \
	X(VENT_HUM_LIM) \
	X(VENT_HUM_THRES) \
	X(VENT_CONST) \
	X(VENT_CYCL) \
	X(VENT_DUR) \
	X(VENT_PAUS_DUR) \
 \
	/* stage 4 settings */ \
	X(PASSVENT) \
	X(PASSVENT_TIME_LIM) \
	X(PASSVENT_TIME_FROM) \
	X(PASSVENT_TIME_TO) \
	X(PASSVENT_TEMP_LIM) \
	X(PASSVENT_TEMP_THRES) \
	X(PASSVENT_HUM_LIM) \
	X(PASSVENT_HUM_THRES) \
 \
	/* stage 5 settings */ \
	X(GR_CYCL_1_DAYS) \
	X(GR_CYCL_2_DAYS) \
	X(GR_CYCL_3_DAYS) \
 \
	/* stage 6 settings */ \
	X(EC_ON) \
	X(EC_CYCL1) \
	X(EC_A1) \
	X(EC_B1) \
	X(EC_C1) \
	X(EC_CYCL2) \
	X(EC_A2) \
	X(EC_B2) \
	X(EC_C2) \
	X(EC_CYCL3) \
	X(EC_A3) \
	X(EC_B3) \
	X(EC_C3) \
	X(EC_PUMPS) \
 \
	/* stage 7 settings */ \
	X(ACID_ON) \
	X(ACID_1) \
	X(ACID_2) \
	X(ACID_3) \
	X(ACID_PUMPS) \
 \
	/* stage 8 settings */ \
	X(PUMP_OFF) \
	X(PUMP_SEC) \
 \
	/* stage 9 settings */ \
	X(AERO_ON) \
	X(AERO_PUMP) \
	X(AERO_PUMP_SEC) \
 \
	/* stage 9_2 settings */ \
	X(STIR_ON) \
	X(STIR_PUMP) \
	X(STIR_PUMP_SEC) \
 \
	/* stage 8_2 settings */ \
	X(FLOOD_HOURS) \
	X(FLOOD_MIN) \
	X(FLOOD_HOLD_MIN) \
	X(FLOOD_HOLD_SEC) \
 \
	/* stage 8_3 settings */ \
	X(SPRAY_PUMP) \
	X(SPRAY_PUMP_SEC) \
	X(SPRAY_CONS) \
	X(SPRAY_CYCL) \
	X(SPRAY_MIN) \
	X(SPRAY_SEC) \
	X(SPRAY_CYCL_MIN) \
	X(SPRAY_CYCL_SEC) \
 \
	/* stage 8_4 settings */ \
	X(DRIP_PUMP) \
	X(DRIP_PUMP_SEC) \
	X(DRIP_CONS) \
	X(DRIP_CYCL) \
	X(DRIP_MIN) \
	X(DRIP_SEC) \
	X(DRIP_CYCL_MIN) \
	X(DRIP_CYCL_SEC) \
 \
	/* additional settings */ \
	X(ADD_LED_BRIGHT) \
	X(START_DAY) \
	X(EC_HYST) \
	X(PH_HYST) \
	X(PUMP_TIMEOUT) /* allowed H20 pump time */ \
	X(NORM_AL_TM_HI) \
	X(NORM_AL_TM_LO) \
	X(ADD_MEAS_INT) \
	X(SOLUTIONS_INT) \
	X(ALLOWED_PH_MIN) \
	X(ALLOWED_PH_MAX) \
	X(ALLOWED_EC_MIN) \
	X(ALLOWED_EC_MAX) \
 \
	X(NSETTINGS)

// all settings (x macro expansion)
typedef enum {
#define X(a) a,
	ALL_SETTINGS
#undef X
} rig_settings_t;

const char* STR(rig_settings_t n)
{
	switch (n) {
		default: return nullptr;
#define X(a) \
		case a: \
			return #a;
		ALL_SETTINGS
#undef X
	}
}

// rig types
typedef enum {
	RIG_DEEPWATER,
	RIG_LAYER,
	RIG_FLOOD,
	RIG_AERO,
	RIG_DRIP,
	RIG_OPENG,
	RIG_GREENH,
	RIG_MIXSOL,
	RIG_NTYPES
} rig_type;

rig_type g_rig_type = RIG_DEEPWATER;

// planting settings (no wifi data or screen settings here...)
class Data {
	public:
		Data()
		{
			set(GR_CYCL_1_DAYS, _firstStageDay);
			set(GR_CYCL_2_DAYS, _secondStageDay);
			set(GR_CYCL_3_DAYS, _thirdStageDay);
			set(EC_A1, f_A);
			set(EC_A2, f_A);
			set(EC_A3, f_A);
			set(EC_B1, f_B);
			set(EC_B2, f_B);
			set(EC_B3, f_B);
			set(EC_C1, f_C);
			set(EC_C2, f_C);
			set(EC_C3, f_C);
			set(EC_CYCL1, EC_1);
			set(EC_CYCL2, EC_2);
			set(EC_CYCL3, EC_3);
			set(ACID_1, 5.5f);
			set(ACID_2, 6.0f);
			set(ACID_3, 6.2f);
			set(DRIP_CONS, true);
			set(DRIP_CYCL, false);
			set(SPRAY_CONS, true);
			set(SPRAY_CYCL, false);
			set(ADD_LED_BRIGHT, 100);
			set(EC_HYST, 0.2f);
			set(PH_HYST, 0.5f);
			set(EC_PUMPS, 10);
			set(ACID_PUMPS, 10);
			set(LIGHT_FROM, 8);
			set(LIGHT_TO, 24);
			set(LIGHT_DAY, 3);
			set(VENT_TIME_FROM, 8);
			set(VENT_TIME_TO, 24);
			set(VENT_TEMP_THRES, 30);
			set(VENT_HUM_THRES, 60);
			set(PASSVENT_TIME_FROM, 8);
			set(PASSVENT_TIME_TO, 24);
			set(PASSVENT_TEMP_THRES, 30);
			set(PASSVENT_HUM_THRES, 60);
			set(PUMP_SEC, 15);
			set(AERO_PUMP_SEC, 15);
			set(PUMP_TIMEOUT, 1);
			set(NORM_AL_TM_HI, 24);
			set(NORM_AL_TM_LO, 0);
			set(ADD_MEAS_INT, 30);
			set(SOLUTIONS_INT, 1);
			set(ALLOWED_PH_MIN, 4.0f);
			set(ALLOWED_PH_MAX, 9.0f);
			set(ALLOWED_EC_MIN, 0.06f);
			set(ALLOWED_EC_MAX, 4.0f);
			set(VENT_CONST, 1);
			set(VENT_CYCL, 0);
			set(VENT_DUR, 10);
			set(VENT_PAUS_DUR, 50);
		}

		void print()
		{
			int count = 0;
			for (auto& i:_data) {
				Serial.print(i);
				Serial.print(" ");
				if (count % 20 == 0)
					Serial.println();
				count++;
			}
		}

		float getFloat(rig_settings_t setting)
		{
			return _data[setting];
		}

		int getInt(rig_settings_t setting)
		{
			return int(_data[setting]);
		}

		void set(rig_settings_t setting, float value)
		{
			_data[setting] = value;
		}

		void set(rig_settings_t setting, int value)
		{
			_data[setting] = float(value);
		}

		void load()
		{
			if (!SPIFFS.exists(data_file)) {
#ifdef APP_DEBUG
				Serial.println("no data file");
#endif
				return;
			}

			File file = SPIFFS.open(data_file, "r");

			if (!file) {
#ifdef APP_DEBUG
				Serial.println("couldn't open data file");
#endif
			}

			try
			{
				String content = file.readStringUntil(EOF);
#ifdef APP_DEBUG
				Serial.println(content);
#endif
				json load = json::parse(content.c_str());

				g_rig_type = load["RIG_TYPE"];

				// load all settings from json
				for (int i = 0; i < NSETTINGS; i++) {
					_data[i] = load[STR((rig_settings_t)i)].get<float>();
				}

				load.clear();

			}
			catch (...)
			{
#ifdef APP_DEBUG
				Serial.println("json exception occured");
#endif
				return;
			}


		}

		void save()
		{
			json d;
			d["RIG_TYPE"] = g_rig_type;

			// save all settings to json
			for (int i = 0; i < NSETTINGS; i++) {
				d[STR((rig_settings_t)i)] = _data[i];
			}

			File file = SPIFFS.open(data_file, "w");
			if (!file) {
#ifdef APP_DEBUG
				Serial.println("failed to save data file");
#endif
				d.clear();
				return;
			}
#ifdef APP_DEBUG
			else {
				Serial.println("data file saved");
			}
#endif
			std::string data = d.dump();
			d.clear();
			file.print(data.c_str());
			file.flush();
			file.close();
		}

	private:
		float _data[NSETTINGS];

		static constexpr int _firstStageDay = 20;
		static constexpr int _secondStageDay = 40;
		static constexpr int _thirdStageDay = 60;
		static constexpr float f_A = 1.5;
		static constexpr float f_B = 1;
		static constexpr float f_C = 0.5;
		static constexpr float f_1 = 0.3;
		static constexpr float f_2 = 0.8;
		static constexpr float f_3 = 1.2;
		static constexpr float EC_1 = 0.3;
		static constexpr float EC_2 = 0.8;
		static constexpr float EC_3 = 1.2;
} g_data;

#endif
