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

#include <vector>

#include "images.h"

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
#define COL_GREY_70_565 0x738E
#define COL_GREY_E3_565 0xE71C

// checkbox
#define CHK_BOX_COL 0xDC
#define CHK_BOX_SIZE 21
#define CHK_BOX_FILE "/check.jpg"

// input field height
#define INPUT_H 24

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
		//bool _wasSelectable;
		bool _isPressed = false;
		bool _isSelected = false;
		bool _invalid = false;
		bool _isSquare = true;
		bool _isCircle = false;
		// cursor erase color
		uint16_t _curCol = 0xffff;
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
			if (!_invalid)
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
		Text(): ScrObj(0, TOP_BAR_HEIGHT - 12, false)
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
			if (!_invalid)
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

			/*
			// TODO: calculate based on longest substring
			// calculate _w based on string wrap
			char* tmp = strtok(const_cast<char*>(scrStrings[_index]), "\n");

			_w = _txtSp->textWidth(tmp) + _paddingX*2;

			// old way:
			*/


			_w = _txtSp->textWidth(scrStrings[_index]) + _paddingX*2;

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
		uint8_t _paddingY = GR_BTN_Y_PADDING;
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
			if (!_invalid)
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
		BodyText(): ScrObj(0, TOP_BAR_HEIGHT - 12)
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
			if (!_invalid)
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
			if (!_invalid || !_isVisible)
				return;

			if (_invalid)
				reload();

			if (_invalid && _jpegFile) {
				ui.drawJpeg(_jpegFile, _x, _y);
				_invalid = false;
			}
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
		}

		void displayHours()
		{
			showLeadZero();
			_isHours = true;
		}


		virtual void freeRes() override
		{
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

		virtual void erase() override
		{
			tft.fillRect(_x, _y, _w, _h, TFT_WHITE);
			_text.erase();
			freeRes();
		}

		virtual void prepare() override
		{


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

		void setValue(float f)
		{
			_fvalue = f;
			_invalid = true;
		}

		virtual int getValue() override
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

class OutputField: public InputField {
	public:
		OutputField(): InputField(0, INPUT_H)
		{
			ignoreLimits();
		}
};

#define HOUR_LIM_GAP 8
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

			int gap = HOUR_LIM_GAP;

			_lower.setXYpos(_x, _y);
			_dash.setXYpos(_x + _lower.getW() + gap, _y + _lower.getH()/2);
			_higher.setXYpos(_x + _lower.getW() + _dash.getW() + gap*2, _y);

			_higher.adjustTextX(-2);
			_lower.adjustTextX(-2);

			_higher.setLimits(0, 23);
			_lower.setLimits(0, 23);

			_lower.setValue(8);
			_higher.setValue(23);
		}

		ScrObj* getHigher()
		{
			return &_higher;
		}

		ScrObj* getLower()
		{
			return &_lower;
		}

		ScrObj* getDash()
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
		InputField _lower = InputField();
		InputField _higher = InputField();
		Line _dash = Line(10);
};

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

#define RAD_BG_COL 0xE3

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

		virtual void draw() override
		{
			if (!_invalid || !_isVisible)
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
			tft.fillSmoothCircle(x, y, _r, _col, greyscaleColor(_bgcol));
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
			_invalid = true;
		}

	private:
		Text _text;
		bool _isOn = false;
		int _r = 5;
		int _dy = 0;
		int _dx = 0;
		uint16_t _col = 0xffff;
		uint8_t _bgcol = RAD_BG_COL;
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
			for (auto& obj:_items)
				obj->prepare();
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

	private:
		bool _iconsVisible = true;
		Page* _prev = nullptr;
		Page* _next = nullptr;
		dispStrings_t _title;
		obj_list _items;
		obj_list _selectable;
		ScrObj* _currItem;
};
#endif
