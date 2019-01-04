/**
 * @file Utility.hpp
 * @author Daniel Starke
 * @copyright Copyright 2017-2019 Daniel Starke
 * @date 2017-04-10
 * @version 2017-12-01
 */
#ifndef __PCF_GUI_UTILITY_HPP__
#define __PCF_GUI_UTILITY_HPP__

#include <FL/Fl.H>


namespace pcf {
namespace gui {


/** Label type to disable symbols. Initialize with Fl::set_labeltype(FL_NO_SYMBOL_LABEL, noSymLabelDraw, noSymLabelMeasure) */
#define FL_NO_SYMBOL_LABEL FL_FREE_LABELTYPE


/**
 * Binds the given class function with the passed widget type.
 * 
 * @param[in] class - current class
 * @param[in] func - function to bind
 * @param[in] type - widget type
 */
#define PCF_GUI_BIND(class, func, type) \
	static inline void func##_callback(Fl_Widget * w, void * user) { \
		if (w == NULL || user == NULL) return; \
		type * widget = static_cast<type *>(w); (void)widget; \
		class * self = static_cast<class *>(user); \
		self->func(widget); \
	}


/**
 * Returns the binding function for the given function binding.
 * 
 * @param[in] func - function bound
 */
#define PCF_GUI_CALLBACK(func) func##_callback


void noSymLabelDraw(const Fl_Label * o, int X, int Y, int W, int H, Fl_Align align);
void noSymLabelMeasure(const Fl_Label * o, int & W, int & H);
int adjDpiH(const int val, const int screen = 0);
int adjDpiV(const int val, const int screen = 0);


/**
 * Helper class to forward drag events to the parent widget.
 */
template <typename Widget>
class DropForward : public Widget {
private:
	bool pasting;
public:
	explicit DropForward(const int X, const int Y, const int W, const int H, const char * L = NULL):
		Widget(X, Y, W, H, L),
		pasting(false)
	{}
	virtual ~DropForward() {}
protected:
	virtual int handle(int e) {
		int result = 0;
		switch (e) {
		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_LEAVE:
		case FL_DND_RELEASE:
			result = 1;
			break;
		case FL_PASTE:
			/* forward event to parent */
			if ( ! this->pasting ) {
				/* prevent event passing from parent to this widget */
				this->pasting = true;
				for (Fl_Widget * p = this->parent(); p != NULL && p != this; p = p->parent()) {
					result = p->handle(FL_PASTE);
					if (result != 0) break;
				}
				this->pasting = false;
			}
			break;
		default:
			result = Widget::handle(e);
			break;
		}
		return result;
	}
};


} /* namespace gui */
} /* namespace pcf */


#endif /* __PCF_GUI_UTILITY_HPP__ */
