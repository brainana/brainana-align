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



//
// Author: Joe Mandeville
//         Athinoula A. Martinos Center for Biomedical Imaging
//         Massachusetts General Hospital

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "util.h"
#include "common-display.h"
#include "display-X11.h"


void loop_X_events()
{
  XEvent event;
  BOOL Running = TRUE;
  int X_event_type;

  while ( Running )
    {
      X_event_type = get_next_event(&event);
      reset_cursor_selections();
      if ( X_event_type == 1 )
	{
	  mosaic.cursor_selection = CURSOR_ON_WINDOW;
	  image_event_handler_general(&event);
	}
      else if ( X_event_type == 2 )
	{
	  graph.cursor_selection = CURSOR_ON_WINDOW;
	  graph_event_handler_general(&event);
	}
      XSync(XDisplay,False);
    }
}

int get_next_event(XEvent *event)
     // returned: 0 = not found, 1 = image window
{
  if ( XEventsQueued(XDisplay, QueuedAfterFlush) )
    {
      XNextEvent(XDisplay,event);
      if (event->xany.window == XWindow_image)
	return(1);
      else if (event->xany.window == XWindow_graph)
	return(2);
      else
	{
	  return(0);
	}
    }
  return(0);
}

int get_key_press(XKeyEvent *key_event, char *which_key)
{
  // return: 1 = successful key press, 0 = not
  char keybuf[32];

  *which_key=0;
  switch(XLookupKeysym(key_event, 0))
    {
    case XK_Up:    *which_key = 'U'-'@';  break;
    case XK_Down:  *which_key = 'D'-'@';  break;
    case XK_Right: *which_key = 'K'-'@';  break;  // right arrow
    case XK_Left:  *which_key = 'L'-'@';  break;
    case XK_Help:  *which_key = 'h';  break;
    case XK_Prior: *which_key = '+';  break;
    case XK_Next:  *which_key = '-';  break;
    case XK_Home:  *which_key = '\r'; break;
    case XK_End:   *which_key = '\r'; break;
    default: 
      *which_key = 0;
      break;
    }
  if ( !*which_key )
    {
      if ( XLookupString(key_event, keybuf, 32, 0, 0) > 0 )
	*which_key = keybuf[0];
    }
  if ( *which_key > 0 )
    return(1);
  else
    return(0);
}

int image_event_handler_general(XEvent *event)
{
  char which_key;
  BOOL current_window;
  static BOOL last_expose=FALSE, last_reparent=FALSE;

  mosaic.update_regions = graph.update_regions = 0;

  current_window = (event->xbutton.window == XWindow_image);

  if ( current_window && ( event->type == KeyPress      || event->type == ButtonPress ||
			   event->type == ButtonRelease || event->type == MotionNotify ||
			   event->type == LeaveNotify ) )
    define_mosaic_cursor_location(event->xbutton.x, event->xbutton.y);

  if ( !current_window && event->type != KeyPress ) return(0);
  switch (event->type)
    {
    case KeyPress:
      {
	if ( !get_key_press(&(event->xkey), &which_key) ) return(1);
	switch (which_key)
	  {
	    ///////////////////////////////////////////////
	    //////////////// non-alphabetic ///////////////
	    ///////////////////////////////////////////////
	  case 'M'-'@':  // ^m, or carriage return
	    action_key_return();
	    break;
	  case 'U'-'@':
	    action_image_arrow_up_down( 1 );
	    break;
	  case 'D'-'@':
	    action_image_arrow_up_down( 0 );
	    break;
	  case 'L'-'@':
	    action_image_arrow_left_right( 1 );
	    break;
	  case 'K'-'@':  // right arrow
	    action_image_arrow_left_right( 0 );
	    break;
	  case ',': // capital is '<'
	  case '<': // "in arrow"
	    action_image_arrow_in_out( 1 );
	    break;
	  case '.': // capital is '>'
	  case '>': // "out arrow"
	    action_image_arrow_in_out( 0 );
	    break;
	  case '3':
	    action_key_3();
	    break;
	    ///////////////////////////////////////////////
	    //////////////// Begin alphabetic /////////////
	    ///////////////////////////////////////////////
	  case 'c':      // color overlay
	    action_key_c();
	    break;
	  case 'C':      // color wire frame
	    action_key_C();
	    break;
	  case 'C'-'@': // control-c
	    action_key_control_c(1);
	    break;
	  case 'd':      // delete overlay
	    action_key_d();
	    break;
	  case 'D':      // delete wire frame
	    action_key_D();
	    break;
	  case 'f':      // fill wire frames
	  case 'F':
	    fill_closed_wires();
	    break;
	  case 'h':
	  case '?':
	    action_help(1,1);
	    break;
	  case 'H':
	    action_help(1,2);
	    break;
	  case 'i':     // image window/level
	  case 'I':
	    action_key_i();
	    break;
	  case 'I'-'@': // control-i (same as tab)
	    action_key_control_i();
	    break;
	  case 'm':     // mirror overlay left-write
	    action_key_m();
	    break;
	  case 'M':     // mirror wire frame left-write
	    action_key_M();
	    break;
	  case 'n':     // new wire frame
	    if ( mosaic.activation.visible )
	      action_key_n();
	    else
	      action_key_N();
	    break;
	  case 'N':     // new wire frame
	    action_key_N();
	    break;
	  case 'r':     // read overlay
	    action_key_r();
	    break;
	  case 'R':     // read wire frame
	    action_key_R();
	    break;
	  case 't':
	  case 'T':
	    action_key_T();
	    break;
	  case 'w':     // write overlay
	    action_key_w();
	    break;
	  case 'W':     // write wire frame
	    action_key_W();
	    break;
	  case 'q':
	  case 'Q':
	    action_quit();
	    break;
	  case 'v': // make range variable
	    graph.yrange_fixed = 0;
	    build_graph_display(1);
	    break;
	  case 'z':
	    action_key_z();    // decrease zoom factor
	    break;
	  case 'Z':
	    action_key_Z();    // increase zoom factor
	    break;
	  case '#': // display grid lines
	    action_select_slices();
	    break;
	  default:
	    // Pass the event to check program-specific functions
	    image_event_handler_specific(event);
	    break;
	  }
	break; /* END OF case KeyPress: */
      }

    case ButtonPress:
      switch (event->xbutton.button)
	{	
	case 1: 
	  action_image_button1(1, event->xbutton.x, event->xbutton.y);
	  break; 
	case 2:
	case 3:
	  action_image_button23(1);
	  break; 
	case 4: // scroll wheel up
	  action_image_button4();
	  break;
	case 5: // scroll wheel down
	  action_image_button5();
	  break;
	}
      break; /* END OF case ButtonPress: */
    case ButtonRelease:
      switch(event->xbutton.button)
	{
	case 1:
	  action_image_button1(0, event->xbutton.x, event->xbutton.y);
	  break;
	case 2:
	case 3:
	  action_image_button23(0);
	  break;
	}
      break; /* END OF case ButtonRelease: */
    case MotionNotify: 
      action_image_motion_notify();
      break;
    case LeaveNotify:
      mosaic.cursor_selection = 0;
      mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
      //      if ( mosaic.activation.visible ) show_information_colorbar();
      break;
    case Expose:
      if ( !last_expose )
	{
	  refresh_display_image(0);
	  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
	}
      break;
    case ConfigureNotify:
      break;
    case MapNotify:
      break;
    case ReparentNotify:
      last_reparent = TRUE;
      break;
    case ClientMessage:
      break;
    case DestroyNotify:
      break;
    default:
      break;
    } /* END OF switch(event->type) */
  if ( event->type == Expose )
    last_expose = TRUE;
  else
    last_expose = FALSE;
  if ( event->type == ReparentNotify )
    last_reparent = TRUE;
  else
    last_reparent = FALSE;

  Update_Regions();
  return(0);
}

int graph_event_handler_general(XEvent *event)
{
  char which_key;
  float dx, yscale, yvalue;
  int xbut, ybut, ixvalue;
  BOOL current_window;

  mosaic.update_regions = graph.update_regions = 0;

  current_window = event->xbutton.window == XWindow_graph;

  xbut = event->xbutton.x;
  ybut = event->xbutton.y;

  if ( current_window && ( event->type == KeyPress      || event->type == ButtonPress ||
			   event->type == ButtonRelease || event->type == MotionNotify ||
			   event->type == LeaveNotify ) )
    define_graph_cursor_location(event->xbutton.x, event->xbutton.y);

  ybut -= INFO_HEIGHT;
  if ( ybut < 0 ) ybut = 0;
  if ( ybut >= graph.nheight ) ybut = graph.nheight-1;

  dx = (float)(graph.nwidth) / (float)(graph.npoints);
  ixvalue = (float)xbut/dx;
  if ( ixvalue >= graph.npoints ) ixvalue = graph.npoints - 1;

  // Report y in data units.
  yscale = (float)(graph.nheight) / (graph.ymax-graph.ymin);
  yvalue = graph.ymax - (float)ybut / yscale;


  if ( !current_window && event->type != KeyPress ) return(0);
  switch (event->type)
    {
    case KeyPress:
      {
	if ( !get_key_press(&(event->xkey), &which_key) ) return(1);
	switch (which_key)
	  {
	  case 'U'-'@':  // up arrow
	    action_graph_arrow_up_down(1, yvalue);
	    break;
	  case 'D'-'@':  // down arrow
	    action_graph_arrow_up_down(0, yvalue);
	    break;
	  case 'L'-'@':  // left arrow
	    action_graph_arrow_left_right(1);
	    break;
	  case 'K'-'@':  // right arrow
	    action_graph_arrow_left_right(0);
	    break;
	  default:
	    if ( current_window )
	      graph_event_handler_specific(event); // 0 indicates event NOT from graph window
	    break;
	  }
	break; /* END OF case KeyPress on graph: */
      }

    case ButtonPress:  // on graph
      switch (event->xbutton.button)
	{
	case 1: 
	  action_graph_button1( ixvalue );
	  break; 
	case 2:
	case 3:
	  action_graph_button23( 1, ixvalue );
	  break; 
	case 4: // scroll wheel up
	  action_graph_arrow_up_down(1, yvalue);
	  break;
	case 5: // scroll wheel down
	  action_graph_arrow_up_down(0, yvalue);
	  break;
	default:
	  fprintf(stderr, "more than 5 buttons?\n");
	  break;
	}
      break; /* END OF case ButtonPress on graph */
    case ButtonRelease: // on graph
      switch(event->xbutton.button)
	{
	case 1:
	  break;
	case 2:
	case 3:
	  action_graph_button23( 0, ixvalue );
	  break;
	default:
	  //	  fprintf(stderr, "more than 3 buttons?\n");
	  break;
	}
      break; /* END OF case ButtonRelease on graph */
    case MotionNotify:
      action_graph_motion_notify();
      break;
    case LeaveNotify:
      graph.cursor_selection = 0;
      //      draw_selectable_text_image();
      show_information_graph(0);
      break;
    case Expose:
      build_graph_display(0);
      break;
    case ConfigureNotify:
      if ( graph.nwidth != event->xconfigure.width ||
	   graph.nheight!= event->xconfigure.height - 4*INFO_HEIGHT )
	{
	  graph.nwidth  = event->xconfigure.width;
	  graph.nheight = event->xconfigure.height - 4*INFO_HEIGHT;
	  update_graph_selectable_text();
	  build_graph_display(1);
	}
      break;
    case MapNotify:
      break;
    case ReparentNotify:
      break;
    case ClientMessage:
      if (event->xclient.data.l[0] == wm_delete_window)
	{
	  XUnmapWindow(XDisplay,XWindow_graph);
	  graph.type = -1;
	  initialize_graph();
	  graph.nwidth = graph.nheight = 0;
	  XWindow_graph = 0;
	}
      break;
    default:
      break;
    } /* END OF switch(event->type) */

  Update_Regions();
  return(0);
}

void initialize_XWindows()
{
  XVisualInfo vinfo;

  if ( !(XDisplay=(Display *)XOpenDisplay(NULL)) )
    {
      printf("Error trying to open the display.\n");
      printf("If you are using ssh to connect from another system, try 'ssh -X'\n");
      exit(1);
    }

  XScreen = XDefaultScreen(XDisplay);
  XDisplay_Window = XRootWindow(XDisplay,XScreen);
  XDisplay_depth = XDefaultDepth(XDisplay, XScreen);

  XColorMap = XDefaultColormap(XDisplay, XScreen);
  // Install the default colormap.
  XInstallColormap(XDisplay, XColorMap);

  if (XMatchVisualInfo(XDisplay, XScreen, XDisplay_depth, TrueColor, &vinfo))
    XVisual = vinfo.visual;
  else
    XVisual = (Visual *)XDefaultVisual(XDisplay, XScreen);

  wm_delete_window = XInternAtom(XDisplay, "WM_DELETE_WINDOW", False);
}

void make_XWindow_image()
{
  float screen_factor = 0.85;  // available screen for display; < 1 due to OS menu bars
  static BOOL first = TRUE, all_free=TRUE;
  IPOINT2D RoughDisplaySize;
  float PixelScale;
  int lStatus, size;
  int lPixelsX, lPixelsX_ImageInfoArea, nx_RightPanel;
  unsigned long cyan, cyan_soft, cyan_dark, cyan_darker, red, green, green_dark, blue;
  unsigned long yellow, purple, white, black, orange, gray;
  unsigned int line_width;

  if ( first )
    {
      if ( mosaic.zoom == 0 )
	{
	  size = MAX(mosaic.PixelDim.x*mosaic.DimPanels.x,
		     mosaic.PixelDim.y*mosaic.DimPanels.y);
	  mosaic.zoom = XDisplayWidth(XDisplay,XScreen) / size * 0.75;
	  mosaic.zoom = MIN( 4,MAX(1,mosaic.zoom) );
	}
      first = FALSE;
    }
  else if ( !all_free )
    {
      XDestroyWindow(XDisplay,XWindow_image);
      XFreePixmap(XDisplay,XPixmap_image);
      XDestroyImage(XImage_background);    // frees image AND data
      XDestroyImage(XImage_TopMenu);       // frees image AND data
      XDestroyImage(XImage_BottomMenu);    // frees image AND data
      XDestroyImage(XImage_ImageInfoArea); // frees image AND data
      XDestroyImage(XImage_RightPanel);    // frees image AND data
      XDestroyImage(XImage_colorbar);      // frees image AND data
      XDestroyImage(XImage_XY);            // frees image AND data
      XDestroyImage(XImage_ZY);            // frees image AND data
      XDestroyImage(XImage_XZ);            // frees image AND data
      XDestroyImage(XImage_YZ);            // frees image AND data
      XDestroyImage(XImage_lineX_cyan);    // frees image AND data
      XDestroyImage(XImage_lineY_cyan);    // frees image AND data
      XDestroyImage(XImage_lineX_red);     // frees image AND data
      XDestroyImage(XImage_lineY_red);     // frees image AND data
      XDestroyImage(XImage_lineZ1_cyan);   // frees image AND data
      XDestroyImage(XImage_lineZ2_cyan);   // frees image AND data
      XDestroyImage(XImage_lineZ1_red);    // frees image AND data
      XDestroyImage(XImage_lineZ2_red);    // frees image AND data
      XDestroyImage(XImage_lineY1_cyan);   // frees image AND data
      XDestroyImage(XImage_lineY1_red);    // frees image AND data
      // Graphics contexts
      XFreeGC(XDisplay, XGC_cyan_text);
      XFreeGC(XDisplay, XGC_red_text);
      XFreeGC(XDisplay, XGC_green_text);
      XFreeGC(XDisplay, XGC_blue_text);
      XFreeGC(XDisplay, XGC_white_text);
      XFreeGC(XDisplay, XGC_purple_text);
      XFreeGC(XDisplay, XGC_yellow_text);
      XFreeGC(XDisplay, XGC_orange_text);
      XFreeGC(XDisplay, XGC_gray_text);

      XFreeGC(XDisplay, XGC_cyan);
      XFreeGC(XDisplay, XGC_red);
      XFreeGC(XDisplay, XGC_green);
      XFreeGC(XDisplay, XGC_blue);
      XFreeGC(XDisplay, XGC_white);
      XFreeGC(XDisplay, XGC_purple);
      XFreeGC(XDisplay, XGC_yellow);
      XFreeGC(XDisplay, XGC_orange);
      XFreeGC(XDisplay, XGC_black);

      all_free = TRUE;
    }
  // Define colors
  black  = 0;
  red    = WHITE_PIXEL << 16;
  green  = WHITE_PIXEL << 8;
  green_dark = (5*WHITE_PIXEL/8) << 8;
  blue   = WHITE_PIXEL;
  cyan   = green + blue;
  cyan_soft  = ((3*WHITE_PIXEL/4)<<8) + (3*WHITE_PIXEL/4); // cyan, less bright
  cyan_dark  = ((3*WHITE_PIXEL/8)<<8) + (3*WHITE_PIXEL/8); // cyan, even less bright
  cyan_darker= ((1*WHITE_PIXEL/4)<<8) + (1*WHITE_PIXEL/4); // cyan, even less bright
  yellow = red + green;
  purple = red + blue;
  white  = red + green + blue;
  orange = red + ((WHITE_PIXEL/2)<<8);
  gray   = ((WHITE_PIXEL/2)<<16) + ((WHITE_PIXEL/2)<<8) + (WHITE_PIXEL/2);

  compute_display_size(&RoughDisplaySize, 1.);
  // The available screen width is a rough guess, due to screen loss from menu bars, etc.
  PixelScale = (float)RoughDisplaySize.x / (screen_factor*(float)XDisplayWidth(XDisplay,XScreen));
  PixelScale = MAX(PixelScale,(float)RoughDisplaySize.y
		   / (screen_factor*(float)XDisplayHeight(XDisplay,XScreen)));
  while ( PixelScale > 1. && mosaic.zoom > 1 ) // too big
    {
      mosaic.zoom--;
      compute_display_size(&RoughDisplaySize, 1.);
      // If the image is too big, cut down the zoom factor.
      PixelScale = (float)RoughDisplaySize.x
	/ (screen_factor*(float)XDisplayWidth(XDisplay,XScreen));
      PixelScale = MAX(PixelScale,(float)RoughDisplaySize.y
		       / (screen_factor*(float)XDisplayHeight(XDisplay,XScreen)));
    }
  PixelScale = MAX(1.,PixelScale);
  compute_display_size(&RoughDisplaySize, PixelScale);
  mosaic.PixelScale = PixelScale;

  nx_RightPanel = MAX(1,mosaic.nx_RightPanel); // prevent 0-length pixmaps, etc.
  lPixelsX = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar + nx_RightPanel;
  lPixelsX_ImageInfoArea = MIN(300,mosaic.ImageRegionPixels.x-1);  // fixed value
  lPixelsX_BottomMenu    = MAX(1,lPixelsX - lPixelsX_ImageInfoArea);
  lXBottomMenu           = lPixelsX_ImageInfoArea;

  // Made it past resize section, so allocation/creation will begin.
  all_free = FALSE;

  // Create the window.
  XWindow_image = XCreateSimpleWindow(XDisplay, XDisplay_Window, 1, 1,
				      mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar + mosaic.nx_RightPanel,
				      mosaic.ImageRegionPixels.y + 5*INFO_HEIGHT,
				      100, WHITE_PIXEL, BLACK_PIXEL);
  // Define the events.
  XSelectInput(XDisplay, XWindow_image, ExposureMask|ButtonPressMask|ButtonReleaseMask
	       |PointerMotionMask|KeyPressMask|StructureNotifyMask|LeaveWindowMask);

  // Give the window a name.
  change_window_name();

  // Graphics context for image
  XGC_image = (GC)XDefaultGC(XDisplay, XScreen);
  // Graphics context for strings in the info area
  XGC_cyan_text   = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_red_text    = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_green_text  = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_blue_text   = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_white_text  = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_purple_text = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_yellow_text = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_orange_text = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_gray_text   = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XSetGraphicsExposures(XDisplay, XGC_cyan_text,  True);
  XSetGraphicsExposures(XDisplay, XGC_red_text,   True);
  XSetGraphicsExposures(XDisplay, XGC_green_text, True);
  XSetGraphicsExposures(XDisplay, XGC_blue_text,  True);
  XSetGraphicsExposures(XDisplay, XGC_white_text, True);
  XSetGraphicsExposures(XDisplay, XGC_purple_text,True);
  XSetGraphicsExposures(XDisplay, XGC_yellow_text,True);
  XSetGraphicsExposures(XDisplay, XGC_orange_text,True);
  XSetGraphicsExposures(XDisplay, XGC_gray_text,  True);
  XSetState(XDisplay, XGC_cyan_text,  black, cyan,  GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_red_text,   black, red,   GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_green_text, black, green, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_blue_text,  black, blue,  GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_white_text, black, white, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_purple_text,black, purple,GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_yellow_text,black, yellow,GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_orange_text,black, orange,GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_gray_text,  black, gray,  GXcopy, AllPlanes);

  // Graphics context for drawing without pixel filling.
  XGC_cyan   = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_red    = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_green  = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_blue   = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_yellow = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_orange = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_purple = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_white  = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XGC_black  = (GC)XCreateGC(XDisplay, XWindow_image, 0, NULL);
  XSetGraphicsExposures(XDisplay, XGC_cyan,   False);
  XSetGraphicsExposures(XDisplay, XGC_red,    False);
  XSetGraphicsExposures(XDisplay, XGC_green,  False);
  XSetGraphicsExposures(XDisplay, XGC_blue,   False);
  XSetGraphicsExposures(XDisplay, XGC_yellow, False);
  XSetGraphicsExposures(XDisplay, XGC_orange, False);
  XSetGraphicsExposures(XDisplay, XGC_purple, False);
  XSetGraphicsExposures(XDisplay, XGC_white,  False);
  XSetGraphicsExposures(XDisplay, XGC_black,  False);
  // Set colors here
  //  XSetState(XDisplay, XGC_cyan,   cyan_soft, 0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_cyan,   cyan,      0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_red,    red,       0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_green,  green,     0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_blue,   blue,      0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_white,  white,     0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_purple, purple,    0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_yellow, yellow,    0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_orange, orange,    0, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_black,  black,     0, GXcopy, AllPlanes);
  // Set line widths here
  line_width = MIN(mosaic.zoom,2);
  //  line_width = MIN(mosaic.zoom,1);
  XSetLineAttributes(XDisplay, XGC_cyan,   line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_red,    line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_green,  line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_blue,   line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_white,  line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_purple, line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_yellow, line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_orange, line_width, LineSolid, CapRound, JoinRound);
  XSetLineAttributes(XDisplay, XGC_black,  line_width, LineSolid, CapRound, JoinRound);

  XData_XY = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,mosaic.PixelDim.x),mosaic.zoom),mosaic.PixelDim.y),XDisplay_depth));
  XData_ZY = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,mosaic.PixelDim.y),mosaic.zoom),mosaic.PixelDim.z),XDisplay_depth));
  XData_XZ = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,mosaic.PixelDim.x),mosaic.zoom),mosaic.PixelDim.z),XDisplay_depth));
  XData_YZ = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,mosaic.PixelDim.z),mosaic.zoom),mosaic.PixelDim.y),XDisplay_depth));
  XData_lineX_cyan  = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.x+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_lineX_red   = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.x+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_lineY_cyan  = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.y+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_lineY_red   = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.y+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_lineY1_cyan = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.y+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_lineY1_red  = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.y+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  if ( mosaic.display_ortho_ZY )
    { // z-y
      XData_lineZ1_cyan = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.z+2)),mosaic.zoom),NBORDER),XDisplay_depth));
      XData_lineZ1_red  = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.z+2)),mosaic.zoom),NBORDER),XDisplay_depth));
    }
  else
    { // y-z
      XData_lineZ1_cyan = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.y+2)),mosaic.zoom),NBORDER),XDisplay_depth));
      XData_lineZ1_red  = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.y+2)),mosaic.zoom),NBORDER),XDisplay_depth));
    }
  XData_lineZ2_cyan = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.z+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_lineZ2_red  = (char *)malloc((size_t)jip_product(jip_product(jip_product(jip_product(mosaic.zoom,(mosaic.PixelDim.z+2)),mosaic.zoom),NBORDER),XDisplay_depth));
  XData_background = (char *)malloc((size_t)jip_product(jip_product(mosaic.ImageRegionPixels.x,mosaic.ImageRegionPixels.y),XDisplay_depth));
  XData_TopMenu    = (char *)malloc((size_t)jip_product(jip_product(jip_product(lPixelsX,2),INFO_HEIGHT),XDisplay_depth));
  XData_BottomMenu = (char *)malloc((size_t)jip_product(jip_product(jip_product(lPixelsX_BottomMenu,3),INFO_HEIGHT),XDisplay_depth));
  XData_ImageInfoArea = (char *)malloc((size_t)jip_product(jip_product(jip_product(lPixelsX_ImageInfoArea,3),INFO_HEIGHT),XDisplay_depth));
  XData_RightPanel = (char *)malloc((size_t)jip_product(jip_product(nx_RightPanel,mosaic.ImageRegionPixels.y),XDisplay_depth));
  XData_colorbar   = (char *)malloc((size_t)jip_product(jip_product(mosaic.nx_ColorBar,mosaic.ImageRegionPixels.y),XDisplay_depth));

  XImage_XY = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_XY,jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.x),jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.y), 8, 0);
  XImage_ZY = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_ZY,jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.z),jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.y), 8, 0);
  XImage_XZ = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_XZ,jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.x),jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.z), 8, 0);
  XImage_YZ = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_YZ,jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.y),jip_int_count(((JIP_INDEX)(mosaic.zoom))*mosaic.PixelDim.z), 8, 0);
  XImage_background = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_background,jip_int_count(mosaic.ImageRegionPixels.x),jip_int_count(mosaic.ImageRegionPixels.y), 8, 0);
  XImage_TopMenu    = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_TopMenu,jip_int_count(lPixelsX),jip_int_count(2*INFO_HEIGHT), 8, 0);
  XImage_BottomMenu = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_BottomMenu,jip_int_count(lPixelsX_BottomMenu),jip_int_count(3*INFO_HEIGHT), 8, 0);
  XImage_ImageInfoArea = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_ImageInfoArea,jip_int_count(lPixelsX_ImageInfoArea),jip_int_count(3*INFO_HEIGHT), 8, 0);
  XImage_RightPanel = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_RightPanel,jip_int_count(nx_RightPanel),jip_int_count(mosaic.ImageRegionPixels.y), 8, 0);
  XImage_colorbar   = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_colorbar,jip_int_count(mosaic.nx_ColorBar),jip_int_count(mosaic.ImageRegionPixels.y), 8, 0);
  XImage_lineX_cyan = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineX_cyan,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.x+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
  XImage_lineX_red  = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineX_red,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.x+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
  XImage_lineY_cyan = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineY_cyan,jip_int_count(mosaic.zoom*NBORDER),jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.y+2)), 8, 0);
  XImage_lineY_red  = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineY_red,jip_int_count(mosaic.zoom*NBORDER),jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.y+2)), 8, 0);
  XImage_lineY1_cyan= XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineY1_cyan,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.y+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
  XImage_lineY1_red = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineY1_red,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.y+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
  if ( mosaic.display_ortho_ZY )
    { // z-y
      XImage_lineZ1_cyan = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineZ1_cyan,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.z+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
      XImage_lineZ1_red  = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineZ1_red,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.z+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
    }
  else
    { // y-z
      XImage_lineZ1_cyan = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineZ1_cyan,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.y+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
      XImage_lineZ1_red  = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineZ1_red,jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.y+2)),jip_int_count(mosaic.zoom*NBORDER), 8, 0);
    }
  XImage_lineZ2_cyan = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineZ2_cyan,jip_int_count(mosaic.zoom*NBORDER),jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.z+2)), 8, 0);
  XImage_lineZ2_red  = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_lineZ2_red,jip_int_count(mosaic.zoom*NBORDER),jip_int_count(((JIP_INDEX)(mosaic.zoom))*(mosaic.PixelDim.z+2)), 8, 0);
  
  lStatus = XInitImage(XImage_XY);
  lStatus = XInitImage(XImage_ZY);
  lStatus = XInitImage(XImage_XZ);
  lStatus = XInitImage(XImage_YZ);
  lStatus = XInitImage(XImage_background);
  lStatus = XInitImage(XImage_TopMenu);
  lStatus = XInitImage(XImage_BottomMenu);
  lStatus = XInitImage(XImage_ImageInfoArea);
  lStatus = XInitImage(XImage_RightPanel);
  lStatus = XInitImage(XImage_colorbar);
  lStatus = XInitImage(XImage_lineX_cyan);
  lStatus = XInitImage(XImage_lineX_red);
  lStatus = XInitImage(XImage_lineY_cyan);
  lStatus = XInitImage(XImage_lineY_red);
  lStatus = XInitImage(XImage_lineZ1_cyan);
  lStatus = XInitImage(XImage_lineZ1_red);
  lStatus = XInitImage(XImage_lineZ2_cyan);
  lStatus = XInitImage(XImage_lineZ2_red);
  lStatus = XInitImage(XImage_lineY1_cyan);
  lStatus = XInitImage(XImage_lineY1_red);

  // Cyan background for image data.
  if ( PixelScale != 1. )
    initialize_XImage(XImage_background,green_dark);
  else
    initialize_XImage(XImage_background,cyan_soft);
  // TopMenu background
  initialize_XImage(XImage_TopMenu,cyan_dark);
  // BottomMenu background
  initialize_XImage(XImage_BottomMenu,cyan_dark);
  // ImageInfoArea background
  initialize_XImage(XImage_ImageInfoArea,white);
  // RightPanel background
  initialize_XImage(XImage_RightPanel,cyan_darker);
  // Line data: used for highlighting selected panels from mosaic.
  initialize_XImage(XImage_lineX_cyan,cyan_soft);
  initialize_XImage(XImage_lineX_red,red);
  initialize_XImage(XImage_lineY_cyan,cyan_soft);
  initialize_XImage(XImage_lineY_red,red);
  initialize_XImage(XImage_lineY1_cyan,cyan_soft);
  initialize_XImage(XImage_lineY1_red,red);
  initialize_XImage(XImage_lineZ1_cyan,cyan_soft);
  initialize_XImage(XImage_lineZ1_red,red);
  initialize_XImage(XImage_lineZ2_cyan,cyan_soft);
  initialize_XImage(XImage_lineZ2_red,red);

  // Create a pixel map for the image data.
  XPixmap_image = XCreatePixmap(XDisplay, XDisplay_Window,
				mosaic.ImageRegionPixels.x+mosaic.nx_ColorBar,
				mosaic.ImageRegionPixels.y, XDisplay_depth);

  // Create a pixel map for the top menu.
  XPixmap_TopMenu    = XCreatePixmap(XDisplay, XDisplay_Window,
				     XImage_TopMenu->width,XImage_TopMenu->height,XDisplay_depth);
  // Create a pixel map for the bottom menu.
  XPixmap_BottomMenu = XCreatePixmap(XDisplay, XDisplay_Window,
				     XImage_BottomMenu->width,XImage_BottomMenu->height,XDisplay_depth);
  // Create a pixel map for the info area.
  XPixmap_ImageInfoArea   = XCreatePixmap(XDisplay, XDisplay_Window,
				     XImage_ImageInfoArea->width,XImage_ImageInfoArea->height,XDisplay_depth);
  // Create a pixel map for the info area.
  XPixmap_RightPanel = XCreatePixmap(XDisplay, XDisplay_Window,
				     XImage_RightPanel->width,XImage_RightPanel->height,XDisplay_depth);
  // Map the window.
  XMapWindow(XDisplay, XWindow_image);

  // Start with the background image behind the image data.
  XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_background, 0,0,0,0,
	     XImage_background->width,XImage_background->height );

  // Initialize the background for the top menu.
  XPutImage( XDisplay, XPixmap_TopMenu, XGC_image, XImage_TopMenu, 0,0,0,0,
	     XImage_TopMenu->width,XImage_TopMenu->height );
  // Initialize the background for the bottom menu.
  XPutImage( XDisplay, XPixmap_BottomMenu, XGC_image, XImage_BottomMenu, 0,0,0,0,
	     XImage_BottomMenu->width,XImage_BottomMenu->height );
  // Initialize the background for the info area.
  XPutImage( XDisplay, XPixmap_ImageInfoArea, XGC_image, XImage_ImageInfoArea, 0,0,0,0,
	     XImage_ImageInfoArea->width,XImage_ImageInfoArea->height );
  // Initialize the background for the right panel.
  XPutImage( XDisplay, XPixmap_RightPanel, XGC_image, XImage_RightPanel, 0,0,0,0,
	     XImage_RightPanel->width,XImage_RightPanel->height );
}

void compute_display_size( IPOINT2D *RoughDisplaySize, float PixelScale )
// Compute Display size, plus:
// - mosaic.PixelResolution
// - mosaic.PixelDim
// - mosaic.ImageRegionPixels
{
  static int save_mosaic_orientation=-1, save_DimPanelsZ=-1, save_DisplayVolumeDimZ;
  float pixel_resolution;
  IPOINT2D PixelDim2D;
  int j;

  if ( mosaic.display_3orthogonal )
    {
      mosaic.DisplayVolumeDim = mosaic.stack->dim;
      mosaic.DisplayVolumeRes = mosaic.stack->resolution;
      pixel_resolution = MIN(mosaic.stack->resolution.x,
			     MIN(mosaic.stack->resolution.y,mosaic.stack->resolution.z));
    }
  else
    {
      // Determine the pixel resolution in each dimension.
      if ( mosaic.mosaic_orientation == ORTHO_XY )
	{
	  mosaic.DisplayVolumeDim = mosaic.stack->dim;
	  mosaic.DisplayVolumeRes = mosaic.stack->resolution;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_ZY )
	{
	  mosaic.DisplayVolumeDim.x = mosaic.stack->dim.z;
	  mosaic.DisplayVolumeDim.y = mosaic.stack->dim.y;
	  mosaic.DisplayVolumeDim.z = mosaic.stack->dim.x;
	  mosaic.DisplayVolumeRes.x = mosaic.stack->resolution.z;
	  mosaic.DisplayVolumeRes.y = mosaic.stack->resolution.y;
	  mosaic.DisplayVolumeRes.z = mosaic.stack->resolution.x;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	{
	  mosaic.DisplayVolumeDim.x = mosaic.stack->dim.x;
	  mosaic.DisplayVolumeDim.y = mosaic.stack->dim.z;
	  mosaic.DisplayVolumeDim.z = mosaic.stack->dim.y;
	  mosaic.DisplayVolumeRes.x = mosaic.stack->resolution.x;
	  mosaic.DisplayVolumeRes.y = mosaic.stack->resolution.z;
	  mosaic.DisplayVolumeRes.z = mosaic.stack->resolution.y;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_YZ )
	{
	  mosaic.DisplayVolumeDim.x = mosaic.stack->dim.y;
	  mosaic.DisplayVolumeDim.y = mosaic.stack->dim.z;
	  mosaic.DisplayVolumeDim.z = mosaic.stack->dim.x;
	  mosaic.DisplayVolumeRes.x = mosaic.stack->resolution.y;
	  mosaic.DisplayVolumeRes.y = mosaic.stack->resolution.z;
	  mosaic.DisplayVolumeRes.z = mosaic.stack->resolution.x;
	}
      pixel_resolution = MIN(mosaic.DisplayVolumeRes.x,mosaic.DisplayVolumeRes.y);
    }

  if ( save_DisplayVolumeDimZ!=mosaic.DisplayVolumeDim.z )
    { // If "slice dimensions" change through a change in orientation,
      // free and reallocate the display vector.
      if ( mosaic.display_slice )
	free_int_vector(mosaic.display_slice);
      mosaic.display_slice = int_vector(mosaic.DisplayVolumeDim.z);
      // Initialize to display everything.
      for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
	mosaic.display_slice[j] = 1;
      // Don't display more panels than slices in the mosaic.
      mosaic.DimPanels.z = MIN(mosaic.DimPanels.z,mosaic.DisplayVolumeDim.z);
    }

  if ( mosaic.DimPanels.z == 0 )
    mosaic.DimPanels.z = mosaic.DisplayVolumeDim.z; // set panels to slices
  else if ( mosaic.DimPanels.z == mosaic.DisplayVolumeDim.z )
    { // If the # of panels equals the number of slices, set all slices to 1.
      for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
	mosaic.display_slice[j] = 1;
    }
  else if ( mosaic.DimPanels.z < mosaic.DisplayVolumeDim.z )
    { //This is a mosaic image for which # panels < # slices
      if ( (save_DisplayVolumeDimZ != mosaic.DisplayVolumeDim.z) ||
	   (save_mosaic_orientation != mosaic.mosaic_orientation) )
	{ // If the number of slices were changed (change in orientation), then
	  // select the central set of slices.
	  for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
	    mosaic.display_slice[j] = 0;
	  if ( mosaic.DimPanels.z%2 )
	    {
	      for (j=mosaic.DisplayVolumeDim.z/2-mosaic.DimPanels.z/2;
		   j<=mosaic.DisplayVolumeDim.z/2+mosaic.DimPanels.z/2;
		   j++)
		mosaic.display_slice[j] = 1;
	    }
	  else
	    {
	      for (j=mosaic.DisplayVolumeDim.z/2-mosaic.DimPanels.z/2;
		   j<mosaic.DisplayVolumeDim.z/2+mosaic.DimPanels.z/2;
		   j++)
		mosaic.display_slice[j] = 1;
	    }
	}
    }

  if ( (mosaic.DimPanels.x == 0) || (save_DimPanelsZ != mosaic.DimPanels.z) )
    // Automatically set both x and y panels.
    automatically_set_panels( mosaic.DimPanels.z, &(mosaic.DimPanels.x), &(mosaic.DimPanels.y) );
  else
    {
      // Adjust y panels to fit all slices.
      mosaic.DimPanels.y = mosaic.DimPanels.z;
      while ( ((JIP_INDEX)(mosaic.DimPanels.x)) * (mosaic.DimPanels.y-1) >= mosaic.DimPanels.z )
	mosaic.DimPanels.y--;
    }
  save_mosaic_orientation = mosaic.mosaic_orientation;
  save_DimPanelsZ         = mosaic.DimPanels.z;
  save_DisplayVolumeDimZ  = mosaic.DisplayVolumeDim.z;
  // 
  pixel_resolution *= PixelScale;
  mosaic.PixelResolution.x = mosaic.stack->resolution.x / pixel_resolution;
  mosaic.PixelResolution.y = mosaic.stack->resolution.y / pixel_resolution;
  mosaic.PixelResolution.z = mosaic.stack->resolution.z / pixel_resolution;
  // Define the number of pixels in x, y, z (excluding zoom factor)
  mosaic.PixelDim.x = (int)(mosaic.stack->dim.x*mosaic.PixelResolution.x);
  if ( mosaic.PixelDim.x != mosaic.stack->dim.x*mosaic.PixelResolution.x )
    mosaic.PixelDim.x++;
  mosaic.PixelDim.y = (int)(mosaic.stack->dim.y*mosaic.PixelResolution.y);
  if ( mosaic.PixelDim.y != mosaic.stack->dim.y*mosaic.PixelResolution.y )
    mosaic.PixelDim.y++;
  mosaic.PixelDim.z = (int)(mosaic.stack->dim.z*mosaic.PixelResolution.z);
  if ( mosaic.PixelDim.z != mosaic.stack->dim.z*mosaic.PixelResolution.z )
    mosaic.PixelDim.z++;

  if ( mosaic.display_3orthogonal )
    { // For orthogonal projections, 3 borders + image pixels
      if ( mosaic.display_ortho_ZY )
	{ // z-y
	  mosaic.ImageRegionPixels.x = MAX(1,mosaic.zoom)*( mosaic.PixelDim.x + 3*NBORDER
							    + mosaic.PixelDim.z );
	  mosaic.ImageRegionPixels.y = MAX(1,mosaic.zoom)*( mosaic.PixelDim.y + 3*NBORDER
							    + mosaic.PixelDim.z );
	}
      else
	{ // y-z
	  mosaic.ImageRegionPixels.x = MAX(1,mosaic.zoom)*( mosaic.PixelDim.x + 3*NBORDER
							    + mosaic.PixelDim.y );
	  mosaic.ImageRegionPixels.y = MAX(1,mosaic.zoom)*( mosaic.PixelDim.y + 3*NBORDER
						     + mosaic.PixelDim.z );
	}
    }
  else
    { // For mosaic presentation, use 1 border between each image slice
      if ( mosaic.mosaic_orientation == ORTHO_XY )
	{
	  PixelDim2D.x = mosaic.PixelDim.x;
	  PixelDim2D.y = mosaic.PixelDim.y;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_ZY )
	{
	  PixelDim2D.x = mosaic.PixelDim.z;
	  PixelDim2D.y = mosaic.PixelDim.y;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	{
	  PixelDim2D.x = mosaic.PixelDim.x;
	  PixelDim2D.y = mosaic.PixelDim.z;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_YZ )
	{
	  PixelDim2D.x = mosaic.PixelDim.y;
	  PixelDim2D.y = mosaic.PixelDim.z;
	}
      mosaic.ImageRegionPixels.x = jip_coordinate(MAX(1,mosaic.zoom) * ( ((JIP_INDEX)(mosaic.DimPanels.x))*PixelDim2D.x
							  + (mosaic.DimPanels.x+1)*NBORDER ));
      mosaic.ImageRegionPixels.y = jip_coordinate(MAX(1,mosaic.zoom) * ( ((JIP_INDEX)(mosaic.DimPanels.y))*PixelDim2D.y
							  + (mosaic.DimPanels.y+1)*NBORDER ));
    }

  if ( mosaic.Attach_ColorBar )
    {
      mosaic.nx_ColorBar = mosaic.ImageRegionPixels.x/8;
      if ( mosaic.nx_ColorBar <  50 ) mosaic.nx_ColorBar = 50;
      if ( mosaic.nx_ColorBar > 100 ) mosaic.nx_ColorBar = 100;
    }
  else
    mosaic.nx_ColorBar = 0;
  
  RoughDisplaySize->x = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar + mosaic.nx_RightPanel;
  RoughDisplaySize->y = mosaic.ImageRegionPixels.y + 5 * INFO_HEIGHT;

}

void initialize_XImage( XImage *Image, unsigned long color )
{
  int jx, jy;

  for (jx=0; jx<Image->width; jx++)
    {
      for (jy=0; jy<Image->height; jy++)
	XPutPixel(Image,jx,jy,color);
    }
}

void make_XWindow_graph()
{
  unsigned long red, green, green_dark, blue, white, yellow, orange, gray, cyan, cyan_dark, black, purple;
  int lStatus, lPixelsX_GraphInfoArea;
  int jevent, jx, jy;
  
  if ( graph.nwidth == 0 || XWindow_graph == 0 ) // should be redundant
    {
      graph.nwidth  = XDisplayWidth(XDisplay,XScreen) /2;
      graph.nheight = XDisplayHeight(XDisplay,XScreen)/3;
    }
  else
    {
      XDestroyWindow(XDisplay,XWindow_graph);
      XDestroyImage(XImage_GraphInfoArea);  // frees image AND data
      XWindow_graph = 0;
    }

  lPixelsX_GraphInfoArea = MIN(200,graph.nwidth-1);  // fixed value

  // Create the window.
  XWindow_graph = XCreateSimpleWindow(XDisplay, XDisplay_Window, 1, 1,
				      graph.nwidth, graph.nheight+4*INFO_HEIGHT,
				      100, WHITE_PIXEL, BLACK_PIXEL);
  (void) XSetWMProtocols(XDisplay, XWindow_graph, &wm_delete_window, 1);
  // Define the events.
  XSelectInput(XDisplay, XWindow_graph, ExposureMask|ButtonPressMask|ButtonReleaseMask
	       |PointerMotionMask|KeyPressMask|StructureNotifyMask|LeaveWindowMask);

  // Give the window a name.
  label_graph_window();

  // Graphics contexts for graph
  XGC_graph_white  = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_yellow = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_orange = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_red    = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_blue   = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_green  = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_gray   = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_cyan   = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_purple = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_white_text = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_cyan_text  = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_red_text   = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_green_text = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_blue_text  = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_gray_text  = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_yellow_text= (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_orange_text= (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XGC_graph_purple_text= (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  for (jevent=0; jevent<9; jevent++)
    XGC_graph_event[jevent] = (GC)XCreateGC(XDisplay, XWindow_graph, 0, NULL);
  XSetGraphicsExposures(XDisplay, XGC_graph_white,      False);
  XSetGraphicsExposures(XDisplay, XGC_graph_yellow,     False);
  XSetGraphicsExposures(XDisplay, XGC_graph_orange,     False);
  XSetGraphicsExposures(XDisplay, XGC_graph_red,        False);
  XSetGraphicsExposures(XDisplay, XGC_graph_blue,       False);
  XSetGraphicsExposures(XDisplay, XGC_graph_green,      False);
  XSetGraphicsExposures(XDisplay, XGC_graph_gray,       False);
  XSetGraphicsExposures(XDisplay, XGC_graph_cyan,       False);
  XSetGraphicsExposures(XDisplay, XGC_graph_purple,     False);
  XSetGraphicsExposures(XDisplay, XGC_graph_cyan_text,  False);
  XSetGraphicsExposures(XDisplay, XGC_graph_red_text,   False);
  XSetGraphicsExposures(XDisplay, XGC_graph_green_text, False);
  XSetGraphicsExposures(XDisplay, XGC_graph_blue_text,  False);
  XSetGraphicsExposures(XDisplay, XGC_graph_gray_text,  False);
  XSetGraphicsExposures(XDisplay, XGC_graph_white_text, False);
  XSetGraphicsExposures(XDisplay, XGC_graph_yellow_text,False);
  XSetGraphicsExposures(XDisplay, XGC_graph_orange_text,False);
  XSetGraphicsExposures(XDisplay, XGC_graph_purple_text,False);
  for (jevent=0; jevent<9; jevent++)
    XSetGraphicsExposures(XDisplay, XGC_graph_event[jevent], False);
  black  = 0;
  red    = WHITE_PIXEL << 16;
  green  = WHITE_PIXEL << 8;
  blue   = WHITE_PIXEL;
  white  = red + green + blue;
  yellow = red + green;
  purple = red + blue;
  orange = red + ((WHITE_PIXEL/2)<<8);
  gray   = ((WHITE_PIXEL/4)<<16) + ((WHITE_PIXEL/4)<<8) + (WHITE_PIXEL/4);
  cyan   = green + blue;
  cyan_dark = ((3*WHITE_PIXEL/8)<<8) + (3*WHITE_PIXEL/8); // cyan, less bright
  green_dark = (WHITE_PIXEL/2) << 8;
  // Points are white, lines are yellow
  XSetState(XDisplay, XGC_graph_white,   white, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_yellow, yellow, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_orange, orange, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_red,       red, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_blue,     blue, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_green,green_dark,0,       GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_gray,gray,      0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_cyan,cyan_dark, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_purple, purple, 0,        GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_cyan_text,      0, cyan,  GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_red_text,       0, red,   GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_green_text,     0, green, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_blue_text,      0, blue,  GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_gray_text,      0, gray,  GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_white_text,     0, white, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_yellow_text,    0,yellow, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_orange_text,    0,orange, GXcopy, AllPlanes);
  XSetState(XDisplay, XGC_graph_purple_text,    0,purple, GXcopy, AllPlanes);

  XSetLineAttributes(XDisplay, XGC_graph_white,   2, LineSolid, CapRound, JoinRound);

  XData_GraphInfoArea  = (char *)malloc((size_t)jip_product(jip_product(jip_product(lPixelsX_GraphInfoArea,3),INFO_HEIGHT),XDisplay_depth));
  XImage_GraphInfoArea = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_GraphInfoArea,jip_int_count(lPixelsX_GraphInfoArea),jip_int_count(3*INFO_HEIGHT), 8, 0);
  lStatus = XInitImage(XImage_GraphInfoArea);

  // Initialize graph data
  for (jx=0; jx<XImage_GraphInfoArea->width; jx++)
    {
      for (jy=0; jy<XImage_GraphInfoArea->height; jy++)
	XPutPixel(XImage_GraphInfoArea,jx,jy,white);
    }

  // Create a pixel map for the info area.
  XPixmap_GraphInfoArea = XCreatePixmap(XDisplay, XDisplay_Window,
					XImage_GraphInfoArea->width,XImage_GraphInfoArea->height,
					XDisplay_depth);

  // Initialize the background for the info area.
  XPutImage( XDisplay, XPixmap_GraphInfoArea, XGC_image, XImage_GraphInfoArea, 0,0,0,0,
	     XImage_GraphInfoArea->width,XImage_GraphInfoArea->height );

  // Map the window.
  XMapWindow(XDisplay, XWindow_graph);
}

void change_window_name()
{
  static char window_string[LEN_STR];

  // Give the window a name.
  if ( mosaic.activation.visible )
    sprintf(window_string,"%s (zoom = %d), map = %s",program_name,mosaic.zoom,mosaic.activation.map_name);
  else
    sprintf(window_string,"%s (zoom = %d)",program_name,mosaic.zoom);
  XStoreName(XDisplay, XWindow_image, window_string);
}

void make_orthogonal_images()
{
  IPOINT2D iPut;

  ///////////////////////////////////////////////////////////////////////////////////////
  // Generate the images in x-y, z-y, and x-z
  ///////////////////////////////////////////////////////////////////////////////////////
  // 0: x-y slice
  make_image(ORTHO_XY,0);
  // 2: x-z slice
  make_image(ORTHO_XZ,0);
  if ( mosaic.display_ortho_ZY )
    // 1: z-y slice
    make_image(ORTHO_ZY,0);
  else
    make_image(ORTHO_YZ,0);

  // Paste the 3 images onto the pixmap.
  // x-y
  iPut.x = mosaic.zoom * NBORDER;
  iPut.y = mosaic.zoom * NBORDER;
  XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_XY, 0,0,iPut.x,iPut.y,
	     XImage_XY->width,XImage_XY->height );
  // x-z
  iPut.x = mosaic.zoom * NBORDER;
  iPut.y = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + 2*NBORDER ));
  XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_XZ, 0,0,iPut.x,iPut.y,
	     XImage_XZ->width,XImage_XZ->height );
  if ( mosaic.display_ortho_ZY )
    { // z-y
      iPut.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + 2*NBORDER ));
      iPut.y = mosaic.zoom * NBORDER;
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_ZY, 0,0,iPut.x,iPut.y,
		 XImage_ZY->width,XImage_ZY->height );
    }
  else
    { // y-z
      iPut.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + 2*NBORDER ));
      iPut.y = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + 2*NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_YZ, 0,0,iPut.x,iPut.y,
		 XImage_YZ->width,XImage_YZ->height );
    }
}

void make_mosaic_images()
{
  IPOINT2D iPanel, iPut;
  int jSlice, iSlice;

  if ( mosaic.mosaic_orientation == ORTHO_XY )
    {
      for (jSlice=0, iSlice=0; jSlice<mosaic.DisplayVolumeDim.z; jSlice++)  // loop over z slices
	{
	  if ( mosaic.display_slice[jSlice] )
	    {
	      make_image(MOSAIC,jSlice);
	      iPanel.x = iSlice % mosaic.DimPanels.x;
	      iPanel.y = iSlice / mosaic.DimPanels.x;
	      iPut.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.x)) * mosaic.PixelDim.x + (iPanel.x+1) * NBORDER ));
	      iPut.y = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.y)) * mosaic.PixelDim.y + (iPanel.y+1) * NBORDER ));
	      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_XY, 0,0,iPut.x,iPut.y,
			 XImage_XY->width,XImage_XY->height );
	      iSlice++;
	    }
	}
    }
  else if ( mosaic.mosaic_orientation == ORTHO_ZY )
    {
      for (jSlice=0, iSlice=0; jSlice<mosaic.stack->dim.x; jSlice++)  // loop over x slices
	{
	  if ( mosaic.display_slice[jSlice] )
	    {
	      make_image(MOSAIC,jSlice);
	      iPanel.x = iSlice % mosaic.DimPanels.x;
	      iPanel.y = iSlice / mosaic.DimPanels.x;
	      iPut.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.x)) * mosaic.PixelDim.z + (iPanel.x+1) * NBORDER ));
	      iPut.y = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.y)) * mosaic.PixelDim.y + (iPanel.y+1) * NBORDER ));
	      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_ZY, 0,0,iPut.x,iPut.y,
			 XImage_ZY->width,XImage_ZY->height );
	      iSlice++;
	    }
	}
    }
  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
    {
      for (jSlice=0, iSlice=0; jSlice<mosaic.stack->dim.y; jSlice++)  // loop over y slices
	{
	  if ( mosaic.display_slice[jSlice] )
	    {
	      make_image(MOSAIC,jSlice);
	      iPanel.x = iSlice % mosaic.DimPanels.x;
	      iPanel.y = iSlice / mosaic.DimPanels.x;
	      iPut.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.x)) * mosaic.PixelDim.x + (iPanel.x+1) * NBORDER ));
	      iPut.y = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.y)) * mosaic.PixelDim.z + (iPanel.y+1) * NBORDER ));
	      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_XZ, 0,0,iPut.x,iPut.y,
			 XImage_XZ->width,XImage_XZ->height );
	      iSlice++;
	    }
	}
    }
  else if ( mosaic.mosaic_orientation == ORTHO_YZ )
    {
      for (jSlice=0, iSlice=0; jSlice<mosaic.stack->dim.x; jSlice++)  // loop over x slices
	{
	  if ( mosaic.display_slice[jSlice] )
	    {
	      make_image(MOSAIC,jSlice);
	      iPanel.x = iSlice % mosaic.DimPanels.x;
	      iPanel.y = iSlice / mosaic.DimPanels.x;
	      iPut.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.x)) * mosaic.PixelDim.y + (iPanel.x+1) * NBORDER ));
	      iPut.y = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(iPanel.y)) * mosaic.PixelDim.z + (iPanel.y+1) * NBORDER ));
	      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_YZ, 0,0,iPut.x,iPut.y,
			 XImage_YZ->width,XImage_YZ->height );
	      iSlice++;
	    }
	}
    }
}

void make_image( int which_image, int iXYZ )
{
  XImage *image;
  float fData, fMap, fOvl, fCross, fProj;
  int jx, jy, jx_zoom, jy_zoom, ovl_color, iCross;
  int which_orientation;
  unsigned long pixel;
  BOOL first_zoom, OnGrid;
  IPOINT2D PixelIndex, PixelDim;

  which_orientation = which_image;
  if ( which_image == MOSAIC )
    which_orientation = mosaic.mosaic_orientation;

  if ( which_orientation == ORTHO_XY )
    { // X-Y
      PixelDim.x = mosaic.PixelDim.x;
      PixelDim.y = mosaic.PixelDim.y;
      OnGrid           = mosaic.OnGridXY;
      image            = XImage_XY;
      if ( which_image == MOSAIC )
	iCross        = iXYZ; // assign slice numbers for mosaic
      else
	iCross        = 0;
    }
  else if ( which_orientation == ORTHO_ZY )
    { // Z-Y
      PixelDim.x = mosaic.PixelDim.z;
      PixelDim.y = mosaic.PixelDim.y;
      OnGrid           = mosaic.OnGridZY;
      image            = XImage_ZY;
      if ( which_image == MOSAIC )
	iCross        = iXYZ; // assign slice numbers for mosaic
      else
	iCross        = 0;
    }
  else if ( which_orientation == ORTHO_XZ )
    { // X-Z
      PixelDim.x = mosaic.PixelDim.x;
      PixelDim.y = mosaic.PixelDim.z;
      OnGrid           = mosaic.OnGridXZ;
      image            = XImage_XZ;
      if ( which_image == MOSAIC )
	iCross        = iXYZ; // assign slice numbers for mosaic
      else
	iCross        = 0;
    }
  else if ( which_orientation == ORTHO_YZ )
    { // Y-Z
      PixelDim.x = mosaic.PixelDim.y;
      PixelDim.y = mosaic.PixelDim.z;
      OnGrid           = mosaic.OnGridYZ;
      image            = XImage_YZ;
      if ( which_image == MOSAIC )
	iCross        = iXYZ; // assign slice numbers for mosaic
      else
	iCross        = 0;
    }

  for (jy=0; jy<PixelDim.y; jy++)
    {
      for (jx=0; jx<PixelDim.x; jx++)
	{
	  for (jy_zoom=0; jy_zoom<mosaic.zoom; jy_zoom++)
	    {
	      PixelIndex.y = jy*mosaic.zoom + jy_zoom;
	      for (jx_zoom=0; jx_zoom<mosaic.zoom; jx_zoom++)
		{
		  PixelIndex.x = jx*mosaic.zoom + jx_zoom;
		  first_zoom = (jx_zoom==0) && (jy_zoom==0);
		  // To speed image display, don't recalculate pixels unless required.
		  if ( first_zoom || !OnGrid )
		    {
		      interpolate_2d(     iCross, which_image, PixelIndex,
					  &fData, &fMap, &fOvl, &fCross, &fProj, &ovl_color);
		      pixel = image_pixel(fData,   fMap,  fOvl,  fCross,  fProj,  ovl_color);
		    }
		  XPutPixel(image,PixelIndex.x, PixelIndex.y, pixel);
		} // jx_zoom
	    } // jy_zoom
	} // jx
    } // jy
}

void label_planes_ortho()
{
  char string[LEN_STR];
  int istep, which_orientation;
  POINT3D pos;
  IPOINT3D ipos;
  IPOINT2D iPut, iBack;
  static BOOL first_time=TRUE;

  // labels
  iBack.x = 30;
  iBack.y = 8;
  if ( mosaic.display_grid == 1 || mosaic.display_grid == 3 )
    {
      // Planes label: X-Y
      // Put position with respect to bregma on image
      ipos.x = mosaic.stack->dim.x;
      if ( mosaic.flip_X11_y )
	ipos.y = 0;
      else
	ipos.y = mosaic.stack->dim.y;
      ipos.z = mosaic.icross.z;
      pos = Convert_Voxel_to_Space(ipos,mosaic.stack);
      iPut = Convert_Space_to_Pixel(ORTHO_XY,pos,1);
      iPut.x -= iBack.x;
      iPut.y -= iBack.y;
      if ( pos.z > 0 )
	sprintf(string,"+%g",pos.z);
      else
	sprintf(string,"%g",pos.z);
      DPRIMITIVE_DrawText(Drawable_ImageData, COLOR_CYAN, iPut, string);
      
      // Planes label: X-Z
      // Put position with respect to bregma on image
      ipos.x = mosaic.stack->dim.x;
      if ( mosaic.flip_X11_y )
	ipos.z = 0;
      else
	ipos.z = mosaic.stack->dim.z;
      ipos.y = mosaic.icross.y;
      pos = Convert_Voxel_to_Space(ipos,mosaic.stack);
      iPut = Convert_Space_to_Pixel(ORTHO_XZ,pos,1);
      iPut.x -= iBack.x;
      iPut.y -= iBack.y;
      if ( pos.y > 0 )
	sprintf(string,"+%g",pos.y);
      else
	sprintf(string,"%g",pos.y);
      DPRIMITIVE_DrawText(Drawable_ImageData, COLOR_CYAN, iPut, string);
      
      // Planes label: Y-Z or Z-Y slice
      // Put position with respect to bregma on image
      if ( mosaic.display_ortho_ZY )
	{
	  which_orientation = ORTHO_ZY;
	  ipos.z = mosaic.stack->dim.z;
	  if ( mosaic.flip_X11_y )
	    ipos.y = 0;
	  else
	    ipos.y = mosaic.stack->dim.y;
	}
      else
	{
	  which_orientation = ORTHO_YZ;
	  ipos.y = mosaic.stack->dim.y;
	  if ( mosaic.flip_X11_y )
	    ipos.z = 0;
	  else
	    ipos.z = mosaic.stack->dim.z;
	}
      ipos.x = mosaic.icross.x;
      pos = Convert_Voxel_to_Space(ipos,mosaic.stack);
      iPut = Convert_Space_to_Pixel(which_orientation,pos,1);
      iPut.x -= iBack.x;
      iPut.y -= iBack.y;
      if ( pos.x > 0 )
	sprintf(string,"+%g",pos.x);
      else
	sprintf(string,"%g",pos.x);
      DPRIMITIVE_DrawText(Drawable_ImageData, COLOR_CYAN, iPut, string);
    }

  // ticks
  if ( mosaic.display_grid == 2 || mosaic.display_grid == 3 )
    {
      // step size in mm
      istep = 10 * mosaic.stack->resolution.x;
      if ( first_time && mosaic.display_grid == 2  )
	{
	  printf("label step size = %d mm\n",istep);
	  first_time = FALSE;
	}
      // x-y slice
      horizontal_ticks_on_axis(ORTHO_XY, istep, -1);
      vertical_ticks_on_axis(  ORTHO_XY, istep, -1);
      // y-z or z-y slice
      if ( mosaic.display_ortho_ZY )
	which_orientation = ORTHO_ZY;
      else
	which_orientation = ORTHO_YZ;
      horizontal_ticks_on_axis(which_orientation, istep, -1);
      vertical_ticks_on_axis(  which_orientation, istep, -1);
      // x-z slice
      horizontal_ticks_on_axis(ORTHO_XZ, istep, -1);
      vertical_ticks_on_axis(  ORTHO_XZ, istep, -1);
    }
}

void label_planes_mosaic()
{
  char string[LEN_STR];
  IPOINT3D ipos;
  POINT3D pos;
  IPOINT2D iPut, iBack;
  int istep, jSlice;
  static BOOL first_time=TRUE;

  if ( mosaic.mosaic_orientation == ORTHO_XY )
    ipos.x = ipos.y = 0;
  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
    ipos.x = ipos.z = 0;
  else // yz or zy
    ipos.y = ipos.z = 0;
  for (jSlice=0; jSlice<mosaic.DisplayVolumeDim.z; jSlice++)  // loop over nz slices
    {
      if ( mosaic.mosaic_orientation == ORTHO_XY )
	ipos.z = jSlice;
      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	ipos.y = jSlice;
      else // yz or zy
	ipos.x = jSlice;
      if ( mosaic.display_slice[jSlice] )
	{
	  // text labels for slice positions
	  iBack.x = 30;
	  iBack.y = 8;
	  if ( mosaic.display_grid == 1 || mosaic.display_grid == 3 )
	    {
	      if ( mosaic.mosaic_orientation == ORTHO_XY )
		{
		  ipos.x = mosaic.stack->dim.x;
		  if ( mosaic.flip_X11_y )
		    ipos.y = 0;
		  else
		    ipos.y = mosaic.stack->dim.y;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
		{
		  ipos.x = mosaic.stack->dim.x;
		  if ( mosaic.flip_X11_y )
		    ipos.z = 0;
		  else
		    ipos.z = mosaic.stack->dim.z;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_YZ )
		{
		  ipos.y = mosaic.stack->dim.y;
		  if ( mosaic.flip_X11_y )
		    ipos.z = 0;
		  else
		    ipos.z = mosaic.stack->dim.z;
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_ZY )
		{
		  ipos.z = mosaic.stack->dim.z;
		  if ( mosaic.flip_X11_y )
		    ipos.y = 0;
		  else
		    ipos.y = mosaic.stack->dim.y;
		}
	      pos  = Convert_Voxel_to_Space(ipos,mosaic.stack);
	      iPut = Convert_Space_to_Pixel(MOSAIC,pos,1);
	      iPut.x -= iBack.x;
	      iPut.y -= iBack.y;
	      if ( mosaic.mosaic_orientation == ORTHO_XY )
		{
		  if ( pos.z > 0 )
		    sprintf(string,"+%g",pos.z);
		  else
		    sprintf(string,"%g",pos.z);
		}
	      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
		{
		  if ( pos.y > 0 )
		    sprintf(string,"+%g",pos.y);
		  else
		    sprintf(string,"%g",pos.y);
		}
	      else
		{
		  if ( pos.x > 0 )
		    sprintf(string,"+%g",pos.x);
		  else
		    sprintf(string,"%g",pos.x);
		}
	      //	      DPRIMITIVE_FillText(Drawable_ImageData, COLOR_CYAN, iPut, string);
	      DPRIMITIVE_DrawText(Drawable_ImageData, COLOR_CYAN, iPut, string);
	    }
	  // ticks
	  if ( mosaic.display_grid == 2 || mosaic.display_grid == 3 )
	    {
	      // step size in mm
	      istep = 10 * mosaic.stack->resolution.x;
	      if ( first_time && mosaic.display_grid == 2  )
		{
		  printf("label step size = %d mm\n",istep);
		  first_time = FALSE;
		}
	      horizontal_ticks_on_axis(MOSAIC, istep, jSlice);
	      vertical_ticks_on_axis(MOSAIC, istep, jSlice);
	    }
	}
    }
}

void horizontal_ticks_on_axis( int which_orientation, int istep, int iZ )
{
  float rMin, rMax;
  int istart, jstep, orientation;
  POINT3D pos1, pos2;
  IPOINT2D iDraw1, iDraw2;
  IPOINT3D ipos;

  if ( which_orientation == MOSAIC )
    orientation = mosaic.mosaic_orientation;
  else
    orientation = which_orientation;

  if ( orientation == ORTHO_XY || orientation == ORTHO_XZ )
    {
      rMin = MIN(mosaic.stack->origin.x,
		 mosaic.stack->origin.x+mosaic.stack->direction.x*mosaic.stack->fov.x);
      rMax = MAX(mosaic.stack->origin.x,
		 mosaic.stack->origin.x+mosaic.stack->direction.x*mosaic.stack->fov.x);
    }
  else if ( orientation == ORTHO_YZ )
    {
      rMin = MIN(mosaic.stack->origin.y,
		 mosaic.stack->origin.y+mosaic.stack->direction.y*mosaic.stack->fov.y);
      rMax = MAX(mosaic.stack->origin.y,
		 mosaic.stack->origin.y+mosaic.stack->direction.y*mosaic.stack->fov.y);
    }
  else if ( orientation == ORTHO_ZY )
    {
      rMin = MIN(mosaic.stack->origin.z,
		 mosaic.stack->origin.z+mosaic.stack->direction.z*mosaic.stack->fov.z);
      rMax = MAX(mosaic.stack->origin.z,
		 mosaic.stack->origin.z+mosaic.stack->direction.z*mosaic.stack->fov.z);
    }
  istart  = istep * (int)(rMin/(float)istep);
  istart -= istep;
  
  for (jstep=istart; jstep < rMax; jstep += istep)
    {
      if ( jstep <= rMin )
	continue;
      // horizontal axes
      if ( iZ >= 0 )
	{
	  if ( orientation == ORTHO_XY )
	    {
	      ipos.x = ipos.y = 0;
	      ipos.z = iZ;
	    }
	  else if ( orientation == ORTHO_XZ )
	    {
	      ipos.x = ipos.z = 0;
	      ipos.y = iZ;
	    }
	  else if ( orientation == ORTHO_YZ || orientation == ORTHO_ZY )
	    {
	      ipos.y = ipos.z = 0;
	      ipos.x = iZ;
	    }
	  pos1 = pos2 = Convert_Voxel_to_Space(ipos,mosaic.stack);
	}
      else
	pos1 = pos2 = Convert_Voxel_to_Space(mosaic.icross,mosaic.stack); // set default positions
      if ( orientation == ORTHO_XY || orientation == ORTHO_XZ )
	{
	  pos1.x = pos2.x = jstep;
	}
      else if ( orientation == ORTHO_YZ )
	pos1.y = pos2.y = jstep;
      else if ( orientation == ORTHO_ZY )
	pos1.z = pos2.z = jstep;
      // low tick marks
      if ( orientation == ORTHO_XY || orientation == ORTHO_ZY )
	{
	  pos1.y  = mosaic.stack->origin.y;
	  pos1.y -= mosaic.stack->direction.y*mosaic.stack->resolution.y/2;
	  pos2.y  = pos1.y + mosaic.stack->direction.y*mosaic.stack->resolution.y;
	}
      else if ( orientation == ORTHO_XZ || orientation == ORTHO_YZ )
	{
	  pos1.z  = mosaic.stack->origin.z;
	  pos1.z -= mosaic.stack->direction.z*mosaic.stack->resolution.z/2;
	  pos2.z  = pos1.z + mosaic.stack->direction.z*mosaic.stack->resolution.z;
	}
      iDraw1  = Convert_Space_to_Pixel(which_orientation,pos1,1);
      iDraw2  = Convert_Space_to_Pixel(which_orientation,pos2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      // high tick marks
      if ( orientation == ORTHO_XY || orientation == ORTHO_ZY )
	{
	  pos1.y  = mosaic.stack->origin.y + mosaic.stack->direction.y*mosaic.stack->fov.y;
	  pos1.y -= mosaic.stack->direction.y*mosaic.stack->resolution.y/2;
	  pos2.y  = pos1.y - mosaic.stack->direction.y*mosaic.stack->resolution.y;
	}
      else if ( orientation == ORTHO_XZ || orientation == ORTHO_YZ )
	{
	  pos1.z  = mosaic.stack->origin.z + mosaic.stack->direction.z*mosaic.stack->fov.z;
	  pos1.z -= mosaic.stack->direction.z*mosaic.stack->resolution.z/2;
	  pos2.z  = pos1.z - mosaic.stack->direction.z*mosaic.stack->resolution.z;
	}
      iDraw1  = Convert_Space_to_Pixel(which_orientation,pos1,1);
      iDraw2  = Convert_Space_to_Pixel(which_orientation,pos2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
    }
}

void vertical_ticks_on_axis( int which_orientation, int istep, int iZ )
{
  float rMin, rMax;
  int istart, jstep, orientation;
  POINT3D pos1, pos2;
  IPOINT2D iDraw1, iDraw2;
  IPOINT3D ipos;

  if ( which_orientation == MOSAIC )
    orientation = mosaic.mosaic_orientation;
  else
    orientation = which_orientation;

  if ( orientation == ORTHO_XY || orientation == ORTHO_ZY )
    {
      rMin = MIN(mosaic.stack->origin.y,
		 mosaic.stack->origin.y+mosaic.stack->direction.y*mosaic.stack->fov.y);
      rMax = MAX(mosaic.stack->origin.y,
		 mosaic.stack->origin.y+mosaic.stack->direction.y*mosaic.stack->fov.y);
    }
  else if ( orientation == ORTHO_YZ || orientation == ORTHO_XZ )
    {
      rMin = MIN(mosaic.stack->origin.z,
		 mosaic.stack->origin.z+mosaic.stack->direction.z*mosaic.stack->fov.z);
      rMax = MAX(mosaic.stack->origin.z,
		 mosaic.stack->origin.z+mosaic.stack->direction.z*mosaic.stack->fov.z);
    }
  istart  = istep * (int)(rMin/(float)istep);
  istart -= istep;
  
  for (jstep=istart; jstep < rMax; jstep += istep)
    {
      if ( jstep <= rMin )
	continue;
      // vertical axes
      // horizontal axes
      if ( iZ >= 0 )
	{
	  if ( orientation == ORTHO_XY )
	    {
	      ipos.x = ipos.y = 0;
	      ipos.z = iZ;
	    }
	  else if ( orientation == ORTHO_XZ )
	    {
	      ipos.x = ipos.z = 0;
	      ipos.y = iZ;
	    }
	  else if ( orientation == ORTHO_YZ || orientation == ORTHO_ZY )
	    {
	      ipos.y = ipos.z = 0;
	      ipos.x = iZ;
	    }
	  pos1 = pos2 = Convert_Voxel_to_Space(ipos,mosaic.stack);
	}
      else
	pos1 = pos2 = Convert_Voxel_to_Space(mosaic.icross,mosaic.stack); // set default positions
      if ( orientation == ORTHO_XY || orientation == ORTHO_ZY )
	pos1.y = pos2.y = jstep;
      else if ( orientation == ORTHO_YZ || orientation == ORTHO_XZ )
	pos1.z = pos2.z = jstep;
      // low tick marks
      if ( orientation == ORTHO_XY || orientation == ORTHO_XZ )
	{
	  pos1.x  = mosaic.stack->origin.x;
	  pos1.x -= mosaic.stack->direction.x*mosaic.stack->resolution.x/2;
	  pos2.x  = pos1.x + mosaic.stack->direction.x*mosaic.stack->resolution.x;
	}
      else if ( orientation == ORTHO_YZ )
	{
	  pos1.y  = mosaic.stack->origin.y;
	  pos1.y -= mosaic.stack->direction.y*mosaic.stack->resolution.y/2;
	  pos2.y  = pos1.y + mosaic.stack->direction.y*mosaic.stack->resolution.y;
	}
      else if ( orientation == ORTHO_ZY )
	{
	  pos1.z  = mosaic.stack->origin.z;
	  pos1.z -= mosaic.stack->direction.z*mosaic.stack->resolution.z/2;
	  pos2.z  = pos1.z + mosaic.stack->direction.z*mosaic.stack->resolution.z;
	}
      iDraw1  = Convert_Space_to_Pixel(which_orientation,pos1,1);
      iDraw2  = Convert_Space_to_Pixel(which_orientation,pos2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      // high tick marks
      if ( orientation == ORTHO_XY || orientation == ORTHO_XZ )
	{
	  pos1.x  = mosaic.stack->origin.x + mosaic.stack->direction.x*mosaic.stack->fov.x;
	  pos1.x -= mosaic.stack->direction.x*mosaic.stack->resolution.x/2;
	  pos2.x  = pos1.x - mosaic.stack->direction.x*mosaic.stack->resolution.x;
	}
      else if ( orientation == ORTHO_YZ )
	{
	  pos1.y  = mosaic.stack->origin.y + mosaic.stack->direction.y*mosaic.stack->fov.y;
	  pos1.y -= mosaic.stack->direction.y*mosaic.stack->resolution.y/2;
	  pos2.y  = pos1.y - mosaic.stack->direction.y*mosaic.stack->resolution.y;
	}
      else if ( orientation == ORTHO_ZY )
	{
	  pos1.z  = mosaic.stack->origin.z + mosaic.stack->direction.z*mosaic.stack->fov.z;
	  pos1.z -= mosaic.stack->direction.z*mosaic.stack->resolution.z/2;
	  pos2.z  = pos1.z - mosaic.stack->direction.z*mosaic.stack->resolution.z;
	}
      iDraw1  = Convert_Space_to_Pixel(which_orientation,pos1,1);
      iDraw2  = Convert_Space_to_Pixel(which_orientation,pos2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
    }
}

void highlight_image_panel( int which_panel, int color )
{
  XImage *XImage_lineX, *XImage_lineY;
  int lx, ly;

  if ( which_panel == 0 )
    { // x-y
      if ( color == 0 ) // cyan
	{
	  XImage_lineX = XImage_lineX_cyan;
	  XImage_lineY = XImage_lineY_cyan;
	}
      else
	{
	  XImage_lineX = XImage_lineX_red;
	  XImage_lineY = XImage_lineY_red;
	}
      // borders around ORTHO_XY plane
      lx = 0;
      ly = 0;
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
      		 XImage_lineX->width,XImage_lineX->height );
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
      		 XImage_lineY->width,XImage_lineY->height );
      lx = 0;
      ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
      		 XImage_lineX->width,XImage_lineX->height );
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER ));
      ly = 0;
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
      		 XImage_lineY->width,XImage_lineY->height );
    }
  else if ( which_panel == 1 )
    { // z-y
      if ( color == 0 ) // cyan
	{
	  XImage_lineX = XImage_lineZ1_cyan;
	  XImage_lineY = XImage_lineY_cyan;
	}
      else
	{
	  XImage_lineX = XImage_lineZ1_red;
	  XImage_lineY = XImage_lineY_red;
	}
      // borders around ORTHO_ZY plane
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER ));
      ly = 0;
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
      		 XImage_lineX->width,XImage_lineX->height );
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
      		 XImage_lineY->width,XImage_lineY->height );
      ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
      		 XImage_lineX->width,XImage_lineX->height );
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER + mosaic.PixelDim.z + NBORDER ));
      ly = 0;
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
      		 XImage_lineY->width,XImage_lineY->height );
    }
  else if ( which_panel == 2 )
    { // x-z
      // borders around ORTHO_XZ plane
      if ( color == 0 ) // cyan
	{
	  XImage_lineX = XImage_lineX_cyan;
	  XImage_lineY = XImage_lineZ2_cyan;
	}
      else
	{
	  XImage_lineX = XImage_lineX_red;
	  XImage_lineY = XImage_lineZ2_red;
	}
      lx = 0;
      ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
      		 XImage_lineX->width,XImage_lineX->height );
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
      		 XImage_lineY->width,XImage_lineY->height );
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
      		 XImage_lineY->width,XImage_lineY->height );
      lx = 0;
      ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + NBORDER + mosaic.PixelDim.z + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
      		 XImage_lineX->width,XImage_lineX->height );
    }
  else if ( which_panel == 3 )
    { // y-z
      if ( color == 0 ) // cyan
	{
	  XImage_lineX = XImage_lineY1_cyan;
	  XImage_lineY = XImage_lineZ2_cyan;
	}
      else
	{
	  XImage_lineX = XImage_lineY1_red;
	  XImage_lineY = XImage_lineZ2_red;
	}
      // borders around ORTHO_YZ plane
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER ));
      ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
		 XImage_lineX->width,XImage_lineX->height );
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
		 XImage_lineY->width,XImage_lineY->height );
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER + mosaic.PixelDim.y + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineY, 0,0,lx,ly,
		 XImage_lineY->width,XImage_lineY->height );
      lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.x + NBORDER ));
      ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( mosaic.PixelDim.y + NBORDER + mosaic.PixelDim.z + NBORDER ));
      XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_lineX, 0,0,lx,ly,
		 XImage_lineX->width,XImage_lineX->height );
    }
}

void action_bring_up_graph()
{
  if ( mosaic.stack->nt != 1 )
    {
      graph.type = 3;
      initialize_graph();
      make_XWindow_graph();
      //      update_graph_data(1);
    }
  else if ( graph.type != 0 || graph.nwidth == 0 || graph.nheight == 0 )
    {
      graph.type = 0;
      initialize_graph();      // type 0 = x graph
      make_XWindow_graph();
      //      update_graph_data(1);
    }
  else
    {
      XRaiseWindow(XDisplay, XWindow_graph);
      //      update_graph_data(0);
    }
  graph.update_regions |= UPDATE_GRAPH;
}

void action_select_slices()
{
  static char CNULL = '\0';
  XEvent event;
  char prompt[LEN_STR], prompt1[LEN_STR], options1[LEN_STR], options2[LEN_STR];
  char slices[LEN_STR], number_string[LEN_STR], key[LEN_STR];
  char which_key, *cptr;
  int j, xbut, ybut, length, all_displayed, mosaic_nx_panels_old, first_slice;
  int nz_temp, *display_slice_temp;
  static int mosaic_nx_panels_save=1, mosaic_ny_panels_save=1, zoom_save=0;
  BOOL crosshair_on_slice;
  IPOINT3D iDisplayVolumeCursor;

  // See if all slices are currently displayed.
  all_displayed = 1;
  for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
    all_displayed = all_displayed && mosaic.display_slice[j];
  // If a subset of slices is displayed, display all slices.
  if ( !all_displayed && !mosaic.display_3orthogonal )
    {
      mosaic.DimPanels.z = 0;  // all panels will be displayed
      mosaic_nx_panels_old = mosaic.DimPanels.x;
      if ( zoom_save != 0 )
	{ // ... then slices were previous selected, and saved values exist
	  mosaic.DimPanels.x = mosaic_nx_panels_save;
	  mosaic.DimPanels.y = mosaic_ny_panels_save;
	  mosaic.zoom = zoom_save;
	}
      else
	{ // ... then # slices was defined on program entry, not in this function
	  mosaic.DimPanels.x = 0;
	  mosaic.zoom = MAX(1,mosaic.zoom);
	}
      update_XWindow_image();
      return;
    }
  else
    { // Set all slices to "don't display"
      // .. in a temporary array
      display_slice_temp = int_vector(mosaic.DisplayVolumeDim.z);
      for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
	display_slice_temp[j] = 0;
      nz_temp = 0;  // temporary version of mosaic.DimPanels.z
    }
  // Try to select a subset of slices.
  if ( mosaic.display_3orthogonal )
    {
      sprintf(prompt1, "Input the number of slices >");
      sprintf(options1,"Valid numbers are 1 to %d",mosaic.DisplayVolumeDim.z);
      sprintf(options2," ");
    }
  else
    {
      sprintf(prompt1,"Select slices >");
      sprintf(options1,"text input or button selection:");
      sprintf(options2,"(number, button1, return, escape)");
    }
  show_on_info_line(0,1,prompt1,options1,options2);
  sprintf(number_string," "); // just initialize
  do
    {
      which_key = 0;
      if ( get_next_event(&event) == 1 )
	{
	  switch (event.type)
	    {
	    case KeyPress:
	      if ( !get_key_press(&(event.xkey), &which_key) ) continue;
	      if ( which_key == 8 || which_key == 127 ) // backspace or delete
		{
		  length = jip_int_count((JIP_INDEX)strlen(number_string));
		  if ( length > 1 )
		    {
		      cptr = number_string;   // space
		      while ( *cptr ) cptr++; // find end of string
		      cptr--;                 // go back one step
		      *cptr = CNULL;
		    }
		  strcpy(prompt,prompt1);
		  strcat(prompt,number_string);
		}
	      else if ( (which_key >= 48 && which_key <= 57) )
		{ // key must be number 0-9 or carriage return
		  sprintf(key,"%c",which_key);
		  strcat(number_string,key);
		  strcpy(prompt,prompt1);
		  strcat(prompt,number_string);
		}
	      show_on_info_line(0,1,prompt,options1,options2);
	      break;
	    case ButtonPress:
	      if ( mosaic.display_3orthogonal )
		{
		  sprintf(prompt1,"Mouse button not allowed for this option");
		  sprintf(options1,"when 3-plane mode is used.");
		  sprintf(options2," ");
		  show_on_info_line(0,1,prompt1,options1,options2);
		  break;  // cannot select slices with mouse button from 3-plane mode
		}
	      which_key = 0;
	      switch (event.xbutton.button)
		{	
		case 1: 
		  // Action here
		  xbut = event.xbutton.x;
		  ybut = event.xbutton.y;
		  define_mosaic_cursor_location(xbut, ybut);
		  if ( mosaic.mosaic_orientation == ORTHO_XY )
		    iDisplayVolumeCursor = mosaic.icursor;
		  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
		    {
		      iDisplayVolumeCursor.x = mosaic.icursor.x;
		      iDisplayVolumeCursor.y = mosaic.icursor.z;
		      iDisplayVolumeCursor.z = mosaic.icursor.y;
		    }
		  else if ( mosaic.mosaic_orientation == ORTHO_YZ )
		    {
		      iDisplayVolumeCursor.x = mosaic.icursor.y;
		      iDisplayVolumeCursor.y = mosaic.icursor.z;
		      iDisplayVolumeCursor.z = mosaic.icursor.x;
		    }
		  else if ( mosaic.mosaic_orientation == ORTHO_ZY )
		    {
		      iDisplayVolumeCursor.x = mosaic.icursor.z;
		      iDisplayVolumeCursor.y = mosaic.icursor.y;
		      iDisplayVolumeCursor.z = mosaic.icursor.x;
		    }

		  if ( mosaic.cursor_selection&CURSOR_ON_MOSAIC )
		    {
		      if ( iDisplayVolumeCursor.z < 0 || iDisplayVolumeCursor.z > mosaic.DisplayVolumeDim.z-1 )
			{
			  printf("Programming error: iDisplayVolumeCursor.z out of range.\n");
			  exit(1);
			}
		      display_slice_temp[iDisplayVolumeCursor.z] = !display_slice_temp[iDisplayVolumeCursor.z];
		      // Update the visual selection based upon border color.
		      update_borders_for_slices(mosaic.DisplayVolumeDim.z, display_slice_temp);
		      // Update the display list.
		      for (j=0, nz_temp=0; j<mosaic.DisplayVolumeDim.z; j++)
			{
			  if ( display_slice_temp[j] )
			    {
			      if ( nz_temp == 0 )
				sprintf(prompt,"Select slices (number, button1, return, escape) > %d ",j);
			      else
				{
				  sprintf(slices,", %d",j);
				  strcat(prompt,slices);
				}
			      nz_temp++;
			    }
			}
		      show_on_info_line(0,1,prompt,options1,options2);
		    }
		  break; 
		default:
		  continue;
		}
	    default:
	      break;
	    }
	}
    }
  while
    ( which_key != 13 && which_key != 27 ); // return and escape

  length = jip_int_count((JIP_INDEX)strlen(number_string));
  if ( which_key == 27 || (nz_temp == 0 && length==1) ) // escape
    { // return with previous configuration
      sprintf(prompt,"canceled");
      show_on_info_line(0,1,prompt,"","");
    }
  else
    { // slices were selected
      if ( length > 1 )
	{
	  sscanf(number_string,"%d",&nz_temp);
	  if ( nz_temp >= mosaic.DimPanels.z && !mosaic.display_3orthogonal )
	    {
	      show_on_info_line(0,0,"all slices shown","","");
	      return;
	    }
	  else
	    {
	      display_slice_temp = int_vector(mosaic.DisplayVolumeDim.z);
	      for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
		display_slice_temp[j] = 0;
	      if ( nz_temp%2 )
		{
		  for (j=mosaic.DisplayVolumeDim.z/2-nz_temp/2; j<=mosaic.DisplayVolumeDim.z/2+nz_temp/2; j++)
		    display_slice_temp[j] = 1;
		}
	      else
		{
		  for (j=mosaic.DisplayVolumeDim.z/2-nz_temp/2; j<mosaic.DisplayVolumeDim.z/2+nz_temp/2; j++)
		    display_slice_temp[j] = 1;
		}
	    }
	}
      mosaic.DimPanels.z = nz_temp;
      for (j=0, crosshair_on_slice=FALSE, first_slice=-1;
	   j<mosaic.DisplayVolumeDim.z; j++)
	{
	  mosaic.display_slice[j] = display_slice_temp[j];
	  if ( first_slice < 0 && mosaic.display_slice[j] == 1 )
	    first_slice = j;
	  if ( j == mosaic.icross.z )
	    crosshair_on_slice = ( mosaic.display_slice[j] == 1 );
	}
      if ( !crosshair_on_slice )
	{
	  mosaic.icross.x = mosaic.stack->dim.x/2;
	  mosaic.icross.y = mosaic.stack->dim.y/2;
	  mosaic.icross.z = first_slice;
	}
      mosaic_nx_panels_old = mosaic.DimPanels.x;
      mosaic_nx_panels_save = mosaic.DimPanels.x;
      mosaic_ny_panels_save = mosaic.DimPanels.y;
      zoom_save = mosaic.zoom;
      automatically_set_panels( mosaic.DimPanels.z, &(mosaic.DimPanels.x), &(mosaic.DimPanels.y) );
      mosaic.zoom = MAX(1,mosaic.zoom * mosaic_nx_panels_old/mosaic.DimPanels.x);
      mosaic.display_3orthogonal = 0;
      update_XWindow_image();
    }
  free_int_vector(display_slice_temp);
}

void action_quit()
{
  char quit_string[LEN_STR];

  if ( get_X11_string(2,"Really quit?","(y,Y,other)","",0,0,quit_string) )
    {
      if ( !strcmp(quit_string,"y") || !strcmp(quit_string,"Y") )
	{
	  action_quit_specific();
	  exit(0);
	}
    }
  show_on_info_line(2,0,"continuing...","","");
}

int get_X11_string(int image_or_graph, char *prompt, char *options, char *default_string,
		   int Input_allowed, int CR_required, char *return_string)
     // Input_allowed: 0 = any keypres, 1 = alpha, 2 = numeric, 3 = alphanumeric
{
  static char CNULL = '\0';
  XEvent event;
  char full_prompt[LEN_STR], attach[LEN_STR], key[LEN_STR];
  char which_key, *cptr;
  int length;
  BOOL Any_allowed, Alpha_allowed, Numeric_allowed, AlphaNumeric_allowed;
  BOOL Alpha, Numeric, AlphaNumeric, done=FALSE;
  
  Any_allowed          = (Input_allowed == 0);
  Alpha_allowed        = (Input_allowed == 1);
  Numeric_allowed      = (Input_allowed == 2);
  AlphaNumeric_allowed = (Input_allowed == 3);

  sprintf(return_string,"%s",default_string); // just initialize
  strcpy(full_prompt,prompt);
  sprintf(attach," >%s",return_string);
  strcat(full_prompt,attach);

  show_on_info_line(image_or_graph,1,full_prompt,options,"");
  do
    {
      which_key = 0;
      if ( get_next_event(&event) && event.type == KeyPress )
	{
	  if ( !get_key_press(&(event.xkey), &which_key) ) continue;

	  Alpha   = (which_key >= 65 && which_key <= 90) || (which_key >= 97 && which_key <= 122);
	  Numeric = (which_key >= 48 && which_key <= 57);
	  AlphaNumeric = Alpha || Numeric;

	  if ( which_key == 8 || which_key == 127 ) // backspace or delete
	    {
	      length = jip_int_count((JIP_INDEX)strlen(return_string));
	      if ( length > 1 )
		{
		  cptr = return_string;   // space
		  while ( *cptr ) cptr++; // find end of string
		  cptr--;                 // go back one step
		  *cptr = CNULL;
		}
	      strcpy(full_prompt,prompt);
	      sprintf(attach," >%s",return_string);
	      strcat(full_prompt,attach);
	      if ( image_or_graph == 0 || image_or_graph == 2 )
		XClearArea(XDisplay, XWindow_image, 0, mosaic.ImageRegionPixels.y+2*INFO_HEIGHT,
			   mosaic.ImageRegionPixels.x/2, 2*INFO_HEIGHT, False);
	      else if ( (image_or_graph == 1 || image_or_graph == 2) &&  XWindow_graph != 0 )
		XClearArea(XDisplay, XWindow_graph, 0, graph.nheight+INFO_HEIGHT,
			   graph.nwidth/2, INFO_HEIGHT, False);
	    }
	  else if ( Any_allowed ||
		    (Alpha&&Alpha_allowed) ||
		    (Numeric&&Numeric_allowed) ||
		    (AlphaNumeric&&AlphaNumeric_allowed) )
	    { // event must be number 1-9, a-z, A-Z, or carriage return
	      sprintf(key,"%c",which_key);
	      strcat(return_string,key);
	      strcpy(full_prompt,prompt);
	      sprintf(attach," >%s",return_string);
	      strcat(full_prompt,attach);
	      if ( !CR_required )
		done = TRUE;
	    }
	  show_on_info_line(image_or_graph,1,full_prompt,options,"");
	}
    }
  while
    ( (CR_required && (which_key != 13 && which_key != 27 )) || // terminate with return or escape
      (!CR_required && !done) );
  if ( which_key == 27 )
    return(0);
  else
    {
      if ( !strcmp(return_string," ") )
	strcpy(return_string,default_string);
      return(1);
    }
}

void action_key_question_mark()
{
  XEvent event;
  BOOL current_window;
  FILE *file;
  char *jip_home, exclamation;
  char file_name[LEN_STR], string[LEN_STR];
  BOOL found, done;
  int icursor, itext, ilength;

  reset_cursor_selections();
  show_on_info_line(0,1,"select a region with a mouse press","output to shell window","other keys cancel");
  do
    {
      if ( get_next_event(&event) == 1 )
	{
	  current_window = (event.xbutton.window == XWindow_image);
	  if ( current_window && event.type == ButtonRelease )
	    {
	      define_mosaic_cursor_location(event.xbutton.x, event.xbutton.y);
	      break;
	    }
	}
    }
  while
    ( event.type != KeyPress || !current_window );

  jip_home = getenv("JIP_HOME");
  if ( !jip_home )
    {
      printf("Define the environment variable JIP_HOME for more help.\n");
      return;
    }
  strcpy(file_name,jip_home);
  strcat(file_name,"/documentation/");
  strcat(file_name,program_name);
  strcat(file_name,"-question-mark.txt");

  // Open the help file.
  if ( !(file = fopen(file_name,"r")) )
    {
      printf("Error: cannot open the file %s.\n",file_name);
      printf("The environment variable JIP_HOME is set to %s\n",jip_home);
      return;
    }
  else
    printf("Help from file %s\n",file_name);

  printf("message code %d %d\n",mosaic.cursor_selection,mosaic.text_selection);
  found = done = FALSE;
  while ( !done && get_line_with_comment(file, string, LEN_STR) )
    {
      icursor = itext = 0;
      sscanf(string,"%c %d %d",&exclamation,&icursor,&itext);
     if ( exclamation=='!' && icursor == mosaic.cursor_selection && itext == mosaic.text_selection )
	found = TRUE;
      else
	{
	  if ( found )
	    {
	      ilength = jip_int_count((JIP_INDEX)strlen(string));
	      if ( ilength == 1 )
		{
		  done = TRUE;
		  printf("\n");
		}
	      else
		printf("%s",string);
	    }
	}
    }
  fclose(file);
  if ( !found )
    printf("Context-sensitive help text not found; add to file.\n");
  return;
}

void action_select_activated_point()
{
  XEvent event;
  BOOL current_window;

  reset_cursor_selections();
  show_on_info_line(0,1,"Select an activated voxel","Using a mouse button","(other keys cancel)");
  do
    {
      if ( get_next_event(&event) == 1 )
	{
	  current_window = (event.xbutton.window == XWindow_image);
	  if ( current_window && event.type == ButtonRelease )
	    {
	      define_mosaic_cursor_location(event.xbutton.x, event.xbutton.y);
	      break;
	    }
	}
    }
  while
    ( event.type != KeyPress || !current_window );

  if ( !(mosaic.cursor_selection&CURSOR_ON_DATA ) )
    {
      show_on_info_line(0,1,"Selection is not a valid voxel.","The function was canceled"," ");
      return;
    }

  // mosaic.icursor should now be located on the selected voxel.
  mosaic.icross = mosaic.icursor;
  // Expand the region and create an overlay:
  action_select_activated_region();

  return;
}

void action_image_draw_overlay( int press )
     // press:     1 = on, -1 = off (released), 0 = moving cursor with button pressed
{
  static IPOINT3D On, Off;
  POINT3D rOn, rOff;
  IPOINT3D voxel;
  IPOINT2D iDraw1, iDraw2;
  int which_orientation, which_image, draw_it;
  JIP_INDEX index;

  if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY )
    which_orientation = ORTHO_XY;
  else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ )
    which_orientation = ORTHO_YZ;
  else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY )
    which_orientation = ORTHO_ZY;
  else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ )
    which_orientation = ORTHO_XZ;

  if ( mosaic.cursor_selection&CURSOR_ON_MOSAIC )
    which_image = MOSAIC;
  else
    which_image = which_orientation;
       
  if ( press == 1 )
    { // button pressed
      On  = Off = mosaic.icursor;
      drawing_overlay = 1;
    }
  if ( press == 1 || drawing_overlay )
    //  else if ( drawing_overlay )
    {
      if ( which_orientation == ORTHO_XY )
	{
	  Off.z = mosaic.icursor.z;
	  Off.x = mosaic.icursor.x + 1;
	  Off.y = mosaic.icursor.y + 1;
	}
      else if ( which_orientation == ORTHO_YZ || which_orientation == ORTHO_ZY )
	{
	  Off.x = mosaic.icursor.x;
	  Off.y = mosaic.icursor.y + 1;
	  Off.z = mosaic.icursor.z + 1;
	}
      else if ( which_orientation == ORTHO_XZ )
	{
	  Off.y = mosaic.icursor.y;
	  Off.x = mosaic.icursor.x + 1;
	  Off.z = mosaic.icursor.z + 1;
	}
    }

  rOn.x = On.x;  rOff.x = Off.x;
  rOn.y = On.y;  rOff.y = Off.y;
  rOn.z = On.z;  rOff.z = Off.z;

  iDraw1  = Convert_rVoxel_to_Pixel(which_image,rOn,0);
  iDraw2  = Convert_rVoxel_to_Pixel(which_image,rOff,0);

  // This section draws a rectangle for visualization before the button is released.
  draw_it = (iDraw2.x >= iDraw1.x && iDraw2.y >= iDraw1.y);
  if ( which_orientation == ORTHO_XY )
    draw_it &= ( Off.z == On.z );
  else if ( which_orientation == ORTHO_YZ || which_orientation == ORTHO_ZY )
    draw_it &= ( Off.x == On.x );
  else if ( which_orientation == ORTHO_XZ )
    draw_it &= ( Off.y == On.y );
  if ( draw_it )
    {
      // Refresh the data region.
      refresh_display_image(1);
      DPRIMITIVE_DrawRectangle(Drawable_ImageData, COLOR_RED, iDraw1, iDraw2);
    }

  // The following section sets the actual overlay when the button is released.
  if ( press == -1 )
    {
      mosaic.update_regions |= UPDATE_IMAGES;
      if ( which_orientation == ORTHO_XY )
	{
	  voxel.z = On.z;
	  for (voxel.x = MIN(On.x,Off.x); voxel.x < MAX(On.x,Off.x); voxel.x++)
	    {
	      for (voxel.y = MIN(On.y,Off.y); voxel.y < MAX(On.y,Off.y); voxel.y++)
		{
		  index = voxel3D_to_1D(voxel);
		  if ( overlay_map.weight[index] == 0. )
		    {
		      overlay_map.weight[index] = 1.;
		      overlay_map.color[index]  = ovl_root.color;
		    }
		  else
		    {
		      overlay_map.weight[index] = 0.;
		      overlay_map.color[index]  = 0;
		    }
		}
	    }
	}
      else if ( which_orientation == ORTHO_YZ || which_orientation == ORTHO_ZY )
	{
	  voxel.x = On.x;
	  for (voxel.z = MIN(On.z,Off.z); voxel.z < MAX(On.z,Off.z); voxel.z++)
	    {
	      for (voxel.y = MIN(On.y,Off.y); voxel.y < MAX(On.y,Off.y); voxel.y++)
		{
		  index = voxel3D_to_1D(voxel);
		  if ( overlay_map.weight[index] == 0. )
		    {
		      overlay_map.weight[index] = 1.;
		      overlay_map.color[index]  = ovl_root.color;
		    }
		  else
		    {
		      overlay_map.weight[index] = 0.;
		      overlay_map.color[index]  = 0;
		    }
		}
	    }
	}
      else if ( which_orientation == ORTHO_XZ )
	{
	  voxel.y = On.y;
	  for (voxel.z = MIN(On.z,Off.z); voxel.z < MAX(On.z,Off.z); voxel.z++)
	    {
	      for (voxel.x = MIN(On.x,Off.x); voxel.x < MAX(On.x,Off.x); voxel.x++)
		{
		  index = voxel3D_to_1D(voxel);
		  if ( overlay_map.weight[index] == 0. )
		    {
		      overlay_map.weight[index] = 1.;
		      overlay_map.color[index]  = ovl_root.color;
		    }
		  else
		    {
		      overlay_map.weight[index] = 0.;
		      overlay_map.color[index]  = 0;
		    }
		}
	    }
	}
      drawing_overlay = 0;
      update_root_overlay(1,1);
      if ( graph.type == 3 )
	graph.update_regions |= UPDATE_GRAPH;
      //      update_graph_data(1);
    }
}

void action_draw_wires()
{
  XEvent event;
  CONTROL_POINT *wire_save=NULL, *wire_original=NULL;
  POINT3D space;
  char prompt[LEN_STR], string[LEN_STR];
  char which_key;
  int xbut, ybut, iZ_save;JIP_INDEX j;JIP_INDEX itest;JIP_INDEX npts_original;
  BOOL undo_available, close_available, fill_overlay, current_window;

  iZ_save = -1;
  npts_original = wire_root.npts;
  wire_original = control_vector(npts_original);
  for (j=0; j<npts_original; j++)
    wire_original[j] = wire_root.control[j];
  sprintf(prompt," L-button=draw, <esc>=cancel");
  show_on_info_line(0,1,prompt,"","");
  //
  undo_available = close_available = fill_overlay = FALSE;
  do
    {
      which_key = 0;
      if ( get_next_event(&event) == 1 )
	{
	  current_window = (event.xbutton.window == XWindow_image);
	  mosaic.cursor_selection = CURSOR_ON_WINDOW;
	  xbut = event.xbutton.x;
	  ybut = event.xbutton.y;
	  if ( current_window && ( event.type == KeyPress      || event.type == ButtonPress ||
				   event.type == ButtonRelease || event.type == MotionNotify ||
				   event.type == LeaveNotify ) )
	    define_mosaic_cursor_location(xbut, ybut);
	  switch (event.type)
	    {
	    case MotionNotify: 
	      space = mosaic.cursor;
	      sprintf(string,"%6.2f %6.2f %6.2f",space.x,space.y,space.z);
	      show_on_info_line(0,1,prompt,"     x      y      z",string);
	      break;
	    case KeyPress:
	      if ( !get_key_press(&(event.xkey), &which_key) ) continue;
	      if ( which_key == 13 && !undo_available )
		which_key = 0;
	      switch (which_key)
		{
		case 'u':
		  // undo the last arc
		  if ( undo_available )
		    {
		      free_control_vector(wire_root.control);
		      wire_root.control = control_vector(wire_root.npts-3);
		      for (j=0; j<wire_root.npts-3; j++)
			wire_root.control[j] = wire_save[j];
		      if ( wire_save ) free_control_vector(wire_save);
		      wire_root.npts -= 3;
		      if ( wire_root.npts > 0 )
			{
			  wire_save = control_vector(wire_root.npts);
			  for (j=0; j<wire_root.npts; j++)
			    wire_save[j] = wire_root.control[j];
			}
		      refresh_display_image(1);
		      draw_wire_frames();
		    }
		  break;
		case 'c':
		  // close the arc to the 1st point
		  if ( close_available )
		    {
		      wire_root.npts++;
		      // Update the wire array, and then save the result in wire_save
		      if ( wire_root.npts>2 ) free_control_vector(wire_root.control);
		      wire_root.control = control_vector(wire_root.npts);
		      for (j=0; j<wire_root.npts-1; j++)
			wire_root.control[j] = wire_save[j];
		      wire_root.control[wire_root.npts-1] = wire_root.control[npts_original];
		      free_control_vector(wire_save);
		      wire_save = control_vector(wire_root.npts);
		      for (j=0; j<wire_root.npts; j++)
			wire_save[j] = wire_root.control[j];
		      //
		      mosaic.update_regions |= UPDATE_IMAGES;
		      Update_Regions();
		      which_key = 13;
		      fill_overlay = TRUE;
		    }
		  break;
		default:
		  break;
		}
	      break;
	    case ButtonPress:
	      which_key = 0;
	      switch (event.xbutton.button)
		{	
		case 1: 
		  if ( wire_root.npts != npts_original )
		    {
		      if ( mosaic.icursor.z == iZ_save )
			{ // Don't allow drawing across slice boundaries
			  wire_root.npts++;
			  // Update the wire array.
			  if ( wire_root.control ) free_control_vector(wire_root.control);
			  wire_root.control = control_vector(wire_root.npts);
 			  for (j=0; j<wire_root.npts-1; j++)
			    wire_root.control[j] = wire_save[j];
			  wire_root.control[wire_root.npts-1].point = mosaic.cursor;
			  wire_root.control[wire_root.npts-1].color = 1;
			  // Make a copy of the array.
			  if ( wire_save ) free_control_vector(wire_save);
			  wire_save = control_vector(wire_root.npts);
 			  for (j=0; j<wire_root.npts; j++)
			    wire_save[j] = wire_root.control[j];
			  // Update the image
			  Update_Regions();
			}
		    }
		  else
		    {
		      iZ_save = mosaic.icursor.z;
		      wire_root.npts = npts_original+1;
		      // Save all points in the pre-existing wire array,
		      // plus the 1st point in the one currently being drawn
		      wire_save = control_vector(wire_root.npts);
		      if ( npts_original != 0 )
			{
			  for (j=0; j<npts_original; j++)
			    wire_save[j] = wire_root.control[j];
			}
		      wire_save[npts_original].point = mosaic.cursor;
		      wire_save[npts_original].color = 1;
		      // Update the root array
		      if ( wire_root.control ) free_control_vector(wire_root.control);
		      wire_root.control = control_vector(wire_root.npts);
		      for (j=0; j<wire_root.npts; j++)
			wire_root.control[j] = wire_save[j];
		      Update_Regions();
		    }
		  break; 
		default:
		  continue;
		}
	    default:
	      break;
	    }
	}
      if ( event.type == KeyPress || event.type == ButtonPress )
	{
	  itest = wire_root.npts - npts_original - 1;
	  undo_available  = (itest != 0 && itest%3 == 0);
	  close_available = (itest != 2 && itest%3 == 2);
	  if ( undo_available )
	    sprintf(prompt," L-button=draw, <esc>=cancel, <ret>=done, u=undo");
	  else if ( close_available )
	    sprintf(prompt," L-button=draw, <esc>=cancel, c=close");
	  else
	    sprintf(prompt," L-button=draw, <esc>=cancel");
	  show_on_info_line(0,1,prompt,"","");
	}
    }
  while
    ( which_key != 27 && which_key != 13 ); // return and escape

  if ( which_key == 13 ) // return
    {
      wire_root.control[wire_root.npts-1].color = 0;
      sprintf(wire_root.name,"interactive");
    }
  else if ( which_key == 27 ) // escape
    {
      if ( wire_root.control ) free_control_vector(wire_root.control);
      wire_root.npts = npts_original;
      wire_root.control = control_vector(wire_root.npts);
      for (j=0; j<npts_original; j++)
	wire_root.control[j] = wire_original[j];
      sprintf(prompt,"canceled");
      show_on_info_line(0,1,prompt,"","");
    }

  if ( wire_save ) free_control_vector(wire_save);
  free_control_vector(wire_original);
  // If the wire was closed, fill all voxels inside as an overlay.
  if ( fill_overlay )
    {
      fill_last_closed_wire(wire_root.npts-1);
      mosaic.update_regions |= UPDATE_IMAGES;
    }
}

void refresh_display_image( int which_area )
// which_area: 0 = ALL
//             1 = data only
//             2 = colorbar only
//             3 = text fields only
//             4 = right panel
{
  IPOINT2D iPut;

  if ( which_area == 0 || which_area == 1 )
    {
      // Refresh the data region.
      iPut.x = 0;
      iPut.y = 2*INFO_HEIGHT;
      XCopyArea( XDisplay, XPixmap_image, XWindow_image, XGC_image,
		 0, 0, mosaic.ImageRegionPixels.x, mosaic.ImageRegionPixels.y,
		 iPut.x, iPut.y);
      // Draw crosshair on top of window after pasting pixmap (above).
      if ( mosaic.display_crosshairs >= 2 && (graph.type!=0 && graph.type!=1 && graph.type!=2) )
	draw_crosshairs(mosaic.display_crosshairs-2);
      if ( mosaic.display_grid != 0 )
	{
	  if ( mosaic.display_3orthogonal )
	    label_planes_ortho();
	  else
	    label_planes_mosaic();
	}
    }
  if ( which_area == 0 || which_area == 2 )
    {
      // Refresh the colorbar region.
      iPut.x = mosaic.ImageRegionPixels.x;
      iPut.y = 2*INFO_HEIGHT;
      XCopyArea( XDisplay, XPixmap_image, XWindow_image, XGC_image,
		 mosaic.ImageRegionPixels.x, 0,
		 mosaic.nx_ColorBar, mosaic.ImageRegionPixels.y,
		 iPut.x, iPut.y);
      // Draw labels on top of window after pasting colorbar (above).
      label_colorbar();
    }
  if ( which_area == 0 || which_area == 3 )
    {
      // Draw text on top of Pixmaps.
      draw_selectable_text_image();
      // Refresh the top menu.
      XCopyArea( XDisplay, XPixmap_TopMenu, XWindow_image, XGC_image,
		 0, 0, XImage_TopMenu->width, XImage_TopMenu->height, 0, 0);
      // Refresh the bottom menu.
      iPut.x = lXBottomMenu;
      iPut.y = mosaic.ImageRegionPixels.y + 2*INFO_HEIGHT;
      XCopyArea( XDisplay, XPixmap_BottomMenu, XWindow_image, XGC_image,
		 0, 0, XImage_BottomMenu->width, XImage_BottomMenu->height, iPut.x, iPut.y);
    }
  if ( which_area == 0 || which_area == 4 )
    {
      if ( mosaic.nx_RightPanel != 0 )
	{
	  update_right_panel();
	  // Refresh the right panel
	  iPut.x = mosaic.ImageRegionPixels.x+mosaic.nx_ColorBar;
	  iPut.y = 2*INFO_HEIGHT;
	  XCopyArea( XDisplay, XPixmap_RightPanel, XWindow_image, XGC_image,
		     0, 0, XImage_RightPanel->width, XImage_RightPanel->height, iPut.x, iPut.y);
	}
    }
}

void refresh_display_graph()
{
  IPOINT2D iPut;

  // Refresh the graph data region.
  iPut.x = 0;
  iPut.y = INFO_HEIGHT;
  XCopyArea( XDisplay, XPixmap_GraphData, XWindow_graph, XGC_image,
  	     0, 0, graph.nwidth, graph.nheight,iPut.x,iPut.y);
}

void show_on_info_line(int image_or_graph, int PromptForInput,
		       char *string1, char *string2, char *string3)
{ // image_or_graph: 0 = image, 1 = graph, 2 = both
  IPOINT2D iPut;
  BOOL ImageAndGraph, ImageOnly;
  int which_drawable;

  ImageOnly = (image_or_graph == 0 ) || (XWindow_graph == 0);
  ImageAndGraph = (image_or_graph == 2 ) && XWindow_graph != 0;
  if ( image_or_graph == 0 )
    which_drawable = Drawable_ImageInfoArea;
  else if ( image_or_graph == 1 )
    which_drawable = Drawable_GraphInfoArea;
  else
    which_drawable = -1;  // force error

  iPut.x = 0;
  if ( image_or_graph == 0 || ImageAndGraph || ImageOnly )
    {
      // Clear the info area
      XPutImage( XDisplay, XPixmap_ImageInfoArea, XGC_image, XImage_ImageInfoArea, 0,0,0,0,
      		 XImage_ImageInfoArea->width,XImage_ImageInfoArea->height );
      iPut.y = INFO_HEIGHT-TEXTOFF;
    }
  if ( image_or_graph == 1 || ImageAndGraph )
    {
      XPutImage( XDisplay, XPixmap_GraphInfoArea, XGC_image, XImage_GraphInfoArea, 0,0,0,0,
		 XImage_GraphInfoArea->width,XImage_GraphInfoArea->height );
      //      XClearArea(XDisplay, XWindow_graph, 0, graph.nheight+2*INFO_HEIGHT,
      //		 graph.nwidth/2, INFO_HEIGHT, True);
      iPut.y = INFO_HEIGHT-TEXTOFF;
    }
      
  if ( PromptForInput )
    {
      if ( ImageAndGraph || ImageOnly )
	{
	  // Image
	  iPut.y = INFO_HEIGHT-TEXTOFF;
	  DPRIMITIVE_DrawText(Drawable_ImageInfoArea, COLOR_RED, iPut, string1);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_DrawText(Drawable_ImageInfoArea, COLOR_BLACK, iPut, string2);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_DrawText(Drawable_ImageInfoArea, COLOR_BLACK, iPut, string3);
	  if ( !ImageOnly )
	    {
	      // Graph
	      iPut.y = INFO_HEIGHT-TEXTOFF;
	      DPRIMITIVE_FillText(Drawable_GraphInfoArea, COLOR_RED, iPut, string1);
	      iPut.y += INFO_HEIGHT;
	      DPRIMITIVE_FillText(Drawable_GraphInfoArea, COLOR_WHITE, iPut, string2);
	    }
	}
      else
	{
	  // Image or graph (not both)
	  DPRIMITIVE_FillText(which_drawable, COLOR_RED, iPut, string1);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_FillText(which_drawable, COLOR_WHITE, iPut, string2);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_FillText(which_drawable, COLOR_WHITE, iPut, string3);
	}
    }
  else
    {
      if ( ImageAndGraph || ImageOnly )
	{
	  // Image
	  iPut.y = INFO_HEIGHT-TEXTOFF;
	  DPRIMITIVE_DrawText(Drawable_ImageInfoArea, COLOR_BLACK, iPut, string1);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_DrawText(Drawable_ImageInfoArea, COLOR_BLACK, iPut, string2);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_DrawText(Drawable_ImageInfoArea, COLOR_BLACK, iPut, string3);
	  if ( !ImageOnly )
	    {
	      // Graph
	      iPut.y = INFO_HEIGHT-TEXTOFF;
	      DPRIMITIVE_FillText(Drawable_GraphInfoArea, COLOR_WHITE, iPut, string1);
	      iPut.y += INFO_HEIGHT;
	      DPRIMITIVE_FillText(Drawable_GraphInfoArea, COLOR_WHITE, iPut, string2);
	    }
	}
      else
	{
	  // Image or graph (not both)
	  DPRIMITIVE_FillText(which_drawable, COLOR_WHITE, iPut, string1);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_FillText(which_drawable, COLOR_WHITE, iPut, string2);
	  iPut.y += INFO_HEIGHT;
	  DPRIMITIVE_FillText(which_drawable, COLOR_WHITE, iPut, string3);
	}
    }
  if ( ImageAndGraph || ImageOnly )
    {
      iPut.x = 0;
      iPut.y = mosaic.ImageRegionPixels.y + 2*INFO_HEIGHT;
      XCopyArea( XDisplay, XPixmap_ImageInfoArea, XWindow_image, XGC_image,
      		 0, 0, XImage_ImageInfoArea->width, XImage_ImageInfoArea->height, iPut.x, iPut.y);
    }
  if ( ImageAndGraph || image_or_graph == 1 )
    {
      iPut.x = 0;
      iPut.y = graph.nheight + INFO_HEIGHT;
      XCopyArea( XDisplay, XPixmap_GraphInfoArea, XWindow_graph, XGC_image,
		 0, 0, XImage_GraphInfoArea->width, XImage_GraphInfoArea->height, iPut.x, iPut.y);
    }
  XFlush(XDisplay);
}

void draw_selectable_text_image()
{
  IPOINT2D iPut;
  struct SelectableText *SelectText;
  int which_drawable;

  update_image_selectable_text();

  // Clear the top lines
  XPutImage( XDisplay, XPixmap_TopMenu, XGC_image, XImage_TopMenu, 0,0,0,0,
	     XImage_TopMenu->width,XImage_TopMenu->height );
  // Clear the bottom lines
  XPutImage( XDisplay, XPixmap_BottomMenu, XGC_image, XImage_BottomMenu, 0,0,0,0,
	     XImage_BottomMenu->width,XImage_BottomMenu->height );

  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->visible )
	{
	  iPut.x = SelectText->XPosition;
	  // The y lines are tagged by numbers, not by pixel locations
	  if ( SelectText->YLocation == 0 )
	    {
	      iPut.y = INFO_HEIGHT - TEXTOFF;
	      which_drawable = Drawable_TopMenu;
	    }
	  else if ( SelectText->YLocation == 1 )
	    {
	      iPut.y = 2*INFO_HEIGHT - TEXTOFF;
	      which_drawable = Drawable_TopMenu;
	    }
	  else if ( SelectText->YLocation == 2 )
	    {
	      iPut.y = INFO_HEIGHT - TEXTOFF;
	      which_drawable = Drawable_BottomMenu;
	    }
	  else if ( SelectText->YLocation == 3 )
	    {
	      iPut.y = 2*INFO_HEIGHT - TEXTOFF;
	      which_drawable = Drawable_BottomMenu;
	    }
	  else if ( SelectText->YLocation == 4 )
	    {
	      iPut.y = 3*INFO_HEIGHT - TEXTOFF;
	      which_drawable = Drawable_BottomMenu;
	    }
	  if (SelectText->selected)
	    DPRIMITIVE_FillText(which_drawable, COLOR_RED,         iPut, SelectText->string);
	  else
	    DPRIMITIVE_FillText(which_drawable, SelectText->color, iPut, SelectText->string);
	}
    }
}

void draw_selectable_text_graph()
{
  IPOINT2D iPut, iDraw1, iDraw2;
  struct SelectableText *SelectText;

  // Prevent errors arising from key events that call this function before a graph has been created.
  if ( !XWindow_graph ) return;

  update_graph_selectable_text();

  // Clear the top line
  iDraw1.x = iDraw1.y = 0;
  iDraw2.x = graph.nwidth;
  iDraw2.y = INFO_HEIGHT;
  DPRIMITIVE_FillRectangle(Drawable_GraphWindow, COLOR_CYAN, iDraw1, iDraw2);

  // Clear the bottom line
  iDraw1.x = XImage_GraphInfoArea->width;
  iDraw2.x = graph.nwidth;
  iDraw1.y = graph.nheight+INFO_HEIGHT;
  iDraw2.y = graph.nheight+4*INFO_HEIGHT;
  DPRIMITIVE_FillRectangle(Drawable_GraphWindow, COLOR_CYAN, iDraw1, iDraw2);

  for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->visible )
	{
	  iPut.x = SelectText->XPosition;
	  if ( SelectText->YLocation == 1 )
	    iPut.y = INFO_HEIGHT-TEXTOFF;
	  else if ( SelectText->YLocation == 2 )
	    iPut.y = graph.nheight+2*INFO_HEIGHT-TEXTOFF;
	  else if ( SelectText->YLocation == 3 )
	    iPut.y = graph.nheight+3*INFO_HEIGHT-TEXTOFF;
	  else if ( SelectText->YLocation == 4 )
	    iPut.y = graph.nheight+4*INFO_HEIGHT-TEXTOFF;
	  if (SelectText->selected)
	    DPRIMITIVE_FillText(Drawable_GraphWindow, COLOR_RED,         iPut, SelectText->string);
	  else
	    DPRIMITIVE_FillText(Drawable_GraphWindow, SelectText->color, iPut, SelectText->string);
	}
    }
}

void build_graph_display_XYZT( BOOL UpdateData )
{
  POINT2D Point1, Point2;
  static POINT2D Scale;
  IPOINT2D iDraw1, iDraw2;
  int j, jx, jy, lStatus;int nbins;
  float *BinXVector, *BinYVector;
  float range, xscale;
  unsigned long black = 0;
  BOOL first_time = TRUE, NonZeroFit;

  // Prevent errors arising from key events that call this function before a graph has been created.
  if ( !XWindow_graph ) return;

  if ( UpdateData )
    {
      if ( !first_time )
	{
	  XFreePixmap(XDisplay,XPixmap_GraphData);
	  XDestroyImage(XImage_GraphData);      // frees image AND data
	}
      first_time = FALSE;
      XData_GraphData  = (char *)malloc((size_t)jip_product(jip_product(graph.nwidth,graph.nheight),XDisplay_depth));
      XImage_GraphData = XCreateImage(XDisplay, XVisual, XDisplay_depth, ZPixmap, 0, XData_GraphData,jip_int_count(graph.nwidth),jip_int_count(graph.nheight), 8, 0);
      lStatus = XInitImage(XImage_GraphData);
      // Initialize graph data
      for (jx=0; jx<XImage_GraphData->width; jx++)
	{
	  for (jy=0; jy<XImage_GraphData->height; jy++)
	    XPutPixel(XImage_GraphData,jx,jy,black);
	}
      // Create a pixel map for the graph data.
      XPixmap_GraphData = XCreatePixmap(XDisplay, XDisplay_Window, graph.nwidth, graph.nheight,XDisplay_depth);
      // Start with the background image behind the graph data.
      XPutImage( XDisplay, XPixmap_GraphData, XGC_image, XImage_GraphData, 0,0,0,0,
		 XImage_GraphData->width,XImage_GraphData->height );

      if ( graph.type==0 || graph.type==1 || graph.type==2 )
	plot_graph_projection_secondary_data();

      // Set the x min and max
      graph.xmin = graph.xVector[0];
      graph.xmax = graph.xVector[graph.npoints-1];

      // Re-scale the graph unless the scale has been fixed.
      if ( graph.ymin == graph.ymax || !graph.yrange_fixed )
	{
	  // Find the min and max for the current time vector.
	  graph.ymin =  1.e10;
	  graph.ymax = -1.e10;
	  for (j=0; j<graph.npoints; j++)
	    {
	      if ( graph.yVector[j] < graph.ymin ) graph.ymin = graph.yVector[j];
	      if ( graph.yVector[j] > graph.ymax ) graph.ymax = graph.yVector[j];
	    }
	  // If units of percent are being used, round to an integer value.
	  if ( graph.type == 3 && graph.units_percent)
	    {
	      graph.ymin = (int)(graph.ymin-1.);
	      graph.ymax = (int)(graph.ymax+1.);
	    }
	  else
	    {
	      range = graph.ymax - graph.ymin;
	      graph.ymin -= range/10.;
	      graph.ymax += range/10.;
	    }
	}

      // Define scales
      Scale.x = (float)(graph.nwidth) / (float)(graph.npoints);
      Scale.y = (float)(graph.nheight) / (graph.ymax-graph.ymin);

      if ( graph.type == 3 && graph.time_model != 0 )
	plot_TimeSeriesModel( &Scale );

      // Smooth the data if necessary.
      if ( graph.type == 3 )
	{ // Only smooth time graphs.
	  if ( graph.bin_data )
	    {
	      nbins = graph.npoints/graph.bin_width;
	      if ( graph.npoints % graph.bin_width )
		nbins++;
	      BinXVector = float_vector(nbins);
	      BinYVector = float_vector(nbins);
	      bin_graph_vector(nbins,graph.yVector,BinXVector,BinYVector);
	    }
	  else
	    {
	      nbins = graph.npoints;
	      BinXVector = float_vector(nbins);
	      BinYVector = float_vector(nbins);
	      for (j=0; j<nbins; j++)
		BinXVector[j] = j;
	      smooth_graph_vector(graph.yVector,BinYVector);
	    }
	}
      else
	{ // For projections, etc., just copy the data.
	  nbins = graph.npoints;
	  BinXVector = float_vector(nbins);
	  BinYVector = float_vector(nbins);
	  for (j=0; j<nbins; j++)
	    {
	      BinXVector[j] = j;
	      BinYVector[j] = graph.yVector[j];
	    }
	}

      // For histograms, shade the region corresponding to the image settings
      if ( graph.type == 4 )
	{
	  xscale = (float)(graph.npoints-1) / (graph.xmax-graph.xmin);
	  Point1.x = (image_scale_min-graph.xmin) * xscale;
	  Point2.x = (image_scale_max-graph.xmin) * xscale;
	  Point1.y = graph.ymax;
	  Point2.y = graph.ymin;
	  iDraw1 = Space2D_to_GraphPixels(Point1, Scale);
	  iDraw2 = Space2D_to_GraphPixels(Point2, Scale);
	  DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_GRAY, iDraw1, iDraw2);
	}

      // Display the data.
      if ( graph.type != 3 )
	plot_DataPoints( nbins, BinXVector, BinYVector, &Scale );
      else
	plot_TimeSeriesModel_DataPoints( nbins, BinXVector, BinYVector, &Scale );

      // For projections, display the fit if value are non-zero.
      if ( graph.type < 3 )
	{
	  for (j=0, NonZeroFit=FALSE; j<graph.npoints; j++)
	    NonZeroFit |= graph.yFit[j] != 0.;
	  if ( NonZeroFit )
	    {
	      Point1.x = BinXVector[0];  Point1.y = graph.yFit[0];
	      iDraw1 = Space2D_to_GraphPixels(Point1, Scale);
	      for (j=1; j<graph.npoints; j++)
		{
		  Point2.x = BinXVector[j];  Point2.y = graph.yFit[j];
		  iDraw2 = Space2D_to_GraphPixels(Point2, Scale);
		  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_RED, iDraw1, iDraw2);
		  iDraw1.x = iDraw2.x;  iDraw1.y = iDraw2.y;
		}
	    }
	}
      free_float_vector(BinXVector);
      free_float_vector(BinYVector);

      // Put a zero line on the graph when this is reasonable.
      if ( graph.ymin < 0 || graph.ymax > 0 )
	{
	  iDraw1.x = 0;
	  iDraw2.x = graph.nwidth;
	  iDraw1.y = iDraw2.y = graph_y( Scale.y, 0. );
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_YELLOW, iDraw1, iDraw2);
	}
    }

  // update the graph window with the pixel map
  XCopyArea( XDisplay, XPixmap_GraphData, XWindow_graph, XGC_image,
	     0, 0, XImage_GraphData->width, XImage_GraphData->height, 0, INFO_HEIGHT);

  // Mark the current graph point with a vertical line.
  if ( graph.type == 0 )
    Point1.x = Point2.x = mosaic.icross.x;
  else if ( graph.type == 1 )
    Point1.x = Point2.x = mosaic.icross.y;
  else if ( graph.type == 2 )
    Point1.x = Point2.x = mosaic.icross.z;
  else if ( graph.type == 3 )
    Point1.x = Point2.x = graph.iTime;
  if ( ! (graph.type == 3 && graph.iTime < 0 ) && (graph.type!=4) )
    {
      Point1.y = graph.ymax;
      Point2.y = graph.ymin;
      iDraw1 = Space2D_to_GraphPixels(Point1, Scale);
      iDraw2 = Space2D_to_GraphPixels(Point2, Scale);
      iDraw1.y += INFO_HEIGHT + 1;
      iDraw2.y += INFO_HEIGHT - 1;
      DPRIMITIVE_DrawLine(Drawable_GraphWindow, COLOR_WHITE, iDraw1, iDraw2);
    }
}

void plot_DataPoints( int nbins, float *BinXVector, float *BinYVector, POINT2D *Scale )
{
  POINT2D Point1, Point2;
  IPOINT2D iDraw1, iDraw2, iFill1, iFill2;
  float psize;
  int j;

  // Define the point size.
  psize = 1.5 * (float)(graph.nwidth) / (float)(nbins);
  if ( psize > 4. ) psize = 4.;
  //
  Point1.x = BinXVector[0];  Point1.y = BinYVector[0];
  iDraw1 = Space2D_to_GraphPixels(Point1, *Scale);
  iFill1.x = iDraw1.x - psize/2;  iFill1.y = iDraw1.y - psize/2;
  iFill2.x = iDraw1.x + psize/2;  iFill2.y = iDraw1.y + psize/2;
  // Draw the data on the graph.
  DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_YELLOW, iFill1, iFill2);
  for (j=1; j<nbins; j++)
    {
      Point2.x = BinXVector[j];  Point2.y = BinYVector[j];
      iDraw2 = Space2D_to_GraphPixels(Point2, *Scale);
      iFill1.x = iDraw2.x - psize/2;  iFill1.y = iDraw2.y - psize/2;
      iFill2.x = iDraw2.x + psize/2;  iFill2.y = iDraw2.y + psize/2;
      DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_YELLOW, iFill1, iFill2);
      DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_YELLOW, iDraw1, iDraw2);
      iDraw1.x = iDraw2.x;  iDraw1.y = iDraw2.y;
    }
}

void plot_graph_projection_secondary_data()
{
  POINT2D Point1, Point2, Scale;
  IPOINT2D iDraw1, iDraw2;
  int j;
  float gmin, gmax;

  // Define scales
  Scale.x = (float)(graph.nwidth) / (float)(graph.npoints);
  Scale.y = (float)(graph.nheight) / (graph.ymax-graph.ymin);

  // Put a gray selection window over the defined range.
  if ( graph.ix_low_select >= 0 )
    {
      Point1.x = (float)graph.ix_low_select;   Point1.y = graph.ymax;
      Point2.x = (float)graph.ix_high_select;  Point2.y = graph.ymin;
      iDraw1 = Space2D_to_GraphPixels(Point1, Scale);
      iDraw2 = Space2D_to_GraphPixels(Point2, Scale);
      DPRIMITIVE_FillRectangle(Drawable_GraphData, COLOR_GRAY, iDraw1, iDraw2);
    }
  // Put secondary data on the projection graph.
  if ( mosaic.activation.visible && mosaic.activation.fSpaceMatrix != NULL )
    {
      gmin = graph.ymin;
      gmax = graph.ymax;
      // Find the min and max for the current time vector.
      graph.ymin =  1.e10;
      graph.ymax = -1.e10;
      for (j=0; j<graph.npoints; j++)
	{
	  if ( graph.yVector1[j] < graph.ymin ) graph.ymin = graph.yVector1[j];
	  if ( graph.yVector1[j] > graph.ymax ) graph.ymax = graph.yVector1[j];
	}
      // Define scales
      Scale.x = (float)(graph.nwidth) / (float)(graph.npoints);
      Scale.y = (float)(graph.nheight) / (graph.ymax-graph.ymin);
      // Display the data.
      Point1.x = 0.;  Point1.y = graph.yVector1[0];
      iDraw1 = Space2D_to_GraphPixels(Point1, Scale);
      // Draw the data on the graph.
      for (j=1; j<graph.npoints; j++)
	{
	  Point2.x = j;  Point2.y = graph.yVector1[j];
	  iDraw2 = Space2D_to_GraphPixels(Point2, Scale);
	  DPRIMITIVE_DrawLine(Drawable_GraphData, COLOR_RED, iDraw1, iDraw2);
	  iDraw1.x = iDraw2.x;  iDraw1.y = iDraw2.y;
	}
      // restore graph min & max values
      graph.ymin = gmin;
      graph.ymax = gmax;
    }
}

void show_information_graph( int move_cursor )
{
  // Prevent errors arising from key events that call this function before a graph has been created.
  if ( !XWindow_graph ) return;

  show_position_graph(move_cursor);
}

void show_position_graph( int move_cursor )
{
  struct SelectableText *SelectText;
  POINT2D Point;
  char string1[LEN_STR], string2[LEN_STR], string3[LEN_STR];
  int ix;

  // Prevent errors arising from key events that call this function before a graph has been created.
  if ( !XWindow_graph ) return;

  if ( graph.cursor_selection&CURSOR_ON_SELECT_TEXT )
    {
      for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
	{
	  if ( SelectText->selected )
	    {
	      if ( SelectText->visible )
		{
		  strcpy(string1,SelectText->message1);
		  strcpy(string2,SelectText->message2);
		  strcpy(string3,SelectText->message3);
		  show_on_info_line(1, 0, string1, string2, string3);
		}
	    }
	}
    }
  else
    {
      if ( graph.type == 0 )
	{
	  ix = mosaic.icross.x;
	  sprintf(string1,"    x      graph");
	}
      else if ( graph.type == 1 )
	{
	  ix = mosaic.icross.y;
	  sprintf(string1,"    y      graph");
	}
      else if ( graph.type == 2 )
	{
	  ix = mosaic.icross.z;
	  sprintf(string1,"    z      graph");
	}
      else if ( graph.type == 3 )
	{
	  ix = graph.iTime;
	  sprintf(string1,"    time   graph");
	}
      else if ( graph.type == 4 )
	{
	  sprintf(string1,"    bin    graph");
	  ix = graph.icursor.x;
	}
      if ( graph.cursor_selection&CURSOR_ON_DATA && move_cursor )
	{
	  ix = graph.icursor.x;
	  Point.x = graph.cursor.x;
	  Point.y = graph.cursor.y;
	}
      else if ( ix >= 0 )
	{
	  Point.x = graph.xVector[ix];
	  Point.y = graph.yVector[ix];
	}
      if ( ix >= 0 )
	{
	  sprintf(string2,"%8g  %8g",Point.x,Point.y);
	  sprintf(string3,"%8d",ix);
	}
      else
	{
	  sprintf(string2,"                 ");
	  sprintf(string3,"%8d",ix);
	}
      // If a time model is being used, override the strings here.
      if ( graph.type ==3 && graph.time_model != 0 )
      	get_info_TimeSeriesModel( move_cursor, string1, string2, string3 );
      if ( move_cursor && ! (graph.cursor_selection&CURSOR_ON_DATA) )
	return;
      else
	show_on_info_line(1, 0, string1, string2, string3);
    }
}

void create_colorbar()
{ // xdisplay only
  float range, yvalue;
  int jx, jy;
  unsigned long gray_pixel, pixel;

  if ( !mosaic.Attach_ColorBar ) return;

  if ( !(mosaic.activation.visible) )
    {
      for (jy=0; jy<mosaic.ImageRegionPixels.y; jy++)
	{
	  range  = image_scale_max - image_scale_min;
	  yvalue = image_scale_max - range * (float)jy/(float)(mosaic.ImageRegionPixels.y);
	  gray_pixel = yvalue * image_scale_slope + image_scale_offset;
	  if ( gray_pixel > WHITE_PIXEL ) gray_pixel = WHITE_PIXEL;
	  if ( gray_pixel < BLACK_PIXEL ) gray_pixel = BLACK_PIXEL;
	  // In a 24-bit display, 8 bits are assigned to red, green, and blue.
	  pixel = (gray_pixel<<16) + (gray_pixel<<8) + gray_pixel;
	  for (jx=0; jx<mosaic.nx_ColorBar; jx++)
	    XPutPixel(XImage_colorbar,jx,jy,pixel);
	}
    }
  else
    {
      // Update the color scheme (set act_scale_slope & act_scale_offset)
      update_color_scheme();
      // Scheme 0
      if ( mosaic.activation.color_scheme == 0 ) // + & - tails
	{
	  // Positive values on RHS of color bar
	  for (jy=0; jy<mosaic.ImageRegionPixels.y; jy++)
	    {
	      range = get_activation_threshold(1) - get_activation_threshold(-1);
	      yvalue = get_activation_threshold(1) - range * (float)jy/(float)(mosaic.ImageRegionPixels.y);
	      pixel = activation_color_scheme0( yvalue, WHITE_PIXEL );
	      for (jx=mosaic.nx_ColorBar/2; jx<mosaic.nx_ColorBar; jx++)
		XPutPixel(XImage_colorbar,jx,jy,pixel);
	    }
	  for (jy=0; jy<mosaic.ImageRegionPixels.y; jy++)
	    {
	      // Negative values on LHS of color bar.
	      range = get_activation_threshold(1) - get_activation_threshold(-1);
	      yvalue = get_activation_threshold(1) - range * (float)jy/(float)(mosaic.ImageRegionPixels.y);
	      pixel = activation_color_scheme0( -yvalue, WHITE_PIXEL );
	      for (jx=0; jx<mosaic.nx_ColorBar/2; jx++)
		XPutPixel(XImage_colorbar,jx,jy,pixel);
	    }
	}
      else // Scheme +/- 1 or +- 2
	{
	  for (jy=0; jy<mosaic.ImageRegionPixels.y; jy++)
	    {
	      range = get_activation_threshold(1) - get_activation_threshold(-1);
	      yvalue = get_activation_threshold(1) - range * (float)jy/(float)(mosaic.ImageRegionPixels.y);
	      if ( abs(mosaic.activation.color_scheme) == 1 )
		pixel = activation_color_scheme1( yvalue, WHITE_PIXEL );
	      else
		pixel = activation_color_scheme2( yvalue, WHITE_PIXEL );
	      for (jx=0; jx<mosaic.nx_ColorBar; jx++)
		XPutPixel(XImage_colorbar,jx,jy,pixel);
	    }
	}
    }
  XPutImage( XDisplay, XPixmap_image, XGC_image, XImage_colorbar, 0,0,mosaic.ImageRegionPixels.x,0,
  	     XImage_colorbar->width,XImage_colorbar->height );
}

void update_color_scheme()
{
  unsigned long top_pixel;

  if ( mosaic.activation.color_scheme == 0 )  // + & - tails
    {
      // red/2 -> red + red->yellow for +
      top_pixel = 1.5*(WHITE_PIXEL+1)-1;
      act_scale_slope  = (top_pixel-BLACK_PIXEL)/
	(get_activation_threshold(1) - get_activation_threshold(-1));
      act_scale_offset = top_pixel - act_scale_slope * get_activation_threshold(1);
    }
  else if ( abs(mosaic.activation.color_scheme) == 1 )
    { // individual tails (+ or -)
      // blue -> red + red -> yellow
      top_pixel = 2*(WHITE_PIXEL+1)-1;
      act_scale_slope  = (top_pixel-BLACK_PIXEL)/
	(get_activation_threshold(1) - get_activation_threshold(-1));
      act_scale_offset = top_pixel - act_scale_slope * get_activation_threshold(1);
    }
  else if ( abs(mosaic.activation.color_scheme) == 2 )
    { // individual tails (+ or -)
      // color spectrum
      top_pixel = 5*(WHITE_PIXEL+1)-1;
      act_scale_slope  = (top_pixel-BLACK_PIXEL)/
	(get_activation_threshold(1) - get_activation_threshold(-1));
      act_scale_offset = top_pixel - act_scale_slope * get_activation_threshold(1);
    }
}

void label_colorbar()
{ // xdisplay only
  BOOL Colored_Scale;
  IPOINT2D iPut, iStart, iEnd;
  char string[LEN_STR];
  float low_value, high_value, range, pow10div, test, distance, smallest;
  int j, ivalue, ilog;
  int istep, itest_array[16] = {1,2,5,10,20,25,50,100,200,250,500,1000,2000,5000,10000,50000};
  
  if ( !mosaic.Attach_ColorBar ) return;

  if ( mosaic.activation.visible )
    {
      low_value  = get_activation_threshold(-1);
      high_value = get_activation_threshold(1);
      Colored_Scale  = TRUE;
    }
  else
    {
      low_value  = image_scale_min;
      high_value = image_scale_max;
      Colored_Scale  = FALSE;
    }
  range = high_value - low_value;
  ilog  = log10(range);

  if ( ilog > 0 && ilog < 5 )
    {
      ilog = 0;
    }
  else if ( ilog < 0 )
    ilog-=2;
  else if ( ilog == 0 && range < 2. )
    ilog-=1;

  pow10div = powf(10.,(float)ilog);

  smallest = 1.e20;
  istep = 1;
  for (j=0; j<16; j++)
    {
      test = range/pow10div / (float)(itest_array[j]);
      distance = fabs((double)(test-6.)); // choose about 6 labels
      if ( distance < smallest )
	{
	  istep = itest_array[j];
	  smallest = distance;
	}
    }
  low_value  /= pow10div;
  high_value /= pow10div;
  range      /= pow10div;

  ivalue = istep*( (int)(low_value)/istep );
  while ( ivalue < high_value )
    {
      if ( ivalue > low_value )
	{
	  iPut.y = iStart.y = iEnd.y = (high_value - ivalue) * (mosaic.ImageRegionPixels.y) / range;
	  iStart.x = mosaic.ImageRegionPixels.x + 1;
	  iEnd.x   = iStart.x + mosaic.nx_ColorBar/4;
	  DPRIMITIVE_DrawLine(Drawable_ImageData, COLOR_WHITE, iStart, iEnd);

	  if ( Colored_Scale && mosaic.activation.scale_log )
	    {
	      if ( ilog != 0 )
		sprintf(string,"10^-(%de%d)",ivalue,ilog);
	      else
	        sprintf(string,"10^-%d",ivalue);
	    }
	  else
	    {
	      if ( ilog != 0 )
		sprintf(string,"%de%d",ivalue,ilog);
	      else
		sprintf(string,"%d",ivalue);
	    }
	  iPut.x = mosaic.ImageRegionPixels.x+0.3*mosaic.nx_ColorBar;
	  iPut.y = MAX(iPut.y-2,INFO_HEIGHT);
	  if ( mosaic.activation.visible )
	    DPRIMITIVE_DrawText(Drawable_ImageData, COLOR_WHITE, iPut, string);
	  else
	    DPRIMITIVE_FillText(Drawable_ImageData, COLOR_WHITE, iPut, string);
	}
      ivalue += istep;
    }
}

void show_information_colorbar()
{
  IPOINT2D iPut;
  char string[LEN_STR];
  float range;
  int which_color;

  //  if ( !mosaic.activation.visible )
  //    return;

  if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR ) // colorbar
    which_color = COLOR_RED;
  else
    which_color = COLOR_WHITE;
  // Put the activation range below the colorbar
  if ( mosaic.activation.nmaps == 0 )
    {
      range = image_scale_max - image_scale_min;
      if ( range > 5. )
	sprintf(string,"%d to %d",(int)image_scale_min,(int)image_scale_max);
      else
	sprintf(string,"%g to %g",image_scale_min,image_scale_max);
    }
  else
    sprintf(string,"%d to %d",(int)(get_activation_threshold(-1)),(int)(get_activation_threshold(1)));

  iPut.x = XImage_BottomMenu->width - mosaic.nx_ColorBar;
  iPut.y = INFO_HEIGHT-TEXTOFF;
  DPRIMITIVE_FillText(Drawable_BottomMenu, which_color, iPut, string);
}

void DPRIMITIVE_DrawLine(int which_drawable, int which_color, IPOINT2D iDraw1, IPOINT2D iDraw2)
{
  GC *gc;
  BOOL graph;

  if ( !XWindow_image ) return;
  graph = (which_drawable == Drawable_GraphWindow)
    ||    (which_drawable == Drawable_GraphData)
    ||    (which_drawable == Drawable_GraphInfoArea);
  if ( graph && !XWindow_graph ) return;

  if ( which_drawable == Drawable_ImageWindow )
    {
      gc = XGC_color_image(which_color);
      XDrawLine(XDisplay,XWindow_image,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_GraphWindow )
    {
      gc = XGC_color_graph(which_color);
      XDrawLine(XDisplay,XWindow_graph,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_ImageData )
    {
      gc = XGC_color_image(which_color);
      //      XDrawLine(XDisplay,XPixmap_image,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
      XDrawLine(XDisplay,XWindow_image,*gc,iDraw1.x,iDraw1.y+2*INFO_HEIGHT,iDraw2.x,iDraw2.y+2*INFO_HEIGHT);
    }
  else if ( which_drawable == Drawable_TopMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawLine(XDisplay,XPixmap_TopMenu,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_BottomMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawLine(XDisplay,XPixmap_BottomMenu,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_ImageInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawLine(XDisplay,XPixmap_ImageInfoArea,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_RightPanel )
    {
      gc = XGC_color_image(which_color);
      XDrawLine(XDisplay,XPixmap_RightPanel,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_GraphData )
    {
      gc = XGC_color_graph(which_color);
      XDrawLine(XDisplay,XPixmap_GraphData,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else if ( which_drawable == Drawable_GraphInfoArea )
    {
      gc = XGC_color_graph(which_color);
      XDrawLine(XDisplay,XPixmap_GraphInfoArea,*gc,iDraw1.x,iDraw1.y,iDraw2.x,iDraw2.y);
    }
  else
    {
      printf("Programming error in DPRIMITIVE_DrawLine: drawable = %d\n",which_drawable);
      exit(1);
    }
}

void DPRIMITIVE_DrawPoint(int which_drawable, int which_color, IPOINT2D iDraw)
{
  GC *gc;
  BOOL graph;

  if ( !XWindow_image ) return;
  graph = (which_drawable == Drawable_GraphWindow)
    ||    (which_drawable == Drawable_GraphData)
    ||    (which_drawable == Drawable_GraphInfoArea);
  if ( graph && !XWindow_graph ) return;

  if ( which_drawable == Drawable_ImageWindow )
    {
      gc = XGC_color_image(which_color);
      XDrawPoint(XDisplay,XWindow_image,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_GraphWindow )
    {
      gc = XGC_color_graph(which_color);
      XDrawPoint(XDisplay,XWindow_graph,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_ImageData )
    {
      gc = XGC_color_image(which_color);
      //      XDrawPoint(XDisplay,XPixmap_image,*gc,iDraw.x,iDraw.y);
      XDrawPoint(XDisplay,XWindow_image,*gc,iDraw.x,iDraw.y+2*INFO_HEIGHT);
    }
  else if ( which_drawable == Drawable_TopMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawPoint(XDisplay,XPixmap_TopMenu,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_BottomMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawPoint(XDisplay,XPixmap_BottomMenu,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_ImageInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawPoint(XDisplay,XPixmap_ImageInfoArea,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_RightPanel )
    {
      gc = XGC_color_image(which_color);
      XDrawPoint(XDisplay,XPixmap_RightPanel,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_GraphData )
    {
      gc = XGC_color_graph(which_color);
      XDrawPoint(XDisplay,XPixmap_GraphData,*gc,iDraw.x,iDraw.y);
    }
  else if ( which_drawable == Drawable_GraphInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawPoint(XDisplay,XPixmap_GraphInfoArea,*gc,iDraw.x,iDraw.y);
    }
  else
    {
      printf("Programming error in DPRIMITIVE_DrawPoint: drawable = %d\n",which_drawable);
      exit(1);
    }
}

void DPRIMITIVE_DrawRectangle(int which_drawable, int which_color, IPOINT2D iDraw1, IPOINT2D iDraw2)
{
  IPOINT2D iStart, iEnd, iArea;
  GC *gc;
  BOOL graph;

  if ( !XWindow_image ) return;
  graph = (which_drawable == Drawable_GraphWindow)
    ||    (which_drawable == Drawable_GraphData)
    ||    (which_drawable == Drawable_GraphInfoArea);
  if ( graph && !XWindow_graph ) return;

  iStart.x = iDraw1.x;
  iEnd.x   = iDraw2.x;

  if ( iDraw2.y >= iDraw1.y )
    {
      iStart.y = iDraw1.y;
      iEnd.y   = iDraw2.y;
    }
  else
    {
      iStart.y = iDraw2.y;
      iEnd.y   = iDraw1.y;
    }
  iArea.x = iEnd.x - iStart.x + 1; // inclusive
  iArea.y = iEnd.y - iStart.y + 1; // inclusive

  if ( which_drawable == Drawable_ImageWindow )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XWindow_image, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_GraphWindow )
    {
      gc = XGC_color_graph(which_color);
      XDrawRectangle(XDisplay, XWindow_graph, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_ImageData )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XPixmap_image, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
      /*
      XDrawRectangle(XDisplay, XWindow_image, *gc,
		     iStart.x, iStart.y+2*INFO_HEIGHT,
		     iArea.x,  iArea.y +2*INFO_HEIGHT);
      */
    }
  else if ( which_drawable == Drawable_TopMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XPixmap_TopMenu, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_BottomMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XPixmap_BottomMenu, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_ImageInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XPixmap_ImageInfoArea, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_RightPanel )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XPixmap_RightPanel, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_GraphData )
    {
      gc = XGC_color_graph(which_color);
      XDrawRectangle(XDisplay, XPixmap_GraphData, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_GraphInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawRectangle(XDisplay, XPixmap_GraphInfoArea, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else
    {
      printf("Programming error in DPRIMITIVE_DrawRectangle: drawable = %d\n",which_drawable);
      exit(1);
    }
}

void DPRIMITIVE_FillRectangle(int which_drawable, int which_color, IPOINT2D iDraw1, IPOINT2D iDraw2)
{
  IPOINT2D iStart, iEnd, iArea;
  GC *gc;
  BOOL graph;

  if ( !XWindow_image ) return;
  graph = (which_drawable == Drawable_GraphWindow)
    ||    (which_drawable == Drawable_GraphData)
    ||    (which_drawable == Drawable_GraphInfoArea);
  if ( graph && !XWindow_graph ) return;

  iStart.x = iDraw1.x;
  iEnd.x   = iDraw2.x;

  if ( iDraw2.y >= iDraw1.y )
    {
      iStart.y = iDraw1.y;
      iEnd.y   = iDraw2.y;
    }
  else
    {
      iStart.y = iDraw2.y;
      iEnd.y   = iDraw1.y;
    }
  iArea.x = iEnd.x - iStart.x + 1; // inclusive
  iArea.y = iEnd.y - iStart.y + 1; // inclusive

  if ( which_drawable == Drawable_ImageWindow )
    {
      gc = XGC_color_image(which_color);
      XFillRectangle(XDisplay, XWindow_image, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_GraphWindow )
    {
      gc = XGC_color_graph(which_color);
      XFillRectangle(XDisplay, XWindow_graph, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_ImageData )
    {
      gc = XGC_color_image(which_color);
      //      XFillRectangle(XDisplay, XPixmap_image, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
      XFillRectangle(XDisplay, XWindow_image, *gc,
		     iStart.x, iStart.y + 2*INFO_HEIGHT,
		     iArea.x,  iArea.y  + 2*INFO_HEIGHT);
    }
  else if ( which_drawable == Drawable_TopMenu )
    {
      gc = XGC_color_image(which_color);
      XFillRectangle(XDisplay, XPixmap_TopMenu, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_BottomMenu )
    {
      gc = XGC_color_image(which_color);
      XFillRectangle(XDisplay, XPixmap_BottomMenu, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_ImageInfoArea )
    {
      gc = XGC_color_image(which_color);
      XFillRectangle(XDisplay, XPixmap_ImageInfoArea, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_RightPanel )
    {
      gc = XGC_color_image(which_color);
      XFillRectangle(XDisplay, XPixmap_RightPanel, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_GraphData )
    {
      gc = XGC_color_graph(which_color);
      XFillRectangle(XDisplay, XPixmap_GraphData, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else if ( which_drawable == Drawable_GraphInfoArea )
    {
      gc = XGC_color_image(which_color);
      XFillRectangle(XDisplay, XPixmap_GraphInfoArea, *gc, iStart.x, iStart.y, iArea.x, iArea.y);
    }
  else
    {
      printf("Programming error in DPRIMITIVE_FillRectangle: drawable = %d\n",which_drawable);
      exit(1);
    }
}

void DPRIMITIVE_DrawText(int which_drawable, int which_color, IPOINT2D iDraw, char *string)
{
  GC *gc;
  BOOL graph;

  if ( !XWindow_image ) return;
  graph = (which_drawable == Drawable_GraphWindow)
    ||    (which_drawable == Drawable_GraphData)
    ||    (which_drawable == Drawable_GraphInfoArea);
  if ( graph && !XWindow_graph ) return;

  if ( which_drawable == Drawable_ImageWindow )
    {
      gc = XGC_color_image(which_color);
      XDrawString(XDisplay,XWindow_image,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_GraphWindow )
    {
      gc = XGC_color_graph(which_color);
      XDrawString(XDisplay,XWindow_graph,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_ImageData )
    {
      gc = XGC_color_image(which_color);
      //      XDrawString(XDisplay,XPixmap_image,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
      XDrawString(XDisplay,XWindow_image,*gc,iDraw.x,iDraw.y+2*INFO_HEIGHT,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_TopMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawString(XDisplay,XPixmap_TopMenu,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_BottomMenu )
    {
      gc = XGC_color_image(which_color);
      XDrawString(XDisplay,XPixmap_BottomMenu,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_ImageInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawString(XDisplay,XPixmap_ImageInfoArea,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_RightPanel )
    {
      gc = XGC_color_image(which_color);
      XDrawString(XDisplay,XPixmap_RightPanel,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_GraphData )
    {
      gc = XGC_color_graph(which_color);
      XDrawString(XDisplay,XPixmap_GraphData,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_GraphInfoArea )
    {
      gc = XGC_color_image(which_color);
      XDrawString(XDisplay,XPixmap_GraphInfoArea,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else
    {
      printf("Programming error in DPRIMITIVE_Drawtext: drawable = %d\n",which_drawable);
      exit(1);
    }
}

void DPRIMITIVE_FillText(int which_drawable, int which_color, IPOINT2D iDraw, char *string)
{
  GC *gc;
  BOOL graph;

  if ( !XWindow_image ) return;
  graph = (which_drawable == Drawable_GraphWindow)
    ||    (which_drawable == Drawable_GraphData)
    ||    (which_drawable == Drawable_GraphInfoArea);
  if ( graph && !XWindow_graph ) return;

  if ( which_drawable == Drawable_ImageWindow )
    {
      gc = XGC_color_image_text(which_color);
      XDrawImageString(XDisplay,XWindow_image,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_GraphWindow )
    {
      gc = XGC_color_graph_text(which_color);
      XDrawImageString(XDisplay,XWindow_graph,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_ImageData )
    {
      gc = XGC_color_image_text(which_color);
      //      XDrawImageString(XDisplay,XPixmap_image,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
      XDrawImageString(XDisplay,XWindow_image,*gc,iDraw.x,iDraw.y+2*INFO_HEIGHT,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_TopMenu )
    {
      gc = XGC_color_image_text(which_color);
      XDrawImageString(XDisplay,XPixmap_TopMenu,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_BottomMenu )
    {
      gc = XGC_color_image_text(which_color);
      XDrawImageString(XDisplay,XPixmap_BottomMenu,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_ImageInfoArea )
    {
      gc = XGC_color_image_text(which_color);
      XDrawImageString(XDisplay,XPixmap_ImageInfoArea,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_RightPanel )
    {
      gc = XGC_color_image_text(which_color);
      XDrawImageString(XDisplay,XPixmap_RightPanel,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_GraphData )
    {
      gc = XGC_color_graph_text(which_color);
      XDrawImageString(XDisplay,XPixmap_GraphData,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else if ( which_drawable == Drawable_GraphInfoArea )
    {
      gc = XGC_color_graph_text(which_color);
      XDrawImageString(XDisplay,XPixmap_GraphInfoArea,*gc,iDraw.x,iDraw.y,string,jip_int_count((JIP_INDEX)strlen(string)));
    }
  else
    {
      printf("Programming error in DPRIMITIVE_FillText: drawable = %d\n",which_drawable);
      exit(1);
    }
}

GC *XGC_color_image( int color )
{
  GC *gc;

  if ( color == COLOR_CYAN )
    gc = &XGC_cyan;
  else if ( color == COLOR_RED )
    gc = &XGC_red;
  else if ( color == COLOR_GREEN )
    gc = &XGC_green;
  else if ( color == COLOR_BLUE )
    gc = &XGC_blue;
  else if ( color == COLOR_WHITE )
    gc = &XGC_white;
  else if ( color == COLOR_PURPLE )
    gc = &XGC_purple;
  else if ( color == COLOR_YELLOW )
    gc = &XGC_yellow;
  else if ( color == COLOR_ORANGE )
    gc = &XGC_orange;
  else if ( color == COLOR_BLACK )
    gc = &XGC_black;
  else
    {
      printf("Programming error: image color = %d\n",color);
      exit(1);
    }

  return(gc);
}

GC *XGC_color_image_text( int color )
{
  GC *gc;

  if ( color == COLOR_CYAN )
    gc = &XGC_cyan_text;
  else if ( color == COLOR_RED )
    gc = &XGC_red_text;
  else if ( color == COLOR_GREEN )
    gc = &XGC_green_text;
  else if ( color == COLOR_BLUE )
    gc = &XGC_blue_text;
  else if ( color == COLOR_WHITE )
    gc = &XGC_white_text;
  else if ( color == COLOR_PURPLE )
    gc = &XGC_purple_text;
  else if ( color == COLOR_YELLOW )
    gc = &XGC_yellow_text;
  else if ( color == COLOR_ORANGE )
    gc = &XGC_orange_text;
  else if ( color == COLOR_GRAY )
    gc = &XGC_gray_text;
  else
    {
      printf("Programming error: image text color = %d\n",color);
      exit(1);
    }

  return(gc);
}

GC *XGC_color_graph( int color )
{
  GC *gc;

  if ( color == COLOR_CYAN )
    gc = &XGC_graph_cyan;
  else if ( color == COLOR_RED )
    gc = &XGC_graph_red;
  else if ( color == COLOR_BLUE )
    gc = &XGC_graph_blue;
  else if ( color == COLOR_GREEN )
    gc = &XGC_graph_green;
  else if ( color == COLOR_WHITE )
    gc = &XGC_graph_white;
  else if ( color == COLOR_YELLOW )
    gc = &XGC_graph_yellow;
  else if ( color == COLOR_GRAY )
    gc = &XGC_graph_gray;
  else if ( color == COLOR_ORANGE )
    gc = &XGC_graph_orange;
  else if ( color == COLOR_PURPLE )
    gc = &XGC_graph_purple;
  else
    {
      printf("Programming error: graph color = %d\n",color);
      exit(1);
    }

  return(gc);
}

GC *XGC_color_graph_text( int color )
{
  GC *gc;

  if ( color == COLOR_CYAN )
    gc = &XGC_graph_cyan_text;
  else if ( color == COLOR_RED )
    gc = &XGC_graph_red_text;
  else if ( color == COLOR_GREEN )
    gc = &XGC_graph_green_text;
  else if ( color == COLOR_BLUE )
    gc = &XGC_graph_blue_text;
  else if ( color == COLOR_WHITE )
    gc = &XGC_graph_white_text;
  else if ( color == COLOR_YELLOW )
    gc = &XGC_graph_yellow_text;
  else if ( color == COLOR_GRAY )
    gc = &XGC_graph_gray_text;
  else if ( color == COLOR_ORANGE )
    gc = &XGC_graph_orange_text;
  else if ( color == COLOR_PURPLE )
    gc = &XGC_graph_purple_text;
  else
    {
      printf("Programming error: graph text color = %d\n",color);
      exit(1);
    }

  return(gc);
}
