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
	NTYPES
} rig_type;

rig_type g_rig_type = RIG_DEEPWATER;

// TODO: save g_data to file, populate g_data on startup from file
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
			d[STR(LIGHT_ON)] = bool(_data[LIGHT_ON]);

			d[STR(LIGHT_ON)] = bool(_data[LIGHT_ON]);
			d[STR(LIGHT_FROM)] = int(_data[LIGHT_FROM]);
			d[STR(LIGHT_TO)] = int(_data[LIGHT_TO]);
			d[STR(LIGHT_DAY)] = int(_data[LIGHT_DAY]);

			d[STR(VENT_ON)] = bool(_data[VENT_ON]);
			d[STR(VENT_TIME_LIM)] = bool(_data[VENT_TIME_LIM]);
			d[STR(VENT_TIME_FROM)] = int(_data[VENT_TIME_FROM]);
			d[STR(VENT_TIME_TO)] = int(_data[VENT_TIME_TO]);
			d[STR(VENT_TEMP_LIM)] = bool(_data[VENT_TEMP_LIM]);
			d[STR(VENT_TEMP_THRES)] = int(_data[VENT_TEMP_THRES]);
			d[STR(VENT_HUM_LIM)] = bool(_data[VENT_HUM_LIM]);
			d[STR(VENT_HUM_THRES)] = int(_data[VENT_HUM_THRES]);

			d[STR(PASSVENT)] = bool(_data[PASSVENT]);
			d[STR(PASSVENT_TIME_LIM)] = bool(_data[PASSVENT_TIME_LIM]);
			d[STR(PASSVENT_TIME_FROM)] = int(_data[PASSVENT_TIME_FROM]);
			d[STR(PASSVENT_TIME_TO)] = _data[PASSVENT_TIME_TO];
			d[STR(PASSVENT_TEMP_LIM)] = bool(_data[PASSVENT_TEMP_LIM]);
			d[STR(PASSVENT_TEMP_THRES)] = int(_data[PASSVENT_TEMP_THRES]);
			d[STR(PASSVENT_HUM_LIM)] = bool(_data[PASSVENT_HUM_LIM]);
			d[STR(PASSVENT_HUM_THRES)] = int(_data[PASSVENT_HUM_THRES]);

			d[STR(GR_CYCL_1_DAYS)] = int(_data[GR_CYCL_1_DAYS]);
			d[STR(GR_CYCL_2_DAYS)] = int(_data[GR_CYCL_2_DAYS]);
			d[STR(GR_CYCL_3_DAYS)] = int(_data[GR_CYCL_3_DAYS]);

			d[STR(EC_ON)] = bool(_data[EC_ON]);
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
			d[STR(EC_PUMPS)] = int(_data[EC_PUMPS]);

			d[STR(ACID_ON)] = bool(_data[ACID_ON]);
			d[STR(ACID_1)] = _data[ACID_1];
			d[STR(ACID_2)] = _data[ACID_2];
			d[STR(ACID_3)] = _data[ACID_3];
			d[STR(ACID_PUMPS)] = int(_data[ACID_PUMPS]);

			d[STR(PUMP_OFF)] = bool(_data[PUMP_OFF]);
			d[STR(PUMP_SEC)] = int(_data[PUMP_SEC]);

			d[STR(AERO_ON)] = bool(_data[AERO_ON]);
			d[STR(AERO_PUMP)] = bool(_data[AERO_PUMP]);
			d[STR(AERO_PUMP_SEC)] = int(_data[AERO_PUMP_SEC]);

			d[STR(STIR_ON)] = bool(_data[STIR_ON]);
			d[STR(STIR_PUMP)] = bool(_data[STIR_PUMP]);
			d[STR(STIR_PUMP_SEC)] = int(_data[STIR_PUMP_SEC]);

			d[STR(FLOOD_HOURS)] = int(_data[FLOOD_HOURS]);
			d[STR(FLOOD_MIN)] = int(_data[FLOOD_MIN]);
			d[STR(FLOOD_HOLD_MIN)] = int(_data[FLOOD_HOLD_MIN]);
			d[STR(FLOOD_HOLD_SEC)] = int(_data[FLOOD_HOLD_SEC]);

			d[STR(SPRAY_PUMP)] = bool(_data[SPRAY_PUMP]);
			d[STR(SPRAY_PUMP_SEC)] = int(_data[SPRAY_PUMP_SEC]);
			d[STR(SPRAY_CONS)] = bool(_data[SPRAY_CONS]);
			d[STR(SPRAY_CYCL)] = bool(_data[SPRAY_CYCL]);
			d[STR(SPRAY_MIN)] = int(_data[SPRAY_MIN]);
			d[STR(SPRAY_SEC)] = int(_data[SPRAY_SEC]);
			d[STR(SPRAY_CYCL_MIN)] = int(_data[SPRAY_CYCL_MIN]);
			d[STR(SPRAY_CYCL_SEC)] = int(_data[SPRAY_CYCL_SEC]);

			d[STR(DRIP_PUMP)] = bool(_data[DRIP_PUMP]);
			d[STR(DRIP_PUMP_SEC)] = int(_data[DRIP_PUMP_SEC]);
			d[STR(DRIP_CONS)] = bool(_data[DRIP_CONS]);
			d[STR(DRIP_CYCL)] = bool(_data[DRIP_CYCL]);
			d[STR(DRIP_MIN)] = int(_data[DRIP_MIN]);
			d[STR(DRIP_SEC)] = int(_data[DRIP_SEC]);
			d[STR(DRIP_CYCL_MIN)] = int(_data[DRIP_CYCL_MIN]);
			d[STR(DRIP_CYCL_SEC)] = int(_data[DRIP_CYCL_SEC]);

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
