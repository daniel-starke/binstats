/**
 * @file SymbolViewer.cpp
 * @author Daniel Starke
 * @copyright Copyright 2017-2023 Daniel Starke
 * @date 2017-12-01
 * @version 2023-04-16
 * @remarks nm -S --size-sort -f bsd -t d <file>
 */
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cxxabi.h>
#include <FL/filename.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Tile.H>
#include <pcf/gui/SymbolViewer.hpp>
#include <pcf/Utility.hpp>
#include <license.hpp>


extern "C" {
#include <errno.h>
#include <libpcf/fdios.h>
#ifdef PCF_IS_WIN
#include <libpcf/fdious.h>
#endif
}


namespace pcf {
namespace gui {


/**
 * Type mapping as reported from nm.
 * 
 * @see https://sourceware.org/binutils/docs/binutils/nm.html
 */
static const char * typeStr[] = {
	/* A */ "absolute value",
	/* B */ "uninitialized data (BSS)",
	/* C */ "uninitialized data (common)",
	/* D */ "initialized data",
	/* E */ "E",
	/* F */ "F",
	/* G */ "initialized data (small)",
	/* H */ "H",
	/* I */ "indirect function",
	/* J */ "J",
	/* K */ "K",
	/* L */ "L",
	/* M */ "M",
	/* N */ "debug",
	/* O */ "O",
	/* P */ "stack unwind",
	/* Q */ "Q",
	/* R */ "read only data",
	/* S */ "uninitialized data (small)",
	/* T */ "code",
	/* U */ "unique global",
	/* V */ "weak object",
	/* W */ "weak object (untagged)",
	/* X */ "X",
	/* Y */ "Y",
	/* Z */ "Z",
	/* ? */ "unknown"
};


/**
 * License information window.
 */
class LicenseInfoWindow : public Fl_Double_Window {
private:
	Fl_Text_Buffer * buffer;
	Fl_Text_Display * license;
public:
	explicit LicenseInfoWindow(const int W, const int H, const char * L = NULL):
		Fl_Double_Window(W, H, L),
		buffer(NULL),
		license(NULL)
	{
		const int spaceH = adjDpiH(10);
		const int spaceV = adjDpiV(10);
		const int widgetV = adjDpiV(20);
		const int licenseV = H - (3 * spaceV) - (2 * widgetV);
		const int okH = adjDpiH(60);
		int y = spaceV;
		
		buffer = new Fl_Text_Buffer();
		license = new Fl_Text_Display(spaceH, y + widgetV, W - (2 * spaceH), licenseV, "License");
		buffer->text(licenseText);
		license->buffer(buffer);
		license->hide_cursor();
		license->textfont(FL_COURIER);
		license->align(FL_ALIGN_TOP | FL_ALIGN_LEFT);
		license->box(FL_BORDER_BOX);
		license->textsize(8 * FL_NORMAL_SIZE / 10);
		license->scroll(0, 0); /* update scroll range according to content */
		y = license->y() + license->h() + spaceV;
		
		Fl_Button * ok = new Fl_Button((W - okH) / 2, y, okH, widgetV, "OK");
		ok->callback(PCF_GUI_CALLBACK(onOk), this);
		
		resizable(license);
		end();
	}
	
	virtual ~LicenseInfoWindow() {
		delete this->license;
		delete this->buffer;
	}
	
private:
	PCF_GUI_BIND(LicenseInfoWindow, onOk, Fl_Button);
	
	void onOk(Fl_Button * /* button */) {
		this->hide();
	}
};


/**
 * Type filter popup.
 */
class TypeFilterPopup : public Fl_Double_Window {
private:
	bool done;
	Fl_Scroll * scroll;
	Fl_Check_Button * filters[30];
public:
	explicit TypeFilterPopup(const int W, const int H, const char * L = NULL):
		Fl_Double_Window(W, H, L),
		done(false),
		scroll(NULL)
	{
		const int spaceH = adjDpiH(3);
		const int spaceV = adjDpiV(3);
		const int widgetV = adjDpiV(20);
		int y = spaceV;
		Fl_Box * sep;
		
		scroll = new Fl_Scroll(0, 0, W, H);
		scroll->box(FL_THIN_UP_BOX);
		scroll->scrollbar_size(adjDpiH(16));
		
		filters[0] = new Fl_Check_Button(spaceH, y, W - spaceH - scroll->scrollbar_size() - Fl::box_dw(scroll->box()), widgetV, "local");
		filters[0]->set();
		y += spaceV + widgetV;
		
		filters[1] = new Fl_Check_Button(spaceH, y, W - spaceH - scroll->scrollbar_size() - Fl::box_dw(scroll->box()), widgetV, "global");
		filters[1]->set();
		y += spaceV + widgetV;
		
		sep = new Fl_Box(Fl::box_dx(scroll->box()), y, W - scroll->scrollbar_size() - Fl::box_dw(scroll->box()), 2);
		sep->box(FL_THIN_DOWN_BOX);
		y += sep->h();
		
		for (size_t n = 0; n < 27; n++) {
			filters[n + 2] = new Fl_Check_Button(spaceH, y, W - spaceH - scroll->scrollbar_size() - Fl::box_dw(scroll->box()), widgetV, typeStr[n]);
			filters[n + 2]->set();
			y += spaceV + widgetV;
		}
		
		scroll->end();
		end();
		
		clear_border();
		set_modal();
		set_menu_window();
	}
	
	virtual ~TypeFilterPopup() {
		delete this->scroll;
	}
	
	void show() {
		/* grab this window (capture target, however, is the current topmost window) */
		Fl::grab(*this);
		/* the window is created from here on */
		this->Fl_Double_Window::show();
		this->done = false;
		while ( ! this->done ) Fl::wait();
		Fl::grab(NULL);
	}
	
	bool isSet(const int index) const {
		if (index < -2 || index > 26) return false;
		return this->filters[index + 2]->value() != 0;
	}
	
protected:
	/**
	 * Event handler.
	 * 
	 * @param[in] e - event 
	 * @return 0 - if the event was not used or understood 
	 * @return 1 - if the event was used and can be deleted
	 */
	int handle(int e) {
		int result = Fl_Double_Window::handle(e);
		switch (e) {
		case FL_UNFOCUS:
			if ( ! contains(Fl::focus()) ) {
				hide();
				result = 1;
			}
			break;
		case FL_PUSH:
			if ( ! Fl::event_inside(0, 0, w(), h()) ) {
				hide();
				result = 1;
			}
			break;
		case FL_KEYBOARD:
			switch (Fl::event_key()) {
			case FL_KP_Enter:
			case FL_Enter:
				do_callback();
				/* fall-through */
			case FL_Escape:
				hide();
				result = 1;
				break;
			default:
				break;
			}
			break;
		case FL_HIDE:
			done = true;
			result = 1;
			break;
		default:
			break;
		}
		return result;
	}
};


namespace {


/**
 * NULL pointer safe variant of strcmp;
 * 
 * @param[in] lhs - left hand statement
 * @param[in] rhs - right hand statement
 * @return see strcmp
 */
int nullSafeStrCmp(const char * lhs, const char * rhs) {
	if (lhs == NULL) return -1;
	if (rhs == NULL) return 1;
	return strcmp(lhs, rhs);
}


/**
 * Find first occurrence in str of any of the characters in find.
 * Returns NULL if nothing was found.
 * 
 * @param[in] str - search within this string
 * @param[in] find - find any of this
 * @return first occurrence of find in str or NULL
 */
const char * xstrpbrk(const char * str, const char * find) {
	if (str == NULL || find == NULL) return NULL;
	for (; *str != 0; str++) {
		if (strchr(find, *str) != NULL) return str;
	}
	return NULL;
}


/**
 * Find last occurrence in str of any of the characters in find.
 * Returns NULL if nothing was found.
 * 
 * @param[in] str - search within this string
 * @param[in] find - find any of this
 * @return last occurrence of find in str or NULL
 */
const char * xstrrpbrk(const char * str, const char * find) {
	if (str == NULL || find == NULL) return NULL;
	const char * res = NULL;
	for (; *str != 0; str++) {
		if (strchr(find, *str) != NULL) res = str;
	}
	return res;
}


/**
 * Less-than comparer for reverse order by size field.
 * 
 * @param[in] lhs - left hand statement
 * @param[in] rhs - right hand statement
 * @return true if less, else false
 */
template <typename T>
inline bool bySizeReverseOrder(const T & lhs, const T & rhs) {
	return lhs.size > rhs.size;
}


/**
 * Rounds the given float value to an integer value.
 * 
 * @param[in] val - value to round
 * @return rounded value
 */
inline int roundToInt(const float val) {
	return (val >= 0.0f) ? int(val + 0.5f) : int(val - 0.5f);
}


/**
 * Returns 1 if text matches pattern, else 0.
 * Valid characters for the matching string are:
 * @li * matches any character 0 to unlimited times
 * @li ? matches any character exactly once
 * @li # matches any digit exactly once
 * Any other character will be expected to be the same as in text to match.
 *
 * @param[in] text - text string
 * @param[in] pattern - wildcard matching string
 * @return 1 on match, else 0
 */
int matchPattern(const char * text, const char * pattern) {
	const char * curText; /* current text */
	const char * curPat; /* current pattern */
	const char * laText; /* look ahead text */
	const char * laPat; /* look ahead pattern */
	const char * sPat; /* saved pattern */
	if (text == NULL || pattern == NULL) return 0;
	curText = text;
	curPat = pattern;
	do {
		switch (*curPat) {
		case '*':
			laText = curText;
			laPat = curPat + 1;
			if (*laPat == 0) return 1;
			if (*laText == 0) {
				curPat = laPat;
				continue;
			}
			sPat = laPat;
			while (*sPat == '*') {
				curPat = sPat;
				sPat++;
			}
			if (*sPat == *curText || *sPat == '?' || (*sPat == '#' && isdigit(*curText))) {
				laPat = sPat;
				while (*laPat != '*' && *laPat != 0) {
					if (*laText == *laPat || *laPat == '?' || (*laPat == '#' && isdigit(*laText))) {
						laText++;
						laPat++;
					} else {
						curText++;
						break;
					}
					if (*laText == 0) {
						while (*laPat == '*') {
							laPat++;
						}
						if (*laPat == 0) {
							return 1;
						} else {
							curText++;
							break;
						}
					}
				}
				if (*laPat == '*') {
					curText = laText;
					sPat = laPat;
				} else {
					if (*laText == *laPat && *laPat == 0) {
						/* this point will never be reached (see previous return) */
						return 1;
					} else if (*laText == 0) {
						continue;
					} else if (*laPat == 0) {
						curText++;
						sPat = curPat;
					} else {
						sPat = curPat;
					}
				}
				curPat = sPat;
			} else {
				curText++;
			}
			break;
		case '?':
			if (*curText != 0) {
				curText++;
				curPat++;
				if (*curText != 0 && *curPat == 0) {
					return 0;
				}
			} else {
				return 0;
			}
			break;
		case '#':
			if ( isdigit(*curText) ) {
				curText++;
				curPat++;
				if (*curText != 0 && *curPat == 0) {
					return 0;
				}
			} else {
				return 0;
			}
			break;
		default:
			if (*curText == *curPat) {
				curText++;
				curPat++;
			} else {
				return 0;
			}
			break;
		}
	} while (*curPat != 0);
	if (*curText != 0) return 0;
	return 1;
}


/**
 * Helper class to couple data to the table view.
 * Changes in headerData and listData need to updated via redraw().
 * Changing the size of headerData and listData also needs to update the view via rows(newValue).
 * 
 * @tparam T - needs to implement const char * T::operator() (const size_t index, U & userData)
 * @tparam Fields - number of fields provided in T
 * @tparam U - user data type
 */
template <typename T, size_t Fields, typename U = bool>
class ListView : public DropForward<Fl_Table_Row> {
public:
	typedef std::vector<T> ListType;

	const char * headerData[Fields];
	ListType listData;
	U userData;
	
	explicit ListView(const int X, const int Y, const int W, const int H, const char * L = NULL):
		DropForward<Fl_Table_Row>(X, Y, W, H, L),
		listData(),
		userData()
	{
		end();
		for (size_t n = 0; n < Fields; n++) this->headerData[n] = NULL;
		table_box(FL_NO_BOX);
		col_header(1);
		col_resize(1);
		cols(Fields);
		scrollbar_size(adjDpiH(16));
		col_width_all((W - scrollbar_size() - Fl::box_dw(box())) / int(Fields));
		row_header(0);
		row_resize(0);
		rows(0);
		tab_cell_nav(0);
		type(SELECT_SINGLE);
		col_header_color(FL_BACKGROUND_COLOR);
		selection_color(FL_SELECTION_COLOR);
	}
	
	virtual ~ListView() {}
	
	virtual void resize(int X, int Y, int W, int H) {
		/* also resize last field accordingly */
		if (W != w()) {
			int lastX = 0;
			for (size_t n = 0; (n + 1) < Fields; n++) {
				lastX += this->col_width(int(n));
			}
			if (W < w() || (lastX + this->col_width(int(Fields) - 1) + this->scrollbar_size() + Fl::box_dx(box())) < w()) {
				this->col_width(int(Fields) - 1, std::max(this->scrollbar_size() + Fl::box_dx(box()), this->col_width(int(Fields) - 1) + W - w()));
			}
		}
		
		this->DropForward<Fl_Table_Row>::resize(X, Y, W, H);
	}
	
	void update() {
		this->rows(int(this->listData.size()));
		this->row_height_all(this->labelsize() + 4);
		this->redraw();
	}
	
protected:
	virtual void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0) {
		const int spaceH = int(adjDpiH(5));
		switch (context) {
		case CONTEXT_COL_HEADER:
			fl_push_clip(X, Y, W, H);
			fl_draw_box(FL_UP_BOX, X, Y, W, H, col_header_color());
			if (this->headerData[size_t(C)] != NULL) {
				fl_color(FL_BLACK);
				fl_draw(this->headerData[size_t(C)], X + spaceH, Y, W, H, FL_ALIGN_LEFT, NULL, 0);
			}
			fl_pop_clip();
			break;
		case CONTEXT_ROW_HEADER:
			/* empty, not supported */
			fl_color(row_header_color());
			fl_rectf(X, Y, W, H);
			break;
		case CONTEXT_CELL:
			fl_push_clip(X, Y, W, H);
			
			/* draw background */
			fl_color(row_selected(R) ? selection_color()  : FL_WHITE);
			fl_rectf(X, Y, W, H);
			
			/* draw text */
			if (this->listData[size_t(R)](size_t(C), this->userData) != NULL) {
				fl_color(row_selected(R) ? fl_contrast(selection_color(), FL_FOREGROUND_COLOR) : FL_FOREGROUND_COLOR);
				fl_draw(this->listData[size_t(R)](size_t(C), this->userData), X + spaceH, Y, W, H, FL_ALIGN_LEFT, NULL, 0);
			}
			
			/* draw borders */
			fl_color(color()); 
			fl_rect(X, Y, W, H);
			
			fl_pop_clip();
			break;
		default:
			break;
		}
	}
};


typedef ListView<SymbolViewer::Statistics, 3, SymbolViewer::Statistics> StatsListView;
typedef ListView<SymbolViewer::Symbol, 3, SymbolViewer::Statistics> SymsListView;


/**
 * Helper class to enable Fl_Input to call its callback on change.
 */
class TriggerHappyInput : public Fl_Input {
public:
	explicit TriggerHappyInput(const int X, const int Y, const int W, const int H, const char * L = NULL):
		Fl_Input(X, Y, W, H, L)
	{}
	
	virtual ~TriggerHappyInput() {}
	
	virtual int handle(int e) {
		const int result = Fl_Input::handle(e);
		if (e == FL_KEYBOARD) this->do_callback();
		return result;
	}
};


} /* anonymous namespace */


/**
 * Returns the string representation of the requested field index.
 * 
 * @param[in] i - field index
 * @param[in] userData - reference values
 * @return string
 */
const char * SymbolViewer::Statistics::operator() (const size_t i, const Statistics & userData) const {
	static char buffer[32];
	switch (i) {
	case 0:
		if (isalpha(this->type)) {
			return typeStr[toupper(this->type) - 'A'];
		} else if (this->type == '_') {
			return "total";
		} else {
			return "unknown";
		}
		break;
	case 1:
		{
			const float percent = 100.0f * float(this->size) / float(userData.size);
			snprintf(buffer, sizeof(buffer), "%lld (%i%%)", this->size, roundToInt(percent));
		}
		buffer[31] = 0;
		return buffer;
		break;
	case 2:
		{
			const float percent = 100.0f * float(this->symbols) / float(userData.symbols);
			snprintf(buffer, sizeof(buffer), "%llu (%i%%)", static_cast<unsigned long long>(this->symbols), roundToInt(percent));
		}
		buffer[31] = 0;
		return buffer;
		break;
	default:
		return NULL;
		break;
	}
}


/**
 * Returns the string representation of the requested field index.
 * 
 * @param[in] i - field index
 * @param[in] userData - reference values
 * @return string
 */
const char * SymbolViewer::Symbol::operator() (const size_t i, const Statistics & userData) const {
	static char buffer[32];
	switch (i) {
	case 0:
		if (isalpha(this->type)) {
			return typeStr[toupper(this->type) - 'A'];
		} else if (this->type == '_') {
			return "total";
		} else {
			return "unknown";
		}
		break;
	case 1:
		{
			const float percent = 100.0f * float(this->size) / float(userData.size);
			snprintf(buffer, sizeof(buffer), "%lld (%i%%)", this->size, roundToInt(percent));
		}
		buffer[31] = 0;
		return buffer;
		break;
	case 2:
		return this->name;
		break;
	default:
		return NULL;
		break;
	}
}


/**
 * Constructor.
 * 
 * @param[in] W - width
 * @param[in] H - height
 * @param[in] L - window label
 */
SymbolViewer::SymbolViewer(const int W, const int H, const char * L):
	Fl_Double_Window(W, H, L),
	baseLabel(NULL),
	nmPath(NULL),
	browseNm(NULL),
	binPath(NULL),
	browseBin(NULL),
	pattern(NULL),
	stats(NULL),
	symbols(NULL),
	chooseNm(NULL),
	chooseBin(NULL),
	licenseWin(NULL),
	typeFilter(NULL),
#ifdef PCF_IS_WIN
	currentNm(strdup("nm.exe")),
#else
	currentNm(strdup("nm")),
#endif
	currentBin(NULL)
{
	const int spaceH  = adjDpiH(10); /* horizontal spacing */
	const int spaceV  = adjDpiV(10); /* vertical spacing */
	const int labelH  = adjDpiH(50);
	const int browseH = adjDpiH(30);
	const int inputV  = adjDpiV(20);
	int y = spaceH;
	Fl_Group * g; /* group widget for resize adjustments */
	
	char * nmFromEnv = fl_getenv("NM");
	if (nmFromEnv != NULL && *nmFromEnv != 0) currentNm = strdup(nmFromEnv);
	
	if (L != NULL) this->baseLabel = strdup(L);
	
	g = new DropForward<Fl_Group>(spaceH, y, W - (2 * spaceH), inputV);
	nmPath = new DroppableReadOnlyInput(spaceH + labelH, y, W - (2 * spaceH) - browseH - labelH - 2, inputV, "nm");
	nmPath->value(currentNm);
	nmPath->callback(PCF_GUI_CALLBACK(onChangeNm), this);
	nmPath->when(FL_WHEN_CHANGED);
	browseNm = new DropForward<Fl_Button>(W - spaceH - browseH, y, browseH, inputV, "...");
	browseNm->callback(PCF_GUI_CALLBACK(onBrowseNm), this);
	g->resizable(nmPath);
	g->end();
	y += inputV;
	
	g = new DropForward<Fl_Group>(spaceH, y, W - (2 * spaceH), inputV);
	binPath = new DroppableReadOnlyInput(spaceH + labelH, y, W - (2 * spaceH) - browseH - labelH - 2, inputV, "binary");
	binPath->callback(PCF_GUI_CALLBACK(onChangeBin), this);
	binPath->when(FL_WHEN_CHANGED);
	browseBin = new DropForward<Fl_Button>(W - spaceH - browseH, y, browseH, inputV, "...");
	browseBin->callback(PCF_GUI_CALLBACK(onBrowseBin), this);
	g->resizable(binPath);
	g->end();
	y += spaceH + inputV;
	
	g = new DropForward<Fl_Group>(spaceH, y, W - (2 * spaceH), inputV);
	pattern = new TriggerHappyInput(spaceH + labelH, y, W - (2 * spaceH) - browseH - labelH - 2, inputV, "pattern");
	pattern->tooltip("* matches any character 0 to unlimited times\n? matches any character exactly once\n# matches any digit exactly once");
	pattern->callback(PCF_GUI_CALLBACK(onChangePattern), this);
	info = new DropForward<Fl_Button>(W - spaceH - browseH, y, browseH, inputV, "@#menu");
	info->labelcolor(fl_lighter(FL_RED));
	info->callback(PCF_GUI_CALLBACK(onInformation), this);
	g->resizable(pattern);
	g->end();
	y += spaceH + inputV;
	
	Fl_Tile * tile = new DropForward<Fl_Tile>(spaceH, y, W - (2 * spaceH), H - y - spaceV);
	Fl_Box * sizeLimiter = new DropForward<Fl_Box>(tile->x(), tile->y() + (3 * spaceV), tile->w(), tile->h() - (6 * spaceV));
	sizeLimiter->hide();
	tile->resizable(*sizeLimiter);
	
	stats = new StatsListView(tile->x(), tile->y(), tile->w(), tile->h() / 2);
	static_cast<StatsListView *>(stats)->headerData[0] = "Type";
	static_cast<StatsListView *>(stats)->headerData[1] = "Size";
	static_cast<StatsListView *>(stats)->headerData[2] = "Symbols";
	static_cast<StatsListView *>(stats)->callback(PCF_GUI_CALLBACK(onTableEvent), this);
	
	symbols = new SymsListView(tile->x(), tile->y() + (tile->h() / 2), tile->w(), tile->h() / 2);
	static_cast<SymsListView *>(symbols)->headerData[0] = "Type";
	static_cast<SymsListView *>(symbols)->headerData[1] = "Size";
	static_cast<SymsListView *>(symbols)->headerData[2] = "Symbol";
	static_cast<SymsListView *>(symbols)->callback(PCF_GUI_CALLBACK(onTableEvent), this);
	
	tile->end();
	
	/* set minimal window size */
	size_range(adjDpiH(320), adjDpiV(240));
	resizable(tile);
	
	end();
	
	/* open nm file dialog */
	chooseNm = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	chooseNm->title("Choose NM..");
	chooseNm->directory(".");
#ifdef PCF_IS_WIN
	chooseNm->filter("nm Binary\t*.exe");
#else
	chooseNm->filter("nm Binary\t*");
#endif
	
	/* open binary file dialog */
	chooseBin = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_FILE);
	chooseBin->title("Choose binary..");
	chooseBin->directory(".");
#ifdef PCF_IS_WIN
	chooseBin->filter("Binary\t*.{exe,dll,a,o}");
#else
	chooseBin->filter("Binary\t*");
#endif

	licenseWin = new LicenseInfoWindow(adjDpiH(560), adjDpiV(600), "About binstats " BINSTATS_VERSION);
	
	typeFilter = new TypeFilterPopup(adjDpiH(240), adjDpiV(320));

	Fl::focus(pattern);
}


/**
 * Destructor.
 */
SymbolViewer::~SymbolViewer() {
	if (this->baseLabel != NULL) free(this->baseLabel);
	delete this->nmPath;
	delete this->browseNm;
	delete this->binPath;
	delete this->browseBin;
	delete this->pattern;
	delete this->info;
	delete this->stats;
	delete this->symbols;
	delete this->chooseNm;
	delete this->chooseBin;
	delete this->licenseWin;
	delete this->typeFilter;
	if (this->currentNm != NULL) free(this->currentNm);
	if (this->currentBin != NULL) free(this->currentBin);
}


/**
 * Event handler.
 * 
 * @param[in] e - event 
 * @return 0 - if the event was not used or understood 
 * @return 1 - if the event was used and can be deleted
 */
int SymbolViewer::handle(int e) {
	int result = Fl_Double_Window::handle(e);
	switch (e) {
	case FL_DND_ENTER:
	case FL_DND_DRAG:
	case FL_DND_LEAVE:
	case FL_DND_RELEASE:
		result = 1;
		break;
	case FL_PASTE:
		binPath->paste();
		this->read();
		result = 1;
		break;
	case FL_SHORTCUT:
		if (Fl::event_key() == (FL_F + 5)) {
			this->read(true);
			result = 1;
		}
		break;
	default:
		break;
	}
	return result;
}


void SymbolViewer::onBrowseNm(Fl_Button * /* button */) {
	if (this->chooseNm->show() == 0) {
		this->nmPath->value(this->chooseNm->filename());
		this->read();
	}
}


void SymbolViewer::onChangeNm(Fl_Input * /* input */) {
	this->read();
}


void SymbolViewer::onBrowseBin(Fl_Button * /* button */) {
	if (this->chooseBin->show() == 0) {
		this->binPath->value(this->chooseBin->filename());
		this->read();
	}
}


void SymbolViewer::onChangeBin(Fl_Input * /* input */) {
	this->read();
}


void SymbolViewer::onInformation(Fl_Button * /* button */) {
	this->licenseWin->show();
}


void SymbolViewer::onChangePattern(Fl_Input * /* input */) {
	this->update();
}


void SymbolViewer::onTableEvent(Fl_Table_Row * /* table */) {
	if (Fl::event() != FL_PUSH || Fl::event_button() != FL_RIGHT_MOUSE) return;
	this->typeFilter->position(Fl::event_x_root(), Fl::event_y_root());
	this->typeFilter->show();
	this->update();
}


/**
 * Reads the symbol list from the binary file and updates the tables if possible.
 * 
 * @param[in] force - forces a fresh read
 */
void SymbolViewer::read(const bool force) {
	static const char * compilerAttrSuffix[] = {
		".constprop.",
		".lto_priv.",
		NULL
	};
	struct stat fileInfo[1];
	/* check values */
	if (this->nmPath->value() == NULL) return;
	if (this->nmPath->value()[0] == 0) return;
	if (this->binPath->value() == NULL) return;
	if (this->binPath->value()[0] == 0) return;
	/* check if we need to read the data again */
	if (force == false && nullSafeStrCmp(this->nmPath->value(), this->currentNm) == 0 && nullSafeStrCmp(this->binPath->value(), this->currentBin) == 0) return;
	/* update current path */
	if (this->currentNm != NULL) free(this->currentNm);
	this->currentNm = strdup(this->nmPath->value());
	if (this->currentBin != NULL) free(this->currentBin);
	this->currentBin = strdup(this->binPath->value());
	/* check paths */
	if (fl_stat(this->nmPath->value(), fileInfo) < 0) {
		fl_message_title("Error");
		fl_alert("Error reading \"%s\".\n%s.", this->nmPath->value(), strerror(errno));
		return;
	}
	if (fileInfo->st_size <= 0) return;
	if (fl_stat(this->binPath->value(), fileInfo) < 0) {
		fl_message_title("Error");
		fl_alert("Error reading \"%s\".\n%s.", this->binPath->value(), strerror(errno));
		return;
	}
	if (fileInfo->st_size <= 0) return;
#ifdef PCF_IS_WIN
	/* convert nm path to UTF-16 */
	const size_t nmPathLen = strlen(this->nmPath->value());
	const unsigned nmPathWLen = fl_utf8toUtf16(this->nmPath->value(), static_cast<unsigned>(nmPathLen), NULL, 0) + 1;
	wchar_t * nmPathW = static_cast<wchar_t *>(malloc(sizeof(wchar_t) * nmPathWLen));
	if (nmPathW == NULL) return;
	nmPathW[fl_utf8toUtf16(this->nmPath->value(), static_cast<unsigned>(nmPathLen), reinterpret_cast<unsigned short *>(nmPathW), nmPathWLen)] = 0;
	/* convert binary path to UTF-16 */
	const size_t binPathLen = strlen(this->binPath->value());
	const unsigned binPathWLen = fl_utf8toUtf16(this->binPath->value(), static_cast<unsigned>(binPathLen), NULL, 0) + 1;
	wchar_t * binPathW = static_cast<wchar_t *>(malloc(sizeof(wchar_t) * binPathWLen));
	if (binPathW == NULL) {
		free(nmPathW);
		return;
	}
	binPathW[fl_utf8toUtf16(this->binPath->value(), static_cast<unsigned>(binPathLen), reinterpret_cast<unsigned short *>(binPathW), binPathWLen)] = 0;
	/* build command-line */
	const wchar_t * cmdW[] = {
		nmPathW + fl_utf8toUtf16(this->nmPath->value(), static_cast<unsigned>(nmPathLen - strlen(fl_filename_name(this->nmPath->value()))), NULL, 0),
		L"-S",
		L"--size-sort",
		L"-f",
		L"bsd",
		L"-t",
		L"d",
		binPathW,
		NULL
	};
	/* call nm */
	tFdioPHandle * nm = fdious_popen(nmPathW, cmdW, NULL, NULL, tFdioPMode(FDIO_USE_STDOUT | FDIO_COMBINE));
#else /* not Windows */
	/* build command-line */
	const char * cmd[] = {
		fl_filename_name(this->nmPath->value()),
		"-S",
		"--size-sort",
		"-f",
		"bsd",
		"-t",
		"d",
		this->binPath->value(),
		NULL
	};
	/* call nm */
	tFdioPHandle * nm = fdios_popen(this->nmPath->value(), cmd, NULL, NULL, tFdioPMode(FDIO_USE_STDOUT | FDIO_COMBINE));
#endif /* not Windows */
	/* parse nm output */
	if (nm != NULL) {
		char * line = NULL;
		char * firstLine = NULL;
		char * next = NULL;
		int len, lineSize;
		this->symbolList.clear();
		while ( ! feof(nm->out) ) {
			len = fdios_getline(&line, &lineSize, nm->out);
			for (len--; (line[len] == '\n' || line[len] == '\r') && len > 0; len--);
			if (len == 0) continue; /* ignore empty lines */
			line[++len] = 0;
			if (firstLine == NULL) firstLine = strdup(line);
			/* parse line fields */
			const long long int value = strtoll(line, &next, 10);
			if (value == 0 && *next != ' ') continue;
			next++;
			const long long int size = strtoll(next, &next, 10);
			if (size == 0 && *next != ' ') continue;
			next++;
			const char type = *next;
			next++;
			if ((!isalpha(type) && type != '?') || *next != ' ') continue;
			next++;
			const char * name = next;
			/* demangle symbol */
			char * compilerAttr = NULL;
			size_t compilerAttrLen = 0;
			for (const char ** cas = compilerAttrSuffix; *cas != NULL; cas++) {
				char * attribute = strstr(next, *cas);
				if (attribute != NULL) {
					compilerAttr = strdup(attribute + 1);
				}
				if (compilerAttr != NULL) {
					attribute[0] = 0;
					compilerAttrLen = strlen(compilerAttr) + 1;
					break;
				}
			}
			const char * symStart = xstrrpbrk(name, ".$");
			if (symStart == NULL) {
				symStart = name;
			} else {
				symStart++;
			}
			int status;
			char * realSymName = abi::__cxa_demangle(symStart, 0, 0, &status);
			if (status == 0) {
				/* got demangled name */
				if (symStart == name) {
					if (compilerAttr == NULL) {
						const Symbol entry(type, size, strdup(realSymName));
						this->symbolList.push_back(entry);
					} else {
						const size_t realSymLen = strlen(realSymName);
						char * newName = static_cast<char *>(realloc(realSymName, sizeof(char) * (realSymLen + compilerAttrLen + 1)));
						if (newName != NULL && newName != realSymName) {
							realSymName = newName;
							realSymName[realSymLen] = '.';
							memcpy(realSymName + realSymLen + 1, compilerAttr, compilerAttrLen);
							const Symbol entry(type, size, strdup(realSymName));
							this->symbolList.push_back(entry);
						}
					}
				} else {
					const size_t realSymLen = strlen(realSymName);
					char * newName = static_cast<char *>(realloc(realSymName, sizeof(char) * (realSymLen + compilerAttrLen + symStart - name + 1)));
					if (newName != NULL && newName != realSymName) {
						realSymName = newName;
						memmove(realSymName + (symStart - name), realSymName, sizeof(char) * (realSymLen + 1));
						memcpy(realSymName, name, sizeof(char) * (symStart - name));
						if (compilerAttr != NULL) {
							realSymName[realSymLen + symStart - name] = '.';
							memcpy(realSymName + realSymLen + (symStart - name) + 1, compilerAttr, compilerAttrLen);
						}
						const Symbol entry(type, size, strdup(realSymName));
						this->symbolList.push_back(entry);
					}
				}
			} else {
				/* failed to demangle symbol */
				if (compilerAttr == NULL) {
					const Symbol entry(type, size, strdup(name));
					this->symbolList.push_back(entry);
				} else {
					const size_t realSymLen = strlen(name);
					char * newName = static_cast<char *>(malloc(sizeof(char) * (realSymLen + compilerAttrLen + 1)));
					if (newName != NULL) {
						realSymName = newName;
						realSymName[realSymLen] = '.';
						memcpy(realSymName, name, realSymLen);
						memcpy(realSymName + realSymLen + 1, compilerAttr, compilerAttrLen);
						const Symbol entry(type, size, strdup(realSymName));
						this->symbolList.push_back(entry);
					}
				}
			}
			if (realSymName != NULL) free(realSymName);
			if (compilerAttr != NULL) free(compilerAttr);
		}
		if ( this->symbolList.empty() ) {
			fl_message_title("Error");
			fl_alert("Failed to read symbols from \"%s\".\n%s", this->binPath->value(), firstLine != NULL ? firstLine : "");
		}
		if (firstLine != NULL) free(firstLine);
		if (line != NULL) free(line);
	}
#ifdef PCF_IS_WIN
	if (nm != NULL) fdious_pclose(nm);
	free(nmPathW);
	free(binPathW);
#else
	if (nm != NULL) fdios_pclose(nm);
#endif
	/* sort by size in descending order */
	std::stable_sort(this->symbolList.begin(), this->symbolList.end(), bySizeReverseOrder<Symbol>);
	/* update view data */
	this->update();
}


/**
 * Updates the symbol tables.
 */
void SymbolViewer::update() {
	StatsListView & statsView = *static_cast<StatsListView *>(this->stats);
	SymsListView & symsView = *static_cast<SymsListView *>(this->symbols);
	SymbolList::const_iterator sym, symEnd = this->symbolList.end();
	
	const char * pat = this->pattern->value();
	Statistics newStats[27];
	bool filter[27];
	Statistics * aStat;
	const bool local = this->typeFilter->isSet(-2);
	const bool global = this->typeFilter->isSet(-1);
	
	for (size_t n = 0; n < 26; n++) {
		newStats[n] = Statistics(char('A' + n));
		filter[n] = this->typeFilter->isSet(int(n));
	}
	newStats[26] = Statistics();
	filter[26] = this->typeFilter->isSet(26);
	
	statsView.listData.clear();
	symsView.listData.clear();
	statsView.userData = Statistics('_');
	
	/* create filtered lists */
	for (sym = this->symbolList.begin(); sym != symEnd; ++sym) {
		/* skip if pattern does not match (or substring not included if no real pattern given) */
		if (pat != NULL && *pat != 0) {
			if (xstrpbrk(pat, "*?#") == NULL) {
				if (strstr(sym->name, pat) == NULL) continue;
			} else if ( ! matchPattern(sym->name, pat) ) {
				continue;
			}
		}
		aStat = NULL;
		if ( isalpha(sym->type) ) {
			/* filter by type */
			if ( ! filter[int(toupper(sym->type) - 'A')] ) continue;
			if (islower(sym->type) && !local) continue;
			if (isupper(sym->type) && !global) continue;
			aStat = newStats + int(toupper(sym->type) - 'A');
		} else if (sym->type == '?') {
			/* filter by type */
			if ( ! filter[26] ) continue;
			aStat = newStats + 26;
		}
		symsView.listData.push_back(*sym);
		if (sym->size > 0) {
			statsView.userData.size += sym->size;
		}
		statsView.userData.symbols++;
		if (aStat != NULL) {
			aStat->size += sym->size;
			aStat->symbols++;
		}
	}
	symsView.userData = statsView.userData;
	
	if ( ! symsView.listData.empty() ) statsView.listData.push_back(statsView.userData);
	for (size_t n = 0; n < 27; n++) {
		if (newStats[n].symbols <= 0) continue;
		statsView.listData.push_back(newStats[n]);
	}
	
	std::stable_sort(statsView.listData.begin(), statsView.listData.end(), bySizeReverseOrder<Statistics>);
	
	statsView.update();
	symsView.update();
}


} /* namespace gui */
} /* namespace pcf */
