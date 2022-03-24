#ifndef _APP_BUILDER_
#define _APP_BUILDER_
#include "Gui.h"

class Builder {
	private:
		Page* buildTimePage();
		Page* buildWifiPage();
		Page* buildWiFiSettPage();
		Page* buildFontPage();
		Page* buildLangPage();
		Page* buildMainPage();
		Page* buildSettingsPage();
	public:
		void rebuildSettingsPage()
		{
			pages[SETT_PG] = buildSettingsPage();
		}
		void build()
		{
			pages[TIME_PG] = buildTimePage();
			pages[WIFI_SETT_PG] = buildWiFiSettPage();
			pages[WIFI_PG] = buildWifiPage();
			pages[FONT_PG] = buildFontPage();
			pages[TEST_PG] = buildTestPage();
			pages[LANG_PG] = buildLangPage();
			pages[SETT_PG] = buildSettingsPage();
			pages[MENU_PG] = buildMainPage();

			pages[TIME_PG]->setPrev(pages[SETT_PG]);
			pages[LANG_PG]->setPrev(pages[SETT_PG]);
			pages[WIFI_SETT_PG]->setPrev(pages[SETT_PG]);
			pages[WIFI_PG]->setPrev(pages[SETT_PG]);

			//pages[]->setPrev(pages[]);

			pages[SETT_PG]->setPrev(pages[MENU_PG]);
			pages[FONT_PG]->setPrev(pages[MENU_PG]);
			pages[TEST_PG]->setPrev(pages[MENU_PG]);

		}
} builder;
#endif
