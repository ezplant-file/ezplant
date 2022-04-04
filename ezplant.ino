//TODO: consider resource manager and page builder classes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_task_wdt.h"

#define TASKS
#define APP_DEBUG

#define PUMP_F 	17
#define PUMP_G 	18
#define PUMP_H 	19
#define LED	32
#define FAN	33

// buttons defines
#define BTN_PREV 	0b00000001
#define BTN_MIN 	0b00000010
#define BTN_OK 		0b00000100
#define BTN_HOME 	0b00001000
#define BTN_NEXT 	0b00010000
#define BTN_PLU 	0b00100000

// interrupt
#define EXPANDER_INT	27
// backlight
#define LED_PIN 	23
// relay
#define TDS_MTR_RLY	7

// Wifi
#include <WiFi.h>
#include <WiFiAP.h>

// GUI & strings
#include "GfxUi.h"
#include "rustrings.h"
#include "enstrings.h"
#include "stringenum.h"
#include "Gui.h"

// json stuff
#include "json.hpp"
using json = nlohmann::json;

static App app;

static Page settingsPage;
// screen buttons
//#define menu1_size 6
//#define settings_size 5
static ImageButton back;
//static ImageButton sett_back;
static BlueTextButton next;

// Lang select and screen settings page items
static InputField gDimseconds;

//static CircRadBtn ruSelect;
//static CircRadBtn enSelect;
static ExclusiveRadio ruSelect;
static ExclusiveRadio enSelect;

// Test page items
Page testPage;
static Toggle testTgl;
static CheckBox testChBox;
static TestPageRadio testRad;
static InputField testInput;
static GreyTextButton testGreyButton;
static BlueTextButton testBlueButton;

/****************** WiFi stuff ***************************************/
const char* ap_ssid = "ezplant_wifi";
const char* ap_password = scrStrings[WI_PASSWORD];
const char* PARAM_1 = "ssid";
const char* PARAM_2 = "password";
String file_ssid;
String file_password;

const char* cred_filename = "/wifi_creds";

const char* sett_file = "/settings";

#include "index.h"
#include "ru_resp.h"
#include "en_resp.h"

bool loadSettings()
{
	if (!SPIFFS.exists(sett_file)) {
#ifdef APP_DEBUG
		Serial.println("no file");
#endif
		return false;
	}

	File file = SPIFFS.open(sett_file, "r");

	if (!file) {
#ifdef APP_DEBUG
		Serial.println("couldn't open file");
#endif
		return false;
	}

	try
	{
		String content = file.readStringUntil(EOF);
#ifdef APP_DEBUG
		Serial.println(content);
#endif
		json load = json::parse(content.c_str());
		g_init_brightness = load["g_init_brightness"].get<int16_t>();
		g_dimafter = load["g_dimafter"].get<uint8_t>();
		g_ntp_sync = load["g_ntp_sync"].get<bool>();

		datetime.initSync(g_ntp_sync);

		gUTC = load["gUTC"].get<int8_t>();

		datetime.initUTC(gUTC);

		g_selected_lang = load["lang"].get<lang_t>();
		gwsWifiChBox.on(load["g_wifi_on"].get<bool>());
		load.clear();
	}

	catch (...)
	{
#ifdef APP_DEBUG
		Serial.println("json exception occured");
#endif
		return false;
	}

	return true;
}

bool saveSettings()
{
	json save;
	save["g_init_brightness"] = gBrightness.getValue();
	save["g_dimafter"] = g_dimafter;
	save["g_ntp_sync"] = datetime.getSync();
	save["gUTC"] = datetime.getUTC();
	save["lang"] = g_selected_lang;
	save["g_wifi_on"] = gwsWifiChBox.isOn();

	File file = SPIFFS.open(sett_file, "w");
	if (!file) {
#ifdef APP_DEBUG
		Serial.println("failed to save settings file");
#endif
		save.clear();
		return false;
	}
#ifdef APP_DEBUG
	else {
		Serial.println("settings file saved");
	}
#endif

	std::string settings = save.dump();
	save.clear();
	file.print(settings.c_str());
	file.flush();
	file.close();

	return true;
}

void notFound()
{
	server.send(404, "text/plain", scrStrings[NOT_FOUND]);
}

void saveFile()
{
	File file = SPIFFS.open(cred_filename, "w");
	if (!file) {
#ifdef APP_DEBUG
		Serial.println("error creating file");
#endif
		return;
	}

	file.println(file_ssid);
	file.println(file_password);
	file.flush();
	file.close();
}

void getCallback()
{
	String arg1 = server.arg(PARAM_1);
	String arg2 = server.arg(PARAM_2);

	if (arg1 != "") {
		file_ssid = arg1;
	}
	else {
		file_ssid = "none";
	}

	file_password = arg2;

	const char* resp;

	if (g_selected_lang == RU_LANG) {
		resp = ru_resp_html;
	}
	else if (g_selected_lang == EN_LANG) {
		resp = en_resp_html;
	}

	server.send(
			200,
			"text/html",
			resp
		     );

	if (file_ssid == "none")
		return;

	saveFile();

	g_wifi_set = true;
	pages[SETT_PG] = buildSettingsPage();

	if (currPage == pages[WIFI_PG]) {
		callPage(pages[WIFI_SETT_PG]);
	}

	connectWithCred();
}



void softAP()
{
	IPAddress ap_ip(192,168,0,1);
	IPAddress ap_gate(192,168,0,1);
	IPAddress ap_sub(255,255,255,0);

	WiFi.softAP(ap_ssid, ap_password);
	WiFi.softAPConfig(ap_ip, ap_gate, ap_sub);

#ifdef APP_DEBUG
	Serial.println();
	Serial.print("IP Address: ");
	Serial.println(WiFi.softAPIP());
#endif

	server.on("/", HTTP_GET, [](){
			server.send(200, "text/html", index_html);
			});

	server.on("/get", HTTP_GET, getCallback);

	server.onNotFound(notFound);
	server.begin();
}

void connectWithCred()
{
	//server.end();
#ifdef APP_DEBUG
	Serial.println((String)"entering " + __func__);
#endif

	WiFi.softAPdisconnect(true);

	WiFi.begin(file_ssid.c_str(), file_password.c_str());

	/*
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed!");
		return;
	}
	*/

#ifdef APP_DEBUG
	Serial.println(WiFi.localIP());
#endif
}

void checkWifi()
{
	File file;

	if (SPIFFS.exists(cred_filename)) {
		file = SPIFFS.open(cred_filename, "r");
	}
#ifdef APP_DEBUG
	else {
		Serial.println("error opening WiFi settings file");
	}
#endif

	if (file) {
#ifdef APP_DEBUG
		Serial.println("connecting without softAP");
#endif
		file_ssid = file.readStringUntil('\n');
		file_password = file.readStringUntil('\n');
		file_ssid.trim();
		file_password.trim();
		Serial.println(file_ssid);
		Serial.println(file_password);
		Serial.println("connect with cred");
		connectWithCred();
		file.close();
		g_wifi_set = true;
	}
	else {
		g_wifi_set = false;
#ifdef APP_DEBUG
		Serial.println("creating softAP");
#endif
		softAP();
	}
}

/*******************************************************************************
callback functions
*******************************************************************************/

void gDimAfter(void* arg)
{
	g_dimafter = gDimseconds.getValue();
}

void gChangeWifi(void* arg)
{
	SPIFFS.remove(cred_filename);
	g_wifi_set = false;

	checkWifi();
	pages[SETT_PG] = buildSettingsPage();
	callPage(pages[WIFI_PG]);
}

void callPage(void* page_ptr)
{
	if (page_ptr == nullptr)
		return;

	Page* page = (Page*) page_ptr;

	app.resetIterator();

	//back.setCallback(callPage, currPage);
	if (page->prev() != nullptr)
		back.setCallback(callPage, page->prev());

	currPage->freeRes();

	page->invalidateAll();
	page->prepare();

	topBar.erase();
	topBar.invalidateAll();
	currPage->erase();
	//Serial.println(page->getTitle());
	topBar.setText(page->getTitle());
	topBar.prepare();
	currPage = page;

	if (!currPage->visibleIcons()) {
		topBar.hideIcons();
	}
	else {
		topBar.showIcons();
	}

	topBar.draw();
	currPage->draw();

	if (currPage == pages[SETT_PG]) {
		saveSettings();
	}
}


void wifiChCallback(void* arg)
{
	// gwsConnection in topBar.update()
	if (gwsWifiChBox.isOn()) {
		gwsWifiChBox.on(false);
		WiFi.disconnect();
	}
	else {
		gwsWifiChBox.on(true);
		WiFi.reconnect();
	}

	gwsWifiChBox.invalidate();
	gwsWifiChBox.draw();
}

void tglCallback(void* arg)
{
	if (testTgl.isOn())
		testTgl.on(false);
	else
		testTgl.on(true);
	testTgl.invalidate();
	testTgl.draw();
}

void chkCallback(void* arg)
{
	if (testChBox.isOn())
		testChBox.on(false);
	else
		testChBox.on(true);
	testChBox.invalidate();
	testChBox.draw();
}

void radCallback(void* arg)
{
	if (testRad.isOn())
		testRad.on(false);
	else
		testRad.on(true);
	testRad.invalidate();
	testRad.draw();
}


void changeLangRus(void* arg)
{
	g_selected_lang = RU_LANG;
	//gBackBtnOnScreen = true;
	ruSelect.on(true);
	enSelect.on(false);

	scrStrings = ruStrings;

	//callPage(currPage);

	topBar.erase();
	topBar.invalidateAll();
	topBar.setText(SCREENLANG);
	currPage->erase();
	currPage->invalidateAll();
	currPage->prepare();
	topBar.prepare();

	topBar.draw();
	currPage->draw();
}

void changeLangEng(void* arg)
{
	g_selected_lang = EN_LANG;
	//gBackBtnOnScreen = true;
	ruSelect.on(false);
	enSelect.on(true);

	scrStrings = engStrings;

	//callPage(currPage);

	topBar.erase();
	topBar.invalidateAll();
	topBar.setText(SCREENLANG);
	currPage->erase();
	currPage->invalidateAll();
	currPage->prepare();
	topBar.prepare();

	topBar.draw();
	currPage->draw();
}

void syncTimeCallback(void* arg)
{
	if (arg == nullptr)
		return;

	CheckBox* checkbox = (CheckBox*)arg;

	bool checked = checkbox->isOn();

	if (checked) {
		checkbox->on(false);
	}
	else {
		checkbox->on(true);
	}

	datetime.setSync(checkbox->isOn());
	datetime.invalidate();
	datetime.prepare();

	//pages[TIME_PG]->restock();

	checkbox->invalidate();
}

// ph calib tasks
#define SOL_SETT_TIM 5000

void calibPH4task(void* arg)
{
	sleep(SOL_SETT_TIM);

	ph_meter.setCalibration(1, 4.00);

	while (ph_meter.getCalibration() == 1) {
		sleep(1000);
	}

	g_ph_calib_4_done = true;

	vTaskDelete(NULL);
}

void calibPH9task(void* arg)
{
	sleep(SOL_SETT_TIM);

	ph_meter.setCalibration(2, 9.18);
	while (ph_meter.getCalibration() == 2) {
		sleep(1000);
	}

	g_ph_calib_9_done = true;

	vTaskDelete(NULL);
}

void createPhCalibTasks(void* arg)
{
	if (currPage == pages[CAL_PH2_PG]) {
		callPage(pages[CAL_PH3_PG]);
	}
	else {
		callPage(pages[CAL_PH5_PG]);
	}

	if (!g_ph_calib_4_done && !g_ph_calib_9_done) {
		xTaskCreate(calibPH4task, "ph4", 2000, NULL, 1, NULL);
	}
	else if (g_ph_calib_4_done && !g_ph_calib_9_done) {
		xTaskCreate(calibPH9task, "ph9", 2000, NULL, 1, NULL);
	}
}

// tds calib tasks
void calibTDS500task(void* arg)
{
	sleep(SOL_SETT_TIM);

	tds_meter.setCalibration(1, 500);
	while (tds_meter.getCalibration() == 1) {
		sleep(1000);
	}

	g_tds_calib_500_done = true;
	vTaskDelete(NULL);
}

void calibTDS1500task(void* arg)
{
	sleep(SOL_SETT_TIM);

	tds_meter.setCalibration(1, 1500);
	while (tds_meter.getCalibration() == 2) {
		sleep(1000);
	}

	g_tds_calib_1500_done = true;
	vTaskDelete(NULL);
}

void createTdsCalibTasks(void* arg)
{
	if (currPage == pages[CAL_TDS2_PG]) {
		callPage(pages[CAL_TDS3_PG]);
	}
	else {
		callPage(pages[CAL_TDS5_PG]);
	}

	if (!g_tds_calib_500_done && !g_tds_calib_1500_done) {
		xTaskCreate(calibTDS500task, "tds500", 2000, NULL, 1, NULL);
	}
	else if (g_tds_calib_500_done && !g_tds_calib_1500_done) {
		xTaskCreate(calibTDS1500task, "tds1500", 2000, NULL, 1, NULL);
	}
}

#define DIAG_WAIT_TIME 2000

void diagPhTaskWait(void* arg)
{
	sleep(DIAG_WAIT_TIME);
	g_ph_diag_wait_done = true;
	vTaskDelete(NULL);
}

void diagTdsTaskWait(void* arg)
{
	sleep(DIAG_WAIT_TIME);
	g_tds_diag_wait_done = true;
	vTaskDelete(NULL);
}


// sensor checkers, called before entering calibration pages
// and diagnostic pages. TDS sensor checker has similar logic
void checkPhSensor(void* arg)
{
	// if function is called from settings page
	if (currPage == pages[CAL_SETT_PG]) {
		// reset global flag
		resetCalibFlags();
		// if failed
		if (!ph_meter.begin()) {
			// change fail page title
			pages[SENS_FAIL_PG]->setTitle(CAL_PH_TITLE);
			// change fail page previous item
			pages[SENS_FAIL_PG]->setPrev(pages[CAL_SETT_PG]);
			callPage(pages[SENS_FAIL_PG]);
		}
		else {
			callPage(pages[CAL_PH1_PG]);
		}
	}
	// if called from diagnostics page
	else if (currPage == pages[SENS_DIAG_PG]) {
		resetDiagFlags();
		// if failed
		if (!ph_meter.begin()) {
			// change fail page title
			pages[SENS_FAIL_PG]->setTitle(DIAG_SENS);
			// change fail page prev item
			pages[SENS_FAIL_PG]->setPrev(pages[SENS_DIAG_PG]);
			callPage(pages[SENS_FAIL_PG]);
		}
		else {
			// call ph diag page and create wait task
			callPage(pages[PH_DIAG_PG]);
			xTaskCreate(diagPhTaskWait, "phWait", 1000, NULL, 1, NULL);
		}
	}
}

// same as ph checker
void checkTdsSensor(void* arg)
{
	if (currPage == pages[CAL_SETT_PG]) {
		resetCalibFlags();
		if (!tds_meter.begin()) {
			pages[SENS_FAIL_PG]->setTitle(CAL_TDS_TITLE);
			pages[SENS_FAIL_PG]->setPrev(pages[CAL_SETT_PG]);
			callPage(pages[SENS_FAIL_PG]);
		}
		else {
			second_expander.digitalWrite(TDS_MTR_RLY, HIGH);
			callPage(pages[CAL_TDS1_PG]);
		}
	}
	else if (currPage == pages[SENS_DIAG_PG]) {
		resetDiagFlags();
		if (!tds_meter.begin()) {
			pages[SENS_FAIL_PG]->setTitle(DIAG_SENS);
			pages[SENS_FAIL_PG]->setPrev(pages[SENS_DIAG_PG]);
			callPage(pages[SENS_FAIL_PG]);
		}
		else {
			second_expander.digitalWrite(TDS_MTR_RLY, HIGH);
			callPage(pages[TDS_DIAG_PG]);
			xTaskCreate(diagTdsTaskWait, "tdsWait", 1000, NULL, 1, NULL);
		}
	}
}

/*******************************************************************************
page builders TODO: move to Gui
*******************************************************************************/

/**************************** COMMON CALIB PAGES ********************************/
Page* buildCalSettPage()
{
	static Page calibSettPage;

	static GreyTextButton tds;
	tds.setXYpos(PG_LEFT_PADD, MB_Y_START);
	tds.setText(CAL_TDS);
	tds.setCallback(checkTdsSensor);

	static GreyTextButton ph;
	ph.setXYpos(PG_LEFT_PADD, MB_Y_START+GREY_BUTTON_HEIGHT+5);
	ph.setText(CAL_PH);
	ph.setCallback(checkPhSensor);

	static Text calText;
	calText.setXYpos(PG_LEFT_PADD, 132);
	calText.setText(CAL_PAR1);
	//TODO: implement this
	calText.rightJustify();

	static Image qrCode;
	qrCode.setXYpos(116, 192);
	qrCode.loadRes(images[IMG_QR_CAL]);

	calibSettPage.setTitle(CAL_TITLE);
	//calibSettPage.setPrev(&settingsPage);
	calibSettPage.addItem(&tds);
	calibSettPage.addItem(&ph);
	calibSettPage.addItem(&calText);
	calibSettPage.addItem(&qrCode);
	calibSettPage.addItem(&back);

	return &calibSettPage;
}

Page* buildSensorFailPage()
{
	static Page fail;

	static Text par1;

	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(SENS_FAIL);

	fail.setTitle(CAL_PH_TITLE);

	fail.addItem(&par1);
	fail.addItem(&back);

	return &fail;
}

/************************ PH CALIBRATE PAGES *********************/
Page* buildPh5Page()
{
	static Page ph5page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH5_PAR1);

	g_ph5wait.setXYpos(75, 105);

	g_ph_succ.setXYpos(PG_LEFT_PADD, 98);
	g_ph_succ.setText(CAL_SUCC);
	g_ph_succ.setColors(GREEN_COL_MACRO, TFT_WHITE);
	g_ph_succ.setInvisible();

	g_ph_done.setXYpos(PG_LEFT_PADD, 120);
	g_ph_done.setText(CAL_DONE);
	g_ph_done.setInvisible();

	ph5page.setTitle(CAL_PH_TITLE);

	ph5page.addItem(&par1);
	ph5page.addItem(&g_ph5wait);
	ph5page.addItem(&g_ph_succ);
	ph5page.addItem(&g_ph_done);

	return &ph5page;
}

Page* buildPh4Page()
{
	static Page ph4page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH4_PAR1);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 77);
	par2.setText(PH4_PAR2);

	static BlueTextButton ph_scan;
	ph_scan.setXYpos(PG_LEFT_PADD, 120);
	ph_scan.setText(CAL_SCAN_9);
	// TODO: set to calib 9 function
	ph_scan.setCallback(createPhCalibTasks);

	ph4page.setTitle(CAL_PH_TITLE);

	ph4page.addItem(&par1);
	ph4page.addItem(&par2);
	ph4page.addItem(&ph_scan);

	return &ph4page;
}

Page* buildPh3Page()
{
	static Page ph3page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH3_PAR1);

	g_ph3wait.setXYpos(75, 105);

	g_ph_next.setXYpos(PG_LEFT_PADD, 95);
	g_ph_next.setText(BLUE_BTN_NEXT);
	g_ph_next.setCallback(callPage, pages[CAL_PH4_PG]);
	g_ph_next.setInvisible();

	ph3page.setTitle(CAL_PH_TITLE);

	ph3page.addItem(&par1);
	ph3page.addItem(&g_ph3wait);
	ph3page.addItem(&g_ph_next);

	return &ph3page;
}

Page* buildPh2Page()
{
	static Page ph2page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH2_PAR1);

	static BlueTextButton ph_scan;
	ph_scan.setXYpos(PG_LEFT_PADD, 82);
	ph_scan.setText(CAL_SCAN_4);
	// TODO: set to calib 4 func
	ph_scan.setCallback(createPhCalibTasks);

	ph2page.setTitle(CAL_PH_TITLE);
	ph2page.addItem(&par1);
	ph2page.addItem(&ph_scan);

	return &ph2page;
}

Page* buildPh1Page()
{
	static Page ph1page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH1_PAR1);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 77);
	par2.setText(PH1_PAR2);

	static BlueTextButton ph_next;
	ph_next.setXYpos(PG_LEFT_PADD, 120);
	ph_next.setText(BLUE_BTN_NEXT);
	ph_next.setCallback(callPage, pages[CAL_PH2_PG]);

	static Text warn;
	warn.setXYpos(PG_LEFT_PADD, 160);
	warn.setText(CAL_WARN);
	warn.setColors(RED_COL_MACRO, TFT_WHITE);

	ph1page.setTitle(CAL_PH_TITLE);
	//ph1page.setPrev(pages[CAL_SETT_PG]);
	ph1page.addItem(&par1);
	ph1page.addItem(&par2);
	ph1page.addItem(&ph_next);
	ph1page.addItem(&warn);
	ph1page.addItem(&back);

	return &ph1page;
}

/************************ TDS CALIBRATE PAGES *********************/
Page* buildTds5Page()
{
	static Page ph5page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH5_PAR1);

	g_tds5wait.setXYpos(75, 105);

	g_tds_succ.setXYpos(PG_LEFT_PADD, 98);
	g_tds_succ.setText(CAL_SUCC);
	g_tds_succ.setColors(GREEN_COL_MACRO, TFT_WHITE);
	g_tds_succ.setInvisible();

	g_tds_done.setXYpos(PG_LEFT_PADD, 120);
	g_tds_done.setText(CAL_DONE);
	g_tds_done.setInvisible();

	ph5page.setTitle(CAL_TDS_TITLE);

	ph5page.addItem(&par1);
	ph5page.addItem(&g_tds5wait);
	ph5page.addItem(&g_tds_succ);
	ph5page.addItem(&g_tds_done);

	return &ph5page;
}

Page* buildTds4Page()
{
	static Page ph4page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH4_PAR1);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 77);
	par2.setText(TDS4_PAR2);

	static BlueTextButton ph_scan;
	ph_scan.setXYpos(PG_LEFT_PADD, 120);
	ph_scan.setText(TDS4_SCAN_1500);

	ph_scan.setCallback(createTdsCalibTasks);

	ph4page.setTitle(CAL_TDS_TITLE);

	ph4page.addItem(&par1);
	ph4page.addItem(&par2);
	ph4page.addItem(&ph_scan);

	return &ph4page;
}

Page* buildTds3Page()
{
	static Page ph3page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH3_PAR1);

	g_tds3wait.setXYpos(75, 105);

	g_tds_next.setXYpos(PG_LEFT_PADD, 95);
	g_tds_next.setText(BLUE_BTN_NEXT);
	g_tds_next.setCallback(callPage, pages[CAL_TDS4_PG]);
	g_tds_next.setInvisible();

	ph3page.setTitle(CAL_TDS_TITLE);

	ph3page.addItem(&par1);
	ph3page.addItem(&g_tds3wait);
	ph3page.addItem(&g_tds_next);

	return &ph3page;
}

Page* buildTds2Page()
{
	static Page ph2page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH2_PAR1);

	static BlueTextButton ph_scan;
	ph_scan.setXYpos(PG_LEFT_PADD, 82);
	ph_scan.setText(TDS2_SCAN_500);
	// TODO: set to calib 4 func
	ph_scan.setCallback(createTdsCalibTasks);

	ph2page.setTitle(CAL_TDS_TITLE);
	ph2page.addItem(&par1);
	ph2page.addItem(&ph_scan);

	return &ph2page;
}

Page* buildTds1Page()
{
	static Page ph1page;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(TDS1_PAR1);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 77);
	par2.setText(PH1_PAR2);

	static BlueTextButton ph_next;
	ph_next.setXYpos(PG_LEFT_PADD, 120);
	ph_next.setText(BLUE_BTN_NEXT);
	ph_next.setCallback(callPage, pages[CAL_TDS2_PG]);

	static Text warn;
	warn.setXYpos(PG_LEFT_PADD, 160);
	warn.setText(CAL_WARN);
	warn.setColors(RED_COL_MACRO, TFT_WHITE);

	ph1page.setTitle(CAL_TDS_TITLE);
	//ph1page.setPrev(pages[CAL_SETT_PG]);
	ph1page.addItem(&par1);
	ph1page.addItem(&par2);
	ph1page.addItem(&ph_next);
	ph1page.addItem(&warn);
	ph1page.addItem(&back);

	return &ph1page;
}


/************************ TIME PAGE ******************************/
Page* buildTimePage()
{
	// time page is in global scope because why not?
	// it has items that are modifiable from DateTime singleton
	static Image timeCal;
	timeCal.setXYpos(167, 82);
	timeCal.loadRes(images[IMG_TIME_CAL]);

	static CheckBox syncCheck;
	syncCheck.setXYpos(PG_LEFT_PADD, 45);
	syncCheck.setText(DT_SYNC);
	syncCheck.adjustTextY(-7);
	syncCheck.on(g_ntp_sync);
	syncCheck.setCallback(syncTimeCallback, &syncCheck);
	//syncCheck.onClick();

	static Text timeZone;
	timeZone.setXYpos(PG_LEFT_PADD, 97);
	timeZone.setText(DT_ZONE);

	static InputField utc;
	//utc.setCallback(setUTC);
	utc.setWidth(TWO_CHR);
	utc.setXYpos(48, 117);
	utc.showPlus(true);
	utc.setLimits(-11, 14);
	utc.setFont(MIDFONT);
	utc.setAlign(LEFT);
	utc.adjustTextY(2);
	utc.adjustTextX(4);
	utc.setText(DT_UTC);
	utc.setValue(gUTC);
	utc.setCallback(std::bind(&DateTime::setUTC, &datetime, std::placeholders::_1), &utc);

	// fields title done in datetime object
	//static Text currTime;
	//currTime.setXYpos(PG_LEFT_PADD, 150);
	//currTime.setText(DT_CURR);

	timePage.setTitle(DT_TITLE);
	//timePage.setPrev(&settingsPage);

	timePage.addItem(&timeCal);
	timePage.addItem(&syncCheck);
	timePage.addItem(&timeZone);
	timePage.addItem(&utc);

	// fields title done in datetime object
	//timePage.addItem(&currTime);

	// build settings input fields
	datetime.build();
	// all time settings input fields added in datetime object
	timePage.addItem(&datetime);

	timePage.addItem(&back);

	return &timePage;
}


/************************ WIFI PAGE ******************************/

#define WI_PG_FONT_COL 0x44

Page* buildWifiPage()
{

	static Page wifiPage;
	// TEXT
	uint16_t bg_col = greyscaleColor(BACKGROUND);
	uint16_t font1_col = greyscaleColor(FONT_COLOR);
	uint16_t font2_col = greyscaleColor(WI_PG_FONT_COL);
	pages[WIFI_PG] = &wifiPage;

	static Text accesPoint;
	accesPoint.setFont(BOLDFONT);
	// создана точка доступа
	accesPoint.setText(WI_AP_CREATED);
	accesPoint.setXYpos(PG_LEFT_PADD, MB_Y_START);
	accesPoint.setColors(font2_col, bg_col);

	static Text ssid;
	ssid.setFont(SMALLFONT);
	// имя сети
	ssid.setText(WI_SSID_TEXT);
	ssid.setXYpos(PG_LEFT_PADD, 56);
	ssid.setColors(font1_col, bg_col);

	static Text wifiName;
	wifiName.setFont(BOLDSMALL);
	// ezplant_wifi
	wifiName.setText(WI_SSID_NAME);
	wifiName.setXYpos(PG_LEFT_PADD, 74);
	wifiName.setColors(font1_col, bg_col);

	static Text pwd_txt;
	pwd_txt.setFont(SMALLFONT);
	// пароль
	pwd_txt.setText(WI_PWD_TEXT);
	pwd_txt.setXYpos(PG_LEFT_PADD, 101);
	pwd_txt.setColors(font1_col, bg_col);

	static Text pwd;
	pwd.setFont(BOLDSMALL);
	// ezplant
	pwd.setText(WI_PASSWORD);
	pwd.setXYpos(PG_LEFT_PADD, 120);
	pwd.setColors(font1_col, bg_col);

	static Text con_inst;
	con_inst.setFont(SMALLFONT);
	// подключитесь к ней...
	con_inst.setText(WI_CONNECT);
	con_inst.setXYpos(PG_LEFT_PADD, 148);
	con_inst.setColors(font2_col, bg_col);

	static Text ip_addr;
	ip_addr.setFont(BOLDSMALL);
	// http://192...
	ip_addr.setText(WI_IP);
	ip_addr.setXYpos(PG_LEFT_PADD, 189);
	ip_addr.setColors(font2_col, bg_col);

	static Text follow;
	follow.setFont(SMALLFONT);
	// следуйте инструкциям
	follow.setText(WI_FOLLOW);
	follow.setXYpos(PG_LEFT_PADD, 211);
	follow.setColors(font2_col, bg_col);

	// IMAGES
	static Image router;
	router.loadRes(images[IMG_ROUTER]);
	router.setXYpos(168, 60);

	static Image qr;
	qr.loadRes(images[IMG_QR_WI]);
	qr.setXYpos(152, 186);

	wifiPage.addItem(&accesPoint);
	wifiPage.addItem(&ssid);
	wifiPage.addItem(&wifiName);
	wifiPage.addItem(&pwd_txt);
	wifiPage.addItem(&pwd);
	wifiPage.addItem(&con_inst);
	wifiPage.addItem(&ip_addr);
	wifiPage.addItem(&follow);
	wifiPage.addItem(&router);
	wifiPage.addItem(&qr);

	wifiPage.setIconsVis(false);
	wifiPage.setTitle(WI_TITLE);
	//wifiPage.setPrev(&settingsPage);
	wifiPage.addItem(&back);

	return &wifiPage;
}


Page* buildWiFiSettPage()
{
	static Page wifiSettPage;

	// colors
	uint16_t bg_col = greyscaleColor(BACKGROUND);
	uint16_t fg_col = greyscaleColor(FONT_COLOR);

	static Image wsLogo;
	wsLogo.loadRes(images[IMG_LOGO_WIFI]);
	wsLogo.setXYpos(179, 35);

	//static CheckBox gwsWifiChBox;
	gwsWifiChBox.setAlign(LEFT);
	gwsWifiChBox.setFont(BOLDFONT);
	gwsWifiChBox.setXYpos(69, 37);
	gwsWifiChBox.setText(WS_CHECK);
	//gwsWifiChBox.adjustTextY(0);
	gwsWifiChBox.prepare();
	gwsWifiChBox.on(g_wifi_on);
	gwsWifiChBox.setCallback(wifiChCallback);

	static Text wsPar;
	wsPar.setFont(SMALLFONT);
	// Для выгрузки статистики..
	wsPar.setText(WS_PAR);
	wsPar.setXYpos(PG_LEFT_PADD, 60);
	wsPar.setColors(fg_col, bg_col);

	static Text subTitle;
	subTitle.setFont(BOLDFONT);
	// Текущие настройки
	subTitle.setText(WS_SUBT);
	subTitle.setXYpos(PG_LEFT_PADD, 120);
	subTitle.setColors(fg_col, bg_col);

	static Text ssid;
	ssid.setFont(SMALLFONT);
	// TODO: change to current settings
	// имя сети
	ssid.setText(WI_SSID_TEXT);
	ssid.setXYpos(PG_LEFT_PADD, 142);
	ssid.setColors(fg_col, bg_col);

	static StringText wifiName;
	wifiName.setFont(BOLDSMALL);
	// ssid from file
	wifiName.setText(file_ssid);
	wifiName.setXYpos(PG_LEFT_PADD, 160);
	wifiName.setColors(fg_col, bg_col);

	static Text pwd_txt;
	pwd_txt.setFont(SMALLFONT);
	// пароль
	pwd_txt.setText(WI_PWD_TEXT);
	pwd_txt.setXYpos(PG_LEFT_PADD, 185);
	pwd_txt.setColors(fg_col, bg_col);

	static StringText pwd;
	pwd.setFont(BOLDSMALL);
	// password from file
	pwd.setText(file_password);
	pwd.setXYpos(PG_LEFT_PADD, 204);
	pwd.setColors(fg_col, bg_col);

	// Text gwsConnection
	gwsConnection.setFont(SMALLFONT);
	gwsConnection.setText(WS_FAIL);
	gwsConnection.setXYpos(PG_LEFT_PADD, 227);
	gwsConnection.setColors(RED_COL_MACRO, bg_col);

	static BlueTextButton changeWifi;
	changeWifi.setXYpos(PG_LEFT_PADD, 252);
	changeWifi.setText(WS_CHANGE);
	changeWifi.setFont(SMALLFONT);
	changeWifi.setCallback(gChangeWifi);

	wifiSettPage.addItem(&wsLogo);
	wifiSettPage.addItem(&gwsWifiChBox);

	wifiSettPage.addItem(&wsPar);
	wifiSettPage.addItem(&subTitle);
	wifiSettPage.addItem(&ssid);
	wifiSettPage.addItem(&wifiName);
	wifiSettPage.addItem(&pwd_txt);
	wifiSettPage.addItem(&pwd);

	wifiSettPage.addItem(&gwsConnection);

	wifiSettPage.addItem(&changeWifi);

	//wifiSettPage.setPrev(&settingsPage);
	wifiSettPage.setTitle(WS_TITLE);
	wifiSettPage.addItem(&back);

	return &wifiSettPage;
}

/************************ TEST PAGE ******************************/

Page* buildTestPage()
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

	testBlueButton.setXYpos(17, 185);
	testBlueButton.setText(BLUE_BUTTON);
	testBlueButton.setFont(SMALLFONT);
	testBlueButton.setCallback(nop);

	static Wait wait;
	wait.setFont(SMALLESTFONT);
	wait.setText(WAIT_TEXT);
	wait.setXYpos(83, 240);

	testPage.addItem(&testTgl);
	testPage.addItem(&testChBox);
	testPage.addItem(&testRad);
	testPage.addItem(&testInput);
	testPage.addItem(&testGreyButton);
	testPage.addItem(&testBlueButton);
	testPage.addItem(&wait);

	testPage.setTitle(TEST_PAGE);
	testPage.addItem(&back);

	return &testPage;
}


/************************ FONT PAGE ******************************/

Page fontPage;

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



Page* buildFontPage()
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
	fontPage.setTitle(FONT_PAGE);
	fontPage.addItem(&back);

	return &fontPage;
}


/************************************ LANG PAGE ******************************/
// hardcoded
Page* buildLangPage()
{
	static Page langPage;
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
	gBrightness.setFont(SMALLFONT);
	gBrightness.setXYpos(PG_LEFT_PADD, 83);
	gBrightness.setValue(g_init_brightness);
	gBrightness.setText(PERCENT);
	gBrightness.setCallback(gSetBacklight);
	gBrightness.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	/*
	static Text percent;
	percent.setFont(SMALLFONT);
        percent.setText(PERCENT);
	percent.setXYpos(
			gBrightness.getX()
			//+ gBrightness.getW()
			+ 40
			+ 3,
			83 + GR_BTN_Y_PADDING
			);
        percent.setColors(
                     greyscaleColor(FONT_COLOR),
                     greyscaleColor(BACKGROUND)
		     );
		     */

	static Text sleep;
	sleep.setFont(SMALLFONT);
	sleep.setText(DIM);
	sleep.setXYpos(PG_LEFT_PADD, 110);
	sleep.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(BACKGROUND)
			);

	static Text after;
	after.setFont(SMALLFONT);
	after.setText(AFTER);
	after.setXYpos(PG_LEFT_PADD, 130);
	after.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(BACKGROUND)
			);


	gDimseconds.setFont(SMALLFONT);
	gDimseconds.setXYpos(55, 125);
	gDimseconds.setValue(g_dimafter);
	gDimseconds.setLimits(LOWER_DIMAFTER, HIGHER_DIMAFTER);
	gDimseconds.setText(SEC);
	gDimseconds.setCallback(gDimAfter);
	gDimseconds.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(GR_BTN_BG_COLOR)
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
	//ruSelect.on(true);
	ruSelect.setCallback(changeLangRus);

	//static CircRadBtn enSelect;
	enSelect.setXYpos(20, 238);
	enSelect.setCurCol(greyscaleColor(TOP_BAR_BG_COL));
	//enSelect.on(false);
	enSelect.setCallback(changeLangEng);

	switch (g_selected_lang) {
		default:
		case RU_LANG: ruSelect.on(true); enSelect.on(false); break;
		case EN_LANG: enSelect.on(true); ruSelect.on(false); break;
	}

	static Image ruFlag;
	ruFlag.loadRes(images[IMG_RU]);
	ruFlag.setXYpos(113, 180);

	static Image usFlag;
	usFlag.loadRes(images[IMG_US]);
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


	langPage.setTitle(LANG);
	langPage.addItem(&boldScreen);
	langPage.addItem(&subtScreen);
	langPage.addItem(&gBrightness);
	//langPage.addItem(&percent);
	langPage.addItem(&sleep);
	langPage.addItem(&after);
	langPage.addItem(&gDimseconds);
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

	//langPage.setPrev(&settingsPage);
	langPage.addItem(&back);

	return &langPage;
}

enum mainMenuItems {
	MM_PLANT,
	MM_MON,
	MM_SETT,
	MM_DIAG,
	MM_TEST,
	MM_FONT,
	MM_NITEMS
};

Page* buildMenuPage()
{
	static Page mainPage;
	static GreyTextButton menu_items[MM_NITEMS];

	// gap between items
	int gap = MENU_GAP;

	dispStrings_t menu1[MM_NITEMS];
	menu1[MM_PLANT] = NEW_PLANT;
	menu1[MM_MON] = ONLINE_MON;
	menu1[MM_SETT] = SETTINGS;
	menu1[MM_DIAG] = DIAG;
	menu1[MM_TEST] = TEST_PAGE;
	menu1[MM_FONT] = FONT_PAGE;

	int j = 0;

	// omg, change that...                           or not.
	for (auto& i:menu_items) {
		i.setCallback(nop);
		i.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));
		i.setFont(SMALLFONT);
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+(GREY_BUTTON_HEIGHT+gap)*j);
		i.setText(menu1[j]);
		j++;
	}

	// set callBacks
	menu_items[MM_SETT].setCallback(callPage, pages[SETT_PG]);
	menu_items[MM_TEST].setCallback(callPage, pages[TEST_PG]);
	menu_items[MM_FONT].setCallback(callPage, pages[FONT_PG]);
	menu_items[MM_DIAG].setCallback(callPage, pages[DIAG_PG]);

	// add all to page
	for (int i = 0; i < MM_NITEMS; i++) {
		mainPage.addItem(&menu_items[i]);
	}

	// TODO: change to setPrev, move "back" init somewhere else
	back.setCallback(nop);
	back.loadRes(images[IMG_PREV]);
	back.setXYpos(7, 284);
	back.setCircle();

	mainPage.setTitle(MENU);
	mainPage.addItem(&back);
	return &mainPage;
}

enum settingsPageMenuItems {
	MN_TIMEDATE,
	MN_WIFI,
	MN_SCREENLANG,
	MN_CALIB,
	MN_THRES,
	MN_NITEMS
};

Page* buildSettingsPage()
{
	static GreyTextButton settings_items[MN_NITEMS];

	// string pointers for items
	dispStrings_t ru_menu_settings[MN_NITEMS];
	ru_menu_settings[MN_TIMEDATE] = TIMEDATE;
	ru_menu_settings[MN_WIFI] = WIFI;
	ru_menu_settings[MN_SCREENLANG] = SCREENLANG;
	ru_menu_settings[MN_CALIB] = CALIB;
	ru_menu_settings[MN_THRES] = THRES;

	int j = 0;
	int gap = MENU_GAP;

	for (auto& i:settings_items) {
		i.setCallback(nop);
		i.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));
		i.setFont(SMALLFONT);
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+(GREY_BUTTON_HEIGHT+gap)*j);
		i.setText(ru_menu_settings[j]);
		j++;
	}

	settings_items[MN_TIMEDATE].setCallback(callPage, pages[TIME_PG]);

	if (g_wifi_set) {
		settings_items[MN_WIFI].setCallback(callPage, pages[WIFI_SETT_PG]);
	}
	else {
		settings_items[MN_WIFI].setCallback(callPage, pages[WIFI_PG]);
	}
	settings_items[MN_SCREENLANG].setCallback(callPage, pages[LANG_PG]);

	settings_items[MN_CALIB].setCallback(callPage, pages[CAL_SETT_PG]);

	for (int i = 0; i < MN_NITEMS; i++) {
		settingsPage.addItem(&settings_items[i]);
	}

	settingsPage.setTitle(SETTINGS);
	settingsPage.addItem(&back);

	return &settingsPage;
}

enum diagPageItems {
	DP_PWROUT,
	DP_DIGIN,
	DP_ADCIN,
	DP_SENSD,
	DP_NITEMS
};

Page* buildDiagPage()
{
	static Page diagPage;

	// string pointers for items
	dispStrings_t diagMenuButtons[DP_NITEMS];
	diagMenuButtons[DP_PWROUT] = DIAG_PWR;
	diagMenuButtons[DP_DIGIN] = DIAG_DIG;
	diagMenuButtons[DP_ADCIN] = DIAG_ADC;
	diagMenuButtons[DP_SENSD] = DIAG_SENS;

	static GreyTextButton diag_items[DP_NITEMS];

	int j = 0;
	int gap = MENU_GAP;

	for (auto& i:diag_items) {
		i.setCallback(nop);
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+(GREY_BUTTON_HEIGHT+gap)*j);
		i.setText(diagMenuButtons[j]);
		diagPage.addItem(&i);
		j++;
	}

	diag_items[DP_SENSD].setCallback(callPage, pages[SENS_DIAG_PG]);

	diagPage.setTitle(DIAG);
	diagPage.addItem(&back);

	return &diagPage;
}

Page* buildSensDiag()
{
	static Page sensDiagPage;

	static GreyTextButton tds;
	tds.setXYpos(PG_LEFT_PADD, MB_Y_START);
	tds.setText(CAL_TDS);
	tds.setCallback(checkTdsSensor);

	static GreyTextButton ph;
	ph.setXYpos(PG_LEFT_PADD, MB_Y_START+GREY_BUTTON_HEIGHT+MENU_GAP);
	ph.setText(CAL_PH);
	ph.setCallback(checkPhSensor);

	sensDiagPage.setTitle(DIAG_SENS);
	sensDiagPage.addItem(&tds);
	sensDiagPage.addItem(&ph);
	sensDiagPage.addItem(&back);

	return &sensDiagPage;
}

void tdsDiagBack(void* arg)
{
	// switch off relay
	second_expander.digitalWrite(TDS_MTR_RLY, LOW);
	// get back
	callPage(currPage->prev());
}

ImageButton tds_diag_back;

Page* buildTDSdiag()
{
	static Page tdsdiag;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(TDS_DIAG_PAR);

	//static OutputField out1;
	g_tds_read.setText(TDS_DIAG_PPM);
	g_tds_read.setXYpos(74, 131);
	g_tds_read.setInvisible();
	g_tds_read.setWidth(FOUR_CHR);

	//static Wait tdsWait;
	g_tdsWait.setXYpos(82, 165);

	tds_diag_back.setCallback(tdsDiagBack);
	tds_diag_back.loadRes(images[IMG_PREV]);
	tds_diag_back.setXYpos(7, 284);
	tds_diag_back.setCircle();

	tdsdiag.setTitle(CAL_TDS);
	tdsdiag.addItem(&par1);
	tdsdiag.addItem(&g_tdsWait);
	tdsdiag.addItem(&g_tds_read);
	tdsdiag.addItem(&tds_diag_back);

	return &tdsdiag;
}

Page* buildPhdiag()
{
	static Page phdiag;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PH_DIAG_PAR);

	//static OutputField out1;
	g_ph_read.setText(PH_DIAG_PH);
	g_ph_read.setXYpos(80, 93);
	g_ph_read.setInvisible();
	g_ph_read.setFloat();

	//static Wait phWait;
	g_phWait.setXYpos(83, 143);

	phdiag.setTitle(CAL_PH);
	phdiag.addItem(&par1);
	phdiag.addItem(&g_phWait);
	phdiag.addItem(&g_ph_read);
	phdiag.addItem(&back);

	return &phdiag;
}

void gSetBacklight(void* arg)
{
	uint8_t mapped_br = map(gBrightness.getValue(), 0, 100, 0, 255);
	analogWrite(LED_PIN, mapped_br);
}

#ifdef APP_DEBUG
#define STACK_CHECK_INTERVAL 10000
unsigned long oldMillis;
#endif

void linkAllPages()
{
#ifdef APP_DEBUG
	Serial.println("pages pointers: ");
	for (auto i:pages)
		DEBUG_PRINT_HEX(i);
#endif

	pages[SENS_FAIL_PG]->setPrev(pages[CAL_SETT_PG]);
	pages[CAL_PH1_PG]->setPrev(pages[CAL_SETT_PG]);
	pages[CAL_TDS1_PG]->setPrev(pages[CAL_SETT_PG]);

	pages[SETT_PG]->setPrev(pages[MENU_PG]);
	pages[FONT_PG]->setPrev(pages[MENU_PG]);
	pages[TEST_PG]->setPrev(pages[MENU_PG]);

	pages[WIFI_SETT_PG]->setPrev(pages[SETT_PG]);
	pages[WIFI_PG]->setPrev(pages[SETT_PG]);
	pages[TIME_PG]->setPrev(pages[SETT_PG]);
	pages[CAL_SETT_PG]->setPrev(pages[SETT_PG]);
	pages[LANG_PG]->setPrev(pages[SETT_PG]);

	pages[DIAG_PG]->setPrev(pages[MENU_PG]);
	pages[SENS_DIAG_PG]->setPrev(pages[DIAG_PG]);

	pages[TDS_DIAG_PG]->setPrev(pages[SENS_DIAG_PG]);
	pages[PH_DIAG_PG]->setPrev(pages[SENS_DIAG_PG]);

	g_ph_done.setCallback(callPage, pages[MENU_PG]);
	g_tds_done.setCallback(callPage, pages[MENU_PG]);
}

void setup(void)
{
#ifdef APP_DEBUG
	Serial.begin(115200);
#endif
	SPIFFS.begin();

	if (!loadSettings()) {
#ifdef APP_DEBUG
		Serial.println("load settings failed");
	}
	else {
		Serial.println("settings loaded");
#endif
	}

	checkWifi();
	// init all stuff in Gui.h
	app.init();

	// diag pages
	pages[TDS_DIAG_PG] = buildTDSdiag();
	pages[PH_DIAG_PG] = buildPhdiag();
	pages[SENS_DIAG_PG] = buildSensDiag();
	pages[DIAG_PG] = buildDiagPage();

	// ph calib pages
	pages[SENS_FAIL_PG] = buildSensorFailPage();
	pages[CAL_PH5_PG] = buildPh5Page();
	pages[CAL_PH4_PG] = buildPh4Page();
	pages[CAL_PH3_PG] = buildPh3Page();
	pages[CAL_PH2_PG] = buildPh2Page();
	pages[CAL_PH1_PG] = buildPh1Page();

	// tds calib pages
	pages[CAL_TDS5_PG] = buildTds5Page();
	pages[CAL_TDS4_PG] = buildTds4Page();
	pages[CAL_TDS3_PG] = buildTds3Page();
	pages[CAL_TDS2_PG] = buildTds2Page();
	pages[CAL_TDS1_PG] = buildTds1Page();

	// common calib page
	pages[CAL_SETT_PG] = buildCalSettPage();

	pages[TIME_PG] = buildTimePage();
	pages[WIFI_SETT_PG] = buildWiFiSettPage();
	pages[WIFI_PG] = buildWifiPage();
	pages[FONT_PG] = buildFontPage();
	pages[TEST_PG] = buildTestPage();
	pages[LANG_PG] = buildLangPage();
	pages[SETT_PG] = buildSettingsPage();
	pages[MENU_PG] = buildMenuPage();

	topBar.build();


	linkAllPages();

	/*
	pinMode(LED_PIN, OUTPUT);
	pinMode(PUMP_F, OUTPUT);
	pinMode(PUMP_G, OUTPUT);
	pinMode(PUMP_H, OUTPUT);
	pinMode(LED, OUTPUT);
	pinMode(FAN, OUTPUT);
	analogWrite(PUMP_F, 127);
	analogWrite(PUMP_G, 127);
	analogWrite(PUMP_H, 127);
	analogWrite(LED, 127);
	analogWrite(FAN, 127);
	*/

	// backlight
	gBrightness.onClick();

	currPage = pages[MENU_PG];
	currPage->setCurrItem(0);
	currItem = currPage->getCurrItem();
	currPage->prepare();

	//buildTopBar();

	currPage->draw();
	topBar.draw();

#ifdef APP_DEBUG
	oldMillis = millis();
#endif

	// expander stuff
	buttons.begin();
	second_expander.begin();
	buttons.portMode(2, pinsAll(INPUT));
	second_expander.portMode(1, pinsAll(INPUT));
	second_expander.portMode(0, pinsAll(OUTPUT));
	second_expander.portWrite(0, pinsAll(LOW));
	pinMode(EXPANDER_INT, INPUT_PULLUP);

	rtc.begin();
	datetime.init();
	datetime.prepare();

	//delay(500);

	/*
	xTaskCreate(
			gui,
			"gui",
			getArduinoLoopTaskStackSize(),
			NULL,
			1,
			NULL
		   );
	vTaskDelete(NULL);
	*/
}

void loop()
{
	app.update();
	//heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
#ifdef APP_DEBUG
	if (millis() - oldMillis > STACK_CHECK_INTERVAL) {

		uint16_t unused = uxTaskGetStackHighWaterMark(NULL);
		Serial.print("gui task unused stack: ");
		Serial.println(unused);
		oldMillis = millis();
	}
#endif

}
