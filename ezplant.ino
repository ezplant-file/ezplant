//TODO: consider resource manager and page builder classes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_task_wdt.h"

#define TASKS
#define APP_DEBUG
#include "settings.h"

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
#include "App.h"
#include "IO.h"

#define second_expander gpio[1]

// json stuff
#include "json.hpp"
using json = nlohmann::json;

static App app;

static Page settingsPage;
// screen buttons
//#define menu1_size 6
//#define settings_size 5
static ImageButton back;
static ImageButton forward;
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
static RadioButton testRad;
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
		g_first_launch = load["g_first_launch"].get<bool>();

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
	save["g_first_launch"] = g_first_launch;

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

void diagToggleCallback(void* arg, int id)
{
	if (arg == nullptr)
		return;

	Toggle* tgl = (Toggle*) arg;

	tgl->on(!tgl->isOn());
	tgl->invalidate();
	tgl->draw();

	if (id < 0 || id > PWR_PG_NITEMS)
		return;

	io.driveOut(id, tgl->isOn());

	if (id == PWR_PG_DOWN && exlMotorTgl[MOTOR_UP]->isOn()) {
		exlMotorTgl[MOTOR_UP]->on(false);
		exlMotorTgl[MOTOR_UP]->invalidate();
	}

	if (id == PWR_PG_UP && exlMotorTgl[MOTOR_DOWN]->isOn()) {
		exlMotorTgl[MOTOR_DOWN]->on(false);
		exlMotorTgl[MOTOR_DOWN]->invalidate();
	}
}

// dim screen setting input field
void gDimAfter(void* arg)
{
	g_dimafter = gDimseconds.getValue();
}

// change wifi button
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

	if (page->prev() == nullptr) {
		back.setCallback(nop);
	}
	else {
		back.setCallback(callPage, page->prev());
	}

	if (page->next() == nullptr) {
		forward.setCallback(nop);
	}
	else if (page == pages[STAGE7_PG]) {
		forward.setCallback(rigtypeForward);
	}
	else if (page == pages[STAGE8_PG]) {
		forward.setCallback(rigtypeForward);
	}
	else if (page == pages[STAGE5_PG]) {
		forward.setCallback(callStage6);
	}
	else {
		forward.setCallback(callPage, page->next());
	}

	if (currPage->lastStage()) {
		g_first_launch = false;
		// save rig setup day of the year
		datetime.setStartDay();
		g_data.set(START_DAY, datetime.getStartDay());
		g_data.save();
		saveSettings();
		g_rig.start();
	}


	currPage->freeRes();

	page->invalidateAll();
	page->prepare();

	topBar.erase();
	topBar.invalidateAll();
	currPage->erase();
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


// wifi on/off checkbox
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

// test page callbacks
void tglCallback(void* arg, int i)
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

// lang page callbacks
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

// time page callback
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
	par1.setText(TDS2_PAR1);

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
	//gwsWifiChBox.prepare();
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
	//testTgl.prepare();
	testTgl.on(false);
	testTgl.setCallback(tglCallback);

	testChBox.setFont(SMALLFONT);
	testChBox.setXYpos(17, 65);
	testChBox.setText(CHECHBOX_TEXT);
	//testChBox.prepare();
	testChBox.on(false);
	testChBox.setCallback(chkCallback);

	testRad.setFont(SMALLFONT);
	testRad.setXYpos(17, 90);
	testRad.setText(RADIO_TEXT);
	//testRad.prepare();
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
	subtScreen.setXYpos(PG_LEFT_PADD, 63);
	subtScreen.setFont(SMALLFONT);
	subtScreen.setText(BRIGHT);
	subtScreen.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(BACKGROUND)
			);

	// Поле ввода
	gBrightness.setXYpos(PG_LEFT_PADD, 83);
	gBrightness.setFont(SMALLFONT);
	gBrightness.setValue(g_init_brightness);
	gBrightness.setText(PERCENT);
	gBrightness.setCallback(gSetBacklight);
	gBrightness.setLimits(1, 100);
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
	gDimseconds.setText(TXT_SEC);
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
	//MM_FIRST_PAGE,
	MM_NITEMS
};

Page* buildMenuPage()
{
	static Page menuPage;
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
	//menu1[MM_FIRST_PAGE] = FP_SUBTTL;

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
	menu_items[MM_PLANT].setCallback(callPage, pages[STAGE1_PG]);
	menu_items[MM_SETT].setCallback(callPage, pages[SETT_PG]);
	menu_items[MM_TEST].setCallback(callPage, pages[TEST_PG]);
	menu_items[MM_FONT].setCallback(callPage, pages[FONT_PG]);
	menu_items[MM_DIAG].setCallback(callPage, pages[DIAG_PG]);
	//menu_items[MM_FIRST_PAGE].setCallback(callPage, pages[STAGE1_PG]);

	// add all to page
	for (int i = 0; i < MM_NITEMS; i++) {
		menuPage.addItem(&menu_items[i]);
	}


	//menuPage.setPrev(&menuPage);

	menuPage.setTitle(MENU);
	menuPage.addItem(&back);
	return &menuPage;
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
	diag_items[DP_ADCIN].setCallback(callPage, pages[ADC_DIAG_PG]);
	diag_items[DP_DIGIN].setCallback(callPage, pages[DIG_DIAG_PG]);
	diag_items[DP_PWROUT].setCallback(callPage, pages[PWR_DIAG_PG]);

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

// tds relay special case
// TODO: make structure to deal with all i2c
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
	g_tds_read.setXYpos(74, 131);
	g_tds_read.setText(TDS_DIAG_PPM);
	g_tds_read.setInvisible();
	g_tds_read.setWidth(FOUR_CHR);

	//static Wait tdsWait;
	g_tdsWait.setXYpos(82, 165);

	tds_diag_back.setXYpos(7, 284);
	tds_diag_back.setCallback(tdsDiagBack);
	tds_diag_back.loadRes(images[IMG_PREV]);
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
	g_ph_read.setXYpos(80, 93);
	g_ph_read.setText(PH_DIAG_PH);
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

Page* buildADCdiag()
{
	static Page adcDiag;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(SENS_DIAG_PAR);

	adcDiag.addItem(&par1);

	static OutputField ADC[N_ADC];

	int x = 94;
	int y = 99;
	int j = 0;
	int gap = 12;

	for (auto& i:ADC) {
		gADC[j] = &i;
		i.setXYpos(x, y+(INPUT_H+gap)*j);
		i.setAlign(LEFT);
		i.setWidth(FOUR_CHR);
		adcDiag.addItem(&i);
		//i.setText(SENS_DIAG_A1+static_cast<dispStrings_t>(j));
		j++;
	}

	ADC[0].setText(SENS_DIAG_A1);
	ADC[1].setText(SENS_DIAG_A2);
	ADC[2].setText(SENS_DIAG_A3);
	ADC[3].setText(SENS_DIAG_A4);

	adcDiag.setTitle(DIAG_ADC);
	adcDiag.addItem(&back);

	return &adcDiag;
}

Page* buildDigDiag()
{
	static Page digDiag;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(DIG_DIAG_PAR);

	digDiag.addItem(&par1);

	static CircIndicator indicators[DIG_NKEYS];

	int x = PG_LEFT_PADD;
	int y = 62;
	int j = 0;
	int k = 0;
	int gap = 12;

	indicators[DIG_KEY1].setText(DIG_DIAG_1);
	indicators[DIG_KEY2].setText(DIG_DIAG_2);
	indicators[DIG_KEY3].setText(DIG_DIAG_3);
	indicators[DIG_KEY4].setText(DIG_DIAG_4);
	indicators[DIG_KEY5].setText(DIG_DIAG_5);
	indicators[DIG_KEY6].setText(DIG_DIAG_6);
	indicators[DIG_KEY7].setText(DIG_DIAG_7);
	indicators[DIG_KEY8].setText(DIG_DIAG_8);
	indicators[DIG_KEY9].setText(DIG_DIAG_9);
	indicators[DIG_KEY10].setText(DIG_DIAG_10);

	for (auto& i:indicators) {
		if (j > 4) {
			x = 129;
			j = 0;
		}
		i.setXYpos(x, y+(RAD_BTN_SIZE+gap)*j);
		digDiag.addItem(&i);
		gKEYS[k] = &i;
		j++;
		k++;
	}

	digDiag.setTitle(DIAG_DIG);

	digDiag.addItem(&back);

	return &digDiag;
}

// power outputs diag page (items enum in IO.h, don't ask...)
Page* buildPwrDiag()
{
	static Page pwrDiag;

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	par1.setText(PWR_PAR);

	static Text motor;
	motor.setXYpos(PG_LEFT_PADD, 244);
	motor.setText(PWR_MOTOR_STR);

	pwrDiag.addItem(&par1);
	pwrDiag.addItem(&motor);

	static Toggle diagItems[PWR_PG_NITEMS];
	exlMotorTgl[MOTOR_UP] = &diagItems[PWR_PG_UP];
	exlMotorTgl[MOTOR_DOWN] = &diagItems[PWR_PG_DOWN];

	int x = 73;
	int y = 62;
	int j = 0;
	int gap = 12;
	int textx = PG_LEFT_PADD;

	for (int i = 0; i < PWR_PG_NITEMS; i++) {
		if (i == 5) {
			textx = 131;
			x = 187;
			j = 0;
		}
		diagItems[i].setTextX(textx);
		diagItems[i].setAlign(LEFT);
		diagItems[i].setXYpos(x, y+(RAD_BTN_SIZE+gap)*j);
		diagItems[i].setCallback(diagToggleCallback, &diagItems[i], i);
		pwrDiag.addItem(&diagItems[i]);
		j++;
	}

	diagItems[PWR_PG_UP].setTextX(162);

	diagItems[PWR_PG_PORT_A].setText(PWR_PORT_A);
	diagItems[PWR_PG_PORT_B].setText(PWR_PORT_B);
	diagItems[PWR_PG_PORT_C].setText(PWR_PORT_C);
	diagItems[PWR_PG_PORT_D].setText(PWR_PORT_D);
	diagItems[PWR_PG_PORT_E].setText(PWR_PORT_E);
	diagItems[PWR_PG_PORT_F].setText(PWR_PORT_F);
	diagItems[PWR_PG_PORT_G].setText(PWR_PORT_G);
	diagItems[PWR_PG_PORT_H].setText(PWR_PORT_H);
	diagItems[PWR_PG_FAN].setText(PWR_FAN);
	diagItems[PWR_PG_LIGHT].setText(PWR_LIGHT);
	diagItems[PWR_PG_UP].setText(PWR_MOTOR_UP);
	diagItems[PWR_PG_DOWN].setText(PWR_MOTOR_DOWN);

	diagItems[PWR_PG_UP].setXYpos(x, 244);
	diagItems[PWR_PG_DOWN].setXYpos(x, 244+RAD_BTN_SIZE+10);

	static Line line(210);
	line.setXYpos(PG_LEFT_PADD, 230);
	pwrDiag.addItem(&line);
	//diagItems[PWR_PG_PORT_A].setCallback();

	pwrDiag.setTitle(DIAG_PWR);

	pwrDiag.addItem(&back);

	return &pwrDiag;
}

/* settings input fields callback */

enum {
	SECOND_OUT,
	THIRD_OUT,
	NOUTS
};

OutputField* daysOut[NOUTS];

void inputsCallback(void* inputptr)
{
	if (inputptr == nullptr)
		return;

	InputField* input = (InputField*) inputptr;

	rig_settings_t id = input->getSettingsId();

	if (input->isFloat())
		g_data.set(id, input->getFvalue());
	else
		g_data.set(id, input->getValue());

	// nmeddog...
	if (id == GR_CYCL_1_DAYS)
		daysOut[SECOND_OUT]->setValue(input->getValue());

	if (id == GR_CYCL_2_DAYS)
		daysOut[THIRD_OUT]->setValue(input->getValue());
}

/******* settings page builders and callbacks *******/
Page* buildFirstPage()
{
	static Page firstPlanting;
	static Text heading1;
	heading1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	heading1.setFont(BOLDFONT);
	heading1.setText(FP_SUBTTL);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 63);
	par1.setText(FP_PAR);

	static BlueTextButton start;
	start.setXYpos(PG_LEFT_PADD, 153);
	start.setText(FP_BTN);
	start.setCallback(callPage, pages[STAGE1_PG]);

	static Image seeds;
	seeds.setXYpos(174, 142);
	seeds.loadRes(images[IMG_SEEDS]);

	firstPlanting.addItem(&heading1);
	firstPlanting.addItem(&par1);
	firstPlanting.addItem(&start);
	firstPlanting.addItem(&seeds);
	firstPlanting.addItem(&back);

	firstPlanting.setTitle(FP_TITLE);

	return &firstPlanting;
}

/*
enum {
	RB_UNDERW,
	RB_LAYER,
	RB_PERIODIC,
	RB_AERO,
	RB_DRIP,
	RB_OPENG,
	RB_GREENHS,
	RB_MIXING,
	RB_NITEMS
};
*/

RigTypeRadioButton rbuttons[RIG_NTYPES];

void rigtypeCallback(void* btn)
{
	if (btn == nullptr)
		return;

	for (auto& i:rbuttons)
		i.on(false);

	RadioButton* button = (RadioButton*) btn;
	button->on(true);

	// :::::::::facepalm, project Zeus:::::::::
	int j = 0;
	for (auto& i:rbuttons) {
		if (i.isOn())
			g_rig_type = (rig_type)j;
		j++;
	}
	forward.setCallback(rigtypeForward);
	/*
	Serial.print("rig type: ");
	Serial.println(g_rig_type);
	*/
}

void rigtypeForward(void* arg)
{
	if (currPage == pages[STAGE1_PG]) {
		switch (g_rig_type) {
			default: break;
			case RIG_DEEPWATER:
			case RIG_LAYER:
			case RIG_FLOOD:
			case RIG_AERO:
			case RIG_DRIP: callPage(pages[STAGE2_PG]); break;
		}
	}
	else if (currPage == pages[STAGE7_PG]) {
		switch (g_rig_type) {
			default: break;
			case RIG_DEEPWATER: callPage(pages[STAGE8_PG]); break;
			case RIG_LAYER: callPage(pages[STAGE8_PG]); break;
			case RIG_FLOOD: callPage(pages[STAGE82_PG]); break;
			case RIG_AERO: callPage(pages[STAGE83_PG]); break;
			case RIG_DRIP: callPage(pages[STAGE84_PG]); break;
		}
	}
	// 83 - RIG_AERO, 84 - RIG_DRIP
	else if (currPage == pages[STAGE83_PG] || currPage == pages[STAGE84_PG]) {
		callPage(pages[STAGE92_PG]);
	}
	// only for deepwater
	else if (currPage == pages[STAGE8_PG]) {
		callPage(pages[STAGE9_PG]);
	}
}

void saveInputFieldSetting(void* in)
{
	if (in == nullptr)
		return;

	InputField* input = (InputField*) in;

	if (input->isFloat())
		g_data.set(input->getSettingsId(), input->getFvalue());
	else
		g_data.set(input->getSettingsId(), input->getValue());
}

void saveCheckBoxSetting(void* check)
{
	if (check == nullptr)
		return;

	CheckBox* ch = (CheckBox*) check;

	ch->on(!ch->isOn());
	g_data.set(ch->getSettingsId(), ch->isOn());
	ch->invalidate();
	ch->prepare();
}

/*
void saveRadioBtnSetting(void* radio)
{
	if (radio == nullptr)
		return;

	RadioButton* rb = (RadioButton*) radio;

	g_data.set(rb->getSettingsId(), rb->isOn());
}
*/

Page* buildStage1()
{
	static Page stage1;
	static Text heading1;
	heading1.setXYpos(PG_LEFT_PADD, MB_Y_START);
	heading1.setFont(BOLDFONT);
	heading1.setText(S1_SUBTTL);

	stage1.addItem(&heading1);


	int gap = 4;
	int j = 0;

	for (auto& i:rbuttons) {
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+20+(RAD_BTN_SIZE+gap)*j);
		i.setCallback(rigtypeCallback, &i);
		stage1.addItem(&i);
		j++;
	}

	rbuttons[RIG_DEEPWATER].setText(S1_UNDERWTR);
	rbuttons[RIG_LAYER].setText(S1_LAYER);
	rbuttons[RIG_FLOOD].setText(S1_PERIODIC);
	rbuttons[RIG_AERO].setText(S1_AERO);
	rbuttons[RIG_DRIP].setText(S1_DRIP);
	rbuttons[RIG_OPENG].setText(S1_OPENG);
	rbuttons[RIG_GREENH].setText(S1_GREENHS);
	rbuttons[RIG_MIXSOL].setText(S1_MIXING);

	//
	rbuttons[g_rig_type].on(true);

	stage1.setNext(pages[STAGE2_PG]);
	stage1.setTitle(S1_TITLE);
	stage1.addItem(&forward);

	return &stage1;
}


// title checkboxes
void checkBoxCallback(void* checkptr, void* pageptr)
{
	if (checkptr == nullptr || pageptr == nullptr)
		return;

	CheckBox* check = (CheckBox*) checkptr;
	Page* page = (Page*) pageptr;


	if (check->isOn()) {
		check->on(false);
		page->setInvisible();
		check->setVisible();
	}
	else {
		check->on(true);
		page->setVisible();
	}
	g_data.set(check->getSettingsId(), check->isOn());
	page->restock();
}

Page* buildStage2()
{
	static Page stage2;
	stage2.setTitle(S2_TITLE);
	stage2.setNext(pages[STAGE3_PG]);

	static Image bulbImg;
	bulbImg.setXYpos(164, 40);
	bulbImg.loadRes(images[IMG_BULB]);
	bulbImg.neverHide();

	static CheckBox lightCheck;
	lightCheck.setXYpos(111, MB_Y_START);
	lightCheck.setText(S2_LIGHT);
	lightCheck.setAlign(LEFT);
	lightCheck.setFont(BOLDFONT);
	//lightCheck.setCallback(lightCallback, &lightCheck);
	lightCheck.setSettingsId(LIGHT_ON);
	lightCheck.setCallback(checkBoxCallback, &lightCheck, &stage2);
	lightCheck.on(g_data.getInt(LIGHT_ON));
	lightCheck.neverHide();

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 67);
	par1.setText(S2_PAR1);
	par1.neverHide();

	static HourLimits hlim = HourLimits(PG_LEFT_PADD, 143);
	InputField* low = (InputField*) hlim.getLowerPtr();
	InputField* high = (InputField*) hlim.getHigherPtr();
	low->setSettingsId(LIGHT_FROM);
	high->setSettingsId(LIGHT_TO);
	low->setCallback(saveInputFieldSetting, low);
	high->setCallback(saveInputFieldSetting, high);
	low->setValue(g_data.getInt(LIGHT_FROM));
	high->setValue(g_data.getInt(LIGHT_TO));

	static Text heading2;
	heading2.setXYpos(PG_LEFT_PADD, 180);
	heading2.setFont(BOLDFONT);
	heading2.setText(S2_SUBTTL2);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 204);
	par2.setText(S2_PAR2);

	static Text s;
	s.setXYpos(PG_LEFT_PADD, 268);
	s.setText(S2_FROM);

	static InputField days;
	days.setXYpos(34, 263);
	days.setWidth(FOUR_CHR);
	days.setText(S2_DAY);
	days.setSettingsId(LIGHT_DAY);
	days.setValue(g_data.getInt(LIGHT_DAY));
	days.setCallback(saveInputFieldSetting, &days);

	stage2.addItem(&bulbImg);
	//stage2.addItem(&heading1);
	stage2.addItem(&lightCheck);
	stage2.addItem(&par1);

	stage2.addItem((ScrObj*)hlim.getLowerPtr());
	stage2.addItem(hlim.getDashPtr());
	stage2.addItem((ScrObj*)hlim.getHigherPtr());

	stage2.addItem(&heading2);
	stage2.addItem(&par2);
	stage2.addItem(&s);
	stage2.addItem(&days);

	stage2.addItem(&forward);

	if (!lightCheck.isOn()) {
		stage2.setInvisible();
		stage2.restock();
	}

	return &stage2;
}

Page* buildStage3()
{
	static Page stage3;
	stage3.setTitle(S3_TITLE);
	stage3.setNext(pages[STAGE4_PG]);

	static CheckBox ventCheck;
	ventCheck.setXYpos(119, MB_Y_START);
	ventCheck.setText(S3_VENT);
	ventCheck.setAlign(LEFT);
	ventCheck.setFont(BOLDFONT);
	ventCheck.neverHide();
	ventCheck.setSettingsId(VENT_ON);
	ventCheck.setCallback(checkBoxCallback, &ventCheck, &stage3);
	ventCheck.on(g_data.getInt(VENT_ON));

	static Image fanImg;
	fanImg.setXYpos(168, MB_Y_START);
	fanImg.loadRes(images[IMG_COOLER]);
	fanImg.neverHide();

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 67);
	par1.setText(S3_PAR1);
	par1.neverHide();

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 117);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S3_SUBTTL);

	static Text timeint;
	timeint.setXYpos(PG_LEFT_PADD, 143);
	timeint.setText(S3_TIME);

	static CheckBox timeCheck;
	timeCheck.setXYpos(PG_LEFT_PADD, 163);
	timeCheck.setText(EMPTY_STR);
	timeCheck.setSettingsId(VENT_TIME_LIM);
	timeCheck.setCallback(saveCheckBoxSetting, &timeCheck);
	timeCheck.on(g_data.getInt(VENT_TIME_LIM));

	static HourLimits timeLimit(45, 163);
	InputField* low = (InputField*) timeLimit.getLowerPtr();
	InputField* high = (InputField*) timeLimit.getHigherPtr();
	low->setSettingsId(VENT_TIME_FROM);
	high->setSettingsId(VENT_TIME_TO);
	low->setCallback(saveInputFieldSetting, low);
	high->setCallback(saveInputFieldSetting, high);
	low->setValue(g_data.getInt(VENT_TIME_FROM));
	high->setValue(g_data.getInt(VENT_TIME_TO));

	static Text temptxt;
	temptxt.setXYpos(PG_LEFT_PADD, 195);
	temptxt.setText(S3_TEMP);

	static CheckBox tempCheck;
	tempCheck.setXYpos(PG_LEFT_PADD, 210);
	tempCheck.setText(EMPTY_STR);
	tempCheck.setSettingsId(VENT_TEMP_LIM);
	tempCheck.setCallback(saveCheckBoxSetting, &tempCheck);
	tempCheck.on(g_data.getInt(VENT_TEMP_LIM));

	static InputField temp;
	temp.setXYpos(45, 210);
	temp.setText(MORE_THAN);
	temp.setSettingsId(VENT_TEMP_THRES);
	temp.setValue(g_data.getInt(VENT_TEMP_THRES));
	temp.setCallback(saveInputFieldSetting, &temp);

	static Text humtxt;
	humtxt.setXYpos(PG_LEFT_PADD, 242);
	humtxt.setText(S3_HUM);

	static CheckBox humCheck;
	humCheck.setXYpos(PG_LEFT_PADD, 261);
	humCheck.setText(EMPTY_STR);
	humCheck.setSettingsId(VENT_HUM_LIM);
	humCheck.setCallback(saveCheckBoxSetting, &humCheck);
	humCheck.on(g_data.getInt(VENT_HUM_LIM));

	static InputField hum;
	hum.setXYpos(45, 261);
	hum.setText(MORE_THAN);
	hum.setSettingsId(VENT_HUM_THRES);
	hum.setValue(g_data.getInt(VENT_HUM_THRES));
	hum.setCallback(saveInputFieldSetting, &hum);

	stage3.addItem(&ventCheck);
	stage3.addItem(&fanImg);
	stage3.addItem(&par1);
	stage3.addItem(&subTitle);
	stage3.addItem(&timeint);
	stage3.addItem(&timeCheck);

	stage3.addItem((ScrObj*)timeLimit.getLowerPtr());
	stage3.addItem(timeLimit.getDashPtr());
	stage3.addItem((ScrObj*)timeLimit.getHigherPtr());

	stage3.addItem(&temptxt);
	stage3.addItem(&tempCheck);
	stage3.addItem(&temp);
	stage3.addItem(&humtxt);
	stage3.addItem(&humCheck);
	stage3.addItem(&hum);
	stage3.addItem(&forward);

	if (!ventCheck.isOn()) {
		stage3.setInvisible();
		stage3.restock();
	}

	return &stage3;
}

Page* buildStage4()
{
	static Page stage4;
	stage4.setTitle(S4_TITLE);
	stage4.setNext(pages[STAGE5_PG]);

	static CheckBox passVent;
	passVent.setXYpos(145, 37);
	passVent.setText(S4_PASSVENT);
	passVent.setAlign(LEFT);
	passVent.setFont(BOLDFONT);
	passVent.setSettingsId(PASSVENT);
	passVent.on(g_data.getInt(PASSVENT));
	passVent.setCallback(checkBoxCallback, &passVent, &stage4);
	passVent.neverHide();

	static Image doorImg;
	doorImg.setXYpos(186, 37);
	doorImg.loadRes(images[IMG_DOOR]);
	doorImg.neverHide();

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 67);
	par1.setText(S4_PAR1);
	par1.neverHide();

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 117);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S4_SUBTTL);

	static Text timeint;
	timeint.setXYpos(PG_LEFT_PADD, 142);
	timeint.setText(S4_TIME);

	static CheckBox timeCheck;
	timeCheck.setXYpos(PG_LEFT_PADD, 163);
	timeCheck.setText(EMPTY_STR);
	timeCheck.setSettingsId(PASSVENT_TIME_LIM);
	timeCheck.setCallback(saveCheckBoxSetting, &timeCheck);
	timeCheck.on(g_data.getInt(PASSVENT_TIME_LIM));

	static HourLimits timeLimit(45, 163);
	InputField* low = (InputField*) timeLimit.getLowerPtr();
	InputField* high = (InputField*) timeLimit.getHigherPtr();
	low->setSettingsId(PASSVENT_TIME_FROM);
	high->setSettingsId(PASSVENT_TIME_TO);
	low->setCallback(saveInputFieldSetting, low);
	high->setCallback(saveInputFieldSetting, high);
	low->setValue(g_data.getInt(PASSVENT_TIME_FROM));
	high->setValue(g_data.getInt(PASSVENT_TIME_TO));

	static Text temptxt;
	temptxt.setXYpos(PG_LEFT_PADD, 195);
	temptxt.setText(S4_TEMP);

	static CheckBox tempCheck;
	tempCheck.setXYpos(PG_LEFT_PADD, 210);
	tempCheck.setText(EMPTY_STR);
	tempCheck.setSettingsId(PASSVENT_TEMP_LIM);
	tempCheck.setCallback(saveCheckBoxSetting, &tempCheck);
	tempCheck.on(g_data.getInt(PASSVENT_TEMP_LIM));

	static InputField temp;
	temp.setXYpos(45, 210);
	temp.setText(MORE_THAN);
	temp.setWidth(FOUR_CHR);
	temp.setStr("°C");
	temp.setSettingsId(PASSVENT_TEMP_THRES);
	temp.setValue(g_data.getInt(PASSVENT_TEMP_THRES));
	temp.setCallback(saveInputFieldSetting, &temp);

	static Text humtxt;
	humtxt.setXYpos(PG_LEFT_PADD, 242);
	humtxt.setText(S4_HUM);

	static CheckBox humCheck;
	humCheck.setXYpos(PG_LEFT_PADD, 261);
	humCheck.setText(EMPTY_STR);
	humCheck.setSettingsId(PASSVENT_HUM_LIM);
	humCheck.setCallback(saveCheckBoxSetting, &humCheck);
	humCheck.on(g_data.getInt(PASSVENT_HUM_LIM));

	//std::unique_ptr<InputField> hum(new InputField());
	static InputField hum;
	hum.setXYpos(45, 261);
	hum.setText(MORE_THAN);
	hum.setStr("%");
	hum.setSettingsId(PASSVENT_HUM_THRES);
	hum.setValue(g_data.getInt(PASSVENT_HUM_THRES));
	hum.setCallback(saveInputFieldSetting, &hum);

	//stage4.addItemPtr(std::move(hum));

	stage4.addItem(&passVent);
	stage4.addItem(&doorImg);
	stage4.addItem(&par1);
	stage4.addItem(&subTitle);
	stage4.addItem(&timeint);
	stage4.addItem(&timeCheck);

	stage4.addItem((ScrObj*)timeLimit.getLowerPtr());
	stage4.addItem(timeLimit.getDashPtr());
	stage4.addItem((ScrObj*)timeLimit.getHigherPtr());

	stage4.addItem(&temptxt);
	stage4.addItem(&tempCheck);
	stage4.addItem(&temp);

	stage4.addItem(&humtxt);
	stage4.addItem(&humCheck);
	stage4.addItem(&hum);

	stage4.addItem(&forward);

	if (!passVent.isOn()) {
		stage4.setInvisible();
		stage4.restock();
	}

	return &stage4;
}

Page* buildStage5()
{
	int bulletsX = 23;
	int daysX = 41;

	static Page stage5;
	stage5.setTitle(S5_TITLE);
	stage5.setNext(pages[STAGE6_PG]);

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 36);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S5_SUBTTL);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 57);
	par1.setText(S5_PAR1);

	static Text first;
	first.setXYpos(bulletsX, 107);
	first.setFont(BOLDFONT);
	first.setText(BULL_1);
	static DayLimits firstStageLimits(daysX, 103);

	InputField* limit = firstStageLimits.getInputFieldPtr();
	limit->setSettingsId(GR_CYCL_1_DAYS);
	limit->setValue(g_data.getInt(GR_CYCL_1_DAYS));
	limit->setCallback(inputsCallback, limit);

	static Text second;
	second.setXYpos(bulletsX, 142);
	second.setText(BULL_2);
	second.setFont(BOLDFONT);
	static DayLimits secondStageLimilts(daysX, 138);
	OutputField* lower = secondStageLimilts.getOutputFieldPtr();
	daysOut[SECOND_OUT] = lower;
	lower->setValue(limit->getValue());

	limit = secondStageLimilts.getInputFieldPtr();
	limit->setSettingsId(GR_CYCL_2_DAYS);
	limit->setValue(g_data.getInt(GR_CYCL_2_DAYS));
	limit->setCallback(inputsCallback, limit);


	static Text third;
	third.setXYpos(bulletsX, 178);
	third.setText(BULL_3);
	third.setFont(BOLDFONT);
	static DayLimits thirdStageLimits(daysX, 174);
	lower = thirdStageLimits.getOutputFieldPtr();
	daysOut[THIRD_OUT] = lower;
	lower->setValue(limit->getValue());

	limit = thirdStageLimits.getInputFieldPtr();
	limit->setSettingsId(GR_CYCL_3_DAYS);
	limit->setValue(g_data.getInt(GR_CYCL_3_DAYS));
	limit->setCallback(inputsCallback, limit);

	static Image sprouts;
	sprouts.setXYpos(38, 218);
	sprouts.loadRes(images[IMG_SPROUTS]);

	stage5.addItem(&subTitle);
	stage5.addItem(&par1);
	stage5.addItem(&first);
	stage5.addItem(firstStageLimits.getLowerPtr());
	stage5.addItem(firstStageLimits.getDashPtr());
	stage5.addItem(firstStageLimits.getHigherPtr());
	stage5.addItem(&second);
	stage5.addItem(secondStageLimilts.getLowerPtr());
	stage5.addItem(secondStageLimilts.getDashPtr());
	stage5.addItem(secondStageLimilts.getHigherPtr());
	stage5.addItem(&third);
	stage5.addItem(thirdStageLimits.getLowerPtr());
	stage5.addItem(thirdStageLimits.getDashPtr());
	stage5.addItem(thirdStageLimits.getHigherPtr());
	stage5.addItem(&sprouts);
	stage5.addItem(&forward);

	return &stage5;
}

String stage1str;
String stage2str;
String stage3str;

void callStage6(void* arg)
{
	stage1str = (String)"0" + " - "
		+ g_data.getInt(GR_CYCL_1_DAYS) + " "
		+ scrStrings[TXT_DAY];
	stage2str = (String)g_data.getInt(GR_CYCL_1_DAYS)
		+ " - " + g_data.getInt(GR_CYCL_2_DAYS) + " "
		+ scrStrings[TXT_DAY];
	stage3str = (String)g_data.getInt(GR_CYCL_2_DAYS) + " - "
		+ g_data.getInt(GR_CYCL_3_DAYS) + " "
		+ scrStrings[TXT_DAY];

	callPage(pages[STAGE6_PG]);
}

Page* buildStage6()
{
	static Page stage6;
	stage6.setTitle(S6_TITLE);
	stage6.setNext(pages[STAGE7_PG]);

	static CheckBox concEc;
	concEc.setXYpos(170, MB_Y_START);
	concEc.setAlign(LEFT);
	concEc.setFont(BOLDFONT);
	concEc.setText(S6_SUBTTL);
	concEc.setSettingsId(EC_ON);
	concEc.setCallback(checkBoxCallback, &concEc, &stage6);
	concEc.on(g_data.getInt(EC_ON));
	concEc.neverHide();
	stage6.addItem(&concEc);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 64);
	par1.setText(S6_PAR1);
	par1.neverHide();
	stage6.addItem(&par1);

	static Line line1(210);
	static Line line2(210);
	static Line line3(210);
	line1.setXYpos(PG_LEFT_PADD, 136);
	line2.setXYpos(PG_LEFT_PADD, 198);
	line3.setXYpos(PG_LEFT_PADD, 259);

	static Text bull1;
	static Text bull2;
	static Text bull3;
	bull1.setXYpos(PG_LEFT_PADD, 88);
	bull1.setText(BULL_1);
	bull2.setXYpos(PG_LEFT_PADD, 149);
	bull2.setText(BULL_2);
	bull3.setXYpos(PG_LEFT_PADD, 210);
	bull3.setText(BULL_3);
	bull1.setFont(BOLDFONT);
	bull2.setFont(BOLDFONT);
	bull3.setFont(BOLDFONT);

	static InputField in1;
	static InputField in2;
	static InputField in3;
	in1.setXYpos(125, 80);
	in1.setFloat();
	in1.setSettingsId(EC_CYCL1);
	in1.setValue(g_data.getFloat(in1.getSettingsId()));
	in1.setCallback(saveInputFieldSetting, &in1);
	in1.setText(TXT_EC);
	in2.setXYpos(125, 142);
	in2.setFloat();
	in2.setSettingsId(EC_CYCL2);
	in2.setValue(g_data.getFloat(in2.getSettingsId()));
	in2.setCallback(saveInputFieldSetting, &in2);
	in2.setText(TXT_EC);
	in3.setXYpos(125, 202);
	in3.setFloat();
	in3.setSettingsId(EC_CYCL3);
	in3.setValue(g_data.getFloat(in3.getSettingsId()));
	in3.setCallback(saveInputFieldSetting, &in3);
	in3.setText(TXT_EC);
	in1.setTDS();
	in2.setTDS();
	in3.setTDS();

	/*
	static String stage1str;
	static String stage2str;
	static String stage3str;
	*/

	stage1str = (String)"0" + " - "
		+ g_data.getInt(GR_CYCL_1_DAYS) + " "
		+ scrStrings[TXT_DAY];
	stage2str = (String)g_data.getInt(GR_CYCL_1_DAYS)
		+ " - " + g_data.getInt(GR_CYCL_2_DAYS) + " "
		+ scrStrings[TXT_DAY];
	stage3str = (String)g_data.getInt(GR_CYCL_2_DAYS) + " - "
		+ g_data.getInt(GR_CYCL_3_DAYS) + " "
		+ scrStrings[TXT_DAY];

	int strOffset = 38;
	static StringText str1;
	static StringText str2;
	static StringText str3;
	str1.setXYpos(strOffset, 88);
	str2.setXYpos(strOffset, 150);
	str3.setXYpos(strOffset, 210);
	str1.setText(stage1str);
	str2.setText(stage2str);
	str3.setText(stage3str);

	static InputField ifA1;
	static InputField ifB1;
	static InputField ifC1;
	ifA1.setXYpos(30, 108);
	ifB1.setXYpos(106, 108);
	ifC1.setXYpos(178, 108);

	// TODO: make float input field class...
	ifA1.setFloat();
	ifB1.setFloat();
	ifC1.setFloat();

	ifA1.setMult();
	ifB1.setMult();
	ifC1.setMult();

	ifA1.setSettingsId(EC_A1);
	ifB1.setSettingsId(EC_B1);
	ifC1.setSettingsId(EC_C1);
	ifA1.setValue(g_data.getFloat(ifA1.getSettingsId()));
	ifB1.setValue(g_data.getFloat(ifB1.getSettingsId()));
	ifC1.setValue(g_data.getFloat(ifC1.getSettingsId()));
	ifA1.setCallback(saveInputFieldSetting, &ifA1);
	ifB1.setCallback(saveInputFieldSetting, &ifB1);
	ifC1.setCallback(saveInputFieldSetting, &ifC1);

	ifA1.setAlign(LEFT);
	ifB1.setAlign(LEFT);
	ifC1.setAlign(LEFT);
	ifA1.setFont(MIDFONT);
	ifB1.setFont(MIDFONT);
	ifC1.setFont(MIDFONT);
	ifA1.setText(TXT_A);
	ifB1.setText(TXT_B);
	ifC1.setText(TXT_C);

	static InputField ifA2;
	static InputField ifB2;
	static InputField ifC2;
	ifA2.setXYpos(30, 170);
	ifB2.setXYpos(106, 170);
	ifC2.setXYpos(178, 170);

	ifA2.setFloat();
	ifB2.setFloat();
	ifC2.setFloat();

	ifA2.setMult();
	ifB2.setMult();
	ifC2.setMult();

	ifA2.setSettingsId(EC_A2);
	ifB2.setSettingsId(EC_B2);
	ifC2.setSettingsId(EC_C2);
	ifA2.setValue(g_data.getFloat(ifA2.getSettingsId()));
	ifB2.setValue(g_data.getFloat(ifB2.getSettingsId()));
	ifC2.setValue(g_data.getFloat(ifC2.getSettingsId()));
	ifA2.setCallback(saveInputFieldSetting, &ifA2);
	ifB2.setCallback(saveInputFieldSetting, &ifB2);
	ifC2.setCallback(saveInputFieldSetting, &ifC2);

	ifA2.setAlign(LEFT);
	ifB2.setAlign(LEFT);
	ifC2.setAlign(LEFT);
	ifA2.setFont(MIDFONT);
	ifB2.setFont(MIDFONT);
	ifC2.setFont(MIDFONT);
	ifA2.setText(TXT_A);
	ifB2.setText(TXT_B);
	ifC2.setText(TXT_C);

	static InputField ifA3;
	static InputField ifB3;
	static InputField ifC3;
	ifA3.setXYpos(30, 230);
	ifB3.setXYpos(106, 230);
	ifC3.setXYpos(178, 230);

	ifA3.setFloat();
	ifB3.setFloat();
	ifC3.setFloat();

	ifA3.setMult();
	ifB3.setMult();
	ifC3.setMult();

	ifA3.setSettingsId(EC_A3);
	ifB3.setSettingsId(EC_B3);
	ifC3.setSettingsId(EC_C3);
	ifA3.setValue(g_data.getFloat(ifA3.getSettingsId()));
	ifB3.setValue(g_data.getFloat(ifB3.getSettingsId()));
	ifC3.setValue(g_data.getFloat(ifC3.getSettingsId()));
	ifA3.setCallback(saveInputFieldSetting, &ifA3);
	ifB3.setCallback(saveInputFieldSetting, &ifB3);
	ifC3.setCallback(saveInputFieldSetting, &ifC3);

	ifA3.setAlign(LEFT);
	ifB3.setAlign(LEFT);
	ifC3.setAlign(LEFT);
	ifA3.setFont(MIDFONT);
	ifB3.setFont(MIDFONT);
	ifC3.setFont(MIDFONT);
	ifA3.setText(TXT_A);
	ifB3.setText(TXT_B);
	ifC3.setText(TXT_C);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 266);
	par2.setText(S6_PAR2);

	static InputField pumptime;
	pumptime.setXYpos(74, 279);
	pumptime.setText(TXT_SEC);
	pumptime.setSettingsId(EC_PUMPS);
	pumptime.setValue(g_data.getInt(EC_PUMPS));
	pumptime.setCallback(saveInputFieldSetting, &pumptime);

	stage6.addItem(&line1);
	stage6.addItem(&line2);
	stage6.addItem(&line3);

	stage6.addItem(&bull1);
	stage6.addItem(&bull2);
	stage6.addItem(&bull3);

	stage6.addItem(&str1);
	stage6.addItem(&str2);
	stage6.addItem(&str3);

	// selectables
	stage6.addItem(&in1);

	stage6.addItem(&ifA1);
	stage6.addItem(&ifB1);
	stage6.addItem(&ifC1);

	stage6.addItem(&in2);

	stage6.addItem(&ifA2);
	stage6.addItem(&ifB2);
	stage6.addItem(&ifC2);

	stage6.addItem(&in3);

	stage6.addItem(&ifA3);
	stage6.addItem(&ifB3);
	stage6.addItem(&ifC3);

	stage6.addItem(&par2);
	stage6.addItem(&pumptime);
	stage6.addItem(&forward);

	if (!concEc.isOn()) {
		stage6.setInvisible();
		stage6.restock();
	}

	return &stage6;
}

Page* buildStage7()
{
	static Page stage7;
	stage7.setTitle(S7_TITLE);
	stage7.setNext(pages[STAGE8_PG]);

	static CheckBox acid;
	acid.setXYpos(165, 36);
	acid.setFont(BOLDFONT);
	acid.setAlign(LEFT);
	acid.setText(S7_SUBTTL);
	acid.setSettingsId(ACID_ON);
	acid.on(g_data.getInt(ACID_ON));
	acid.setCallback(checkBoxCallback, &acid, &stage7);
	acid.neverHide();

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 64);
	par1.setText(S7_PAR1);
	par1.neverHide();

	static Text bull1;
	static Text bull2;
	static Text bull3;
	bull1.setXYpos(PG_LEFT_PADD, 107);
	bull2.setXYpos(PG_LEFT_PADD, 138);
	bull3.setXYpos(PG_LEFT_PADD, 169);
	bull1.setText(BULL_1);
	bull2.setText(BULL_2);
	bull3.setText(BULL_3);

	int strOffset = 38;
	static StringText str1;
	str1.setXYpos(strOffset, 109);
	str1.setText(stage1str);
	static StringText str2;
	str2.setXYpos(strOffset, 140);
	str2.setText(stage2str);
	static StringText str3;
	str3.setXYpos(strOffset, 170);
	str3.setText(stage3str);

	static InputField in1;
	static InputField in2;
	static InputField in3;
	in1.setXYpos(126, 100);
	in2.setXYpos(126, 131);
	in3.setXYpos(126, 162);
	in1.setText(TXT_PH);
	in2.setText(TXT_PH);
	in3.setText(TXT_PH);
	in1.setFloat();
	in2.setFloat();
	in3.setFloat();
	in1.setPh();
	in2.setPh();
	in3.setPh();
	in1.setSettingsId(ACID_1);
	in2.setSettingsId(ACID_2);
	in3.setSettingsId(ACID_3);
	in1.setValue(g_data.getFloat(in1.getSettingsId()));
	in2.setValue(g_data.getFloat(in2.getSettingsId()));
	in3.setValue(g_data.getFloat(in3.getSettingsId()));
	in1.setCallback(saveInputFieldSetting, &in1);
	in2.setCallback(saveInputFieldSetting, &in2);
	in3.setCallback(saveInputFieldSetting, &in3);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 202);
	par2.setText(S6_PAR2);

	static InputField pumptime;
	pumptime.setXYpos(74, 216);
	pumptime.setText(TXT_SEC);
	pumptime.setSettingsId(ACID_PUMPS);
	pumptime.setValue(g_data.getInt(ACID_PUMPS));
	pumptime.setCallback(saveInputFieldSetting, &pumptime);

	stage7.addItem(&acid);
	stage7.addItem(&par1);
	stage7.addItem(&bull1);
	stage7.addItem(&bull2);
	stage7.addItem(&bull3);
	stage7.addItem(&str1);
	stage7.addItem(&str2);
	stage7.addItem(&str3);
	stage7.addItem(&in1);
	stage7.addItem(&in2);
	stage7.addItem(&in3);
	stage7.addItem(&par2);
	stage7.addItem(&pumptime);

	stage7.addItem(&forward);

	if (!acid.isOn()) {
		stage7.setInvisible();
		stage7.restock();
	}

	return &stage7;
}

void tmpCheckBoxCallback(void* check)
{
	if (check == nullptr)
		return;

	CheckBox* ch = (CheckBox*) check;
	ch->on(!ch->isOn());
	ch->invalidate();
	ch->prepare();
}

Page* buildStage8()
{
	static Page stage8;
	stage8.setTitle(S8_TITLE);
	stage8.setNext(pages[STAGE9_PG]);

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 38);
	subTitle.setAlign(LEFT);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S8_SUBTTL);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 80);
	par1.setText(S8_PAR1);

	static CheckBox pumps;
	pumps.setXYpos(PG_LEFT_PADD, 162);
	pumps.setText(S8_CHECK);
	pumps.setSettingsId(PUMP_OFF);
	pumps.on(g_data.getInt(PUMP_OFF));
	pumps.setCallback(saveCheckBoxSetting, &pumps);

	static Text za;
	za.setXYpos(PG_LEFT_PADD, 200);
	za.setText(S8_INTXT);

	static InputField seconds;
	seconds.setXYpos(36, 194);
	seconds.setText(S8_INPUT);
	seconds.setSettingsId(PUMP_SEC);
	seconds.setValue(g_data.getInt(PUMP_SEC));
	seconds.setCallback(saveInputFieldSetting, &seconds);

	stage8.addItem(&subTitle);
	stage8.addItem(&par1);
	stage8.addItem(&pumps);
	stage8.addItem(&za);
	stage8.addItem(&seconds);

	stage8.addItem(&forward);

	return &stage8;
}

Page* buildStage9()
{
	static Page stage9;
	stage9.setTitle(S9_TITLE);
	stage9.setLastStage();

	static CheckBox aeration;
	aeration.setXYpos(164, 33);
	aeration.setAlign(LEFT);
	aeration.setFont(BOLDFONT);
	//aeration.adjustTextX(5);
	aeration.setText(S9_SUBTTL);
	aeration.setSettingsId(AERO_ON);
	aeration.on(g_data.getInt(AERO_ON));
	aeration.setCallback(checkBoxCallback, &aeration, &stage9);
	aeration.neverHide();

	static Image bubbles;
	bubbles.setXYpos(190, 33);
	bubbles.loadRes(images[IMG_AERO]);
	bubbles.neverHide();

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 64);
	par1.setText(S9_PAR1);
	par1.neverHide();

	static CheckBox pumps;
	pumps.setXYpos(PG_LEFT_PADD, 102);
	pumps.setText(S9_CHECK);
	pumps.setSettingsId(AERO_PUMP);
	pumps.on(g_data.getInt(AERO_PUMP));
	pumps.setCallback(saveCheckBoxSetting, &pumps);

	static Text za;
	za.setXYpos(PG_LEFT_PADD, 140);
	za.setText(S9_INTXT);

	static InputField seconds;
	seconds.setXYpos(36, 134);
	seconds.setText(S9_INPUT);
	seconds.setSettingsId(AERO_PUMP_SEC);
	seconds.setValue(g_data.getInt(AERO_PUMP_SEC));
	seconds.setCallback(saveInputFieldSetting, &seconds);

	stage9.addItem(&aeration);
	stage9.addItem(&bubbles);
	stage9.addItem(&par1);
	stage9.addItem(&pumps);
	stage9.addItem(&za);
	stage9.addItem(&seconds);

	stage9.addItem(&forward);

	if (!aeration.isOn()) {
		stage9.setInvisible();
		stage9.restock();
	}

	return &stage9;
}

Page* buildStage9_2()
{
	static Page stage9;
	stage9.setTitle(S92_TITLE);
	stage9.setLastStage();

	static Text stir;
	stir.setXYpos(PG_LEFT_PADD, MB_Y_START);
	stir.setFont(BOLDFONT);
	stir.setText(S92_SUBTTL);
	stir.neverHide();

	static CheckBox stirCh;
	stirCh.setXYpos(156, 43);
	stirCh.setText(EMPTY_STR);
	stirCh.setSettingsId(STIR_ON);
	stirCh.on(g_data.getInt(STIR_ON));
	stirCh.setCallback(checkBoxCallback, &stirCh, &stage9);
	stirCh.neverHide();

	static Image cycle;
	cycle.setXYpos(188, 38);
	cycle.loadRes(images[IMG_CYCL]);
	cycle.neverHide();

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 87);
	par1.setText(S92_PAR1);
	par1.neverHide();

	static CheckBox pumps;
	pumps.setXYpos(PG_LEFT_PADD, 148);
	pumps.setText(S9_CHECK);
	pumps.setSettingsId(STIR_PUMP);
	pumps.on(g_data.getInt(STIR_PUMP));
	pumps.setCallback(saveCheckBoxSetting, &pumps);

	static Text za;
	za.setXYpos(PG_LEFT_PADD, 186);
	za.setText(S9_INTXT);

	static InputField seconds;
	seconds.setXYpos(36, 180);
	seconds.setText(S9_INPUT);
	seconds.setSettingsId(STIR_PUMP_SEC);
	seconds.setValue(g_data.getInt(STIR_PUMP_SEC));
	seconds.setCallback(saveInputFieldSetting, &seconds);

	stage9.addItem(&stir);
	stage9.addItem(&stirCh);
	stage9.addItem(&cycle);
	stage9.addItem(&par1);
	stage9.addItem(&pumps);
	stage9.addItem(&za);
	stage9.addItem(&seconds);

	stage9.addItem(&forward);

	if (!stirCh.isOn()) {
		stage9.setInvisible();
		stage9.restock();
	}

	return &stage9;
}

Page* buildStage8_2()
{
	static Page stage8;
	stage8.setTitle(S82_TITLE);
	stage8.setLastStage();

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 38);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S82_SUBTTL);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 88);
	par1.setText(S82_PAR1);

	static InputField fminutes;
	fminutes.setXYpos(PG_LEFT_PADD, 127);
	fminutes.setWidth(TWO_CHR);
	fminutes.setText(TXT_MINUTES);
	fminutes.setSettingsId(FLOOD_HOURS);
	fminutes.setValue(g_data.getInt(FLOOD_HOURS));
	fminutes.setCallback(saveInputFieldSetting, &fminutes);

	static InputField fseconds;
	fseconds.setXYpos(97, 127);
	fseconds.setWidth(TWO_CHR);
	fseconds.setText(TXT_SECONDS);
	fseconds.setSettingsId(FLOOD_MIN);
	fseconds.setValue(g_data.getInt(FLOOD_MIN));
	fseconds.setCallback(saveInputFieldSetting, &fseconds);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 174);
	par2.setText(S82_PAR2);

	static InputField hminutes;
	hminutes.setXYpos(PG_LEFT_PADD, 212);
	hminutes.setWidth(TWO_CHR);
	hminutes.setText(TXT_MINUTES);
	hminutes.setSettingsId(FLOOD_HOLD_MIN);
	hminutes.setValue(g_data.getInt(FLOOD_HOLD_MIN));
	hminutes.setCallback(saveInputFieldSetting, &hminutes);

	static InputField hseconds;
	hseconds.setXYpos(97, 212);
	hseconds.setText(TXT_SECONDS);
	hseconds.setWidth(TWO_CHR);
	hseconds.setSettingsId(FLOOD_HOLD_SEC);
	hseconds.setValue(g_data.getInt(FLOOD_HOLD_SEC));
	hseconds.setCallback(saveInputFieldSetting, &hseconds);

	stage8.addItem(&subTitle);
	stage8.addItem(&par1);
	stage8.addItem(&fminutes);
	stage8.addItem(&fseconds);
	stage8.addItem(&par2);
	stage8.addItem(&hminutes);
	stage8.addItem(&hseconds);

	stage8.addItem(&forward);

	return &stage8;
}


enum {
	RB_CONST,
	RB_CYCL,
	RB_NBUTTONS
};

RadioButton* spray_type[RB_NBUTTONS];

void sprayRadioCallback(void* btn)
{
	if (btn == nullptr)
		return;

	for (auto& i:spray_type)
		i->on(false);

	RadioButton* button = (RadioButton*) btn;
	button->on(true);
	for (auto& i:spray_type)
		g_data.set(i->getSettingsId(), i->isOn());
}

Page* buildStage8_3()
{
	static Page stage8;
	stage8.setTitle(S83_TITLE);

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 38);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S83_SUBTTL);

	static CheckBox pumps;
	pumps.setXYpos(PG_LEFT_PADD, 59);
	pumps.setText(S83_CHECK);
	pumps.setSettingsId(SPRAY_PUMP);
	pumps.on(g_data.getInt(SPRAY_PUMP));
	pumps.setCallback(saveCheckBoxSetting, &pumps);

	static Text za;
	za.setXYpos(PG_LEFT_PADD, 96);
	za.setText(S83_INTXT);

	static InputField seconds;
	seconds.setXYpos(36, 91);
	seconds.setText(S83_INPUT);
	seconds.setSettingsId(SPRAY_PUMP_SEC);
	seconds.setValue(g_data.getInt(SPRAY_PUMP_SEC));
	seconds.setCallback(saveInputFieldSetting, &seconds);

	static Text subTitle2;
	subTitle2.setXYpos(PG_LEFT_PADD, 131);
	subTitle2.setFont(BOLDFONT);
	subTitle2.setText(S83_MODE);

	static RadioButton cons;
	spray_type[RB_CONST] = &cons;
	cons.setXYpos(PG_LEFT_PADD, 153);
	cons.setText(S83_CONST);
	cons.setSettingsId(SPRAY_CONS);
	cons.on(g_data.getInt(SPRAY_CONS));
	cons.setCallback(sprayRadioCallback, &cons);

	static RadioButton cycl;
	spray_type[RB_CYCL] = &cycl;
	cycl.setXYpos(119, 153);
	cycl.setText(S83_CYCL);
	cycl.setSettingsId(SPRAY_CYCL);
	cycl.on(g_data.getInt(SPRAY_CYCL));
	cycl.setCallback(sprayRadioCallback, &cycl);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 184);
	par1.setText(S83_DUR);

	static InputField dminutes;
	dminutes.setXYpos(PG_LEFT_PADD, 204);
	dminutes.setWidth(TWO_CHR);
	dminutes.setText(TXT_MINUTES);
	dminutes.setSettingsId(SPRAY_MIN);
	dminutes.setValue(g_data.getInt(SPRAY_MIN));
	dminutes.setCallback(saveInputFieldSetting, &dminutes);

	static InputField dseconds;
	dseconds.setXYpos(97, 204);
	dseconds.setWidth(TWO_CHR);
	dseconds.setText(TXT_SECONDS);
	dseconds.setSettingsId(SPRAY_SEC);
	dseconds.setValue(g_data.getInt(SPRAY_SEC));
	dseconds.setCallback(saveInputFieldSetting, &dseconds);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 238);
	par2.setText(S83_EVR);

	static InputField cminutes;
	cminutes.setXYpos(PG_LEFT_PADD, 260);
	cminutes.setWidth(TWO_CHR);
	cminutes.setText(TXT_MINUTES);
	cminutes.setSettingsId(SPRAY_CYCL_MIN);
	cminutes.setValue(g_data.getInt(SPRAY_CYCL_MIN));
	cminutes.setCallback(saveInputFieldSetting, &cminutes);

	static InputField cseconds;
	cseconds.setXYpos(97, 260);
	cseconds.setText(TXT_SECONDS);
	cseconds.setWidth(TWO_CHR);
	cseconds.setSettingsId(SPRAY_CYCL_SEC);
	cseconds.setValue(g_data.getInt(SPRAY_CYCL_SEC));
	cseconds.setCallback(saveInputFieldSetting, &cseconds);

	stage8.addItem(&subTitle);
	stage8.addItem(&pumps);
	stage8.addItem(&za);
	stage8.addItem(&seconds);
	stage8.addItem(&subTitle2);
	stage8.addItem(&cons);
	stage8.addItem(&cycl);
	stage8.addItem(&par1);
	stage8.addItem(&dminutes);
	stage8.addItem(&dseconds);
	stage8.addItem(&par2);
	stage8.addItem(&cminutes);
	stage8.addItem(&cseconds);

	stage8.addItem(&forward);

	return &stage8;
}

RadioButton* drip_type[RB_NBUTTONS];

void dripRadioCallback(void* btn)
{
	if (btn == nullptr)
		return;

	for (auto& i:drip_type)
		i->on(false);

	RadioButton* button = (RadioButton*) btn;
	button->on(true);

	for (auto& i:drip_type)
		g_data.set(i->getSettingsId(), i->isOn());
}

Page* buildStage8_4()
{
	static Page stage8;
	stage8.setTitle(S84_TITLE);

	static Text subTitle;
	subTitle.setXYpos(PG_LEFT_PADD, 38);
	subTitle.setFont(BOLDFONT);
	subTitle.setText(S84_SUBTTL);

	static CheckBox pumps;
	pumps.setXYpos(PG_LEFT_PADD, 59);
	pumps.setText(S83_CHECK);
	pumps.setSettingsId(DRIP_PUMP);
	pumps.on(g_data.getInt(DRIP_PUMP));
	pumps.setCallback(saveCheckBoxSetting, &pumps);

	static Text za;
	za.setXYpos(PG_LEFT_PADD, 96);
	za.setText(S83_INTXT);

	static InputField seconds;
	seconds.setXYpos(36, 91);
	seconds.setText(S83_INPUT);
	seconds.setSettingsId(DRIP_PUMP_SEC);
	seconds.setValue(g_data.getInt(DRIP_PUMP_SEC));
	seconds.setCallback(saveInputFieldSetting, &seconds);

	static Text subTitle2;
	subTitle2.setXYpos(PG_LEFT_PADD, 131);
	subTitle2.setFont(BOLDFONT);
	subTitle2.setText(S84_MODE);

	static RadioButton cons;
	drip_type[RB_CONST] = &cons;
	cons.setXYpos(PG_LEFT_PADD, 153);
	cons.setText(S83_CONST);
	cons.setSettingsId(DRIP_CONS);
	cons.on(g_data.getInt(DRIP_CONS));
	cons.setCallback(dripRadioCallback, &cons);

	static RadioButton cycl;
	drip_type[RB_CYCL] = &cycl;
	cycl.setXYpos(119, 153);
	cycl.setText(S83_CYCL);
	cycl.setSettingsId(DRIP_CYCL);
	cycl.on(g_data.getInt(DRIP_CYCL));
	cycl.setCallback(dripRadioCallback, &cycl);

	static Text par1;
	par1.setXYpos(PG_LEFT_PADD, 184);
	par1.setText(S83_DUR);

	static InputField dminutes;
	dminutes.setXYpos(PG_LEFT_PADD, 204);
	dminutes.setWidth(TWO_CHR);
	dminutes.setText(TXT_MINUTES);
	dminutes.setSettingsId(DRIP_MIN);
	dminutes.setValue(g_data.getInt(DRIP_MIN));
	dminutes.setCallback(saveInputFieldSetting, &dminutes);

	static InputField dseconds;
	dseconds.setXYpos(97, 204);
	dseconds.setWidth(TWO_CHR);
	dseconds.setText(TXT_SECONDS);
	dseconds.setSettingsId(DRIP_SEC);
	dseconds.setValue(g_data.getInt(DRIP_SEC));
	dseconds.setCallback(saveInputFieldSetting, &dseconds);

	static Text par2;
	par2.setXYpos(PG_LEFT_PADD, 238);
	par2.setText(S83_EVR);

	static InputField cminutes;
	cminutes.setXYpos(PG_LEFT_PADD, 260);
	cminutes.setWidth(TWO_CHR);
	cminutes.setText(TXT_MINUTES);
	cminutes.setSettingsId(DRIP_CYCL_MIN);
	cminutes.setValue(g_data.getInt(DRIP_CYCL_MIN));
	cminutes.setCallback(saveInputFieldSetting, &cminutes);

	static InputField cseconds;
	cseconds.setXYpos(97, 260);
	cseconds.setWidth(TWO_CHR);
	cseconds.setText(TXT_SECONDS);
	cseconds.setSettingsId(DRIP_CYCL_SEC);
	cseconds.setValue(g_data.getInt(DRIP_CYCL_SEC));
	cseconds.setCallback(saveInputFieldSetting, &cseconds);

	stage8.addItem(&subTitle);
	stage8.addItem(&pumps);
	stage8.addItem(&za);
	stage8.addItem(&seconds);
	stage8.addItem(&subTitle2);
	stage8.addItem(&cons);
	stage8.addItem(&cycl);
	stage8.addItem(&par1);
	stage8.addItem(&dminutes);
	stage8.addItem(&dseconds);
	stage8.addItem(&par2);
	stage8.addItem(&cminutes);
	stage8.addItem(&cseconds);


	stage8.addItem(&forward);

	return &stage8;
}

#define FP_LEFT_PADDING 7
Page* buildMainPage()
{
	static Page mainPage;
	mainPage.setTitle(FP_TITLE);

	static BlueTextButton menu;
	menu.setXYpos(FP_LEFT_PADDING, 289);
	menu.setText(MENU);
	menu.setCallback(callPage, pages[MENU_PG]);

	mainPage.addItem(&menu);

	return &mainPage;
}

enum {
	BTN_92,
	BTN_82,
	BTN_83,
	BTN_84,
	LP_NBTNS
};

/*
Page* lastPagesList()
{
	static Page menuPage;

	static GreyTextButton menu_items[LP_NBTNS];

	// gap between items
	int gap = MENU_GAP;

	dispStrings_t menu1[LP_NBTNS];
	menu1[BTN_92] = S92_TITLE;
	menu1[BTN_82] = S82_TITLE;
	menu1[BTN_83] = S83_TITLE;
	menu1[BTN_84] = S84_TITLE;

	int j = 0;

	for (auto& i:menu_items) {
		i.setCallback(nop);
		i.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));
		i.setFont(SMALLFONT);
		i.setXYpos(PG_LEFT_PADD, MB_Y_START+(GREY_BUTTON_HEIGHT+gap)*j);
		i.setText(menu1[j]);
		j++;
	}

	// set callBacks
	menu_items[BTN_92].setCallback(callPage, pages[STAGE92_PG]);
	menu_items[BTN_82].setCallback(callPage, pages[STAGE82_PG]);
	menu_items[BTN_83].setCallback(callPage, pages[STAGE83_PG]);
	menu_items[BTN_84].setCallback(callPage, pages[STAGE84_PG]);

	// add all to page
	for (int i = 0; i < LP_NBTNS; i++) {
		menuPage.addItem(&menu_items[i]);
	}


	menuPage.setTitle(MENU);
	menuPage.addItem(&back);

	return &menuPage;
}
*/


void gSetBacklight(void* arg)
{
	analogWrite(LED_PIN, gBrightness.getValue());
}

#ifdef APP_DEBUG
#define STACK_CHECK_INTERVAL 10000
unsigned long oldMillis;
#endif

void buildAllPages()
{
	// stage8_4
	pages[STAGE84_PG] = buildStage8_4();

	// stage8_3
	pages[STAGE83_PG] = buildStage8_3();

	// stage8_2 (stage 2)
	pages[STAGE82_PG] = buildStage8_2();

	// stage9_2 (stage 2)
	pages[STAGE92_PG] = buildStage9_2();

	// stage9
	pages[STAGE9_PG] = buildStage9();

	// stage8
	pages[STAGE8_PG] = buildStage8();

	// stage7
	pages[STAGE7_PG] = buildStage7();

	// stage6
	pages[STAGE6_PG] = buildStage6();

	// stage 5
	pages[STAGE5_PG] = buildStage5();

	// stage 4
	pages[STAGE4_PG] = buildStage4();

	// stage 3
	pages[STAGE3_PG] = buildStage3();

	// stage 2
	pages[STAGE2_PG] = buildStage2();

	// stage 1
	pages[STAGE1_PG] = buildStage1();

	// first page
	pages[FIRST_PG] = buildFirstPage();

	// diag pages
	pages[PWR_DIAG_PG] = buildPwrDiag();
	pages[DIG_DIAG_PG] = buildDigDiag();
	pages[ADC_DIAG_PG] = buildADCdiag();
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

	// list of last stages
	//pages[LSTAGES] = lastPagesList();

	pages[MENU_PG] = buildMenuPage();
	pages[MAIN_PG] = buildMainPage();
}

void linkPages()
{
#ifdef APP_DEBUG
	Serial.println("pages pointers: ");
	for (auto i:pages)
		DEBUG_PRINT_HEX(i);
#endif

	// debug last stages
//	pages[LSTAGES]->setPrev(pages[MENU_PG]);

	// new planting first page
	pages[STAGE1_PG]->setPrev(pages[MAIN_PG]);
	pages[STAGE1_PG]->setNext(pages[STAGE2_PG]);

	// planting last page
	pages[STAGE9_PG]->setNext(pages[MAIN_PG]);
	pages[STAGE92_PG]->setNext(pages[MAIN_PG]);
	pages[STAGE82_PG]->setNext(pages[MAIN_PG]);
	pages[STAGE83_PG]->setNext(pages[MAIN_PG]);
	pages[STAGE84_PG]->setNext(pages[MAIN_PG]);

	// menu page prev
	pages[MENU_PG]->setPrev(pages[MAIN_PG]);

	// first page
	pages[FIRST_PG]->setPrev(pages[MENU_PG]);

	// util pages
	pages[SENS_FAIL_PG]->setPrev(pages[CAL_SETT_PG]);
	pages[CAL_PH1_PG]->setPrev(pages[CAL_SETT_PG]);
	pages[CAL_TDS1_PG]->setPrev(pages[CAL_SETT_PG]);

	// menu pages
	pages[SETT_PG]->setPrev(pages[MENU_PG]);
	pages[FONT_PG]->setPrev(pages[MENU_PG]);
	pages[TEST_PG]->setPrev(pages[MENU_PG]);

	// settings pages
	pages[WIFI_SETT_PG]->setPrev(pages[SETT_PG]);
	pages[WIFI_PG]->setPrev(pages[SETT_PG]);
	pages[TIME_PG]->setPrev(pages[SETT_PG]);
	pages[CAL_SETT_PG]->setPrev(pages[SETT_PG]);
	pages[LANG_PG]->setPrev(pages[SETT_PG]);

	// diag pages
	pages[DIAG_PG]->setPrev(pages[MENU_PG]);
	pages[SENS_DIAG_PG]->setPrev(pages[DIAG_PG]);
	pages[ADC_DIAG_PG]->setPrev(pages[DIAG_PG]);
	pages[PWR_DIAG_PG]->setPrev(pages[DIAG_PG]);
	pages[DIG_DIAG_PG]->setPrev(pages[DIAG_PG]);

	pages[TDS_DIAG_PG]->setPrev(pages[SENS_DIAG_PG]);
	pages[PH_DIAG_PG]->setPrev(pages[SENS_DIAG_PG]);

	// calibration pages done buttons
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

	g_data.load();

	// connect to wifi or create AP
	checkWifi();

	// init all stuff in App.h
	app.init();


	buildAllPages();

	topBar.build();

	// setPrev on required pages
	linkPages();

	// backlight
	gBrightness.onClick();

	if (g_first_launch) {
		currPage = pages[FIRST_PG];
	}
	else {
		currPage = pages[MAIN_PG];
		datetime.loadStartDay();
		g_rig.start();
	}

	currPage->setCurrItem(0);
	currItem = currPage->getCurrItem();
	currPage->prepare();

	// back button
	back.setCallback(callPage, currPage->prev());
	back.loadRes(images[IMG_PREV]);
	back.setXYpos(7, 284);
	back.setCircle();
	back.neverHide();

	// forward button
	forward.setCallback(callPage, currPage->next());
	forward.loadRes(images[IMG_NEXT]);
	forward.setXYpos(204, 284);
	forward.setCircle();
	forward.neverHide();

	//buildTopBar();

	currPage->draw();
	topBar.setText(currPage->getTitle());
	topBar.prepare();
	topBar.draw();

#ifdef APP_DEBUG
	oldMillis = millis();
#endif

	// init all expanders (TODO: init all i2c there)
	io.init();

	rtc.begin();
	datetime.init();
}

void deleteSettingsFile()
{
	if (SPIFFS.exists(sett_file)) {
		SPIFFS.remove(sett_file);
		Serial.println("settings file removed");
	}
}

void deleteDataFile()
{
	if (SPIFFS.exists(data_file)) {
		SPIFFS.remove(data_file);
		Serial.println("data file removed");
	}
}

unsigned long debMils = 0;
#define DEBUG_INT 1000

void loop()
{
	app.update();

	// read init command
	while (Serial.available() > 0) {
		String cmd = Serial.readStringUntil('\n');
		cmd.trim();

		if (cmd == "data") {
			g_data.print();
		}
		else if (cmd == "init") {
			deleteSettingsFile();
			deleteDataFile();
		}
		else if (cmd == "load") {
			g_data.load();
		}
		else if (cmd == "start") {
			g_rig.start();
		}
		else if (cmd == "stop") {
			g_rig.halt();
		}
	}

#ifdef APP_DEBUG
	if (millis() - debMils > DEBUG_INT) {
		Serial.print("Tem: ");
		Serial.println(io.getTem());
		Serial.print("Hum: ");
		Serial.println(io.getHum());
		Serial.println("********");
		//Serial.print("Hour: ");
		//Serial.println(datetime.getHour());
		//Serial.print("Day: ");
		//Serial.println(datetime.getDays());
		debMils = millis();
	}
#endif
	/*
#ifdef APP_DEBUG
	if (millis() - oldMillis > STACK_CHECK_INTERVAL) {
		uint16_t unused = uxTaskGetStackHighWaterMark(NULL);
		Serial.print("gui task unused stack: ");
		Serial.println(unused);
		Serial.println();
		oldMillis = millis();
	}
#endif
*/

}
