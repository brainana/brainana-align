#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define TEXTOFF (2)

// X Initialization for all windows
Display *XDisplay;
int XScreen;
Window XDisplay_Window;    // The root window, which covers the whole screen
int XDisplay_depth;
Colormap XColorMap;
Visual *XVisual;
Atom wm_delete_window;

// The image window and associated X characteristics
Window XWindow_image;   // The image window
Pixmap XPixmap_image, XPixmap_image_save;
Pixmap XPixmap_TopMenu, XPixmap_BottomMenu, XPixmap_ImageInfoArea, XPixmap_RightPanel;
// pixel arrays
XImage *XImage_XY, *XImage_ZY, *XImage_XZ, *XImage_YZ;
XImage *XImage_colorbar, *XImage_background, *XImage_TopMenu, *XImage_BottomMenu;
XImage *XImage_ImageInfoArea, *XImage_RightPanel;
XImage *XImage_lineX_cyan, *XImage_lineX_red, *XImage_lineY_cyan, *XImage_lineY_red;
XImage *XImage_lineY1_cyan, *XImage_lineY1_red;
XImage *XImage_lineZ1_cyan, *XImage_lineZ1_red, *XImage_lineZ2_cyan, *XImage_lineZ2_red;
char *XData_XY, *XData_ZY, *XData_XZ, *XData_YZ, *XData_colorbar, *XData_background;
char *XData_TopMenu, *XData_BottomMenu, *XData_ImageInfoArea, *XData_RightPanel;
char *XData_lineX_cyan,  *XData_lineX_red;
char *XData_lineY_cyan,  *XData_lineY_red,  *XData_lineY1_cyan, *XData_lineY1_red;
char *XData_lineZ1_cyan, *XData_lineZ1_red, *XData_lineZ2_cyan, *XData_lineZ2_red;
GC XGC_image;
GC XGC_cyan, XGC_red, XGC_green, XGC_blue, XGC_white;
GC XGC_purple, XGC_yellow, XGC_orange, XGC_black;
GC XGC_cyan_text, XGC_red_text, XGC_green_text, XGC_blue_text, XGC_white_text, XGC_purple_text;
GC XGC_yellow_text, XGC_orange_text, XGC_gray_text;
// The graph window and associated X characteristics
Window XWindow_graph;   // The image window
Pixmap XPixmap_GraphData, XPixmap_GraphInfoArea;
XImage *XImage_GraphData, *XImage_GraphInfoArea;
char *XData_GraphData, *XData_GraphInfoArea;
GC XGC_graph_red, XGC_graph_blue, XGC_graph_green, XGC_graph_white, XGC_graph_yellow;
GC XGC_graph_cyan, XGC_graph_gray, XGC_graph_orange, XGC_graph_purple;
GC XGC_graph_red_text, XGC_graph_blue_text, XGC_graph_green_text, XGC_graph_white_text, XGC_graph_yellow_text;
GC XGC_graph_cyan_text, XGC_graph_gray_text, XGC_graph_orange_text, XGC_graph_purple_text;
GC XGC_graph_event[9];

// prototypes
int get_next_event(XEvent *event);
int get_key_press(XKeyEvent *key_event, char *which_key);
int image_event_handler_general(XEvent *event);
int image_event_handler_specific(XEvent *event);
int graph_event_handler_general(XEvent *event);
int graph_event_handler_specific(XEvent *event);
GC *XGC_color_image( int color );
GC *XGC_color_graph( int color );
GC *XGC_color_image_text( int color );
GC *XGC_color_graph_text( int color );
void initialize_XImage( XImage *Image, unsigned long color );
