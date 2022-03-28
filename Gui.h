//TODO: page builder
//TODO: figure out i2c time timezone
//
//
// precalculate all colors to uint16_t
//
// Obj:
// progress bar -
// input field - done
// checkbox - done
// switch - done
// wifi - done
// internet - done
//
// settings list:
// global set brightness - g_init_brightness
// global set wifi on - g_wifi_on
// global ntp sync - g_ntp_sync
#ifndef __GUI_H__
#define __GUI_H__

#include <vector>
#include <atomic>
#include <ctime>

// hardware... stuff
#include <SPI.h>
#include <TFT_eSPI.h>

// time
#include <iarduino_RTC.h>
iarduino_RTC rtc(RTC_RX8025);

// buttons
#include <iarduino_PCA9555.h>
iarduino_PCA9555 buttons(0x20); //first expander
iarduino_PCA9555 second_expander(0x21);

// ping
#include <ESP32Ping.h>

#include "images.h"

// web server

#include <WebServer.h>


/***************************** defines *************************/
// print object address
#define DEBUG_PRINT(A) Serial.println((unsigned long long) (A))

// green and red macros
#define RED_COL_MACRO (tft.color565(0xff, 0,0))
#define GREEN_COL_MACRO (tft.color565(0x4c, 0xaf, 0x50))

// grey button
#define GREY_BUTTON_HEIGHT 24
#define GREY_BUTTON_WIDTH 210
#define GR_BTN_X_PADDING 8
#define GR_BTN_Y_PADDING 7
#define GR_BTN_TXT_COLOR 0x70
#define GR_BTN_BG_COLOR 0xE3

// blue button
#define BLUE_BUTTON_HEIGHT 24
#define BL_BTN_X_PADDING 8
#define BL_BTN_Y_PADDING 7

// top bar
#define TOP_BAR_HEIGHT 27
#define TOP_BAR_BG_COL 0xE3
#define FONT_COLOR 0x70
#define sleep(A) (vTaskDelay((A) / portTICK_PERIOD_MS))
#define LEFTMOST 17
#define TOPMOST 11

// input field
#define IN_FLD_X_PADDING 8
#define IN_FLD_Y_PADDING 7



// common
#define SCR_WIDTH 240
#define SCR_HEIGHT 320
#define BACKGROUND 0xFF
#define SELECTABLE true

// left page padding
#define PG_LEFT_PADD 15

// vertical layout first item Y
#define MB_Y_START 41

//cursor color
#define CURCOL 0x70

/* calculated colors to 565 */
// default font color precalculated from 0x70
#define FONT_COL_565 0x738E
#define COL_GREY_E3_565 0xE71C

// debounce stuff
#define CURSOR_TIMER 500
#define DEBOUNCE 200

// checkbox
#define CHK_BOX_COL 0xDC
#define CHK_BOX_SIZE 21
#define CHK_BOX_FILE "/check.jpg"

// input field height
#define INPUT_H 24

// radio button
#define RAD_BTN_SIZE 22

// dim screen after
#define LOWER_DIMAFTER 3
#define HIGHER_DIMAFTER 180

// settings
uint8_t g_dimafter = 20;
int16_t g_init_brightness = 50;
bool g_ntp_sync = false;
int8_t gUTC = 0;
bool g_wifi_on = true;

// lang settings after typedef
typedef enum {
	RU_LANG,
	EN_LANG
} lang_t;

lang_t g_selected_lang = RU_LANG;



// fonts
typedef enum {
	SMALLESTFONT,
	SMALLFONT,
	MIDFONT,
	LARGEFONT,
	LARGESTFONT,
	BOLDFONT,
	BOLDFONT2,
	BOLDSMALL,
	END_OF_FONTS
} fonts_t;

const char* FONTS[END_OF_FONTS] = {
	"SegoeUI-12",
	"SegoeUI-14",
	"SegoeUI-16",
	"SegoeUI-18",
	"SegoeUI-20",
	"SegoeUI-Bold-16",
	"SegoeUI-Bold-18",
	"SegoeUI-Bold-14"
};

typedef enum {
	RIGHT,
	LEFT,
	TOP
} align_t;

// pointer to current language strings
const char** scrStrings = ruStrings;

// objs
TFT_eSPI tft = TFT_eSPI();

GfxUi ui = GfxUi(&tft);

// utils
const char* REMOTE_HOST = "www.iocontrol.ru";

//WiFiClient client;

WebServer server(80);

std::atomic<bool> g_rapid_blink;
std::atomic<bool> g_ping_success;
std::atomic<bool> g_wifi_set;
//atomic_bool g_ping_success = false;
//bool g_ping_success = false;
//iarduino_RTC rtc(RTC_DS3231);

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

/*
void rtc_task(void* arg)
{
	iarduino_RTC rtc(RTC_DS3231);
	rtc.begin();
	for (;;) {
		Serial.println(rtc.gettime("H:i"));
		sleep(10000);
	}
}
*/

/*
void rapid_blink_callback(void* arg)
{
	g_rapid_blink = true;
	cursorDraw(false);
	sleep(50);
	cursorDraw(true);
	sleep(50);
	cursorDraw(false);
	sleep(50);
	cursorDraw(true);
	sleep(50);
	cursorDraw(false);
	sleep(50);
	cursorDraw(true);
	sleep(50);
	cursorDraw(false);
	g_rapid_blink = false;
	vTaskDelete(NULL);
}
*/

void nop(void* arg)
{
}

int clamp(int n, int n_min, int n_max)
{
	return max(min(n_max, n), n_min);
}

uint16_t greyscaleColor(uint8_t g)
{
	return tft.color565(g,g,g);
}


class ScrObj {
	public:
		ScrObj(uint16_t w = 0, uint16_t h = 0, bool isSelectable = false):
			_w(w),
			_h(h),
			_isSelectable(isSelectable)
		{
		}

		virtual ~ScrObj()
		{
		}

		virtual void draw() = 0;
		virtual void freeRes() = 0;
		virtual void prepare()
		{
			nop(nullptr);
		}

		virtual void erase()
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			freeRes();
		}

		virtual bool hasInput()
		{
			return false;
		}

		virtual void onClick()
		{
			_callback(_objptr);
		}

		void* returnPtr()
		{
			return _objptr;
		}

		void setCallback(std::function<void(void*)> callback, void* objptr = nullptr)
		{
			_callback = callback;
			_objptr = objptr;
		}


		void setSelectable(bool isSelectable = true)
		{
			_isSelectable = isSelectable;
		}

		bool isVisible()
		{
			return _isVisible;
		}

		void setVisible()
		{
			_isVisible = true;
		}

		void setInvisible()
		{
			_isVisible = false;
		}


		virtual void invalidate()
		{
			_invalid = true;
			freeRes();
		}

		bool isSelectable()
		{
			return _isSelectable;
		}

		bool isPressed()
		{
			return _isPressed;
		}

		bool isSelected()
		{
			return _isSelected;
		}

		void setXYpos(int x, int y)
		{
			_x = x;
			_y = y;
		}

		void setWH(int w, int h)
		{
			_w = w;
			_h = h;
		}

		int getX()
		{
			return _x;
		}

		int getY()
		{
			return _y;
		}

		int getW()
		{
			return _w;
		}

		int getH()
		{
			return _h;
		}

		// set circle cursor
		void setCircle()
		{
			_isSquare = false;
			_isCircle = true;
		}

		bool isCircle()
		{
			return _isCircle;
		}

		bool isSquare()
		{
			return _isSquare;
		}

		virtual uint16_t getCurCol()
		{
			return _curCol;
		}

		virtual void setText(dispStrings_t index)
		{
			if (index > END_OF_STRINGS)
				return;
			_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex)
		{
			_fontIndex = fontIndex;
			_invalid = true;
		}

		virtual int16_t getValue()
		{
			return 0;
		}

		virtual void setValue(int16_t value)
		{
			return;
		}

		bool isInvalid()
		{
			return _invalid;
		}

		void setAlign(align_t align)
		{
			_align = align;
		}

	protected:
		align_t _align = RIGHT;
		dispStrings_t _index = NO_STRING;
		fonts_t _fontIndex = SMALLFONT;
		uint16_t _x = 0;
		uint16_t _y = 0;
		int16_t _w;
		int16_t _h;
		//void (*_callback)(void*) = nop;
		std::function<void(void*)> _callback = nop;
		void* _objptr = nullptr;
		bool _isVisible = true;
		bool _isSelectable;
		bool _isPressed = false;
		bool _isSelected = false;
		bool _invalid = false;
		bool _isSquare = true;
		bool _isCircle = false;
		// cursor erase color
		uint16_t _curCol = 0xffff;
		int16_t _value = 0;
};

// current selected item
ScrObj* currItem = nullptr;
typedef std::vector<ScrObj*> obj_list;


class BlueTextButton: public ScrObj {

	public:
		BlueTextButton(): ScrObj(0, BLUE_BUTTON_HEIGHT, SELECTABLE)
		{
			_setColors();
		}

		virtual void freeRes() override
		{
		}

		virtual void draw() override
		{
			//_h = BLUE_BUTTON_HEIGHT;
			if (!_invalid)
				return;
			tft.setTextColor(_fg, _bg);
			tft.loadFont(FONTS[_fontIndex]);
			_w = tft.textWidth(scrStrings[_index]) + _paddingX*2;
			tft.fillRect(_x, _y, _w, _h, _bg);
			tft.setCursor(_x+_paddingX, _y+_paddingY);
			tft.print(scrStrings[_index]);
			tft.unloadFont();
			_invalid = false;
		}

		//void setColors(uint16_t fg, uint16_t bg)
	private:
		void _setColors()
		{
			_bg = tft.color565(0x61, 0xb4, 0xe4);
			_fg = greyscaleColor(BACKGROUND);
			_invalid = true;
		}

	private:
		uint16_t _fg;
		uint16_t _bg;
		uint8_t _paddingX = BL_BTN_X_PADDING;
		uint8_t _paddingY = BL_BTN_Y_PADDING;
};


class GreyTextButton: public ScrObj {
	public:
		GreyTextButton(): ScrObj(GREY_BUTTON_WIDTH, GREY_BUTTON_HEIGHT, SELECTABLE)
		{
			setColors(greyscaleColor(FONT_COLOR), greyscaleColor(GR_BTN_BG_COLOR));
			setFont(SMALLFONT);
		}

		virtual void freeRes() override
		{
			_btnSpr.deleteSprite();
		}

		virtual void prepare() override
		{
			//_w = GREY_BUTTON_WIDTH;
			//_h = GREY_BUTTON_HEIGHT;
			if (_invalid) {
				_btnSpr.createSprite(_w, _h);
				_btnSpr.fillSprite(_bg);
				_btnSpr.loadFont(FONTS[_fontIndex]);
				_btnSpr.setTextColor(_fg, _bg);
				_btnSpr.setCursor(_paddingX, _paddingY);
				_btnSpr.print(scrStrings[_index]);
				_btnSpr.setCursor(197, _paddingY);
				_btnSpr.print(">");
				_btnSpr.unloadFont();
			}
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;
			_btnSpr.pushSprite(_x, _y);
			_invalid = false;
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
			_invalid = true;
			_isSelectable = true;
		}


	private:
		uint16_t _fg;
		uint16_t _bg;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		TFT_eSprite _btnSpr = TFT_eSprite(&tft);
};


// print text to sprite
class Text: public ScrObj {

	public:
		//TODO: calculate height based on font
		Text(): ScrObj(0, TOP_BAR_HEIGHT - 12, false)
		{
		}

		virtual void freeRes() override
		{
			_txtSp.deleteSprite();
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;
			//freeRes();
			//prepare();
			_txtSp.pushSprite(_x + _dx, _y + _dy, TFT_TRANSPARENT);
			_invalid = false;
			//freeRes();
		}

		/*
		void setTextPadding(uint16_t padding)
		{
			_padding = padding;
		}
		*/

		virtual void prepare() override
		{
			if (!_invalid)
				return;

			//_h = TOP_BAR_HEIGHT - 12;
			//tft.setTextPadding(_padding);
			_txtSp.loadFont(FONTS[_fontIndex]);

			/*
			// TODO: calculate based on longest substring
			// calculate _w based on string wrap
			char* tmp = strtok(const_cast<char*>(scrStrings[_index]), "\n");

			_w = _txtSp.textWidth(tmp) + _paddingX*2;

			// old way:
			*/


			_w = _txtSp.textWidth(scrStrings[_index]) + _paddingX*2;

			if (_w > SCR_WIDTH)
				_w = SCR_WIDTH;

			_txtSp.createSprite(_w, _h);
			_txtSp.fillSprite(TFT_TRANSPARENT);
			_txtSp.setTextColor(_fg, _bg);
			//_txtSp.print(_text);
			_txtSp.print(scrStrings[_index]);
			_txtSp.unloadFont();
		}

		virtual void erase() override
		{
			tft.fillRect(_x + _dx, _y + _dy, _w, _h, _bg);
			freeRes();
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			// set colors
			_fg = fg;
			_bg = bg;
			_txtSp.setColorDepth(16);
		}

		virtual void setText(dispStrings_t index) override
		{
			//_padding = tft.getTextPadding();

			if (index > END_OF_STRINGS)
				return;

			_index = index;
			_invalid = true;

			int mult = 1;

			// calculate sprite height based on newline occurrences
			for (int i = 0; scrStrings[_index][i]; i++) {
				if (scrStrings[_index][i] == '\n') {
					mult++;
				}
			}
			_h *= mult;
		}

		dispStrings_t getStrIndex()
		{
			return _index;
		}

		fonts_t getFontIndex()
		{
			return _fontIndex;
		}

		uint8_t getYpadding()
		{
			return _paddingY;
		}

		uint8_t getXpadding()
		{
			return _paddingX;
		}

		TFT_eSprite* getSpritePtr()
		{
			return &_txtSp;
		}

		uint8_t getPaddingX()
		{
			return _paddingX;
		}

		uint16_t getFg()
		{
			return _fg;
		}

		uint16_t getBg()
		{
			return _bg;
		}

		void adjustX(int8_t x)
		{
			_dx = x;
		}

		void adjustY(int8_t y)
		{
			_dy = y;
		}

	private:
		//uint16_t _padding;
		uint16_t _fg = FONT_COL_565;
		uint16_t _bg = TFT_WHITE;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		int8_t _dx = 0;
		int8_t _dy = 0;
		TFT_eSprite _txtSp = TFT_eSprite(&tft);
};

// global text from wifiSettPage
Text gwsConnection;

// text that doesn't have strings in static memory
class StringText: public Text {
		using Text::Text;
	public:
		StringText()
		{
		}

		~StringText()
		{
			freeRes();
		}

		virtual void prepare() override
		{
			if (!_invalid)
				return;

			auto txtSp = this->getSpritePtr();
			auto paddingX = this->getPaddingX();
			auto bg = this->getBg();
			auto fg = this->getFg();
			//auto h = this->getH();

			//_h = TOP_BAR_HEIGHT - 12;
			//tft.setTextPadding(_padding);
			txtSp->loadFont(FONTS[_fontIndex]);

			/*
			// TODO: calculate based on longest substring
			// calculate _w based on string wrap
			char* tmp = strtok(const_cast<char*>(scrStrings[_index]), "\n");

			_w = _txtSp.textWidth(tmp) + _paddingX*2;

			// old way:
			*/


			_w = txtSp->textWidth(*_txt) + paddingX*2;

			if (_w > SCR_WIDTH)
				_w = SCR_WIDTH;

			txtSp->createSprite(_w, _h);
			txtSp->fillSprite(TFT_TRANSPARENT);
			txtSp->setTextColor(fg, bg);
			//_txtSp.print(_text);
			txtSp->print(*_txt);
			txtSp->unloadFont();

		}

		void setText(String& txt)
		{
			_txt = &txt;
		}
	private:
		String* _txt;

};

class BodyText: public ScrObj {

	public:
		BodyText(): ScrObj(0, TOP_BAR_HEIGHT - 12)
		{
		}

		virtual void freeRes() override
		{
			_txtSp.deleteSprite();
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;
			_txtSp.pushSprite(_x, _y, TFT_TRANSPARENT);
			_invalid = false;
		}

		virtual void prepare() override
		{
			//_h = TOP_BAR_HEIGHT - 12;
			if (_invalid) {
				_txtSp.loadFont(FONTS[_fontIndex]);
				_w = _txtSp.textWidth(scrStrings[_index]) + _paddingX*2;
				_txtSp.createSprite(_w, _h);
				_txtSp.fillSprite(TFT_TRANSPARENT);
				_txtSp.setTextColor(_fg, _bg);
				//_txtSp.print(_text);
				_txtSp.print(scrStrings[_index]);
				_txtSp.unloadFont();
			}
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			// set colors
			_fg = fg;
			_bg = bg;
			_txtSp.setColorDepth(16);
		}

	private:
		uint16_t _fg, _bg;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		TFT_eSprite _txtSp = TFT_eSprite(&tft);
};


class Image: public ScrObj {

	public:
		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			reload();

			if (_jpegFile) {
				ui.drawJpeg(_jpegFile, _x, _y);
				_w = JpegDec.width;
				_h = JpegDec.height;
				_invalid = false;
			}
		}

		void reload()
		{
			_jpegFile = SPIFFS.open(_filename, "r");
		}

		void loadRes(const String& filename)
		{
			//_jpegFile = SPIFFS.open(filename, "r");
			_invalid = true;
			//_isSelectable = true;
			_filename = filename;
		}

		virtual void freeRes() override
		{
			if (_jpegFile)
				_jpegFile.close();
		}

	private:
		fs::File _jpegFile;
		String _filename;
};

#define IMG_BTN_SIZE 30

class ImageButton: public ScrObj {

	public:
		ImageButton(): ScrObj(IMG_BTN_SIZE, IMG_BTN_SIZE, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			if (_invalid)
				reload();

			if (_invalid && _jpegFile) {
				ui.drawJpeg(_jpegFile, _x, _y);
				_invalid = false;
			}
		}

		virtual void erase() override
		{

		}

		void reload()
		{
			_jpegFile = SPIFFS.open(_filename, "r");
		}

		void loadRes(const String& filename)
		{
			//_jpegFile = SPIFFS.open(filename, "r");
			//_w = _h = 30;
			_invalid = true;
			//_isSelectable = true;
			_filename = filename;
		}

		virtual void freeRes() override
		{
			if (_jpegFile)
				_jpegFile.close();
		}

	private:
		fs::File _jpegFile;
		String _filename;
};

class SimpleBox: public ScrObj {
	public:
		virtual void draw() override
		{
			if (_invalid) {
				tft.fillRect(_x, _y, _w, _h, _col);
				_invalid = false;
			}
		}

		virtual void freeRes() override
		{
		}

		void setColor(uint16_t col)
		{
			_col = col;
		}


	private:
		uint16_t _col = 0;
};


typedef enum {
	ONE_CHR,
	TWO_CHR,
	THREE_CHR,
	FOUR_CHR
} placeholder_t;

const char* placeholder[] = {
	"0",
	"00",
	"000",
	"0000"
};

class InputField: public ScrObj {
	public:
		InputField(): ScrObj(0, INPUT_H, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			_text.draw();

			tft.setTextColor(_fg, _bg);
			tft.loadFont(FONTS[_fontIndex]);
			//_w = tft.textWidth(String(_value)) + _paddingX*2;
			tft.fillRect(_x, _y, _w, _h, _bg);

			if (_showPlus) {
				tft.setCursor(_x + _dx, _y+_paddingY);
			}
			else {
				tft.setCursor(_x + _dx +_paddingX, _y+_paddingY);
			}

			String tmp = String(_value);

			// leading zeros
			if (_showLead && _value < 10) {
				tmp = "0" + tmp;
			}

			// draw positive
			if (_showPlus && _value >= 0) {
				tmp = "+" + tmp;
			}


			tft.print(tmp);
			//tft.print(_value);

			/*
			tft.setCursor(_x+_w+_paddingX, _y+_paddingY);
			tft.setTextColor(_fg, TFT_WHITE);
			_textw = tft.textWidth(scrStrings[_index]) + _paddingX*2;
			tft.print(scrStrings[_index]);
			*/
			tft.unloadFont();

			_invalid = false;
		}



		virtual void freeRes() override
		{
		}

		void setWidth(placeholder_t width)
		{
			_width = width;
		}

		void adjustX(int8_t x)
		{
			_dx = x;
		}

		/*
		void setXpadding(uint8_t padding)
		{
			_paddingX = padding;
		}
		*/

		void adjustTextX(int8_t x)
		{
			_text.adjustX(x);
		}

		void adjustTextY(int8_t y)
		{
			_text.adjustY(y);
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, TFT_WHITE);
			_text.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			_w = tft.textWidth(placeholder[_width]) + _paddingX*2;

			//adjust width
			_w += _dw;

			// calculate text position
			switch (_align) {
				default:
				case RIGHT:
					_text.setXYpos(_x + _w + _text.getXpadding(),
							_y + _dy + int(_h/2) - int(_text.getH()/2));
							//+  _text.getYpadding()/2);
					break;
				case LEFT:
					_text.setXYpos(_x - _textLength - _text.getXpadding(),
							_y + _dy + int(_h/2) - int(_text.getH()/2));
							//+ _text.getYpadding()/2);
					break;
				case TOP:
					// TODO: calculate this. Currently same as RIGHT
					_text.setXYpos(_x + _w + _text.getXpadding(),
							_y + _dy + _text.getYpadding()/2);
					break;
			}


			/*
			_text.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
					*/

			_text.invalidate();
			_text.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;

			// get length in current font
			tft.loadFont(FONTS[_text.getFontIndex()]);

			// only needed for LEFT align
			//_textLength = tft.textWidth(scrStrings[_text.getStrIndex()]);
			_textLength = tft.textWidth(scrStrings[index]);

			// prevent text going out of screen
			if (_textLength > SCR_WIDTH) {
				_textLength = 0;
			}

			tft.unloadFont();

			_text.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_fontIndex = fontIndex;
			_text.setFont(fontIndex);
			_invalid = true;
		}


		virtual bool hasInput() override
		{
			return true;
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
			_invalid = true;
		}

		virtual void setValue(int16_t value) override
		{
			if (value > _upper)
				value = _lower;
			if (value < _lower)
				value = _upper;
			_invalid = true;
			_value = value;
		}

		virtual int16_t getValue() override
		{
			return _value;
		}

		void setLimits(int16_t lower, int16_t upper)
		{
			_upper = upper;
			_lower = lower;
		}

		void showPlus(bool show)
		{
			_showPlus = show;
		}

		void showLeadZero(bool showLead = true)
		{
			_showLead = showLead;
		}

		void adjustWidth(int8_t width)
		{
			//_w += width;
			_dw = width;
		}

	private:
		Text _text;
		int _textLength = 0;
		bool _showLead = false;
		bool _showPlus = false;
		uint16_t _fg = FONT_COL_565;
		uint16_t _bg = COL_GREY_E3_565;
		int8_t _dy = 0;
		int8_t _dx = 0;
		int8_t _dw = 0;
		//uint16_t _textw;
		int16_t _upper = 100;
		int16_t _lower = 0;
		uint8_t _paddingX = IN_FLD_X_PADDING;
		uint8_t _paddingY = IN_FLD_Y_PADDING;
		placeholder_t _width = THREE_CHR;
};

InputField gBrightness;

//TODO: manually decode JPG, push array to sprite
TFT_eSprite checkSprite = TFT_eSprite(&tft);

class CheckBox: public ScrObj {
	public:
		CheckBox(): ScrObj(CHK_BOX_SIZE, CHK_BOX_SIZE, SELECTABLE)
		{
			reload();
		}

		~CheckBox()
		{
			freeRes();
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			reload();

			_text.draw();

			if (!_isOn) {
				tft.fillRect(_x, _y, _w, _h, greyscaleColor(CHK_BOX_COL));
				_invalid = false;
			}
			else if (_jpegFile) {
				ui.drawJpeg(_jpegFile, _x, _y);
				_invalid = false;
			}
		}

		virtual void freeRes() override
		{
			if (_jpegFile)
				_jpegFile.close();
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			_text.erase();
			freeRes();
		}

		void adjustTextY(int8_t dy)
		{
			_dy = dy;
		}

		virtual void prepare() override
		{
			// get length in current font
			tft.loadFont(FONTS[_text.getFontIndex()]);

			int textLength = tft.textWidth(scrStrings[_text.getStrIndex()]);

			if (textLength > SCR_WIDTH) {
				textLength = 0;
			}

			// calculate text position
			switch (_align) {
				default:
				case RIGHT:
					_text.setXYpos(_x + _w + _text.getXpadding(),
							_y + _dy +  _text.getYpadding()/2);
					break;
				case LEFT:
					_text.setXYpos(_x - textLength - _text.getXpadding(),
							_y + _dy + _text.getYpadding()/2);
					break;
				case TOP:
					// TODO: calculate this. Currently same as RIGHT
					_text.setXYpos(_x + _w + _text.getXpadding(),
							_y + _dy + _text.getYpadding()/2);
					break;
			}

			tft.unloadFont();

			_text.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_text.invalidate();
			_text.prepare();
		}


		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_text.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_text.setFont(fontIndex);
			_invalid = true;
		}


		bool isOn()
		{
			return _isOn;
		}

		void on(bool isOn)
		{
			_isOn = isOn;
		}

		void reload()
		{
			_jpegFile = SPIFFS.open(_filename, "r");
		}

	private:
		Text _text;
		uint16_t _bg;
		int8_t _dy = 0;
		//bool _textAligned = false;
		bool _isOn = false;
		//TFT_eSprite* spr;
		fs::File _jpegFile;
		const char* _filename = CHK_BOX_FILE;
};

// wifiSettPage global items
CheckBox gwsWifiChBox;

#define TGL_BG 0xDC
#define TGL_RAD 10
#define TGL_W 33
#define TGL_H 17
//#define TGL_ON_COL
#define TGL_OFF_COL 0x6E
#define TGL_SHF_RAD 6

// TODO: replace with antialiased image
class Toggle: public ScrObj {
	public:
		Toggle(): ScrObj(TGL_W, TGL_H, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			tft.fillRoundRect(_x, _y, _w, _h, TGL_RAD, greyscaleColor(TGL_BG));

			_text.draw();

			if (_isOn) {
				// TODO: draw on image
				_col = tft.color565(0x4C, 0xAF, 0x50);
				_shaftX = _x + _w - TGL_RAD;
				_shaftY = _y + _h/2;
				_invalid = false;
			}
			else {
				// TODO: draw off image
				_col = greyscaleColor(TGL_OFF_COL);
				_shaftX = _x + TGL_RAD - 1;
				_shaftY = _y + _h/2;
				_invalid = false;
			}

			tft.fillCircle(_shaftX, _shaftY, TGL_SHF_RAD, _col);
		}

		virtual void freeRes() override
		{
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			_text.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			_text.setXYpos(_x + _w + _text.getXpadding(), _y + _text.getYpadding()/2);
			_text.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_text.invalidate();
			_text.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_text.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_text.setFont(fontIndex);
			_invalid = true;
		}

		bool isOn()
		{
			return _isOn;
		}

		void on(bool isOn)
		{
			_isOn = isOn;
		}

	private:
		//dispStrings_t _index;
		Text _text;
		//fonts_t _fontIndex;
		uint16_t _bg, _fg, _col, _shaftX, _shaftY;
		//bool _textAligned = false;
		bool _isOn = false;
};

#define RAD_BG_COL 0xE3

// TODO: replace with antialiased image
class CircRadBtn: public ScrObj {
	public:
		CircRadBtn(): ScrObj(RAD_BTN_SIZE, RAD_BTN_SIZE, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			tft.fillRect(_x, _y, _w, _h, greyscaleColor(_bgcol));

			if (_isOn)
				//#4CAF50 - checked green
				_col = tft.color565(0x4C, 0xAF, 0x50);
			else
				_col = 0xFFFF;

			int x = _x + _w/2 - 1;
			int y = _y + _h/2 - 1;
			tft.fillCircle(x, y, _r, _col);
			_invalid = false;
		}

		virtual void freeRes() override
		{
		}

		/*
		virtual void onClick() override
		{
			if (!_isOn)
				_callback();
		}
		*/


		bool isOn()
		{
			return _isOn;
		}

		// set cursor erase color
		void setCurCol(uint16_t col)
		{
			_curCol = col;
		}

		/*
		virtual uint16_t getCurCol() override
		{
			return greyscaleColor(GR_BTN_BG_COLOR);
		}
		*/
		void setBgColor(uint8_t bgcol)
		{
			_bgcol = bgcol;
		}

		void on(bool isOn)
		{
			_isOn = isOn;
		}

	private:
		bool _isOn = false;
		int _r = 5;
		uint16_t _col = 0xffff;
		uint8_t _bgcol = RAD_BG_COL;
};

class ExclusiveRadio: public CircRadBtn {
	public:
		virtual void onClick() override
		{
			if (!this->isOn()) {
				_callback(nullptr);
			}
		}
};

class TestPageRadio: public ScrObj {
	public:
		TestPageRadio(): ScrObj(RAD_BTN_SIZE, RAD_BTN_SIZE, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			tft.fillRect(_x, _y, _w, _h, greyscaleColor(_bgcol));

			_text.draw();

			if (_isOn)
				//#4CAF50 - checked green
				_col = tft.color565(0x4C, 0xAF, 0x50);
			else
				_col = 0xFFFF;

			int x = _x + _w/2 - 1;
			int y = _y + _h/2 - 1;
			tft.fillCircle(x, y, _r, _col);
			_invalid = false;
		}

		virtual void freeRes() override
		{
		}

		/*
		virtual void onClick() override
		{
			if (!_isOn)
				_callback();
		}
		*/

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			_text.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			_text.setXYpos(_x + _w + _text.getXpadding(), _y + _text.getYpadding()/2);
			_text.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_text.invalidate();
			_text.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_text.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_text.setFont(fontIndex);
			_invalid = true;
		}


		bool isOn()
		{
			return _isOn;
		}

		// set cursor erase color
		void setCurCol(uint16_t col)
		{
			_curCol = col;
		}

		/*
		virtual uint16_t getCurCol() override
		{
			return greyscaleColor(GR_BTN_BG_COLOR);
		}
		*/
		void setBgColor(uint8_t bgcol)
		{
			_bgcol = bgcol;
		}

		void on(bool isOn)
		{
			_isOn = isOn;
		}

	private:
		Text _text;
		bool _isOn = false;
		int _r = 5;
		uint16_t _col = 0xffff;
		uint8_t _bgcol = RAD_BG_COL;
};

#define WAIT_RECT_SIZ 10
#define WAIT_WDTH 75
#define WAIT_DARK 0x8F
#define WAIT_LIGHT 0xE3
#define DRAW_INTERVAL 333

class Wait: public ScrObj {
	public:
		Wait(): ScrObj(WAIT_WDTH, WAIT_RECT_SIZ)
		{
			setFont(SMALLESTFONT);
			setText(WAIT_TEXT);
		}

		void setInterval(unsigned long interval)
		{
			_interval = interval;
		}

		virtual void freeRes() override
		{
			//_waitText.freeRes();
		}

		virtual void draw() override
		{
			if (_invalid)
				_waitText.draw();

			//if (!_invalid)
				//return;

			if (millis() - _timestamp < _interval)
				return;
			else
				_timestamp = millis();

			if (_darkSquare > _squares)
				_darkSquare = 0;

			uint8_t gap = 0;
			for (int i = 0; i < _squares; i++) {
				uint16_t color = 0;
				if (i == _darkSquare)
					color = greyscaleColor(WAIT_DARK);
				else
					color = greyscaleColor(WAIT_LIGHT);
				tft.fillRect(_x + (_h+gap)*i, _y, _h, _h, color);
				gap = _gap;
			}
			_darkSquare++;
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			_waitText.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			_waitText.setXYpos(_x + _waitText.getXpadding(), _y - _waitText.getYpadding()*2);
			_waitText.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_waitText.invalidate();
			_waitText.prepare();

		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_waitText.setText(index);
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_waitText.setFont(fontIndex);
			_invalid = true;
		}

	private:
		uint16_t _dark = WAIT_DARK;
		uint16_t _light = WAIT_LIGHT;
		uint8_t _darkSquare = 0;
		uint8_t _gap = 3;
		uint8_t _squares = 6;
		Text _waitText;
		unsigned long _timestamp = 0;
		unsigned long _interval = DRAW_INTERVAL;
};

class Cursor {
	public:
		void draw(bool blink)
		{
			if (!currItem)
				return;
			_setCoord(currItem);

			if (blink)
				_draw();
			else
				_erase();
		}

	private:
		void _draw()
		{
			if (_isCircle) {
				tft.drawCircle(
						_x,
						_y,
						_h/2 + 1,
						greyscaleColor(CURCOL)
					      );

			}
			else {
				tft.drawRect(
						_x,
						_y,
						_w,
						_h,
						greyscaleColor(CURCOL)
					    );
			}
		}

		void _erase()
		{
			if (_isCircle) {
				tft.drawCircle(
						_x,
						_y,
						_h/2 + 1,
						currItem->getCurCol()
					      );

			}
			else {
				tft.drawRect(
						_x,
						_y,
						_w,
						_h,
						currItem->getCurCol()
					    );
			}

		}

		void _setCoord(ScrObj* obj)
		{
			if (!obj)
				return;

			if (obj->isCircle()) {
				_isCircle = true;
				int x = currItem->getX();
				int y = currItem->getY();
				_h = currItem->getH();
				_w = currItem->getW();
				_x = x+(_w/2) - 1;
				_y = y+(_h/2) - 1;
			}
			else {
				_isCircle = false;
				_x = currItem->getX() - 1;
				_y = currItem->getY() - 1;
				_w = currItem->getW() + 1;
				_h = currItem->getH() + 1;
			}
		}

	private:
		bool _isCircle;
		int _x;
		int _y;
		int _w;
		int _h;
};

class Page {
	public:
		void addItem(ScrObj* scrobj)
		{
			_items.push_back(scrobj);
			if (scrobj->isSelectable()) {
				_selectable.push_back(scrobj);
			}
		}

		void restock()
		{
			_selectable.clear();
			for(auto& i:_items) {
				if (i->isSelectable()) {
					_selectable.push_back(i);
				}
			}
		}

		void draw()
		{
			for (auto& obj:_items)
				obj->draw();
		}

		// set all screen objects for redraw
		void invalidateAll()
		{
			for (auto& obj:_items)
				obj->invalidate();
		}

		void erase()
		{
			//tft.fillRect(0, 28, 240, 284 - 28, greyscaleColor(BACKGROUND));
			for (auto& obj:_items)
				obj->erase();
		}

		void prepare()
		{
			for (auto& obj:_items)
				obj->prepare();
		}

		ScrObj* getCurrItem()
		{
			return _currItem;
		}

		ScrObj* getCurrItemAt(size_t i)
		{
			return _selectable.at(i);
		}

		size_t nItems()
		{
			return _selectable.size();
		}

		void setCurrItem(size_t i)
		{
			if (i < _selectable.size())
				_currItem = _selectable.at(i);
		}

		size_t selSize()
		{
			return _selectable.size();
		}

		void setTitle(dispStrings_t index)
		{
			if (index > END_OF_STRINGS)
				return;
			_title = index;
		}

		dispStrings_t getTitle()
		{
			return _title;
		}

		Page* prev()
		{
			return _prev;
		}

		void setPrev(Page* page)
		{
			_prev = page;
		}

		bool visibleIcons()
		{
			return _iconsVisible;
		}

		void setIconsVis(bool vis)
		{
			_iconsVisible = vis;
		}


	private:
		bool _iconsVisible = true;
		Page* _prev = nullptr;
		dispStrings_t _title;
		obj_list _items;
		obj_list _selectable;
		ScrObj* _currItem;
};

typedef enum {
	MENU_PG,
	SETT_PG,
	LANG_PG,
	FONT_PG,
	TEST_PG,
	WIFI_PG,
	WIFI_SETT_PG,
	TIME_PG,
	CAL_SETT_PG,
	CAL_PH1_PG,
	NPAGES
} pages_t;

Page* pages[NPAGES];
Page* currPage;


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
			getI2Ctime();
			if (_sync) {
				syncNTP();
			}
		}

	private:
		InputField _visible[N_DATETIME_VISIBLE];
		Text _fieldsTitle;
		bool _sync = false;
		int8_t _utc = 0;
		struct tm  _timeinfo;
		unsigned long _oldMils = 0;
		const char* const _nptServer = "pool.ntp.org";
		//uint16_t _y = 225;
		//TFT_eSprite _sprite = TFT_eSprite(&tft);
	public:

		void update()
		{
			if (millis() - _oldMils > RTC_CHECK_INTERVAL) {
				_oldMils = millis();

				if (_sync) {
					syncNTP();
				}
				else {
					getI2Ctime();
				}

				invalidate();
				prepare();
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
			prepare();
		}

		int8_t getUTC()
		{
			return _utc;
		}

		void initUTC(int8_t utc)
		{
			_utc = utc;
		}

		void setUTC(void* obj)
		{
			if (obj == nullptr)
				return;

			InputField* utc = (InputField*) obj;

			_utc = utc->getValue();

			// TODO: postpone or move sync to different task
			if (this->_sync) {
				syncNTP();
			}

			prepare();
			invalidate();
			/*
			_visible[HOUR].invalidate();
			_visible[HOUR].prepare();
			_visible[HOUR].draw();
			*/
			/*
			else {
				_visible[HOUR].setValue(_visible[HOUR].getValue() + _utc);
				setHours(nullptr);
			}
			*/
		}

		void getI2Ctime()
		{
			time_t time;
			time = rtc.gettimeUnix();
			struct tm *tmp = localtime(&time);
			_timeinfo = *tmp;
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
			rtc.settimeUnix(mktime(&_timeinfo));
		}

		void syncNTP()
		{
			if (!g_ping_success) {
				//setI2Ctime();
				return;
			}

			configTime(3600*_utc, 0, _nptServer);
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

class App {
	private:
		unsigned long _oldMils = 0;
		unsigned long _dbMils = 0;
		unsigned long _dimMils = 0;
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
			switch (g_selected_lang) {
				default:
				case RU_LANG: scrStrings = ruStrings; break;
				case EN_LANG: scrStrings = engStrings; break;
			}
				//rtc.begin();
			g_ping_success = false;
			//SPIFFS.begin();
			tft.init();
			tft.initDMA(true);
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
			uint8_t user_input = buttons.portRead(0);

			// debounce
			if ((uint8_t)~user_input) {
				if (millis() - _dbMils > DEBOUNCE) {
					user_input = buttons.portRead(0);
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
				currItem = currPage->getCurrItemAt(_iterator);
				_cursor.draw(true);
				_dbFlag = false;
			}
			//else if (!digitalRead(BTN_NEXT)) {
			else if (~user_input & BTN_NEXT) {
				_cursor.draw(false);
				_iterator++;
				if (_iterator > currPage->selSize() - 1)
					_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
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

			// don't blink if buttons were pressed...
			_oldMils = millis();
		}
};

#endif
