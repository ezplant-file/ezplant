#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include "Gui.h"
// TODO: always get time from i2c, sync i2c once per hour...

// NTP
#define NTP_SERVER "pool.ntp.org"

Page timePage;

extern Page* pages[NPAGES];
extern Page* currPage;

#define RTC_CHECK_INTERVAL 60000
#define USER_INPUT_SETTLE 500

std::atomic<int8_t> g_utc;

extern std::atomic<bool> g_ping_success;


enum {
	HOUR,
	MIN,
	DAY,
	MON,
	YEAR,
	N_DATETIME_VISIBLE
};

typedef std::unique_lock<std::mutex> lock_t;

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
			timeTread = std::thread(std::bind(&DateTime::_updateTime, this));
			_sync_succ = false;
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

		std::atomic<bool> _sync_succ;
		std::thread timeTread;
		std::mutex timeMutex;
		std::condition_variable conditionVar;
		static constexpr int UPDATE_TIMEOUT = 60;

		void _updateTime()
		{
			//std::unique_lock<std::mutex> lock(timeMutex);
			lock_t lock(timeMutex);
			conditionVar.wait(lock);
			for (;;) {
				syncNTP();
				conditionVar.wait_for(lock, std::chrono::minutes(UPDATE_TIMEOUT));
			}
		}

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
			return _timeinfo.tm_yday - _startday + 1;
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

				// notify
				conditionVar.notify_one();

				/***************/
				//syncNTP();
				//prepare();
				//invalidate();
				_userInputSettled = false;
				_timeSynced = true;
			}

			if (_sync_succ) {
				invalidate();
				prepare();
				_sync_succ = false;
			}


			if (millis() - _rtcMils > RTC_CHECK_INTERVAL) {
				_rtcMils = millis();

				getI2Ctime();

				if (currPage == pages[TIME_PG]) {
					invalidate();
					prepare();
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
				//syncNTP();
				conditionVar.notify_one();
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
			//lock_t lock(timeMutex);
			time_t time;
			time = rtc.gettimeUnix();
			struct tm *tmp = localtime(&time);
			_timeinfo = *tmp;
			//lock.unlock();
		}

		void setI2Ctime()
		{
			//lock_t lock(timeMutex);
			rtc.settimeUnix(mktime(&_timeinfo));
			//lock.unlock();
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
				_sync_succ = true;
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
			/*
			lock_t lock(timeMutex);
			*/

			mktime(&_timeinfo);

			_visible[HOUR].setValue(_timeinfo.tm_hour);
			_visible[MIN].setValue(_timeinfo.tm_min);
			_visible[DAY].setValue(_timeinfo.tm_mday);
			_visible[MON].setValue(_timeinfo.tm_mon + 1);
			_visible[YEAR].setValue(_timeinfo.tm_year + 1900);

			//lock.unlock();

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
				//i.adjustX(-4);
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
			lock_t lock(timeMutex);

			String hour = "";
			if (_timeinfo.tm_hour < 10)
				hour += "0";
			String min = "";
			if (_timeinfo.tm_min < 10)
				min += "0";
			min += String(_timeinfo.tm_min);
			hour += String(_timeinfo.tm_hour);
			_timeString = (String) hour + ":" + min;

			lock.unlock();

			return &_timeString;
		}

		String* getDateStr()
		{
			lock_t lock(timeMutex);

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

			lock.unlock();

			return &_dateString;
		}

	private:
		String _dateString;
		String _timeString;
} datetime;

#endif
