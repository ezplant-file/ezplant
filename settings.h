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
} plant_settings_t;

#endif
