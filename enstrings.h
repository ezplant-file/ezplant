#ifndef _ENGSTRINGS_H_
#define _ENGSTRINGS_H_
#include "stringenum.h"


const char* engStrings[END_OF_STRINGS] PROGMEM = {
	"String not set",
	"",
	"Next",
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
	"Dim screen",
	"after",
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
	"password",
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
	"Sync time with\ninternet",
	"Timezone",
	"UTC",
	"Current time and date",
	"Manual setup",
	":",
	".",

	// utility
	"Page not found",

	/* Calib pages */
	"Sensor calibration",
	"EC/TDS meter",
	"pH meter",
	"Extensive instructions\non calibration and making\nof calibration liquids",

	"Successful calibration",
	"Done",
	"Warning! This process is\nirreversible.",

	"pH calibration",
	"TDS calibration",

	/* Ph calib pages */
	// ph1
	"1. Prepare calibration liquids 4.0pH and 9.18pH",
	"2. Clean the sensor and rinse\nit in distilled water",

	// ph2
	"3. Dip the sensor in liquid\n%f%s and press the button",
	"Scan 4.0pH",

	// ph3
	"4. Taking measurements.\nIt will take a minute",

	// ph4
	"5. Rinse the sensor\nin distilled water",
	"6. Dip the sensor in\n 9.18pH liquid",
	"Scan 9.18 pH",

	// ph5
	"7. Taking measurements.\nIt will take a minute"

	/* TDS calib pages */

	//end of strings
};

#endif
