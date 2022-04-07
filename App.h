#ifndef __APP_H__
#define __APP_H__

#include <ctime>
#include <atomic>
#include "Gui.h"
#include "IO.h"

// hardware... stuff
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WebServer.h>

WebServer server(80);

/*
// time
#include <iarduino_RTC.h>
iarduino_RTC rtc(RTC_RX8025);

// ph meter
#include <iarduino_I2C_pH.h>
iarduino_I2C_pH ph_meter(0x0a);

// tds meter
#include <iarduino_I2C_TDS.h>
iarduino_I2C_TDS tds_meter(0x0b);
*/

/*
// buttons
#include <iarduino_PCA9555.h>
iarduino_PCA9555 buttons(0x20); //first expander
iarduino_PCA9555 second_expander(0x21);
*/

// ping
#include <ESP32Ping.h>


// utils
const char* REMOTE_HOST = "www.iocontrol.ru";

//WiFiClient client;

std::atomic<bool> g_rapid_blink;
std::atomic<bool> g_ping_success;
std::atomic<bool> g_wifi_set;

std::atomic<bool> g_ph_calib_4_done;
std::atomic<bool> g_ph_calib_9_done;

std::atomic<bool> g_tds_calib_500_done;
std::atomic<bool> g_tds_calib_1500_done;

std::atomic<bool> g_ph_diag_wait_done;
std::atomic<bool> g_tds_diag_wait_done;

enum {
	HOUR,
	MIN,
	DAY,
	MON,
	YEAR,
	N_DATETIME_VISIBLE
};

Page timePage;

#define RTC_CHECK_INTERVAL 60000
#define USER_INPUT_SETTLE 500

std::atomic<bool> g_sync_succ;
SemaphoreHandle_t xNTPmutex;
std::atomic<int8_t> g_utc;

void ping_task_callback(void* arg)
{
	for(;;) {
		if (g_wifi_set) {
			if (Ping.ping(REMOTE_HOST, 3)) {
				g_ping_success = true;
			}
			else {
				g_ping_success = false;
			}
		}
#ifdef APP_DEBUG
		uint16_t unused = uxTaskGetStackHighWaterMark(NULL);
		Serial.print("ping task unused stack: ");
		Serial.println(unused);
#endif
		sleep(10000);
	}
}

typedef enum {
	MAIN_PG,
	MENU_PG,
	SETT_PG,
	LANG_PG,
	FONT_PG,
	TEST_PG,
	WIFI_PG,
	WIFI_SETT_PG,
	TIME_PG,
	// common sensor settings
	CAL_SETT_PG,
	SENS_FAIL_PG,
	// ph
	CAL_PH1_PG,
	CAL_PH2_PG,
	CAL_PH3_PG,
	CAL_PH4_PG,
	CAL_PH5_PG,
	// tds
	CAL_TDS1_PG,
	CAL_TDS2_PG,
	CAL_TDS3_PG,
	CAL_TDS4_PG,
	CAL_TDS5_PG,
	// diag
	DIAG_PG,
	SENS_DIAG_PG,
	TDS_DIAG_PG,
	PH_DIAG_PG,
	ADC_DIAG_PG,
	DIG_DIAG_PG,
	PWR_DIAG_PG,
	NPAGES
} pages_t;

Page* pages[NPAGES];
Page* currPage;

InputField gBrightness;

// global text from wifiSettPage
Text gwsConnection;

// calib pages global items
Wait g_ph3wait;
BlueTextButton g_ph_next;
Wait g_ph5wait;
Text g_ph_succ;
BlueTextButton g_ph_done;

Wait g_tds3wait;
BlueTextButton g_tds_next;
Wait g_tds5wait;
Text g_tds_succ;
BlueTextButton g_tds_done;

// diag pages global items
Wait g_phWait;
Wait g_tdsWait;

OutputField g_ph_read;
OutputField g_tds_read;


#define WIFI_UPDATE_INTERVAL 5000
#define WIFI_IMG_X 213
#define NET_IMG_X 186

class Panel {
	public:
		void setText(dispStrings_t index)
		{
			_menuText.setText(index);
			_menuText.prepare();
		}

		void invalidateAll()
		{
			_topBox.invalidate();
			_menuText.invalidate();
			_statusWIFI.invalidate();
			_statusInternet.invalidate();
		}

		void prepare()
		{
		}

		void erase()
		{
			_menuText.erase();
		}

		void hideIcons()
		{
			_statusWIFI.setInvisible();
			_statusInternet.setInvisible();
		}

		void showIcons()
		{
			_statusWIFI.setVisible();
			_statusInternet.setVisible();
		}

		void draw()
		{
			_topBox.draw();
			_menuText.draw();

			_statusWIFI.reload();
			_statusWIFI.draw();
			_statusWIFI.freeRes();

			_statusInternet.reload();
			_statusInternet.draw();
			_statusInternet.freeRes();
		}

		void update()
		{
			if (!g_wifi_set) {
				server.handleClient();
			}

			if (millis() - _timestamp < _interval) {
				return;
			}

			_timestamp = millis();

			int dBm = WiFi.RSSI();
			uint8_t strength = map(dBm, -95, -45, 0, 4);

#ifdef APP_DEBUG
			Serial.print("WiFi strength: ");
			Serial.println(dBm);
			Serial.print("Wifi status: ");
			Serial.println(WiFi.status());
#endif

			strength = clamp(strength, 0, 4);

			/*
			if (WiFi.status() == WL_DISCONNECTED && gwsWifiChBox.isOn()) {
				WiFi.reconnect();
			}
			*/

			if (WiFi.status() != WL_CONNECTED) {
				_curWiFiImage = IMG_NO_WIFI;
				_curNetImage = IMG_NET_NO;
				g_ping_success = false;

				if (gwsWifiChBox.isOn()) {
					WiFi.reconnect();
				}
			}
			else {
				if (g_ping_success) {
					_curNetImage = IMG_NET_OK;

				}
				else {
					_curNetImage = IMG_NET_NO;
				}

				switch (strength) {
					case 0: _curWiFiImage = IMG_NO_WIFI; break;
					case 1: _curWiFiImage = IMG_WIFI1; break;
					case 2: _curWiFiImage = IMG_WIFI2; break;
					case 3: _curWiFiImage = IMG_WIFI3; break;
					case 4: _curWiFiImage = IMG_WIFI4; break;
					default: _curWiFiImage = IMG_NO_WIFI; break;
				}
			}

			// if connection status changed
			if (_curNetImage != _prevNetImage) {
				_statusInternet.loadRes(images[_curNetImage]);
				_statusInternet.draw();
				_statusInternet.freeRes();
				_prevNetImage = _curNetImage;
				_changed = true;

			}

			// if wifi status changed
			if (_curWiFiImage != _prevWiFiImage) {
				_statusWIFI.loadRes(images[_curWiFiImage]);
				_statusWIFI.draw();
				_statusWIFI.freeRes();
				_prevWiFiImage = _curWiFiImage;
			}
		}

		bool netChanged()
		{
			return _changed;
		}

		void resetChange()
		{
			_changed = false;
		}

		void build()
		{
			_topBox.setColor(greyscaleColor(TOP_BAR_BG_COL));
			_topBox.setWH(SCR_WIDTH, TOP_BAR_HEIGHT);
			_topBox.invalidate();
			_menuText.setFont(MIDFONT);
			_menuText.setXYpos(LEFTMOST, TOPMOST);
			_menuText.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(TOP_BAR_BG_COL));
			_menuText.setText(MENU);
			_menuText.prepare();

			_time.setFont(MIDFONT);
			_time.setXYpos(LEFTMOST, TOPMOST);
			_time.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(TOP_BAR_BG_COL));
			_time.setText(MENU); // special case
			_time.prepare();
//rtc.gettime("H:i")
			_statusWIFI.loadRes(images[_curWiFiImage]);
			_statusWIFI.setXYpos(WIFI_IMG_X, 0);
			//_statusWIFI.freeRes();

			_statusInternet.loadRes(images[_curNetImage]);
			_statusInternet.setXYpos(NET_IMG_X, 0);
			//_statusInternet.freeRes();
		}
	private:
		SimpleBox _topBox;
		Image _statusWIFI;
		Image _statusInternet;
		Text _menuText;
		Text _time;
		bool _changed = false;
		bool _wifiIsON = false;
	private:
		//bool _connected = false;
		unsigned long _timestamp = 0;
		unsigned long _interval = WIFI_UPDATE_INTERVAL;
		images_t _curWiFiImage = IMG_NO_WIFI;
		images_t _curNetImage = IMG_NET_NO;
		images_t _prevWiFiImage = IMG_NO_WIFI;
		images_t _prevNetImage = IMG_NET_NO;
} topBar;




void vSyncNTPtask(void* arg)
{
	if (arg == nullptr)
		vTaskDelete(NULL);

	xSemaphoreTake(xNTPmutex, portMAX_DELAY);

	struct tm* task_tm = (struct tm*) arg;
	//configTime(3600*_utc, 0, NTP_SERVER);
	configTime(3600*g_utc, 0, NTP_SERVER);
	if (!getLocalTime(task_tm)) {
		g_sync_succ = false;
#ifdef APP_DEBUG
		Serial.println("failed to sync ntp time");
#endif
	}
	else {
		g_sync_succ = true;
	}
	xSemaphoreGive(xNTPmutex);

	vTaskDelete(NULL);
}



class DateTime: public ScrObj {
	public:
		DateTime()
		{
			_y = 174;
		}
		~DateTime()
		{
			freeRes();
		}

		void init()
		{
			g_sync_succ = false;
			xNTPmutex = xSemaphoreCreateMutex();
			getI2Ctime();
			if (_sync) {
				syncNTP();
			}
		}

	private:
		//SemaphoreHandle_t xNTPmutex;
		InputField _visible[N_DATETIME_VISIBLE];
		Text _fieldsTitle;
		bool _sync = false;
		bool _userInputSettled = false;
		bool _timeSynced = false;
		//int8_t _utc = 0;
		struct tm  _timeinfo;
		unsigned long _oldMils = 0;
		const char* const _nptServer = NTP_SERVER;
		unsigned long _userInputTimestamp = 0;
		//uint16_t _y = 225;
		//TFT_eSprite _sprite = TFT_eSprite(&tft);
	public:

		void update()
		{
			if (
					millis()
					- _userInputTimestamp
					> USER_INPUT_SETTLE
					&& !_userInputSettled
					&& !_timeSynced
					) {
				_userInputSettled = true;
			}

			if (_userInputSettled && _sync) {
				syncNTP();
				prepare();
				invalidate();
				_userInputSettled = false;
				_timeSynced = true;
			}

			if (g_sync_succ) {
				//setI2Ctime();
				invalidate();
				prepare();
				g_sync_succ = false;
			}

			if (millis() - _oldMils > RTC_CHECK_INTERVAL) {
				_oldMils = millis();

				if (_sync) {
					syncNTP();
				}
				else {
					getI2Ctime();
				}

				//invalidate();
				//prepare();
			}
		}

		virtual void freeRes() override
		{
		}

		virtual void draw() override
		{
			// page methods deal with that
		}

		/*
		virtual void erase() override
		{
			for (auto& i:_visible) {
				i.erase();
			}
		}
		*/

		// GUI functions
		void setHours(void* obj)
		{
			//xSemaphoreTake(xNTPmutex, portMAX_DELAY);
			_timeinfo.tm_hour = _visible[HOUR].getValue();
			rtc.settimeUnix(mktime(&_timeinfo));
			//xSemaphoreGive(xNTPmutex);
		}

		void setMinutes(void* obj)
		{
			//xSemaphoreTake(xNTPmutex, portMAX_DELAY);
			_timeinfo.tm_min = _visible[MIN].getValue();
			rtc.settimeUnix(mktime(&_timeinfo));
			//xSemaphoreGive(xNTPmutex);
		}

		void setDay(void* obj)
		{
			//xSemaphoreTake(xNTPmutex, portMAX_DELAY);
			_timeinfo.tm_mday = _visible[DAY].getValue();
			rtc.settimeUnix(mktime(&_timeinfo));
			//xSemaphoreGive(xNTPmutex);
		}

		void setMon(void* obj)
		{
			//xSemaphoreTake(xNTPmutex, portMAX_DELAY);
			_timeinfo.tm_mon = _visible[MON].getValue() - 1;
			rtc.settimeUnix(mktime(&_timeinfo));
			//xSemaphoreGive(xNTPmutex);
		}

		void setYear(void* obj)
		{
			//xSemaphoreTake(xNTPmutex, portMAX_DELAY);
			_timeinfo.tm_year = _visible[YEAR].getValue() - 1900;
			rtc.settimeUnix(mktime(&_timeinfo));
			//xSemaphoreGive(xNTPmutex);
		}

		bool getSync()
		{
			return _sync;
		}

		void initSync(bool sync)
		{
			_sync = sync;
		}

		void setSync(bool sync = true)
		{
			_sync = sync;

			if (_sync) {
				// TODO: add wait animation
				syncNTP();
			}
			else {
				getI2Ctime();
			}
			//prepare();
		}

		int8_t getUTC()
		{
			return g_utc;
		}

		void initUTC(int8_t utc)
		{
			g_utc = utc;
		}

		void setUTC(void* obj)
		{
			if (obj == nullptr)
				return;

			_timeSynced = false;
			_userInputSettled = false;
			_userInputTimestamp = millis();

			InputField* utc = (InputField*) obj;

			g_utc = utc->getValue();

			/*
			// TODO: postpone or move sync to different task
			if (this->_sync) {
				syncNTP();
			}

			prepare();
			invalidate();
			*/
		}

		void getI2Ctime()
		{
			time_t time;
			time = rtc.gettimeUnix();
			struct tm *tmp = localtime(&time);
			//xSemaphoreTake(xNTPmutex, (TickType_t) 0);
			_timeinfo = *tmp;
			//xSemaphoreGive(xNTPmutex);
		}

		/*
		int getI2CgmtimeHours()
		{
			time_t time;
			time = rtc.gettimeUnix();
			struct tm *tmp = gmtime(&time);
			//_timeinfo = *tmp;
			return *tmp.tm_hour;
		}
		*/

		void setI2Ctime()
		{
			//xSemaphoreTake(xNTPmutex, (TickType_t) 0);
			rtc.settimeUnix(mktime(&_timeinfo));
			//xSemaphoreGive(xNTPmutex);
		}

		void syncNTP()
		{
			if (!g_ping_success) {
				//setI2Ctime();
				return;
			}

			/*
			xTaskCreate(
					vSyncNTPtask,
					//std::bind(&DateTime::_vSyncNTPtask, this, std::placeholders::_1),
					"syncNTPtask",
					2000,
					&_timeinfo,
					1,
					NULL
					);
					*/
			configTime(3600*g_utc, 0, _nptServer);
			if (!getLocalTime(&_timeinfo)) {
				getI2Ctime();
#ifdef APP_DEBUG
				Serial.println("failed to sync ntp time");
#endif
				return;
			}
			else {
				setI2Ctime();
			}
		}

		virtual void invalidate() override
		{
			for (auto& i:_visible) {
				i.invalidate();
			}
		}

		virtual void prepare() override
		{

			//xSemaphoreTake(xNTPmutex, ( TickType_t ) 0);
			mktime(&_timeinfo);

			_visible[HOUR].setValue(_timeinfo.tm_hour);
			_visible[MIN].setValue(_timeinfo.tm_min);
			_visible[DAY].setValue(_timeinfo.tm_mday);
			_visible[MON].setValue(_timeinfo.tm_mon + 1);
			_visible[YEAR].setValue(_timeinfo.tm_year + 1900); // +1900

			for (auto& i:_visible) {
				if (_sync) {
					i.setColors(FONT_COL_565, TFT_WHITE);
					i.setSelectable(false);
				}
				else {
					i.setColors(FONT_COL_565, COL_GREY_E3_565);
					i.setSelectable(true);
				}
				i.prepare();
			}

			if (currPage == pages[TIME_PG]) {
				_fieldsTitle.erase();
			}

			if (_sync) {
				_fieldsTitle.setText(DT_CURR);
			}
			else {
				_fieldsTitle.setText(DT_MANUAL);
			}

			_fieldsTitle.invalidate();
			_fieldsTitle.prepare();

			pages[TIME_PG]->restock();
			//xSemaphoreGive(xNTPmutex);
		}

		void build()
		{
			_fieldsTitle.setXYpos(PG_LEFT_PADD, 150);


			timePage.addItem(&_fieldsTitle);

			for (auto& i:_visible) {
				i.adjustX(-4);
				i.adjustTextX(-4);
				i.showLeadZero();
				i.setFont(MIDFONT);
				i.setText(EMPTY_STR);
				i.setWidth(TWO_CHR);
				timePage.addItem(&i);
			}


			_visible[HOUR].setLimits(0, 23);
			_visible[MIN].setLimits(0, 59);

			_visible[DAY].setLimits(1, 31);
			_visible[MON].setLimits(1, 12);
			_visible[YEAR].setLimits(1900, 2100);

			_visible[HOUR].setText(DT_SEMI);
			_visible[DAY].setText(DT_DOT);
			_visible[MON].setText(DT_DOT);

			_visible[HOUR].setXYpos(PG_LEFT_PADD, _y);
			_visible[MIN].setXYpos(53, _y);
			_visible[DAY].setXYpos(109, _y);
			_visible[MON].setXYpos(147, _y);
			_visible[YEAR].setXYpos(187, _y);

			_visible[DAY].adjustTextY(2);
			_visible[MON].adjustTextY(2);

			_visible[HOUR].setCallback(std::bind(&DateTime::setHours, this, std::placeholders::_1));
			_visible[MIN].setCallback(std::bind(&DateTime::setMinutes, this, std::placeholders::_1));
			_visible[DAY].setCallback(std::bind(&DateTime::setDay, this, std::placeholders::_1));
			_visible[MON].setCallback(std::bind(&DateTime::setMon, this, std::placeholders::_1));
			_visible[YEAR].setCallback(std::bind(&DateTime::setYear, this, std::placeholders::_1));

			_visible[YEAR].setWidth(FOUR_CHR);
			_visible[YEAR].adjustWidth(4);
		}
} datetime;


void resetCalibFlags()
{
	g_ph_calib_4_done = false;
	g_ph_calib_9_done = false;
	g_tds_calib_500_done = false;
	g_tds_calib_1500_done = false;
}

void resetDiagFlags()
{
	g_tds_diag_wait_done = false;
	g_ph_diag_wait_done = false;
}

#define SENS_READ_MILS 1000

// pointers to ADC diag output fields
OutputField* gADC[N_ADC];
#define ADC_READ_INTERVAL 200
#define DIG_READ_INTERVAL 200

// pointers to DIG diag indicators
CircIndicator* gKEYS[DIG_NKEYS];

class App {
	private:
		unsigned long _digMils = 0;
		unsigned long _adcMils = 0;
		unsigned long _oldMils = 0;
		unsigned long _dbMils = 0;
		unsigned long _dimMils = 0;
		unsigned long _sensMils = 0;
		bool _blink = false;
		bool _dbFlag = false;
		Cursor _cursor;
		int _iterator = 0;
		int16_t _dimmed = 10;
		int16_t _prevBright = g_init_brightness;
		bool _inactive = false;

	public:
		void draw()
		{
			currPage->draw();
		}

		void resetIterator()
		{
			_iterator = 0;
		}

		void init()
		{
			resetCalibFlags();
			switch (g_selected_lang) {
				default:
				case RU_LANG: scrStrings = ruStrings; break;
				case EN_LANG: scrStrings = engStrings; break;
			}
				//rtc.begin();
			g_ping_success = false;
			//SPIFFS.begin();
			tft.init();
			//tft.initDMA(true);
			tft.setRotation(0);
			tft.fillScreen(greyscaleColor(BACKGROUND));
			//Serial.println("Finish INIT");
#ifdef TASKS
			createTasks();
		}

		void createTasks()
		{
			xTaskCreate(
					ping_task_callback,
					"ping task",
					2000,
					NULL,
					3,
					NULL
				   );
			/*
			xTaskCreate(
					rtc_task,
					"rtc",
					2000,
					NULL,
					1,
					NULL
				   );
				   */
#endif
		}

		void update()
		{

			topBar.update();
			datetime.update();

			/* diag page analog inputs */

			if (currPage == pages[ADC_DIAG_PG] && millis() - _adcMils > ADC_READ_INTERVAL) {
				io.readADC();

				uint16_t* values = io.getAnalogValues();

				for (int i = 0; i < N_ADC; i++) {
					/*
					if (gADC[i] == nullptr)
						continue;
						*/
					gADC[i]->setValue(values[i]);
				}

				_adcMils = millis();
			}

			if (currPage == pages[DIG_DIAG_PG] && millis() - _digMils > DIG_READ_INTERVAL) {
				io.readDigital();

				bool* keys = io.getDigitalValues();

				for (int i = 0; i < DIG_NKEYS; i++) {
					/*
					if (gKEYS[i] == nullptr)
						continue;
						*/
					gKEYS[i]->on(keys[i]);
				}

				_dimMils = millis();
			}

			/* calib pages global items. */

			// wait for ph1 clalib
			if (currPage == pages[CAL_PH3_PG] && g_ph_calib_4_done && g_ph3wait.isVisible()) {
				g_ph3wait.erase();
				g_ph3wait.setInvisible();
				g_ph_next.setVisible();
				currItem = &g_ph_next;
				_iterator = 0;
			}

			// wait for ph2 clalib
			if (currPage == pages[CAL_PH5_PG] && g_ph_calib_9_done && g_ph5wait.isVisible()) {
				g_ph5wait.erase();
				g_ph5wait.setInvisible();
				g_ph_done.setVisible();
				g_ph_succ.setVisible();
				currItem = &g_ph_done;
				_iterator = 0;
			}

			// wait for tds1 calib
			if (currPage == pages[CAL_TDS3_PG] && g_tds_calib_500_done && g_tds3wait.isVisible()) {
				g_tds3wait.erase();
				g_tds3wait.setInvisible();
				g_tds_next.setVisible();
				currItem = &g_tds_next;
				_iterator = 0;
			}

			// wait for tds2 calib
			if (currPage == pages[CAL_TDS5_PG] && g_tds_calib_1500_done && g_tds5wait.isVisible()) {
				g_tds5wait.erase();
				g_tds5wait.setInvisible();
				g_tds_done.setVisible();
				g_tds_succ.setVisible();
				currItem = &g_tds_done;
				_iterator = 0;
				gpio[1].digitalWrite(TDS_MTR_RLY, LOW);
			}

			// reset wait flags and pages items
			if (currPage == pages[SENS_DIAG_PG] && (g_ph_diag_wait_done || g_tds_diag_wait_done)) {
				resetDiagFlags();
				g_phWait.setVisible();
				g_tdsWait.setVisible();
				g_ph_read.setInvisible();
				g_tds_read.setInvisible();
			}

			// wait for ph diag
			if (currPage == pages[PH_DIAG_PG] && g_ph_diag_wait_done && g_phWait.isVisible()) {
				g_phWait.erase();
				g_phWait.setInvisible();
				g_ph_read.setVisible();
				currItem = currPage->getCurrItemAt(0);
				//currItem = &back;
				_iterator = 0;
			}

			// show ph
			if (currPage == pages[PH_DIAG_PG] && g_ph_diag_wait_done
					&& millis() - _sensMils > SENS_READ_MILS) {

				_sensMils = millis();
				// update ph readings
				float ph = ph_meter.getPH();
				g_ph_read.setValue(ph);
#ifdef APP_DEBUG
				Serial.print("ph readings: ");
				Serial.println(ph, 1);
#endif
			}

			// wait for tds diag
			if (currPage == pages[TDS_DIAG_PG] && g_tds_diag_wait_done && g_tdsWait.isVisible()) {
				g_tdsWait.erase();
				g_tdsWait.setInvisible();
				g_tds_read.setVisible();
				currItem = currPage->getCurrItemAt(0);
				//currItem = &tds_diag_back;
				_iterator = 0;
			}

			// show tds
			if (currPage == pages[TDS_DIAG_PG] && g_tds_diag_wait_done
					&& millis() - _sensMils > SENS_READ_MILS) {
				_sensMils = millis();
				// update tds readings
				int tds = tds_meter.getTDS();
				g_tds_read.setValue(tds);
#ifdef APP_DEBUG
				Serial.print("tds readings: ");
				Serial.println(tds);
#endif
			}

			if (millis() - _dimMils > g_dimafter * 1000 && !_inactive) {
				_inactive = true;
				_dimMils = millis();
				_prevBright = gBrightness.getValue();
				gBrightness.setValue(_dimmed);
				gBrightness.onClick();
			}

			// if we are on wifi settings page and net status changed
			if (currPage == pages[WIFI_SETT_PG] && topBar.netChanged()) {
				// reset flag
				topBar.resetChange();
				// change one line at the bottom of wifi settings page
				if (g_ping_success) {
					gwsConnection.setText(WS_SUCC);
					gwsConnection.setColors(GREEN_COL_MACRO, greyscaleColor(BACKGROUND));
				}
				else {
					gwsConnection.setText(WS_FAIL);
					gwsConnection.setColors(RED_COL_MACRO, greyscaleColor(BACKGROUND));
				}
				gwsConnection.erase();
				gwsConnection.invalidate();
				gwsConnection.prepare();
			}

			draw();
#ifdef TASKS
			//yield();
			sleep(10);
#endif
#ifdef APP_DEBUG
#define DEBUG_TIMER 10000
			static unsigned long debugMils = 0;
			if (millis() - debugMils > DEBUG_TIMER) {
				debugMils = millis();
				Serial.print("Free heap: ");
				Serial.println(ESP.getFreeHeap());
			}
#endif

			// cursor blink
			if (millis() - _oldMils > CURSOR_TIMER) {
				_cursor.draw(_blink);
				_oldMils = millis();
				_blink = !_blink;
			}


			// return if no interupts from expander
			//pinMode(EXPANDER_INT, INPUT);
			if (digitalRead(EXPANDER_INT) == HIGH) {
				return;
			}

#ifdef APP_DEBUG
			Serial.println("Interrupt...");
#endif

			// read buttons
			uint8_t user_input = gpio[0].portRead(0);

			// debounce
			if ((uint8_t)~user_input) {
				if (millis() - _dbMils > DEBOUNCE) {
					user_input = gpio[0].portRead(0);
					if ((uint8_t)~user_input){
						_dbMils = millis();
						_dbFlag = true;
						_dimMils = millis();
					}
				}
			}

			// input proc
			if (!_dbFlag)
				return;

			//userinput.update();

			if (_inactive) {
				//Serial.println("Bang!");
				_inactive = false;
				gBrightness.setValue(_prevBright);
				gBrightness.onClick();
			}


			//if (!digitalRead(BTN_PREV)) {
			if (~user_input & BTN_PREV) {
				//Serial.println("Bang!");
				_cursor.draw(false);
				_iterator--;
				if (_iterator < 0)
					_iterator = currPage->selSize() - 1;

				ScrObj* tmp;
				if ((tmp = currPage->getCurrItemAt(_iterator)) != nullptr) {
					currItem = tmp;
				}

				//currItem = currPage->getCurrItemAt(_iterator);
				_cursor.draw(true);
				_dbFlag = false;
			}
			//else if (!digitalRead(BTN_NEXT)) {
			else if (~user_input & BTN_NEXT) {
				_cursor.draw(false);
				_iterator++;
				if (_iterator > currPage->selSize() - 1)
					_iterator = 0;

				ScrObj* tmp;
				if ((tmp = currPage->getCurrItemAt(_iterator)) != nullptr) {
					currItem = tmp;
				}

				_cursor.draw(true);
				_dbFlag = false;
			}
			//else if (!digitalRead(BTN_MIN)) {
			else if (~user_input & BTN_MIN) {
				currItem->setValue(currItem->getValue() - 1);
				if (currItem->hasInput())
					currItem->onClick();
				currItem->draw();
				_dbFlag = false;
			}
			else if (~user_input & BTN_PLU) {
				currItem->setValue(currItem->getValue() + 1);
				if (currItem->hasInput())
					currItem->onClick();
				currItem->draw();
				_dbFlag = false;
			}
			else if (~user_input & BTN_OK) {
				_cursor.draw(false);
				currItem->onClick();
				if (_iterator >= currPage->nItems())
					_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
				_dbFlag = false;
			}
			/*
			else if (~user_input & BTN_HOME) {
				struct HeapStats_t stats;
				vPortGetHeapStats(&stats);
			}
			*/

			// don't blink if buttons were pressed...
			_oldMils = millis();
		}
};

#endif
