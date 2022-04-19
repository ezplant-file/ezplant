#ifndef _STRINGS_ENUM_
#define _STRINGS_ENUM_



typedef enum {
	NO_STRING,
	EMPTY_STR,
	MORE_THAN,
	TXT_DAY,
	BLUE_BTN_NEXT,
	BULL_1,
	BULL_2,
	BULL_3,
	TXT_EC,
	TXT_PH,
	// main menu strings
	MENU,
	NEW_PLANT,
	ONLINE_MON,
	SETTINGS,
	DIAG,
	TEST_PAGE,
	FONT_PAGE,

	// settings strings
	TIMEDATE,
	WIFI,
	SCREENLANG,
	CALIB,
	THRES,

	// display/lang settings
	SCREEN,
	BRIGHT,
	PERCENT,
	DIM,
	AFTER,
	TXT_SEC,
	LANG,
	RUS,
	ENG,

	// test page
	TOGGLE_TEXT,
	CHECHBOX_TEXT,
	RADIO_TEXT,
	INPUT_TEXT,
	GREY_BUTTON,
	BLUE_BUTTON,
	WAIT_TEXT,

	// font page
	SMALLESTFONT_TEXT,
	SMALLFONT_TEXT,
	MIDFONT_TEXT,
	LARGEFONT_TEXT,
	LARGESTFONT_TEXT,
	BOLDFONT_TEXT,
	BOLDFONT2_TEXT,

	// wifi init
	WI_TITLE,
	WI_AP_CREATED,
	WI_SSID_TEXT,
	WI_SSID_NAME,
	WI_PWD_TEXT,
	WI_PASSWORD,
	WI_CONNECT,
	WI_IP,
	WI_FOLLOW,

	// wifi settings
	WS_TITLE,
	WS_CHECK,
	WS_PAR,
	WS_SUBT,
	WS_SUCC,
	WS_FAIL,
	WS_CHANGE,

	//date and time settings
	DT_TITLE,
	DT_SYNC,
	DT_ZONE,
	DT_UTC,
	DT_CURR,
	DT_MANUAL,
	DT_SEMI,
	DT_DOT,

	// utility
	NOT_FOUND,

	/* Calib pages */
	CAL_TITLE,
	CAL_TDS,
	CAL_PH,
	CAL_PAR1,

	CAL_SUCC,
	CAL_DONE,
	CAL_WARN,

	CAL_PH_TITLE,
	CAL_TDS_TITLE,

	/* pH calib pages */
	// ph1
	PH1_PAR1,
	PH1_PAR2,

	// ph2
	PH2_PAR1,
	CAL_SCAN_4,

	// ph3
	PH3_PAR1,

	// ph4
	PH4_PAR1,
	PH4_PAR2,
	CAL_SCAN_9,

	// ph5
	PH5_PAR1,

	// ph fail
	SENS_FAIL,

	/* TDS calib pages */
	// tds1
	TDS1_PAR1,
	// tds2
	TDS2_PAR1,
	TDS2_SCAN_500,
	// tds4
	TDS4_PAR2,
	TDS4_SCAN_1500,

	/* diag menu */
	// diag title from menu page // DIAG
	DIAG_PWR,
	DIAG_DIG,
	DIAG_ADC,
	DIAG_SENS,

	// TDS diag
	TDS_DIAG_PAR,
	TDS_DIAG_PPM,

	// pH diag
	PH_DIAG_PAR,
	PH_DIAG_PH,

	// analog inputs
	SENS_DIAG_PAR,
	SENS_DIAG_A1,
	SENS_DIAG_A2,
	SENS_DIAG_A3,
	SENS_DIAG_A4,

	// digital inputs
	DIG_DIAG_PAR,
	DIG_DIAG_1,
	DIG_DIAG_2,
	DIG_DIAG_3,
	DIG_DIAG_4,
	DIG_DIAG_5,
	DIG_DIAG_6,
	DIG_DIAG_7,
	DIG_DIAG_8,
	DIG_DIAG_9,
	DIG_DIAG_10,

	// power outputs
	PWR_PAR,
	PWR_PORT_A,
	PWR_PORT_B,
	PWR_PORT_C,
	PWR_PORT_D,
	PWR_PORT_E,
	PWR_PORT_F,
	PWR_PORT_G,
	PWR_PORT_H,
	PWR_FAN,
	PWR_LIGHT,
	PWR_MOTOR_STR,
	PWR_MOTOR_UP,
	PWR_MOTOR_DOWN,

	// fist page items
	FP_TITLE,
	FP_SUBTTL,
	FP_PAR,
	FP_BTN,

	// stage 1
	S1_TITLE,
	S1_SUBTTL,
	S1_UNDERWTR,
	S1_LAYER,
	S1_PERIODIC,
	S1_AERO,
	S1_DRIP,
	S1_OPENG,
	S1_GREENHS,
	S1_MIXING,

	// stage 2
	S2_TITLE,
	S2_SUBTTL1,
	S2_PAR1,
	S2_SUBTTL2,
	S2_PAR2,
	S2_FROM,
	S2_DAY,

	// stage 3
	S3_TITLE,
	S3_VENT,
	S3_PAR1,
	S3_SUBTTL,
	S3_TIME,
	S3_TEMP,
	S3_HUM,

	// stage 4
	S4_TITLE,
	S4_PASSVENT,
	S4_PAR1,
	S4_SUBTTL,
	S4_TIME,
	S4_TEMP,
	S4_HUM,

	// stage 5
	S5_TITLE,
	S5_SUBTTL,
	S5_PAR1,

	// stage 6
	S6_TITLE,
	S6_SUBTTL,
	S6_PAR1,
	S6_PAR2,
	TXT_A, TXT_B, TXT_C,

	// stage 7
	S7_TITLE,
	S7_SUBTTL,
	S7_PAR1,

	// stage 8
	S8_TITLE,
	S8_SUBTTL,
	S8_PAR1,
	S8_CHECK,
	S8_INTXT,
	S8_INPUT,

	// stage 9
	S9_TITLE,
	S9_SUBTTL,
	S9_PAR1,
	S9_CHECK,
	S9_INTXT,
	S9_INPUT,

	END_OF_STRINGS
} dispStrings_t;

#endif
