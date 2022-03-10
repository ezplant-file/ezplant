#ifndef _STRINGS_ENUM_
#define _STRINGS_ENUM_



typedef enum {
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
	DIMAFTER,
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

	END_OF_STRINGS
} dispStrings_t;

#endif
