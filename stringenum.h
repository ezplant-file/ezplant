#ifndef _STRINGS_ENUM_
#define _STRINGS_ENUM_



typedef enum {
	NO_STRING,
	EMPTY_STR,
	BLUE_BTN_NEXT,
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
	SEC,
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

	END_OF_STRINGS
} dispStrings_t;

#endif
