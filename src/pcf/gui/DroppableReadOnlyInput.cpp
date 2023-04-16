/**
 * @file DroppableReadOnlyInput.cpp
 * @author Daniel Starke
 * @copyright Copyright 2017-2023 Daniel Starke
 * @date 2017-12-01
 * @version 2017-12-05
 */
#include <cxxabi.h>
#include <cstring>
#include <FL/fl_draw.H>
#include <pcf/gui/DroppableReadOnlyInput.hpp>
#include <pcf/gui/Utility.hpp>


namespace pcf {
namespace gui {


/**
 * Constructor.
 * 
 * @param[in] X - x coordinate
 * @param[in] Y - y coordinate
 * @param[in] W - width
 * @param[in] H - height
 * @param[in] L - window label
 */
DroppableReadOnlyInput::DroppableReadOnlyInput(const int X, const int Y, const int W, const int H, const char * L):
	Fl_Input(X, Y, W, H, L)
{
	align(FL_ALIGN_LEFT);
	readonly(1);
	color(FL_BACKGROUND_COLOR);
	
}


/**
 * Destructor.
 */
DroppableReadOnlyInput::~DroppableReadOnlyInput() {
}


/**
 * Pastes the event text.
 */
void DroppableReadOnlyInput::paste() {
	char * files, * repl;
	const size_t size = size_t(Fl::event_length());
	files = reinterpret_cast<char *>(malloc(size + 1));
	if (files == NULL) return;
	strncpy(files, Fl::event_text(), size);
	files[size] = 0;
	/* take only the first file */
	for (repl = files; *repl != 0; repl++) {
		if (*repl == '\n') {
			*repl = 0;
			break;
		}
	}
	if (strncmp(files, "file://", 7) == 0) {
		this->value(files + 7);
	} else {
		this->value(files);
	}
	this->do_callback();
}


/**
 * Event handler.
 * 
 * @param[in] e - event 
 * @return 0 - if the event was not used or understood 
 * @return 1 - if the event was used and can be deleted
 */
int DroppableReadOnlyInput::handle(int e) {
	int result = 0;
	switch (e) {
	case FL_DND_ENTER:
	case FL_DND_DRAG:
	case FL_DND_LEAVE:
	case FL_DND_RELEASE:
		result = 1;
		break;
	case FL_PASTE:
		if ( Fl::event_inside(this->x(), this->y(), this->w(), this->h()) ) {
			this->paste();
			result = 1;
		}
		break;
	default:
		result = Fl_Input::handle(e);;
		break;
	}
	return result;
}


} /* namespace gui */
} /* namespace pcf */
