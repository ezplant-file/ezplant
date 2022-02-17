#ifndef _ENGSTRINGS_H_
#define _ENGSTRINGS_H_
#include "stringenum.h"


const char* engStrings[END_OF_STRINGS] PROGMEM = {
	// main menu strings
	"Menu",
	"New planting",
	"Online monitoring",
	"Settings",
	"Diagnostics",

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
	"English"
};

#endif