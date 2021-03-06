#ifndef _RUSTRINGS_H_
#define _RUSTRINGS_H_
#include "stringenum.h"


const char* ruStrings[END_OF_STRINGS] PROGMEM = {
	"Строка не установлена",
	"",
	">",
	"день",
	"Далее",
	"1.",
	"2.",
	"3.",
	"EC",
	"pH",
	"PH:",
	"EC:",
	"минут",
	"секунд",
	"Пауза",
	"Старт",
	"...",
	"Постоянно",
	"Циклически",
	// main menu strings
	"Меню",
	"Новая посадка",
	"Текущая посадка",
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
	"Доп. настройки",

	// display/lang settings
	"Экран",
	"Яркость подсветки дисплея",
	"%",
	"Спящий режим при бездействии",
	"через",
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
	"password",
	"Подключитесь к ней, после чего\nоткройте в браузере сайт",
	"http://192.168.0.1",
	"Следуйте\nинструкциям.",

	// wifi settings
	"WiFi",
	"Wi-Fi",
	"Для выгрузки статистики,\nполучения обновлений\nи синхронизации времени",
	"Текущие настройки",
	"Соединение установлено",
	"Нет соединения",
	"Изменить",

	// date and time settings
	"Дата и время",
	"Автоматическая синхро- \nнизация времени из \nинтернета",
	"Часовой пояс",
	"UTC",
	"Текущие время и дата",
	"Настройка вручную",
	":",
	".",

	// utility
	"Страница не найдена",

	/* Calib pages */
	"Калибровка датчиков",
	"Датчик EC/TDS",
	"Датчик pH",
	"Подробная инструкция\nпо калибровке и приготовлению\nкалибровочных растворов",

	"Калибровка прошла успешно",
	"Готово",
	"Внимание! Отменить калиб-\nровку будет невозможно",

	"Калибровка pH",
	"Калибровка TDS",

	/* Ph calib pages */
	// ph1
	"1. Приготовьте калибровочные \nжидкости 4,0pH и 9,18pH",
	"2. Очистите датчик и сполосните\nего в дистилллированной воде.",

	// ph2
	"3. Погрузите датчик в жидкость\n4,0pH и нажмите кнопку",
	"Сканировать 4,0 pH",

	// ph3
	"4. Сейчас происходит\nсканирование показаний.\nЭто займёт около минуты.",

	// ph4
	"5. Сполосните датчик в дистил-\nлированной воде.",
	"6. Поместите датчик в жидкость с\nкислотностью 9,18pH.",
	"Сканировать 9,18 pH",

	// ph5
	"7. Сейчас происходит\nсканирование показаний.\nЭто займёт около минуты.",

	// ph/tds fail
	"Невозможно инициализировать\nдатчик. Проверьте подключение\nили замените датчик.",

	/* TDS calib pages */
	// tds1
	"1. Приговтовьте клибровочные\nжидкости 500 и 1500 ppm.",
	// tds2
	"3. Погрузите датчик в жидкость\n500 ppm и нажмите кнопку.",
	"Сканировать 500 ppm",
	// tds4
	"6. Погрузите датчик в жидкость\n1500 ppm и нажмите кнопку.",
	"Сканировать 1500 ppm",

	/* diag menu */

	"Выходы силовые",
	"Входы цифровые",
	"Входы аналоговые",
	"Диагностика датчиков",

	// TDS diag
	"Отображение показаний \nв реальном времени \nо содержании растворенных \nтвердых веществ",
	"ppm",

	// pH diag
	"Отображение водородного \nпоказателя в реальном времени",
	"pH",

	// analog inputs
	"Отображение аналоговых \nсигналов датчиков \nв интервале 0-1023",
	"ANALOG 1",
	"ANALOG 2",
	"ANALOG 3",
	"ANALOG 4",

	// digital inputs
	"Отображение сигналов датчиков",
	"KEY 1",
	"KEY 2",
	"KEY 3",
	"KEY 4",
	"KEY 5",
	"KEY 6",
	"KEY 7",
	"KEY 8",
	"KEY 9",
	"KEY 10",

	// power outputs
	"Включение устройств вручную",
	"PORT A",
	"PORT B",
	"PORT C",
	"PORT D",
	"PORT E",
	"PORT F",
	"PORT G",
	"PORT H",
	"FAN",
	"LIGHT",
	"MOTOR:",
	"UP",
	"DOWN",

	// first page items
	"Состояние",
	"Первый посев",
	"Для начала работы необходимо \nперейти к настройке первой \nпосадки и задать все необхо- \nдимые параметры",
	"Начать",

	// stage 1
	"Посадка. Этап 1",
	"Тип системы",
	"Глубоководные культуры",
	"Питательный слой",
	"Периодическое затопление",
	"Аэропоника",
	"Капельная гидропоника",
	"Открытый грунт",
	"Теплица",
	"Приготовление раствора",

	// stage 2
	"Посадка. Этап 2",
	"Освещение",
	"Задаём временной \nинтервал, в который \nбудет производиться \nдосвет",
	"С какого дня досвет",
	"Указываем, с какого дня \nс момента посадки начать \nдосвечивать растения",
	"С",
	"дня",

	// stage 3
	"Посадка. Этап 3",
	"Обдув растений",
	"Режим",
	// constant
	"По какому условию",
	"По времени",
	"При температуре больше",
	"При влажности больше",
	// cyclic
	"Параметры",
	"Время работы",
	"Длит. обдува",
	"Длит. паузы",

	// stage 4
	"Посадка. Этап 4",
	"Проветривание",
	"Открытие окна или двери \nлинейным приводом",
	"По какому условию",
	"Времменой интервал",
	"Если температура больше",
	"Если влажность больше",

	// stage 5
	"Посадка. Этап 5",
	"Цикл роста",
	"Параметры пит. раствора \nв зависимости от стадии роста.",

	// stage 6
	"Посадка. Этап 6",
	"Концентрация (EC)",
	"Задайте пропорции растворов",
	"Время работы насосов (размер \nпорции)",
	"A", "B", "C",

	// stage 7
	"Посадка. Этап 7",
	"Кислотность (pH)",
	"Задайте целевую кислотность и \nразмер порции.",

	// stage 8
	"Посадка. Этап 8",
	"Отключение насоса \nпри измерении",
	"Рекомендуется для прекраще- \nния движения жидкости \nи получения более точных \nрезультатов измерения",
	"Отключать при измерении",
	"за",
	"сек. до измерения",

	// stage 9
	"Посадка. Этап 9",
	"Аэрация жидкости",
	"Подача воздуха \nк корням растений",
	"Отключать при измерении",
	"за",
	"сек. до измерения",

	// stage 9_2 (stage 2)
	"Посадка. Этап 9",
	"Перемешивание \nжидкости",
	"Перемешивание питательного \nраствора при помощи мешалки \nили другим способом",
	"Отключать при измерении",
	"за",
	"сек. до измерения",

	// stage 8_2 (stage 2)
	"Посадка. Этап 8",
	"Заполнение резервуара \nдля растений",
	"Затопление растений один раз \nза указанный период",
	"Удержание питательного раствора",

	// stage 8_3 (stage 2)
	"Посадка. Этап 8",
	"Орошение форсунками",
	"Отключать при измерении",
	"за",
	"сек. до измерения",
	"Режим орошения",
	"Подача жидкости в течении",
	"Цикличность подачи жидкости",

	// stage 8_4
	"Посадка. Этап 8",
	"Капельный полив",
	"Режим полива",

	// main page string
	"Показание от ",

	// debug measure
	"стадия",

	// addit. settings page 1
	"Мощность освещения",
	"Разрешённое время \nдля нормализации раствора",
	"Разрешённое (аварийное) \nвремя долива воды",
	"Период измерения pH и EC",

	// addit settings page 2
	"Гистересис pH",
	"Гистересис EC",
	"Текущий день посадки",
	"Интервал между доб. \nконцентратов, сек",
	"Диапазон значений, которые \nсчитать корректными:",

	// online monitoring
	"Мониторинг",
	"Онлайн мониторинг",
	"Зарегестрируйте \nустройство на сайте",
       	"https://dashboard.ezplant.ru",
	"Регистрация устройства",
	"Код устройства",
	"Запросить новый код через",
	"Соединение установлено",
	"Отвязать устройство",

	// socket class
	"Настройка розетки",
	"Выполняемая роль",
	"Розетка с идентификатором #",
	"Освещение",
	"Циркуляционный насос",
	"Аэрация",
	"Вентиляция",
	"Применить"

	//end of strings
};

#endif
