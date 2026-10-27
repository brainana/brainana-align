#ifndef JIP_LEGACY_STORAGE
#define JIP_LEGACY_STORAGE
#endif
// Copyright (c) The General Hospital Corporation
// All rights reserved.
// 
// This agreement ("Software License") covers downloads from the JIP fMRI Analysis Toolkit
// maintained by The General Hospital Corporation. Your downloading, copying, modifying,
// distributing, displaying, or use of any software and/or data from JIP fMRI Analysis Toolkit
// constitutes acceptance of all of the terms and conditions of this Agreement. If you do not
// agree to such terms and conditions, you have no right to download, copy, modify, distribute,
// display, or use the Software.
// 
// 1. As used in this Software License,
// "you" means the individual downloading and/or using, reproducing, modifying, and/or displaying
// the Software and the institution or entity which employs or is otherwise affiliated with such
// individual in connection therewith; The General Hospital Corporation d/b/a Massachusetts
// General Hospital ("Hospital") hereby grants you with respect to Hospital's rights in the
// software, and data, if any, which is the subject of this software license (collectively,
// the "Software"), a royalty-free, non-exclusive license under Hospital's copyright rights,
// to redistribute, reproduce, modify, and display the Software (in source or binary forms),
// provided that: 
// 
// (a) you accept and adhere to all of the terms and conditions of this Software License; 
// 
// (b) redistributions of source code must retain the above copyright notice, this list of conditions
// and the following disclaimer;
// 
// (c) redistributions in binary form must reproduce the above copyright notice, this list of conditions
// and the following disclaimer in the documentation and/or other materials provided with distribution;
// 
// 2. The Software is provided "AS IS" and neither Hospital nor any contributor to the software
// (each a "Contributor") shall have any obligation to provide maintenance, support, updates,
// enhancements or modifications thereto. HOSPITAL AND ALL CONTRIBUTORS SPECIFICALLY DISCLAIM
// ALL EXPRESS AND IMPLIED WARRANTIES OF ANY KIND INCLUDING, BUT NOT LIMITED TO, ANY WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
// HOSPITAL OR ANY CONTRIBUTOR BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL,
// EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY ARISING IN ANY WAY RELATED TO THE SOFTWARE, EVEN IF HOSPITAL OR ANY
// CONTRIBUTOR HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. TO THE MAXIMUM EXTENT NOT
// PROHIBITED BY LAW OR REGULATION, YOU FURTHER ASSUME ALL LIABILITY FOR YOUR USE, REPRODUCTION,
// DISPLAY, LICENSE OR DISTRIBUTION OF THE SOFTWARE AND AGREE TO INDEMNIFY AND HOLD HARMLESS HOSPITAL
// AND ALL CONTRIBUTORS FROM AND AGAINST ANY AND ALL CLAIMS, SUITS, ACTIONS, DEMANDS AND JUDGMENTS
// ARISING THEREFROM. 
// 
// 3. None of the names, logos or trademarks of Hospital or any of Hospital's affiliates or any
// of the Contributors, or any funding agency, may be used to endorse or promote products produced
// in whole or in part by operation of the Software or derived from or based on the Software without
// specific prior written permission from the applicable party.  In the case of Hospital, permission
// needs to be obtained from its Public Affairs Office.
// 
// 4. You agree to provide acknowledgement to Hospital when you publish and present results of your
// use of the Software in peer-reviewed journals in accordance with standard academic practice. 
// 
// 5. This Software License does not grant any rights with respect to third party software,
// except those rights that Hospital has been authorized by a third party to grant to you,
// and accordingly you are solely responsible for obtaining any permissions from third parties
// that you need to use, reproduce, distribute, modify, and display the Software. 
// 
// 6. The Software has been designed for research purposes only and has not been reviewed or approved
// by the Food and Drug Administration or by any other agency. YOU ACKNOWLEDGE AND AGREE THAT
// CLINICAL APPLICATIONS ARE NEITHER RECOMMENDED NOR ADVISED. You further agree to use, reproduce,
// and display the Software in compliance with all applicable governmental laws, regulations and orders,
// including without limitation those relating to export and import control.



#define BLACK_PIXEL 0
#define WHITE_PIXEL 255
#define INFO_HEIGHT (15)
#define NBORDER 1          // # of pixels to use as a border between mosaic panels

// General cursor locations for image window or graph window
// Bit-tricks are used so that multiple values can be set at the same time.
#define CURSOR_ON_WINDOW          (1<<0)  // anywhere on image/graph window
#define CURSOR_ON_DATA            (1<<1)  // on image/graph window, on data area
#define CURSOR_ON_SELECT_TEXT     (1<<2)  // on image/graph window, on selectable text
#define CURSOR_ON_NOTHING_SPECIAL (1<<3)  // not on image/graph, on border or something
#define CURSOR_ON_COLORBAR        (1<<4)  // on image window, on color bar
#define CURSOR_ON_RIGHT_PANEL     (1<<5)  // on image window, on right panel (colorbar, ...)
// Specific cursor locations: image data
#define CURSOR_ON_MOSAIC          (1<<6)  // on image window, excluding special areas below
#define CURSOR_ON_ORTHO_XY        (1<<7)  // on x-y graph in 3-plane mode
#define CURSOR_ON_ORTHO_ZY        (1<<8)  // on z-y graph in 3-plane mode
#define CURSOR_ON_ORTHO_XZ        (1<<9)  // on x-z graph in 3-plane mode
#define CURSOR_ON_ORTHO_YZ        (1<<10) // on y-z graph in 3-plane mode
#define CURSOR_ON_MOSAIC_BORDER   (1<<11) // on y-z graph in 3-plane mode
// Specific cursor locations: right panel
#define CURSOR_ON_COLORBAR_HIGH   (1<<12) // on actual colorbar
#define CURSOR_ON_COLORBAR_LOW    (1<<13) // on actual colorbar

// Update locations for image window
// Bit-tricks are used so that multiple values can be set at the same time.
#define UPDATE_IMAGES             (1<<0)  // image mosaic or 3-plane display
#define UPDATE_NEIGHBORHOODS      (1<<1)  // image mosaic or 3-plane display
#define UPDATE_IMAGE_CROSSHAIRS   (1<<2)  // image text & cross-hairs, but not image necessarily
#define UPDATE_IMAGE_TEXT_LINES   (1<<3)  // image text & lines, but not image necessarily
#define UPDATE_COLORBAR           (1<<4)  // colorbar scale
#define UPDATE_COLORBAR_LABELS    (1<<5)  // labels on colorbar
#define UPDATE_RIGHT_PANEL        (1<<6)  // labels on colorbar
#define UPDATE_SELECT_TEXT_IMAGE  (1<<7)  // selectable text on image window
#define UPDATE_INFO_AREA_IMAGE    (1<<8)  // info area on image window
#define UPDATE_CURSOR_IMAGE       (1<<9) // info area on image window
// Update locations for graph window
// Bit-tricks are used so that multiple values can be set at the same time.
#define UPDATE_GRAPH              (1<<0) // graph display
#define UPDATE_GRAPH_TEXT_LINES   (1<<1) // text and cross-hairs, but not graph necessarily
#define UPDATE_SELECT_TEXT_GRAPH  (1<<2)  // selectable text on graph window
#define UPDATE_INFO_AREA_GRAPH    (1<<3)  // info area on graph window
#define UPDATE_CURSOR_GRAPH       (1<<4) // info area on image window

enum CommonSelectAbleTextFields_Image
  { // selectable text on image window
    TEXTFIELD_OVL_OR_WIRE     = 1,  // selects overlay or wireframe
    TEXTFIELD_WHICH_OVL_WIRE  = 2,  // selects overlay or wireframe
    TEXTFIELD_DATA_TYPE       = 3,  // mag, phase, real, imaginary
    TEXTFIELD_MOSAIC_3PLANE   = 4,  // mosaic or 3-plane
    TEXTFIELD_XPANELS         = 5,  // sets x/y panels for mosaic mode; sets ZY or YZ for 3-plane
    TEXTFIELD_COLORBAR_TAILP  = 6,  // on + tail above colorbar
    TEXTFIELD_COLORBAR_TAILN  = 7,  // on - tail above colorbar
    TEXTFIELD_NEW_OVL_WIRE    = 8,  // new        wire
    TEXTFIELD_READ_OVL_WIRE   = 9,  // read   ovl/wire
    TEXTFIELD_WRITE_OVL_WIRE  = 10, // write  ovl/wire
    TEXTFIELD_DEL_OVL_WIRE    = 11, // delete ovl/wire
    TEXTFIELD_COLOR_OVL_WIRE  = 12, // color  ovl/wire
    TEXTFIELD_MIRROR_OVL_WIRE = 13, // color  ovl/wire
    TEXTFIELD_FILL_WIRE       = 14, // color  ovl/wire
    TEXTFIELD_OVL_STATS       = 15, // statistics within ovl
    TEXTFIELD_TRANSPARENT     = 16, // transparent ovl
    TEXTFIELD_IMAGE_LEVEL     = 17, // window/level
    TEXTFIELD_IMAGE_ZOOM      = 18, // zoom
    TEXTFIELD_IMAGE_CROSS     = 19, // crosshairs
    TEXTFIELD_IMAGE_INTERP    = 20, // interpolate (zoom != 1)
    TEXTFIELD_IMAGE_GRAPH     = 21, // graph
    TEXTFIELD_HELP            = 22, // context help
    TEXTFIELD_QUIT            = 23, // quit
    TEXTFIELD_QUESTION        = 24, // general help
    TEXTFIELD_POUND_SIGN      = 25, // general help
    TEXTFIELD_MOSAIC_ORIENT   = 26 // general help
  };
enum CommonSelectAbleTextFields_Graph
  {
    // Selectable text fields for graph window
    TEXTFIELD_GRAPH_TYPE      = 1,  // on graph type text field
    TEXTFIELD_GRAPH_BIN       = 2,  // on graph bin text field
    TEXTFIELD_GRAPH_BIN_VALUE = 3,  // on graph bin value text field
    TEXTFIELD_GRAPH_UNITS     = 4,  // on graph units text field
    TEXTFIELD_GRAPH_AUTORANGE = 5,  // on graph range text field
    TEXTFIELD_GRAPH_YLOW      = 6,  // on graph range text field
    TEXTFIELD_GRAPH_YHIGH     = 7,  // on graph range text field
    TEXTFIELD_GRAPH_HIST_CLIP = 8,  // histogram clipping mode
    TEXTFIELD_GRAPH_MODEL     = 9  // on graph model text field
  };

enum display_types
  {
    MOSAIC   = -1,
    ORTHO_XY =  0,
    ORTHO_ZY =  1,
    ORTHO_XZ =  2,
    ORTHO_YZ =  3
  };
enum defined_colors
  {
    COLOR_CYAN   = 1,
    COLOR_RED    = 2,
    COLOR_GREEN  = 3,
    COLOR_BLUE   = 4,
    COLOR_WHITE  = 5,
    COLOR_PURPLE = 6,
    COLOR_YELLOW = 7,
    COLOR_ORANGE = 8,
    COLOR_GRAY   = 9,
    COLOR_BLACK  = 10
  };
enum X11_drawables
  {
    Drawable_ImageWindow   = 0,
    Drawable_GraphWindow   = 1,
    Drawable_ImageInfoArea = 2,
    Drawable_TopMenu       = 3,
    Drawable_BottomMenu    = 4,
    Drawable_ImageData     = 5,
    Drawable_RightPanel    = 6,
    Drawable_GraphData     = 7,
    Drawable_GraphInfoArea = 8
  };

enum REGISTRATION_TYPES
  {
    REG_ALLEN_MOUSE = 1, // Allen brain project
    REG_PAXINOS_RAT = 2, // Paxinos-Watson rat atlas
    REG_MONKEY      = 3  // monkey
  };

// List of input image files
struct file_list
{
  char file_name[LEN_STR];// phMRI data file name for a run
  int nt;                 // # time points from file (may differ from header)
  int start_image;        // starting image number in file
  int end_image;          // ending image number in file
  int scale_log;          // 0 = linear, 1 = log_10, 2 = ln (log_e) using old xds convention
  int type;               // 0=default, 1=P file, 2=T file, 3=S file
  struct XD_HEADER header;// header from file
  struct file_list *next;
};

// List of pre-defined overlays
struct ovl_list
{
  char name[LEN_STR]; // name of the pre-defined overlay
  JIP_INDEX npts;           // the number of overlay pixels
  short color;        // color of overlay; 1 = purple, 2 = yellow, 3 = green, 4 = blue, 5 = red
  BOOL active;        // this is the active overlay (TRUE or FALSE)
  IPOINT3D *voxel;    // dimension = npts; array of (x,y,z) triplets
  float *weight;      // dimension = npts; array of weights
  struct ovl_list *next;
};

// Overlay structure with dimensions same as stack.
struct ovl_map
{
  float *weight;      // dimension nx*ny*nz (all voxels); 0 = no overlay
  short *color;       // dimension nx*ny*nz (all voxels); color as above
  int number;         // keeps track of how many times ROIs have been written with this overlay
};

// List of wire frames
typedef struct
{
  POINT3D point;
  int color;} CONTROL_POINT;
struct wire_list
{
  char name[LEN_STR];     // name of the wire frame
  BOOL active;            // this is the active wire (TRUE or FALSE)
  CONTROL_POINT *control; // dimension npts; 0 = no wire, else = color as above
  JIP_INDEX npts;               // the number of points in the wire frame
  struct wire_list *next;
};

// List of ROI files
struct roi_file_list
{
  char name[LEN_STR];  // name of ROI file
  int number_rois;          // # of rois written into file
  BOOL contains_ovl_list;   // the ROI file has entries for the predefined overlays
  struct roi_file_list *next;
};

struct Neighborhood
{
  int number;          // The number of neighbors contributing to pixel value
  IPOINT2D address[4]; // 2-D addresses into data matrices: (x,y), or (x,z), or (y,z)
  float weight[4];     // The NORMALIZED weight assigned to each neighbor (dimension="number")
};

struct SelectableText
{
  int YLocation;        // 0,1 = TOP, 2,3,4 = BOTTOM
  int XPosition;        // text position in x (pixels)
  int XSelectStart;     // starting selection location in x (pixels)
  int XSelectEnd;       // ending selection location in x (pixels)
  int color;            // color of text field when NOT selected
  char string[LEN_STR]; // text field
  char message1[LEN_STR];// information message sent to info line
  char message2[LEN_STR];// information message sent to info line
  char message3[LEN_STR];// information message sent to info line
  int FieldID;          // identification tag
  BOOL visible;         // 0 or 1
  BOOL selected;        // 0 or 1
  struct SelectableText *next;
};

struct Activation
{
  int nmaps;                // number of activation maps
  int imap;                 // index of current displayed activation map
  char map_name[LEN_STR];
  int scale_log;            // value taken from the current file_list
  float thresh_lin_low;     // low threshold for linear activation overlays (always >= 0. )
  float thresh_lin_high;    // high threshold for linear activation overlays (always > low threshold)
  float thresh_log_low;     // low threshold for logarithmic activation overlays (always >= 0. )
  float thresh_log_high;    // high threshold for logarithmic activation overlays (always > low threshold)
  int color_scheme;         // 0 = +/- tails, -1=-tail, 1=+tail,-2=-tail (spectrum), 2=+tail (spectrum)
  int color_sign;           // 1 or -1; only for 2 tail display (color_scheme=0)
  int visible;              // show activation overlays on images
  BOOL transparent;         // make tranparent activation color overlays (color/gray fusion)
  float *fSpaceMatrix;      // A pointer to the activation overlay.
  float *pFilter;           // For linear maps, a pointer to a p value map using the log thresholds
  struct data_stack *stack; // A pointer to the activation stack
};

// The root overlay is writable. No other overlays can be modified.
JIP_LEGACY_STORAGE struct ovl_list ovl_root;
// The root overlay is downloaded into the overlay map.
JIP_LEGACY_STORAGE struct ovl_map overlay_map;
// The root wire is writable. No other wires can be modified.
JIP_LEGACY_STORAGE struct wire_list wire_root;

// The generic name "image_stack" is used throughout the programs.
// This could refer either to
// 1) a time series (display program), or
// 2) the output of a registration procedure (alignment program)
JIP_LEGACY_STORAGE struct data_stack image_stack;       // time series images
JIP_LEGACY_STORAGE struct data_stack activation_stack;  // activation images

// The characteristics of the current image window
JIP_LEGACY_STORAGE struct mosaic_display
{
  // The "DisplayVolume" is a (potentially) rotated version of the original volume
  // All 3-D fields refer to (x,y,z) display directions in mosaic mode, but to real directions in 3-plane mode
  IPOINT3D DisplayVolumeDim; // dimensions of display volume
  POINT3D  DisplayVolumeRes; // resolution of display volume
  IPOINT3D DimPanels;        // number of mosaic panels in x, y, z (z is # slices in mosaic)
  IPOINT2D ImageRegionPixels;// dimensions of mosaic, including zoom & borders

  float    PixelScale;       // scaling between data and pixels (>1 when screen is too small)
  IPOINT3D PixelDim;         // pixel dimensions of real (x,y,z) directions, excluding zoom factor
  POINT3D  PixelResolution;  // relative pixel res (>=1 always) for DisplayVolume (mosaic) or real volume (3d)
  IPOINT3D icross;           // x,y,z position in integer voxel coordinates
  IPOINT3D icursor;          // x,y,z position in integer voxel coordinates
  POINT3D  cursor;           // x,y,z position in real space coordinates
  int cursor_selection;      // bits indicate the position of cursor; more than 1 bit can be set
  int text_selection;        // current selectable text field
  int update_regions;        // bits indicate regions on the image window to update
  int zoom;                  // zoom factor for image window
  int flip_X11_y;            // swap the y dimension of all 2D images to reverse X11 y convention
  int display_3orthogonal;   // display as 3 orthogonal, not a collection of parallel slices in a mosaic
  int display_ortho_ZY;      // display ZY panel rather YZ panel
  int display_crosshairs;    // flag to display cross-hairs (0=none,1=pixelated,2=lines,3=crosses)
  int display_grid;          // flag to display labels/ticks; 0=none,1=labels only,2=ticks only, 3=both
  int display_data_type;     // 0 = magnitude, 1 = phase, 2 = real, 3 = imaginary
  int mosaic_orientation;    // ORTHO_XY, ORTHO_ZY, ORTHO_XY, or ORTHO_YZ
  int *display_slice;        // array for displaying each slice (0 or 1); dimensions = DimVolume.x or y or z
  BOOL Attach_ColorBar;      // if yes, include a colorbar (color or grayscale)
  int nx_ColorBar;           // x dimension of colorbar
  int nx_RightPanel;         // x dimension of right panel (parameters)
  int toggle_overlays;       // 0 = not available, 1 = overlays, 2 = wires
  int transparent_overlay;   // 0 or 1
  int interpolate;           // interpolate across zoom factor
  int clipping_mode;         // 0 = saturate (white), 1 = clip (black)
  struct Activation activation;     // activation overlay
  float *fSpaceMatrix;       // A pointer to the current time point in the stack space-time data (float)
  struct data_stack *stack;  // A pointer to the current stack
  struct Neighborhood *NeighborsXY; // ORTHO_XY (x-y)
  struct Neighborhood *NeighborsZY; // ORTHO_ZY (z-y)
  struct Neighborhood *NeighborsXZ; // ORTHO_XZ (x-z)
  struct Neighborhood *NeighborsYZ; // ORTHO_YZ (y-z)
  BOOL OnGridXY, OnGridZY, OnGridXZ, OnGridYZ; // // true if all pixels have just 1 neighbor
  struct SelectableText SelectText; // Selectable text on the image window.
} mosaic;

// The characteristics of the current graph window
JIP_LEGACY_STORAGE struct graph_struct
{
  int type;           // type of graph: 0 = x, 1 = y, 2 = z, 3 = time, 4 = histogram
  int time_model;     // GLM, etc.
  int freq_domain;    // frequency domain (only for GLM)
  int cursor_selection; // bits indicate the position of cursor; more than 1 bit can be set
  int text_selection;   // bits indicate the selectable text field
  int update_regions;   // bits indicate regions on the graph window to update
  int nwidth;         // width of the graph in pixel units
  int nheight;        // height of the graph in pixel units
  int npoints;        // # of points in graph (x, y, or t)
  float xmin;         // graph x minimum for display purposes
  float xmax;         // graph x maximum for display purposes
  float ymin;         // graph y minimum for display purposes
  float ymax;         // graph y maximum for display purposes
  int ix_low_select;  // low x value selected in graph window by mouse
  int ix_high_select; // high x value selected in graph window by mouse
  float *yVector;     // data in the graph window (length npoints)
  float *yVector1;    // secondary data in the graph window (for x or y projection)
  float *yFit;        // fit values to fVector; non-zero values always shown on graph
  float *xVector;     // fit values to fVector; non-zero values always shown on graph
  IPOINT2D icursor;   // x,y,z position in integer voxel coordinates
  POINT2D  cursor;    // x,y,z position in real space coordinates
  IPOINT2D imarker;   // x,y,z position in integer voxel coordinates
  POINT2D marker;     // x,y,z position in integer voxel coordinates
  int iTime;          // time index for current graph point
  int iLastTime;      // previous graph time point (for toggling back and forth)
  int units_percent;  // units of y scale: 0 = absolute, 1 = percent
  int yrange_fixed;   // fix y range (0 => variable y range)
  BOOL bin_data;      // if yes, bin the data; otherwise, smooth the data
  int bin_width;      // the width of the binning or smoothing range
  int histogram_bins; // number of bins in the histogram
  struct SelectableText SelectText; // Selectable text on the image window.
} graph;

// miscellaneous global variables
JIP_LEGACY_STORAGE char program_name[LEN_STR];
JIP_LEGACY_STORAGE float contrast_low, contrast_high;
JIP_LEGACY_STORAGE float image_scale_min, image_scale_max, image_scale_slope, image_scale_offset;
JIP_LEGACY_STORAGE float act_scale_slope, act_scale_offset;
JIP_LEGACY_STORAGE int drawing_overlay;

JIP_LEGACY_STORAGE int lXBottomMenu, lPixelsX_BottomMenu;



#include "common-prototypes.h"
