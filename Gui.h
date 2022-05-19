/******************************************************************************

  GUI elements classes.
  setXYpos method should always be called first while building ScrOjbs.

  ****************************************************************************/

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
#define COLOR_DEPTH 16

#include <vector>	// keep page items in vector
#include <iostream>
//#include <string>	// calculate _w based on longest sting

#include "images.h"
#include "settings.h"	// settings enum

/***************************** defines *************************/
// gap between menu items
#define MENU_GAP 5

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
#define LEFTMOST 17
#define TOPMOST 11

// test
#define TEXT_HEIGHT 15

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

#define COL_GREY_DC 0xDC
/* calculated colors to 565 */
// default font color precalculated from 0x70
#define FONT_COL_565 0x738E
#define COL_GREY_70_565 0x738E
#define COL_GREY_E3_565 0xE71C
#define COL_GREY_DC_565 0xDEFB

// tank red
#define COL_RED_TANK_565 0xE3CF

// checkbox
#define CHK_BOX_COL 0xDC
#define CHK_BOX_SIZE 22
#define CHK_BOX_FILE "/check.jpg"

// input field height
#define INPUT_H 22

// radio button
#define RAD_BTN_SIZE 22

// lang settings after typedef
typedef enum {
	RU_LANG,
	EN_LANG
} lang_t;

lang_t g_selected_lang = RU_LANG;

// pointer to current language strings
const char** scrStrings = ruStrings;



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

// objs
TFT_eSPI tft = TFT_eSPI();

GfxUi ui = GfxUi(&tft);

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

		virtual void setCallback(std::function<void(void*)> callback, void* objptr = nullptr)
		{
			_callback = callback;
			_objptr = objptr;
		}


		void setSelectable(bool isSelectable = true)
		{
			_isSelectable = isSelectable;
		}

		virtual bool isVisible()
		{
			return _isVisible;
		}

		// TODO: deal with _isSelectable on not selectable obj
		void setVisible()
		{
			/*
			if (_wasSelectable) {
				_isSelectable = true;
			}
			*/
			_isVisible = true;
			_invalid = true;
		}

		void setInvisible()
		{
			//_isSelectable = false;
			_invalid = true;
			if (_neverHide)
				return;
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

		virtual int getValue()
		{
			return 0;
		}

		virtual void setValue(int value)
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

		void neverHide()
		{
			_neverHide = true;
		}

		bool canBeHidden()
		{
			return !_neverHide;
		}

		rig_settings_t getSettingsId()
		{
			return _sett_id;
		}

		void setSettingsId(rig_settings_t id)
		{
			_sett_id = id;
		}

		virtual void add()
		{
		}

		virtual void sub()
		{
		}

	protected:
		align_t _align = RIGHT;
		dispStrings_t _index = NO_STRING;
		fonts_t _fontIndex = SMALLFONT;
		uint16_t _x = 0;
		uint16_t _y = 0;
		int16_t _w;
		int16_t _h;
		std::function<void(void*)> _callback = nop;
		void* _objptr = nullptr;
		bool _isVisible = true;
		bool _isSelectable;
		bool _isPressed = false;
		bool _isSelected = false;
		bool _invalid = false;
		bool _isSquare = true;
		bool _isCircle = false;
		bool _neverHide = false;
		// cursor erase color
		uint16_t _curCol = 0xffff;
		rig_settings_t _sett_id = SETT_EMPTY;
};

class Line: public ScrObj {
	public:
		Line(int w): ScrObj()
		{
			_w = w;
		}

		virtual void draw() override
		{
			tft.drawFastHLine(_x, _y, _w, COL_GREY_70_565);
		}

		virtual void erase() override
		{
			tft.drawFastHLine(_x, _y, _w, TFT_WHITE);
		}

		virtual void freeRes() override
		{
		}
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
			if (!_invalid || !_isVisible)
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
			if (!_btnSpr)
				return;

			_btnSpr->deleteSprite();
			delete _btnSpr;
			_btnSpr = nullptr;
		}

		virtual void prepare() override
		{
			//_w = GREY_BUTTON_WIDTH;
			//_h = GREY_BUTTON_HEIGHT;
			if (!_invalid || !_isVisible)
				return;
			_btnSpr = new TFT_eSprite(&tft);

			_btnSpr->setColorDepth(COLOR_DEPTH);
			_btnSpr->createSprite(_w, _h);
#ifdef APP_DEBUG
			if (!_btnSpr->created()) {
				Serial.println("************ Failed to create sprite **********");
			}
#endif
			_btnSpr->fillSprite(_bg);

			/*
			if (!SPIFFS.exists(FONTS[_fontIndex])) {
				freeRes();
				return;
			}
			*/

			_btnSpr->loadFont(FONTS[_fontIndex]);
			_btnSpr->setTextColor(_fg, _bg);
			_btnSpr->setCursor(_paddingX, _paddingY);
			_btnSpr->print(scrStrings[_index]);
			_btnSpr->setCursor(197, _paddingY);
			_btnSpr->print(">");
			_btnSpr->unloadFont();
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;
			_btnSpr->pushSprite(_x, _y);

			// checking
			freeRes();

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
		//TFT_eSprite _btnSpr = TFT_eSprite(&tft);
		TFT_eSprite* _btnSpr = nullptr;
};


// print text to sprite
class Text: public ScrObj {
	public:
		//TODO: calculate height based on font
		Text(): ScrObj(0, TEXT_HEIGHT, false)
		{
		}

		virtual void freeRes() override
		{
			if (_index == EMPTY_STR)
				return;

			if (!_txtSp)
				return;

			_txtSp->deleteSprite();
			delete _txtSp;
			_txtSp = nullptr;
		}

		virtual void draw() override
		{
			if (_index == EMPTY_STR)
				return;

			if (!_invalid || !_isVisible)
				return;

			if (!_txtSp)
				return;

			_txtSp->pushSprite(_x + _dx, _y + _dy, TFT_TRANSPARENT);
			_invalid = false;

			// checking
			freeRes();
		}

		/*
		void setTextPadding(uint16_t padding)
		{
			_padding = padding;
		}
		*/

		virtual void prepare() override
		{
			if (_index == EMPTY_STR)
				return;

			if (!_invalid || !_isVisible)
				return;

			//_txtSp = new TFT_eSprite(&tft);
			createSpriteObj();

			_txtSp->setColorDepth(COLOR_DEPTH);
			//_h = TOP_BAR_HEIGHT - 12;
			//tft.setTextPadding(_padding);

			/*
			   if (!SPIFFS.exists(FONTS[_fontIndex])) {
			   freeRes();
			   return;
			   }
			 */

			_txtSp->loadFont(FONTS[_fontIndex]);

			if (_rightjsfy) {
				_txtSp->setTextDatum(TR_DATUM);
			}

			/**************************************************/

			// TODO: calculate based on longest substring
			// calculate _w based on string wrap

			const char* str = scrStrings[_index];
			const char* longest = str;
			const char* currentLine = str;
			size_t longestSize = 0;

			while (*str) {
				const char* next = str + 1;
				if (*str == '\n' || !*next) {
					const size_t currentLinelen = str
						- currentLine
						+ (*next ? 0 : 1);

					if (currentLinelen > longestSize) {
						longestSize = currentLinelen;
						longest = currentLine;
					}
					currentLine = next;
				}
				str++;
			}

			std::string longestAlone{longest, longestSize};

			_w = _txtSp->textWidth(longestAlone.c_str()) + _paddingX; //*2;

			/*
#ifdef APP_DEBUG
			Serial.println();
			Serial.println("Calculated sprite width: ");
			Serial.println(_w);
#endif
*/
			if (_w > SCR_WIDTH)
				_w = SCR_WIDTH;

			_txtSp->createSprite(_w, _h);
#ifdef APP_DEBUG
			if (!_txtSp->created()) {
				Serial.println("************ Failed to create sprite **********");
			}
#endif
			_txtSp->fillSprite(TFT_TRANSPARENT);
			_txtSp->setTextColor(_fg, _bg);
			//_txtSp->print(_text);
			_txtSp->print(scrStrings[_index]);
			_txtSp->unloadFont();
		}

		virtual void erase() override
		{
			if (_index == EMPTY_STR)
				return;

			tft.fillRect(_x + _dx, _y + _dy, _w, _h, _bg);
			freeRes();
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			// set colors
			_fg = fg;
			_bg = bg;
			//_txtSp->setColorDepth(8);
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

		/*
		virtual void setFont(fonts_t fontIndex) override
		{
			_fontIndex = fontIndex;
			switch (_fontIndex) {
				default: break;
				case SMALLFONT: _h = TEXT_HEIGHT; break;
				case BOLDFONT: _h = TEXT_HEIGHT+3; break;
			}
			_invalid = true;
		}
		*/


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

		void createSpriteObj()
		{
			_txtSp = new TFT_eSprite(&tft);
		}

		TFT_eSprite* getSpritePtr()
		{
			return _txtSp;
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

		void rightJustify()
		{
			_rightjsfy = true;
		}

	private:
		bool _rightjsfy = false;
		//uint16_t _padding;
		uint16_t _fg = FONT_COL_565;
		uint16_t _bg = TFT_WHITE;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = 10; //GR_BTN_Y_PADDING;
		int8_t _dx = 0;
		int8_t _dy = 0;
		TFT_eSprite* _txtSp = nullptr;
		//TFT_eSprite _txtSp = TFT_eSprite(&tft);
};

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
			if (!_invalid || !_isVisible)
				return;

			this->createSpriteObj();

			auto txtSp = this->getSpritePtr();
			auto paddingX = this->getPaddingX();
			auto bg = this->getBg();
			auto fg = this->getFg();
			//auto h = this->getH();

			//_h = TOP_BAR_HEIGHT - 12;
			//tft.setTextPadding(_padding);
			/*
			if (!SPIFFS.exists(FONTS[_fontIndex])) {
				freeRes();
				return;
			}
			*/
			txtSp->loadFont(FONTS[_fontIndex]);

			/*
			// TODO: calculate based on longest substring
			// calculate _w based on string wrap
			char* tmp = strtok(const_cast<char*>(scrStrings[_index]), "\n");

			_w = _txtSp->textWidth(tmp) + _paddingX*2;

			// old way:
			*/


			_w = txtSp->textWidth(*_txt) + paddingX*2;

			if (_w > SCR_WIDTH)
				_w = SCR_WIDTH;

			txtSp->createSprite(_w, _h);
			txtSp->fillSprite(TFT_TRANSPARENT);
			txtSp->setTextColor(fg, bg);
			//_txtSp->print(_text);
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
		BodyText(): ScrObj(0, TEXT_HEIGHT)
		{
		}

		virtual void freeRes() override
		{
			if (!_txtSp)
				return;

			_txtSp->deleteSprite();
			delete _txtSp;
			_txtSp = nullptr;
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;
			_txtSp->pushSprite(_x, _y, TFT_TRANSPARENT);
			_invalid = false;

			// checking
			freeRes();
		}

		virtual void prepare() override
		{
			//_h = TOP_BAR_HEIGHT - 12;
			if (!_invalid || !_isVisible)
				return;

			_txtSp = new TFT_eSprite(&tft);

			if (!_txtSp)
				return;

			_txtSp->setColorDepth(COLOR_DEPTH);

			/*
			if (!SPIFFS.exists(FONTS[_fontIndex])) {
				freeRes();
				return;
			}
			*/

			_txtSp->loadFont(FONTS[_fontIndex]);
			_w = _txtSp->textWidth(scrStrings[_index]) + _paddingX*2;
			_txtSp->createSprite(_w, _h);
			_txtSp->fillSprite(TFT_TRANSPARENT);
			_txtSp->setTextColor(_fg, _bg);
			_txtSp->print(scrStrings[_index]);
			_txtSp->unloadFont();
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			// set colors
			_fg = fg;
			_bg = bg;
		}

	private:
		uint16_t _fg, _bg;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		TFT_eSprite* _txtSp = nullptr;
		//TFT_eSprite _txtSp = TFT_eSprite(&tft);
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

		void loadRes(const char* filename)
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
		const char* _filename;
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

			reload();

			if (_jpegFile) {
				ui.drawJpeg(_jpegFile, _x, _y);
				_invalid = false;
			}
		}

		virtual bool isVisible() override
		{
			return true;
		}

		/*
		virtual void erase() override
		{

		}
		*/

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
			if (!_invalid || !_isVisible) {
				return;
			}

			tft.fillRect(_x, _y, _w, _h, _col);
			_invalid = false;
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

		InputField(int w, int h): ScrObj(w, h)
		{
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			if (!_notext)
				_text.draw();

			tft.setTextColor(_fg, _bg);
			tft.loadFont(FONTS[_fontIndex]);
			//_w = tft.textWidth(String(_value)) + _paddingX*2;
			if (_background)
				tft.fillRect(_x, _y, _w, _h, _bg);

			if (_showPlus) {
				tft.setCursor(_x + _dx, _y+_paddingY);
			}
			else {
				tft.setCursor(_x + _dx +_paddingX, _y+_paddingY);
			}

			String tmp = "";
			if (_isFloat) {
				tmp = String(_fvalue, 1);
			}
			else {
				tmp = String(_value);
			}

			// leading zeros
			if (_showLead && _value < 10) {
				tmp = "0" + tmp;
			}

			// draw positive
			if (_showPlus && _value >= 0) {
				tmp = "+" + tmp;
			}

			if (_isHours) {
				tmp+=":00";
			}

			tmp += _str;

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


		void setFloat()
		{
			_isFloat = true;
			_delta.f = 0.1;
		}

		bool isFloat()
		{
			return _isFloat;
		}

		void displayHours()
		{
			showLeadZero();
			_isHours = true;
		}


		virtual void freeRes() override
		{
			if (_notext)
				return;
			_text.freeRes();
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

		void trim()
		{
			_w -= 4;
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, TFT_WHITE);
			if (!_notext)
				_text.erase();
			freeRes();
		}

		virtual void prepare() override
		{
			if (_notext)
				return;
			_text.invalidate();
			_text.prepare();
		}

		virtual void setText(dispStrings_t index) override
		{
			if (index > END_OF_STRINGS || _notext)
				return;

			// get length in current font
			tft.loadFont(FONTS[_text.getFontIndex()]);

			// only needed for LEFT align
			//_textLength = tft.textWidth(scrStrings[_text.getStrIndex()]);
			_textLength = tft.textWidth(scrStrings[index]);


			_w = tft.textWidth(placeholder[_width]) + _paddingX*2;

			//adjust width
			_w += _dw;

			// calculate text position
			switch (_align) {
				default:
				case RIGHT:
					_text.setXYpos(_x + _w + _text.getXpadding(),
							_y + _dy + int(_h/2) - int(_text.getH()/2) + 3);
							//+  _text.getYpadding()/2);
					break;
				case LEFT:
					_text.setXYpos(_x - _textLength - _text.getXpadding(),
							_y + _dy + int(_h/2) - int(_text.getH()/2) + 3);
							//+ _text.getYpadding()/2);
					break;
				case TOP:
					// TODO: calculate this. Currently same as RIGHT
					_text.setXYpos(_x + _w + _text.getXpadding(),
							_y + _dy + _text.getYpadding()/2);
					break;
			}

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

		virtual void add() override
		{
			if (_isFloat) {
				setValue(_fvalue += _delta.f);
			}
			else {
				setValue(_value += _delta.i);
			}
		}

		virtual void sub() override
		{
			if (_isFloat) {
				setValue(_fvalue -= _delta.f);
			}
			else {
				setValue(_value -= _delta.i);
			}
		}

		void setDelta(int delta)
		{
			_delta.i = delta;
		}

		void setDelta(float fdelta)
		{
			_delta.f = fdelta;
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
			_text.setColors(fg, bg);
			_invalid = true;
		}

		virtual void setValue(int value) override
		{
			if (!_ignoreLimits) {
				if (value > _upper)
					value = _lower;
				if (value < _lower)
					value = _upper;
			}
			_invalid = true;
			_value = value;
		}

		void ignoreLimits()
		{
			_ignoreLimits = true;
		}

		void setValue(float value)
		{
			if (!_ignoreLimits) {
				if (value > _fupper)
					value = _flower;
				if (value < _flower)
					value = _fupper;
			}

			_fvalue = value;
			_isFloat = true;
			_invalid = true;
		}

		virtual int getValue() override
		{
			return _value;
		}

		float getFvalue()
		{
			return _fvalue;
		}

		void setLimits(int16_t lower, int16_t upper)
		{
			_upper = upper;
			_lower = lower;
		}

		void setfLimits(float lower, float upper)
		{
			_fupper = upper;
			_flower = lower;
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

		void setStr(const char* str)
		{
			_str = str;
		}

		void setTDS()
		{
			setfLimits(0.1, 5.0);
		}

		void setPh()
		{
			setfLimits(3.0, 8.0);
		}

		void setMult()
		{
			setfLimits(0.1, 10.0);
		}

		void noXpadding()
		{
			_paddingX = 0;
		}

		void noBg()
		{
			_background = false;
		}

		void noText()
		{
			_notext = true;
		}

	protected:
		union {
			int i = 1;
			float f;
		} _delta;

		int16_t _value = 0;
		float _fvalue = 0.0;
		bool _isFloat = false;
		bool _isHours = false;
		bool _ignoreLimits = false;
		Text _text;
		int _textLength = 0;
		bool _showLead = false;
		bool _showPlus = false;
		uint16_t _fg = FONT_COL_565;
		//uint16_t _bg = COL_GREY_E3_565;
		uint16_t _bg = greyscaleColor(0xDC);
		int8_t _dy = 0;
		int8_t _dx = 0;
		int8_t _dw = 0;
		//uint16_t _textw;
		int16_t _upper = 100;
		int16_t _lower = 0;
		float _fupper = 14.0;
		float _flower = 0.1;
		uint8_t _paddingX = IN_FLD_X_PADDING;
		uint8_t _paddingY = IN_FLD_Y_PADDING;
		const char* _str = "";
		bool _background = true;
		bool _notext = false;
		placeholder_t _width = THREE_CHR;
};

class OutputField: public InputField {
	public:
		OutputField(): InputField(0, INPUT_H)
		{
			ignoreLimits();
		}

};

class OutputFieldMain: public InputField {
	public:
		OutputFieldMain(): InputField(0, 13)
		{
			ignoreLimits();
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			tft.setTextColor(_fg, _bg);
			tft.loadFont(FONTS[_fontIndex]);
			tft.fillRect(_x+_dx, _y, _w, _h, _bg);

			tft.setCursor(_x+_dx, _y);

			String tmp = "";
			if (_isFloat) {
				tmp = String(_fvalue, 1);
			}
			else {
				tmp = String(_value);
			}

			// leading zeros
			if (_showLead && _value < 10) {
				tmp = "0" + tmp;
			}

			// draw positive
			if (_showPlus && _value >= 0) {
				tmp = "+" + tmp;
			}

			if (_isHours) {
				tmp+=":00";
			}

			tmp += _str;

			tft.print(tmp);
			tft.unloadFont();

			_invalid = false;
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, TFT_WHITE);
			freeRes();
		}

		virtual void freeRes() override
		{
		}
};

class HourLimits: public ScrObj {
	public:
		HourLimits(int x, int y)
		{
			setXYpos(x, y);

			_lower.displayHours();
			_higher.displayHours();

			_lower.setWidth(FOUR_CHR);
			_higher.setWidth(FOUR_CHR);

			_lower.setText(EMPTY_STR);
			_higher.setText(EMPTY_STR);

			_lower.setXYpos(_x, _y);
			_dash.setXYpos(_x + _lower.getW() + _GAP, _y + _lower.getH()/2);
			_higher.setXYpos(_x + _lower.getW() + _dash.getW() + _GAP*2, _y);

			_higher.adjustTextX(-2);
			_lower.adjustTextX(-2);

			_higher.setLimits(0, 23);
			_lower.setLimits(0, 23);

			_lower.setValue(8);
			_higher.setValue(23);
		}

		void* getHigherPtr()
		{
			return &_higher;
		}

		void* getLowerPtr()
		{
			return &_lower;
		}

		ScrObj* getDashPtr()
		{
			return &_dash;
		}

		virtual void invalidate() override
		{
		}

		virtual void prepare() override
		{
		}

		virtual void draw() override
		{
		}

		virtual void freeRes() override
		{
		}

		virtual void erase() override
		{
		}

	private:
		static constexpr uint8_t _GAP = 8;
		InputField _lower = InputField();
		InputField _higher = InputField();
		Line _dash = Line(10);
};

class DayLimits: public ScrObj {
	public:
		DayLimits(int x, int y)
		{
			setXYpos(x, y);

			_lower.setWidth(TWO_CHR);
			_higher.setWidth(FOUR_CHR);

			_lower.setText(EMPTY_STR);
			_higher.setText(EMPTY_STR);

			_lower.setXYpos(_x, _y);
			_dash.setXYpos(_x + _lower.getW() + _GAP, _y + _lower.getH()/2);
			_higher.setXYpos(_x + _lower.getW() + _dash.getW() + _GAP*2, _y);
			_lower.setColors(COL_GREY_70_565, TFT_WHITE);
		}

		InputField* getInputFieldPtr()
		{
			return &_higher;
		}

		OutputField* getOutputFieldPtr()
		{
			return &_lower;
		}

		ScrObj* getHigherPtr()
		{
			return &_higher;
		}

		ScrObj* getLowerPtr()
		{
			return &_lower;
		}

		ScrObj* getDashPtr()
		{
			return &_dash;
		}

		virtual void invalidate() override
		{
		}

		virtual void prepare() override
		{
		}

		virtual void draw() override
		{
		}

		virtual void freeRes() override
		{
		}

		virtual void erase() override
		{
		}

	private:
		static constexpr uint8_t _GAP = 16;
		OutputField _lower = OutputField();
		InputField _higher = InputField();
		Line _dash = Line(6);
};


class CheckBox: public ScrObj {
	public:
		CheckBox(): ScrObj(CHK_BOX_SIZE, CHK_BOX_SIZE, SELECTABLE)
		{
		}

		~CheckBox()
		{
			freeRes();
		}

		int getTextX()
		{
			return _text.getX();
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
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
			if (!_invalid || !_isVisible)
				return;
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

		virtual void setCallback(std::function<void(void*)> callback, void* objptr = nullptr) override
		{
			_callback = callback;
			_objptr = objptr;
		}

		void setCallback(std::function<void(void*, void*)> callback, void* objptr, void* objptr2 = nullptr)
		{
			_callback2 = callback;
			_objptr = objptr;
			_objptr2 = objptr2;
		}

		virtual void onClick() override
		{
			if (_objptr2)
				_callback2(_objptr, _objptr2);
			else
				_callback(_objptr);
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
		std::function<void(void*, void*)> _callback2;
		void* _objptr2 = nullptr;
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
#define TGL_H 18
//#define TGL_ON_COL
#define TGL_OFF_COL 0x6E
#define TGL_SHF_RAD 6

class Toggle: public ScrObj {
	public:
		Toggle(): ScrObj(TGL_W, TGL_H, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			tft.fillSmoothRoundRect(_x, _y, _w, _h-2, TGL_RAD, greyscaleColor(TGL_BG), TFT_WHITE);

			_text.draw();

			if (_isOn) {
				_col = tft.color565(0x4C, 0xAF, 0x50);
				_shaftX = _x + _w - TGL_RAD;
			}
			else {
				_col = greyscaleColor(TGL_OFF_COL);
				_shaftX = _x + TGL_RAD - 1;
			}

			_shaftY = _y + _h/2 - 1;

			tft.fillSmoothCircle(_shaftX, _shaftY, TGL_SHF_RAD, _col, greyscaleColor(TGL_BG));

			_invalid = false;
		}

		virtual void freeRes() override
		{
			_text.freeRes();
		}

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			_text.erase();
			freeRes();
		}

		virtual void onClick() override
		{
			_callback(_objptr, _id);
		}

		void setCallback(std::function<void(void*, int)> callback, void* objptr = nullptr, int id = 0)
		{
			_callback = callback;
			_objptr = objptr;
			_id = id;
		}

		/*
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
		*/

		void setTextX(uint16_t x)
		{
			_textX = x;
			_textHardX = true;
		}

		virtual void prepare() override
		{

			if (!_invalid || !_isVisible)
				return;

			if (_textHardX)
				goto skip;

			// calculate text position
			switch (_align) {
				default:
				case RIGHT:
					_textX = _x + _w + _text.getXpadding();
					break;
				case LEFT:
					_textX = _x - _textLength - _text.getXpadding();
					break;
				case TOP:
					_textX = _x + _w + _text.getXpadding();
					break;
			}
skip:
			_text.setXYpos(_textX, _y + _dy + int(_h/2) - int(_text.getH()/2));
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
		std::function<void(void*, int)> _callback;
		//dispStrings_t _index;
		Text _text;
		//fonts_t _fontIndex;
		uint16_t _bg, _fg, _col, _shaftX, _shaftY, _textX;
		int _textLength = 0;
		int _dy = 0;
		int _id = -1;
		//bool _textAligned = false;
		bool _isOn = false;
		bool _textHardX = false;
};

//#define RAD_BG_COL 0xE3
#define RAD_BG_COL 0xDC


// TODO: replace with antialiased image
class CircRadBtn: public ScrObj {
	public:
		CircRadBtn(): ScrObj(RAD_BTN_SIZE, RAD_BTN_SIZE, SELECTABLE)
		{
		}

		CircRadBtn(int w, int h): ScrObj(w, h)
		{
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			tft.fillRect(_x, _y, _w, _h, greyscaleColor(_bgcol));

			if (_isOn)
				//#4CAF50 - checked green
				_col = tft.color565(0x4C, 0xAF, 0x50);
			else
				_col = 0xFFFF;

			int x = _x + _w/2 - 1;
			int y = _y + _h/2 - 1;
			tft.fillSmoothCircle(x, y, _r, _col, greyscaleColor(_bgcol));
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

class RadioButton: public ScrObj {
	public:
		RadioButton(): ScrObj(RAD_BTN_SIZE, RAD_BTN_SIZE, SELECTABLE)
		{
		}

		RadioButton(int w, int h): ScrObj(w, h)
		{
		}

		void adjustCircleY(int dy)
		{
			_dy = dy;
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			if (_background)
				tft.fillRect(_x, _y, _w, _h, _bgcol);

			_text.draw();

			if (_isOn)
				//#4CAF50 - checked green
				_col = tft.color565(0x4C, 0xAF, 0x50);
			else
				_col = 0xFFFF;

			int x = _x + _w/2 - 1;
			int y = _y + _h/2 - 1 + _dy;
			tft.fillSmoothCircle(x, y, _r, _col, _bgcol);
			//tft.fillCircle(x, y, _r, _col);
			_invalid = false;
		}

		virtual void freeRes() override
		{
			_text.freeRes();
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
			if (!_invalid || !_isVisible)
				return;
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
		void setBgColor(uint16_t bgcol)
		{
			_bgcol = bgcol;
		}

		void on(bool isOn)
		{
			_isOn = isOn;
			_invalid = true;
		}

		void noBg()
		{
			_background = false;
		}

	private:
		Text _text;
		bool _isOn = false;
		int _r = 5;
		int _dy = 0;
		int _dx = 0;
		uint16_t _col = 0xffff;
		uint16_t _bgcol = COL_GREY_DC_565;
		bool _background = true;
};

#include "settings.h" // rig_typ

class RigTypeRadioButton: public RadioButton {
	public:
		virtual void onClick() override
		{
			if (!this->isOn())
				_callback(_objptr);
		}

		/*
		virtual void setCallback(std::function<void(void*)> callback, void* objptr = nullptr) override
		{
			_callback = callback;
			_objptr = objptr;
		}
		*/

		/*
		void setCallback(std::function<void(void*, rig_type)> callback, void* objptr, rig_type rig)
		{
			_callback = callback;
			_rig = rig;
		}

		virtual void onClick() override
		{
			_callback(_objptr, _rig);
		}

	private:
		std::function<void(void*, rig_type)> _callback;
		*/
};

class CircIndicator: public RadioButton {
	public:
		CircIndicator(): RadioButton(RAD_BTN_SIZE, RAD_BTN_SIZE)
		{
		}
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
			_waitText.setColors(COL_GREY_70_565, TFT_WHITE);
		}

		void setInterval(unsigned long interval)
		{
			_interval = interval;
		}

		virtual void freeRes() override
		{
			_waitText.freeRes();
		}

		virtual void draw() override
		{
			if (!_isVisible) {
				return;
			}

			if (_invalid) {
				_waitText.draw();
			}

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
			if (!_invalid || !_isVisible)
				return;

			_waitText.setXYpos(_x + _waitText.getXpadding(), _y - _waitText.getYpadding()*2);
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

class CompositeBox: public ScrObj {
	public:
		virtual void draw() override
		{
			if (!_invalid || !_isVisible) {
				return;
			}

			tft.fillRect(_x, _y, _w, _h, _empty ? _red : _bg);

			for (auto& obj:_items) {
				if (obj->isVisible()) {
					obj->draw();
				}
				else {
					obj->erase();
				}
			}

			_invalid = false;
		}

		virtual void prepare() override
		{
			for (auto& obj:_items) {
				obj->prepare();
			}
		}

		virtual void invalidate() override
		{
			_invalid = true;
			for (auto& obj:_items) {
				obj->invalidate();
			}
		}

		virtual void freeRes() override
		{
		}

		// adjust first item XY if it is text (it's magical!)
		void adjust()
		{
			if (!_items.size())
				return;

			_items[0]->setXYpos(_x+2, _y+6);

			if (_items.size() == 1)
				return;

			_items[1]->setXYpos(((_x+_w)-_w/3) - _items[1]->getW()/2, (_y+_h/2) - _items[1]->getH()/2);

			/*
			if (_items.size() == 2)
				return;

			_items[1]->setXYpos(_x+_w-2*_w/3, (_y+_h/2) - _items[1]->getH()/2);
			_items[2]->setXYpos(_x+_w-_w/3, (_y+_h/2) - _items[2]->getH()/2);
			*/
		}

		void addItem(ScrObj* item)
		{
			if (!_items.size())
				item->setXYpos(_x,_y);
			else if (_items.size() == 1) {
				item->setXYpos(((_x+_w)-_w/4) - item->getW()/2, (_y+_h/2) - item->getH()/2);
			}
			else if (_items.size() == 2) {
				ScrObj* prevItem = _items.at(_items.size() - 1);
				prevItem->setXYpos(prevItem->getX(), (_y+_h/3) + 2 - prevItem->getH()/2);
				item->setXYpos(((_x+_w)-_w/4) - item->getW()/2, (_y+_h-_h/3) + 2 - item->getH()/2);
			}


			_items.push_back(item);
		}

		void setEmpty(bool empty)
		{
			_empty = empty;
			invalidate();
			prepare();
		}

	private:
		obj_list _items;
		uint16_t _bg = COL_GREY_DC_565;
		uint16_t _red = COL_RED_TANK_565;
		bool _empty = false;
		//uint16_t _bg = 0xFFFF;
};

#define COL_RED_EMPTY_565 0xfbcf
class SmallBox: public ScrObj {
	public:
		virtual void draw() override
		{
			if (!_invalid || !_isVisible) {
				return;
			}

			tft.fillRect(_x, _y, _w, _h, _empty ? _red : _bg);

			if (_image) {
				//_image->invalidate();
				//_image->reload();
				_image->draw();
			}

			if (_text) {
				//_text->invalidate();
				_text->prepare();
				_text->draw();
			}

			if (_check) {
				//_check->invalidate();
				_check->draw();
			}

			_invalid = false;
		}

		virtual void prepare() override
		{
			if (_text) {
				_text->invalidate();
				_text->prepare();
				//Serial.println("prepared!");
			}
		}

		virtual void invalidate() override
		{
			_invalid = true;
			if (_text)
				_text->invalidate();
			if (_check)
				_check->invalidate();
			if (_image)
				_image->invalidate();
		}

		virtual void freeRes() override
		{
			if (_text)
				_text->freeRes();
			if (_check)
				_check->freeRes();
			if (_image)
				_image->freeRes();
		}

		void on(bool flag)
		{
			if (!_check)
				return;
			_check->on(flag);
			invalidate();
		}

		bool isOn()
		{
			if (!_check)
				return false;

			return _check->isOn();
		}

		bool getEmpty()
		{
			return _empty;
		}

		void setEmpty(bool empty = true)
		{
			_empty = empty;

			if (_empty) {
				_image = _imgEmp;
				if (_text)
					_text->setColors(_fg, _red);
				if (_check)
					_check->setBgColor(_red);
			}
			else {
				_image = _imgFull;
				if (_text)
					_text->setColors(_fg, _bg);
				if (_check)
					_check->setBgColor(_bg);
			}
			invalidate();
		}

		void setText(Text* text)
		{
			_text = text;
			_text->setXYpos(_x+4, _y+6);
		}

		void setCheck(CircIndicator* check)
		{
			_check = check;
			_check->setXYpos(((_x+_w)-_w/4) - _check->getW()/2, (_y+_h/2) - _check->getH()/2);
		}

		void setImages(Image* imgFull, Image* imgEmp)
		{
			_image = imgFull;
			_imgFull = imgFull;
			_imgEmp = imgEmp;
			_imgFull->setXYpos(_x, _y);
			_imgEmp->setXYpos(_x, _y);
		}

	private:
		uint16_t _fg = 0;
		uint16_t _bg = COL_GREY_DC_565;
		uint16_t _red = COL_RED_EMPTY_565;
		bool _empty = false;
		Text* _text = nullptr;
		CircIndicator* _check = nullptr;
		Image* _image = nullptr;
		Image* _imgEmp = nullptr;
		Image* _imgFull = nullptr;
};

typedef enum {
	T_EMPTY,
	T_HALF,
	T_TWOTHIRDS,
	T_FULL
} tank_state_t;

class Tank: public ScrObj {
	public:
		Tank(): ScrObj(19, 65)
		{
			invalidate();
		}

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
				return;

			_drawEmpty();

			if (_state == T_EMPTY) {
				// draw only lines
				_drawBottom();
				_drawLines();
			}
			else if (_state == T_HALF) {
				// draw bottom fill and lines
				_drawMiddleBottom();
				_drawLines();
			}
			else if (_state == T_TWOTHIRDS) {
				// draw middle fill lines
				_drawMiddleTop();
				_drawLines();
			}
			else if (_state == T_FULL) {
				_drawFull();
				_drawLines();
				// draw full and lines
			}
			_invalid = false;
		}

		virtual void freeRes() override
		{
		}

		void setState(tank_state_t state)
		{
			_state = state;
			invalidate();
		}

		Tank& operator++(int)
		{
			invalidate();
			switch (_state) {
				default: _state = T_EMPTY; break;
				case T_FULL: _state = T_EMPTY; break;
				case T_EMPTY: _state = T_HALF; break;
				case T_HALF: _state = T_TWOTHIRDS; break;
				case T_TWOTHIRDS: _state = T_FULL; break;
			}
			return *this;
		}

	private:
		void _drawLines()
		{
			tft.drawFastHLine(_x, _y+50, _w, _fg);
			tft.drawFastHLine(_x, _y+19, _w, _fg);
		}

		void _drawEmpty()
		{
			tft.fillSmoothRoundRect(_x, _y, _w, _h, 7, _fg);
			tft.fillSmoothRoundRect(_x+1, _y+1, _w-2, _h-2, 5, _bg);
		}

		void _drawBottom()
		{
			int subrectH = 6;
			int botRectH = 20;
			tft.fillSmoothRoundRect(_x+1, _y+_h-botRectH-1, _w-2, botRectH, 5, COL_RED_TANK_565);
			tft.fillRect(_x+1, _y+_h-botRectH-1, _w-2, subrectH, _bg);
		}

		void _drawMiddleBottom()
		{
			int subrectH = 6;
			int botRectH = 34;
			tft.fillSmoothRoundRect(_x+1, _y+_h-botRectH-1, _w-2, botRectH, 5, _water);
			tft.fillRect(_x+1, _y+_h-botRectH-1, _w-2, subrectH, _bg);
		}

		void _drawMiddleTop()
		{
			int subrectH = 6;
			int botRectH = 52;
			tft.fillSmoothRoundRect(_x+1, _y+_h-botRectH-1, _w-2, botRectH, 5, _water);
			tft.fillRect(_x+1, _y+_h-botRectH-1, _w-2, subrectH, _bg);
		}

		void _drawFull()
		{
			tft.fillSmoothRoundRect(_x+1, _y+1, _w-2, _h-2, 5, _water);

			int subrectH = 6;
			int botRectH = 23;
			tft.fillSmoothRoundRect(_x+1, _y+1, _w-2, botRectH, 5, COL_RED_TANK_565);
			tft.fillRect(_x+1, _y-subrectH+botRectH+1, _w-2, subrectH, _water);
		}

		tank_state_t _state = T_EMPTY;
		uint16_t _bg = 0xFFFF;
		uint16_t _fg = COL_GREY_70_565;
		uint16_t _water = tft.color565(0xaa, 0xe5, 0xe9);
} g_tankBig;

Tank g_tankSmall;

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

//#include "settings.h" // g_data
class Page {
	public:
		void addItem(ScrObj* scrobj)
		{
			_items.push_back(scrobj);
			if (scrobj->isSelectable() && scrobj->isVisible()) {
				_selectable.push_back(scrobj);
			}
		}

		void restock()
		{
			_selectable.clear();
			for(auto& i:_items) {
				if (i->isSelectable() && i->isVisible()) {
					_selectable.push_back(i);
				}
			}
		}

		void draw()
		{
			for (auto& obj:_items) {
				if (obj->isVisible()) {
					//obj->prepare();
					obj->draw();
				}
				else {
					obj->erase();
				}
			}
		}

		// set all screen objects for redraw
		void invalidateAll()
		{
			for (auto& obj:_items)
				obj->invalidate();
		}

		void freeRes()
		{
			for (auto& obj:_items)
				obj->freeRes();
		}

		void erase()
		{
			//tft.fillRect(0, 28, 240, 284 - 28, greyscaleColor(BACKGROUND));
			for (auto& obj:_items)
				obj->erase();
		}

		void prepare()
		{
			for (auto& obj:_items) {
				/*
				// ::::::: omg, project Zeus :::::::::
				if (obj->hasInput()) {
					InputField* itm = (InputField*) currItem;
					if (itm->isFloat() && itm->getSettingsId())
						itm->setValue(g_data.getFloat(itm->getSettingsId()));
					else
						itm->setValue(g_data.getInt(itm->getSettingsId()));
				}
				*/

				if (obj->isVisible())
					obj->prepare();
			}
		}

		ScrObj* getCurrItem()
		{
			return _currItem;
		}

		ScrObj* getCurrItemAt(size_t i)
		{
			if (_selectable.at(i)->isVisible())
				return _selectable.at(i);
			else
				return nullptr;
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

		Page* next()
		{
			return _next;
		}

		void setPrev(Page* page)
		{
			_prev = page;
		}

		void setNext(Page* page)
		{
			_next = page;
		}

		bool visibleIcons()
		{
			return _iconsVisible;
		}

		void setIconsVis(bool vis)
		{
			_iconsVisible = vis;
		}

		void setInvisible()
		{
			for (auto& obj:_items) {
				if (obj->canBeHidden())
					obj->setInvisible();
			}
		}

		void setVisible()
		{
			for (auto& obj:_items) {
				obj->setVisible();
				obj->prepare();
			}
		}

		bool lastStage()
		{
			return _lastStage;
		}

		void setLastStage()
		{
			_lastStage = true;
		}

	private:
		bool _lastStage = false;
		bool _iconsVisible = true;
		Page* _prev = nullptr;
		Page* _next = nullptr;
		dispStrings_t _title;
		obj_list _items;
		obj_list _selectable;
		ScrObj* _currItem;
};
#endif
