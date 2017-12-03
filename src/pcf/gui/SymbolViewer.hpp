/**
 * @file SymbolViewer.hpp
 * @author Daniel Starke
 * @copyright Copyright 2017 Daniel Starke
 * @date 2017-12-01
 * @version 2017-12-03
 */
#ifndef __PCF_GUI_SYMBOLVIEWER_HPP__
#define __PCF_GUI_SYMBOLVIEWER_HPP__

#include <cstring>
#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Table_Row.H>
#include <pcf/gui/DroppableReadOnlyInput.hpp>
#include <pcf/gui/Utility.hpp>


namespace pcf {
namespace gui {


/* forward declarations */
class InfoWindow;


/**
 * Viewer for binary symbols.
 */
class SymbolViewer : public Fl_Double_Window {
public:
	struct Statistics {
		char type;
		size_t size;
		size_t symbols;
		
		explicit Statistics(const char aType = '?', const size_t aSize = 0, const size_t aSymbols = 0):
			type(aType),
			size(aSize),
			symbols(aSymbols)
		{}
		
		const char * operator() (const size_t i, const Statistics & userData) const;
	};
	
	struct Symbol {
		char type;
		size_t size;
		char * name; /* automatically freed on destruction */
		
		explicit Symbol(const char aType = '?', const size_t aSize = 0, char * aName = NULL):
			type(aType),
			size(aSize),
			name(aName)
		{}
		
		Symbol(const Symbol & o):
			type(o.type),
			size(o.size),
			name(strdup(o.name))
		{}
		
		~Symbol() {
			if (this->name != NULL) free(this->name);
		}
		
		Symbol & operator= (const Symbol & o) {
			if (this != &o) {
				this->type = o.type;
				this->size = o.size;
				this->name = strdup(o.name);
			}
			return *this;
		}
		
		const char * operator() (const size_t i, const Statistics & userData) const;
	};
	typedef std::vector<Symbol> SymbolList;
private:
	char                    * baseLabel;
	SymbolList                symbolList;
	DroppableReadOnlyInput  * nmPath;
	Fl_Button               * browseNm;
	DroppableReadOnlyInput  * binPath;
	Fl_Button               * browseBin;
	Fl_Input                * pattern;
	Fl_Button               * info;
	Fl_Table_Row            * stats;
	Fl_Table_Row            * symbols;
	Fl_Native_File_Chooser  * chooseNm;
	Fl_Native_File_Chooser  * chooseBin;
	InfoWindow              * infoWin;
	char                    * currentNm;
	char                    * currentBin;
public:
	explicit SymbolViewer(const int W, const int H, const char * L = NULL);
	virtual ~SymbolViewer();
	
	const char * getNmPath() const { return this->nmPath->value(); }
	const char * getBinPath() const { return this->binPath->value(); }
	const char * getPattern() const { return this->pattern->value(); }
	void setNmPath(const char * val) { this->nmPath->value(val); this->read(); }
	void setBinPath(const char * val) { this->binPath->value(val); this->read(); }
	void setPattern(const char * val) { this->pattern->value(val); this->update(); }
protected:
	virtual int handle(int e);
private:
	PCF_GUI_BIND(SymbolViewer, onBrowseNm, Fl_Button)
	PCF_GUI_BIND(SymbolViewer, onChangeNm, Fl_Input)
	PCF_GUI_BIND(SymbolViewer, onBrowseBin, Fl_Button)
	PCF_GUI_BIND(SymbolViewer, onChangeBin, Fl_Input)
	PCF_GUI_BIND(SymbolViewer, onInformation, Fl_Button)
	PCF_GUI_BIND(SymbolViewer, onChangePattern, Fl_Input)
	
	void onBrowseNm(Fl_Button * button);
	void onChangeNm(Fl_Input * input);
	void onBrowseBin(Fl_Button * button);
	void onChangeBin(Fl_Input * input);
	void onInformation(Fl_Button * button);
	void onChangePattern(Fl_Input * input);
	
	void read();
	void update();
};


} /* namespace gui */
} /* namespace pcf */


#endif /* __PCF_GUI_SYMBOLVIEWER_HPP__ */
