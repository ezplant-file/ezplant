#ifndef __APP_H__
#define __APP_H__

// print object address
#define DEBUG_PRINT(A) Serial.println((unsigned long long) (A))
#define DEBUG_PRINT_HEX(A) Serial.println((unsigned long long) (A), HEX)

#include <ctime>
#include <atomic>
#include "Gui.h"
#include "IO.h"
#include "Rig.h"
#include "DateTime.h"
#include "Online.h"

// hardware... stuff
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WebServer.h>

WebServer server(80);

// ping
#include <ESP32Ping.h>

#define PRESET_UTC 3

/*--------------------------------------------------------------------------------------------*/

// debounce stuff
#define CURSOR_TIMER 500
#define DEBOUNCE 150

#define sleep(A) (vTaskDelay((A) / portTICK_PERIOD_MS))

// dim screen after
#define LOWER_DIMAFTER 5
#define HIGHER_DIMAFTER 300

// settings
uint8_t g_dimafter = 20;
int16_t g_init_brightness = 30;
bool g_ntp_sync = true;
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

#ifdef STACK_DEBUG
		uint16_t unused = uxTaskGetStackHighWaterMark(NULL);
		Serial.print("ping task unused stack: ");
		Serial.println(unused);
#endif

		sleep(10000);
	}
}


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

String gTimeStr;


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

			//_statusWIFI.reload();
			_statusWIFI.draw();
			_statusWIFI.freeRes();

			//_statusInternet.reload();
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

			/* time stuff */
			if (currPage == pages[MAIN_PG]) {
				gTimeStr = *datetime.getTimeStr();
				_time.invalidate();
				_time.prepare();
				_time.erase();
				_time.invalidate();
				_time.draw();
			}

			_timestamp = millis();

			int dBm = WiFi.RSSI();
			uint8_t strength = map(dBm, -95, -45, 0, 4);

#ifdef WIFI_DEBUG
			Serial.print("WiFi strength: ");
			Serial.println(dBm);
			Serial.print("Wifi status: ");
			Serial.println(WiFi.status());
#endif

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
		}
	private:

		void _showTime()
		{
			_showtime = true;
			_time.setVisible();
		}

		void _hideTime()
		{
			_showtime = false;
			_time.setInvisibleNoErase();
		}

		SimpleBox _topBox;
		Image _statusWIFI;
		Image _statusInternet;
		Text _menuText;
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
} topBar;

// main page indicators
OutputFieldMain* g_tem;
OutputFieldMain* g_hum;
OutputField* g_ph;
OutputField* g_tds;
CircIndicator* g_light;
CircIndicator* g_passvent;
CircIndicator* g_vent;
CircIndicator* g_pump;

// small boxes
SmallBox* g_Tap;
SmallBox* g_A;
SmallBox* g_B;
SmallBox* g_C;
SmallBox* g_ph_up;
SmallBox* g_ph_dw;

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

bool saveSettings(void);

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
		int today = 0;

	public:
		int days()
		{
			return today;
		}

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
			resetCalibFlags();
			switch (g_selected_lang) {
				default:
				case RU_LANG: scrStrings = ruStrings; break;
				case EN_LANG: scrStrings = engStrings; break;
			}
			//rtc.begin();
			g_ping_success = false;

			tft.init();
			//tft.initDMA(true);
			tft.setRotation(0);
			tft.fillScreen(greyscaleColor(BACKGROUND));

			createTasks();

			online.init();

			today = datetime.getDays();
			uint8_t percent = (float) today / g_data.getInt(GR_CYCL_3_DAYS) * 100.0;
			if (g_first_launch)
				percent = 0;
			Serial.print("Today percent init: ");
			Serial.println(percent);
			//Serial.print("Init day ");
			//Serial.println(today);
			g_ProgBar.setValue(percent);
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

			// tds and ph normalization
			setMeasIntervalMinutes(g_data.getInt(ADD_MEAS_INT));

			// updaters
			topBar.update();
			datetime.update();
			g_rig.update();

			/* online stuff */
			online.update();
			if (online.tokenLoaded()) {
				online.sendData();
			}

			if (millis() - _measMils > _measInterval  || !_initDone) {
				_measMils = millis();
				g_rig.measureTime();
				_initDone = true;
#ifdef RIG_DEBUG
				Serial.println();
				Serial.println("***************************************");
				Serial.println("Подошло время снятия показаний датчиков");
#endif
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
				gMainPageText->invalidate();
				gMainPageText->draw();

			}

			if (today != datetime.getDays()) {
				today = datetime.getDays();
				uint8_t percent = (float) today / g_data.getInt(GR_CYCL_3_DAYS) * 100.0;
				percent = clamp(percent, 0, 100);
#ifdef TIME_DEBUG
				Serial.print("Today percent: ");
				Serial.println(percent);
#endif
				g_ProgBar.setValue(percent);
			}

			if (currPage == pages[MAIN_PG] && millis() - _mainMils > MAIN_P_UPDATE) {
				//Serial.println(percent);
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
				g_pump->on(g_rig.getPump());
				g_pump->draw();

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

				_digMils = millis();
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
			else if (io.userHome() && currPage != pages[MAIN_PG]) {
				_cursor.draw(false);
				callPage(pages[MAIN_PG]);
				currItem = currPage->getCurrItemAt(_iterator);
				saveSettings();
				g_data.save();
			}

			_dbFlag = false;

			// don't blink and don't dim if buttons were pressed...
			_blinkMils = _dimMils = millis();
			_triggered = true;
		}
};

#endif
