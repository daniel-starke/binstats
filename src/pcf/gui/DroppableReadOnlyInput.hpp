/**
 * @file DroppableReadOnlyInput.hpp
 * @author Daniel Starke
 * @copyright Copyright 2017-2018 Daniel Starke
 * @date 2017-12-01
 * @version 2017-12-01
 */
#ifndef __PCF_GUI_DROPPABLEREADONLYINPUT_HPP__
#define __PCF_GUI_DROPPABLEREADONLYINPUT_HPP__

#include <FL/Fl_Input.H>


namespace pcf {
namespace gui {


/**
 * Droppable read-only input widget.
 */
class DroppableReadOnlyInput : public Fl_Input {
public:
	explicit DroppableReadOnlyInput(const int X, const int Y, const int W, const int H, const char * L = NULL);
	
	virtual ~DroppableReadOnlyInput();
	
	void paste();
protected:
	virtual int handle(int e);
};


} /* namespace gui */
} /* namespace pcf */


#endif /* __PCF_GUI_DROPPABLEREADONLYINPUT_HPP__ */
