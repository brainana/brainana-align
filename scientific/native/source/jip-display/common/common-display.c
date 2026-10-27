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

/////////////////////////////////////////////////////////////////////////
// These functions are used for basic image display operations. /////////
// However, X11 is not used explicitly in these functions. //////////////
/////////////////////////////////////////////////////////////////////////


void define_mosaic_cursor_location(int x2d, int y2d)
{
  // Set the default cursor position.
  // "icursor" values point to a data index, so never let these out of range.
  mosaic.icursor.x = mosaic.icursor.y = mosaic.icursor.z = 0.;
  // "cursor" values are real. They may indicate real data indices, or simply window locations.
  mosaic.cursor.x  = mosaic.cursor.y  = mosaic.cursor.z  = 0.;

  // Text options above the image window
  if ( y2d < 2*INFO_HEIGHT )
    CursorSelection_AboveImageWindow( x2d, y2d );
  else if ( y2d >= mosaic.ImageRegionPixels.y+2*INFO_HEIGHT )
    {
      y2d -= mosaic.ImageRegionPixels.y+2*INFO_HEIGHT;
      CursorSelection_BelowImageWindow( x2d, y2d );
    }
  else
    { // The cursor is somewhere on the image window, including the colorbar.
      if ( x2d > mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar )  // cursor is on the right region
	mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_RIGHT_PANEL;
      else if ( x2d > mosaic.ImageRegionPixels.x ) // cursor is on the color bar
	CursorSelection_OnColorBar( y2d );
      else // Otherwise, the cursor is somewhere on the image window.
	CursorSelection_OnImageWindow(x2d, y2d);
    }

  // Highlight the active window when the display shows orthogonal planes.
  if ( mosaic.display_3orthogonal )
    highlight_active_image_plane();

}

void CursorSelection_AboveImageWindow( int ix_pos, int iy_pos )
{
  int FieldID;

  if (( FieldID=CursorOnSelectableText_AboveImageWindow(ix_pos,iy_pos) ))
    {
      mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_SELECT_TEXT;
      mosaic.text_selection   = FieldID;
    }
  else
    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
}

void CursorSelection_BelowImageWindow( int ix_pos, int iy_pos )
{
  int FieldID;

  if (( FieldID=CursorOnSelectableText_BelowImageWindow(ix_pos,iy_pos) ))
    {
      mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_SELECT_TEXT;
      mosaic.text_selection   = FieldID;
    }
  else
    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
}

int CursorOnSelectableText_AboveImageWindow( int ix_pos, int iy_pos )
{
  struct SelectableText *SelectText;
  int ignore, FieldID;

  FieldID = 0;
  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->YLocation==0 && SelectText->visible )
	{ // Then the text is just above the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > 0 && iy_pos < INFO_HEIGHT )
	    {
	      ignore  = FALSE;
	      if ( !ignore )
		{
		  FieldID = SelectText->FieldID;
		  SelectText->selected = 1;
		  // printf("for Y0, x: (%d, %d)\n",SelectText->XSelectStart,SelectText->XSelectEnd);
		}
	    }
	  else
	    SelectText->selected = 0;
	}
      else if ( SelectText->YLocation==1 && SelectText->visible )
	{ // Then the text is just above the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > INFO_HEIGHT && iy_pos < 2*INFO_HEIGHT )
	    {
	      ignore  = (SelectText->FieldID==TEXTFIELD_DATA_TYPE && mosaic.stack->data_type==0);
	      ignore |= (SelectText->FieldID==TEXTFIELD_WHICH_OVL_WIRE && mosaic.toggle_overlays==0);
	      if ( !ignore )
		{
		  FieldID = SelectText->FieldID;
		  SelectText->selected = 1;
		}
	    }
	  else
	    SelectText->selected = 0;
	}
    }
  return(FieldID);
}

int CursorOnSelectableText_BelowImageWindow( int ix_pos, int iy_pos )
{
  struct SelectableText *SelectText;
  int FieldID;

  FieldID = 0;
  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->YLocation==2 && SelectText->visible )
	{ // Then the text is below the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > 0 && iy_pos < INFO_HEIGHT )
	    {
	      FieldID = SelectText->FieldID;
	      SelectText->selected = 1;
	    }
	  else
	    SelectText->selected = 0;
	}
      else if ( SelectText->YLocation==3 && SelectText->visible )
	{ // Then the text is below the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > INFO_HEIGHT && iy_pos < 2*INFO_HEIGHT )
	    {
	      FieldID = SelectText->FieldID;
	      SelectText->selected = 1;
	    }
	  else
	    SelectText->selected = 0;
	}
      else if ( SelectText->YLocation==4 && SelectText->visible )
	{ // Then the text is below the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > 2*INFO_HEIGHT && iy_pos < 3*INFO_HEIGHT )
	    {
	      FieldID = SelectText->FieldID;
	      SelectText->selected = 1;
	    }
	  else
	    SelectText->selected = 0;
	}
    }
  return(FieldID);
}

int CursorOnSelectableText_AboveGraphWindow( int ix_pos )
{
  struct SelectableText *SelectText;
  int ignore, FieldID;

  FieldID = 0;
  for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->YLocation==1 && SelectText->visible )
	{ // Then the text is above the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos <= SelectText->XSelectEnd )
	    {
	      ignore  = FALSE;
	      if ( !ignore )
		{
		  FieldID = SelectText->FieldID;
		  SelectText->selected = 1;
		}
	    }
	  else
	    SelectText->selected = 0;
	}
    }
  return(FieldID);
}

int CursorOnSelectableText_BelowGraphWindow( int ix_pos, int iy_pos )
{
  struct SelectableText *SelectText;
  int FieldID;

  FieldID = 0;
  for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->YLocation==2 && SelectText->visible )
	{ // Then the text is below the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > 0 && iy_pos < INFO_HEIGHT )
	    {
	      FieldID = SelectText->FieldID;
	      SelectText->selected = 1;
	    }
	  else
	    SelectText->selected = 0;
	}
      else if ( SelectText->YLocation==3 && SelectText->visible )
	{ // Then the text is below the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > INFO_HEIGHT && iy_pos < 2*INFO_HEIGHT )
	    {
	      FieldID = SelectText->FieldID;
	      SelectText->selected = 1;
	    }
	  else
	    SelectText->selected = 0;
	}
      else if ( SelectText->YLocation==4 && SelectText->visible )
	{ // Then the text is below the image window and visible.
	  if ( ix_pos >= SelectText->XSelectStart && ix_pos < SelectText->XSelectEnd &&
	       iy_pos > 2*INFO_HEIGHT && iy_pos < 3*INFO_HEIGHT )
	    {
	      FieldID = SelectText->FieldID;
	      SelectText->selected = 1;
	    }
	  else
	    SelectText->selected = 0;
	}
    }
  return(FieldID);
}

void CursorSelection_OnColorBar( int iy )
{
  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_COLORBAR;

  if ( iy - 2*INFO_HEIGHT - mosaic.zoom*NBORDER < mosaic.ImageRegionPixels.y/2 )
    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_COLORBAR_HIGH;
  else
    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_COLORBAR_LOW;
  // The y cursor location is needed for colorbar selection,
  // so save this value in the "cursor" variable.
  mosaic.cursor.y = iy - 2*INFO_HEIGHT - mosaic.zoom * NBORDER; // subtract off y offset
}

void CursorSelection_OnImageWindow( int x2d, int y2d )
{
  IPOINT2D iPixel, iPanel, PixelDim;
  POINT2D  rPixel, PixelRes;
  IPOINT3D icursor;
  POINT3D   cursor;
  int iZ, iZstep, jz;

  // Account for the image display boundaries
  iPixel.x = x2d - mosaic.zoom * NBORDER;               // subtract off x offset
  iPixel.y = y2d - 2*INFO_HEIGHT - mosaic.zoom * NBORDER; // subtract off y offset
  rPixel.x = iPixel.x;
  rPixel.y = iPixel.y;
  // Account for the zoom factor
  iPixel.x = iPixel.x / mosaic.zoom;
  iPixel.y = iPixel.y / mosaic.zoom;
  rPixel.x = rPixel.x / (float)mosaic.zoom;
  rPixel.y = rPixel.y / (float)mosaic.zoom;

  if ( iPixel.x < 0 || iPixel.y < 0 )
    {
      // The cursor is on a border.
      mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
      return;
    }

  // Save the cursor location.
  if ( mosaic.display_3orthogonal )
    { // The displays shows 3 projections.

      // Find the display panel.
      iPanel.x = iPanel.y = 0;
      if ( iPixel.x >= mosaic.PixelDim.x )
	{
	  iPixel.x -= mosaic.PixelDim.x + NBORDER;
	  rPixel.x -= mosaic.PixelDim.x + NBORDER;
	  iPanel.x++;
	}
      if ( iPixel.y >= mosaic.PixelDim.y )
	{
	  iPixel.y -= mosaic.PixelDim.y + NBORDER;
	  rPixel.y -= mosaic.PixelDim.y + NBORDER;
	  iPanel.y++;
	}

      if ( iPanel.x == 0 && iPanel.y == 0 )
	{ // x-y
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_XY;
	  // Set the z position from the crosshair.
	  mosaic.icursor.z = mosaic.icross.z;  // Z is not on the x-y projection.
	  mosaic.cursor.z  = mosaic.icursor.z;
	  // Set the x and y positions on the mosaic.
	  mosaic.cursor.x = rPixel.x / mosaic.PixelResolution.x;
	  mosaic.cursor.y = rPixel.y / mosaic.PixelResolution.y;
	  mosaic.icursor.x = floor((double)mosaic.cursor.x); // truncation of real value
	  mosaic.icursor.y = floor((double)mosaic.cursor.y);
	}
      else if ( iPanel.x > 0 && iPanel.y > 0 )
	{ // y-z
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_YZ;
	  if ( mosaic.display_ortho_ZY )
	    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
	  else
	    {
	      // Set the x position from the crosshair.
	      mosaic.icursor.x = mosaic.icross.x;  // x is not on the y-z projection
	      mosaic.cursor.x  = mosaic.icursor.x;
	      // Set the y (from x) and z (from y) position.
	      mosaic.cursor.y = rPixel.x / mosaic.PixelResolution.y; // horizontal
	      mosaic.cursor.y = rPixel.x / mosaic.PixelResolution.y;
	      mosaic.cursor.z = rPixel.y / mosaic.PixelResolution.z; // vertical
	      mosaic.cursor.z = rPixel.y / mosaic.PixelResolution.z;
	      mosaic.icursor.y = floor((double)mosaic.cursor.y); // truncation of real value
	      mosaic.icursor.z = floor((double)mosaic.cursor.z);
	    }
	}
      else if ( iPanel.x > 0 )
	{ // z-y
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_ZY;
	  if ( mosaic.display_ortho_ZY )
	    {
	      // Set the x position from the crosshair.
	      mosaic.icursor.x = mosaic.icross.x;  // x is not on the y-z projection
	      mosaic.cursor.x  = mosaic.icursor.x;
	      // Set the z (from x) and y (from y) position.
	      mosaic.cursor.z = rPixel.x / mosaic.PixelResolution.z; // horizontal
	      mosaic.cursor.z = rPixel.x / mosaic.PixelResolution.z;
	      mosaic.cursor.y = rPixel.y / mosaic.PixelResolution.y; // vertical
	      mosaic.cursor.y = rPixel.y / mosaic.PixelResolution.y;
	      mosaic.icursor.z = floor((double)mosaic.cursor.z); // truncation of real value
	      mosaic.icursor.y = floor((double)mosaic.cursor.y);
	    }
	  else
	    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
	}
      else if ( iPanel.y > 0 )
	{ // x-z
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_XZ;
	  // Set the y position from the crosshair.
	  mosaic.icursor.y = mosaic.icross.y;  // y is not on the x-z projection
	  mosaic.cursor.y  = mosaic.icursor.y;
	  // Set the x (from x) and z (from y) position.
	  mosaic.cursor.x = rPixel.x / mosaic.PixelResolution.x; // horizontal
	  mosaic.cursor.x = rPixel.x / mosaic.PixelResolution.x;
	  mosaic.cursor.z = rPixel.y / mosaic.PixelResolution.z; // vertical
	  mosaic.cursor.z = rPixel.y / mosaic.PixelResolution.z;
	  mosaic.icursor.x = floor((double)mosaic.cursor.x); // truncation of real value
	  mosaic.icursor.z = floor((double)mosaic.cursor.z);
	}
    }
  else
    { // mosaic display

      // Find the display panel.
      iPanel.x = iPanel.y = 0;

      if ( mosaic.mosaic_orientation == ORTHO_XY )
	{
	  PixelDim.x = mosaic.PixelDim.x;
	  PixelDim.y = mosaic.PixelDim.y;
	  PixelRes.x = mosaic.PixelResolution.x;
	  PixelRes.y = mosaic.PixelResolution.y;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_ZY )
	{
	  PixelDim.x = mosaic.PixelDim.z;
	  PixelDim.y = mosaic.PixelDim.y;
	  PixelRes.x = mosaic.PixelResolution.z;
	  PixelRes.y = mosaic.PixelResolution.y;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	{
	  PixelDim.x = mosaic.PixelDim.x;
	  PixelDim.y = mosaic.PixelDim.z;
	  PixelRes.x = mosaic.PixelResolution.x;
	  PixelRes.y = mosaic.PixelResolution.z;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_YZ )
	{
	  PixelDim.x = mosaic.PixelDim.y;
	  PixelDim.y = mosaic.PixelDim.z;
	  PixelRes.x = mosaic.PixelResolution.y;
	  PixelRes.y = mosaic.PixelResolution.z;
	}

      while ( iPixel.x >= PixelDim.x )
	{
	  iPixel.x -= PixelDim.x + NBORDER;
	  rPixel.x -= PixelDim.x + NBORDER;
	  iPanel.x++;
	}
      while ( iPixel.y >= PixelDim.y )
	{
	  iPixel.y -= PixelDim.y + NBORDER;
	  rPixel.y -= PixelDim.y + NBORDER;
	  iPanel.y++;
	}

      // Set the x and y positions on the mosaic.
      mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_MOSAIC;
      cursor.x = rPixel.x / PixelRes.x;
      cursor.y = rPixel.y / PixelRes.y;
      icursor.x = floor((double)cursor.x); // truncation of real value
      icursor.y = floor((double)cursor.y);
      // Set the z position.
      icursor.z = -1;
      iZ = jip_coordinate(((JIP_INDEX)(iPanel.y)) * mosaic.DimPanels.x + iPanel.x);
      for ( jz=0, iZstep=-1; jz<mosaic.DisplayVolumeDim.z; jz++ )
	{
	  if ( mosaic.display_slice[jz] ) iZstep++;
	  if ( iZstep == iZ )
	    {
	      icursor.z = jz;
	      cursor.z  = jz;
	      break;
	    }
	}
      if ( mosaic.mosaic_orientation == ORTHO_XY )
	{
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_XY;
	  mosaic.icursor = icursor;
	  mosaic.cursor  =  cursor;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_ZY )
	{
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_ZY;
	  mosaic.icursor.x = icursor.z;
	  mosaic.icursor.y = icursor.y;
	  mosaic.icursor.z = icursor.x;
	  mosaic.cursor.x  =  cursor.z;
	  mosaic.cursor.y  =  cursor.y;
	  mosaic.cursor.z  =  cursor.x;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	{
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_XZ;
	  mosaic.icursor.x = icursor.x;
	  mosaic.icursor.y = icursor.z;
	  mosaic.icursor.z = icursor.y;
	  mosaic.cursor.x  =  cursor.x;
	  mosaic.cursor.y  =  cursor.z;
	  mosaic.cursor.z  =  cursor.y;
	}
      else if ( mosaic.mosaic_orientation == ORTHO_YZ )
	{
	  mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_ORTHO_YZ;
	  mosaic.icursor.x = icursor.z;
	  mosaic.icursor.y = icursor.x;
	  mosaic.icursor.z = icursor.y;
	  mosaic.cursor.x  =  cursor.z;
	  mosaic.cursor.y  =  cursor.x;
	  mosaic.cursor.z  =  cursor.y;
	}
    }

  if ( !(mosaic.cursor_selection&CURSOR_ON_NOTHING_SPECIAL) &&
       mosaic.icursor.x>= 0 && mosaic.icursor.x<mosaic.stack->dim.x &&
       mosaic.icursor.y>= 0 && mosaic.icursor.y<mosaic.stack->dim.y &&
       mosaic.icursor.z>= 0 && mosaic.icursor.z<mosaic.stack->dim.z )
    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_DATA;
  else
    //    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
    mosaic.cursor_selection = mosaic.cursor_selection|CURSOR_ON_MOSAIC_BORDER;

  // Don't leave this function without making sure indices are in range.
  if ( mosaic.icursor.x < 0 || mosaic.icursor.x >= mosaic.stack->dim.x ) mosaic.icursor.x = 0;
  if ( mosaic.icursor.y < 0 || mosaic.icursor.y >= mosaic.stack->dim.y ) mosaic.icursor.y = 0;
  if ( mosaic.icursor.z < 0 || mosaic.icursor.z >= mosaic.stack->dim.z ) mosaic.icursor.z = 0;

  //  printf("(ix,iy,iz) = (%d,%d,%d)\n",mosaic.icursor.x,mosaic.icursor.y,mosaic.icursor.z);

  // Potentially flip the convention for y.
  // X11 defines positive y as down.
  flip_vertical_CursorIndex();

  // Convert the cursor position to spatial coordinates.
  mosaic.cursor = Convert_rVoxel_to_Space(mosaic.cursor,mosaic.stack);
  // shift the position by 1/2 res. so that the CENTER of the 1st voxel is the origin.
  if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY ||
       mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ )
    mosaic.cursor.x -= mosaic.stack->direction.x*mosaic.stack->resolution.x/2.;
  if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY ||
       mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY ||
       mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ )
    mosaic.cursor.y -= mosaic.stack->direction.y*mosaic.stack->resolution.y/2.;
  if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY ||
       mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ )
    mosaic.cursor.z -= mosaic.stack->direction.z*mosaic.stack->resolution.z/2.;
}

void reset_cursor_selections()
{
  struct SelectableText *SelectText;

  mosaic.cursor_selection = mosaic.text_selection = 0;
  graph.cursor_selection  = graph.text_selection  = 0;
  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    SelectText->selected = 0;
  for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
    SelectText->selected = 0;
}

void define_graph_cursor_location(int x2d, int y2d)
{
  int FieldID;
  float xscale, yscale;

  if ( y2d < INFO_HEIGHT )
    { // Text options above the graph window
      if (( FieldID=CursorOnSelectableText_AboveGraphWindow(x2d) ))
	{
	  graph.cursor_selection = graph.cursor_selection|CURSOR_ON_SELECT_TEXT;
	  graph.text_selection   = FieldID;
	}
      else
	graph.cursor_selection = graph.cursor_selection|CURSOR_ON_NOTHING_SPECIAL;
    }
  else if ( y2d >= graph.nheight + INFO_HEIGHT )
    { // below the graph window
      y2d -= graph.nheight + INFO_HEIGHT;
      if ( (FieldID=CursorOnSelectableText_BelowGraphWindow(x2d,y2d)) )
	{
	  graph.cursor_selection = graph.cursor_selection|CURSOR_ON_SELECT_TEXT;
	  graph.text_selection   = FieldID;
	}
    }
  else
    graph.cursor_selection = graph.cursor_selection|CURSOR_ON_DATA;

  // define the graph cursor coordinates

  xscale = (float)(graph.nwidth)  / (float)(graph.npoints);

  graph.icursor.x = (float)x2d/xscale;
  if ( graph.icursor.x >= graph.npoints )
    graph.icursor.x = -1;

  graph.icursor.y = y2d - INFO_HEIGHT;
  if ( graph.icursor.y < 0 || graph.icursor.y >= graph.nheight )
    graph.icursor.y = -1;

  xscale = (float)(graph.npoints-1) / (graph.xmax-graph.xmin);
  yscale = (float)(graph.nheight)   / (graph.ymax-graph.ymin);

  //  graph.cursor.x = graph.icursor.x;
  graph.cursor.x = graph.xmin + (float)graph.icursor.x/ xscale;
  // Report y in data units.
  graph.cursor.y = graph.ymax - (float)graph.icursor.y / yscale;

}

JIP_INDEX index4d(int ix, int iy, int iz, int it)
{
  if ( ix < 0 || ix > mosaic.stack->dim.x-1 ||
       iy < 0 || iy > mosaic.stack->dim.y-1 || 
       iz < 0 || iz > mosaic.stack->dim.z-1 ||
       it < 0 || it > mosaic.stack->nt-1 )
    {
      printf("Programming error: index out of range in index4d.\n");
      printf("(x,y,z,t) = (%d,%d,%d,%d)\n",ix,iy,iz,it);
      exit(1);
    }
  return((JIP_INDEX)it * mosaic.stack->n3d + ((JIP_INDEX)(iz)) * mosaic.stack->dim.x*mosaic.stack->dim.y + ((JIP_INDEX)(iy)) * mosaic.stack->dim.x + ix);
}

float stack_data_4d(int ix, int iy, int iz, int it)
{
  FCOMPLEX *FCData;
  float data, mag, phase;
  JIP_INDEX index;

  index = index4d(ix,iy,iz,it);
  if ( mosaic.stack->data_type == 0 )      // data is magnitude only
    data  = mosaic.stack->SpaceTime.fData[index];
  else if ( mosaic.stack->data_type == 1 ) // data is real-imaginary
    {
      FCData = (FCOMPLEX *)mosaic.stack->SpaceTime.fcData;
      if ( mosaic.display_data_type == 0 )      // magnitude
	data = compute_amplitude(FCData[index]);
      else if ( mosaic.display_data_type == 1 ) // phase in degrees
	data = 180./PI * compute_phase(FCData[index]);
      else if ( mosaic.display_data_type == 2 ) // real
	data = FCData[index].real;
      else if ( mosaic.display_data_type == 3 ) // imaginary
	data = FCData[index].imag;
    }
  else if ( mosaic.stack->data_type == 2 ) // data is magnitude-phase
    {
      FCData = (FCOMPLEX *)mosaic.stack->SpaceTime.fcData;
      if ( mosaic.display_data_type == 0 )      // magnitude
	data = FCData[index].real;
      else if ( mosaic.display_data_type == 1 ) // phase
	data = FCData[index].imag;
      else if ( mosaic.display_data_type == 2 ) // real
	{
	  mag   = FCData[index].real;
	  phase = FCData[index].imag;
	  data  = mag * cos((double)phase);
	}
      else if ( mosaic.display_data_type == 3 ) // imaginary
	{
	  mag   = FCData[index].real;
	  phase = FCData[index].imag;
	  data  = mag * sin((double)phase);
	}
    }
  else if ( mosaic.stack->data_type == 3 ) // data is rgb
    data = mosaic.stack->SpaceTime.fData[index];
  else
    {
      printf("Programming error: data type = %d in function stack_data_4d\n",mosaic.stack->data_type);
      exit(1);
    }
  return(data);
}

float mosaic_data_1d(JIP_INDEX index_1d)
{
  FCOMPLEX *FCData;
  float data, mag, phase;

  if ( index_1d < 0 || index_1d >= mosaic.stack->n3d )
    {
      fprintf(stdout,"Programming error: index (%td) out of range (%td) in mosaic_data_1d\n",
	      index_1d,mosaic.stack->n3d);
      fflush(stdout);
      exit(1);
    }

  if ( mosaic.stack->data_type == 0 )      // data is magnitude only
    data = mosaic.fSpaceMatrix[index_1d];
  else if ( mosaic.stack->data_type == 1 ) // data is real-imaginary
    {
      FCData = (FCOMPLEX *)mosaic.fSpaceMatrix;
      if ( mosaic.display_data_type == 0 )      // magnitude
	data = compute_amplitude(FCData[index_1d]);
      else if ( mosaic.display_data_type == 1 ) // phase in degrees
	data = 180./PI * compute_phase(FCData[index_1d]);
      else if ( mosaic.display_data_type == 2 ) // real
	data = FCData[index_1d].real;
      else if ( mosaic.display_data_type == 3 ) // imaginary
	data = FCData[index_1d].imag;
    }
  else if ( mosaic.stack->data_type == 2 ) // data is magnitude-phase
    {
      FCData = (FCOMPLEX *)mosaic.fSpaceMatrix;
      if ( mosaic.display_data_type == 0 )      // magnitude
	data = FCData[index_1d].real;
      else if ( mosaic.display_data_type == 1 ) // phase
	data = FCData[index_1d].imag;
      else if ( mosaic.display_data_type == 2 ) // real
	{
	  mag   = FCData[index_1d].real;
	  phase = FCData[index_1d].imag;
	  data  = mag * cos((double)phase);
	}
      else if ( mosaic.display_data_type == 3 ) // imaginary
	{
	  mag   = FCData[index_1d].real;
	  phase = FCData[index_1d].imag;
	  data  = mag * sin((double)phase);
	}
    }
  else if ( mosaic.stack->data_type == 3 ) // data is rgb
    data = mosaic.stack->SpaceTime.fData[index_1d];
  else
    {
      printf("Programming error: data type = %d in function mosaic_data_1d\n",mosaic.stack->data_type);
      exit(1);
    }
  return(data);
}

float mosaic_data_3d(IPOINT3D voxel)
{
  JIP_INDEX index_1d;

  index_1d = voxel3D_to_1D(voxel);
  return(mosaic_data_1d(index_1d));
}

JIP_INDEX voxel3D_to_1D(IPOINT3D voxel)
{
  if ( voxel.x < 0 || voxel.x > mosaic.stack->dim.x-1 ||
       voxel.y < 0 || voxel.y > mosaic.stack->dim.y-1 || 
       voxel.z < 0 || voxel.z > mosaic.stack->dim.z-1 )
    {
      printf("Programming error: index out of range in voxel3D_to_1D\n");
      printf("(i,j,k) = (%d,%d,%d)\n",voxel.x,voxel.y,voxel.z);
      exit(1);
    }

  return(((JIP_INDEX)(voxel.z)) * mosaic.stack->dim.x*mosaic.stack->dim.y + ((JIP_INDEX)(voxel.y)) * mosaic.stack->dim.x + voxel.x);
}

IPOINT2D Convert_Space_to_Pixel(int which_orientation, POINT3D Space, int CenterPixel)
{
  POINT3D DataIndex;
  IPOINT2D iDraw;

  DataIndex = Convert_Space_to_rVoxel(Space, mosaic.stack);
  iDraw = Convert_rVoxel_to_Pixel(which_orientation, DataIndex, CenterPixel);
  return(iDraw);
}

IPOINT2D Convert_rVoxel_to_Pixel(int which_slice, POINT3D DataIndex, int CenterPixel)
{
  int which_orientation;
  POINT3D FlippedIndex;
  IPOINT2D iDraw;

  if ( which_slice == MOSAIC )
    which_orientation = mosaic.mosaic_orientation;
  else
    which_orientation = which_slice;

  FlippedIndex = flip_vertical_DataIndex(which_orientation,DataIndex);
  if ( which_slice == MOSAIC )
    {
      // x-y slice in mosaic mode.
      iDraw = Convert_3DMosaicIndex_to_Pixel(which_orientation, FlippedIndex, CenterPixel);
    }
  else if ( which_slice == ORTHO_XY )
    { // x-y slice
      iDraw.x = mosaic.zoom * ( mosaic.PixelResolution.x * FlippedIndex.x + NBORDER );
      iDraw.y = mosaic.zoom * ( mosaic.PixelResolution.y * FlippedIndex.y + NBORDER );
    }
  else if ( which_slice == ORTHO_ZY )
    { // z-y slice
      iDraw.x = mosaic.zoom * ( mosaic.PixelResolution.z * FlippedIndex.z +
				2*NBORDER + mosaic.PixelDim.x);
      iDraw.y = mosaic.zoom * ( mosaic.PixelResolution.y * FlippedIndex.y + NBORDER );
    }
  else if ( which_slice == ORTHO_XZ )
    { // x-z slice
      iDraw.x = mosaic.zoom * ( mosaic.PixelResolution.x * FlippedIndex.x + NBORDER );
      iDraw.y = mosaic.zoom * ( mosaic.PixelResolution.z * FlippedIndex.z +
				2*NBORDER + mosaic.PixelDim.y);
    }
  else if ( which_slice == ORTHO_YZ )
    { // y-z slice
      iDraw.x = mosaic.zoom * ( mosaic.PixelResolution.y * FlippedIndex.y +
				2*NBORDER + mosaic.PixelDim.x);
      iDraw.y = mosaic.zoom * ( mosaic.PixelResolution.z * FlippedIndex.z +
				2*NBORDER + mosaic.PixelDim.y);
    }

  if ( CenterPixel )
    {
      iDraw.x += mosaic.zoom/2;
      if ( mosaic.flip_X11_y )
	iDraw.y -= mosaic.zoom/2;
      else
	iDraw.y += mosaic.zoom/2;
    }
  return(iDraw);
}

IPOINT2D Convert_3DMosaicIndex_to_Pixel(int which_orientation, POINT3D DataIndex, int CenterPixel)
{ // This function is the inverse transformation from "define_cursor_location"
  int jSlice, iSlice_display, found, lx, ly, lxPanel, lyPanel, ix, iy;
  double round( double arg );
  IPOINT3D PixelDim, iPos;
  POINT3D DisplayIndex, PixelResolution, point;
  IPOINT2D iDraw;

  if ( which_orientation == ORTHO_XY )
    {
      DisplayIndex    = DataIndex;
      PixelResolution = mosaic.PixelResolution;
      PixelDim        = mosaic.PixelDim;
    }
  else if ( which_orientation == ORTHO_XZ )
    {
      DisplayIndex.x    = DataIndex.x;
      DisplayIndex.y    = DataIndex.z;
      DisplayIndex.z    = DataIndex.y;
      PixelResolution.x = mosaic.PixelResolution.x;
      PixelResolution.y = mosaic.PixelResolution.z;
      PixelResolution.z = mosaic.PixelResolution.y;
      PixelDim.x        = mosaic.PixelDim.x;
      PixelDim.y        = mosaic.PixelDim.z;
      PixelDim.z        = mosaic.PixelDim.y;
    }
  else if ( which_orientation == ORTHO_YZ )
    {
      DisplayIndex.x    = DataIndex.y;
      DisplayIndex.y    = DataIndex.z;
      DisplayIndex.z    = DataIndex.x;
      PixelResolution.x = mosaic.PixelResolution.y;
      PixelResolution.y = mosaic.PixelResolution.z;
      PixelResolution.z = mosaic.PixelResolution.x;
      PixelDim.x        = mosaic.PixelDim.y;
      PixelDim.y        = mosaic.PixelDim.z;
      PixelDim.z        = mosaic.PixelDim.x;
    }
  else if ( which_orientation == ORTHO_ZY )
    {
      DisplayIndex.x    = DataIndex.z;
      DisplayIndex.y    = DataIndex.y;
      DisplayIndex.z    = DataIndex.x;
      PixelResolution.x = mosaic.PixelResolution.z;
      PixelResolution.y = mosaic.PixelResolution.y;
      PixelResolution.z = mosaic.PixelResolution.x;
      PixelDim.x        = mosaic.PixelDim.z;
      PixelDim.y        = mosaic.PixelDim.y;
      PixelDim.z        = mosaic.PixelDim.x;
    }

  // Determine if the slice is shown, and then set the x and y panel numbers.
  for (jSlice=found=0, iSlice_display=-1; jSlice<mosaic.DisplayVolumeDim.z&&!found; jSlice++)
    {
      if ( mosaic.display_slice[jSlice] )
	{
	  iSlice_display++;
	  if ( fabs((double)(jSlice-DisplayIndex.z)) < mosaic.DisplayVolumeRes.z/100. ) found = 1;
	}
    }
  iPos.x = iPos.y = 0;  iPos.z = rint((double)DisplayIndex.z);

  if ( !found || !coordinates_on_slice(Convert_Voxel_to_Space(iPos,mosaic.stack)) )
    {
      point = Convert_Voxel_to_Space(iPos,mosaic.stack);
      // If the slice is not shown, return -1
      iDraw.x = iDraw.y = -1;
      return(iDraw);
    }

  if ( mosaic.display_3orthogonal )
    lxPanel = lyPanel = 0;
  else
    {
      // Find the x and y panel number.
      lxPanel = iSlice_display % mosaic.DimPanels.x;
      lyPanel = iSlice_display / mosaic.DimPanels.x;
    }
  // The x and y starting positions of the image
  lx = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(lxPanel)) * PixelDim.x + (lxPanel+1) * NBORDER ));
  ly = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(lyPanel)) * PixelDim.y + (lyPanel+1) * NBORDER ));

  // Convert the x,y positions
  ix = mosaic.zoom * PixelResolution.x * DisplayIndex.x;
  iy = mosaic.zoom * PixelResolution.y * DisplayIndex.y;

  // The position on the mosaic
  iDraw.x = lx + ix;
  iDraw.y = ly + iy;

  return(iDraw);
}

void update_XWindow_image()
{
  make_XWindow_image();

  // Set the update flags to update everything.
  mosaic.update_regions  = UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_NEIGHBORHOODS;
  mosaic.update_regions |= UPDATE_IMAGE_TEXT_LINES;
  mosaic.update_regions |= UPDATE_COLORBAR;
  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
  mosaic.update_regions |= UPDATE_INFO_AREA_IMAGE;
  mosaic.update_regions |= UPDATE_RIGHT_PANEL;
  Update_Regions();

  if ( mosaic.display_3orthogonal )
    {
      if ( mosaic.PixelScale > 1. )
	{
	  show_on_info_line(0,0,"Data are undersampled.","Pixel size / data size < 1.",
			    "Try mosaic mode with 1 or a few slices");
	}
      else
	show_on_info_line(0,0,"Navigate using mouse clicks,","OR 4 arrows + > or <",
			  "OR mouse scroll wheel.");
    }
  else
    {
      if ( mosaic.PixelScale > 1. )
	show_on_info_line(0,0,"Data are undersampled.","Pixel size / data size < 1.",
			  "Reduce slice set or try 3-plane display.");
      else if ( mosaic.DimPanels.z != mosaic.DisplayVolumeDim.z )
	show_on_info_line(0,0,"Reduced slice set is shown.","Use in/out (</>) arrows",
			  "or mouse scroll wheel to show other slices.");
    }

}

void Update_Regions()
{
  BOOL Projection, UpdateImages;

  image_scale_slope  = (WHITE_PIXEL-BLACK_PIXEL)/(image_scale_max - image_scale_min);
  image_scale_offset = WHITE_PIXEL - image_scale_slope * image_scale_max;

  if ( mosaic.update_regions&UPDATE_NEIGHBORHOODS )
    create_neighborhoods();

  // Update images if specifically requested ...
  UpdateImages  = mosaic.update_regions&UPDATE_IMAGES;
  // ... OR if the crosshairs changed and the crosshair is pixelated
  UpdateImages |= mosaic.update_regions&UPDATE_IMAGE_CROSSHAIRS && mosaic.display_crosshairs == 1;
  // ... OR if the crosshairs changed and graph are projections
  Projection = graph.type == 0 || graph.type == 2 || graph.type == 2;
  UpdateImages |= mosaic.update_regions&UPDATE_IMAGE_CROSSHAIRS &&
    ( Projection || mosaic.display_3orthogonal );
  if ( UpdateImages )
    {
      if ( mosaic.update_regions&UPDATE_COLORBAR || mosaic.update_regions&UPDATE_COLORBAR_LABELS )
	update_color_scheme();
      if ( mosaic.display_3orthogonal )
	make_orthogonal_images();
      else
	make_mosaic_images();
    }
      
  if ( mosaic.update_regions&UPDATE_COLORBAR )
    create_colorbar(); // this is needed for label only to recalculate act_scale_slope and act_scale_offset

  if ( wire_root.npts != 0 )
    draw_wire_frames();

  // refresh the display
  if ( mosaic.update_regions&UPDATE_IMAGES ||
       mosaic.update_regions&UPDATE_IMAGE_CROSSHAIRS ||
       mosaic.update_regions&UPDATE_IMAGE_TEXT_LINES )
    {
      refresh_display_image(1);
    }

  if ( mosaic.update_regions&UPDATE_COLORBAR || mosaic.update_regions&UPDATE_COLORBAR_LABELS )
    refresh_display_image(2);

  if ( mosaic.update_regions&UPDATE_SELECT_TEXT_IMAGE )
    refresh_display_image(3);

  if ( mosaic.update_regions&UPDATE_RIGHT_PANEL )
    refresh_display_image(4);

  if ( mosaic.update_regions&UPDATE_INFO_AREA_IMAGE  ||
       mosaic.update_regions&UPDATE_IMAGE_CROSSHAIRS ||
       mosaic.update_regions&UPDATE_CURSOR_IMAGE )
    show_information_text_image();

  if ( graph.update_regions&UPDATE_GRAPH )
    {
      fill_graph_vector(0);
      build_graph_display(1);
    }
  else if ( graph.update_regions&UPDATE_GRAPH_TEXT_LINES )
    build_graph_display(0);

  if ( graph.update_regions&UPDATE_SELECT_TEXT_GRAPH )
    draw_selectable_text_graph();

  if ( graph.update_regions&UPDATE_CURSOR_GRAPH )
    show_information_graph(1);
  else if ( graph.update_regions&UPDATE_INFO_AREA_GRAPH ||
	    graph.update_regions&UPDATE_SELECT_TEXT_GRAPH )
    show_information_graph(0);
}

void create_neighborhoods()
{
  // Create neighborhoods in each orthogonal plane.
  create_neighborhood(ORTHO_XY);
  create_neighborhood(ORTHO_ZY);
  create_neighborhood(ORTHO_XZ);
  create_neighborhood(ORTHO_YZ);

  // invert the neighborhoods to counter the X11 image direction (y down)
  if ( mosaic.flip_X11_y )
    {
      invert_neighborhood(ORTHO_XY);
      invert_neighborhood(ORTHO_ZY);
      invert_neighborhood(ORTHO_XZ);
      invert_neighborhood(ORTHO_YZ);
    }
}

void create_neighborhood( int which_slice )
// which_slice: could be x-y, z-y, or x-z
//
// This function should be called whenever:
// 1) the zoom factor changes,
// 2) the display orientation changes,
//
{
  // Define coordinate system over {0,R_x*(n_x-1)}, where
  //      - R_x is the resolution in dimension x relative to the un-zoomed pixel size
  //      - n_x is the number of data values in dimension x
  //      - pixel coordinates lie on half-integer values (0.5,1.5,...n-0.5)
  // Pixel coordinates: p_k = k
  // Data  coordinates: d_j = R * j, where R = relative resolution
  //                    k in {0,R*(n-1)}, j in {0,n-1}
  // Nearest neighbors: delta = min(p_k-d_j) = min[k-R*j]
  //                    => j = (k-delta)/R
  //                    jmin @ delta = 0.5,  jmax @ delta = -0.5
  IPOINT2D DimensionPixels; // defines 2D pixel dimensions  for this slice plane
  POINT2D  Resolution;      // defines 2D resolution        for this slice plane
  IPOINT2D DimensionArrays; // defines 2D data array ranges for this slice plane
  POINT2D PixelIndex, DataIndex;
  IPOINT2D Neighbor0Index, Neighbor1Index, Neighbor2Index, Neighbor3Index;
  float distance1, distance2, weight;
  int iLow, iHigh, lWidth;JIP_INDEX j, jx, jy;JIP_INDEX npixels, i2d;
  struct Neighborhood *Neighbors2D;
  static BOOL first_time=TRUE;
  int iMaxNeighbors, *OnGrid;

  iMaxNeighbors = -1;
  if ( which_slice == ORTHO_XY )
    { // x-y
      DimensionPixels.x = mosaic.PixelDim.x;
      DimensionPixels.y = mosaic.PixelDim.y;
      Resolution.x      = mosaic.PixelResolution.x;
      Resolution.y      = mosaic.PixelResolution.y;
      DimensionArrays.x = mosaic.stack->dim.x;
      DimensionArrays.y = mosaic.stack->dim.y;
      if ( !first_time )
	free((char *)mosaic.NeighborsXY);
      npixels = ((JIP_INDEX)(mosaic.zoom))*DimensionPixels.x * mosaic.zoom*DimensionPixels.y;
      mosaic.NeighborsXY = (struct Neighborhood *)malloc((size_t)jip_product(npixels,sizeof(struct Neighborhood)));
      Neighbors2D        = mosaic.NeighborsXY;
      OnGrid             = &(mosaic.OnGridXY);
    }
  else if ( which_slice == ORTHO_ZY )
    { // z-y
      DimensionPixels.x = mosaic.PixelDim.z;
      DimensionPixels.y = mosaic.PixelDim.y;
      Resolution.x      = mosaic.PixelResolution.z;
      Resolution.y      = mosaic.PixelResolution.y;
      DimensionArrays.x = mosaic.stack->dim.z;
      DimensionArrays.y = mosaic.stack->dim.y;
      if ( !first_time )
	free((char *)mosaic.NeighborsZY);
      npixels = ((JIP_INDEX)(mosaic.zoom))*DimensionPixels.x * mosaic.zoom*DimensionPixels.y;
      mosaic.NeighborsZY = (struct Neighborhood *)malloc((size_t)jip_product(npixels,sizeof(struct Neighborhood)));
      Neighbors2D        = mosaic.NeighborsZY;
      OnGrid             = &(mosaic.OnGridZY);
    }
  else if ( which_slice == ORTHO_XZ )
    { // x-z
      DimensionPixels.x = mosaic.PixelDim.x;
      DimensionPixels.y = mosaic.PixelDim.z;
      Resolution.x      = mosaic.PixelResolution.x;
      Resolution.y      = mosaic.PixelResolution.z; 
      DimensionArrays.x = mosaic.stack->dim.x;
      DimensionArrays.y = mosaic.stack->dim.z;
      if ( !first_time )
	free((char *)mosaic.NeighborsXZ);
      npixels = ((JIP_INDEX)(mosaic.zoom))*DimensionPixels.x * mosaic.zoom*DimensionPixels.y;
      mosaic.NeighborsXZ = (struct Neighborhood *)malloc((size_t)jip_product(npixels,sizeof(struct Neighborhood)));
      Neighbors2D        = mosaic.NeighborsXZ;
      OnGrid             = &(mosaic.OnGridXZ);
   }
  else if ( which_slice == ORTHO_YZ )
    { // y-z
      DimensionPixels.x = mosaic.PixelDim.y;
      DimensionPixels.y = mosaic.PixelDim.z;
      Resolution.x      = mosaic.PixelResolution.y;
      Resolution.y      = mosaic.PixelResolution.z;
      DimensionArrays.x = mosaic.stack->dim.y;
      DimensionArrays.y = mosaic.stack->dim.z;
      if ( !first_time )
	free((char *)mosaic.NeighborsYZ);
      npixels = ((JIP_INDEX)(mosaic.zoom))*DimensionPixels.x * mosaic.zoom*DimensionPixels.y;
      mosaic.NeighborsYZ = (struct Neighborhood *)malloc((size_t)jip_product(npixels,sizeof(struct Neighborhood)));
      Neighbors2D        = mosaic.NeighborsYZ;
      OnGrid             = &(mosaic.OnGridYZ);
    }
  first_time = FALSE;

  // Create the XY neighborhood
  for (jy=0; jy<((JIP_INDEX)(mosaic.zoom))*DimensionPixels.y; jy++)
    {
      for (jx=0; jx<((JIP_INDEX)(mosaic.zoom))*DimensionPixels.x; jx++)
	{
	  if ( mosaic.interpolate )
	    { // For interpolation, increment the pixel coordinates over the zoom region.
	      PixelIndex.x = (float)(jx+0.5)/(float)mosaic.zoom-0.5;
	      PixelIndex.y = (float)(jy+0.5)/(float)mosaic.zoom-0.5;
	    }
	  else
	    { // Without interpolation, put the pixel coordinates on integer values
	      PixelIndex.x = jx/mosaic.zoom;
	      PixelIndex.y = jy/mosaic.zoom;
	    }
	  DataIndex.x = PixelIndex.x / Resolution.x;
	  DataIndex.y = PixelIndex.y / Resolution.y;
	  // Define neighborhood coordinates to look like this:
	  // 2 3
	  // 0 1
	  iLow  = floor((double)DataIndex.x);
	  iHigh = ceil((double)DataIndex.x);
	  if ( iHigh <  0 ) iHigh += DimensionArrays.x;
	  if ( iLow  <  0 ) iLow  +=  DimensionArrays.x;
	  if ( iLow  >= DimensionArrays.x ) iLow  -= DimensionArrays.x;
	  if ( iHigh >= DimensionArrays.x ) iHigh -= DimensionArrays.x;
	  Neighbor0Index.x = Neighbor2Index.x = iLow;
	  Neighbor1Index.x = Neighbor3Index.x = iHigh;

	  iLow  = floor((double)DataIndex.y);
	  iHigh = ceil((double)DataIndex.y);
	  if ( iHigh <  0 ) iHigh += DimensionArrays.y;
	  if ( iLow  <  0 ) iLow  +=  DimensionArrays.y;
	  if ( iLow  >= DimensionArrays.y ) iLow  -= DimensionArrays.y;
	  if ( iHigh >= DimensionArrays.y ) iHigh -= DimensionArrays.y;
	  Neighbor0Index.y = Neighbor1Index.y = iLow;
	  Neighbor2Index.y = Neighbor3Index.y = iHigh;

	  // index into neighborhood array
	  i2d = ((JIP_INDEX)(jy)) * mosaic.zoom * DimensionPixels.x + jx;
	  lWidth = 2;  // width of Lanczos filter
	  // Reduce the neighborhood size if possible in order to speed calculations.
	  // In the following, weights are assigned using a Lanczos filter of width 2 (points).
	  // Don't confuse this with real sinc interpolation. Here, the width of the
	  // supporting region is just 1 data index, and the sinc is used to weight by
	  // 1 at distance=0 and by 0 at distance=1.
	  if ( Neighbor0Index.x == Neighbor1Index.x )
	    { // X coords are the same
	      if ( Neighbor0Index.y == Neighbor2Index.y )
		{ // All point are the same.
		  Neighbors2D[i2d].number     = 1;
		  Neighbors2D[i2d].address[0] = Neighbor0Index;
		  Neighbors2D[i2d].weight[0]  = 1.;
		}
	      else
		{ // X coords are the same; different y values.
		  Neighbors2D[i2d].number     = 2;
		  Neighbors2D[i2d].address[0] = Neighbor0Index;
		  Neighbors2D[i2d].address[1] = Neighbor2Index;
		  distance1 = fabs((double)(DataIndex.y-Neighbor0Index.y));
		  distance2 = fabs((double)(DataIndex.y-Neighbor2Index.y));
		  if ( distance1 > DimensionArrays.y/2 ) distance1 = fabs(DimensionArrays.y-distance1);
		  if ( distance2 > DimensionArrays.y/2 ) distance2 = fabs(DimensionArrays.y-distance2);
		  Neighbors2D[i2d].weight[0] = Lanczos_filter(distance1,lWidth);
		  Neighbors2D[i2d].weight[1] = Lanczos_filter(distance2,lWidth);
		}
	    }
	  else
	    { // X coords are different.
	      if ( Neighbor0Index.y == Neighbor2Index.y )
		{ // Y coords are the same; different x values.
		  Neighbors2D[i2d].number     = 2;
		  Neighbors2D[i2d].address[0] = Neighbor0Index;
		  Neighbors2D[i2d].address[1] = Neighbor1Index;
		  distance1 = fabs((double)(DataIndex.x-Neighbor0Index.x));
		  distance2 = fabs((double)(DataIndex.x-Neighbor1Index.x));
		  if ( distance1 > DimensionArrays.x/2 ) distance1 = fabs(DimensionArrays.x-distance1);
		  if ( distance2 > DimensionArrays.x/2 ) distance2 = fabs(DimensionArrays.x-distance2);
		  Neighbors2D[i2d].weight[0] = Lanczos_filter(distance1,lWidth);
		  Neighbors2D[i2d].weight[1] = Lanczos_filter(distance2,lWidth);
		}
	      else
		{ // All points are different.
		  Neighbors2D[i2d].number     = 4;
		  Neighbors2D[i2d].address[0] = Neighbor0Index;
		  Neighbors2D[i2d].address[1] = Neighbor1Index;
		  Neighbors2D[i2d].address[2] = Neighbor2Index;
		  Neighbors2D[i2d].address[3] = Neighbor3Index;

		  distance1 = fabs((double)(DataIndex.x-Neighbor0Index.x));
		  distance2 = fabs((double)(DataIndex.y-Neighbor0Index.y));
		  if ( distance1 > DimensionArrays.x/2 ) distance1 = fabs(DimensionArrays.x-distance1);
		  if ( distance2 > DimensionArrays.y/2 ) distance2 = fabs(DimensionArrays.y-distance2);
		  Neighbors2D[i2d].weight[0] = Lanczos_filter(distance1,lWidth)
		    *                          Lanczos_filter(distance2,lWidth);

		  distance1 = fabs((double)(DataIndex.x-Neighbor1Index.x));
		  distance2 = fabs((double)(DataIndex.y-Neighbor1Index.y));
		  if ( distance1 > DimensionArrays.x/2 ) distance1 = fabs(DimensionArrays.x-distance1);
		  if ( distance2 > DimensionArrays.y/2 ) distance2 = fabs(DimensionArrays.y-distance2);
		  Neighbors2D[i2d].weight[1] = Lanczos_filter(distance1,lWidth)
		    *                          Lanczos_filter(distance2,lWidth);

		  distance1 = fabs((double)(DataIndex.x-Neighbor2Index.x));
		  distance2 = fabs((double)(DataIndex.y-Neighbor2Index.y));
		  if ( distance1 > DimensionArrays.x/2 ) distance1 = fabs(DimensionArrays.x-distance1);
		  if ( distance2 > DimensionArrays.y/2 ) distance2 = fabs(DimensionArrays.y-distance2);
		  Neighbors2D[i2d].weight[2] = Lanczos_filter(distance1,lWidth)
		    *                          Lanczos_filter(distance2,lWidth);

		  distance1 = fabs((double)(DataIndex.x-Neighbor3Index.x));
		  distance2 = fabs((double)(DataIndex.y-Neighbor3Index.y));
		  if ( distance1 > DimensionArrays.x/2 ) distance1 = fabs(DimensionArrays.x-distance1);
		  if ( distance2 > DimensionArrays.y/2 ) distance2 = fabs(DimensionArrays.y-distance2);
		  Neighbors2D[i2d].weight[3] = Lanczos_filter(distance1,lWidth)
		    *                          Lanczos_filter(distance2,lWidth);
		}
	    }
	  // Make sure weights sum to 1.
	  for (j=0, weight=0.; j<Neighbors2D[i2d].number; j++)
	    weight += Neighbors2D[i2d].weight[j];
	  for (j=0; j<Neighbors2D[i2d].number; j++)
	    Neighbors2D[i2d].weight[j] /= weight;

	  if ( Neighbors2D[i2d].number > iMaxNeighbors )
	    iMaxNeighbors = Neighbors2D[i2d].number;

	} // end loop over x voxels
    } // end loop over y voxels.
  if ( iMaxNeighbors == 1 )
    *OnGrid = 1;
  else
    *OnGrid = 0;
}

void invert_neighborhood( int which_slice )
{
  IPOINT2D DimensionPixels; // defines 2D pixel dimensions  for this slice plane
  struct Neighborhood *Neighbors2D, *NeighborsSave;
  int jy_flip;JIP_INDEX jx, jy;JIP_INDEX i2d, npixels;

  if ( which_slice == ORTHO_XY )
    { // x-y
      DimensionPixels.x = mosaic.PixelDim.x;
      DimensionPixels.y = mosaic.PixelDim.y;
      Neighbors2D       = mosaic.NeighborsXY;
    }
  else if ( which_slice == ORTHO_ZY )
    { // z-y
      DimensionPixels.x = mosaic.PixelDim.z;
      DimensionPixels.y = mosaic.PixelDim.y;
      Neighbors2D       = mosaic.NeighborsZY;
    }
  else if ( which_slice == ORTHO_XZ )
    { // x-z
      DimensionPixels.x = mosaic.PixelDim.x;
      DimensionPixels.y = mosaic.PixelDim.z;
      Neighbors2D       = mosaic.NeighborsXZ;
   }
  else if ( which_slice == ORTHO_YZ )
    { // y-z
      DimensionPixels.x = mosaic.PixelDim.y;
      DimensionPixels.y = mosaic.PixelDim.z;
      Neighbors2D       = mosaic.NeighborsYZ;
   }

  // Allocate space a copy of one y line.
  npixels = ((JIP_INDEX)(mosaic.zoom))*DimensionPixels.y;
  NeighborsSave = (struct Neighborhood *)malloc((size_t)jip_product(npixels,sizeof(struct Neighborhood)));

  // Create the XY neighborhood
  for (jx=0; jx<((JIP_INDEX)(mosaic.zoom))*DimensionPixels.x; jx++)
    {
      // Make a copy of the y line of the array.
      for (jy=0; jy<((JIP_INDEX)(mosaic.zoom))*DimensionPixels.y; jy++)
	{
	  i2d = ((JIP_INDEX)(jy)) * mosaic.zoom * DimensionPixels.x + jx;
	  NeighborsSave[jy] = Neighbors2D[i2d];
	}
      // flip the matrix in y
      for (jy=0; jy<((JIP_INDEX)(mosaic.zoom))*DimensionPixels.y; jy++)
	{
	  i2d = ((JIP_INDEX)(jy)) * mosaic.zoom * DimensionPixels.x + jx;
	  jy_flip = jip_coordinate(((JIP_INDEX)(mosaic.zoom))*DimensionPixels.y - jy - 1);
	  Neighbors2D[i2d] = NeighborsSave[jy_flip];
	}
    }
  // Free the temporary array.
  free((char *)NeighborsSave);
}

void interpolate_2d( int iCross, int which_image, IPOINT2D PixelIndex,
		     float *fData, float *fMap, float *fOvl, float *fCross, float *fProj, int *ovl_color )
{
  struct Neighborhood Neighbors;
  IPOINT3D voxel;
  float pFilter;
  int which_orientation, iCrossHair, iProjection;JIP_INDEX i2d;JIP_INDEX index1d;
  int jneighbor;

  if ( which_image == MOSAIC )
    which_orientation = mosaic.mosaic_orientation;
  else
    which_orientation = which_image;

  if ( which_orientation == ORTHO_XY )      // x-y
    {
      i2d = ((JIP_INDEX)(PixelIndex.y)) * mosaic.zoom*mosaic.PixelDim.x + PixelIndex.x;
      Neighbors = mosaic.NeighborsXY[i2d];
    }
  else if ( which_orientation == ORTHO_ZY ) // z-y
    {
      i2d = ((JIP_INDEX)(PixelIndex.y)) * mosaic.zoom*mosaic.PixelDim.z + PixelIndex.x;
      Neighbors = mosaic.NeighborsZY[i2d];
    }
  else if ( which_orientation == ORTHO_XZ)  // x-z
    {
      i2d = ((JIP_INDEX)(PixelIndex.y)) * mosaic.zoom*mosaic.PixelDim.x + PixelIndex.x;
      Neighbors = mosaic.NeighborsXZ[i2d];
    }
  else if ( which_orientation == ORTHO_YZ ) // y-z
    {
      i2d = ((JIP_INDEX)(PixelIndex.y)) * mosaic.zoom*mosaic.PixelDim.y + PixelIndex.x;
      Neighbors = mosaic.NeighborsYZ[i2d];
    }

  *fData = *fMap = *fOvl = *fCross = *fProj = pFilter = 0.;
  *ovl_color = 0;
  for (jneighbor=0; jneighbor<Neighbors.number; jneighbor++)
    {
      // For this neighbor, find the index into the 3D data arrays.
      if ( which_orientation == ORTHO_XY )      // x-y
	{ // x-y
	  voxel.x = Neighbors.address[jneighbor].x;
	  voxel.y = Neighbors.address[jneighbor].y;
	  iCrossHair = (voxel.x==mosaic.icross.x) || (voxel.y==mosaic.icross.y);
	  if ( which_image == MOSAIC )
	    {
	      voxel.z = iCross; // in mosaic mode, each slice is passed
	      iCrossHair &= voxel.z==mosaic.icross.z;
	    }
	  else
	    voxel.z = mosaic.icross.z;
	}
      else if ( which_orientation == ORTHO_ZY )
	{ // z-y
	  voxel.z = Neighbors.address[jneighbor].x;
	  voxel.y = Neighbors.address[jneighbor].y;
	  iCrossHair = (voxel.z==mosaic.icross.z) || (voxel.y==mosaic.icross.y);
	  if ( which_image == MOSAIC )
	    {
	      voxel.x = iCross; // in mosaic mode, each slice is passed
	      iCrossHair &= voxel.x==mosaic.icross.x;
	    }
	  else
	    voxel.x = mosaic.icross.x;
	}
      else if ( which_orientation == ORTHO_XZ )
	{ // x-z
	  voxel.x = Neighbors.address[jneighbor].x;
	  voxel.z = Neighbors.address[jneighbor].y;
	  iCrossHair = (voxel.x==mosaic.icross.x) || (voxel.z==mosaic.icross.z);
	  if ( which_image == MOSAIC )
	    {
	      voxel.y = iCross; // in mosaic mode, each slice is passed
	      iCrossHair &= voxel.y==mosaic.icross.y;
	    }
	  else
	    voxel.y = mosaic.icross.y;
	}
      else if ( which_orientation == ORTHO_YZ )
	{ // y-z
	  voxel.y = Neighbors.address[jneighbor].x;
	  voxel.z = Neighbors.address[jneighbor].y;
	  iCrossHair = (voxel.y==mosaic.icross.y) || (voxel.z==mosaic.icross.z);
	  if ( which_image == MOSAIC )
	    {
	      voxel.x = iCross; // in mosaic mode, each slice is passed
	      iCrossHair &= voxel.x==mosaic.icross.x;
	    }
	  else
	    voxel.x = mosaic.icross.x;
	}
      
      if ( graph.type == 0 )
	{ // x projection
	  iProjection = voxel.x!=mosaic.icross.x;
	  if ( which_orientation == ORTHO_XY )
	    {
	      iProjection &= voxel.y==mosaic.icross.y;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.z==mosaic.icross.z;
	    }
	  else if ( which_orientation == ORTHO_XZ )
	    {
	      iProjection &= voxel.z==mosaic.icross.z;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.y==mosaic.icross.y;
	    }
	  else if ( which_orientation == ORTHO_YZ )
	    {
	      iProjection &= voxel.y==mosaic.icross.y;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.z==mosaic.icross.z;
	    }
	  else if ( which_orientation == ORTHO_ZY )
	    {
	      iProjection &= voxel.z==mosaic.icross.z;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.y==mosaic.icross.y;
	    }
	}
      else if ( graph.type == 1 )
	{ // y projection
	  iProjection = voxel.y!=mosaic.icross.y;
	  if ( which_orientation == ORTHO_XY )
	    {
	      iProjection &= voxel.x==mosaic.icross.x;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.z==mosaic.icross.z;
	    }
	  else if ( which_orientation == ORTHO_YZ )
	    {
	      iProjection &= voxel.z==mosaic.icross.z;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.x==mosaic.icross.x;
	    }
	  else if ( which_orientation == ORTHO_ZY )
	    {
	      iProjection &= voxel.z==mosaic.icross.z;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.x==mosaic.icross.x;
	    }
	  else if ( which_orientation == ORTHO_XZ )
	    {
	      iProjection &= voxel.x==mosaic.icross.x;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.z==mosaic.icross.z;
	    }
	}
      else if ( graph.type == 2 )
	{ // z projection
	  iProjection = voxel.z!=mosaic.icross.z;
	  if ( which_orientation == ORTHO_XY )
	    {
	      iProjection &= voxel.x==mosaic.icross.x;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.y==mosaic.icross.y;
	    }
	  if ( which_orientation == ORTHO_XZ )
	    {
	      iProjection &= voxel.x==mosaic.icross.x;
	      if ( which_image == MOSAIC )
		{
		  iProjection &= voxel.y==mosaic.icross.y;
		}
	    }
	  else if ( which_orientation == ORTHO_YZ )
	    {
	      iProjection &= voxel.y==mosaic.icross.y;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.x==mosaic.icross.x;
	    }
	  else if ( which_orientation == ORTHO_ZY )
	    {
	      iProjection &= voxel.y==mosaic.icross.y;
	      if ( which_image == MOSAIC )
		iProjection &= voxel.x==mosaic.icross.x;
	    }
	}
      else
	iProjection = 0;

      iCrossHair &= !iProjection;

      index1d = voxel3D_to_1D(voxel);
      *fData  += Neighbors.weight[jneighbor] * mosaic_data_1d(index1d);
      *fOvl   += Neighbors.weight[jneighbor] * overlay_map.weight[index1d];
      *fCross += Neighbors.weight[jneighbor] * iCrossHair;
      *fProj  += Neighbors.weight[jneighbor] * iProjection;

      if ( mosaic.activation.visible )
	{
	  *fMap += Neighbors.weight[jneighbor] * mosaic.activation.fSpaceMatrix[index1d];
	  if ( !mosaic.activation.scale_log && mosaic.activation.pFilter != NULL )
	    // linear map with preceding p-value map
	    pFilter += Neighbors.weight[jneighbor] * mosaic.activation.pFilter[index1d];
	}

      // Set the overlay color to be non-zero, if one is encountered.
      if ( overlay_map.color[index1d] != 0 )
      	*ovl_color = overlay_map.color[index1d];
    }
  if ( mosaic.activation.visible && !mosaic.activation.scale_log && mosaic.activation.pFilter != NULL )
    { // If a linear activation map is preceded by a p-value map & the p value does not meet threshold ...
      if ( fabs((double)pFilter) < mosaic.activation.thresh_log_low )
	*fMap = 0.;
    }
}

unsigned long image_pixel(float fData, float fMap, float fOvl,
			  float fCross, float fProj, int ovl_color)
{
  float trans, fMap1, sign;
  int overlay, activation;
  unsigned long gray_pixel, transparent_pixel;
  unsigned long red, green, blue, pixel, crosshair;

  // Convert from data to gray_scale.
  gray_pixel = fData * image_scale_slope + image_scale_offset;
  if ( fData < image_scale_min )  gray_pixel = BLACK_PIXEL;
  if ( mosaic.clipping_mode == 0 )
    {
      if ( fData > image_scale_max )  gray_pixel = WHITE_PIXEL;
    }
  else
    {
      if ( fData > image_scale_max )  gray_pixel = BLACK_PIXEL;
    }
  if ( gray_pixel > WHITE_PIXEL ) gray_pixel = WHITE_PIXEL;
  if ( gray_pixel < BLACK_PIXEL ) gray_pixel = BLACK_PIXEL;

  if ( mosaic.stack->data_type == 3 )
    {
      pixel = fData;
      red   = (pixel<<8 )>>16;
      green = (pixel<<16)>>24;
      blue  = (pixel<<24)>>24;
      pixel = (red<<16) + (green<<8) + blue;
      return(pixel);
    }
  
  overlay = (fOvl != 0.);

  // Set the sign of the colormap.
  if ( mosaic.activation.color_scheme == 0 )
    sign = mosaic.activation.color_sign;
  else
    sign = 1.; // cases are considered explicitly below
  fMap1 = fMap * sign;

  activation = mosaic.activation.visible;
  if ( activation )
    {
      // Show activation for colors above the lower threshold.
      if ( mosaic.activation.color_scheme == 0 )      // + & - tails
	activation &= fabs((double)(fMap1)) > get_activation_threshold(-1);
      else if ( mosaic.activation.color_scheme > 0 ) // + tail
	activation &=  fMap1 > get_activation_threshold(-1);
      else                                     // - tail
	activation &= -fMap1 > get_activation_threshold(-1);
    }
  
  if ( fProj != 0. )
    { // yellow pixels for projection lines
      trans = 1. - fProj;
      if ( trans > 1. ) trans = 1.;
      transparent_pixel = (unsigned long)(trans * gray_pixel + (1-trans)*WHITE_PIXEL);
      red  = green = transparent_pixel; // yellow
      blue = 0;
      pixel = (red<<16) + (green<<8) + blue;
    }
  else if ( overlay )
    {
      // If this is an overlay pixel, make the pixel purple (red + blue).
      if ( mosaic.transparent_overlay )
	trans = 0.85;
      else
	trans = 1. - fOvl;
      if ( trans > 1. ) trans = 1.;
      transparent_pixel = (unsigned long)(trans * gray_pixel + (1-trans)*WHITE_PIXEL);

      red = blue = green = 0;
      if ( ovl_color == 1 )
	blue  = green = transparent_pixel;       // cyan
      else if ( ovl_color == 2 )
	red  = transparent_pixel;                // red
      else if ( ovl_color == 3 )
	green = transparent_pixel;               // green
      else if ( ovl_color == 4 )
	blue = transparent_pixel;                // blue
      else if ( ovl_color == 5 )
	red  = green = blue = transparent_pixel; // white
      else if ( ovl_color == 6 )
	red  = blue = transparent_pixel;         // purple
      else if ( ovl_color == 7 )
	red  = green = transparent_pixel;        // yellow
      else
	{
	  red    = transparent_pixel;            // orange
	  green  = transparent_pixel/2;
	}
      
      // In a 24-bit display, 8 bits are assigned to red, green, and blue.
      pixel = (red<<16) + (green<<8) + blue;
    }
  else if ( activation )
    {
      if ( mosaic.activation.color_scheme == 0 )       // + & - tails
	pixel = activation_color_scheme0( fMap1, gray_pixel );
      else if ( mosaic.activation.color_scheme == 1 )  // + tail
	pixel = activation_color_scheme1( fMap1, gray_pixel );
      else if ( mosaic.activation.color_scheme == -1 ) // - tail
	pixel = activation_color_scheme1( -fMap1, gray_pixel );
      else if ( mosaic.activation.color_scheme == 2 )  // + tail
	pixel = activation_color_scheme2( fMap1, gray_pixel );
      else if ( mosaic.activation.color_scheme == -2 ) // - tail
	pixel = activation_color_scheme2( -fMap1, gray_pixel );
    }
  else
    {
      // For grayscale, make all values (RBG) the same.
      red   = gray_pixel;
      green = gray_pixel;
      blue  = gray_pixel;
      // In a 24-bit display, 8 bits are assigned to red, green, and blue.
      pixel = (red<<16) + (green<<8) + blue;
    }

  if ( mosaic.display_crosshairs == 1 && fCross != 0. )
    {
      if ( mosaic.transparent_overlay )
	trans = 0.85;
      else
	trans = 1. - fCross;
      if ( trans > 1. ) trans = 1.;
      transparent_pixel = (unsigned long)(trans * gray_pixel + (1-trans)*WHITE_PIXEL);

      //      transparent_pixel = (unsigned long)(0.75 * gray_pixel + 0.1*WHITE_PIXEL);
      red = blue = green = transparent_pixel;
      //  crosshair = (red<<16) + blue;
      crosshair = (green<<8) + blue; // cyan
      pixel = crosshair;
    }

  return(pixel);
}

unsigned long activation_color_scheme0( float value, unsigned long gray_pixel )
{
  //
  // This function uses a color scheme of
  //          red to yellow for +
  //  and     blue to green for -
  //
  int negative_sign;
  unsigned long red, green, blue, pixel, level, top_level, bottom_level;
  float yvalue;
  float trans;

  // Set the top pixel to the full yellow range plus 1/2 the red range.
  top_level = 1.5*(WHITE_PIXEL+1)-1;
  bottom_level = WHITE_PIXEL/2;
  
  negative_sign = (value < 0.);
  if ( negative_sign )
    yvalue = - value;
  else
    yvalue = value;

  level = yvalue * act_scale_slope + act_scale_offset;
  if ( yvalue > get_activation_threshold(1) || level > top_level )
    level = top_level;
  if ( yvalue < get_activation_threshold(-1) || level < BLACK_PIXEL )
    level = BLACK_PIXEL;

  if ( negative_sign )
    {
      // blue to green (red=0)
      red = 0;
      if ( level <= WHITE_PIXEL/2 )
	{ // blue: 128 to 255
	  blue  = (WHITE_PIXEL+1)/2 + level;
	  green = 0;
	}
      else
	{ // blue = 255 to 0; green: 0 to 255
	  //	  blue = WHITE_PIXEL;
	  //	  green = level - (WHITE_PIXEL+1)/2;
	  blue   = top_level - level;
	  green = level - (WHITE_PIXEL+1)/2;
	}
    }
  else
    {
      // red to yellow (blue = 0)
      blue = 0;
      if ( level <= WHITE_PIXEL/2 )
	{ // red: 128 to 255
	  red   = (WHITE_PIXEL+1)/2 + level;
	  green = 0;
	}
      else
	{ // red = 255; green: 0 to 255
	  red   = WHITE_PIXEL;
	  green = level - (WHITE_PIXEL+1)/2;
	}
    }

  if ( mosaic.activation.transparent )
    trans = 0.70;
  else
    trans = 0.;
  red   = (unsigned long)(trans * gray_pixel + (1-trans)*red);
  green = (unsigned long)(trans * gray_pixel + (1-trans)*green);
  blue  = (unsigned long)(trans * gray_pixel + (1-trans)*blue);

  pixel = (red<<16) + (green<<8) + blue;
  return(pixel);
}

unsigned long activation_color_scheme1( float value, unsigned long gray_pixel )
{
  //
  // This function uses a color scheme of
  //       blue to red to yellow for +
  // and   nothing for -
  //
  unsigned long red, green, blue, pixel, level, top_pixel;
  float trans;

  top_pixel = 2*(WHITE_PIXEL+1)-1;

  level = value * act_scale_slope + act_scale_offset;
  if ( value > get_activation_threshold(1) || level > top_pixel )
    level = top_pixel;
  if ( value < get_activation_threshold(-1) || level < BLACK_PIXEL )
    level = BLACK_PIXEL;

  if ( level <= WHITE_PIXEL )
    { // blue: 255->0; red: 0->255
      green = 0;
      blue  = WHITE_PIXEL - level;
      red   = level;
    }
  else
    { // red: 255; green: 0->255
      blue  = 0;
      red   = WHITE_PIXEL;
      green = level - WHITE_PIXEL - 1;
    }

  if ( mosaic.activation.transparent )
    trans = 0.70;
  else
    trans = 0.;
  red   = (unsigned long)(trans * gray_pixel + (1-trans)*red);
  green = (unsigned long)(trans * gray_pixel + (1-trans)*green);
  blue  = (unsigned long)(trans * gray_pixel + (1-trans)*blue);

  pixel = (red<<16) + (green<<8) + blue;
  return(pixel);
}

unsigned long activation_color_scheme2( float value, unsigned long gray_pixel )
{
  //
  // This function uses a color scheme of
  //       blue to red to yellow for +
  // and   nothing for -
  //
  unsigned long red, green, blue, pixel, level, top_pixel, white_pixel;
  float trans;

  white_pixel = WHITE_PIXEL+1;
  top_pixel = 5*white_pixel-1;

  level = value * act_scale_slope + act_scale_offset;
  if ( value > get_activation_threshold(1) || level > top_pixel )
    level = top_pixel;
  if ( value < get_activation_threshold(-1) || level < BLACK_PIXEL )
    level = BLACK_PIXEL;

  if ( level < white_pixel ) // 0 -> 255
    { // purple to blue
      green = 0;
      blue  = WHITE_PIXEL;
      red   = WHITE_PIXEL - level; // 255 ->0
    }
  else if ( level < 2*white_pixel )
    { // blue to cyan
      green = level - white_pixel; // 0 -> 255
      blue  = WHITE_PIXEL;
      red   = 0;
    }
  else if ( level < 3*white_pixel )
    { // cyan to green
      green = WHITE_PIXEL;
      blue  = 3*white_pixel - level - 1; // 255 -> 0
      red   = 0;
    }
  else if ( level < 4*white_pixel )
    { // green to yellow
      green = WHITE_PIXEL;
      blue  = 0;
      red   = level - 3*white_pixel; // 0 -> 255
    }
  else
    { // yellow to red
      blue  = 0;
      red   = WHITE_PIXEL;
      green = 5*white_pixel - level - 1; // 255 -> 0
    }
  if ( mosaic.activation.transparent )
    trans = 0.70;
  else
    trans = 0.;
  red   = (unsigned long)(trans * gray_pixel + (1-trans)*red);
  green = (unsigned long)(trans * gray_pixel + (1-trans)*green);
  blue  = (unsigned long)(trans * gray_pixel + (1-trans)*blue);

  pixel = (red<<16) + (green<<8) + blue;
  return(pixel);
}

void draw_crosshairs( int itype )
{ // itype: 0 = full, 1 = small
  POINT3D data1, data2;
  IPOINT2D iDraw1, iDraw2;

  if ( mosaic.display_3orthogonal )
    {
      ///////////////////////////////////////////////////////////////////////////////////////
      // Display crosshairs in x-y, z-y, and x-z: draw on the pixmap.
      ///////////////////////////////////////////////////////////////////////////////////////
      // First horizontal crosshairs, because the y values may need to be flipped ...
      // horizontal crosshair #1: x-y slice
      if ( itype == 0 )
	{
	  data1.x = 0.;
	  data2.x = mosaic.stack->dim.x-1;  // full range in x
	}
      else
	{
	  data1.x = MAX(0,mosaic.icross.x - 5);
	  data2.x = MIN(mosaic.stack->dim.x-1,mosaic.icross.x + 5);
	}
      data1.y = data2.y = mosaic.icross.y;
      data1.z = data2.z = mosaic.icross.z; // constant z
      iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_XY,data1,1);
      iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_XY,data2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      // horizontal crosshair #2: x-z slice
      iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_XZ,data1,1);
      iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_XZ,data2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      // horizontal crosshair #3: z-y or y-z
      if ( mosaic.display_ortho_ZY )
	{ // z-y slice
	  data1.x = data2.x = mosaic.icross.x; // constant x
	  if ( itype == 0 )
	    {
	      data1.z = 0.;
	      data2.z = mosaic.stack->dim.z-1.;       // full range in z
	    }
	  else
	    {
	      data1.z = MAX(0,mosaic.icross.z - 5);
	      data2.z = MIN(mosaic.stack->dim.z-1,mosaic.icross.z + 5);
	    }
	  data1.y = data2.y = mosaic.icross.y;
	  iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_ZY,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_ZY,data2,1);
	}
      else
	{ // y-z slice
	  data1.x = data2.x = mosaic.icross.x; // constant x
	  if ( itype == 0 )
	    {
	      data1.y = 0.;
	      data2.y = mosaic.stack->dim.y-1.;       // full range in y
	    }
	  else
	    {
	      data1.y = MAX(0,mosaic.icross.y - 5);
	      data2.y = MIN(mosaic.stack->dim.y-1,mosaic.icross.y + 5);
	    }
	  data1.z = data2.z = mosaic.icross.z;
	  iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_YZ,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_YZ,data2,1);
	}
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      
      // vertical crosshair #1: x-y slice
      data1.x = data2.x = mosaic.icross.x; // center voxel in x
      if ( itype == 0 )
	{
	  data1.y = 0.;
	  data2.y = mosaic.stack->dim.y-1.;       // full range in y
	}
      else
	{
	  data1.y = MAX(0,mosaic.icross.y - 5);
	  data2.y = MIN(mosaic.stack->dim.y-1,mosaic.icross.y + 5);
	}
      data1.z = data2.z = mosaic.icross.z;     // constant z
      iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_XY,data1,1);
      iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_XY,data2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      // vertical crosshair #2: x-z slice
      data1.y = data2.y = mosaic.icross.y;     // constant y
      data1.x = data2.x = mosaic.icross.x;
      if ( itype == 0 )
	{
	  data1.z = 0.;
	  data2.z = mosaic.stack->dim.z-1.;       // full range in z
	}
      else
	{
	  data1.z = MAX(0,mosaic.icross.z - 5);
	  data2.z = MIN(mosaic.stack->dim.z-1,mosaic.icross.z + 5);
	}
      iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_XZ,data1,1);
      iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_XZ,data2,1);
      DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
      // vertical crosshair #3: z-y or y-z
      if ( mosaic.display_ortho_ZY )
	{ // z-y slice
	  data1.x = data2.x = mosaic.icross.x;     // constant x
	  data1.z = data2.z = mosaic.icross.z;
	  if ( itype == 0 )
	    {
	      data1.y = 0.;
	      data2.y = mosaic.stack->dim.y-1.;       // full range in y
	    }
	  else
	    {
	      data1.y = MAX(0,mosaic.icross.y - 5);
	      data2.y = MIN(mosaic.stack->dim.y-1,mosaic.icross.y + 5);
	    }
	  iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_ZY,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_ZY,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
      else
	{ // y-z slice
	  data1.x = data2.x = mosaic.icross.x;     // constant x
	  data1.y = data2.y = mosaic.icross.y;
	  if ( itype == 0 )
	    {
	      data1.z = 0.;
	      data2.z = mosaic.stack->dim.z-1.;       // full range in z
	    }
	  else
	    {
	      data1.z = MAX(0,mosaic.icross.z - 5);
	      data2.z = MIN(mosaic.stack->dim.z-1,mosaic.icross.z + 5);
	    }
	  iDraw1 = Convert_rVoxel_to_Pixel(ORTHO_YZ,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(ORTHO_YZ,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
    }
  else
    { // MOSAIC display: just 1 crosshair.
      // horizontal line.
      if ( mosaic.mosaic_orientation == ORTHO_XY || mosaic.mosaic_orientation == ORTHO_XZ )
	{
	  if ( itype == 0 )
	    {
	      data1.x = 0.;
	      data2.x = mosaic.stack->dim.x-1.;       // full range in x
	    }
	  else
	    {
	      data1.x = MAX(0,mosaic.icross.x - 5);
	      data2.x = MIN(mosaic.stack->dim.x-1,mosaic.icross.x + 5);
	    }
	  data1.y = data2.y = mosaic.icross.y;  // constant y,z
	  data1.z = data2.z = mosaic.icross.z;
	  iDraw1 = Convert_rVoxel_to_Pixel(MOSAIC,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(MOSAIC,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
      else if ( mosaic.mosaic_orientation == ORTHO_ZY )
	{
	  data1.x = data2.x = mosaic.icross.x; // constant x,y
	  if ( itype == 0 )
	    {
	      data1.z = 0.;
	      data2.z = mosaic.stack->dim.z-1.;       // full range in z
	    }
	  else
	    {
	      data1.z = MAX(0,mosaic.icross.z - 5);
	      data2.z = MIN(mosaic.stack->dim.z-1,mosaic.icross.z + 5);
	    }
	  data1.y = data2.y = mosaic.icross.y;
	  iDraw1 = Convert_rVoxel_to_Pixel(MOSAIC,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(MOSAIC,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
      else if ( mosaic.mosaic_orientation == ORTHO_YZ )
	{
	  data1.x = data2.x = mosaic.icross.x; // constant x,z
	  if ( itype == 0 )
	    {
	      data1.y = 0.;
	      data2.y = mosaic.stack->dim.y-1.;       // full range in y
	    }
	  else
	    {
	      data1.y = MAX(0,mosaic.icross.y - 5);
	      data2.y = MIN(mosaic.stack->dim.y-1,mosaic.icross.y + 5);
	    }
	  data1.z = data2.z = mosaic.icross.z;
	  iDraw1 = Convert_rVoxel_to_Pixel(MOSAIC,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(MOSAIC,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
      // vertical line.
      if ( mosaic.mosaic_orientation == ORTHO_XY || mosaic.mosaic_orientation == ORTHO_ZY )
	{
	  if ( itype == 0 )
	    {
	      data1.y = 0.;
	      data2.y = mosaic.stack->dim.y-1.;       // full range in y
	    }
	  else
	    {
	      data1.y = MAX(0,mosaic.icross.y - 5);
	      data2.y = MIN(mosaic.stack->dim.y-1,mosaic.icross.y + 5);
	    }
	  data1.x = data2.x = mosaic.icross.x;    // constant x,z
	  data1.z = data2.z = mosaic.icross.z;
	  iDraw1 = Convert_rVoxel_to_Pixel(MOSAIC,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(MOSAIC,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
      else if ( mosaic.mosaic_orientation == ORTHO_XZ || mosaic.mosaic_orientation == ORTHO_YZ )
	{
	  data1.y = data2.y = mosaic.icross.y;     // constant x,y
	  data1.x = data2.x = mosaic.icross.x;
	  if ( itype == 0 )
	    {
	      data1.z = 0.;
	      data2.z = mosaic.stack->dim.z-1.;       // full range in z
	    }
	  else
	    {
	      data1.z = MAX(0,mosaic.icross.z - 5);
	      data2.z = MIN(mosaic.stack->dim.z-1,mosaic.icross.z + 5);
	    }
	  iDraw1 = Convert_rVoxel_to_Pixel(MOSAIC,data1,1);
	  iDraw2 = Convert_rVoxel_to_Pixel(MOSAIC,data2,1);
	  DPRIMITIVE_DrawLine(Drawable_ImageData,COLOR_CYAN,iDraw1,iDraw2);
	}
    }
}

void highlight_active_image_plane()
{
  static int last_cursor_position = 0;

  if ( (mosaic.cursor_selection&CURSOR_ON_ORTHO_XY && last_cursor_position==CURSOR_ON_ORTHO_XY) ||
       (mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ && last_cursor_position==CURSOR_ON_ORTHO_XZ) ||
       (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ && last_cursor_position==CURSOR_ON_ORTHO_YZ) ||
       (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY && last_cursor_position==CURSOR_ON_ORTHO_ZY) )
    return;

  if ( (mosaic.cursor_selection&CURSOR_ON_DATA) && (mosaic.cursor_selection&CURSOR_ON_ORTHO_XY) )
    {
      if ( mosaic.display_ortho_ZY )
	highlight_image_panel(1,0);  // cyan borders around ORTHO_ZY plane
      highlight_image_panel(2,0);    // cyan borders around ORTHO_XZ plane
      if ( !mosaic.display_ortho_ZY )
	highlight_image_panel(3,0);  // cyan borders around ORTHO_YZ plane
      highlight_image_panel(0,1);    // red borders around ORTHO_XY plane
      last_cursor_position = CURSOR_ON_ORTHO_XY;
    }
  else if ( (mosaic.cursor_selection&CURSOR_ON_DATA) && (mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY) )
    {
      highlight_image_panel(0,0);    // cyan borders around ORTHO_XY plane
      highlight_image_panel(2,0);    // cyan borders around ORTHO_XZ plane
      highlight_image_panel(1,1);    // red borders around ORTHO_ZY plane
      last_cursor_position = CURSOR_ON_ORTHO_ZY;
    }
  else if ( (mosaic.cursor_selection&CURSOR_ON_DATA) && (mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ) )
    {
      highlight_image_panel(0,0);    // cyan borders around ORTHO_XY plane
      if ( mosaic.display_ortho_ZY )
	highlight_image_panel(1,0);  // cyan borders around ORTHO_ZY plane
      if ( !mosaic.display_ortho_ZY )
	highlight_image_panel(3,0);  // cyan borders around ORTHO_YZ plane
      highlight_image_panel(2,1);    // red borders around ORTHO_XZ plane
      last_cursor_position = CURSOR_ON_ORTHO_XZ;
    }
  else if ( (mosaic.cursor_selection&CURSOR_ON_DATA) && (mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ) )
    {
      highlight_image_panel(0,0);    // cyan borders around ORTHO_XY plane
      highlight_image_panel(2,0);    // cyan borders around ORTHO_XZ plane
      highlight_image_panel(3,1);    // red borders around ORTHO_YZ plane
      last_cursor_position = CURSOR_ON_ORTHO_YZ;
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_WINDOW )
    {
      if ( last_cursor_position != 0 )
	{
	  highlight_image_panel(0,0);    // cyan borders around ORTHO_XY plane
	  if ( mosaic.display_ortho_ZY )
	    highlight_image_panel(1,0);  // cyan borders around ORTHO_ZY plane
	  highlight_image_panel(2,0);    // cyan borders around ORTHO_XZ plane
	  if ( !mosaic.display_ortho_ZY )
	    highlight_image_panel(3,0);  // cyan borders around ORTHO_YZ plane
	}
      last_cursor_position = 0;
    }
}

void update_borders_for_slices( int nz, int *display_slice)
{
  int lxPanel, lyPanel, jZ;
  IPOINT2D iCorner1, iCorner2, iCorner3, iCorner4;
  IPOINT3D PixelDim;

  if ( mosaic.mosaic_orientation == ORTHO_XY )
    {
      PixelDim = mosaic.PixelDim;
    }
  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
    {
      PixelDim.x = mosaic.PixelDim.x;
      PixelDim.y = mosaic.PixelDim.z;
      PixelDim.z = mosaic.PixelDim.y;
    }
  else if ( mosaic.mosaic_orientation == ORTHO_YZ )
    {
      PixelDim.x = mosaic.PixelDim.y;
      PixelDim.y = mosaic.PixelDim.z;
      PixelDim.z = mosaic.PixelDim.x;
    }
  else if ( mosaic.mosaic_orientation == ORTHO_ZY )
    {
      PixelDim.x = mosaic.PixelDim.z;
      PixelDim.y = mosaic.PixelDim.y;
      PixelDim.z = mosaic.PixelDim.x; 
    }


  // Initialize each slice by drawing a cyan border.
  for (jZ=0; jZ<nz; jZ++)
    {
      // Find the panel.
      lxPanel = jZ % mosaic.DimPanels.x;
      lyPanel = jZ / mosaic.DimPanels.x;
      // Define the 4 corners.
      iCorner1.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(lxPanel))*PixelDim.x + (lxPanel+1)*NBORDER ) - 1);
      iCorner3.x = jip_coordinate(iCorner1.x + ((JIP_INDEX)(PixelDim.x)) * mosaic.zoom + 2);
      iCorner1.y = jip_coordinate(2*INFO_HEIGHT + ((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(lyPanel))*PixelDim.y + (lyPanel+1)*NBORDER ) - 1);
      iCorner3.y = jip_coordinate(iCorner1.y + ((JIP_INDEX)(PixelDim.y)) * mosaic.zoom + 2);
      iCorner2.x = iCorner3.x;
      iCorner2.y = iCorner1.y;
      iCorner4.x = iCorner1.x;
      iCorner4.y = iCorner3.y;
      // Draw the lines: first initialize with cyan.
      DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_CYAN,iCorner1,iCorner2);
      DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_CYAN,iCorner3,iCorner4);
      DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_CYAN,iCorner1,iCorner4);
      DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_CYAN,iCorner2,iCorner3);
    }
  // For each selected slice, highlight the border.
  for (jZ=0; jZ<nz; jZ++)
    {
      // Find the panel.
      lxPanel = jZ % mosaic.DimPanels.x;
      lyPanel = jZ / mosaic.DimPanels.x;
      // Define the 4 corners.
      iCorner1.x = jip_coordinate(((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(lxPanel))*PixelDim.x + (lxPanel+1)*NBORDER ) - 1);
      iCorner3.x = jip_coordinate(iCorner1.x + ((JIP_INDEX)(PixelDim.x)) * mosaic.zoom + 2);
      iCorner1.y = jip_coordinate(2*INFO_HEIGHT + ((JIP_INDEX)(mosaic.zoom)) * ( ((JIP_INDEX)(lyPanel))*PixelDim.y + (lyPanel+1)*NBORDER ) - 1);
      iCorner3.y = jip_coordinate(iCorner1.y + ((JIP_INDEX)(PixelDim.y)) * mosaic.zoom + 2);
      iCorner2.x = iCorner3.x;
      iCorner2.y = iCorner1.y;
      iCorner4.x = iCorner1.x;
      iCorner4.y = iCorner3.y;
      // If the slice is selected, draw again using red.
      if ( display_slice[jZ] )
	{
	  DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_RED,iCorner1,iCorner2);
	  DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_RED,iCorner3,iCorner4);
	  DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_RED,iCorner1,iCorner4);
	  DPRIMITIVE_DrawLine(Drawable_ImageWindow,COLOR_RED,iCorner2,iCorner3);
	}
    }
}

void draw_wire_frames()
{
  POINT3D point_control[4], wire_draw[40], point;
  IPOINT2D iDraw1, iDraw2;
  int j;JIP_INDEX jpt;
  int color;JIP_INDEX start_wire;JIP_INDEX previous_terminated_point;
  int which_orientation;
  BOOL close_segment;

  if ( mosaic.display_3orthogonal )
    which_orientation = ORTHO_XY;
  else
    which_orientation = MOSAIC;

  close_segment = wire_root.control[wire_root.npts-1].color == 0;

  previous_terminated_point = -1;
  for ( jpt=0; jpt<wire_root.npts-1; jpt++ ) // do not count the very last point (often terminated)
    if ( wire_root.control[jpt].color == 0 ) previous_terminated_point = jpt;
  
  color = wire_root.control[0].color;
  start_wire = 0;
  for ( jpt=0; jpt<wire_root.npts; jpt++ )
    {
      if ( (jpt-start_wire)>=3 && (jpt-start_wire)%3 == 0 )
	{ // require at least 4 control points.
	  point_control[0] = wire_root.control[jpt-3].point;
	  point_control[1] = wire_root.control[jpt-2].point;
	  point_control[2] = wire_root.control[jpt-1].point;
	  point_control[3] = wire_root.control[jpt].point;
	  // Convert the 4 control points to a 10x vector of 40 points.
	  ComputeBezier( point_control, 40, wire_draw );
	  point = Convert_Space_to_rVoxel(wire_draw[0],mosaic.stack);
	  if ( coordinates_on_slice(wire_draw[0]) )
	    {
	      iDraw1 = Convert_Space_to_Pixel(which_orientation,wire_draw[0],1);
	      for (j=1; j<40; j++)
		{
		  iDraw2 = Convert_Space_to_Pixel(which_orientation,wire_draw[j],1);
		  DPRIMITIVE_DrawLine(Drawable_ImageData,color,iDraw1,iDraw2);
		  iDraw1 = iDraw2;
		}
	    }
	}
      else if ( !close_segment && jpt >= previous_terminated_point )
	{ // draw a point
	  iDraw2 = Convert_Space_to_Pixel(which_orientation,wire_root.control[jpt].point,1);
	  if ( coordinates_on_slice(wire_root.control[jpt].point) )
	    DPRIMITIVE_DrawPoint(Drawable_ImageData,2,iDraw2);
	}
      if ( wire_root.control[jpt].color == 0 )
	{
	  start_wire = jpt+1;
	  color = wire_root.control[jpt+1].color;
	  if ( jpt == wire_root.npts ) break;
	}
    }
  fflush(stdout);
}

void show_information_text_image()
{
  struct SelectableText *SelectText;
  IPOINT3D voxel;
  POINT3D space;
  char line1[LEN_STR], line2[LEN_STR], line3[LEN_STR];
  float low_value, high_value, range, yvalue, signal, fActivation, time;

  sprintf(line1,"     x      y      z      t    signal");
  time = graph.iTime * mosaic.stack->resolution_t;
  if ( mosaic.cursor_selection&CURSOR_ON_DATA )
    {
      if ( mosaic.update_regions&UPDATE_CURSOR_IMAGE )
	voxel = mosaic.icursor;
      else
	voxel = mosaic.icross;
      space = Convert_Voxel_to_Space(voxel,mosaic.stack);
      signal = mosaic_data_3d(voxel);
      sprintf(line2,"%6d %6d %6d %6d  %8g",
	      voxel.x,voxel.y,voxel.z,graph.iTime,signal);
      if ( mosaic.activation.visible )
	{
	  fActivation = mosaic.activation.fSpaceMatrix[voxel3D_to_1D(voxel)];
	  if (  mosaic.activation.scale_log )
	    {
	      if ( fActivation > 0 )
		fActivation = pow(10.,-(double)fActivation);
	      else
		fActivation = - pow(10.,(double)fActivation);
	      sprintf(line3,"%6.2f %6.2f %6.2f %6g  %8.1e",
		      space.x,space.y,space.z,time,fActivation);
	    }
	  else	
	    sprintf(line3,"%6.2f %6.2f %6.2f %6g  %8g",
		    space.x,space.y,space.z,time,fActivation);
	}
      else
	sprintf(line3,"%6.2f %6.2f %6.2f %6g  --------",
		space.x,space.y,space.z,time);
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_COLORBAR ) // colorbar
    {
      if ( mosaic.activation.visible )
	{
	  low_value  = get_activation_threshold(-1);
	  high_value = get_activation_threshold(1);
	}
      else
	{
	  low_value  = image_scale_min;
	  high_value = image_scale_max;
	}
      range = high_value - low_value;
      yvalue = high_value - range * mosaic.cursor.y/(float)(mosaic.ImageRegionPixels.y);
      sprintf(line1,"Use '?' function for      t    signal");
      sprintf(line2,"a list of options    %6d   ------",graph.iTime);
      sprintf(line3,"for modifying colorbar       %8g",yvalue);
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL ) // right text panel
    {
      sprintf(line1,"Use mouse clicks or RETURN for other panels.");
      sprintf(line2,"U/D arrows to navigate panels.");
      sprintf(line3,"L/R arrows to change values");
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_SELECT_TEXT )
    {
      sprintf(line2,"------ ------ ------ %6d   ------",graph.iTime);
      sprintf(line3,"------ ------ ------ ------   ------");
      for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
	{
	  if ( SelectText->selected )
	    {
	      if ( SelectText->visible )
		{
		  strcpy(line1,SelectText->message1);
		  strcpy(line2,SelectText->message2);
		  strcpy(line3,SelectText->message3);
		}
	    }
	}
    }
  else
    {
      voxel = mosaic.icross;
      space = Convert_Voxel_to_Space(voxel,mosaic.stack);
      signal = mosaic_data_3d(voxel);
      sprintf(line2,"%6d %6d %6d %6d  %8g",
	      voxel.x,voxel.y,voxel.z,graph.iTime,signal);
      if ( mosaic.activation.visible )
	{
	  fActivation = mosaic.activation.fSpaceMatrix[voxel3D_to_1D(voxel)];
	  if (  mosaic.activation.scale_log )
	    {
	      if ( fActivation > 0 )
		fActivation = pow(10.,-(double)fActivation);
	      else
		fActivation = - pow(10.,(double)fActivation);
	      sprintf(line3,"%6.2f %6.2f %6.2f %6g  %8.1e",
		      space.x,space.y,space.z,time,fActivation);
	    }
	  else	
	    sprintf(line3,"%6.2f %6.2f %6.2f %6g  %8g",
		    space.x,space.y,space.z,time,fActivation);
	}
      else
	sprintf(line3,"%6.2f %6.2f %6.2f %6g  --------",
		space.x,space.y,space.z,time);
      //      sprintf(other," h for help, or see PDF in ~jbm/xdisplay");
    }

  show_on_info_line(0,0,line1,line2,line3);
}

void flip_vertical_CursorIndex()
{
  // This function assumes that the cursor positions, which are indices
  // into the data array, have been defined based upon the X11 convention (+y downward).
  // This function takes top-down positions received from X11 and converts them to
  // bottom-up positions by swapping y.

  // Do nothing if the convention is not swapped.
  if ( ! mosaic.flip_X11_y )
    return;
  //  printf("flipping!!\n");

  if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XY ||
       mosaic.cursor_selection&CURSOR_ON_ORTHO_ZY )      // vertical = y
    {
      mosaic.icursor.y = mosaic.stack->dim.y - mosaic.icursor.y - 1;
      mosaic.cursor.y  = mosaic.stack->dim.y - mosaic.cursor.y;
    }
  else if ( mosaic.cursor_selection&CURSOR_ON_ORTHO_XZ ||
	    mosaic.cursor_selection&CURSOR_ON_ORTHO_YZ ) // vertical = z
    {
      mosaic.icursor.z = mosaic.stack->dim.z - mosaic.icursor.z - 1;
      mosaic.cursor.z  = mosaic.stack->dim.z - mosaic.cursor.z;
    }
}

POINT3D flip_vertical_DataIndex( int which_panel, POINT3D Voxel )
{
  POINT3D Flipped;

  Flipped = Voxel;
  // Do nothing if the convention is not swapped.
  if ( mosaic.flip_X11_y )
    {
      if ( which_panel == ORTHO_XY || // x-y
	   which_panel == ORTHO_ZY )  // z-y
	Flipped.y = mosaic.stack->dim.y - Voxel.y;
      else if ( which_panel == ORTHO_XZ || // x-z
		which_panel == ORTHO_YZ )  // y-z
	Flipped.z = mosaic.stack->dim.z - Voxel.z;
    }
  return(Flipped);
}

void read_program_init_file( char *init_file_name, char *arguments[LEN_STR], int *myargc)
{ // make common function
  FILE *file;
  char string[LEN_STR], arg[LEN_STR], *cptr;

  // Attempt to open the file.
  if ( !(file = fopen(init_file_name,"r")) )
    return;
  // get the one and only string
  if ( !(cptr=fgets(string, LEN_STR, file)) ) return;
  // the first argument should be the executable name
  if ( sscanf(cptr,"%s",arg) != 1 ) return;
  cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(arg)));
  // read the remaining arguments
  while ( sscanf(cptr,"%s",arg) == 1 )
    {
      strcpy(arguments[*myargc],arg);
      cptr = next_non_white(cptr+jip_int_count((JIP_INDEX)strlen(arg)));
      (*myargc)++;
    }
  fclose(file);
  return;
}

void add_image_file( struct file_list *file_root, char *file_name, int scale_log, int type )
{
  struct file_list *file_current, *file_last;

  if ( !strcmp(file_root->file_name,"") )
    {
      file_root->next = NULL;
      // Copy the file name into the file list.
      strcpy(file_root->file_name,file_name);
      file_root->scale_log = scale_log;
      file_root->type = type;
    }
  else
    {
      file_current = file_root;
      while ( file_current )
	{
	  file_last = file_current;
	  file_current = file_current->next;
	}
      // Allocate a new member of the list;
      if ( !(file_current = (struct file_list *)malloc(sizeof(struct file_list))) )
	{
	  printf("Error: memory problem creating a new file structure in the list.\n");
	  exit(1);
	}
      // Copy the file name into the file list.
      strcpy(file_current->file_name,file_name);
      file_current->scale_log = scale_log;
      file_current->type = type;
      file_current->next = NULL;
      file_last->next = file_current;
    }
}

void update_root_overlay( BOOL root_selected, BOOL update_text )
{
  IPOINT3D voxel;
  JIP_INDEX index;

  // Update the root overlay structure based upon settings of the
  // overlay map.
  ovl_root.npts = 0;
  for (voxel.z=0; voxel.z<mosaic.stack->dim.z; voxel.z++)
    {
      for (voxel.y=0; voxel.y<mosaic.stack->dim.y; voxel.y++)
	{
	  for (voxel.x=0; voxel.x<mosaic.stack->dim.x; voxel.x++)
	    {
	      index = voxel3D_to_1D(voxel);
	      if ( overlay_map.weight[index] != 0. )
		{
		  ovl_root.voxel[ovl_root.npts].x = voxel.x;
		  ovl_root.voxel[ovl_root.npts].y = voxel.y;
		  ovl_root.voxel[ovl_root.npts].z = voxel.z;
		  ovl_root.weight[ovl_root.npts] = overlay_map.weight[index];
		  ovl_root.npts++;
		}
	    }
	}
    }
  if ( root_selected )
    {
      // Set the root name.
      if ( ovl_root.npts == 0 )
	sprintf(ovl_root.name,"none");
      else
	sprintf(ovl_root.name,"ovl%d",overlay_map.number);
    }
  if ( update_text )
    // Display info on the image window.
    draw_selectable_text_image();
  if ( graph.type == 4 )
    graph.update_regions |= UPDATE_GRAPH;
}

void update_overlay_map()
{
  JIP_INDEX j, index;

  for (j=0; j<mosaic.stack->n3d; j++)
    {
      overlay_map.weight[j] = 0.;
      overlay_map.color[j]  = 0;
    }
  for (j=0; j<ovl_root.npts; j++)
    {
      if ( ovl_root.weight[j] != 0. )
	{
	  index = voxel3D_to_1D(ovl_root.voxel[j]);
	  overlay_map.weight[index] = ovl_root.weight[j];
	  overlay_map.color[index]  = ovl_root.color;
	}
    }
}

void fill_closed_wires()
{
  POINT3D StartPoint;
  JIP_INDEX jpt;

  if ( wire_root.npts == 0 )
    return;

  StartPoint = wire_root.control[0].point;
  for (jpt=1; jpt<wire_root.npts; jpt++)
    {
      if ( wire_root.control[jpt].color == 0 ) // termination
	{
	  if ( wire_root.control[jpt].point.x == StartPoint.x &&
	       wire_root.control[jpt].point.y == StartPoint.y &&
	       wire_root.control[jpt].point.z == StartPoint.z )
	    {
	      fill_last_closed_wire(jpt);
	      Update_Regions();
	    }
	  // Advance the point, and redefine a new start point
	  jpt++;
	  StartPoint = wire_root.control[jpt].point;
	}
    }
}

void fill_last_closed_wire( JIP_INDEX iLastPoint )
{ // called after closing a wire frame
  struct data_stack temp_stack;
  char info[LEN_STR];
  POINT3D point_end, point_control[4], wire_draw[40], *wire_loop;
  POINT3D WireMin, WireMax, voxel_start, voxel_end, pos_start, pos_end, TempMin, TempMax;
  POINT2D pos2D_start, pos2D_end, wire_start, wire_end;
  IPOINT2D iMin, iMax;
  IPOINT3D voxel, DimLow;
  BOOL inside, intersecting;
  double round( double arg );
  int ix1, ix2, iy1, iy2, iDimHigh;
  int found, number_crossings;JIP_INDEX start_wire, ilength, number_segments, index, index2d;
  JIP_INDEX jpt, j;

  if ( iLastPoint+1 > wire_root.npts )
    {
      printf("Programming error: point %td in fill_last_closed_wire > # wire points (%td)\n",
	     iLastPoint+1,wire_root.npts);
      exit(1);
    }
  else if ( wire_root.control[iLastPoint].color != 0 )
    {
      printf("Programming error: last point in wire array is not a termination.\n");
      exit(1);
    }
  // Save the last point.
  point_end = wire_root.control[iLastPoint].point;
  // Find the 1st point in the loop, which should be the same as the last point.
  for (jpt=iLastPoint-2, found=0; jpt>=0; jpt--)
    {
      if ( wire_root.control[jpt].point.x == point_end.x &&
	   wire_root.control[jpt].point.y == point_end.y &&
	   wire_root.control[jpt].point.z == point_end.z )
	{
	  found = 1;
	  start_wire = jpt;
	  break;
	}
      else if ( wire_root.control[jpt].color == 0 )
	// Oops, a wire end was found before a matched point.
	break;
    }
  if ( !found )
    {
      printf("Programming error: no closed loop in fill_last_closed_loop\n");
      exit(1);
    }

  // The number of control points
  ilength = iLastPoint+1 - start_wire;
  // The number of segments in the loop
  number_segments = (ilength - 4)/3 + 1;
  // The number of drawing points per segment
  ilength = 40*number_segments;
  wire_loop = point3d_vector(ilength);
  for ( jpt=start_wire, index=0; jpt<iLastPoint+1; jpt++ )
    {
      if ( (jpt-start_wire)>=3 && (jpt-start_wire)%3 == 0 ) // require at least 4 control points.
	{
	  point_control[0] = wire_root.control[jpt-3].point;
	  point_control[1] = wire_root.control[jpt-2].point;
	  point_control[2] = wire_root.control[jpt-1].point;
	  point_control[3] = wire_root.control[jpt].point;
	  // Convert the 4 control points to a 10x vector of 40 points.
	  ComputeBezier( point_control, 40, wire_draw );
	  for (j=0; j<40; j++)
	    {
	      wire_loop[index] = wire_draw[j];
	      index++;
	    }
	}
    }
  // Find the minimum and maximum of the drawing array.
  WireMin.z = WireMax.z =wire_loop[0].z;
  WireMin.x = WireMin.y =  1.e10;
  WireMax.x = WireMax.y = -1.e10;
  for (jpt=0; jpt<ilength; jpt++)
    {
      if ( wire_loop[jpt].x < WireMin.x ) WireMin.x = wire_loop[jpt].x;
      if ( wire_loop[jpt].y < WireMin.y ) WireMin.y = wire_loop[jpt].y;
      if ( wire_loop[jpt].x > WireMax.x ) WireMax.x = wire_loop[jpt].x;
      if ( wire_loop[jpt].y > WireMax.y ) WireMax.y = wire_loop[jpt].y;
    }

  // Create a temporary stack at high resolution.
  iDimHigh = 128;
  if ( mosaic.stack->dim.x < iDimHigh || mosaic.stack->dim.y < iDimHigh )
    {
      temp_stack.dim.x = temp_stack.dim.y = iDimHigh;
      temp_stack.dim.z = temp_stack.nt = 1;
      temp_stack.n2d   = temp_stack.n3d = temp_stack.n4d = jip_product(temp_stack.dim.x,temp_stack.dim.y);
      temp_stack.resolution.x = mosaic.stack->resolution.x *
	(float)mosaic.stack->dim.x/(float)temp_stack.dim.x;
      temp_stack.resolution.y = mosaic.stack->resolution.y *
	(float)mosaic.stack->dim.y/(float)temp_stack.dim.y;
      temp_stack.resolution.z = mosaic.stack->resolution.z;
      temp_stack.direction    = mosaic.stack->direction;
      temp_stack.origin       = mosaic.stack->origin;
      temp_stack.ijk_to_xyz   = scale_and_shift_to_mat44(temp_stack.resolution,
							 temp_stack.origin,
							 temp_stack.direction);
      temp_stack.xyz_to_ijk   = invert_mat44( temp_stack.ijk_to_xyz );
      temp_stack.data_type    = 0;
      temp_stack.SpaceTime.fData = float_vector(temp_stack.n4d);
      for (j=0; j<temp_stack.n4d; j++)
	temp_stack.SpaceTime.fData[j] = 0.;
    }
  else
    {
      temp_stack = *(mosaic.stack);  // inherit properties
      temp_stack.nt  = 1;            // one time point only
      temp_stack.n4d = jip_product(temp_stack.n3d,temp_stack.nt);
      temp_stack.SpaceTime.fData = float_vector(temp_stack.n4d);
      for (j=0; j<temp_stack.n4d; j++)
	temp_stack.SpaceTime.fData[j] = 0.;
    }

  // convert to pixels
  WireMin = Convert_Space_to_rVoxel(WireMin,&temp_stack);
  WireMax = Convert_Space_to_rVoxel(WireMax,&temp_stack);
  //
  TempMin = WireMin;
  TempMax = WireMax;
  WireMin.x = MIN(TempMin.x,TempMax.x);
  WireMax.x = MAX(TempMin.x,TempMax.x);
  WireMin.y = MIN(TempMin.y,TempMax.y);
  WireMax.y = MAX(TempMin.y,TempMax.y);

  ix1 = MAX(0,floor((double)WireMin.x)-2);
  iy1 = MAX(0,floor((double)WireMin.y)-2);
  ix2 = MIN(temp_stack.dim.x,ceil((double)WireMax.x)+2);
  iy2 = MIN(temp_stack.dim.y,ceil((double)WireMax.y)+2);
  iMin.x = MIN(ix1,ix2);
  iMax.x = MAX(ix1,ix2);
  iMin.y = MIN(iy1,iy2);
  iMax.y = MAX(iy1,iy2);

  // Scan over a rectangular area.
  // For each line in y, toggle the setting of "inside" at each crossing
  // between a voxel vector and a wire vector.
  voxel_start.z = voxel_end.z = voxel.z = 0;
  for (voxel.y=iMin.y; voxel.y<iMax.y; voxel.y++)
    {
      if ( (iMax.y-iMin.y>20) && (voxel.y-iMin.y)%((iMax.y-iMin.y)/20) == 0 )
	{
	  sprintf(info,"filling overlay: %d %c",
		  (int)(5.*(float)(voxel.y-iMin.y)/((float)(iMax.y-iMin.y)/20.)),37);
	  show_on_info_line(0,0,info,"","");
	  refresh_display_image(1);
	  //	  update_graph_data(1);
	}
      inside = FALSE;
      voxel_start.y = voxel.y;
      voxel_end.y   = voxel_start.y;
      for (voxel.x=iMin.x; voxel.x<iMax.x; voxel.x++)
	{
	  number_crossings = 0;
	  if ( inside )
	    {
	      index2d = ((JIP_INDEX)(voxel.y))*temp_stack.dim.x + voxel.x;
	      temp_stack.SpaceTime.fData[index2d] = 1.;
	    }
	  voxel_start.x = voxel.x;
	  voxel_end.x   = voxel.x + 1.;

	  pos_start   = Convert_rVoxel_to_Space(voxel_start,&temp_stack);
	  pos_end     = Convert_rVoxel_to_Space(voxel_end,&temp_stack);
	  pos2D_start.x = pos_start.x;  pos2D_end.x = pos_end.x;
	  pos2D_start.y = pos_start.y;  pos2D_end.y = pos_end.y;
	  for (jpt=1, intersecting=FALSE; jpt<ilength; jpt++)
	    {
	      wire_start.x = wire_loop[jpt-1].x;
	      wire_start.y = wire_loop[jpt-1].y;
	      wire_end.x   = wire_loop[jpt].x;
	      wire_end.y   = wire_loop[jpt].y;
	      if ( intersecting_2D_vectors(wire_start, wire_end, pos2D_start, pos2D_end) )
		{
		  intersecting = TRUE;
		  // For a complex shape like a figure-8, multiple crossings can occur
		  // on the scale of single voxels.
		  number_crossings++;
		}
	    }
	  // if ( intersecting )
	  for ( j=0; j<number_crossings; j++)
	    inside = !inside;
	}
    }
  free_point3d_vector(wire_loop);

  // down-sample the temporary stack
  if ( mosaic.stack->dim.x < iDimHigh || mosaic.stack->dim.y < iDimHigh )
    {
      DimLow = mosaic.stack->dim;
      DimLow.z = 1;
      downsample_stack( &temp_stack, DimLow );
    }
  voxel.z = round((double)(WireMin.z));
  for (voxel.y=0; voxel.y<mosaic.stack->dim.y; voxel.y++)
    {
      for (voxel.x=0; voxel.x<mosaic.stack->dim.x; voxel.x++)
	{
	  index2d = ((JIP_INDEX)(voxel.y))*temp_stack.dim.x + voxel.x;
	  if ( temp_stack.SpaceTime.fData[index2d] > 0.05 )
	    {
	      index = voxel3D_to_1D(voxel);
	      overlay_map.weight[index] = temp_stack.SpaceTime.fData[index2d];
	      overlay_map.color[index] = ovl_root.color;
	    }
	}
    }
  free_float_vector(temp_stack.SpaceTime.fData);
  update_root_overlay(1,1);
  if ( graph.type == 3 )
    graph.update_regions |= UPDATE_GRAPH;
}

BOOL intersecting_2D_vectors(POINT2D v1_start, POINT2D v1_end, POINT2D v2_start, POINT2D v2_end)
{
  ////////////////////////////////////////////////////////////////////////
  // Given two vectors v1 and v2 such that
  //    v1 = v1_end - v1_start
  //    v2 = v2_end - v2_start  ,
  // the two vectors intersect (if they do) at a common point, which is
  //    v1_start + v1_intersect * v1_unit = v2_start + v2_intersect * v2_unit
  // This vector equation provides 2 scalar equations with 2 unknowns,
  // and the vectors cross if the solution is
  //    0<=v1_intersect<=mag(v1)
  //    0<=v2_intersect<=mag(v2)
  //
  // *** This function also assumes that v2 projects along x only.
  ////////////////////////////////////////////////////////////////////////
  POINT2D v1, v2, v1_unit, v2_unit;
  float v1_length, v2_length, v1_intersect, v2_intersect;
  BOOL intersect;

  // calculate vectors as end - start
  v1 = vector_difference_2D(v1_start,v1_end); // v1_end - v1_start
  v2 = vector_difference_2D(v2_start,v2_end);
  // normalize to find unit vectors
  v1_unit = unit_vector_2D(v1);
  v2_unit = unit_vector_2D(v2);
  // If the vectors are parallel, then there is no intersection.
  if ( cross_product_2D(v2_unit,v1_unit) == 0. )
    return FALSE;
  else
    // Find the length along v1 where intersection occurs.
    v1_intersect = (cross_product_2D(v2_unit,v2_start) - cross_product_2D(v2_unit,v1_start))
      / cross_product_2D(v2_unit,v1_unit);
  // The vectors intersect if this value falls along v1.
  v1_length = sqrt((double)dot_product_2D(v1,v1));
  v2_length = sqrt((double)dot_product_2D(v2,v2));
  if ( v2_unit.x == 0. )
    { // This function assumes v2 projects along x only
      printf("Programming error: v2 - the voxel vector - should always point along x\n");
      exit(1);
    }
  v2_intersect = ( v1_start.x + v1_length*v1_unit.x - v2_start.x ) / v2_unit.x;

  // For both v1 and v2, the vector endpoints on one pass will be the start points on
  // the next pass, so use one <= and one < in the tests below so that intersections
  // are not counted 2ce.
  intersect =  (0 <= v1_intersect && v1_intersect < v1_length);
  intersect &= (0 <= v2_intersect && v2_intersect < v2_length);
  return(intersect);
}

POINT2D vector_difference_2D(POINT2D v1, POINT2D v2)
{ // assume v1 = start, v2 = end; return v2-v1
  POINT2D diff;

  diff.x = v2.x - v1.x;
  diff.y = v2.y - v1.y;
  return(diff);
}

POINT2D unit_vector_2D(POINT2D v1)
{ // return the unit vector along direction v1
  POINT2D v1_unit;
  float mag;

  mag = sqrt((double)dot_product_2D(v1,v1));
  v1_unit.x = v1.x / mag;
  v1_unit.y = v1.y / mag;
  return(v1_unit);
}

float cross_product_2D(POINT2D v1, POINT2D v2)
{ // calculate the 2-dimensional analog of a cross product (or determinant in 2D)
  return (v1.x * v2.y - v1.y * v2.x);
}

float dot_product_2D(POINT2D v1, POINT2D v2)
{
  return (v1.x * v2.x + v1.y * v2.y);
}

void ComputeBezier( POINT3D* control_point, int npts, POINT3D* curve )
     ////////////////////////////////////////////////////////////////////////
     // ComputeBezier fills an array of POINT2D structs with the curve points
     // generated from the control points control_point.
     //
     // control_point[4] is a 4 element array or control points for the Bezier
     // curve[npts] is returned
     // mode: 0 = fill all points, 1 = fill only last 1/4 of points
     ////////////////////////////////////////////////////////////////////////
{
  float ax, bx, cx, ay, by, cy, tSquared, tCubed, t;
  float dt;
  int j;

  // calculate the polynomial coefficients
  cx = 3.0 * (control_point[1].x - control_point[0].x);
  bx = 3.0 * (control_point[2].x - control_point[1].x) - cx;
  ax = control_point[3].x - control_point[0].x - cx - bx;

  cy = 3.0 * (control_point[1].y - control_point[0].y);
  by = 3.0 * (control_point[2].y - control_point[1].y) - cy;
  ay = control_point[3].y - control_point[0].y - cy - by;

  dt = 1.0 / ( npts - 1 );
  for( j = 0; j < npts; j++)
    {
      t = j * dt;  // 0 <= t <= 1
      // calculate the curve point at parameter value t
      tSquared = t * t;
      tCubed = tSquared * t;
      curve[j].x = (ax * tCubed) + (bx * tSquared) + (cx * t) + control_point[0].x;
      curve[j].y = (ay * tCubed) + (by * tSquared) + (cy * t) + control_point[0].y;
      curve[j].z = control_point[0].z;
    }
}

CONTROL_POINT *control_vector(JIP_INDEX length)
{
  CONTROL_POINT *vec;

  vec = (CONTROL_POINT *)jip_allocate(length,sizeof(CONTROL_POINT));
  if (!vec) alloc_error("Allocation error in float_vector().");
  return(vec);
}
void free_control_vector(CONTROL_POINT *vec)
{
  free((char *)vec);
}

BOOL coordinates_on_slice( POINT3D Point )
{
  POINT3D DataIndex;
  IPOINT3D OnSlice;
  struct data_stack *stack;
  float p1, p2, low, high;
  static BOOL first_time = TRUE;
  int j;

  OnSlice.x = OnSlice.y = OnSlice.z = 0;
  stack = mosaic.stack;

  p1 = stack->origin.x;
  p2 = p1 + stack->dim.x * stack->resolution.x * stack->direction.x;
  low  = MIN(p1,p2);
  high = MAX(p1,p2);
  OnSlice.x = (Point.x >= low) && (Point.x <= high);

  p1 = stack->origin.y;
  p2 = p1 + stack->dim.y * stack->resolution.y * stack->direction.y;
  low  = MIN(p1,p2);
  high = MAX(p1,p2);
  OnSlice.y = (Point.y >= low) && (Point.y <= high);

  DataIndex = Convert_Space_to_rVoxel(Point,mosaic.stack);
  if ( mosaic.display_3orthogonal )
    OnSlice.z = fabs((double)(DataIndex.z - mosaic.icross.z)) < 0.01; // 1/100th of slice spacing
  else
    {
      for (j=0; j<mosaic.DisplayVolumeDim.z; j++)
	{
	  if ( mosaic.display_slice[j] )
	    OnSlice.z = OnSlice.z || fabs((double)(DataIndex.z - j)) < 0.01; // 1/100th of slice
	}
    }
  if ( first_time && OnSlice.z && (!OnSlice.x || !OnSlice.y) )
    {
      first_time = FALSE;
      printf("Warning: a coordinate is out of range on x or y but not z.\n");
      printf("Check your coordinate system or wire frame coordinates.\n");
    }

  return(OnSlice.z);
}

void automatically_set_panels( int nslices, int *nx, int *ny)
{
  if ( nslices == 1 )
    *nx = *ny = 1;
  else if ( nslices == 2 )
    {
      *nx = 2;
      *ny = 1;
    }
  else if ( nslices == 3 )
    {
      *nx = 3;
      *ny = 1;
    }
  else if ( nslices == 4 )
    *nx = *ny = 2;
  else if ( nslices <= 6 )
    {
      *nx      = 3;
      *ny      = 2;
    }
  else if ( nslices <= 8 )
    {
      *nx = 4;
      *ny = 2;
    }
  else if ( nslices == 9 )
    *nx = *ny = 3;
  else if ( nslices == 10 )
    {
      *nx = 5;
      *ny = 2;
    }
  else if ( nslices <= 12 )
    {
      *nx = 4;
      *ny = 3;
    }
  else if ( nslices <= 15 )
    {
      *nx = 5;
      *ny = 3;
    }
  else if ( nslices == 16 )
    *nx = *ny = 4;
  else if ( nslices == 18 )
    {
      *nx = 6;
      *ny = 3;
    }
  else if ( nslices <= 20 )
    {
      *nx = 5;
      *ny = 4;
    }
  else if ( nslices <= 21 )
    {
      *nx = 7;
      *ny = 3;
    }
  else if ( nslices <= 24 )
    {
      *nx = 6;
      *ny = 4;
    }
  else if ( nslices <= 25 )
    *nx = *ny = 5;
  else if ( nslices <= 28 )
    {
      *nx = 7;
      *ny = 4;
    }
  else if ( nslices <= 30 )
    {
      *nx = 6;
      *ny = 5;
    }
  else if ( nslices <= 36 )
    *nx = *ny = 6;
  else if ( nslices <= 40 )
    {
      *nx = 8;
      *ny = 5;
    }
  else if ( nslices <= 42 )
    {
      *nx = 7;
      *ny = 6;
    }
  else if ( nslices <= 48 )
    {
      *nx = 8;
      *ny = 6;
    }
  else if ( nslices <= 49 )
    *nx = *ny = 7;
  else if ( nslices <= 56 )
    {
      *nx = 8;
      *ny = 7;
    }
  else if ( nslices <= 64 )
    *nx = *ny = 8;
  else if ( nslices <= 81 )
    *nx = *ny = 9;
  else if ( nslices <= 100 )
    *nx = *ny = 10;
  else if ( nslices <= 108 )
    {
      *nx = 12;
      *ny = 9;
    }
  else if ( nslices <= 121 )
    *nx = *ny = 11;
  else if ( nslices <= 128 )
    {
      *nx = 16;
      *ny = 8;
    }
  else if ( nslices <= 144 )
    *nx = *ny = 12;
  else if ( nslices <= 169 )
    *nx = *ny = 13;
  else if ( nslices <= 196 )
    *nx = *ny = 14;
  else if ( nslices <= 225 )
    *nx = *ny = 15;
  else
    *nx = *ny = 16;
}

void initialize_selectable_text()
{
  initialize_common_selectable_text();
  initialize_specific_selectable_text();
  update_image_selectable_text();
  update_graph_selectable_text();
}

void update_image_selectable_text()
{
  update_common_image_selectable_text();
  update_specific_image_selectable_text();
}

void update_graph_selectable_text()
{
  update_common_graph_selectable_text();
  update_specific_graph_selectable_text();
}

void initialize_common_selectable_text()
{
  ////////////////////////////////////////////////////////////////////
  // Initialize the "standard" set of text fields for the image window.
  // new_selectable_image_text(Y-loc, ID)

  // top line
  new_selectable_image_text(0,   TEXTFIELD_NEW_OVL_WIRE);   // new      wire
  new_selectable_image_text(0,   TEXTFIELD_READ_OVL_WIRE);  // read   ovl/wire
  new_selectable_image_text(0,   TEXTFIELD_WRITE_OVL_WIRE); // write  ovl/wire
  new_selectable_image_text(0,   TEXTFIELD_DEL_OVL_WIRE);   // delete ovl/wire
  new_selectable_image_text(0,   TEXTFIELD_COLOR_OVL_WIRE); // color  ovl/wire
  new_selectable_image_text(0,   TEXTFIELD_MIRROR_OVL_WIRE);// mirror  ovl/wire
  new_selectable_image_text(0,   TEXTFIELD_FILL_WIRE);      // fill wire frame
  new_selectable_image_text(0,   TEXTFIELD_OVL_STATS);      // stats  for ovl
  new_selectable_image_text(0,   TEXTFIELD_TRANSPARENT);    // trans  ovl/wire
  new_selectable_image_text(0,   TEXTFIELD_IMAGE_LEVEL);    // image level (window)
  new_selectable_image_text(0,   TEXTFIELD_IMAGE_ZOOM);     // image zoom
  new_selectable_image_text(0,   TEXTFIELD_IMAGE_INTERP);   // image interpolate
  new_selectable_image_text(0,   TEXTFIELD_IMAGE_CROSS);    // image crosshairs
  //  new_selectable_image_text(0,   TEXTFIELD_IMAGE_GRID);     // image grid
  new_selectable_image_text(0,   TEXTFIELD_IMAGE_GRAPH);    // graph (t,x,y,z)
  new_selectable_image_text(0,   TEXTFIELD_HELP);           // help
  new_selectable_image_text(0,   TEXTFIELD_QUIT);           // quit
  new_selectable_image_text(0,   TEXTFIELD_QUESTION);       // context-sensitive help
  new_selectable_image_text(0,   TEXTFIELD_POUND_SIGN);     // # slices in mosaic

  // choose display of Overlays / wire frames
  new_selectable_image_text(1,   TEXTFIELD_OVL_OR_WIRE);
  // Specific overlay or wire frame
  new_selectable_image_text(1,   TEXTFIELD_WHICH_OVL_WIRE);
  // data type
  new_selectable_image_text(1,   TEXTFIELD_DATA_TYPE);
  // mosaic or 3-plane
  new_selectable_image_text(1,   TEXTFIELD_MOSAIC_3PLANE);
  // mosaic panels or YZ/ZY
  new_selectable_image_text(1,   TEXTFIELD_XPANELS);
  // colorbar tail +
  new_selectable_image_text(1,   TEXTFIELD_COLORBAR_TAILP);
  // colorbar tail -
  new_selectable_image_text(1,   TEXTFIELD_COLORBAR_TAILN);
  // mosaic orientation
  new_selectable_image_text(3,   TEXTFIELD_MOSAIC_ORIENT);

  ////////////////////////////////////////////////////////////////////
  // Initialize the "standard" set of text fields for the graph window.
  // new_selectable_image_text(Y-loc, ID)
  // graph type
  new_selectable_graph_text(1,   TEXTFIELD_GRAPH_TYPE);
  // graph bin or smooth field
  new_selectable_graph_text(1,   TEXTFIELD_GRAPH_BIN);
  // graph smooth or bin value
  new_selectable_graph_text(1,   TEXTFIELD_GRAPH_BIN_VALUE);
  // graph units
  new_selectable_graph_text(1,   TEXTFIELD_GRAPH_UNITS);
  // graph range
  new_selectable_graph_text(1,   TEXTFIELD_GRAPH_AUTORANGE);
  // y high
  new_selectable_graph_text(1,   TEXTFIELD_GRAPH_YHIGH);
  // y low
  new_selectable_graph_text(2,   TEXTFIELD_GRAPH_YLOW);
  // histogram clipping mode
  new_selectable_graph_text(2,   TEXTFIELD_GRAPH_HIST_CLIP);
  // graph time model
  new_selectable_graph_text(2,   TEXTFIELD_GRAPH_MODEL);
}

void update_common_image_selectable_text()
{ // This function is called whenever mosaic.ImageRegionPixels.x changes.
  struct SelectableText *SelectText;
  BOOL wire_enabled;
  int istep;

  istep = MAX(15,mosaic.ImageRegionPixels.x/25);
  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    {
      // By default, enable all fields
      SelectText->visible = 1;
      SelectText->color = COLOR_WHITE;
      sprintf(SelectText->message1,"Function:"); // default value; usually replaced
      sprintf(SelectText->message2," ");         // these should always be replaced
      sprintf(SelectText->message3," ");
      if ( SelectText->FieldID == TEXTFIELD_NEW_OVL_WIRE )
	{
	  SelectText->XPosition    = 0;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  wire_enabled = (mosaic.display_3orthogonal || mosaic.mosaic_orientation==ORTHO_XY);
	  if ( wire_enabled && mosaic.activation.visible )
	    {
	      sprintf(SelectText->string," N ");  // text field
	      sprintf(SelectText->message1,"Keyboard: n/N  (wire frames on X-Y only)");
	      sprintf(SelectText->message2,"New overlay from activation (N, L button)");
	      sprintf(SelectText->message3,"New wire frame (n, R button");
	    }
	  else if ( wire_enabled )
	    {
	      sprintf(SelectText->string," N ");  // text field
	      sprintf(SelectText->message1,"Keyboard: N (wire frames on X-Y only)");
	      sprintf(SelectText->message2,"New wire frame (either button)");
	      sprintf(SelectText->message3,"use R button on image for overlay");
	    }
	  else if ( mosaic.activation.visible )
	    {
	      sprintf(SelectText->string," n ");  // text field
	      sprintf(SelectText->message1,"Keyboard: n");
	      sprintf(SelectText->message2,"New overlay based upon map:");
	      sprintf(SelectText->message3,"use L button on active voxel");
	    }
	  else if ( !wire_enabled && !mosaic.activation.visible )
	    SelectText->visible = 0;  // no activation (no n) and no wire frame (no N)
	}
      else if ( SelectText->FieldID == TEXTFIELD_READ_OVL_WIRE )  // read   ovl/wire
	{
	  SelectText->XPosition    = istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  sprintf(SelectText->string," r ");  // text field
	  sprintf(SelectText->message1,"Keyboard: r/R");
	  sprintf(SelectText->message2,"Read overlay (r) or wire (R)");
	  sprintf(SelectText->message3,"L button         or R button");
	}
      else if ( SelectText->FieldID == TEXTFIELD_WRITE_OVL_WIRE ) // write  ovl/wire
	{
	  SelectText->XPosition    = 2*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( ovl_root.npts == 0 && wire_root.npts == 0 ) // no overlay or wire frame
	    SelectText->visible = 0;
	  else if ( wire_root.npts == 0 ) // only overlay exists
	    {
	      sprintf(SelectText->string," w ");  // text field
	      sprintf(SelectText->message1,"Keyboard: w");
	      sprintf(SelectText->message2,"Write overlay");
	      sprintf(SelectText->message3,"L button");
	    }
	  else if (ovl_root.npts == 0 )   // only wire frame exists
	    {
	      sprintf(SelectText->string," W ");  // text field
	      sprintf(SelectText->message1,"Keyboard: W");
	      sprintf(SelectText->message2,"Write wire frame");
	      sprintf(SelectText->message3,"R button");
	    }
	  else
	    { // both overlay and wire exist
	      sprintf(SelectText->string," w ");  // text field
	      sprintf(SelectText->message1,"Keyboard: w/W");
	      sprintf(SelectText->message2,"Write overlay (w) or wire (W)");
	      sprintf(SelectText->message3,"L button          or R button");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_DEL_OVL_WIRE )   // delete ovl/wire
	{
	  SelectText->XPosition    = 3*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( ovl_root.npts == 0 && wire_root.npts == 0 ) // no overlay or wire frame
	    SelectText->visible = 0;
	  else if ( wire_root.npts == 0 ) // only overlay exists
	    {
	      sprintf(SelectText->string," d ");  // text field
	      sprintf(SelectText->message1,"Keyboard: d");
	      sprintf(SelectText->message2,"Delete overlay");
	      sprintf(SelectText->message3,"L button");
	    }
	  else if (ovl_root.npts == 0 )   // only wire frame exists
	    {
	      sprintf(SelectText->string," D ");  // text field
	      sprintf(SelectText->message1,"Keyboard: D");
	      sprintf(SelectText->message2,"Delete wire frame");
	      sprintf(SelectText->message3,"R button");
	    }
	  else
	    { // both overlay and wire exist
	      sprintf(SelectText->string," d ");  // text field
	      sprintf(SelectText->message1,"Keyboard: d/D");
	      sprintf(SelectText->message2,"Delete overlay (d) or wire (D)");
	      sprintf(SelectText->message3,"L button           or R button");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_COLOR_OVL_WIRE ) // color  ovl/wire
	{
	  SelectText->XPosition    = 4*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( ovl_root.npts == 0 && wire_root.npts == 0 ) // no overlay or wire frame
	    SelectText->visible = 0;
	  else if ( wire_root.npts == 0 ) // only overlay exists
	    {
	      sprintf(SelectText->string," c ");  // text field
	      sprintf(SelectText->message1,"Keyboard: c");
	      sprintf(SelectText->message2,"Color overlay");
	      sprintf(SelectText->message3,"L button");
	    }
	  else if (ovl_root.npts == 0 )   // only wire frame exists
	    {
	      sprintf(SelectText->string," C ");  // text field
	      sprintf(SelectText->message1,"Keyboard: C");
	      sprintf(SelectText->message2,"Color wire frame");
	      sprintf(SelectText->message3,"R button");
	    }
	  else
	    { // both overlay and wire exist
	      sprintf(SelectText->message1,"Keyboard: c/D");
	      sprintf(SelectText->message2,"Color overlay (c) or wire (C)");
	      sprintf(SelectText->message3,"L button          or R button");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_MIRROR_OVL_WIRE ) // mirror ovl/wire
	{
	  SelectText->XPosition    = 5*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( ovl_root.npts == 0 && wire_root.npts == 0 ) // no overlay or wire frame
	    SelectText->visible = 0;
	  else if ( wire_root.npts == 0 ) // only overlay exists
	    {
	      sprintf(SelectText->string," m ");  // text field
	      sprintf(SelectText->message1,"Keyboard: m");
	      sprintf(SelectText->message2,"Mirror overlay");
	      sprintf(SelectText->message3,"L button");
	    }
	  else if (ovl_root.npts == 0 )   // only wire frame exists
	    {
	      sprintf(SelectText->string," M ");  // text field
	      sprintf(SelectText->message1,"Keyboard: M");
	      sprintf(SelectText->message2,"Mirror wire frame");
	      sprintf(SelectText->message3,"R button");
	    }
	  else
	    { // both overlay and wire exist
	      sprintf(SelectText->string," m ");  // text field
	      sprintf(SelectText->message1,"Keyboard: m/M");
	      sprintf(SelectText->message2,"Mirror overlay (m) or wire (M)");
	      sprintf(SelectText->message3,"L button           or R button");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_FILL_WIRE ) // fill wire frame
	{
	  SelectText->XPosition    = 6*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( wire_root.npts == 0 ) // no wire frame
	    SelectText->visible = 0;
	  else
	    {
	      sprintf(SelectText->string," F ");  // text field
	      sprintf(SelectText->message1,"Keyboard: f/F");
	      sprintf(SelectText->message2,"Fill closed wire frames");
	      sprintf(SelectText->message3,"L/R button");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_OVL_STATS )      // stats  ovl
	{
	  SelectText->XPosition    = 7*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  sprintf(SelectText->string," S ");  // text field
	  sprintf(SelectText->message1,"Keyboard: s/S");
	  sprintf(SelectText->message2,"Stats for 1 (s) or all (S) time points");
	  sprintf(SelectText->message3,"L button        or R button");
	  if (ovl_root.npts == 0 )   // no overlay
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_TRANSPARENT ) // trans  ovl/wire
	{
	  SelectText->XPosition    = 8*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  sprintf(SelectText->string," T ");  // text field
	  sprintf(SelectText->message1,"Keyboard: t/T");
	  sprintf(SelectText->message2,"Transparent overlays");
	  sprintf(SelectText->message3,"either button");
	  if ( ovl_root.npts == 0 && !mosaic.activation.visible ) // no overlay
	    SelectText->visible = 0;
	}
      // leave room for ROI field
      else if ( SelectText->FieldID == TEXTFIELD_IMAGE_LEVEL )    // image level (window )
	{
	  SelectText->XPosition    = 11*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_WHITE;  // group display field by color
	  sprintf(SelectText->string," i ");  // text field
	  sprintf(SelectText->message1,"Keyboard: i");
	  sprintf(SelectText->message2,"image contrast & brightness (auto)");
	  sprintf(SelectText->message3,"use alone or with overlay");
	  if ( mosaic.ImageRegionPixels.x < 200 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_IMAGE_ZOOM )     // image zoom
	{
	  SelectText->XPosition    = 12*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_WHITE;  // group display field by color
	  sprintf(SelectText->string," z ");  // text field
	  sprintf(SelectText->message1,"Keyboard: z/Z");
	  sprintf(SelectText->message2,"decrease (z) or increase (Z) zoom");
	  sprintf(SelectText->message3,"L button     or R button");
	  if ( mosaic.ImageRegionPixels.x < 200 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_IMAGE_CROSS )    // image crosshairs
	{
	  SelectText->XPosition    = 13*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_WHITE;  // group display field by color
	  sprintf(SelectText->string," + ");  // text field
	  sprintf(SelectText->message1,"Keyboard: ^c");
	  sprintf(SelectText->message2,"change cross-hair display");
	  sprintf(SelectText->message3,"line, cross, pixelated, none");
	  if ( mosaic.ImageRegionPixels.x < 200 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_IMAGE_INTERP )   // image interpolate
	{
	  SelectText->XPosition    = 14*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_WHITE;  // group display field by color
	  sprintf(SelectText->string," I ");  // text field
	  sprintf(SelectText->message1,"Keyboard: tab or ^i");
	  sprintf(SelectText->message2,"Interpolate across zoom factor");
	  sprintf(SelectText->message3,"image, activation, overlays, crosshairs");
	  if ( mosaic.zoom == 1 || mosaic.ImageRegionPixels.x < 200 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_IMAGE_GRAPH )     // graph (x,y,z,t)
	{
	  SelectText->XPosition    = 16*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_ORANGE;  // group display field by color
	  sprintf(SelectText->string," g ");  // text field
	  sprintf(SelectText->message1,"Keyboard: g or G");
	  sprintf(SelectText->message2,"Bring up graph window");
	  sprintf(SelectText->message3,"x,y,z,t graphs");
	  //	  if ( mosaic.ImageRegionPixels.x < 250 )
	  SelectText->visible = 1;
	}
      else if ( SelectText->FieldID == TEXTFIELD_HELP )           // help
	{
	  SelectText->XPosition    = 17*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_YELLOW;  // group help by color
	  sprintf(SelectText->string," h ");  // text field
	  sprintf(SelectText->message1,"Keyboard: h/H");
	  sprintf(SelectText->message2,"Help me! short (h) or long (H) forms");
	  sprintf(SelectText->message3,"output goes to shell window");
	  if ( mosaic.ImageRegionPixels.x < 325 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_QUIT )           // quit
	{
	  SelectText->XPosition    = 18*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  sprintf(SelectText->string," q ");  // text field
	  sprintf(SelectText->message1,"Keyboard: q");
	  sprintf(SelectText->message2,"Quit");
	  if ( mosaic.ImageRegionPixels.x < 325 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_QUESTION )           // context help
	{
	  SelectText->XPosition    = 19*istep;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + istep-1;
	  SelectText->color        = COLOR_YELLOW;  // group help by color
	  sprintf(SelectText->string," ? ");  // text field
	  sprintf(SelectText->message1,"Keyboard: ? or /");
	  sprintf(SelectText->message2,"context-sensitive help");
	  sprintf(SelectText->message3,"click here & then on something else!");
	  if ( mosaic.ImageRegionPixels.x < 325 )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_POUND_SIGN )         // # in mosaic
	{
	  if ( mosaic.display_3orthogonal )
	    {
	      SelectText->XPosition    = mosaic.ImageRegionPixels.x - 20;
	      SelectText->XSelectStart = SelectText->XPosition;
	      SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x;
	      SelectText->color        = COLOR_GREEN;  // group help by color
	      sprintf(SelectText->string," # ");
	      sprintf(SelectText->message1,"Keyboard: #");
	      sprintf(SelectText->message2,"# panels in mosaic");
	      sprintf(SelectText->message3,"Input number; switch to mosaic");
	    }
	  else
	    {
	      SelectText->XPosition    = mosaic.ImageRegionPixels.x - 90;
	      SelectText->XSelectStart = SelectText->XPosition;
	      SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x;
	      SelectText->color        = COLOR_GREEN;  // group help by color
	      sprintf(SelectText->string," # (%d of %d)",mosaic.DimPanels.z,mosaic.DisplayVolumeDim.z);
	      sprintf(SelectText->message1,"Keyboard: #");
	      sprintf(SelectText->message2,"# panels in mosaic");
	      sprintf(SelectText->message3,"Set explicitly or select with mouse");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_OVL_OR_WIRE )
	{
	  SelectText->XPosition    = 0;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( mosaic.ImageRegionPixels.x < 150 )
	    SelectText->visible = 0;
	  // text:
	  if ( mosaic.toggle_overlays == 1 )
	    {
	      sprintf(SelectText->string,"overlay:");          // text field
	      sprintf(SelectText->message2,"switch to wires");
	      sprintf(SelectText->message3,"(if loaded)");
	    }
	  else if ( mosaic.toggle_overlays == 2 )
	    {
	      sprintf(SelectText->string," wire  :");
	      sprintf(SelectText->message2,"switch to overlays");
	      sprintf(SelectText->message3,"(if loaded)");
	    }
	  else
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_WHICH_OVL_WIRE )
	{
	  SelectText->XPosition    = 60;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
	  SelectText->color        = COLOR_CYAN;  // group overlay/wire by color
	  if ( mosaic.ImageRegionPixels.x < 150 )
	    SelectText->visible = 0;
	  // text:
	  if ( mosaic.toggle_overlays == 1 )
	    {
	      sprintf(SelectText->string," %s",ovl_root.name);
	      sprintf(SelectText->message2,"next or previous overlay");
	    }
	  else if ( mosaic.toggle_overlays == 2 )
	    {
	      sprintf(SelectText->string," %s",wire_root.name);
	      sprintf(SelectText->message2,"next or previous wire frame");
	    }
	  else
	    SelectText->visible = 0;
	  sprintf(SelectText->message3,"use mouse/UD arrows or LR arrows");
	}
      else if ( SelectText->FieldID == TEXTFIELD_DATA_TYPE )
	{
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x/2;
	  SelectText->XSelectStart = mosaic.ImageRegionPixels.x/2-50;
	  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x/2+50;
	  SelectText->color        = COLOR_GRAY;  // group display field by color
	  if ( mosaic.ImageRegionPixels.x < 150 )
	    SelectText->visible = 0;
	  // text:
	  if ( mosaic.display_data_type == 0 )
	    {
	      sprintf(SelectText->message2,"magnitude, phase");
	      sprintf(SelectText->message3,"real, imaginary");
	      strcpy(SelectText->string,"mag. ");
	      if ( mosaic.stack->data_type == 0 )
		SelectText->visible = 0; // don't show field if mag. is the only option
	    }
	  else if ( mosaic.display_data_type == 1 )
	    strcpy(SelectText->string,"phase");
	  else if ( mosaic.display_data_type == 2 )
	    strcpy(SelectText->string,"real ");
	  else if ( mosaic.display_data_type == 3 )
	    strcpy(SelectText->string,"imag ");
	}
      else if ( SelectText->FieldID == TEXTFIELD_MOSAIC_3PLANE )
	{
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x-90;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x-50;
	  SelectText->color        = COLOR_GREEN;  // group display field by color
	  sprintf(SelectText->message1,"keyboard: 3"); // default value; usually replaced
	  sprintf(SelectText->message2,"change display mode");
	  sprintf(SelectText->message3,"mosaic <--> 3-plane");
	  // text:
	  if ( mosaic.display_3orthogonal )
	    { // 3-plane display
	      if ( mosaic.text_selection == TEXTFIELD_MOSAIC_3PLANE )
		strcpy(SelectText->string,"to mosaic");
	      else
		strcpy(SelectText->string,"3-plane:");
	    }
	  else
	    { // mosaic display
	      if ( mosaic.text_selection == TEXTFIELD_MOSAIC_3PLANE )
		strcpy(SelectText->string,"to 3-plane");
	      else
		strcpy(SelectText->string,"mosaic: ");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_XPANELS )
	{
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x-45;
	  SelectText->XSelectStart = mosaic.ImageRegionPixels.x-49;
	  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x;
	  SelectText->color        = COLOR_GREEN;  // group display field by color
	  // text:
	  if ( mosaic.display_3orthogonal )
	    {
	      sprintf(SelectText->message2,"switch orientation");
	      sprintf(SelectText->message3,"of right panel");
	      if ( mosaic.text_selection == TEXTFIELD_XPANELS )
		{
		  if ( mosaic.display_ortho_ZY ) // currently y versus z
		    strcpy(SelectText->string," Z vs Y"); // to z versus y
		  else
		    strcpy(SelectText->string," Y vs Z");
		}
	      else
		{
		  if ( mosaic.display_ortho_ZY ) // currently y verus z
		    strcpy(SelectText->string," Y vs Z");
		  else
		    strcpy(SelectText->string," Z vs Y");
		}
	    }
	  else
	    {
	      sprintf(SelectText->string," %d x %d",mosaic.DimPanels.x,mosaic.DimPanels.y);
	      sprintf(SelectText->message2,"buttons L & R differently");
	      sprintf(SelectText->message3,"change mosaic display");
	    }

	  if ( mosaic.text_selection == TEXTFIELD_MOSAIC_3PLANE )
	    SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == TEXTFIELD_MOSAIC_ORIENT )
	{
	  // start position is wrt bottom menu, but select position is wrt image region
	  SelectText->XPosition    = lPixelsX_BottomMenu - mosaic.nx_ColorBar- mosaic.nx_RightPanel - 90;
	  SelectText->XSelectStart = mosaic.ImageRegionPixels.x - 90;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 90;
	  SelectText->color        = COLOR_GREEN;  // group display field by color
	  sprintf(SelectText->message1,"keyboard: none"); // default value; usually replaced
	  sprintf(SelectText->message2,"change mosaic orientation: X-Y, X-Z, Y-Z or Z-Y");
	  if ( mosaic.display_3orthogonal )
	    SelectText->visible = 0;
	  // text:
	  if ( mosaic.mosaic_orientation == ORTHO_XY )
	    { // 3-plane display
	      if ( mosaic.text_selection == TEXTFIELD_MOSAIC_ORIENT )
		{
		  if ( mosaic.display_ortho_ZY )
		    {
		      strcpy(SelectText->message3,"L: X-Z, R: Z-Y");
		      strcpy(SelectText->string,  " X-Z  |  Z-Y ");
		    }
		  else
		    {
		      strcpy(SelectText->message3,"L: X-Z, R: Y-Z");
		      strcpy(SelectText->string,  " X-Z  |  Y-Z ");
		    }
		}
	      else
		strcpy(SelectText->string,"orient: X-Y");
	    }
	  else if ( mosaic.mosaic_orientation == ORTHO_XZ )
	    { // 3-plane display
	      if ( mosaic.text_selection == TEXTFIELD_MOSAIC_ORIENT )
		{
		  if ( mosaic.display_ortho_ZY )
		    {
		      strcpy(SelectText->message3,"L: Z=Y, R: X-Y");
		      strcpy(SelectText->string,  " Z-Y  |  X-Y ");
		    }
		  else
		    {
		      strcpy(SelectText->message3,"L: Y-Z, R: X-Y");
		      strcpy(SelectText->string,  " Y-Z  |  X-Y ");
		    }
		}
	      else
		{
		  strcpy(SelectText->string,"orient: X-Z");
		}
	    }
	  else if ( mosaic.mosaic_orientation == ORTHO_YZ || mosaic.mosaic_orientation == ORTHO_ZY )
	    { // 3-plane display
	      if ( mosaic.text_selection == TEXTFIELD_MOSAIC_ORIENT )
		strcpy(SelectText->string,"L: X-Y, R: X-Z");
	      else
		{
		  if ( mosaic.display_ortho_ZY )
		    strcpy(SelectText->string,"orient: Z-Y");
		  else
		    strcpy(SelectText->string,"orient: Y-Z");
		}
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_COLORBAR_TAILP )
	{
	  sprintf(SelectText->string," + ");
	  if ( mosaic.activation.color_scheme < 0 ) // - tail only
	    SelectText->visible = 0;
	  else
	    SelectText->visible = 1;
	  if ( mosaic.activation.nmaps == 0 ) SelectText->visible = 0;
	  if ( mosaic.activation.color_scheme == 0 ) // 2 tails
	    {
	      sprintf(SelectText->message2,"button L: select + sign");
	      sprintf(SelectText->message3,"button R: switch   signs");
	      if ( mosaic.activation.color_sign == 1 )
		{ // right side
		  SelectText->XPosition    = mosaic.ImageRegionPixels.x + 3*mosaic.nx_ColorBar/5;
		  SelectText->XSelectStart = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/2;
		  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
		}
	      else
		{ // left side
		  SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/5;
		  SelectText->XSelectStart = mosaic.ImageRegionPixels.x;
		  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/2;
		}
	    }
	  else if ( mosaic.activation.color_scheme > 0 ) // + tail only
	    { // middle
	      sprintf(SelectText->message2,"button L: back to +- signs");
	      sprintf(SelectText->message3,"button R: switch to - sign");
	      SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/2;
	      SelectText->XSelectStart = mosaic.ImageRegionPixels.x;
	      SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_COLORBAR_TAILN )
	{
	  sprintf(SelectText->string," - ");
	  if ( mosaic.activation.color_scheme > 0 ) // + tail only
	    SelectText->visible = 0;
	  else
	    SelectText->visible = 1;
	  if ( mosaic.activation.nmaps == 0 ) SelectText->visible = 0;
	  if ( mosaic.activation.color_scheme == 0 ) // 2 tails
	    {
	      sprintf(SelectText->message2,"button L: select - sign");
	      sprintf(SelectText->message3,"button R: switch   signs");
	      if ( mosaic.activation.color_sign == 1 )
		{ // left side
		  SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/5;
		  SelectText->XSelectStart = mosaic.ImageRegionPixels.x;
		  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/2;
		}
	      else
		{ // right side
		  SelectText->XPosition    = mosaic.ImageRegionPixels.x + 3*mosaic.nx_ColorBar/5;
		  SelectText->XSelectStart = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/2;
		  SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
		}
	    }
	  else if ( mosaic.activation.color_scheme < 0 ) // - tail only
	    { // middle
	      sprintf(SelectText->message2,"button L: back to +- signs");
	      sprintf(SelectText->message3,"button R: switch to + sign");
	      SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar/2;
	      SelectText->XSelectStart = mosaic.ImageRegionPixels.x;
	      SelectText->XSelectEnd   = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
	    }
	}
    }
}

void update_common_graph_selectable_text()
{ // This function is called whenever mosaic.ImageRegionPixels.x changes.
  struct SelectableText *SelectText;

  for (SelectText = &(graph.SelectText); SelectText; SelectText=SelectText->next)
    {
      // By default, enable all fields
      SelectText->visible = 1;
      SelectText->color = COLOR_WHITE;
      if ( SelectText->FieldID == TEXTFIELD_GRAPH_TYPE )
	{
	  SelectText->color = COLOR_YELLOW;
	  SelectText->XPosition    = 0;
	  SelectText->XSelectStart = 0;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 80;
	  // text:
	  if ( graph.type == 0)
	    strcpy(SelectText->string,"graph: x");
	  else if ( graph.type == 1)
	    strcpy(SelectText->string,"graph: y");
	  else if ( graph.type == 2)
	    strcpy(SelectText->string,"graph: z");
	  else if ( graph.type == 3)
	    strcpy(SelectText->string,"graph: time");
	  else if ( graph.type == 4)
	    strcpy(SelectText->string,"graph: histogram");
	  sprintf(SelectText->message1,"Switch graph types:");
	  sprintf(SelectText->message2,"x, y, z, time, histogram");
	  sprintf(SelectText->message3," ");
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_BIN )
	{
	  SelectText->XPosition    = graph.nwidth*0.25;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 30;
	  if ( graph.type == 0 ||   // x graph
	       graph.type == 1 ||   // y graph
	       graph.type == 2 ||   // z graph
	       (graph.type == 3 && graph.time_model == 2 ) || // REM time model (value, error, ...)
	       graph.nwidth < 150 ) // width too small
	    SelectText->visible = 0;
	  if ( graph.type == 4 ) // histogram
	    {
	      strcpy(SelectText->string,"bins  : ");
	      sprintf(SelectText->message1," ");
	      sprintf(SelectText->message2," ");
	      sprintf(SelectText->message3," ");
	    }
	  else
	    {
	      // text:
	      if ( graph.bin_data )
		strcpy(SelectText->string,"bin   : ");
	      else
		strcpy(SelectText->string,"smooth: ");
	      sprintf(SelectText->message1,"Bin or smooth data and fit");
	      sprintf(SelectText->message2,"for viewing noisy data.");
	      sprintf(SelectText->message3,"Use mouse clicks.");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_BIN_VALUE )
	{
	  SelectText->XPosition    = graph.nwidth*0.25 + 40;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 80;
	  if ( graph.type == 0 ||   // x graph
	       graph.type == 1 ||   // y graph
	       graph.type == 2 ||   // z graph
	       (graph.type == 3 && graph.time_model == 2 ) || // REM time model (value, error, ...)
	       graph.nwidth < 150 ) // width too small
	    SelectText->visible = 0;
	  if ( graph.type == 3 ) // time
	    {
	      // text:
	      sprintf(SelectText->string,"%4d",graph.bin_width);
	      sprintf(SelectText->message1,"Increase or decrease bin value.");
	      sprintf(SelectText->message2,"Use mouse clicks");
	      sprintf(SelectText->message3,"or use L/R arrows");
	    }
	  else if ( graph.type == 4 )
	    {
	      sprintf(SelectText->string,"%4d",graph.histogram_bins);
	      sprintf(SelectText->message1,"Increase or decrease # histogram bins.");
	      sprintf(SelectText->message2,"Use mouse clicks");
	      sprintf(SelectText->message3,"or use L/R arrows");
	    }
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_UNITS )
	{
	  SelectText->XPosition    = graph.nwidth*0.45;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 80;
	  if ( graph.type == 0 ||   // x graph
	       graph.type == 1 ||   // y graph
	       graph.type == 2 ||   // z graph
	       graph.type == 4 ||   // histogram
	       (graph.type == 3 && graph.time_model == 2 ) || // REM time model (value, error, ...)
	       graph.nwidth < 150 ) // width too small
	    SelectText->visible = 0;
	  // text:
	  if ( graph.units_percent )
	    sprintf(SelectText->string," units: %c ",37);
	  else
	    sprintf(SelectText->string," units: abs ");
	  sprintf(SelectText->message1,"Toggle between %c or abs units",37);
	  sprintf(SelectText->message2,"using mouse clicks");
	  sprintf(SelectText->message3," ");
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_AUTORANGE )
	{
	  SelectText->XPosition    = graph.nwidth - 250;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 100;
	  // text:
	  if ( graph.yrange_fixed )
	    sprintf(SelectText->string," range: fixed ");
	  else
	    sprintf(SelectText->string," range: variable ");
	  sprintf(SelectText->message1,"Auto-scale or fix graph range.");
	  sprintf(SelectText->message2,"(Use up/down arrows near up/down");
	  sprintf(SelectText->message3," borders to change limits)");
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_YLOW )
	{
	  SelectText->XPosition    = graph.nwidth - 50;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
	  // text:
	  sprintf(SelectText->string," %g",graph.ymin);
	  sprintf(SelectText->message1,"Lower graph graph range:");
	  sprintf(SelectText->message2,"Use up/down arrows by low border,");
	  sprintf(SelectText->message3,"or select to input range.");
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_YHIGH )
	{
	  SelectText->XPosition    = graph.nwidth - 50;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
	  // text:
	  sprintf(SelectText->string," %g",graph.ymax);
	  sprintf(SelectText->message1,"Upper graph graph range:");
	  sprintf(SelectText->message2,"Use up/down arrows by high border,");
	  sprintf(SelectText->message3,"or select to input range.");
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_HIST_CLIP )
	{
	  //	  SelectText->visible = (graph.type == 4 );
	  SelectText->visible = 0; // turn off this function for now
	  SelectText->color = COLOR_GREEN;
	  // Occupy the region of "data_type", which was turned off above.
	  SelectText->XPosition    = graph.nwidth - 200;;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 120;
	  if ( mosaic.clipping_mode == 0 ) // saturation
	    sprintf(SelectText->string,"Saturate high range");
	  else
	    sprintf(SelectText->string,"Clip the high range");
	  sprintf(SelectText->message1,"Clipping mode for image.");
	  sprintf(SelectText->message2,"Above the upper window level,");
	  sprintf(SelectText->message3,"saturate (white) or clip (black).");
	}
      else if ( SelectText->FieldID == TEXTFIELD_GRAPH_MODEL )
	{
	  SelectText->color = COLOR_CYAN;
	  SelectText->visible = (graph.type == 3 && graph.time_model != 0 );
	  if ( graph.time_model == 0 )
	    strcpy(SelectText->string,"model: none");
	  else if ( graph.time_model == 1 )
	    {
	      if ( graph.freq_domain )
		strcpy(SelectText->string,"model: FT ");
	      else
		strcpy(SelectText->string,"model: GLM");
	    }
	  else if ( graph.time_model == 2 )
	    strcpy(SelectText->string,"model: REM");
	  SelectText->XPosition    = 200;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + 50;
	  sprintf(SelectText->message1,"Model type");
	  sprintf(SelectText->message2,"This currently is fixed.");
	  sprintf(SelectText->message3,"It is defined on entry.");
	}
    }
}

void new_selectable_image_text(int yloc, int IDTag)
{
  static int first_time=TRUE;
  static struct SelectableText *LastSelectText;
  struct SelectableText *CurrentSelectText;

  if ( first_time )
    CurrentSelectText = &(mosaic.SelectText);
  else
    {
      if ( !(CurrentSelectText = (struct SelectableText *)malloc(sizeof(struct SelectableText))) )
	{
	  printf("Allocation error in new_selectable_image_text()\n");
	  exit(1);
	}
      LastSelectText->next = CurrentSelectText;
    }
  first_time = FALSE;
  // Define the current entry in the linked list.
  CurrentSelectText->YLocation    = yloc;   // 0,1 = top, 2,3,4 = bottom line
  CurrentSelectText->visible      = 1;      // visible by default
  CurrentSelectText->selected     = 0;      // not selected
  CurrentSelectText->FieldID      = IDTag;  // not selected
  CurrentSelectText->next         = NULL;

  LastSelectText = CurrentSelectText;
}

void new_selectable_graph_text(int yloc, int IDTag)
{
  static int first_time=TRUE;
  static struct SelectableText *LastSelectText;
  struct SelectableText *CurrentSelectText;

  if ( first_time )
    CurrentSelectText = &(graph.SelectText);
  else
    {
      if ( !(CurrentSelectText = (struct SelectableText *)malloc(sizeof(struct SelectableText))) )
	{
	  printf("Allocation error in new_selectable_graph_text()\n");
	  exit(1);
	}
      LastSelectText->next = CurrentSelectText;
    }
  first_time = FALSE;
  // Define the current entry in the linked list.
  CurrentSelectText->YLocation    = yloc;   // 1 = top line, else = bottom line
  CurrentSelectText->visible      = 1;      // visible by default
  CurrentSelectText->selected     = 0;      // not selected
  CurrentSelectText->FieldID      = IDTag;  // not selected
  CurrentSelectText->next         = NULL;

  LastSelectText = CurrentSelectText;
}

void build_graph_display( BOOL UpdateAll )
{
  if ( graph.nwidth != 0 )
    {
      if ( UpdateAll )
	draw_selectable_text_graph();
      build_graph_display_XYZT(UpdateAll);
      // show graph info
      show_information_graph(0);
    }
}

void update_graph_data( BOOL UpdateGraphData )
{
  BOOL replot;
  static IPOINT3D last_position;

  if ( graph.nwidth == 0 ) return;

  replot = ( !(mosaic.update_regions&UPDATE_CURSOR_IMAGE) ) || // any reason other than cursor motion
    ( !mosaic.display_crosshairs ) ||                         // cursor motion, but no crosshairs
    ( mosaic.icross.x != last_position.x ||                   // crosshairs, and change of position
      mosaic.icross.y != last_position.y ||
      mosaic.icross.z != last_position.z );
  if ( replot )
    {
      fill_graph_vector(0);
      build_graph_display(UpdateGraphData);
    }
  last_position = mosaic.icross;
}

void set_graph_location(int iGraph)
{
  int ihigh;

  if ( graph.type == 0 ) // x
    {
      mosaic.icross.x = iGraph;
      ihigh = mosaic.stack->dim.x;
    }
  else if ( graph.type == 1 ) // y
    {
      mosaic.icross.y = iGraph;
      ihigh = mosaic.stack->dim.y;
    }
  else if ( graph.type == 2 ) // z
    {
      mosaic.icross.z = iGraph;
      ihigh = mosaic.stack->dim.z;
    }
  else if ( graph.type == 3 )
    {
      graph.iLastTime = graph.iTime;
      graph.iTime = iGraph;
      mosaic.fSpaceMatrix = &(mosaic.stack->SpaceTime.fData[iGraph*mosaic.stack->npts_volume]);
      ihigh = mosaic.stack->nt;
    }
  else if ( graph.type == 4 )
    return; // do nothing

  if ( iGraph < 0 || iGraph > ihigh - 1 )
    {
      printf("Programming error: iGraph out of range in set_graph_location().\n");
      exit(1);
    }
  graph.update_regions |= UPDATE_SELECT_TEXT_GRAPH;
}

void initialize_graph()  // 0 = x, 1 = y, 2 = z, 3 = t
{
  static int first_time=1;
  int j;

  if ( first_time )
    {
      graph.type = -1;
      graph.nwidth = graph.nheight = 0;
      graph.npoints = mosaic.stack->nt;
      graph.histogram_bins = 100;
      first_time = 0;
    }
  else
    {
      free_float_vector(graph.xVector);
      free_float_vector(graph.yVector);
      free_float_vector(graph.yVector1);
      free_float_vector(graph.yFit);
    }
  graph.units_percent = 0;
  graph.yrange_fixed  = 0;
  graph.bin_data      = 0;
  graph.bin_width     = 1;
  graph.xmin = graph.xmax = graph.ymin = graph.ymax = 0.;
  graph.ix_low_select = graph.ix_high_select = -1;

  // type-specific parameters:
  if ( graph.type == 0 ) // x
    graph.npoints = mosaic.stack->dim.x;
  else if ( graph.type == 1 ) // y
    graph.npoints = mosaic.stack->dim.y;
  else if ( graph.type == 2 ) // z
    graph.npoints = mosaic.stack->dim.z;
  else if ( graph.type == 3 ) // time
    graph.npoints = mosaic.stack->nt;
  else if ( graph.type == 4 ) // histogram
    graph.npoints = graph.histogram_bins;
  else
    graph.npoints = mosaic.stack->nt;

  if ( graph.type == 0 || graph.type == 1 || graph.type == 2 )
    mosaic.display_crosshairs = 1; // pixelated
  else
    mosaic.display_crosshairs = 3; // line

  // Allocate &n intialize arrays
  graph.xVector   = float_vector(graph.npoints);
  graph.yVector   = float_vector(graph.npoints);
  graph.yVector1  = float_vector(graph.npoints);
  graph.yFit      = float_vector(graph.npoints);
  for (j=0; j<graph.npoints; j++)
    {
      graph.xVector[j] = j * mosaic.stack->resolution_t;
      graph.yVector[j] = graph.yFit[j] = graph.yVector1[j] = 0.;
    }
  label_graph_window();
  // Set the update flags to update everything.
  graph.update_regions   = UPDATE_GRAPH;
  graph.update_regions  |= UPDATE_GRAPH_TEXT_LINES;
  graph.update_regions  |= UPDATE_SELECT_TEXT_GRAPH;
  graph.update_regions  |= UPDATE_INFO_AREA_GRAPH;
  mosaic.update_regions |= UPDATE_IMAGES;
}

int graph_x( float dx, float x )
     // x, dx in units of time points (but may be fractional)
{
  int ix;
  ix = (int)( (x+0.5)*dx );
  return(ix);
}

int graph_y( float yscale, float y )
     // y, yscale in units of time points
{
  int iy;
  iy = (int)( yscale * (graph.ymax - y) );
  return(iy);
}

IPOINT2D Space2D_to_GraphPixels( POINT2D Space2D, POINT2D Scale )
{
  IPOINT2D GraphPixels;

  GraphPixels.x = (int)( (Space2D.x+0.5)*Scale.x );
  GraphPixels.y = (int)( Scale.y * (graph.ymax - Space2D.y) );
  return(GraphPixels);
}

void rescale_image( int find_extrema )
{
  static float min, max;
  int test;JIP_INDEX j;
  /*
  ** Determine the min and max of the data.
  */
  if ( find_extrema )
    {
      max = -1.;  min = 1.e20;
      for (j=0; j<mosaic.stack->n3d; j++)
	{
	  if ( ovl_root.npts != 0 )
	    test = overlay_map.weight[j] > 0.;
	  else
	    test = 1;
	  if ( test )
	    {
	      if ( mosaic_data_1d(j) < min ) min = mosaic_data_1d(j);
	      if ( mosaic_data_1d(j) > max ) max = mosaic_data_1d(j);
	    }
	}
    }
  // 
  image_scale_min = min + (max-min) * contrast_low;
  image_scale_max = min + (max-min) * contrast_high;

  mosaic.update_regions |= UPDATE_IMAGES;
  mosaic.update_regions |= UPDATE_COLORBAR_LABELS;
  mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
  rescale_image_settings();

  if ( graph.type == 4 )
    graph.update_regions |= UPDATE_GRAPH;
  //    build_graph_display(1);
}

void fill_graph_vector( int mode )
     // mode: 0 means update the graph
     //       1 means this is for interactive output (using 's' or 'S' or 'r' or 'R')
{
  IPOINT3D voxel;
  POINT3D space;
  int jt, jBin;JIP_INDEX jset;JIP_INDEX j, index;
  float value, average, stdev, weight_sum, value_sum, xmin, xmax;
  BOOL update_by_cursor_motion;

  // To speed up display, do nothing when a graph window is not active.
  if ( mode == 0 && graph.nwidth == 0 )
    return;

  for (j=0; j<graph.npoints; j++)
    {
      graph.xVector[j] = j * mosaic.stack->resolution_t;
      graph.yVector[j] = graph.yVector1[j]= 0.;
    }

  if ( graph.type == 0 && mosaic.icross.y >= 0 && mosaic.icross.z >= 0 ) // x graph
    {
      // Fill vector
      voxel = mosaic.icross;
      for (j=0; j<mosaic.stack->dim.x; j++)
	{
	  voxel.x = jip_int_count(j);
	  index = voxel3D_to_1D(voxel);
	  if ( mosaic.activation.visible && mosaic.activation.fSpaceMatrix != NULL )
	    {
	      graph.yVector[j] = mosaic.activation.fSpaceMatrix[index];
	      graph.yVector1[j]= mosaic_data_3d(voxel);
	    }
	  else
	    graph.yVector[j]= mosaic_data_3d(voxel);
	  space = Convert_Voxel_to_Space(voxel, mosaic.stack);
	  graph.xVector[j] = space.x;
	}
    }
  else if ( graph.type == 1 && mosaic.icross.x >= 0 && mosaic.icross.z >= 0 ) // y graph
    {
      // Fill vector
      voxel = mosaic.icross;
      for (j=0; j<mosaic.stack->dim.y; j++)
	{
	  voxel.y = jip_int_count(j);
	  index = voxel3D_to_1D(voxel);
	  if ( mosaic.activation.visible && mosaic.activation.fSpaceMatrix != NULL )
	    {
	      graph.yVector[j] = mosaic.activation.fSpaceMatrix[index];
	      graph.yVector1[j]= mosaic_data_3d(voxel);
	    }
	  else
	    graph.yVector[j]= mosaic_data_3d(voxel);
	  space = Convert_Voxel_to_Space(voxel, mosaic.stack);
	  graph.xVector[j] = space.y;
	}
    }
  else if ( graph.type == 2 && mosaic.icross.x >= 0 && mosaic.icross.y >= 0 ) // z graph
    {
      // Fill vector
      voxel = mosaic.icross;
      for (j=0; j<mosaic.stack->dim.z; j++)
	{
	  voxel.z = jip_int_count(j);
	  index = voxel3D_to_1D(voxel);
	  if ( mosaic.activation.visible && mosaic.activation.fSpaceMatrix != NULL )
	    {
	      graph.yVector[j] = mosaic.activation.fSpaceMatrix[index];
	      graph.yVector1[j]= mosaic_data_3d(voxel);
	    }
	  else
	    graph.yVector[j]= mosaic_data_3d(voxel);
	  space = Convert_Voxel_to_Space(voxel, mosaic.stack);
	  graph.xVector[j] = space.z;
	}
    }
  else if ( graph.type == 4 ) // histogram
    {
      if ( ovl_root.npts == 0 )
	{
	  // Find the min and max of image values.
	  xmin = 1.e40;   xmax = -1.e40;
	  for (j=0; j<mosaic.stack->n3d; j++)
	    {
	      value = mosaic.fSpaceMatrix[j];
	      if ( value < xmin ) xmin = value;
	      if ( value > xmax ) xmax = value;
	    }
	  // Initialize the histogram
	  for (jBin=0; jBin<graph.histogram_bins; jBin++)
	    {
	      graph.xVector[jBin] = xmin + jBin * (xmax-xmin) / (float)graph.histogram_bins;
	      graph.yVector[jBin] = 0.;
	    }
	  // Fill the histogram
	  for (j=0, value_sum=0.; j<mosaic.stack->n3d; j++)
	    {
	      value = mosaic.fSpaceMatrix[j];
	      if ( value != 0. )
		{
		  jBin = MIN(graph.histogram_bins-1,graph.histogram_bins*(value-xmin)/(xmax-xmin));
		  jBin = MAX(0,jBin);
		  graph.yVector[jBin]++;
		  value_sum += value;
		}
	    }
	}
      else
	{ // there is an active overlay
	  // Find the min and max of image values.
	  xmin = 1.e40;   xmax = -1.e40;
	  for (j=0; j<ovl_root.npts; j++)
	    {
	      value = mosaic_data_3d(ovl_root.voxel[j]);
	      if ( value < xmin ) xmin = value;
	      if ( value > xmax ) xmax = value;
	    }
	  // Initialize the histogram
	  for (jBin=0; jBin<graph.histogram_bins; jBin++)
	    {
	      graph.xVector[jBin] = xmin + jBin * (xmax-xmin) / (float)graph.histogram_bins;
	      graph.yVector[jBin] = 0.;
	    }
	  // Fill the histogram
	  for (j=0, value_sum=0.; j<ovl_root.npts; j++)
	    {
	      value = mosaic_data_3d(ovl_root.voxel[j]);
	      if ( value != 0. )
		{
		  jBin = MIN(graph.histogram_bins-1,graph.histogram_bins*(value-xmin)/(xmax-xmin));
		  graph.yVector[jBin]++;
		  value_sum += value;
		}
	    }
	}
      // Normalize the histogram
      //      for (jBin=0; jBin<graph.histogram_bins; jBin++)
      //	graph.yVector[jBin] *= (100./value_sum);
    }
  else // time graph or no graph
    {
      // Assign the time array:
      for (jt=0; jt<mosaic.stack->nt; jt++)
	graph.xVector[jt] = jt * mosaic.stack->resolution_t;
      if ( ovl_root.npts == 0 )
	{
	  update_by_cursor_motion = ( !mosaic.display_crosshairs && 
				      mosaic.icursor.x >= 0 && mosaic.icursor.x < mosaic.stack->dim.x &&
				      mosaic.icursor.y >= 0 && mosaic.icursor.y < mosaic.stack->dim.y &&
				      mosaic.icursor.z >= 0 && mosaic.icursor.z < mosaic.stack->dim.z );
	  if ( update_by_cursor_motion )
	    { // use current cursor location
	      voxel = mosaic.icursor; // for fill_TimeSeriesModel below
	      for (j=0; j<mosaic.stack->nt; j++)
		graph.yVector[j] = stack_data_4d(mosaic.icursor.x,mosaic.icursor.y,mosaic.icursor.z,jip_int_count(j));
	    }
	  else
	    {
	      voxel = mosaic.icross; // for fill_TimeSeriesModel below
	      for (j=0; j<mosaic.stack->nt; j++)
		graph.yVector[j] = stack_data_4d(mosaic.icross.x,mosaic.icross.y,mosaic.icross.z,jip_int_count(j));
	    }
	}
      else
	{  // use overlay
	  voxel.x = voxel.y = voxel.z = -1; // for fill_TimeSeriesModel below

	  for (jt=0; jt<mosaic.stack->nt; jt++)
	    {
	      // Average across the overlay.
	      for (jset=0, average=0., weight_sum=0.;
		   jset<ovl_root.npts; jset++)
		{
		  value = stack_data_4d(ovl_root.voxel[jset].x,
					ovl_root.voxel[jset].y,
					ovl_root.voxel[jset].z,jt);
		  average    += ovl_root.weight[jset] * value;
		  weight_sum += ovl_root.weight[jset];
		}
	      average /= weight_sum;
	      // Average across the overlay.
	      for (jset=0, stdev=0., weight_sum=0.;
		   jset<ovl_root.npts; jset++)
		{
		  value = stack_data_4d(ovl_root.voxel[jset].x,
					ovl_root.voxel[jset].y,
					ovl_root.voxel[jset].z,jt);
		  stdev      += ovl_root.weight[jset] * SQR( value - average );
		  weight_sum += ovl_root.weight[jset];
		}
	      stdev = sqrt( (double)(stdev/weight_sum) );
	      // Store the values in the mosaic vectors.
	      graph.yVector[jt]  = average;
	      graph.yVector1[jt] = stdev;
	    }
	}
    }

  // Convert to percent, if requested.
  if ( graph.units_percent == 1 )
    {
      // Convert to percent.
      average = 0.;
      for (j=0, average=0.; j<graph.npoints; j++)
	average += graph.yVector[j];
      average /= (float)(graph.npoints);
      if ( average != 0. )
	{
	  for (j=0; j<graph.npoints; j++)
	    {
	      graph.yVector[j]  = 100. * (graph.yVector[j]/average  - 1.);
	      graph.yVector1[j] = 100. * (graph.yVector1[j]/average - 1.);
	    }
	}
    }
  if ( graph.type == 3 )
    fill_TimeSeriesModel( voxel );
}

void increment_graph_location( BOOL change_time )
{
  if ( graph.type == 3 || change_time ) // time
    increment_graph_location_time();
  else if ( graph.type == 0 ) // x
    {
      (mosaic.icross.x)++;
      if ( mosaic.icross.x == graph.npoints ) mosaic.icross.x = 0;
    }
  else if ( graph.type == 1 ) // y
    {
      (mosaic.icross.y)++;
      if ( mosaic.icross.y == graph.npoints ) mosaic.icross.y = 0;
    }
  else if ( graph.type == 2 ) // z
    {
      (mosaic.icross.z)++;
      if ( mosaic.icross.z == graph.npoints ) mosaic.icross.z = 0;
    }
}

void decrement_graph_location( BOOL change_time )
{
  if ( graph.type == 3 || change_time ) // time
    decrement_graph_location_time();
  else if ( graph.type == 0 ) // x
    {
      (mosaic.icross.x)--;
      if ( mosaic.icross.x < 0 ) mosaic.icross.x = graph.npoints - 1;
    }
  else if ( graph.type == 1 ) // y
    {
      (mosaic.icross.y)--;
      if ( mosaic.icross.y < 0 ) mosaic.icross.y = graph.npoints - 1;
    }
  else if ( graph.type == 2 ) // z
    {
      (mosaic.icross.z)--;
      if ( mosaic.icross.z < 0 ) mosaic.icross.z = graph.npoints - 1;
    }
}

void smooth_graph_vector( float *RawVector, float *SmoothVector)
{
  int j, jrel, irel;
  float bin_size;

  for (j=0; j<graph.npoints; j++)
    {
      SmoothVector[j] = 0.;
      for (jrel=-graph.bin_width/2, bin_size=0.; jrel<MAX(1,graph.bin_width/2); jrel++)
	{
	  irel = j + jrel;
	  if ( irel >= 0 && irel < graph.npoints )
	    {
	      SmoothVector[j] += RawVector[irel];
	      bin_size += 1.;
	    }
	}
      SmoothVector[j] /= bin_size;
    }
}

void bin_graph_vector( int nbins, float *RawVector, float *BinXVector, float *BinYVector)
{
  int j, jbin;
  float bin_size;

  bin_size = 0.;  
  for (j=0, jbin=0; j<graph.npoints; j++)
    {
      if ( !(j%graph.bin_width) )
	{
	  BinXVector[jbin] /= bin_size;
	  BinYVector[jbin] /= bin_size;
	  bin_size = 0.;
	  jbin++;
	  if ( jbin == nbins )
	    return;
	  BinXVector[jbin] = BinYVector[jbin] = 0.;
	}
      BinXVector[jbin] += j;
      BinYVector[jbin] += RawVector[j];
      bin_size += 1.;
    }
}
