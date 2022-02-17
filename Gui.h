// defines
#define GREY_BUTTON_HEIGHT 24
#define GREY_BUTTON_WIDTH 210
#define GR_BTN_X_PADDING 8
#define GR_BTN_Y_PADDING 7
#define GR_BTN_TXT_COLOR 0x70
#define GR_BTN_BG_COLOR 0xE3

#define BLUE_BUTTON_HEIGHT 24
#define BL_BTN_X_PADDING 8
#define BL_BTN_Y_PADDING 7

#define TOP_BAR_HEIGHT 27

#define DEBUG_PRINT(A) Serial.println((unsigned long long) (A))

#define TOP_BAR_BG_COL 0xE3
#define FONT_COLOR 0x70
#define sleep(A) (vTaskDelay((A) / portTICK_PERIOD_MS))
#define BACKGROUND 0xFF

#define BLUE_BUTTON_HEIGHT 24
#define BL_BTN_X_PADDING 8
#define BL_BTN_Y_PADDING 7

#define GREY_BUTTON_HEIGHT 24
#define GREY_BUTTON_WIDTH 210
#define GR_BTN_X_PADDING 8
#define GR_BTN_Y_PADDING 7
#define GR_BTN_TXT_COLOR 0x70
#define GR_BTN_BG_COLOR 0xE3

#define TOP_BAR_HEIGHT 27
#define SCR_WIDTH 240
#define SCR_HEIGHT 320

// defines for top bar
#define LEFTMOST 17
#define TOPMOST 11

// left page padding
#define PG_LEFT_PADD 15

// vertical layout first item Y
#define MB_Y_START 41

//cursor color
#define CURCOL 0x70

// consts
const char* SMALLFONT = "SegoeUI-12";
const char* LARGEFONT = "SegoeUI-18";
const char* BOLDFONT = "SegoeUI-Bold-16";

// point to current language strings
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
		}

		void setCallback(void(*callback)())
		{
			_callback = callback;
		}

		//virtual void onClick()
		void onClick()
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

	protected:
		int _x = 0;
		int _y = 0;
		int _w = 0;
		int _h = 0;
		void (*_callback)() = nop;
		bool _isVisible = true;
		bool _isSelectable = false;
		bool _isPressed = false;
		bool _isSelected = false;
		bool _invalid = false;
		bool _isSquare = true;
		bool _isCircle = false;
		uint16_t _curCol = 0xffff;
};

// current selected item
static ScrObj* currItem = nullptr;
typedef std::vector<ScrObj*> obj_list;


class BlueTextButton: public ScrObj {

	public:
		virtual void freeRes() override
		{
		}

		virtual void draw() override
		{
			_h = BLUE_BUTTON_HEIGHT;
			if (_invalid) {
				tft.setTextColor(_fg, _bg);
				tft.loadFont(_fontName);
				_w = tft.textWidth(scrStrings[_index]) + _paddingX*2;
				tft.fillRect(_x, _y, _w, _h, _bg);
				tft.setCursor(_x+_paddingX, _y+_paddingY);
				//tft.print(_bText);
				tft.print(scrStrings[_index]);
				tft.unloadFont();
			}
			_invalid = false;
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
			_invalid = true;
			_isSelectable = true;
		}

		void setFont(const String& fontName)
		{
			_fontName = fontName;
			_invalid = true;
			_isSelectable = true;
		}

		void setText(dispStrings_t index)
		{
			//_bText = text;
			_index = index;
			_invalid = true;
			_isSelectable = true;
		}

	private:
		String _fontName;
		//String _bText;
		dispStrings_t _index;
		uint16_t _fg;
		uint16_t _bg;
		uint8_t _paddingX = BL_BTN_X_PADDING;
		uint8_t _paddingY = BL_BTN_Y_PADDING;
};


class GreyTextButton: public ScrObj {

	public:
		virtual void freeRes() override
		{
			_btnSpr.deleteSprite();
		}

		virtual void prepare() override
		{
			_w = GREY_BUTTON_WIDTH;
			_h = GREY_BUTTON_HEIGHT;
			if (_invalid) {
				//fontCol = greyscaleColor(GR_BTN_TXT_COLOR);
				//setColors(fontCol, greyscaleColor(0xE3));
				_btnSpr.createSprite(_w, _h);
				//_btnSpr.fillRect(_x, _y, _w, _h, _bg);
				_btnSpr.fillSprite(_bg);
				_btnSpr.loadFont(_fontName);
				_btnSpr.setTextColor(_fg, _bg);
				//_btnSpr.setCursor(_x+_paddingX, _y+_paddingY);
				_btnSpr.setCursor(_paddingX, _paddingY);
				//_btnSpr.print(_bText);
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

		void setFont(const String& fontName)
		{
			_fontName = fontName;
			_invalid = true;
			_isSelectable = true;
		}

		void setText(dispStrings_t index)
		{
			//_bText = text;
			_index = index;
			_invalid = true;
			_isSelectable = true;
		}

	private:
		String _fontName;
		//String _bText;
		dispStrings_t _index;
		uint16_t _fg;
		uint16_t _bg;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		TFT_eSprite _btnSpr = TFT_eSprite(&tft);
};


// print text to sprite
class Text: public ScrObj {

	public:
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
			_h = TOP_BAR_HEIGHT - 12;
			if (_invalid) {
				_txtSp.loadFont(_fontName);
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

		void setFont(const char* fontName)
		{
			// set font
			_fontName = fontName;
		}

		void setText(dispStrings_t index)
		{
			_index = index;
			//_text = text;
			_invalid = true;
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			// set colors
			_fg = fg;
			_bg = bg;
			_txtSp.setColorDepth(16);
		}

	private:
		dispStrings_t _index;
		uint16_t _fg, _bg;
		String _fontName;
		//const char* _text;
		uint8_t _paddingX = GR_BTN_X_PADDING;
		uint8_t _paddingY = GR_BTN_Y_PADDING;
		TFT_eSprite _txtSp = TFT_eSprite(&tft);
};


class BodyText: public ScrObj {

	public:
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
			_h = TOP_BAR_HEIGHT - 12;
			if (_invalid) {
				_txtSp.loadFont(_fontName);
				_w = _txtSp.textWidth(scrStrings[_index]) + _paddingX*2;
				_txtSp.createSprite(_w, _h);
				_txtSp.fillSprite(TFT_TRANSPARENT);
				_txtSp.setTextColor(_fg, _bg);
				//_txtSp.print(_text);
				_txtSp.print(scrStrings[_index]);
				_txtSp.unloadFont();
			}
		}

		void setFont(const char* fontName)
		{
			// set font
			_fontName = fontName;
		}

		void setText(dispStrings_t index)
		{
			_index = index;
			//_text = text;
			_invalid = true;
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			// set colors
			_fg = fg;
			_bg = bg;
			_txtSp.setColorDepth(16);
		}

	private:
		dispStrings_t _index;
		uint16_t _fg, _bg;
		String _fontName;
		//const char* _text;
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


class ImageButton: public ScrObj {

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
			_w = _h = 30;
			_invalid = true;
			_isSelectable = true;
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
		virtual void draw() override
		{
			_h = BLUE_BUTTON_HEIGHT;
			if (_invalid) {
				tft.setTextColor(_fg, _bg);
				tft.loadFont(_fontName);
				_w = tft.textWidth(_text) + _paddingX*2;
				tft.fillRect(_x, _y, _w, _h, _bg);
				tft.setCursor(_x+_paddingX, _y+_paddingY);
				//tft.print(_bText);
				tft.print(_text);
				tft.unloadFont();
			}
			_invalid = false;

		}

		virtual void freeRes() override
		{
		}

		void setColors(uint16_t fg, uint16_t bg)
		{
			_bg = bg;
			_fg = fg;
			_invalid = true;
			_isSelectable = true;
		}

		void setFont(const String& fontName)
		{
			_fontName = fontName;
			_invalid = true;
			_isSelectable = true;
		}

		void setText(const String& text)
		{
			_text = text;
			//_index = index;
			_invalid = true;
			_isSelectable = true;
		}

	private:
		String _fontName;
		String _text;
		//String _bText;
		//dispStrings_t _index;
		uint16_t _fg;
		uint16_t _bg;
		uint8_t _paddingX = BL_BTN_X_PADDING;
		uint8_t _paddingY = BL_BTN_Y_PADDING;
};

class CircRadBtn: public ScrObj {
	public:
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
		
		/*
		virtual void onClick() override
		{
			if (!_isOn)
				_callback();
		}
		*/

		virtual void prepare() override
		{
		}
		
		virtual uint16_t getCurCol() override
		{
			return greyscaleColor(GR_BTN_BG_COLOR);
		}

		void on(bool isOn)
		{
			_isOn = isOn;
			//TODO: move it somewhere else
			_isSelectable = true;
			_w = _h = 22;
		}

	private:
		bool _isOn = false;
		int _r = 5;
		uint16_t _col = 0xffff;
};


namespace Cursor {

	static bool _isCircle;
	static int _x;
	static int _y;
	static int _w;
	static int _h;

	static void draw()
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
	
	static void erase()
	{
		if (_isCircle) {
			tft.drawCircle(
					_x, 
					_y, 
					_h/2 + 1, 
					currItem->getCurCol()
					//greyscaleColor(BACKGROUND)
				      );

		}
		else {
			tft.drawRect(
					_x,
					_y,
					_w,
					_h,
					currItem->getCurCol()
					//greyscaleColor(BACKGROUND)
				    );
		}

	}

	static void setCoord(ScrObj* obj)
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

class App {
	public:
		App() {
			SPIFFS.begin();
			tft.init();
			tft.setRotation(0);
			tft.fillScreen(greyscaleColor(BACKGROUND));
		}
		~App(){}
};
