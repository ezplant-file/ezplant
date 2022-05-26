#ifndef __APP_H__
#define __APP_H__

//TODO: page builder

#include <ctime>
#include <atomic>
#include "Gui.h"
#include "IO.h"

// hardware... stuff
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WebServer.h>

WebServer server(80);

// ping
#include <ESP32Ping.h>

/*--------------------------------------------------------------------------------------------*/

// print object address
#define DEBUG_PRINT(A) Serial.println((unsigned long long) (A))
#define DEBUG_PRINT_HEX(A) Serial.println((unsigned long long) (A), HEX)

// debounce stuff
#define CURSOR_TIMER 500
#define DEBOUNCE 150

#define sleep(A) (vTaskDelay((A) / portTICK_PERIOD_MS))

// dim screen after
#define LOWER_DIMAFTER 3
#define HIGHER_DIMAFTER 180

// NTP
#define NTP_SERVER "pool.ntp.org"

// settings
uint8_t g_dimafter = 20;
int16_t g_init_brightness = 30;
bool g_ntp_sync = false;
int8_t gUTC = 3;
bool g_wifi_on = true;
bool g_first_launch = true;

/*--------------------------------------------------------------------------------------------*/

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
std::atomic<int8_t> g_utc;

void ping_task_callback(void* arg)
{
	sleep(5000);
	for(;;) {
		if (g_wifi_set) {
			if (Ping.ping(REMOTE_HOST, 3)) {
				g_ping_success = true;
			}
			else {
				g_ping_success = false;
			}
		}
		/*
#ifdef APP_DEBUG
		uint16_t unused = uxTaskGetStackHighWaterMark(NULL);
		Serial.print("ping task unused stack: ");
		Serial.println(unused);
#endif
*/
		sleep(10000);
	}
}

typedef enum {
	MAIN_PG,
	MENU_PG,
	SETT_PG,
	LANG_PG,
	//FONT_PG,
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
	FIRST_PG,
	STAGE1_PG,
	STAGE2_PG,
	STAGE3_PG,
	STAGE4_PG,
	STAGE5_PG,
	STAGE6_PG,
	STAGE7_PG,
	STAGE8_PG,
	STAGE9_PG,
	// stage 2 alt
	STAGE92_PG,
	STAGE82_PG,
	STAGE83_PG,
	STAGE84_PG,
	//LSTAGES,
	ADDSETT_PG,
	ADDSETT2_PG,
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

#include "settings.h" // rig_settings_t, rig_type, g_data


// ezplant.ino:
void callPage(void*);

/*
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
*/

String gTimeStr;
// TODO: always get time from i2c, sync i2c once per hour...
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
			getI2Ctime();
			if (_sync) {
				syncNTP();
			}
		}

	private:
		InputField _visible[N_DATETIME_VISIBLE];
		Text _fieldsTitle;
		bool _sync = false;
		bool _userInputSettled = false;
		bool _timeSynced = false;
		struct tm  _timeinfo;
		unsigned long _rtcMils = 0;
		const char* const _nptServer = NTP_SERVER;
		unsigned long _userInputTimestamp = 0;
		int _count = 0;
		int _startday = 0;
	public:

		int getHour()
		{
			return _timeinfo.tm_hour;
		}

		int getMinute()
		{
			return _timeinfo.tm_min;
		}

		int getDays()
		{
			return _timeinfo.tm_yday - _startday;
		}

		void setStartDay()
		{
			_startday = _timeinfo.tm_yday;
		}

		void loadStartDay()
		{
			_startday = g_data.getInt(START_DAY);
		}

		int getStartDay()
		{
			return _startday;
		}

		// TODO: redo.
		void update()
		{
			if (millis() - _userInputTimestamp > USER_INPUT_SETTLE
					&& !_userInputSettled
					&& !_timeSynced) {
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
				invalidate();
				prepare();
				g_sync_succ = false;
			}

			if (millis() - _rtcMils > RTC_CHECK_INTERVAL) {
				_rtcMils = millis();

				/*****

				Serial.print("now: ");
				time_t now = time(0);
				Serial.println(now);
				struct tm* date = localtime(&now);
				Serial.print("y day: ");
				Serial.println(date->tm_yday);

				*****/

				if (_sync && _count == 0) {
					syncNTP();
					setI2Ctime();
				}

				_count++;
				if (_count == 60) {
					_count = 0;
				}


				getI2Ctime();

				if (currPage == pages[TIME_PG]) {
					prepare();
					invalidate();
				}
			}
		}

		virtual void freeRes() override
		{
		}

		virtual void draw() override
		{
			// page methods deal with that
		}

		// GUI functions
		void setHours(void* obj)
		{
			_timeinfo.tm_hour = _visible[HOUR].getValue();
			rtc.settimeUnix(mktime(&_timeinfo));
		}

		void setMinutes(void* obj)
		{
			_timeinfo.tm_min = _visible[MIN].getValue();
			rtc.settimeUnix(mktime(&_timeinfo));
		}

		void setDay(void* obj)
		{
			_timeinfo.tm_mday = _visible[DAY].getValue();
			rtc.settimeUnix(mktime(&_timeinfo));
		}

		void setMon(void* obj)
		{
			_timeinfo.tm_mon = _visible[MON].getValue() - 1;
			rtc.settimeUnix(mktime(&_timeinfo));
		}

		void setYear(void* obj)
		{
			_timeinfo.tm_year = _visible[YEAR].getValue() - 1900;
			rtc.settimeUnix(mktime(&_timeinfo));
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
		}

		int8_t getUTC()
		{
			return g_utc;
		}

		void initUTC(int8_t utc)
		{
			g_utc = utc;
			configTime(g_utc*3600, 0, "");
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
		}

		void getI2Ctime()
		{
			time_t time;
			time = rtc.gettimeUnix();
			struct tm *tmp = localtime(&time);
			_timeinfo = *tmp;
		}

		void setI2Ctime()
		{
			rtc.settimeUnix(mktime(&_timeinfo));
		}

		void syncNTP()
		{
			if (!g_ping_success) {
				return;
			}

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

			mktime(&_timeinfo);

			_visible[HOUR].setValue(_timeinfo.tm_hour);
			_visible[MIN].setValue(_timeinfo.tm_min);
			_visible[DAY].setValue(_timeinfo.tm_mday);
			_visible[MON].setValue(_timeinfo.tm_mon + 1);
			_visible[YEAR].setValue(_timeinfo.tm_year + 1900);

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

			_visible[HOUR].setText(DT_COLON);
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

		String* getTimeStr()
		{
			String hour = "";
			if (_timeinfo.tm_hour < 10)
				hour += "0";
			String min = "";
			if (_timeinfo.tm_min < 10)
				min += "0";
			min += String(_timeinfo.tm_min);
			hour += String(_timeinfo.tm_hour);
			_timeString = (String) hour + ":" + min;
			return &_timeString;

		}

		String* getDateStr()
		{
			String hour = "";
			if (_timeinfo.tm_hour < 10)
				hour += "0";
			String min = "";
			if (_timeinfo.tm_min < 10)
				min += "0";
			String mon = "";
			if (_timeinfo.tm_mon < 10)
				mon += "0";
			min += String(_timeinfo.tm_min);
			hour += String(_timeinfo.tm_hour);
			mon += String(_timeinfo.tm_mon);
			_dateString = (String) hour
				+ ":" + min
				+ " " + _timeinfo.tm_mday
				+ "." + mon
				+ "." + _timeinfo.tm_year%100;
			return &_dateString;
		}

	private:
		String _dateString;
		String _timeString;
} datetime;

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
			_time.invalidate();
			//_hours.invalidate();
			//_minutes.invalidate();
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

			if (currPage == pages[MAIN_PG]) {
				_showTime();
				gTimeStr = *datetime.getTimeStr();
				_time.invalidate();
				_time.prepare();
				_time.draw();
				/*
				_hours.setValue(
				_hours.prepare();
				_hours.draw();
				_minutes.prepare();
				_minutes.draw();
				*/
			}
			else {
				_hideTime();
			}

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

			gTimeStr = *datetime.getTimeStr();
			_time.invalidate();
			_time.prepare();
			_time.erase();
			_time.draw();
			/*
			_hours.erase();
			_minutes.erase();
			_hours.invalidate();
			_minutes.invalidate();
			_hours.setValue(datetime.getHour());
			_minutes.setValue(datetime.getMinute());
			_hours.prepare();
			_hours.draw();
			_minutes.prepare();
			_minutes.draw();
			*/

			_timestamp = millis();

			int dBm = WiFi.RSSI();
			uint8_t strength = map(dBm, -95, -45, 0, 4);

			/*
#ifdef APP_DEBUG
			Serial.print("WiFi strength: ");
			Serial.println(dBm);
			Serial.print("Wifi status: ");
			Serial.println(WiFi.status());
#endif
*/

			strength = clamp(strength, 0, 4);

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

			/*
			_time.setFont(MIDFONT);
			_time.setXYpos(LEFTMOST, TOPMOST);
			_time.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(TOP_BAR_BG_COL));
			_time.setText(MENU); // special case
			*/
			_statusWIFI.loadRes(images[_curWiFiImage]);
			_statusWIFI.setXYpos(WIFI_IMG_X, 0);

			_statusInternet.loadRes(images[_curNetImage]);
			_statusInternet.setXYpos(NET_IMG_X, 0);

			_time.setXYpos(145, 12);
			_time.setColors(COL_GREY_70_565, COL_GREY_E3_565);
			_time.setPaddingX(0);
			_time.setWH(0, 12);
			_time.setText(gTimeStr);
			/*
			_hours.setXYpos(140, 5);
			_hours.noBg();
			_hours.setText(DT_COLON);
			_hours.adjustTextX(-21);
			_hours.setColors(COL_GREY_70_565, COL_GREY_E3_565);
			_hours.showLeadZero();

			_minutes.setXYpos(161, 5);
			_minutes.noBg();
			_minutes.noText();
			_minutes.setColors(COL_GREY_70_565, COL_GREY_E3_565);
			_minutes.showLeadZero();
			*/
		}
	private:

		void _showTime()
		{
			_showtime = true;
			_time.setVisible();
			//_hours.setVisible();
			//_minutes.setVisible();
			/*
			_hours.setValue(datetime.getHour());
			_minutes.setValue(datetime.getMinute());
			_hours.invalidate();
			_minutes.invalidate();
			_hours.prepare();
			_minutes.prepare();
			_hours.draw();
			_minutes.draw();
			*/
		}

		void _hideTime()
		{
			_showtime = false;
			_time.setInvisibleNoErase();
			//_hours.setInvisible();
			//_minutes.setInvisible();
		}

		SimpleBox _topBox;
		Image _statusWIFI;
		Image _statusInternet;
		Text _menuText;
		//Text _time;
		bool _changed = false;
		bool _wifiIsON = false;
		bool _showtime = false;
		unsigned long _timestamp = 0;
		unsigned long _interval = WIFI_UPDATE_INTERVAL;
		images_t _curWiFiImage = IMG_NO_WIFI;
		images_t _curNetImage = IMG_NET_NO;
		images_t _prevWiFiImage = IMG_NO_WIFI;
		images_t _prevNetImage = IMG_NET_NO;
		StringText _time;
		//OutputField _hours, _minutes;
} topBar;

#ifdef RIG_DEBUG
OutputField gMesState;
#endif

// main page indicators
OutputFieldMain* g_tem;
OutputFieldMain* g_hum;
OutputField* g_ph;
OutputField* g_tds;
CircIndicator* g_light;
CircIndicator* g_passvent;
CircIndicator* g_vent;

// small boxes
SmallBox* g_Tap;
SmallBox* g_A;
SmallBox* g_B;
SmallBox* g_C;
SmallBox* g_ph_up;
SmallBox* g_ph_dw;


class Rig {
	public:
		Rig(): _paused(true){};

		float lastPH()
		{
			return _lastPh;
		}

		float lastEC()
		{
			return _lastEc;
		}

		void start()
		{
			_paused = false;
		}

		void halt()
		{
			_paused = true;
			_led = false;
			_passvent = false;
			_vent = false;
			io.haltAll();
			//Serial.println("halted...");
		}

		void update()
		{
			if (_paused)
				return;

			_updateLight();
			_updateVent();
			_updatePassVent();
			_updateSolutions();
			_updateRigType();
		}

		bool getLed()
		{
			return _led;
		}

		bool getVent()
		{
			return _vent;
		}

		bool getPassVent()
		{
			return _passvent;
		}

		void setMeasureWindow(bool window=true)
		{
			_measureWindow = window;
		}

		void measureTime()
		{
			_measuretime = true;
		}

		bool measureDone()
		{
			bool measuredone = _measuredone;
			_measuredone = false;
			return measuredone;
		}

	private:
		bool _measuredone = false;
		bool _measuretime = false;
		bool _paused;
		bool _window_opened = false;
		bool _window_energized = false;
		unsigned long _window_timer = 0;
		static constexpr unsigned long WINDOW_INT = 60000;
		bool _led = false;
		bool _vent = false;
		bool _passvent = false;
		bool _measureWindow = false;

		void _updateLight()
		{
			_led = false;

			if (!g_data.getInt(LIGHT_ON)) {
				io.haltPWMout(PWR_PG_LIGHT);
				return;
			}

			// check day
			if (datetime.getDays() < g_data.getInt(LIGHT_DAY)) {
				io.haltPWMout(PWR_PG_LIGHT);
				return;
			}

			// check hour
			int tmp = datetime.getHour();
			if (g_data.getInt(LIGHT_FROM) <= tmp && tmp < g_data.getInt(LIGHT_TO)) {

				// set led brightness
				uint8_t brightness = g_data.getInt(ADD_LED_BRIGHT);
				io.drivePWMout(PWR_PG_LIGHT, brightness);
				_led = true;
			}
			else {
				io.haltPWMout(PWR_PG_LIGHT);
			}
		}


		void _updateVent()
		{
			_vent = false;
			if (!g_data.getInt(VENT_ON)) {
				io.driveOut(PWR_PG_FAN, false);
				return;
			}

			bool a = g_data.getInt(VENT_TIME_LIM);
			bool b = g_data.getInt(VENT_TEMP_LIM);
			bool c = g_data.getInt(VENT_HUM_LIM);
			int tmp = datetime.getHour();
			bool x = (g_data.getInt(VENT_TIME_FROM) <= tmp) && (tmp < g_data.getInt(VENT_TIME_TO));
			bool y = int(io.getTem()) > g_data.getInt(VENT_TEMP_THRES);
			bool z = int(io.getHum()) > g_data.getInt(VENT_HUM_THRES);

			if (!a)
				x = false;
			if (!b || io.noSht())
				y = false;
			if (!c || io.noSht())
				z = false;

			bool Q = x || y || z;

			if (Q) {
				io.driveOut(PWR_PG_FAN, true);
				_vent = true;
			}
			else
				io.driveOut(PWR_PG_FAN, false);
		}

		void _openWindow()
		{
			io.driveOut(PWR_PG_UP, true);
			_window_energized = true;
			_window_timer = millis();
		}

		void _closeWindow()
		{
			io.driveOut(PWR_PG_DOWN, true);
			_window_energized = true;
			_window_timer = millis();
		}

		void _updatePassVent()
		{
			if (_window_energized || _window_opened) {
				_passvent = true;
			}
			else {
				_passvent = false;
			}

			if (!g_data.getInt(PASSVENT)) {
				return;
			}

			if (!_window_energized) {
				io.driveOut(PWR_PG_UP, false);
			}

			if (_window_energized) {
				if (millis() - _window_timer > WINDOW_INT) {
					_window_energized = false;
					_window_opened = !_window_opened;
				}
				return;
			}

			bool a = g_data.getInt(PASSVENT_TIME_LIM);
			bool b = g_data.getInt(PASSVENT_TEMP_LIM);
			bool c = g_data.getInt(PASSVENT_HUM_LIM);
			int tmp = datetime.getHour();
			bool x = (g_data.getInt(PASSVENT_TIME_FROM) <= tmp)
				&& (tmp < g_data.getInt(PASSVENT_TIME_TO));
			bool y = int(io.getTem()) > g_data.getInt(PASSVENT_TEMP_THRES);
			bool z = int(io.getHum()) > g_data.getInt(PASSVENT_HUM_THRES);

			if (!a)
				x = false;
			if (!b || io.noSht())
				y = false;
			if (!c || io.noSht())
				z = false;

			bool Q = x || y || z;

			if (Q && !_window_opened) {
				_openWindow();
			}
			else if (!Q && _window_opened)
				_closeWindow();
		}

		bool _commenceABC = false;
		bool _prepareABC = false;
		bool _commencePH = false;
		bool _preparePH = false;
		int _pumpAseconds = 0;
		int _pumpBseconds = 0;
		int _pumpCseconds = 0;
		int _pumpPHseconds = 0;

		static constexpr unsigned long _begin_interval = 10000;
		unsigned long _begin_mils = 0;
		bool _metersBusy = false;
		float _lastPh;
		float _lastEc;

		bool _localMeas = false;
		bool _localMeas2 = false;
		unsigned long _meas_mils = 0;
		static constexpr unsigned long _meas_interval = 15000;

		void _measure()
		{
			if (!_metersBusy) {
				io.initMeters();

#ifdef RIG_DEBUG
				gMesState.setValue(1);
#endif

				_begin_mils = millis();
				_metersBusy = true;
			}

			if (millis() - _begin_mils > _begin_interval && !_localMeas) {

				for (int i = 0; i < 3; i++) {
					_lastPh = io.getPH();
					_lastEc = io.getEC();
				}

				_meas_mils = millis();
				_localMeas = true;
				_localMeas2 = true;
			}

			if (millis() - _meas_mils > _meas_interval && _localMeas2) {
				_lastPh = io.getPH();
				_lastEc = io.getEC();

#ifdef RIG_DEBUG
				gMesState.setValue(2);
#endif

				_measuretime = false;
				_localMeas = false;
				_metersBusy = false;
				_localMeas2 = false;
				_measuredone = true;

				if (!io.noTds()) {
					_prepareABC = true;

				}
				else if (!io.noPh()) {
					_preparePH = true;
				}
#ifdef RIG_DEBUG
				else
					gMesState.setValue(0);
#endif


				_mainpumpwindow = false;
				_aeropumpwindow = false;
				_rigInitDone = true;
			}
		}

		void _updateSolutions()
		{
			if (_measuretime) {
				_measure();
				return;
			}

			_updateABC();
			_squirtABC();
			_updatePH();
			_squirtPH();
		}


		void _updateABC()
		{
			if (!_prepareABC)
				return;

			if (_commenceABC)
				return;

			if (!g_data.getInt(EC_ON)) {
				_prepareABC = false;
				Serial.println("EC not set in settings");
				return;
			}

			// return if one of the tanks is empty
			bool* dig = io.getDigitalValues();
			bool a = dig[DIG_KEY10];
			bool b = dig[DIG_KEY9];
			bool c = dig[DIG_KEY8];

			if (a || b || c) {
				Serial.print("A: ");
				Serial.println(a);
				Serial.print("B: ");
				Serial.println(b);
				Serial.print("C: ");
				Serial.println(c);
				_prepareABC = false;
				_preparePH = true;
				return;
			}

			rig_settings_t ec_set, ec_a, ec_b, ec_c;
			int pumptime = g_data.getInt(EC_PUMPS);
			int today = datetime.getDays();

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
				ec_set = EC_CYCL1;
				ec_a = EC_A1;
				ec_b = EC_B1;
				ec_c = EC_C1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL2;
				ec_a = EC_A2;
				ec_b = EC_B2;
				ec_c = EC_C2;
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {
				ec_set = EC_CYCL3;
				ec_a = EC_A3;
				ec_b = EC_B3;
				ec_c = EC_C3;
			}

			// if EC more than what's set
			if (g_data.getFloat(ec_set) - g_data.getFloat(EC_HYST) <= _lastEc) {
				_prepareABC = false;
				_preparePH = true;
				return;
			}

			_commenceABC = true;;
			//squirt amount set in settings
			_pumpAseconds = g_data.getFloat(ec_a) * pumptime;
			_pumpBseconds = g_data.getFloat(ec_b) * pumptime;
			_pumpCseconds = g_data.getFloat(ec_c) * pumptime;
			_pump_mils = millis();
			_prepareABC = false;

			//Serial.println("ABC updated");
		}

		unsigned long _pump_mils = 0;
		enum {
			A_ON,
			B_ON,
			C_ON
		} pumps = A_ON;

		void _squirtABC()
		{
			if (!_commenceABC)
				return;

			if (pumps == A_ON) {
				if (!io.getOut(PWR_PG_PORT_A)) {
#ifdef RIG_DEBUG
					gMesState.setValue(3);
#endif

					io.driveOut(PWR_PG_PORT_A, true);
				}
				if (millis() - _pump_mils > _pumpAseconds*1000) {
					io.driveOut(PWR_PG_PORT_A, false);
					pumps = B_ON;
					_pump_mils = millis();
				}
			}

			if (pumps == B_ON) {
				if (!io.getOut(PWR_PG_PORT_B)) {
					io.driveOut(PWR_PG_PORT_B, true);
				}
				if (millis() - _pump_mils> _pumpBseconds*1000) {
					io.driveOut(PWR_PG_PORT_B, false);
					pumps = C_ON;
					_pump_mils = millis();
				}
			}

			if (pumps == C_ON) {
				if (!io.getOut(PWR_PG_PORT_C)) {
					io.driveOut(PWR_PG_PORT_C, true);
				}
				if (millis() - _pump_mils> _pumpCseconds*1000) {
					io.driveOut(PWR_PG_PORT_C, false);
					pumps = A_ON;
					_commenceABC = false;
					_preparePH = true;
				}
			}
			//Serial.println("ABC squirted");
		}

		enum {
			NO_PH,
			PH_UP,
			PH_DW,
		} _ph_state;

		void _updatePH()
		{

			if (!_preparePH || _commencePH)
				return;

			if (io.noPh()) {
				_preparePH = false;
#ifdef RIG_DEBUG
				gMesState.setValue(0);
#endif
				return;
			}


			if (!g_data.getInt(ACID_ON)) {
				_preparePH = false;
#ifdef RIG_DEBUG
				gMesState.setValue(0);
				Serial.println("PH not set in settings");
#endif
				return;
			}


			rig_settings_t ph_set;
			int today = datetime.getDays();

			if (today < g_data.getInt(GR_CYCL_1_DAYS)) {
				ph_set = ACID_1;
			}
			else if (g_data.getInt(GR_CYCL_1_DAYS) <= today && today < g_data.getInt(GR_CYCL_2_DAYS)) {
				ph_set = ACID_2;
			}
			else if (today >= g_data.getInt(GR_CYCL_2_DAYS)) {
				ph_set = ACID_3;
			}

			/*
			Serial.println("PH - HYST: ");
			Serial.println(g_data.getFloat(ph_set) - g_data.getFloat(PH_HYST), 1);
			Serial.print("Last PH: ");
			Serial.println(_lastPh, 1);
			*/

			float ph = g_data.getFloat(ph_set);
			float ph_hyst = g_data.getFloat(PH_HYST);

			// if PH is less
			if (ph - ph_hyst > _lastPh) {
				_ph_state = PH_UP;
			}
			else if (ph + ph_hyst < _lastPh) {
				_ph_state = PH_DW;
			}
			else {
				_ph_state = NO_PH;
			}

			_commencePH = true;
			_preparePH = false;
			_pump_mils = millis();
		}


		void _squirtPH()
		{
			if (!_commencePH)
				return;

			// tank states
			bool* dig = io.getDigitalValues();
			bool up_empty = dig[DIG_KEY7];
			bool down_empty = dig[DIG_KEY6];

			_pumpPHseconds = g_data.getInt(ACID_PUMPS);

			if (_ph_state == PH_UP) {
				if (up_empty) {
					Serial.println("PH UP EMPTY");
#ifdef RIG_DEBUG
					gMesState.setValue(0);
#endif
					_commencePH = false;
					return;
				}
				if (!io.getOut(PWR_PG_PORT_D)) {
#ifdef RIG_DEBUG
					gMesState.setValue(4);
#endif

					io.driveOut(PWR_PG_PORT_D, true);
				}
				if (millis() - _pump_mils > _pumpPHseconds*1000) {
					io.driveOut(PWR_PG_PORT_D, false);
					_ph_state = NO_PH;
#ifdef RIG_DEBUG
					gMesState.setValue(0);
#endif
					_commencePH = false;
				}
			}
			else if (_ph_state == PH_DW) {
				if (down_empty) {
					Serial.println("PH DOWN EMPTY");
#ifdef RIG_DEBUG
					gMesState.setValue(0);
#endif
					_commencePH = false;
					return;
				}
				if (!io.getOut(PWR_PG_PORT_E)) {
					// TODO: remove in prod
#ifdef RIG_DEBUG
					gMesState.setValue(4);
#endif

					io.driveOut(PWR_PG_PORT_E, true);
				}
				if (millis() - _pump_mils > _pumpPHseconds*1000) {
					io.driveOut(PWR_PG_PORT_E, false);
					_ph_state = NO_PH;
#ifdef RIG_DEBUG
					gMesState.setValue(0);
#endif
					_commencePH = false;
				}

			}
			else if (_ph_state == NO_PH) {
#ifdef RIG_DEBUG
				gMesState.setValue(0);
#endif
				_commencePH = false;
				return;
			}
		}

		void _updateRigType()
		{
			switch (g_rig_type) {
				default: break;
				case RIG_DEEPWATER: _deepwater(); break;
				case RIG_LAYER: _layer(); break;
				case RIG_FLOOD: _flood(); break;
				case RIG_AERO: _aero(); break;
				case RIG_DRIP: _drip(); break;
				case RIG_OPENG: _openg(); break;
				case RIG_GREENH: _greenh(); break;
				case RIG_MIXSOL: _mixsol(); break;
			}
		}

		bool _mainpumpwindow = false;
		bool _aeropumpwindow = false;
		bool _rigInitDone = false;
	public:
		void setMainPumpWindow()
		{
			_mainpumpwindow = true;
		}

		void setAeroPumpWindow()
		{
			_aeropumpwindow = true;
		}

	private:
		bool _pumpison = false;
		bool _haltpump = false;
		unsigned long _pumpMils = 0;
		static constexpr unsigned long _PUMP_TIMEOUT = 60000;

		void _deepwater()
		{
			if (!_rigInitDone)
				return;
			// TODO: H2O pump time limit
			// H2O pump
			if (!_haltpump) {
				if (!io.getDigital(DIG_KEY3) && !_measuretime) {
					io.driveOut(PWR_PG_PORT_F, !io.getDigital(DIG_KEY5));
				}
				else if (io.getDigital(DIG_KEY3) || io.getDigital(DIG_KEY4) || _measuretime) {
					io.driveOut(PWR_PG_PORT_F, false);
				}
			}
			else {
				//g_Tap->setBlink();
				io.driveOut(PWR_PG_PORT_F, false);
			}

			// reset flag is pump is off
			if (!io.getOut(PWR_PG_PORT_F)) {
				_pumpison = false;
			}

			// raise flag if pump is on
			if (!_pumpison && io.getOut(PWR_PG_PORT_F)) {
				_pumpison = true;
				// remember when
				_pumpMils = millis();
			}

			if (!_haltpump && _pumpison && millis() - _pumpMils > _PUMP_TIMEOUT) {
				_haltpump = true;
				Serial.println("Main pump timed out");
			}

			// aero pump
			if (_aeropumpwindow && g_data.getInt(AERO_PUMP)) {
				io.driveOut(PWR_PG_PORT_H, false);
			}
			else if (g_data.getInt(AERO_ON)){
				io.driveOut(PWR_PG_PORT_H, true);
			}

			// main pump
			if (_mainpumpwindow || _measuretime) {
				io.driveOut(PWR_PG_PORT_G, false);
			}
			else {
				io.driveOut(PWR_PG_PORT_G, io.getDigital(DIG_KEY2));
			}
		}

		void _layer()
		{
		}

		void _flood()
		{
		}

		void _aero()
		{
		}

		void _drip()
		{
		}

		void _openg()
		{
		}

		void _greenh()
		{
		}

		void _mixsol()
		{
		}

} g_rig;


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
#define ADC_READ_INTERVAL 500
#define DIG_READ_INTERVAL 200
#define MAIN_P_UPDATE 1000

// pointers to DIG diag indicators
CircIndicator* gKEYS[DIG_NKEYS];

enum {
	MOTOR_UP,
	MOTOR_DOWN,
	MOTOR_NITEMS
};

Toggle* exlMotorTgl[MOTOR_NITEMS];

StringText* gMainPageText;
String gMainPageStr = String(scrStrings[MP_STRING]);

class App {
	private:
		unsigned long _digMils = 0;
		unsigned long _adcMils = 0;
		unsigned long _blinkMils = 0;
		unsigned long _dbMils = 0;
		unsigned long _dimMils = 0;
		unsigned long _sensMils = 0;
		unsigned long _repMils = 0;
		unsigned long _mainMils = 0;
		unsigned long _measMils = 0;
		unsigned long _measInterval = 30 * 60000;
		bool _blink = false;
		bool _dbFlag = false;
		bool _triggered = false;
		bool _repeat = false;
		bool _initDone = false;
		Cursor _cursor;
		int _iterator = 0;
		int16_t _dimmed = 10;
		int16_t _prevBright = g_init_brightness;
		bool _inactive = false;
		static constexpr unsigned long REPEAT_INT = 1000;

	public:
		void setMeasIntervalMinutes(unsigned interval)
		{
			_measInterval = interval * 60000;
		}

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
#ifdef APP_TESTING
			setMeasIntervalMinutes(5);
#endif

			resetCalibFlags();
			switch (g_selected_lang) {
				default:
				case RU_LANG: scrStrings = ruStrings; break;
				case EN_LANG: scrStrings = engStrings; break;
			}
				//rtc.begin();
			g_ping_success = false;

			//datetime.init();
			//gMainPageStr = String(scrStrings[MP_STRING]) + *datetime.getDateStr();

			tft.init();
			//tft.initDMA(true);
			tft.setRotation(0);
			tft.fillScreen(greyscaleColor(BACKGROUND));

			//Serial.println("Finish INIT");
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
		}

		void update()
		{

			topBar.update();
			datetime.update();
			g_rig.update();

			/*
			Serial.println("measure stuff");
			Serial.println(_measInterval);
			Serial.println(_measMils);
			Serial.println(_initDone);
			Serial.println();
			*/
			if (millis() - _measMils > _measInterval  || !_initDone) {
				_measMils = millis();
				g_rig.measureTime();
				_initDone = true;
				Serial.println("Measure TIME!");
			}

			// main pump window set
			if (millis() - _measMils > _measInterval
					- g_data.getInt(PUMP_SEC)*1000
					&& g_data.getInt(PUMP_OFF)
					&& io.getOut(PWR_PG_PORT_G)) {
				g_rig.setMainPumpWindow();
			}

			// aero pump window set
			if (millis() - _measMils > _measInterval
					- g_data.getInt(AERO_PUMP_SEC)*1000
					&& g_data.getInt(AERO_PUMP)
					&& io.getOut(PWR_PG_PORT_H)) {
				g_rig.setAeroPumpWindow();
			}

			/* main page updates */
			if (currPage == pages[MAIN_PG] && g_rig.measureDone()) {
				g_ph->setValue(g_rig.lastPH());
				g_tds->setValue(g_rig.lastEC());


				gMainPageStr = String(scrStrings[MP_STRING]) + *datetime.getDateStr();
				gMainPageText->invalidate();
				gMainPageText->prepare();
				gMainPageText->erase();
				gMainPageText->draw();

			}

			if (currPage == pages[MAIN_PG] && millis() - _mainMils > MAIN_P_UPDATE) {
				g_tem->setValue(io.getTem());
				g_tem->draw();
				g_hum->setValue(int(io.getHum()));
				g_hum->draw();
				g_vent->on(g_rig.getVent());
				g_vent->draw();
				g_passvent->on(g_rig.getPassVent());
				g_passvent->draw();
				g_light->on(g_rig.getLed());
				g_light->draw();

				// bottom boxes
				/* inputs */
				bool* dig = io.getDigitalValues();
				if (dig[DIG_KEY10] != g_A->getEmpty()) {
					g_A->setEmpty(dig[DIG_KEY10]);
				}

				if (dig[DIG_KEY9] != g_B->getEmpty()) {
					g_B->setEmpty(dig[DIG_KEY9]);
				}

				if (dig[DIG_KEY8] != g_C->getEmpty()) {
					g_C->setEmpty(dig[DIG_KEY8]);
				}

				if (dig[DIG_KEY7] != g_ph_up->getEmpty()) {
					g_ph_up->setEmpty(dig[DIG_KEY7]);
				}

				if (dig[DIG_KEY6] != g_ph_dw->getEmpty()) {
					g_ph_dw->setEmpty(dig[DIG_KEY6]);
				}

				if (dig[DIG_KEY5] != g_Tap->getEmpty()) {
					g_Tap->setEmpty(dig[DIG_KEY5]);
				}

				/* tank */

				bool tanklow, tankmid, tankhi;
				tanklow = dig[DIG_KEY2];
				tankmid = dig[DIG_KEY3];
				tankhi = dig[DIG_KEY4];

				if (!tanklow && !tankmid) {
					g_tankBig.setState(T_EMPTY);
				}
				else if (tanklow && !tankmid) {
					g_tankBig.setState(T_HALF);
				}
				else if (tankmid && !tankhi) {
					g_tankBig.setState(T_TWOTHIRDS);
				}
				else if (tankmid && tankhi) {
					g_tankBig.setState(T_FULL);
				}

				/* outputs */
				if (io.getOut(PWR_PG_PORT_A) != g_A->isOn()) {
					g_A->on(io.getOut(PWR_PG_PORT_A));
				}

				if (io.getOut(PWR_PG_PORT_B) != g_B->isOn()) {
					g_B->on(io.getOut(PWR_PG_PORT_B));
				}

				if (io.getOut(PWR_PG_PORT_C) != g_C->isOn()) {
					g_C->on(io.getOut(PWR_PG_PORT_C));
				}

				if (io.getOut(PWR_PG_PORT_D) != g_ph_up->isOn()) {
					g_ph_up->on(io.getOut(PWR_PG_PORT_D));
				}

				if (io.getOut(PWR_PG_PORT_E) != g_ph_dw->isOn()) {
					g_ph_dw->on(io.getOut(PWR_PG_PORT_E));
				}

				if (io.getOut(PWR_PG_PORT_F) != g_Tap->isOn()) {
					g_Tap->on(io.getOut(PWR_PG_PORT_F));
				}


				_mainMils = millis();
			}

			/* diag page analog inputs */
			if (currPage == pages[ADC_DIAG_PG] && millis() - _adcMils > ADC_READ_INTERVAL) {
				io.readADC();

				uint16_t* values = io.getAnalogValues();

				for (int i = 0; i < N_ADC; i++) {
					gADC[i]->setValue(values[i]);
				}

				_adcMils = millis();
			}

			if (currPage == pages[DIG_DIAG_PG]
					&& millis() - _digMils > DIG_READ_INTERVAL) {
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
			sleep(10);
#endif
#ifdef HEAP_DEBUG
#define DEBUG_TIMER 2000
			static unsigned long debugMils = 0;
			if (millis() - debugMils > DEBUG_TIMER) {
				debugMils = millis();
				Serial.println("**************************************");
				Serial.print("Free heap: ");
				Serial.println(ESP.getFreeHeap());
				Serial.println();
				Serial.print("lagest 8-bit block: ");
				Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
				Serial.println();
				Serial.print("lagest 32-bit block: ");
				Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_32BIT));
				Serial.println();
				Serial.println("**************************************");
			}
#endif

			// cursor blink
			if (millis() - _blinkMils > CURSOR_TIMER) {
				_cursor.draw(_blink);
				_blinkMils = millis();
				_blink = !_blink;
			}

			bool pressed = io.update();

			if (pressed) {
				_blinkMils = millis();
			}

			if (!pressed) {
				_repMils = millis();
				_triggered = false;
				_repeat = false;
			}

			// brightness back to original on key press
			if (pressed && _inactive) {
				_inactive = false;
				gBrightness.setValue(_prevBright);
				gBrightness.onClick();
			}

			/* slow down user input */
			if (pressed && millis() - _dbMils > DEBOUNCE) {
				_dbMils = millis();
				_dbFlag = true;
			}

			if (!_dbFlag) {
				return;
			}

			/******** repeat ********/
			if (_triggered && pressed && (millis() - _repMils > REPEAT_INT)) {
				_repeat = true;
			}

			if (!_repeat && _triggered) {
				return;
			}

			/******* user input ******/
			if (io.userBack()) {
				_cursor.draw(false);
				_iterator--;
				if (_iterator < 0)
					_iterator = currPage->selSize() - 1;

				ScrObj* tmp;
				if ((tmp = currPage->getCurrItemAt(_iterator)) != nullptr) {
					currItem = tmp;
				}

				_cursor.draw(true);
			}
			else if (io.userForw()) {
				_cursor.draw(false);
				_iterator++;
				if (_iterator > currPage->selSize() - 1)
					_iterator = 0;

				ScrObj* tmp;
				if ((tmp = currPage->getCurrItemAt(_iterator)) != nullptr) {
					currItem = tmp;
				}

				_cursor.draw(true);
			}
			else if (io.userMinus()) {
				if (currItem->hasInput()) {
					currItem->sub();
					currItem->onClick();
				}
				currItem->draw();
			}
			else if (io.userPlus()) {
				if (currItem->hasInput()) {
					currItem->add();
					currItem->onClick();
				}
				currItem->draw();
			}
			else if (io.userOK()) {
				_cursor.draw(false);
				currItem->onClick();
				if (_iterator >= currPage->nItems())
					_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
			}
			else if (io.userHome()) {
				_cursor.draw(false);
				callPage(pages[MAIN_PG]);
				currItem = currPage->getCurrItemAt(_iterator);
			}

			_dbFlag = false;

			// don't blink and don't dim if buttons were pressed...
			_blinkMils = _dimMils = millis();
			_triggered = true;
		}
};

#endif
