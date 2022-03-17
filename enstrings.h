#ifndef _ENGSTRINGS_H_
#define _ENGSTRINGS_H_
#include "stringenum.h"


const char* engStrings[END_OF_STRINGS] PROGMEM = {
	"String not set",
	// main menu strings
	"Menu",
	"New planting",
	"Online monitoring",
	"Settings",
	"Diagnostics",
	"Test page",
	"Fonts",

	// settings strings
	"Time and date",
	"Wi-Fi",
	"Screen and language",
	"Sensors calibration",
	"Threshold settings",

	// display/lang settings
	"Screen",
	"Display brightness",
	"%",
	"Dim screen after",
	"seconds",
	"Language",
	"Русский",
	"English",

	// test page
	"Toggle",
	"Checkbox",
	"Radio button",
	"Input field",
	"Grey menu button",
	"Blue button",
	"Please wait...",

	// font page
	"Font SegoeUI-12",
	"Font SegoeUI-14",
	"Font SegoeUI-16",
	"Font SegoeUI-18",
	"Font SegoeUI-20",
	"Font SegoeUI-Bold-16",
	"Font SegoeUI-Bold-18",

	// wifi init
	"WiFi settings",
	"Access point created",
	"WiFi name",
	"ezplant_wifi",
	"Password",
	"ezplant",
	"Open this site after\njoining access point",
	"http://192.168.0.1",
	"and follow\ninstructions",

	// wifi settings
	"WiFi",
	"Wi-Fi",
	"For data upload,\nupdates and\ntime sync",
	"Current settings",
	"Connection OK",
	"No connection",
	"Change",

	// date and time settings
	"Date and time",
	"Sync time from\ninternet",
	"Timezone",
	"UTC",
	"Current time and date",
	"Manual setup"

	//end of strings
};

#endif
