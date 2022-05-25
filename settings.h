#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "json.hpp"
#include <SPIFFS.h>
#define STR(A) (#A)

const char* data_file = "/data";

using json = nlohmann::json;

// all settings
typedef enum {
	SETT_EMPTY,
	// stage 2 settings
	LIGHT_ON,
	LIGHT_FROM,
	LIGHT_TO,
	LIGHT_DAY,

	// stage 3 settings
	VENT_ON,
	VENT_TIME_LIM,
	VENT_TIME_FROM,
	VENT_TIME_TO,
	VENT_TEMP_LIM,
	VENT_TEMP_THRES,
	VENT_HUM_LIM,
	VENT_HUM_THRES,

	// stage 4 settings
	PASSVENT,
	PASSVENT_TIME_LIM,
	PASSVENT_TIME_FROM,
	PASSVENT_TIME_TO,
	PASSVENT_TEMP_LIM,
	PASSVENT_TEMP_THRES,
	PASSVENT_HUM_LIM,
	PASSVENT_HUM_THRES,

	// stage 5 settings
	GR_CYCL_1_DAYS,
	GR_CYCL_2_DAYS,
	GR_CYCL_3_DAYS,

	// stage 6 settings
	EC_ON,
	EC_CYCL1,
	EC_A1,
	EC_B1,
	EC_C1,
	EC_CYCL2,
	EC_A2,
	EC_B2,
	EC_C2,
	EC_CYCL3,
	EC_A3,
	EC_B3,
	EC_C3,
	EC_PUMPS,

	// stage 7 settings
	ACID_ON,
	ACID_1,
	ACID_2,
	ACID_3,
	ACID_PUMPS,

	// stage 8 settings
	PUMP_OFF,
	PUMP_SEC,

	// stage 9 settings
	AERO_ON,
	AERO_PUMP,
	AERO_PUMP_SEC,

	// stage 9_2 settings
	STIR_ON,
	STIR_PUMP,
	STIR_PUMP_SEC,

	// stage 8_2 settings
	FLOOD_HOURS,
	FLOOD_MIN,
	FLOOD_HOLD_MIN,
	FLOOD_HOLD_SEC,

	// stage 8_3 settings
	SPRAY_PUMP,
	SPRAY_PUMP_SEC,
	SPRAY_CONS,
	SPRAY_CYCL,
	SPRAY_MIN,
	SPRAY_SEC,
	SPRAY_CYCL_MIN,
	SPRAY_CYCL_SEC,

	// stage 8_4 settings
	DRIP_PUMP,
	DRIP_PUMP_SEC,
	DRIP_CONS,
	DRIP_CYCL,
	DRIP_MIN,
	DRIP_SEC,
	DRIP_CYCL_MIN,
	DRIP_CYCL_SEC,

	// additional settings
	ADD_LED_BRIGHT,
	START_DAY,
	EC_HYST,
	PH_HYST,
	PUMP_TIME, // allowed H20 pump time

	NSETTINGS
} rig_settings_t;

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
			set(ADD_LED_BRIGHT, 127);
			set(EC_HYST, 0.2f);
			set(PH_HYST, 0.5f);
			set(EC_PUMPS, 10);
			set(ACID_PUMPS, 10);
			set(LIGHT_FROM, 8);
			set(LIGHT_TO, 23);
			set(LIGHT_DAY, 3);
			set(VENT_TIME_FROM, 8);
			set(VENT_TIME_TO, 23);
			set(VENT_TEMP_THRES, 30);
			set(VENT_HUM_THRES, 60);
			set(PASSVENT_TIME_FROM, 8);
			set(PASSVENT_TIME_TO, 23);
			set(PASSVENT_TEMP_THRES, 30);
			set(PASSVENT_HUM_THRES, 60);
			set(PUMP_SEC, 15);
			set(AERO_PUMP_SEC, 15);
			set(PUMP_TIME, 15);
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
				_data[LIGHT_ON] = load[STR(LIGHT_ON)].get<float>();
				_data[LIGHT_FROM] = load[STR(LIGHT_FROM)].get<float>();
				_data[LIGHT_TO] = load[STR(LIGHT_TO)].get<float>();
				_data[LIGHT_DAY] = load[STR(LIGHT_DAY)].get<float>();
				_data[VENT_ON] = load[STR(VENT_ON)].get<float>();
				_data[VENT_TIME_LIM] = load[STR(VENT_TIME_LIM)].get<float>();
				_data[VENT_TIME_FROM] = load[STR(VENT_TIME_FROM)].get<float>();
				_data[VENT_TIME_TO] = load[STR(VENT_TIME_TO)].get<float>();
				_data[VENT_TEMP_LIM] = load[STR(VENT_TEMP_LIM)].get<float>();
				_data[VENT_TEMP_THRES] = load[STR(VENT_TEMP_THRES)].get<float>();
				_data[VENT_HUM_LIM] = load[STR(VENT_HUM_LIM)].get<float>();
				_data[VENT_HUM_THRES] = load[STR(VENT_HUM_THRES)].get<float>();
				_data[PASSVENT] = load[STR(PASSVENT)].get<float>();
				_data[PASSVENT_TIME_LIM] = load[STR(PASSVENT_TIME_LIM)].get<float>();
				_data[PASSVENT_TIME_FROM] = load[STR(PASSVENT_TIME_FROM)].get<float>();
				_data[PASSVENT_TIME_TO] = load[STR(PASSVENT_TIME_TO)].get<float>();
				_data[PASSVENT_TEMP_LIM] = load[STR(PASSVENT_TEMP_LIM)].get<float>();
				_data[PASSVENT_TEMP_THRES] = load[STR(PASSVENT_TEMP_THRES)].get<float>();
				_data[PASSVENT_HUM_LIM] = load[STR(PASSVENT_HUM_LIM)].get<float>();
				_data[PASSVENT_HUM_THRES] = load[STR(PASSVENT_HUM_THRES)].get<float>();
				_data[GR_CYCL_1_DAYS] = load[STR(GR_CYCL_1_DAYS)].get<float>();
				_data[GR_CYCL_2_DAYS] = load[STR(GR_CYCL_2_DAYS)].get<float>();
				_data[GR_CYCL_3_DAYS] = load[STR(GR_CYCL_3_DAYS)].get<float>();
				_data[EC_ON] = load[STR(EC_ON)].get<float>();
				_data[EC_CYCL1] = load[STR(EC_CYCL1)].get<float>();
				_data[EC_A1] = load[STR(EC_A1)].get<float>();
				_data[EC_B1] = load[STR(EC_B1)].get<float>();
				_data[EC_C1] = load[STR(EC_C1)].get<float>();
				_data[EC_CYCL2] = load[STR(EC_CYCL2)].get<float>();
				_data[EC_A2] = load[STR(EC_A2)].get<float>();
				_data[EC_B2] = load[STR(EC_B2)].get<float>();
				_data[EC_C2] = load[STR(EC_C2)].get<float>();
				_data[EC_CYCL3] = load[STR(EC_CYCL3)].get<float>();
				_data[EC_A3] = load[STR(EC_A3)].get<float>();
				_data[EC_B3] = load[STR(EC_B3)].get<float>();
				_data[EC_C3] = load[STR(EC_C3)].get<float>();
				_data[EC_PUMPS] = load[STR(EC_PUMPS)].get<float>();
				_data[ACID_ON] = load[STR(ACID_ON)].get<float>();
				_data[ACID_1] = load[STR(ACID_1)].get<float>();
				_data[ACID_2] = load[STR(ACID_2)].get<float>();
				_data[ACID_3] = load[STR(ACID_3)].get<float>();
				_data[ACID_PUMPS] = load[STR(ACID_PUMPS)].get<float>();
				_data[PUMP_OFF] = load[STR(PUMP_OFF)].get<float>();
				_data[PUMP_SEC] = load[STR(PUMP_SEC)].get<float>();
				_data[AERO_ON] = load[STR(AERO_ON)].get<float>();
				_data[AERO_PUMP] = load[STR(AERO_PUMP)].get<float>();
				_data[AERO_PUMP_SEC] = load[STR(AERO_PUMP_SEC)].get<float>();
				_data[STIR_ON] = load[STR(STIR_ON)].get<float>();
				_data[STIR_PUMP] = load[STR(STIR_PUMP)].get<float>();
				_data[STIR_PUMP_SEC] = load[STR(STIR_PUMP_SEC)].get<float>();
				_data[FLOOD_HOURS] = load[STR(FLOOD_HOURS)].get<float>();
				_data[FLOOD_MIN] = load[STR(FLOOD_MIN)].get<float>();
				_data[FLOOD_HOLD_MIN] = load[STR(FLOOD_HOLD_MIN)].get<float>();
				_data[FLOOD_HOLD_SEC] = load[STR(FLOOD_HOLD_SEC)].get<float>();
				_data[SPRAY_PUMP] = load[STR(SPRAY_PUMP)].get<float>();
				_data[SPRAY_PUMP_SEC] = load[STR(SPRAY_PUMP_SEC)].get<float>();
				_data[SPRAY_CONS] = load[STR(SPRAY_CONS)].get<float>();
				_data[SPRAY_CYCL] = load[STR(SPRAY_CYCL)].get<float>();
				_data[SPRAY_MIN] = load[STR(SPRAY_MIN)].get<float>();
				_data[SPRAY_SEC] = load[STR(SPRAY_SEC)].get<float>();
				_data[SPRAY_CYCL_MIN] = load[STR(SPRAY_CYCL_MIN)].get<float>();
				_data[SPRAY_CYCL_SEC] = load[STR(SPRAY_CYCL_SEC)].get<float>();
				_data[DRIP_PUMP] = load[STR(DRIP_PUMP)].get<float>();
				_data[DRIP_PUMP_SEC] = load[STR(DRIP_PUMP_SEC)].get<float>();
				_data[DRIP_CONS] = load[STR(DRIP_CONS)].get<float>();
				_data[DRIP_CYCL] = load[STR(DRIP_CYCL)].get<float>();
				_data[DRIP_MIN] = load[STR(DRIP_MIN)].get<float>();
				_data[DRIP_SEC] = load[STR(DRIP_SEC)].get<float>();
				_data[DRIP_CYCL_MIN] = load[STR(DRIP_CYCL_MIN)].get<float>();
				_data[DRIP_CYCL_SEC] = load[STR(DRIP_CYCL_SEC)].get<float>();
				_data[ADD_LED_BRIGHT] = load[STR(ADD_LED_BRIGHT)].get<float>();
				_data[START_DAY] = load[STR(START_DAY)].get<float>();

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
			d[STR(LIGHT_ON)] = (_data[LIGHT_ON]);

			d[STR(LIGHT_ON)] = (_data[LIGHT_ON]);
			d[STR(LIGHT_FROM)] = (_data[LIGHT_FROM]);
			d[STR(LIGHT_TO)] = (_data[LIGHT_TO]);
			d[STR(LIGHT_DAY)] = (_data[LIGHT_DAY]);

			d[STR(VENT_ON)] = (_data[VENT_ON]);
			d[STR(VENT_TIME_LIM)] = (_data[VENT_TIME_LIM]);
			d[STR(VENT_TIME_FROM)] = (_data[VENT_TIME_FROM]);
			d[STR(VENT_TIME_TO)] = (_data[VENT_TIME_TO]);
			d[STR(VENT_TEMP_LIM)] = (_data[VENT_TEMP_LIM]);
			d[STR(VENT_TEMP_THRES)] = (_data[VENT_TEMP_THRES]);
			d[STR(VENT_HUM_LIM)] = (_data[VENT_HUM_LIM]);
			d[STR(VENT_HUM_THRES)] = (_data[VENT_HUM_THRES]);

			d[STR(PASSVENT)] = (_data[PASSVENT]);
			d[STR(PASSVENT_TIME_LIM)] = (_data[PASSVENT_TIME_LIM]);
			d[STR(PASSVENT_TIME_FROM)] = (_data[PASSVENT_TIME_FROM]);
			d[STR(PASSVENT_TIME_TO)] = _data[PASSVENT_TIME_TO];
			d[STR(PASSVENT_TEMP_LIM)] = (_data[PASSVENT_TEMP_LIM]);
			d[STR(PASSVENT_TEMP_THRES)] = (_data[PASSVENT_TEMP_THRES]);
			d[STR(PASSVENT_HUM_LIM)] = (_data[PASSVENT_HUM_LIM]);
			d[STR(PASSVENT_HUM_THRES)] = (_data[PASSVENT_HUM_THRES]);

			d[STR(GR_CYCL_1_DAYS)] = (_data[GR_CYCL_1_DAYS]);
			d[STR(GR_CYCL_2_DAYS)] = (_data[GR_CYCL_2_DAYS]);
			d[STR(GR_CYCL_3_DAYS)] = (_data[GR_CYCL_3_DAYS]);

			d[STR(EC_ON)] = (_data[EC_ON]);
			d[STR(EC_CYCL1)] = _data[EC_CYCL1];
			d[STR(EC_A1)] = _data[EC_A1];
			d[STR(EC_B1)] = _data[EC_B1];
			d[STR(EC_C1)] = _data[EC_C1];
			d[STR(EC_CYCL2)] = _data[EC_CYCL2];
			d[STR(EC_A2)] = _data[EC_A2];
			d[STR(EC_B2)] = _data[EC_B2];
			d[STR(EC_C2)] = _data[EC_C2];
			d[STR(EC_CYCL3)] = _data[EC_CYCL3];
			d[STR(EC_A3)] = _data[EC_A3];
			d[STR(EC_B3)] = _data[EC_B3];
			d[STR(EC_C3)] = _data[EC_C3];
			d[STR(EC_PUMPS)] = (_data[EC_PUMPS]);

			d[STR(ACID_ON)] = (_data[ACID_ON]);
			d[STR(ACID_1)] = _data[ACID_1];
			d[STR(ACID_2)] = _data[ACID_2];
			d[STR(ACID_3)] = _data[ACID_3];
			d[STR(ACID_PUMPS)] = (_data[ACID_PUMPS]);

			d[STR(PUMP_OFF)] = (_data[PUMP_OFF]);
			d[STR(PUMP_SEC)] = (_data[PUMP_SEC]);

			d[STR(AERO_ON)] = (_data[AERO_ON]);
			d[STR(AERO_PUMP)] = (_data[AERO_PUMP]);
			d[STR(AERO_PUMP_SEC)] = (_data[AERO_PUMP_SEC]);

			d[STR(STIR_ON)] = (_data[STIR_ON]);
			d[STR(STIR_PUMP)] = (_data[STIR_PUMP]);
			d[STR(STIR_PUMP_SEC)] = (_data[STIR_PUMP_SEC]);

			d[STR(FLOOD_HOURS)] = (_data[FLOOD_HOURS]);
			d[STR(FLOOD_MIN)] = (_data[FLOOD_MIN]);
			d[STR(FLOOD_HOLD_MIN)] = (_data[FLOOD_HOLD_MIN]);
			d[STR(FLOOD_HOLD_SEC)] = (_data[FLOOD_HOLD_SEC]);

			d[STR(SPRAY_PUMP)] = (_data[SPRAY_PUMP]);
			d[STR(SPRAY_PUMP_SEC)] = (_data[SPRAY_PUMP_SEC]);
			d[STR(SPRAY_CONS)] = (_data[SPRAY_CONS]);
			d[STR(SPRAY_CYCL)] = (_data[SPRAY_CYCL]);
			d[STR(SPRAY_MIN)] = (_data[SPRAY_MIN]);
			d[STR(SPRAY_SEC)] = (_data[SPRAY_SEC]);
			d[STR(SPRAY_CYCL_MIN)] = (_data[SPRAY_CYCL_MIN]);
			d[STR(SPRAY_CYCL_SEC)] = (_data[SPRAY_CYCL_SEC]);

			d[STR(DRIP_PUMP)] = (_data[DRIP_PUMP]);
			d[STR(DRIP_PUMP_SEC)] = (_data[DRIP_PUMP_SEC]);
			d[STR(DRIP_CONS)] = (_data[DRIP_CONS]);
			d[STR(DRIP_CYCL)] = (_data[DRIP_CYCL]);
			d[STR(DRIP_MIN)] = (_data[DRIP_MIN]);
			d[STR(DRIP_SEC)] = (_data[DRIP_SEC]);
			d[STR(DRIP_CYCL_MIN)] = (_data[DRIP_CYCL_MIN]);
			d[STR(DRIP_CYCL_SEC)] = (_data[DRIP_CYCL_SEC]);
			d[STR(ADD_LED_BRIGHT)] = (_data[ADD_LED_BRIGHT]);
			d[STR(START_DAY)] = (_data[START_DAY]);

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
