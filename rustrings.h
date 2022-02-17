#ifndef _RUSTRINGS_H_
#define _RUSTRINGS_H_
#include "stringenum.h"


const char* ruStrings[END_OF_STRINGS] PROGMEM = {
	// main menu strings
	"Меню",
	"Новая посадка",
	"Онлайн мониторинг",
	"Настройки",
	"Диагностика оборудования",

	// settings strings
	"Дата и время",
	"Wi-Fi",
	"Экран и язык интерфейса",
	"Калибровка датчиков",
	"Настройки порогов",

	// display/lang settings
	"Экран",
	"Яркость подсветки дисплея",
	"%",
	"Спящий режим при бездействии через",
	"секунд",
	"Язык интерфейса",
	"Русский",
	"English"
};

#endif
