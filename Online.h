#ifndef __ONLINE_H__
#define __ONLINE_H__
//#include <WiFiClientSecure.h>
#include "json.hpp"
#include <HTTPClient.h>
#include "stringenum.h"
#include "settings.h"
#include "IO.h"

using json = nlohmann::json;

// online page dynamic text
String codeTxt = "";
String secTxt = "";

class OnlineMointor {
	private:
		Page* _page = nullptr;
		ScrObj* _button = nullptr;
		ScrObj* _codetxt = nullptr;
		ScrObj* _onlinePageCode = nullptr;
		ScrObj* _reqTxt = nullptr;
		ScrObj* _timeOutText = nullptr;
		ScrObj* _connectedText = nullptr;
		bool _wait_for_resp = false;

		const char* _keyKey = "key";
		const char* _timeIntervalKey = "timeInterval";
		const char* _tokenAPIKey = "tokenAPI";
		int _key = 0;
		int _keepFor = 0;
		int _countDown = 0;
		const char* _root_ca= ""; //\
				      "-----BEGIN CERTIFICATE-----\n" \
				      "MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/\n" \
				      "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
				      "DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow\n" \
				      "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
				      "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB\n" \
				      "AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC\n" \
				      "ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL\n" \
				      "wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D\n" \
				      "LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK\n" \
				      "4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5\n" \
				      "bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y\n" \
				      "sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ\n" \
				      "Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4\n" \
				      "FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc\n" \
				      "SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql\n" \
				      "PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND\n" \
				      "TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n" \
				      "SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1\n" \
				      "c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx\n" \
				      "+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB\n" \
				      "ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu\n" \
				      "b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E\n" \
				      "U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu\n" \
				      "MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC\n" \
				      "5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW\n" \
				      "9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG\n" \
				      "WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O\n" \
				      "he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC\n" \
				      "Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5\n" \
				      "-----END CERTIFICATE-----\n";

		WiFiClient _client;

		const char*  _server = "api.ezplant.ru";
		const char* _request1 = "/api/device-add.php";
		const char* _request2 = "/api/device-add-chek.php?key=";
		const char* _postReq = "/api/data.php";
		const char* _protocol = "http://"; // will be https eventually
		const char* _tokenFile = "/token";
		// for POST
		const char* _settings = "SETTINGS";
		const char* _sensors = "SENSORS";

		unsigned long _confirmMils = 0;
		unsigned long _countDownMils = 0;
		unsigned long _req1_timestamp = 0;
		unsigned long _sentMils = 0;
		static constexpr unsigned long _CONF_REPEAT = 20000;
		static constexpr unsigned long _POST_REPEAT = 30000;
		bool _tokenLoaded = false;
		bool _onTimer = false;

		String _token = "";

		void _waitForConfirm()
		{
			if (!_countDown)
				return;

			if (millis() - _countDownMils > 1000) {
				_countDownMils = millis();

				_countDown--;
				//secTxt = (String) _countDown; // + " " + scrStrings[TXT_SECONDS];
				if (_timeOutText) {
					_timeOutText->setValue(_countDown);
					//_timeOutText->erase();
					//_timeOutText->invalidate();
					//_timeOutText->prepare();
				}
			}
		}

		void _redrawOnlinePage()
		{

			if (_codetxt) {
				_codetxt->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_codetxt->erase();
			}

			if (_onlinePageCode) {
				_onlinePageCode->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_onlinePageCode->erase();
			}

			if (_reqTxt) {
				_reqTxt->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_reqTxt->erase();
			}

			if (_timeOutText) {
				_timeOutText->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_timeOutText->erase();
			}

			if (_connectedText) {
				_connectedText->setVisible();
				if (currPage == pages[ONLINE_PG]) {
					_connectedText->prepare();
					_connectedText->draw();
				}
			}

			if (_button) {
				_button->setText(OM_DISCONNECT);
				_button->setCallback([this](void*){this->untether();});
				_button->setVisible();
			}

			if (_page)
				_page->restock();
		}

		void _redrawExpired()
		{
			if (_codetxt) {
				_codetxt->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_codetxt->erase();
			}

			if (_onlinePageCode) {
				_onlinePageCode->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_onlinePageCode->erase();
			}

			if (_reqTxt) {
				_reqTxt->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_reqTxt->erase();
			}

			if (_timeOutText) {
				_timeOutText->setInvisible();
				if (currPage == pages[ONLINE_PG])
					_timeOutText->erase();
			}

			if (_button)
				_button->setVisible();

			if (_page)
				_page->restock();
		}

		void _deleteTokenFile()
		{
			if (!SPIFFS.exists("/token"))
				return;
			SPIFFS.remove("/token");
			_tokenLoaded = false;
#ifdef ONLINE_DEBUG
			Serial.println("Token file removed");
#endif
		}

	public:
		void untether()
		{
			if (_button) {
				_button->setText(OM_BUTTON);
				_button->setCallback([this](void*){this->startOnline();});
			}

			if (_connectedText) {
				_connectedText->setInvisible();
			}

			_deleteTokenFile();
			_redrawExpired();
		}

		void setOnline(
				Page* page,
				ScrObj* button,
				ScrObj* codetxt,
				ScrObj* num,
				ScrObj* req,
				ScrObj* sec,
				ScrObj* connected)
		{
			_page = page;
			_button = button;
			_codetxt = codetxt;
			_onlinePageCode = num;
			_reqTxt = req;
			_timeOutText = sec;
			_connectedText = connected;
		}

		void startOnline() {

			if (!connect())
				return;

			_wait_for_resp = true;

			_button->setInvisible();
			_button->erase();

			//_button->setInvisibleNoErase();
			//_button->erase();

			_codetxt->setVisible();
			_codetxt->prepare();
			_codetxt->draw();

			_onlinePageCode->setVisible();

			_reqTxt->setVisible();
			_reqTxt->prepare();
			_reqTxt->draw();

			_timeOutText->setVisible();
			_timeOutText->prepare();

			_page->restock();

		}

		bool tokenLoaded()
		{
			return _tokenLoaded;
		}

		void init()
		{
			_tokenLoaded = loadToken();
		}

		bool loadToken()
		{
			if (!SPIFFS.exists(_tokenFile)) {
#ifdef ONLINE_DEBUG
			Serial.println("token file doesn't exist");
#endif
				return false;
			}

			File file = SPIFFS.open(_tokenFile, "r");

			if (!file) {
#ifdef ONLINE_DEBUG
				Serial.println("failed to load token file");
#endif
				return false;
			}

			_token = file.readStringUntil(EOF);
#ifdef ONLINE_DEBUG
			Serial.println(_token);
#endif
			file.close();

			return true;
		}

		bool saveToken()
		{
			File file = SPIFFS.open(_tokenFile, "w");
			if (!file) {
#ifdef ONLINE_DEBUG
				Serial.println("failed to save token file");
#endif
				return false;
			}

			file.print(_token.c_str());
			file.flush();
			file.close();
			_tokenLoaded = true;

			return true;
		}

		void reset()
		{
			_key = 0;
			_keepFor = 0;
			codeTxt = "";
			_client.stop();
			_onTimer = false;
			// TODO: reset page
		}

		bool connect()
		{
			if (!g_ping_success) {
#ifdef ONLINE_DEBUG
				Serial.println("Нет подключения");
#endif
				return false;
			}

			// TODO: add action if _tokenLoaded

			HTTPClient http;
			String req = (String)_protocol+_server+_request1;
			http.begin(_client, req.c_str());

			int httpCode = http.GET();

			if (httpCode != HTTP_CODE_OK) {
#ifdef ONLINE_DEBUG
				Serial.println("Http code: " + String(httpCode));
#endif
				_client.stop();
				return false;
			}

			String payload = http.getString();
#ifdef ONLINE_DEBUG
			Serial.println(payload);
#endif
			try
			{
				json resp = json::parse(payload.c_str());
				_key = resp[_keyKey].get<int>();
				_keepFor = resp[_timeIntervalKey].get<int>();
				_countDown = _keepFor;
#ifdef ONLINE_DEBUG
				Serial.println("responses");
				Serial.println(_key);
				Serial.println(_keepFor);
#endif
				resp.clear();
			}

			catch (...)
			{
#ifdef ONLINE_DEBUG
				Serial.println("json failed");
#endif
				_client.stop();
				return false;
			}

			_req1_timestamp = millis();
			_client.stop();
			return true;
		}

		bool confirm()
		{
			if (millis() - _confirmMils < _CONF_REPEAT)
				return false;
#ifdef ONLINE_DEBUG
			Serial.println("Trying to confirm...");
#endif

			if (millis() - _req1_timestamp > _keepFor*1000 && _key) {
				reset();
#ifdef ONLINE_DEBUG
				_redrawExpired();
				Serial.println("Key expired");
#endif
				return false;
			}

			_confirmMils = millis();

			HTTPClient http;
			String req = (String)_protocol+_server+_request2+String(_key);
			http.begin(_client, req.c_str());

			int httpCode = http.GET();

			if (httpCode != HTTP_CODE_OK) {
#ifdef ONLINE_DEBUG
				Serial.println("Http code: " + String(httpCode));
#endif
				_client.stop();
				return false;
			}

			String payload = http.getString();
#ifdef ONLINE_DEBUG
			Serial.println(payload);
#endif
			try
			{
				json resp = json::parse(payload.c_str());
				std::string token = resp[_tokenAPIKey].get<std::string>();
				_token = String(token.c_str());
#ifdef ONLINE_DEBUG
				Serial.println("responses");
				Serial.println(_token);
#endif
				resp.clear();
				saveToken();

				_redrawOnlinePage();

				reset();
			}
			catch (...)
			{
#ifdef ONLINE_DEBUG
				Serial.println("online confirm json failed");
#endif
				_client.stop();
				return false;
			}
			return true;
		}

		int getCurrKey()
		{
			return _key;
		}

		void sendData()
		{
			if (!g_ping_success)
				return;

			if (millis() - _sentMils < _POST_REPEAT)
				return;

			_sentMils = millis();

			json data;

			// token
			data[_tokenAPIKey] = _token.c_str();

			// all rig settings
			data[_settings]["RIG_TYPE"] = g_rig_type;

			for (int i = 0; i < NSETTINGS; i++) {
				data[_settings][STR((rig_settings_t)i)] = g_data.getFloat((rig_settings_t)i);
			}

			//all rig sensors
			if (io.noTds())
				data[_sensors]["EC"] = "no EC sensor";
			else
				data[_sensors]["EC"] = io.getEC();

			if (io.noPh())
				data[_sensors]["PH"] = "no pH sensor";
			else
				data[_sensors]["PH"] = io.getPH();

			if (io.noSht())
				data[_sensors]["SHT"] = "no SHT sensor";
			else {
				data[_sensors]["SHT"]["TEM"] = io.getTem();
				data[_sensors]["SHT"]["HUM"] = io.getHum();
			}

			io.readADC();

			uint16_t* values = io.getAnalogValues();

			data[_sensors]["ADC1"] = values[ADC_1];
			data[_sensors]["ADC2"] = values[ADC_2];
			data[_sensors]["ADC3"] = values[ADC_3];
			data[_sensors]["ADC4"] = values[ADC_4];

			data[_sensors]["PORT_A"] = io.getOut(PWR_PG_PORT_A);
			data[_sensors]["PORT_B"] = io.getOut(PWR_PG_PORT_B);
			data[_sensors]["PORT_C"] = io.getOut(PWR_PG_PORT_C);
			data[_sensors]["PORT_D"] = io.getOut(PWR_PG_PORT_D);
			data[_sensors]["PORT_E"] = io.getOut(PWR_PG_PORT_E);
			data[_sensors]["PORT_F"] = io.getOut(PWR_PG_PORT_F);
			data[_sensors]["PORT_G"] = io.getOut(PWR_PG_PORT_G);
			data[_sensors]["PORT_H"] = io.getOut(PWR_PG_PORT_H);
			data[_sensors]["FAN"] = io.getOut(PWR_PG_FAN);
			data[_sensors]["LIGHT"] = io.getOut(PWR_PG_LIGHT);
			data[_sensors]["MOTOR_UP"] = io.getOut(PWR_PG_UP);
			data[_sensors]["MOTOR_DOWN"] = io.getOut(PWR_PG_DOWN);

			HTTPClient http;
			String req = (String)_protocol+_server+_postReq;
			http.begin(_client, req);

			int httpCode = http.POST(data.dump().c_str());

			if (httpCode = HTTP_CODE_OK) {
#ifdef ONLINE_DEBUG
				Serial.println("Data sent");
				Serial.println(data.dump().c_str());
#endif
			}
			data.clear();
		}

		void update()
		{
			if (!_key)
				return;

			if (_wait_for_resp) {

				if (_key < 100000)
					codeTxt = "0";

				codeTxt += String(_key);

				if (_onlinePageCode) {
					_onlinePageCode->prepare();
				}

				_wait_for_resp = false;
				_onTimer = true;
			}

			if (_onTimer) {
				_waitForConfirm();
			}

			confirm();
		}

} online;

#endif
