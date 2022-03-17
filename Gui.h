//TODO: page container, generic call procedure
//TODO: page builder
//TODO: banish global pointers
//
//
// precalculate all colors to uint16_t
//
// Obj:
// progress bar -
// input field -
// checkbox -
// switch -
// wifi -
// internet -
//

#include <vector>
#include <atomic>

// hardware... stuff
#include <SPI.h>
#include <TFT_eSPI.h>
//#include <iarduino_RTC.h>


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

// debounce stuff
#define TIMER 500
#define DEBOUNCE 200

// checkbox
#define CHK_BOX_COL 0xDC
#define CHK_BOX_SIZE 21
#define CHK_BOX_FILE "/check.jpg"

// input field height
#define INPUT_H 24

// radio button
#define RAD_BTN_SIZE 22


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

#ifdef TASKS
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

#endif

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
			_callback(_page_ptr);
		}


		void setCallback(void(*callback)(void*), void* page_ptr)
		{
			_callback = callback;
			_page_ptr = page_ptr;
		}

		void setCallback(void(*callback)(void*))
		{
			_callback = callback;
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


		void invalidate()
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
		dispStrings_t _index;
		fonts_t _fontIndex;
		uint16_t _x = 0;
		uint16_t _y = 0;
		int16_t _w;
		int16_t _h;
		void (*_callback)(void*) = nop;
		void* _page_ptr = nullptr;
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
			_txtSp.pushSprite(_x, _y, TFT_TRANSPARENT);
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
			tft.fillRect(_x, _y, _w, _h, _bg);
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

	private:
		//uint16_t _padding;
		uint16_t _fg, _bg;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
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
#ifdef APP_DEBUG
				Serial.print("jpeg width: ");
				Serial.println(JpegDec.width);
#endif
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


class InputField: public ScrObj {
	public:
		InputField(): ScrObj(0, INPUT_H, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			tft.setTextColor(_fg, _bg);
			tft.loadFont(FONTS[_fontIndex]);
			//_w = tft.textWidth(String(_value)) + _paddingX*2;
			_w = tft.textWidth("000") + _paddingX*2;
			tft.fillRect(_x, _y, _w, _h, _bg);
			tft.setCursor(_x+_paddingX, _y+_paddingY);
			tft.print(_value);
			tft.setCursor(_x+_w+_paddingX, _y+_paddingY);
			tft.setTextColor(_fg, TFT_WHITE);
			_textw = tft.textWidth(scrStrings[_index]) + _paddingX*2;
			tft.print(scrStrings[_index]);
			tft.unloadFont();
			_invalid = false;
		}

		virtual void freeRes() override
		{
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, TFT_WHITE);
			tft.fillRect(
					_x+_w+_paddingX,
					_y,
					_textw,
					_h,
					TFT_WHITE
					);
			freeRes();
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

		void setLimits(uint8_t upper, uint8_t lower)
		{
			_upper = upper;
			_lower = lower;
		}

	private:
		uint16_t _fg, _bg, _textw;
		uint8_t _upper = 100;
		uint8_t _lower = 0;
		uint8_t _paddingX = BL_BTN_X_PADDING;
		uint8_t _paddingY = BL_BTN_Y_PADDING;
};

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

			_tglText.draw();

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
			_tglText.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			// get length in current font
			tft.loadFont(FONTS[_tglText.getFontIndex()]);

			int textLength = tft.textWidth(scrStrings[_index]);

			// calculate text position
			switch (_align) {
				case RIGHT:
					_tglText.setXYpos(_x + _w + _tglText.getXpadding(),
							_y + _tglText.getYpadding()/2);
					break;
				case LEFT:
					_tglText.setXYpos(_x - textLength - _tglText.getXpadding(),
							_y + _tglText.getYpadding()/2);
					break;
				case TOP:
					// TODO: calculate this. Currently same as RIGHT
					_tglText.setXYpos(_x + _w + _tglText.getXpadding(),
							_y + _tglText.getYpadding()/2);
					break;
			}

			tft.unloadFont();

			_tglText.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_tglText.invalidate();
			_tglText.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_tglText.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_tglText.setFont(fontIndex);
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
		Text _tglText;
		uint16_t _bg;
		bool _textAligned = false;
		bool _isOn = false;
		//TFT_eSprite* spr;
		fs::File _jpegFile;
		const char* _filename = CHK_BOX_FILE;
};

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

			_tglText.draw();

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
			_tglText.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			_tglText.setXYpos(_x + _w + _tglText.getXpadding(), _y + _tglText.getYpadding()/2);
			_tglText.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_tglText.invalidate();
			_tglText.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_tglText.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_tglText.setFont(fontIndex);
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
		Text _tglText;
		//fonts_t _fontIndex;
		uint16_t _bg, _fg, _col, _shaftX, _shaftY;
		bool _textAligned = false;
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

			_tglText.draw();

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
			_tglText.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			_tglText.setXYpos(_x + _w + _tglText.getXpadding(), _y + _tglText.getYpadding()/2);
			_tglText.setColors(
					greyscaleColor(FONT_COLOR),
					greyscaleColor(BACKGROUND)
					);
			_tglText.invalidate();
			_tglText.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS)
				return;
			_tglText.setText(index);
			//_index = index;
			_invalid = true;
		}

		virtual void setFont(fonts_t fontIndex) override
		{
			if (fontIndex > END_OF_FONTS)
				return;
			_tglText.setFont(fontIndex);
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
		Text _tglText;
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
	NPAGES
} pages_t;

Page* pages[NPAGES];
Page* currPage;


#define WIFI_UPDATE_INTERVAL 500
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
			if (!g_wifi_set)
				server.handleClient();

			if (millis() - _timestamp < _interval)
				return;

			_timestamp = millis();

			int dBm = WiFi.RSSI();
			uint8_t strength = map(dBm, -95, -45, 0, 4);

			strength = clamp(strength, 0, 4);

			if (WiFi.status() != WL_CONNECTED) {
				_curWiFiImage = IMG_NO_WIFI;
				_curNetImage = IMG_NET_NO;
				g_ping_success = false;
			}
			else {
				if (g_ping_success) {
					_curNetImage = IMG_NET_OK;

				}
				else {
					_curNetImage = IMG_NET_NO;
				}

				//client.stop();

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
	private:
		unsigned long _timestamp = 0;
		unsigned long _interval = WIFI_UPDATE_INTERVAL;
		images_t _curWiFiImage = IMG_NO_WIFI;
		images_t _curNetImage = IMG_NET_NO;
		images_t _prevWiFiImage = IMG_NO_WIFI;
		images_t _prevNetImage = IMG_NET_NO;
} topBar;

class Builder {
	public:
		void buildTimePage()
		{
		}
};


#define INPUT_READ (!digitalRead(BTN_PREV) || !digitalRead(BTN_NEXT) || !digitalRead(BTN_OK) || !digitalRead(BTN_PLU) || !digitalRead(BTN_MIN))
class App {
	private:
		unsigned long _oldMils = 0;
		unsigned long _dbMils = 0;
		bool _blink = false;
		bool _dbFlag = false;
		Cursor _cursor;
		int _iterator = 0;

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
			//rtc.begin();
			tft.initDMA(true);
			g_ping_success = false;
			//SPIFFS.begin();
			tft.init();
			tft.setRotation(2);
			tft.fillScreen(greyscaleColor(BACKGROUND));
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
#define DEBUG_TIMER 1000
			static unsigned long debugMils = 0;
			if (millis() - debugMils > DEBUG_TIMER) {
				debugMils = millis();
				Serial.print("Free heap: ");
				Serial.println(ESP.getFreeHeap());
			}
#endif

			// cursor blink
			if (millis() - _oldMils > TIMER) {
				_cursor.draw(_blink);
				_oldMils = millis();
				_blink = !_blink;
			}

			// debounce
			if (INPUT_READ) {
				if (millis() - _dbMils > DEBOUNCE) {
					if (INPUT_READ){
						_dbMils = millis();
						_dbFlag = true;
					}
				}
			}

			// input proc
			if (!_dbFlag)
				return;

			if (!digitalRead(BTN_PREV)) {
				_cursor.draw(false);
				_iterator--;
				if (_iterator < 0)
					_iterator = currPage->selSize() - 1;
				currItem = currPage->getCurrItemAt(_iterator);
				_cursor.draw(true);
				_dbFlag = false;
			}
			else if (!digitalRead(BTN_NEXT)) {
				_cursor.draw(false);
				_iterator++;
				if (_iterator > currPage->selSize() - 1)
					_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
				_cursor.draw(true);
				_dbFlag = false;
			}
			else if (!digitalRead(BTN_MIN)) {
				currItem->setValue(currItem->getValue() - 1);
				if (currItem->hasInput())
					currItem->onClick();
				currItem->draw();
				_dbFlag = false;
			}
			else if (!digitalRead(BTN_PLU)) {
				currItem->setValue(currItem->getValue() + 1);
				if (currItem->hasInput())
					currItem->onClick();
				currItem->draw();
				_dbFlag = false;
			}
			else if (!digitalRead(BTN_OK)) {
				_cursor.draw(false);
				currItem->onClick();
				if (_iterator >= currPage->nItems())
					_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
				_dbFlag = false;
			}

			_oldMils = millis();
		}
};
