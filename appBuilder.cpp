#include "appBuilder.h"
#include "Gui.h"
/*******************************************************************************
page builders TODO: move to Gui
*******************************************************************************/

/************************ TIME PAGE ******************************/
Page* Builder::buildTimePage()
{
	static Page timePage;
	//pages[TIME_PG] = &timePage;

	static Image timeCal;
	timeCal.setXYpos(167, 82);
	timeCal.loadRes(images[IMG_TIME_CAL]);

	static CheckBox syncCheck;
	syncCheck.setXYpos(PG_LEFT_PADD, 45);
	syncCheck.setText(DT_SYNC);
	syncCheck.adjustTextY(-7);
	syncCheck.on(false);
	syncCheck.setCallback(syncTimeCallback, &syncCheck);

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

Page* Builder::buildWifiPage()
{

	static Page wifiPage;
	// TEXT
	uint16_t bg_col = greyscaleColor(BACKGROUND);
	uint16_t font1_col = greyscaleColor(FONT_COLOR);
	uint16_t font2_col = greyscaleColor(WI_PG_FONT_COL);
	//pages[WIFI_PG] = &wifiPage;

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


Page* Builder::buildWiFiSettPage()
{
	static Page wifiSettPage;
	//pages[WIFI_SETT_PG] = &wifiSettPage;

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
	gwsWifiChBox.on(true);
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

Page* Builder::buildTestPage()
{
	static Page testPage;
	//pages[TEST_PG] = &testPage;
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

	static InputField testInput;
	testInput.setFont(SMALLFONT);
	testInput.setXYpos(17, 115);
	testInput.setValue(100);
	testInput.setText(INPUT_TEXT);
	testInput.setColors(
			greyscaleColor(FONT_COLOR),
			greyscaleColor(GR_BTN_BG_COLOR)
			);

	static GreyTextButton testGreyButton;
	testGreyButton.setXYpos(17, 150);
	testGreyButton.setText(GREY_BUTTON);
	testGreyButton.setFont(SMALLFONT);
	testGreyButton.setCallback(nop);

	//TODO: depricate this:
	testGreyButton.setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));

	static BlueTextButton testBlueButton;
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
	//testPage.setPrev(&mainPage);
	testPage.addItem(&back);
	return &testPage;
}


/************************ FONT PAGE ******************************/
Page* Builder::buildFontPage()
{
	static Page fontPage;

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

	//pages[FONT_PG] = &fontPage;
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
	//fontPage.setPrev(&mainPage);
	fontPage.addItem(&back);

	return &fontPage;
}


/************************************ LANG PAGE ******************************/
Page* Builder::buildLangPage()
{
	static Page langPage;
	//pages[LANG_PG] = &langPage;
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
	gBrightness.setValue(init_brightness);
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
	ruSelect.on(true);
	ruSelect.setCallback(changeLangRus);

	//static CircRadBtn enSelect;
	enSelect.setXYpos(20, 238);
	enSelect.setCurCol(greyscaleColor(TOP_BAR_BG_COL));
	enSelect.on(false);
	enSelect.setCallback(changeLangEng);

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

#define menu1_size 6
Page* Builder::buildMainPage()
{
	static Page mainPage;
	//pages[MENU_PG] = &mainPage;

	static GreyTextButton menu_items[menu1_size];
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

	//menu_items[2].setCallback(callSettingsPage);
	menu_items[2].setCallback(callPage, pages[SETT_PG]);
	//menu_items[4].setCallback(callTestPage);
	menu_items[4].setCallback(callPage, pages[TEST_PG]);
	//menu_items[5].setCallback(callFontPage);
	menu_items[5].setCallback(callPage, pages[FONT_PG]);

	for (int i = 0; i < menu1_size; i++) {
		mainPage.addItem(&menu_items[i]);
	}

	back.setCallback(nop);
	back.loadRes(images[IMG_PREV]);
	back.setXYpos(7, 284);
	back.setCircle();

	mainPage.setTitle(MENU);
	mainPage.addItem(&back);
	return &mainPage;
}

#define settings_size 5

Page* Builder::buildSettingsPage()
{
	static Page settingsPage;
	static GreyTextButton settings_items[settings_size];
	//pages[SETT_PG] = &settingsPage;
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

	settings_items[0].setCallback(callPage, pages[TIME_PG]);
	//settings_items[2].setCallback(callLangPage);
	if (g_wifi_set) {
		settings_items[1].setCallback(callPage, pages[WIFI_SETT_PG]);
	}
	else {
		settings_items[1].setCallback(callPage, pages[WIFI_PG]);
	}
	settings_items[2].setCallback(callPage, pages[LANG_PG]);

	for (int i = 0; i < settings_size; i++) {
		settingsPage.addItem(&settings_items[i]);
	}

	settingsPage.setTitle(SETTINGS);
	//settingsPage.setPrev(pages[MENU_PG]);
	//settingsPage.setPrev(&mainPage);
	settingsPage.addItem(&back);
	return &settingsPage;
}

