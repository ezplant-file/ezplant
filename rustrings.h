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
	"Тест-страница",
	"Шрифты",

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
	"English",

	// test page
	"Переключатель",
	"Галка",
	"Радио-кнопка",
	"Поле ввода",
	"Пункт меню",
	"Синяя кнопка",
	"Ожидайте...",

	// font page
	"Шрифт SegoeUI-12",
	"Шрифт SegoeUI-14",
	"Шрифт SegoeUI-16",
	"Шрифт SegoeUI-18",
	"Шрифт SegoeUI-20",
	"Шрифт SegoeUI-Bold-16",
	"Шрифт SegoeUI-Bold-18",

	// wifi init
	"Настройка WiFi",
	"Создана точка доступа",
	"Имя сети",
	"ezplant_wifi",
	"Пароль",
	"ezplant",
	"Подключитесь к ней, после чего\n откройте в браузере сайт",
	"http://192.168.0.1",
	"Следуйте\n инструкциям."
		
	//end of strings
};

#endif
