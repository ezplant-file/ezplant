#ifndef __SETTINGS_H__
#define __SETTINGS_H__

typedef enum {
	SETT_EMPY,
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

	NSETTINGS
} rig_settings_t;

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
			/*
			set(EC_CYCL1, _1);
			set(EC_CYCL2, _2);
			set(EC_CYCL3, _3);
			*/
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
} g_data;

#endif
