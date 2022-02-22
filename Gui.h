//TODO:
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

// hardware... stuff
#include <SPI.h>
#include <TFT_eSPI.h>


/***************************** defines *************************/
// debug print object address
#define DEBUG_PRINT(A) Serial.println((unsigned long long) (A))

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

// input field
#define INPUT_H 24

// radio button
#define RAD_BTN_SIZE 22

// fonts
typedef enum {
	SMALLFONT,
	LARGEFONT,
	BOLDFONT,
	END_OF_FONTS
} fonts_t;

const char* FONTS[END_OF_FONTS] = {
	"SegoeUI-12",
	"SegoeUI-18",
	"SegoeUI-Bold-16"
};

// pointer to current language strings
const char** scrStrings = ruStrings;

// objs
TFT_eSPI tft = TFT_eSPI();

GfxUi ui = GfxUi(&tft);

void nop()
{
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
			nop();
		}

		virtual void erase()
		{
			tft.fillRect(_x, _y, _w, _h, greyscaleColor(BACKGROUND));
			freeRes();
		}

		void setCallback(void(*callback)())
		{
			_callback = callback;
		}

		virtual void onClick()
		{
			_callback();
		}

		bool isVisible()
		{
			return _isVisible;
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

		void setText(dispStrings_t index)
		{
			if (index > END_OF_STRINGS)
				return;
			_index = index;
			_invalid = true;
		}

		void setFont(fonts_t fontIndex)
		{
			_fontIndex = fontIndex;
			_invalid = true;
		}

	protected:
		dispStrings_t _index;
		fonts_t _fontIndex;
		uint16_t _x = 0;
		uint16_t _y = 0;
		uint16_t _w;
		uint16_t _h;
		void (*_callback)() = nop;
		bool _isVisible = true;
		bool _isSelectable;
		bool _isPressed = false;
		bool _isSelected = false;
		bool _invalid = false;
		bool _isSquare = true;
		bool _isCircle = false;
		// cursor erase color
		uint16_t _curCol = 0xffff;
};

// current selected item
ScrObj* currItem = nullptr;
typedef std::vector<ScrObj*> obj_list;


class BlueTextButton: public ScrObj {

	public:
		BlueTextButton(): ScrObj(0, BLUE_BUTTON_HEIGHT, SELECTABLE)
		{
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

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
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
		Text(): ScrObj(0, TOP_BAR_HEIGHT - 12, false)
		{
		}

		virtual void freeRes() override
		{
			_txtSp.deleteSprite();
		}

		virtual void draw() override
		{
			_invalid = false;
			_txtSp.pushSprite(_x, _y, TFT_TRANSPARENT);
		}

		void prepare()
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

	private:
		uint16_t _fg, _bg;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		TFT_eSprite _txtSp = TFT_eSprite(&tft);
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
			_invalid = false;
			_txtSp.pushSprite(_x, _y, TFT_TRANSPARENT);
		}

		void prepare()
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
			if (_invalid)
				reload();

			if (_invalid && _jpegFile) {
				ui.drawJpeg(_jpegFile, _x, _y);
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

//TODO: increase/decrease with buttons
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
			_w = tft.textWidth(String(_value)) + _paddingX*2;
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

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
			_invalid = true;
		}

		void setValue(uint16_t value)
		{
			_invalid = true;
			_value = value;
		}

		uint16_t getValue() 
		{
			return _value;
		}

	private:
		uint16_t _fg, _bg, _textw;
		uint8_t _paddingX = BL_BTN_X_PADDING;
		uint8_t _paddingY = BL_BTN_Y_PADDING;
		uint16_t _value = 0;
};

class CheckBox: public ScrObj {
	public:
		CheckBox(): ScrObj(CHK_BOX_SIZE, CHK_BOX_SIZE, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;
			if (!_isOn) {
				tft.drawRect(_x, _y, _w, _h, greyscaleColor(CHK_BOX_COL));
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

		void on(bool isOn)
		{
			_isOn = isOn;
		}

		void reload()
		{
			_jpegFile = SPIFFS.open(_filename, "r");
		}

	private:
		uint16_t _bg;
		bool _textAligned = false;
		bool _isOn = false;
		fs::File _jpegFile;
		const char* _filename = CHK_BOX_FILE;
};

#define TGL_BG 0xDC
#define TGL_RAD 9
#define TGL_W 33
#define TGL_H 17
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
			if (!_invalid)
				return;

			tft.fillRoundRect(_x, _y, _w, _h, TGL_RAD, greyscaleColor(TGL_H));

			if (_isOn) {
				_col = tft.color565(0x4C, 0xAF, 0x50);
				_shaftX = _x + uint16_t(3/4*_w);
				_shaftY = _y + _h/2;
				_invalid = false;
			}
			else {
				_col = greyscaleColor(TGL_OFF_COL);
				_shaftX = _x + uint16_t(3/4*_w);
				_shaftY = _y + _h/2;
				_invalid = false;
			}
			
			tft.fillCircle(_shaftX, _shaftY, TGL_SHF_RAD, _col);
		}

		virtual void freeRes() override
		{
		}

		void on(bool isOn)
		{
			_isOn = isOn;
		}

	private:
		dispStrings_t _index;
		fonts_t _fontIndex;
		uint16_t _bg, _fg, _col, _shaftX, _shaftY;
		bool _textAligned = false;
		bool _isOn = false;
};


class CircRadBtn: public ScrObj {
	public:
		CircRadBtn(): ScrObj(RAD_BTN_SIZE, RAD_BTN_SIZE, SELECTABLE)
		{
		}

		virtual void draw() override
		{
			if (!_invalid)
				return;

			if (_isOn)
				//#4CAF50 - checked green
				_col = tft.color565(0x4C, 0xAF, 0x50);
			else
				_col = 0xFFFF;

			int x = _x + _w/2 - 1;
			int y = _y + _h/2 - 1;
			tft.fillCircle(x, y, _r, _col);
		}

		virtual void freeRes() override
		{
		}
		
		virtual void onClick() override
		{
			if (!_isOn)
				_callback();
		}

		virtual void prepare() override
		{
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

		void on(bool isOn)
		{
			_isOn = isOn;
		}

	private:
		bool _isOn = false;
		int _r = 5;
		uint16_t _col = 0xffff;
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
} cursor;

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
			return _items.size();
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

	private:
		obj_list _items;
		obj_list _selectable;
		ScrObj* _currItem;
};

Page* currPage;

class App {
	private:
		unsigned long _oldMils = 0;
		unsigned long _dbMils = 0;
		bool _blink = false;
		bool _dbFlag = false;
		Cursor _cursor;
		int _iterator = 0;
	public:
		void init()
		{
			SPIFFS.begin();
			tft.init();
			tft.setRotation(0);
			tft.fillScreen(greyscaleColor(BACKGROUND));
		}

		void update()
		{
#ifdef TASKS
			yield();
			sleep(10);
#endif

			if (millis() - _oldMils > TIMER) {
				cursor.draw(_blink);
				_oldMils = millis();
				_blink = !_blink;
			}

			if (!digitalRead(BTN_PREV) || !digitalRead(BTN_NEXT) || !digitalRead(BTN_OK)) {
				if (millis() - _dbMils > DEBOUNCE) {
					if (!digitalRead(BTN_PREV) || !digitalRead(BTN_NEXT) || !digitalRead(BTN_OK)) {
						_dbMils = millis();
						_dbFlag = true;
					}
				}
			}

			if (!digitalRead(BTN_PREV) && _dbFlag) {
				_oldMils = millis();
				//Serial.println(currPage->selSize());
				cursor.draw(false);
				_iterator--;
				if (_iterator < 0)
					_iterator = currPage->selSize() - 1;
				currItem = currPage->getCurrItemAt(_iterator);
				cursor.draw(true);
				_dbFlag = false;
			}
			else if (!digitalRead(BTN_NEXT) && _dbFlag) {
			_oldMils = millis();
				cursor.draw(false);
				_iterator++;
				if (_iterator > currPage->selSize() - 1)
					_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
				_cursor.draw(true);
			 _dbFlag = false;
			}
			else if (!digitalRead(BTN_OK) && _dbFlag) {
			_oldMils = millis();
				_cursor.draw(false);
				currItem->onClick();
				// TODO: don't reset iterator unless needed
				_iterator = 0;
				currItem = currPage->getCurrItemAt(_iterator);
			_dbFlag = false;
			}
		}
};
