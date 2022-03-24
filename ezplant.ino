//TODO: consider resource manager and page builder classes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "esp_task_wdt.h"

// Wifi
#include <WiFi.h>
#include <WiFiAP.h>

// GUI & strings
#include "GfxUi.h"
#include "rustrings.h"
#include "enstrings.h"
#include "stringenum.h"
#include "Gui.h"
#include "appBuilder.h"

static App app;

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
	builder.rebuildSettingsPage();
	callPage(pages[WIFI_PG]);
}

void callPage(void* page_ptr)
{
	if (page_ptr == nullptr)
		return;

	Page* page = (Page*) page_ptr;

	app.resetIterator();

	//back.setCallback(callPage, currPage);
	back.setCallback(callPage, page->prev());

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

	pages[TIME_PG]->restock();

	checkbox->invalidate();
}

// gui task
#ifdef APP_DEBUG
#define STACK_CHECK_INTERVAL 10000
#endif
//static iarduino_PCA9555 buttons(0x20);
void gui(void* arg)
{
#ifdef APP_DEBUG
	Serial.begin(115200);
#endif
	//Serial.println("Start INIT");
	// init all stuff in Gui.h
	app.init();


#ifdef APP_DEBUG
	unsigned long oldMillis = millis();
#endif


	//delay(500);

	for(;;) {
#if CONFIG_FREERTOS_UNICORE
		yieldIfNecessary();
#endif
		app.update();
#ifdef APP_DEBUG
		if (millis() - oldMillis > STACK_CHECK_INTERVAL) {

			//Serial.print("gpio status: ");
			//Serial.println(buttons.portRead(0), BIN);

			uint16_t unused = uxTaskGetStackHighWaterMark(NULL);
			Serial.print("gui task unused stack: ");
			Serial.println(unused);
			oldMillis = millis();
		}
#endif
		if (serialEventRun) serialEventRun();
		//yield();
		//sleep(10);
	}
}

void gSetBacklight(void* arg)
{
	uint8_t mapped_br = map(gBrightness.getValue(), 0, 100, 0, 255);
	//ledcWrite(0, mapped_br);
	//Serial.println("the brightness is: ");
	//Serial.println(mapped_br);
	analogWrite(LED_PIN, mapped_br);
}

void setup(void)
{

	xTaskCreate(
			gui,
			"gui",
			getArduinoLoopTaskStackSize(),
			NULL,
			1,
			NULL
		   );
	vTaskDelete(NULL);
}

void loop() { }
