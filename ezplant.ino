//TODO: consider resource manager and page builder classes

//#define TASKS

// buttons defines
#define PREV 18
#define NEXT 23
#define OK 5

// GUI & strings
#include "GfxUi.h"
#include "rustrings.h"
#include "enstrings.h"
#include "stringenum.h"
#include "Gui.h"


// prevent redrawing control buttons... Maybe make control bar object?
bool gBackBtnOnScreen = false;
bool gFwdBtnOnScreen = false;

static Page topBar;
static SimpleBox topBox;
static Image statusWIFI;
static Image statusInternet;
static Text menuText;

void buildTopBar()
{
	topBox.setColor(greyscaleColor(TOP_BAR_BG_COL));
	topBox.setWH(SCR_WIDTH, TOP_BAR_HEIGHT);
	topBox.invalidate();
	menuText.setFont(LARGEFONT);
	menuText.setXYpos(LEFTMOST, TOPMOST);
	menuText.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(TOP_BAR_BG_COL));
	menuText.setText(MENU);
	menuText.prepare();

	statusWIFI.loadRes("/wifi_ok.jpg");
	statusWIFI.setXYpos(213, 0);
	statusWIFI.freeRes();

	statusInternet.loadRes("/internet_ok.jpg");
	statusInternet.setXYpos(186, 0);
	statusInternet.freeRes();

	topBar.addItem(&topBox);
	topBar.addItem(&menuText);
	topBar.addItem(&statusInternet);
	topBar.addItem(&statusWIFI);
}

static Page mainPage;
static Page settingsPage;

void cursorDraw(bool);
bool gRapidBlink = false;

void rapidblink(void* par)
{
	gRapidBlink = true;
	cursorDraw(false);
	sleep(50);
	cursorDraw(true);
	sleep(50);
	cursorDraw(false);
	sleep(50);
	cursorDraw(true);
	sleep(50);
	cursorDraw(false);
	sleep(50);
	cursorDraw(true);
	sleep(50);
	cursorDraw(false);
	gRapidBlink = false;
	vTaskDelete(NULL);
}

/***********************************************************************
  Lang select and screen settings page
************************************************************************/

static Page langPage;

static CircRadBtn ruSelect;
static CircRadBtn enSelect;

void changeLangRus()
{
	gBackBtnOnScreen = true;
	ruSelect.on(true);
	enSelect.on(false);

	scrStrings = ruStrings;

	menuText.erase();
	//menuText.invalidate();
	topBar.invalidateAll();
	//menuText.setText(SCREENLANG);
	currPage->erase();
	currPage->invalidateAll();
	currPage->prepare();
	menuText.prepare();

	topBar.draw();
	currPage->draw();
}

void changeLangEng()
{
	gBackBtnOnScreen = true;
	ruSelect.on(false);
	enSelect.on(true);

	scrStrings = engStrings;

	menuText.erase();
	//menuText.invalidate();
	topBar.invalidateAll();
	//menuText.setText(SCREENLANG);
	currPage->erase();
	currPage->invalidateAll();
	currPage->prepare();
	menuText.prepare();

	topBar.draw();
	currPage->draw();
}


// hardcoded
void buildLangPage()
{
	// Экран
	static Text boldScreen;
	boldScreen.setFont(BOLDFONT);
	boldScreen.setText(SCREEN);
	boldScreen.setXYpos(PG_LEFT_PADD, MB_Y_START);
	boldScreen.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(BACKGROUND)
			);

	// Яркость подсветки дисплея
	static Text subtScreen;
	subtScreen.setFont(SMALLFONT);
	subtScreen.setText(BRIGHT);
	subtScreen.setXYpos(PG_LEFT_PADD, 63);
	subtScreen.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(BACKGROUND)
			);

	// Поле ввода
	static InputField brightness;
	brightness.setFont(SMALLFONT);
	brightness.setXYpos(PG_LEFT_PADD, 83);
	brightness.setText("100");
	brightness.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	static Text percent;
	percent.setFont(SMALLFONT);
        percent.setText(PERCENT);
	percent.setXYpos(
			brightness.getX() 
			//+ brightness.getW()
			+ 40
			+ 3, 
			83 + GR_BTN_Y_PADDING
			);
        percent.setColors(
                     greyscaleColor(FONT_COLOR), 
                     greyscaleColor(BACKGROUND)
		     );

	static Text sleepAfter;
	sleepAfter.setFont(SMALLFONT);
	sleepAfter.setText(DIMAFTER);
	sleepAfter.setXYpos(PG_LEFT_PADD, 110);
	sleepAfter.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(BACKGROUND)
			);


	static InputField seconds;
	seconds.setFont(SMALLFONT);
	seconds.setXYpos(PG_LEFT_PADD, 130);
	seconds.setText("100");
	seconds.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	static Text secText;
	secText.setFont(SMALLFONT);
	//secText.setXYpos(PG_LEFT_PADD, 127);
	secText.setXYpos(
			PG_LEFT_PADD
			//+ brightness.getW()
			+ 40
			+ 3, 
			130 + GR_BTN_Y_PADDING
			);

	secText.setText(SEC);
	secText.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(BACKGROUND)
			);

	static Text boldLang;
	boldLang.setFont(BOLDFONT);
	boldLang.setText(LANG);
	boldLang.setXYpos(PG_LEFT_PADD, 157);
	boldLang.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(BACKGROUND)
			);


	static SimpleBox ruBox;
	ruBox.setColor(greyscaleColor(TOP_BAR_BG_COL));
	ruBox.setWH(150, 45);
	ruBox.setXYpos(15, 174);

	static SimpleBox usBox;
	usBox.setColor(greyscaleColor(TOP_BAR_BG_COL));
	usBox.setWH(150, 45);
	usBox.setXYpos(15, 226);

	//static CircRadBtn ruSelect;
	ruSelect.setXYpos(20, 186);
	ruSelect.setCurCol(greyscaleColor(TOP_BAR_BG_COL));
	ruSelect.on(true);
	ruSelect.setCallback(changeLangRus);

	//static CircRadBtn enSelect;
	enSelect.setXYpos(20, 238);
	enSelect.setCurCol(greyscaleColor(TOP_BAR_BG_COL));
	enSelect.on(false);
	enSelect.setCallback(changeLangEng);


	static ImageButton lang_back;
	lang_back.setCallback(callSettingsPage);
	lang_back.loadRes("/prev.jpg");
	lang_back.setXYpos(7, 284);
	lang_back.setCircle();

	static Image ruFlag;
	ruFlag.loadRes("/ru.jpg");
	ruFlag.setXYpos(113, 180);

	static Image usFlag;
	usFlag.loadRes("/us.jpg");
	usFlag.setXYpos(113, 232);

	static BodyText langRu;
	langRu.setFont(SMALLFONT);
	langRu.setText(RUS);
	langRu.setXYpos(49, 189);
	langRu.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	static BodyText langEng;
	langEng.setFont(SMALLFONT);
	langEng.setText(ENG);
	langEng.setXYpos(49, 241);
	langEng.setColors(
			greyscaleColor(FONT_COLOR), 
			greyscaleColor(GR_BTN_BG_COLOR)
			);


	langPage.addItem(&boldScreen);
	langPage.addItem(&subtScreen);
	langPage.addItem(&brightness);
	langPage.addItem(&percent);
	langPage.addItem(&sleepAfter);
	langPage.addItem(&seconds);
	langPage.addItem(&secText);
	langPage.addItem(&boldLang);

	langPage.addItem(&ruBox);
	langPage.addItem(&usBox);

	langPage.addItem(&ruSelect);
	langPage.addItem(&enSelect);

	langPage.addItem(&ruFlag);
	langPage.addItem(&usFlag);

	langPage.addItem(&langRu);
	langPage.addItem(&langEng);

	langPage.addItem(&lang_back);
}

/******************************************************************************* 
callback functions
*******************************************************************************/

void callLangPage()
{
	gBackBtnOnScreen = true;

	langPage.invalidateAll();
	langPage.prepare();

	menuText.erase();
	topBar.invalidateAll();
	currPage->erase();
	menuText.setText(SCREENLANG);
	menuText.prepare();
	currPage = &langPage;
	topBar.draw();
	currPage->draw();
}

void callSettingsPage()
{
	gBackBtnOnScreen = true;

	settingsPage.invalidateAll();
	settingsPage.prepare();

	menuText.erase();
	topBar.invalidateAll();
	currPage->erase();
	menuText.setText(SETTINGS);
	menuText.prepare();
	currPage = &settingsPage;
	//currPage->invalidateAll();
	//currPage->prepare();
	//while(gRapidBlink){};
	topBar.draw();
	currPage->draw();
	//DrawTopBar("Настройки");
}

void callMainPage()
{
	gBackBtnOnScreen = true;

	mainPage.invalidateAll();
	mainPage.prepare();

	menuText.erase();
	topBar.invalidateAll();
	currPage->erase();
	menuText.setText(MENU);
	menuText.prepare();
	currPage = &mainPage;
	//currPage->invalidateAll();
	//currPage->prepare();
	//while(gRapidBlink){};
	topBar.draw();
	currPage->draw();
	//DrawTopBar("Меню");
}

bool initDone = false;

//static Cursor cursor;

// Cursor task function
/*
void cursorDraw(bool blink)
{
	if (!currItem)
		return;
	//DEBUG_PRINT(currItem);

	cursor.setCoord(currItem);

	if (blink) {
		cursor.draw();
	}
	else {
		cursor.erase();
	}
}
*/

bool gblink = false;


// screen buttons
#define menu1_size 4
#define settings_size 5
static GreyTextButton menu_items[menu1_size];
static GreyTextButton settings_items[settings_size];
static ImageButton back;
static ImageButton sett_back;
static BlueTextButton next;


void buildMainPage()
{

	//////// TODO: calculate gap?
	int gap = 5;

	dispStrings_t ru_menu1[menu1_size];
	ru_menu1[0] = NEW_PLANT;
	ru_menu1[1] = ONLINE_MON;
	ru_menu1[2] = SETTINGS;
	ru_menu1[3] = DIAG;

	int j = 0;

	// omg, change that...
	for (auto& i:menu_items) {
		i.setCallback(nop);
		i.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));
		i.setFont(SMALLFONT);
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+(GREY_BUTTON_HEIGHT+gap)*j);
		i.setText(ru_menu1[j]);
		j++;
	}

	menu_items[2].setCallback(callSettingsPage);

	for (int i = 0; i < menu1_size; i++) {
		mainPage.addItem(&menu_items[i]);
	}

	back.setCallback(nop);
	back.loadRes("/prev.jpg");
	back.setXYpos(7, 284);
	back.setCircle();

	mainPage.addItem(&back);
}

void buildSettingsPage()
{
	dispStrings_t ru_menu_settings[settings_size];
	ru_menu_settings[0] = TIMEDATE;
	ru_menu_settings[1] = WIFI;   
	ru_menu_settings[2] = SCREENLANG;
	ru_menu_settings[3] = CALIB;
	ru_menu_settings[4] = THRES;

	int j = 0;
	int gap = 5;

	for (auto& i:settings_items) {
		i.setCallback(nop);
		i.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));
		i.setFont(SMALLFONT);
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+(GREY_BUTTON_HEIGHT+gap)*j);
		i.setText(ru_menu_settings[j]);
		j++;
	}

	settings_items[2].setCallback(callLangPage);

	for (int i = 0; i < settings_size; i++) {
		settingsPage.addItem(&settings_items[i]);
	}

	sett_back.setCallback(callMainPage);
	sett_back.loadRes("/prev.jpg");
	sett_back.setXYpos(7, 284);
	sett_back.setCircle();
	settingsPage.addItem(&sett_back);

}

static App app;

#ifdef TASKS
void gui(void* arg)
{
	for(;;) {
		app.update();
		//yield();
		//sleep(10);
	}
}
#endif

void setup(void)
{
	// init all stuff in Gui.h
	app.init();

	Serial.begin(115200);

	// backlight
	pinMode(19, OUTPUT);
	analogWrite(19, 127);

	// buttons
	pinMode(PREV, INPUT_PULLUP);
	pinMode(NEXT, INPUT_PULLUP);
	pinMode(OK, INPUT_PULLUP);

	buildMainPage();
	buildSettingsPage();
	buildLangPage();

	//mainPage.prepare();

	currPage = &mainPage;
	currPage->setCurrItem(0);
	currItem = currPage->getCurrItem();
	currPage->prepare();

	buildTopBar();

	currPage->draw();
	topBar.draw();

	// flag for cursor
	initDone = true;

	// cursor
#ifdef TASKS
	xTaskCreate(
			gui,
			"gui",
			20000,
			NULL,
			0,
			NULL
		   );

	/*
	xTaskCreate(
			nav,
			"nav",
			10000,
			NULL,
			2,
			NULL
		   );
		   */
#endif
}

void loop() {
#ifndef TASKS
	app.update();
	delay(10);
#endif
}
