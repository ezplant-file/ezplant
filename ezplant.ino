//TODO: consider resource manager and page builder classes

//#define TASKS

// buttons defines
#define BTN_PREV 18
#define BTN_NEXT 23
#define BTN_OK 5
#define BTN_MIN 16
#define BTN_PLU 17

// GUI & strings
//#include "data/wifi.h"
//#include <WiFi.h>
//#include <WebServer.h>
#include "GfxUi.h"
#include "rustrings.h"
#include "enstrings.h"
#include "stringenum.h"
#include "Gui.h"

static App app;

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
	menuText.setFont(MIDFONT);
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
// screen buttons
#define menu1_size 6
#define settings_size 5
static GreyTextButton menu_items[menu1_size];
static GreyTextButton settings_items[settings_size];
static ImageButton back;
//static ImageButton sett_back;
static BlueTextButton next;



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

//static CircRadBtn ruSelect;
//static CircRadBtn enSelect;
static ExclusiveRadio ruSelect;
static ExclusiveRadio enSelect;

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

/************************ TEST PAGE ******************************/

Page testPage;
static Toggle testTgl;
static CheckBox testChBox;
static TestPageRadio testRad;
static InputField testInput;
static GreyTextButton testGreyButton;
static BlueTextButton testBlueButton;

void tglCallback()
{
	if (testTgl.isOn())
		testTgl.on(false);
	else
		testTgl.on(true);
	testTgl.invalidate();
	testTgl.draw();
}

void chkCallback()
{
	if (testChBox.isOn())
		testChBox.on(false);
	else
		testChBox.on(true);
	testChBox.invalidate();
	testChBox.draw();
}

void radCallback()
{
	if (testRad.isOn())
		testRad.on(false);
	else
		testRad.on(true);
	testRad.invalidate();
	testRad.draw();
}

static Text smallestFont, smallFont, \
		    midFont, largeFont, \
		    largestFont, boldFont, \
		    boldFont2;
Text* fonts[] = {
	&smallestFont,
	&smallFont,
	&midFont,
	&largeFont,
	&largestFont,
	&boldFont,
	&boldFont2
};

Page fontPage;

void buildFontPage()
{


	smallestFont.setFont(SMALLESTFONT);
	smallFont.setFont(SMALLFONT);
	midFont.setFont(MIDFONT);
	largeFont.setFont(LARGEFONT);
	largestFont.setFont(LARGESTFONT);
	boldFont.setFont(BOLDFONT);
	boldFont2.setFont(BOLDFONT2);

	smallestFont.setText(SMALLESTFONT_TEXT);
	smallFont.setText(SMALLFONT_TEXT);
	midFont.setText(MIDFONT_TEXT);
	largeFont.setText(LARGEFONT_TEXT);
	largestFont.setText(LARGESTFONT_TEXT);
	boldFont.setText(BOLDFONT_TEXT);
	boldFont2.setText(BOLDFONT2_TEXT);

	int gap = 5;
	int j = 0;

	for (auto i:fonts) {
		i->setCallback(nop);
		i->setXYpos(
				PG_LEFT_PADD,
				MB_Y_START
				+(GREY_BUTTON_HEIGHT+gap)*j
				);
		i->setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(BACKGROUND)
			);
		j++;
		fontPage.addItem(i);
	}
	fontPage.addItem(&back);
}

void buildTestPage()
{
	testTgl.setFont(SMALLFONT);
	testTgl.setXYpos(17, 41);
	testTgl.setText(TOGGLE_TEXT);
	testTgl.prepare();
	testTgl.on(false);
	testTgl.setCallback(tglCallback);

	testChBox.setFont(SMALLFONT);
	testChBox.setXYpos(17, 65);
	testChBox.setText(CHECHBOX_TEXT);
	testChBox.prepare();
	testChBox.on(false);
	testChBox.setCallback(chkCallback);

	testRad.setFont(SMALLFONT);
	testRad.setXYpos(17, 90);
	testRad.setText(RADIO_TEXT);
	testRad.prepare();
	testRad.on(false);
	testRad.setBgColor(0xDC);
	testRad.setCallback(radCallback);

	testInput.setFont(SMALLFONT);
	testInput.setXYpos(17, 115);
	testInput.setValue(100);
	testInput.setText(INPUT_TEXT);
	testInput.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	testGreyButton.setXYpos(17, 150);
	testGreyButton.setText(GREY_BUTTON);
	testGreyButton.setFont(SMALLFONT);
	testGreyButton.setCallback(nop);

	//TODO: depricate this:
	testGreyButton.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));

	testBlueButton.setXYpos(17, 185);
	testBlueButton.setText(BLUE_BUTTON);
	testBlueButton.setFont(SMALLFONT);
	testBlueButton.setCallback(nop);


	testPage.addItem(&testTgl);
	testPage.addItem(&testChBox);
	testPage.addItem(&testRad);
	testPage.addItem(&testInput);
	testPage.addItem(&testGreyButton);
	testPage.addItem(&testBlueButton);

	testPage.addItem(&back);
}

/************************ END TEST PAGE *************************/

static InputField brightness;
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
	brightness.setFont(SMALLFONT);
	brightness.setXYpos(PG_LEFT_PADD, 83);
	brightness.setValue(50);
	brightness.setText(PERCENT);
	brightness.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	/*
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
		     */

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
	seconds.setValue(100);
	seconds.setText(SEC);
	seconds.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	/*
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
			*/

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


	/*
	static ImageButton lang_back;
	lang_back.setCallback(callSettingsPage);
	lang_back.loadRes("/prev.jpg");
	lang_back.setXYpos(7, 284);
	lang_back.setCircle();
	*/

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
	//langPage.addItem(&percent);
	langPage.addItem(&sleepAfter);
	langPage.addItem(&seconds);
	//langPage.addItem(&secText);
	langPage.addItem(&boldLang);

	langPage.addItem(&ruBox);
	langPage.addItem(&usBox);

	langPage.addItem(&ruSelect);
	langPage.addItem(&enSelect);

	langPage.addItem(&ruFlag);
	langPage.addItem(&usFlag);

	langPage.addItem(&langRu);
	langPage.addItem(&langEng);

	langPage.addItem(&back);
}

/*******************************************************************************
callback functions
*******************************************************************************/
void callTestPage()
{
	gBackBtnOnScreen = true;

	app.resetIterator();

	back.setCallback(callMainPage);

	testPage.invalidateAll();
	testPage.prepare();

	menuText.erase();
	topBar.invalidateAll();
	currPage->erase();
	menuText.setText(TEST_PAGE);
	menuText.prepare();
	currPage = &testPage;
	topBar.draw();
	currPage->draw();
}

void callFontPage()
{
	gBackBtnOnScreen = true;

	app.resetIterator();

	back.setCallback(callMainPage);

	fontPage.invalidateAll();
	fontPage.prepare();

	menuText.erase();
	topBar.invalidateAll();
	currPage->erase();
	menuText.setText(FONT_PAGE);
	menuText.prepare();
	currPage = &fontPage;
	topBar.draw();
	currPage->draw();
}

void callLangPage()
{
	gBackBtnOnScreen = true;

	app.resetIterator();

	back.setCallback(callSettingsPage);

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

	app.resetIterator();

	back.setCallback(callMainPage);

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

	app.resetIterator();

	back.setCallback(nop);

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



void buildMainPage()
{

	//////// TODO: calculate gap?
	int gap = 5;

	dispStrings_t ru_menu1[menu1_size];
	ru_menu1[0] = NEW_PLANT;
	ru_menu1[1] = ONLINE_MON;
	ru_menu1[2] = SETTINGS;
	ru_menu1[3] = DIAG;
	ru_menu1[4] = TEST_PAGE;
	ru_menu1[5] = FONT_PAGE;

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
	menu_items[4].setCallback(callTestPage);
	menu_items[5].setCallback(callFontPage);

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

	/*
	sett_back.setCallback(callMainPage);
	sett_back.loadRes("/prev.jpg");
	sett_back.setXYpos(7, 284);
	sett_back.setCircle();
	settingsPage.addItem(&sett_back);
	*/
	settingsPage.addItem(&back);

}


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

/*
WebServer server(80);

void handleClient()
{
	listRootToHtml();
}

bool handleFileRead(String path)
{
	File file = SPIFFS.open(path, "r");

	if (!file) {

		server.send(404, "text/plain", "FileNotFound");
		return false;
	}

	server.streamFile(file, "text/html");
	file.close();
	return true;
}

void listRootToHtml()
{
    File root = SPIFFS.open("/");

    if (!root) {
        Serial.println("error");
        return;
    }

    String html = "<html><meta charset=\"UTF-8\"><body>";

    if (root.isDirectory()) {

        File file = root.openNextFile();
        while (file) {

            String name = file.name();

            html += "<p><a href=\"";
            html += name;
            html += (String)"\" download=\"";
            html += name +"\">";
            html += name;
            html += "</a></p>";

            file = root.openNextFile();
        }
        html += "</body></html>";
    }

    server.send(200, "text/html", html);
}
*/

#define LED_PIN 19

uint8_t g_curr_brightness;

void setBacklight(uint8_t br)
{
	uint8_t mapped_br = map(br, 0, 100, 0, 255);
	//Serial.println(mapped_br);
	analogWrite(LED_PIN, mapped_br);
}

void setup(void)
{
	Serial.begin(115200);

	// init all stuff in Gui.h
	app.init();

	/*
	// wifi stuff
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
	}
	Serial.println();
	Serial.println(WiFi.localIP());
*/

	// buttons
	pinMode(BTN_PREV, INPUT_PULLUP);
	pinMode(BTN_NEXT, INPUT_PULLUP);
	pinMode(BTN_OK, INPUT_PULLUP);
	pinMode(BTN_MIN, INPUT_PULLUP);
	pinMode(BTN_PLU, INPUT_PULLUP);

	buildFontPage();
	buildTestPage();
	buildLangPage();
	buildSettingsPage();
	buildMainPage();

	// backlight
	g_curr_brightness = brightness.getValue();
	pinMode(LED_PIN, OUTPUT);
	setBacklight(g_curr_brightness);

	//mainPage.prepare();

	currPage = &mainPage;
	currPage->setCurrItem(0);
	currItem = currPage->getCurrItem();
	currPage->prepare();

	buildTopBar();

	currPage->draw();
	topBar.draw();

	/*
	// flag for cursor
	//initDone = true;

	// webserver stuff
	server.on("/",  HTTP_GET, handleClient);
	server.onNotFound([]() {
			if (!handleFileRead(server.uri())) {
			server.send(404, "text/plain", "FILE NOT FOUND");
			}
			});

	server.begin();
	*/

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

#define INTERVAL 500
unsigned long oldMils = 0;

void loop() {
#ifndef TASKS
	//server.handleClient();

	if (millis() - oldMils > INTERVAL) {
		uint8_t new_br = brightness.getValue();
		if (new_br != g_curr_brightness) {
			setBacklight(new_br);
			g_curr_brightness = new_br;
		}
	/*
		Serial.print("Free heap: ");
		Serial.println(ESP.getFreeHeap());
		Serial.print("WiFi strength: ");
		Serial.println(WiFi.RSSI());
	*/
		/*
		Serial.print("REG: ");
		// NEXT - 28, PREV - 23, OK - 5, PLUS - 22, MINUS - 21
		Serial.println(REG_READ(GPIO_IN_REG), BIN);
		oldMils = millis();
		*/
	}

	app.update();
	//app.draw();
	delay(10);
#endif
}
