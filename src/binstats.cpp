/**
 * @file binstats.cpp
 * @author Daniel Starke
 * @copyright Copyright 2017-2023 Daniel Starke
 * @date 2017-12-01
 * @version 2017-12-03
 */
#include <FL/fl_ask.H>
#include <pcf/gui/SymbolViewer.hpp>
#include <pcf/gui/Utility.hpp>
#include "binstats.hpp"


int main() {
	Fl::visual(FL_DOUBLE | FL_RGB);
	Fl::set_color(FL_BACKGROUND_COLOR, 212, 208, 200);
	Fl::set_labeltype(FL_NO_SYMBOL_LABEL, pcf::gui::noSymLabelDraw, pcf::gui::noSymLabelMeasure);
	FL_NORMAL_SIZE = pcf::gui::adjDpiV(FL_NORMAL_SIZE);
	pcf::gui::SymbolViewer window(pcf::gui::adjDpiH(640), pcf::gui::adjDpiV(480), "binstats " BINSTATS_VERSION);
	window.show();
	return Fl::run();
}
