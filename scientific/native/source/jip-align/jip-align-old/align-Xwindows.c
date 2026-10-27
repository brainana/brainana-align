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
#include "align.h"
#include "display-X11.h"

#define IRANGE 65536
#define MAX_ARGS 100


int main(int argc, char *argv[])
{
  char arguments[MAX_ARGS][LEN_STR], *myargv[MAX_ARGS];
  char *optarg, ovl_name[LEN_STR], wire_name[LEN_STR];
  int option, which_arg;
  int myargc, number_panels, nfiles, iWrap, j;
  BOOL batch_mode;

  strcpy(program_name,"align");

  myargc = argc;
  for (j=0; j<MAX_ARGS; j++)
    {
      if ( j < argc ) strcpy(arguments[j],argv[j]);
      myargv[j] = arguments[j];
    }

  if ( myargc < 2 || (myargc == 2 && (!strcmp("-y",myargv[1]) || !strcmp("-Y",myargv[1]))) )
    {
      // Read the file .xdisplay if it exists.
      read_program_init_file(".align",myargv,&myargc);
      if ( myargc < 2 )
	{
	  printf("Error: this program should be invoked as\n");
	  printf("  align <individual file> -t <template file> [optional arguments]\n\n");
	  action_help(0,1);
	  exit(1);
	}
    }

  template_unfiltered.nt = template_unfiltered.registration = 0;
  strcpy(ovl_name,"");
  strcpy(wire_name,"");
  mosaic.display_3orthogonal = mosaic.DimPanels.x = 0;
  number_panels = 0;
  mosaic.zoom = 0;
  mosaic.flip_X11_y = TRUE;
  mosaic.display_ortho_ZY = TRUE;
  Alignment.WrapAround.x  = 0;      // default: wrap x & y but not z
  Alignment.WrapAround.y  = 0;
  Alignment.WrapAround.z  = 0;
  batch_mode = FALSE;

  while ( (option=my_getopt(myargc,myargv,"z:Z:w:W:o:O:t:T:m:M:i:I:13aAhHyY",&which_arg)) )
    {
      optarg = myargv[which_arg];
      switch ((char)option)
	{
	case 'a':
	case 'A':
	  batch_mode = TRUE;
	  break;
	case 'h':
	  action_help(0,1);
	  exit(0);
	  break;
	case 'H':
	  help_menu(0,"align-menu.txt");
	  exit(0);
	  break;
	case 'i':
	case 'I':
	  add_image_file( &individual_files, optarg, 0, 0 );
	  break;
	case 'm': // mosaic: set number of panels
	case 'M':
	  sscanf(optarg,"%d",&number_panels);
	  break;
	case 'o':
	case 'O':
	  strcpy(ovl_name,optarg);
	  break;
	case 't':
	case 'T':
	  add_image_file( &template_files, optarg, 0, 0 );
	  break;
	case 'w':
	  strcpy(wire_name,optarg);
	  break;
	case 'W':
	  sscanf(optarg,"%d",&iWrap);
	  Alignment.WrapAround.x = iWrap/100;
	  Alignment.WrapAround.y = (iWrap - Alignment.WrapAround.x*100)/10;
	  Alignment.WrapAround.z = iWrap - Alignment.WrapAround.x*100 - Alignment.WrapAround.y*10;
	  printf("Wrapping in (x,y,z) = (%d,%d,%d)\n",
		 Alignment.WrapAround.x,Alignment.WrapAround.y,Alignment.WrapAround.z);
	  break;
	case 'y':
	  mosaic.flip_X11_y = !mosaic.flip_X11_y;
	  break;
	case 'Y':
	  mosaic.display_ortho_ZY = !mosaic.display_ortho_ZY;
	  break;
	case 'z':
	case 'Z':
	  sscanf(optarg,"%d",&mosaic.zoom);
	  if ( mosaic.zoom < 1 || mosaic.zoom > 10 ) mosaic.zoom = 0;
	  break;
	case '3': // orthogonal planes
	  mosaic.display_3orthogonal = 1;
	  break;
	default:
	  if ( option == -1 )
	    printf("Option %s not valid.\n",argv[which_arg]);
	  else
	    printf("Valid option -%c not yet programmed.\n",*optarg);
	  printf("Program exiting.\n");
	  exit(1);
	}
    }
  
  // Read data files: individual
  if ( !strcmp(individual_files.file_name,"") )
    {
      printf("Error: no files for the individual subject.\n");
      exit(1);
    }
  else
    {
      printf("\n");
      if ( read_SpaceTime_data(0,&original_unfiltered, &individual_files, &nfiles) )
	exit(1);
    }
  // Read data files: template
  if ( !strcmp(template_files.file_name,"") )
    {
      printf("Error: no files for the template subject.\n");
      exit(1);
    }
  else
    {
      printf("\n");
      if ( read_SpaceTime_data(0,&template_unfiltered, &template_files, &nfiles) )
	exit(1);
      if ( template_unfiltered.off_diagonal )
	{
	  printf("Warning: the template file has voxel indices    (i,j,k)\n");
	  printf("         that are not parallel with coordinates (x,y,z)\n.");
	  printf("         When applying the transformation to image data using jip,\n");
	  printf("         output file will have parallel voxels and coordinates.\n");
	  printf("         Modify the template to make these parallel.\n");
	}
    }

  // Initialize the mosaic and graph structures.
  initialize_mosaic_ovl( number_panels );
  // initialize alignment variables
  initialize_alignment();

  // re-define the origins to center all coordinates.
  AlignAid.ijk_to_xyz = centered_ijk_to_xyz(template_unfiltered);  // target voxels into space
  AlignAid.xyz_to_ijk = centered_xyz_to_ijk(original_unfiltered);  // space into source voxels

  filter_image_data();
  transform_coordinates(1,0);
  align_individual_template_centroids();
  transform_coordinates(1,1);
  cost_function(0);

  // Initialize selectable text.
  initialize_selectable_text();

  // point to the template image and window it
  AlignAid.DisplayType = 1;
  point_mosaic_to_stack_data();
  rescale_image(1);

  // point to the individual image and window it
  AlignAid.DisplayType = 0;
  point_mosaic_to_stack_data();
  rescale_image(1);

  // explicitly set a window for distortion images
  distort_scale_min = -5.; // shift in mm
  distort_scale_max =  5.; // shift in mm

  initialize_graph();

  // Bring up the display.
  initialize_XWindows();

  update_XWindow_image();

  show_alignment_info();
  update_graph_data(0,1);

  // Read the overlay and wire lists;
  // this must be done after the mosaic has been initialized.
  if ( strcmp(wire_name,"") )
    {
      if ( read_wire_list(wire_name) ) exit(1);
      mosaic.toggle_overlays = 2;
    }
  if ( strcmp(ovl_name,"") )
    {
      if ( read_overlay_list(ovl_name) ) exit(1);
      mosaic.toggle_overlays = 1;
      // On start-up, put the 1st overlay on the window.
      action_step_overlay_list( 1, 0 );
      build_image_display(1);
    }

  if ( batch_mode )
    {
      // If a file named "align.com" exists, read it and update.
      // This allows alignment to be repeated easily from an starting point.
      recall_alignment_parameters();
      calculate_orientation_rotation();
      update_alignment(1);
      // Adjust using all 12 parameters.
      AlignAid.iParameter = 0;
      AlignAid.DisplaySet = 1;
      show_alignment_info();
      auto_adjust_affine_iterations(0,3,1);
      save_alignment_parameters();
      return(0);
    }
  else
    // Interactive events:
    loop_X_events();
  // The above never returns, so we never reach the exit.
  exit(1);
}

void filter_image_data()
{
  struct data_stack temporary;
  static int first_time = TRUE;
  POINT3D smooth_mm;
  int n4d, j, jt;

  // NOTE that this function assumes magnitude data.
  // It will not work for complex data without modification.

  if ( !first_time )
    {
      free_float_vector(template.SpaceTime.fData);
      free_float_vector(individual.SpaceTime.fData);
    }
  first_time = FALSE;

  // Allocate data space for the template and individual.
  // Keep data matrices at the same size for other function calls.
  n4d = template_unfiltered.n3d * template_unfiltered.nt;
  template.SpaceTime.fData = float_vector(n4d);
  individual    = template_unfiltered;
  individual.nt = original.nt;
  individual.SpaceTime.fData = float_vector(individual.n3d*individual.nt);

  if ( AlignAid.smooth_fov_fraction == 0 )
    { // make a copy of the unfiltered template
      n4d = template_unfiltered.n3d * template_unfiltered.nt;
      for (j=0; j<n4d; j++)
	template.SpaceTime.fData[j] = template_unfiltered.SpaceTime.fData[j];
      n4d = original.n3d * original.nt;
      for (j=0; j<n4d; j++)
	original.SpaceTime.fData[j] = original_unfiltered.SpaceTime.fData[j];
    }
  else
    {
      // Make a temporary copy of the template.
      temporary = template_unfiltered;
      n4d = temporary.n3d * temporary.nt;
      temporary.SpaceTime.fData = float_vector(n4d);
      for (j=0; j<n4d; j++)
	temporary.SpaceTime.fData[j] = template_unfiltered.SpaceTime.fData[j];
      // Smooth the copy based upon the template FOV
      smooth_mm.x = template.fov.x / (float)AlignAid.smooth_fov_fraction;
      smooth_mm.y = template.fov.y / (float)AlignAid.smooth_fov_fraction;
      smooth_mm.z = template.fov.z / (float)AlignAid.smooth_fov_fraction;
      printf("smooth template by (%g %g %g), ",smooth_mm.x,smooth_mm.y,smooth_mm.z);
      smooth_or_sharpen_isotropic(0,smooth_mm, &temporary);
      // divide the smoothed version into the unfiltered version
      for (j=0; j<n4d; j++)
	{
	  if ( temporary.SpaceTime.fData[j] > 1.e-6 )
	    template.SpaceTime.fData[j] = template_unfiltered.SpaceTime.fData[j] /
	      temporary.SpaceTime.fData[j];
	  else
	    template.SpaceTime.fData[j] = 0.;
	}
      free_float_vector(temporary.SpaceTime.fData);

      // Do the same for the individual's data.
      // Make a temporary copy of the original.
      temporary = original_unfiltered;
      n4d = temporary.n3d * temporary.nt;
      temporary.SpaceTime.fData = float_vector(n4d);
      for (j=0; j<n4d; j++)
	temporary.SpaceTime.fData[j] = original_unfiltered.SpaceTime.fData[j];
      // Smooth the copy based upon the original FOV
      smooth_mm.x = original.fov.x / (float)AlignAid.smooth_fov_fraction;
      smooth_mm.y = original.fov.y / (float)AlignAid.smooth_fov_fraction;
      smooth_mm.z = original.fov.z / (float)AlignAid.smooth_fov_fraction;
      printf("smooth original by (%g %g %g)\n",smooth_mm.x,smooth_mm.y,smooth_mm.z);
      smooth_or_sharpen_isotropic(0,smooth_mm, &temporary);
      // divide the smoothed version into the unfiltered version
      for (j=0; j<n4d; j++)
	{
	  // Divide the original data by a smoothed version of itself.
	  original.SpaceTime.fData[j] = original_unfiltered.SpaceTime.fData[j] /
	    temporary.SpaceTime.fData[j];
	}
      free_float_vector(temporary.SpaceTime.fData);
    }
  // Filter with the overlay mask
  if ( AlignAid.nmask != 0 )
    {
      for (jt=0; jt<template_unfiltered.nt; jt++)
	{
	  for (j=0; j<template_unfiltered.n3d; j++)
	    {
	      if ( AlignAid.mask[j] == 0 )
		template.SpaceTime.fData[j+jt*template_unfiltered.n3d] = 0;
	    }
	}
    }
}

void initialize_mosaic_ovl( int number_panels )
{
  IPOINT2D RoughDisplaySize;
  int j, npts;

  // Generic global variables
  contrast_low  = 0.;
  contrast_high = 1.;
  drawing_overlay = 0;

  mosaic.nx_RightPanel = 120;

  // The registered "individual" stack inherits the properties of the template.
  original   = original_unfiltered;
  template   = template_unfiltered;
  individual = template_unfiltered;
  // The distortion images also match the template.
  NL_bases   = template;
  distort_x  = template;
  distort_y  = template;
  distort_z  = template;
  // The individual's time points should match the input.
  individual.nt  = original.nt;
  individual.n4d = individual.n3d * individual.nt;
  // Bases map has 1 time point; distortion maps have 2 time points.
  NL_bases.nt   = 1;
  distort_x.nt  = distort_y.nt  = distort_z.nt  = 2; // 2nd time point for saved values
  NL_bases.n4d  = NL_bases.n3d * NL_bases.nt;
  distort_x.n4d = distort_y.n4d = distort_z.n4d = distort_x.n3d * distort_x.nt;
  // Allocate the individual matrix.
  original.SpaceTime.fData   = float_vector(original.n4d);
  individual.SpaceTime.fData = float_vector(individual.n4d);
  NL_bases.SpaceTime.fData   = float_vector(NL_bases.n4d);
  distort_x.SpaceTime.fData  = float_vector(distort_x.n4d);
  distort_y.SpaceTime.fData  = float_vector(distort_y.n4d);
  distort_z.SpaceTime.fData  = float_vector(distort_z.n4d);

  for (j=0; j<NL_bases.n4d; j++)
    NL_bases.SpaceTime.fData[j] = 0;
  for (j=0; j<distort_x.n4d; j++)
    distort_x.SpaceTime.fData[j] = distort_y.SpaceTime.fData[j] = distort_z.SpaceTime.fData[j] = 0.;

  mosaic.stack = &template;
  mosaic.activation.stack = NULL;

  if ( number_panels < 0 )
    mosaic.display_3orthogonal = 1;
  else if ( number_panels > mosaic.stack->dim.z )
    number_panels = mosaic.stack->dim.z;

  mosaic.interpolate = 0;
  mosaic.display_crosshairs = 2;  // 0 = none, 1 = pixelated, 2 = line
  mosaic.display_grid = 0;
  mosaic.display_data_type = 0;
  mosaic.transparent_overlay = 1; // make transparent overlays the default

  // Compute display size
  if ( number_panels > 0 )
    mosaic.DimPanels.z = number_panels;
  else if ( number_panels < 0 )
    mosaic.DimPanels.z = 4;
  else if ( number_panels == 0 )
    mosaic.DimPanels.z = mosaic.stack->dim.z;
  compute_display_size(&RoughDisplaySize,1.);

  // Orthogonal display values.
  mosaic.icross.x = mosaic.stack->dim.x/2;
  mosaic.icross.y = mosaic.stack->dim.y/2;
  mosaic.icross.z = mosaic.stack->dim.z/2;

  // Set the pointer for the display data to the individual data array.
  mosaic.fSpaceMatrix = &(mosaic.stack->SpaceTime.fData[0]);
  // Set the complex pointer equal to the floating pointer.
  mosaic.activation.nmaps = mosaic.activation.imap = mosaic.activation.visible = 0;

  mosaic.activation.color_scheme = 0;
  mosaic.activation.color_sign   = 1;
  mosaic.toggle_overlays  = 0;

  // Initialize the overlay map.
  overlay_map.weight = float_vector(mosaic.stack->n3d);
  overlay_map.color  = short_vector(mosaic.stack->n3d);
  for (j=0; j<mosaic.stack->n3d; j++)
    {
      overlay_map.weight[j] = 0.;
      overlay_map.color[j] = 0;
    }
  overlay_map.number = 1;

  // Initialize the root overlay.
  sprintf(ovl_root.name,"none");
  ovl_root.color = COLOR_YELLOW;
  ovl_root.npts = 0;
  ovl_root.active = TRUE; // This is always true
  ovl_root.next = NULL;
  // "over-allocate" these arrays for simplicity (npts dimensions < n3d)
  
  npts = mosaic.stack->n3d;
  ovl_root.voxel  = ipoint3d_vector(npts);
  ovl_root.weight = float_vector(npts);
  for (j=0; j<npts; j++)
    {
      ovl_root.weight[j] = 0.;
      ovl_root.voxel[j].x = ovl_root.voxel[j].y = ovl_root.voxel[j].z = 0;
    }

  // Initialize the root wire frame
  sprintf(wire_root.name,"none");
  wire_root.active = TRUE;
  wire_root.npts   = 0;
  wire_root.control= NULL;
  wire_root.next   = NULL;
}

void action_change_resolution()
{
  POINT3D scale;
  SpaceTime SpaceTime;
  float SubSampleMax_old, SubSampleMax_new;
  int j;

  if ( AlignAid.SubSampleSetting.x == AlignAid.SubSample.x &&
       AlignAid.SubSampleSetting.y == AlignAid.SubSample.y &&
       AlignAid.SubSampleSetting.z == AlignAid.SubSample.z )
    return;  // no change necessary

  AlignAid.LowResolution =
    ( AlignAid.SubSampleSetting.x != 1 ) ||
    ( AlignAid.SubSampleSetting.y != 1 ) ||
    ( AlignAid.SubSampleSetting.z != 1 );

  SubSampleMax_old = MAX(AlignAid.SubSample.z,MAX(AlignAid.SubSample.x,AlignAid.SubSample.y));
  SubSampleMax_new = MAX(AlignAid.SubSampleSetting.z,
			 MAX(AlignAid.SubSampleSetting.x,AlignAid.SubSampleSetting.y));
  mosaic.zoom *= SubSampleMax_new/SubSampleMax_old;
  mosaic.zoom = MAX(1,mosaic.zoom);

  // reset the alignment matrix, but keep the fov constant
  Alignment.dimension.x = template_unfiltered.dim.x / AlignAid.SubSampleSetting.x;
  Alignment.dimension.y = template_unfiltered.dim.y / AlignAid.SubSampleSetting.y;
  Alignment.dimension.z = template_unfiltered.dim.z / AlignAid.SubSampleSetting.z;
  scale.x =  (float)template_unfiltered.dim.x / (float)Alignment.dimension.x;
  scale.y =  (float)template_unfiltered.dim.y / (float)Alignment.dimension.y;
  scale.z =  (float)template_unfiltered.dim.z / (float)Alignment.dimension.z;
  Alignment.resolution.x = template_unfiltered.resolution.x * scale.x;
  Alignment.resolution.y = template_unfiltered.resolution.y * scale.y;
  Alignment.resolution.z = template_unfiltered.resolution.z * scale.z;

  // Filter the image data using current settings.
  // This step will ALWAYS restore the template data to FULL resolution.
  filter_image_data();
  // Copy all aspects of the unfiltered template back into the template,
  // except for the data, which was altered by the filtering step above.
  SpaceTime           = template.SpaceTime;
  template            = template_unfiltered;
  template.SpaceTime  = SpaceTime; // attach the filtered data

  if ( AlignAid.LowResolution )
    downsample_stack(&template,Alignment.dimension);  // downsample template
  else
    {
      // Replace the space->voxel transformations with original ones.
      template.ijk_to_xyz = scale_and_shift_to_mat44(template.resolution,template.origin,template.direction);
      template.xyz_to_ijk = invert_mat44( template.ijk_to_xyz );
    }

  if ( AlignNL.Adjusted.x || AlignNL.Adjusted.y || AlignNL.Adjusted.z )
    { // resample the distortion fields
      if ( Alignment.dimension.x < distort_x.dim.x ||
	   Alignment.dimension.y < distort_x.dim.y ||
	   Alignment.dimension.z < distort_x.dim.z )
	{
	  downsample_stack(&distort_x,Alignment.dimension); // downsample distortion fields
	  downsample_stack(&distort_y,Alignment.dimension);
	  downsample_stack(&distort_z,Alignment.dimension);
	}
      else if ( Alignment.dimension.x > distort_x.dim.x ||
		Alignment.dimension.y > distort_x.dim.y ||
		Alignment.dimension.z > distort_x.dim.z )
	{
	  printf("resampling distortion fields ...\n");
	  upsample_stack(&distort_x,Alignment.dimension); // downsample distortion fields
	  upsample_stack(&distort_y,Alignment.dimension);
	  upsample_stack(&distort_z,Alignment.dimension);
	}
    }
  else
    { // just free the distortion fields.
      free_float_vector(distort_x.SpaceTime.fData);
      free_float_vector(distort_y.SpaceTime.fData);
      free_float_vector(distort_z.SpaceTime.fData);
      distort_x  = template;
      distort_y  = template;
      distort_z  = template;
      distort_x.nt  = distort_y.nt  = distort_z.nt  = 2; // 2nd time point for saved values
      distort_x.n4d = distort_y.n4d = distort_z.n4d = distort_x.n3d * distort_x.nt;
      distort_x.SpaceTime.fData  = float_vector(distort_x.n4d);
      distort_y.SpaceTime.fData  = float_vector(distort_y.n4d);
      distort_z.SpaceTime.fData  = float_vector(distort_z.n4d);
      for (j=0; j<distort_x.n4d; j++)
	distort_x.SpaceTime.fData[j] = distort_y.SpaceTime.fData[j] = distort_z.SpaceTime.fData[j] = 0.;
    }
  update_nonlinear_alignment_parameters();
  
  AlignAid.SubSample  = AlignAid.SubSampleSetting;
  AlignAid.ijk_to_xyz = centered_ijk_to_xyz(template);            // template voxel to space
  AlignAid.xyz_to_ijk = centered_xyz_to_ijk(original_unfiltered); // source space to voxel

  // Free the individual data matrix and reallocate at template resolution.
  free_float_vector(individual.SpaceTime.fData);
  individual     = template;
  individual.nt  = original.nt;
  individual.n4d = individual.n3d* individual.nt;
  individual.SpaceTime.fData = float_vector(individual.n4d);
  point_mosaic_to_stack_data();  // reset the data pointer

  // Reset the crosshair location.
  mosaic.icross.x = mosaic.stack->dim.x/2;
  mosaic.icross.y = mosaic.stack->dim.y/2;
  mosaic.icross.z = mosaic.stack->dim.z/2;
  // Zero the overlay when not at full resolution.
  if ( AlignAid.LowResolution )
    {  // zero the overlay
      for (j=0; j<template_unfiltered.n3d; j++)
	{
	  overlay_map.weight[j] = 0.;
	  overlay_map.color[j]  = 0;
	}
      ovl_root.npts = 0;
    }

  transform_coordinates(1,1);
  cost_function_nonlinear(AlignAid.CostFunction,0);
  // re-create the window display
  mosaic.DimPanels.z = 0; // all panels will be displayed
  update_XWindow_image();
  show_alignment_info();
  update_graph_data(0,1);
}

int image_event_handler_specific(XEvent *event)
{
  char which_key;
  BOOL current_window, affine, nonlinear, UpdateAll;
  int j;

  current_window = (event->xbutton.window == XWindow_image);
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
	  case ' ':
	    action_change_display_type(0);
	    break;
	  case '+':
	  case '=':
	    action_increment_image();
	    build_image_display(1);
	    if ( graph.type == 3 )
	      UpdateAll = FALSE;
	    else
	      UpdateAll = TRUE;
	    update_graph_data(0,UpdateAll);
	    break;
	  case '-':
	  case '_':
	    action_decrement_image();
	    build_image_display(1);
	    if ( graph.type == 3 )
	      UpdateAll = FALSE;
	    else
	      UpdateAll = TRUE;
	    update_graph_data(0,UpdateAll);
	    break;
	    ///////////////////////////////////////////////
	    //////////////// Begin alphabetic /////////////
	    ///////////////////////////////////////////////
	  case 'a':
	    affine = FALSE;
	    for (j=0; j<NAFFINE; j++)
	      affine |= AlignAid.Adjust[j];
	    nonlinear = AlignNL.Adjust.x || AlignNL.Adjust.y || AlignNL.Adjust.z;
	    if ( affine && nonlinear )
	      {
		if ( AlignAid.DisplaySet == 2 )
		  affine = FALSE;
		else
		  nonlinear = FALSE;
	      }
	    if ( affine ) // affine
	      action_auto_adjust_affine(1);
	    else if ( nonlinear )
	      action_auto_adjust_nonlinear(1);
	    break;
	  case 'A':
	    affine = FALSE;
	    for (j=0; j<NAFFINE; j++)
	      affine |= AlignAid.Adjust[j];
	    nonlinear = AlignNL.Adjust.x || AlignNL.Adjust.y || AlignNL.Adjust.z;
	    if ( affine && nonlinear )
	      {
		if ( AlignAid.DisplaySet == 2 )
		  affine = FALSE;
		else
		  nonlinear = FALSE;
	      }
	    if ( affine ) // affine
	      action_auto_adjust_affine(2);
	    else if ( nonlinear )
	      action_auto_adjust_nonlinear(2);
	    break;
	  case 'A'-'@':
	    Alignment.RigidBody = !Alignment.RigidBody;
	    transform_coordinates(1,1);
	    update_image_and_graph(1);
	    break;
	  case 'g':   // graph
	    if ( graph.type != 0 || graph.nwidth == 0 || graph.nheight == 0 )
	      {
		graph.type = 0;
		initialize_graph(); // type 0 = x graph
		make_XWindow_graph();
	      }
	    else
	      XRaiseWindow(XDisplay, XWindow_graph);
	    update_image_and_graph(1);
	    break;
	  case 'x':
	  case 'X':
	    if ( AlignAid.DisplaySet == 1 )
	      AlignAid.Adjust[AlignAid.iParameter] = !AlignAid.Adjust[AlignAid.iParameter];
	    else if ( AlignAid.DisplaySet == 2 )
	      {
		if ( AlignAid.iParameter == 0 )
		  AlignNL.Adjust.x = !AlignNL.Adjust.x;
		else if ( AlignAid.iParameter == 1 )
		  AlignNL.Adjust.y = !AlignNL.Adjust.y;
		else if ( AlignAid.iParameter == 2 )
		  AlignNL.Adjust.z = !AlignNL.Adjust.z;
		else if ( AlignAid.iParameter == 4 )
		  AlignNL.ViewBasis = !AlignNL.ViewBasis;
	      }
	    draw_selectable_text_image();
	    show_alignment_info();
	    break;
	  default:
	    // Pass the event to the graph handler if it came from the image window.
	    if ( current_window )
	      graph_event_handler_general(event); // 0 indicates event NOT from graph window
	    break;
	  }
	break; /* END OF case KeyPress: */
      }
    default:
      break;
    } /* END OF switch(event->type) */

  return(0);
}

int graph_event_handler_specific(XEvent *event)
{
  BOOL current_window;

  current_window = (event->xbutton.window == XWindow_graph);
  if ( !current_window && event->type != KeyPress ) return(0);

  if ( current_window )
    image_event_handler_general(event); // 0 indicates event NOT from graph window

  return(0);
}

void label_graph_window()
{
  char graph_type[LEN_STR];

  if ( !XWindow_graph )
    return;

  if ( graph.type == 0 )      // x
    sprintf(graph_type,"x projection");
  else if ( graph.type == 1 ) // y
    sprintf(graph_type,"y projection");
  else if ( graph.type == 2 ) // z
    sprintf(graph_type,"z projection");
  else if ( graph.type == 3 ) // t
    sprintf(graph_type,"time points");

  // Give the window a name.
  XStoreName(XDisplay, XWindow_graph, graph_type);
}

void initialize_specific_selectable_text()
{
  ////////////////////////////////////////////////////////////////////
  // Initialize the "standard" set of text fields for the image window.
  // new_selectable_image_text(Y-loc, ID)
  // individual/template toggle text
  new_selectable_image_text(1, CURSOR_ON_INDIVIDUAL_TEMPLATE);
  // "select all" or "select none"
  new_selectable_image_text(2, CURSOR_ON_SELECT_PARAMETERS);
  // cost function
  new_selectable_image_text(3, CURSOR_ON_COST_FUNCTION);
  // reset/recall
  new_selectable_image_text(4, CURSOR_ON_RESET_RECALL);
  // auto align
  new_selectable_image_text(0, CURSOR_ON_AUTO_ALIGN);
  // do everything
  new_selectable_image_text(1, CURSOR_ON_DO_EVERYTHING);

  ////////////////////////////////////////////////////////////////////
  // Initialize the "standard" set of text fields for the graph window.
  // new_selectable_image_text(Y-loc, ID)
  // graph type
  //  new_selectable_graph_text(1,   CURSOR_ON_GRAPH_TYPE);
}

void update_specific_image_selectable_text()
{ // This function is called whenever mosaic.ImageRegionPixels.x changes.
  struct SelectableText *SelectText;
  BOOL AllSelected, NonZero, SavedValue, affine, nonlinear, AdjustedNL;
  int iRightPanel, j;
  static int save_ovl_npts=0;

  iRightPanel = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar - lXBottomMenu;
  for (SelectText = &(mosaic.SelectText); SelectText; SelectText=SelectText->next)
    {
      if ( SelectText->FieldID == CURSOR_ON_WHICH_OVL_WIRE )
	{
	  if ( AlignAid.LowResolution )
	    SelectText->visible = 0;
	  if ( save_ovl_npts != ovl_root.npts )
	    show_alignment_info();
	  save_ovl_npts = ovl_root.npts;
	}
      if ( SelectText->FieldID == CURSOR_ON_DATA_TYPE )
	{ // turn off display of the data type (mag, phase, real, imag)
	  SelectText->visible = 0;
	}
      else if ( SelectText->FieldID == CURSOR_ON_INDIVIDUAL_TEMPLATE )
	{
	  SelectText->visible = 1;
	  if ( AlignAid.DisplayType == 0 )  // individual
	    {
	      if ( individual.nt > 1 )
		sprintf(SelectText->string," individual (%d)",graph.iTime);
	      else
		sprintf(SelectText->string," individual ");
	      sprintf(SelectText->message1,"Switch to template image or distortion fields");
	      sprintf(SelectText->message2,"alternative: use spacebar for template/individual");
	      sprintf(SelectText->message3,"L forward, R reverse through images");
	    }
	  else if ( AlignAid.DisplayType == 1 )  // template
	    {
	      if ( template.nt > 1 )
		sprintf(SelectText->string," template (%d) ",iTime_Template);
	      else
		sprintf(SelectText->string," template ");
	      sprintf(SelectText->message1,"Switch to individual image or distortion fields");
	      sprintf(SelectText->message2,"alternative: use spacebar");
	      sprintf(SelectText->message3,"L forward, R reverse through images");
	    }
	  else if ( AlignAid.DisplayType == 2 )  // non-linear X
	    sprintf(SelectText->string,"distort X ");
	  else if ( AlignAid.DisplayType == 3 )  // non-linear X
	    sprintf(SelectText->string,"distort Y ");
	  else if ( AlignAid.DisplayType == 4 )  // non-linear Y
	    sprintf(SelectText->string,"distort Z ");
	  else if ( AlignAid.DisplayType == 5 )  // non-linear Z
	    sprintf(SelectText->string,"NL basis functions");
	  // Occupy the region of "data_type", which was turned off above.
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x/2;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart+100;
	}
      else if ( SelectText->FieldID == CURSOR_ON_SELECT_PARAMETERS )
	{
	  SelectText->visible = 1;
	  SelectText->XPosition    = iRightPanel; // relative to bottom menu position
	  SelectText->XSelectStart = SelectText->XPosition  + lXBottomMenu;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + mosaic.nx_RightPanel;
	  if ( AlignAid.DisplaySet == 0 )
	    SelectText->visible = 0;
	  else if ( AlignAid.DisplaySet == 1 )
	    { // affine
	      AllSelected = TRUE;
	      for (j=0; j<NAFFINE; j++)
		AllSelected &= AlignAid.Adjust[j];
	      if ( AllSelected )
		{
		  strcpy(SelectText->string,"select none");
		  sprintf(SelectText->message1,"De-select all affine parameters");
		  sprintf(SelectText->message2,"Note: individual parameters can be");
		  sprintf(SelectText->message3,"selected or de-selected using 'x'");
		}
	      else
		{
		  strcpy(SelectText->string,"select  all");
		  sprintf(SelectText->message1,"Select all affine parameters");
		  sprintf(SelectText->message2,"Note: individual parameters can be");
		  sprintf(SelectText->message3,"selected or de-selected using 'x'");
		}
	    }
	  else if ( AlignAid.DisplaySet == 2 )
	    { // non-linear X
	      AllSelected = AlignNL.Adjust.x && AlignNL.Adjust.y && AlignNL.Adjust.z;
	      if ( AllSelected )
		{
		  strcpy(SelectText->string,"select none");
		  sprintf(SelectText->message1,"De-select all 3 distortion fields");
		  sprintf(SelectText->message2,"Note: individual fields can be");
		  sprintf(SelectText->message3,"selected or de-selected using 'x'");
		}
	      else
		{
		  strcpy(SelectText->string,"select  all");
		  sprintf(SelectText->message1,"Select all 3 distortion fields");
		  sprintf(SelectText->message2,"Note: individual fields can be");
		  sprintf(SelectText->message3,"selected or de-selected using 'x'");
		}
	    }
 	}
      else if ( SelectText->FieldID == CURSOR_ON_COST_FUNCTION )
	{
	  SelectText->visible = 1;
	  SelectText->XPosition    = iRightPanel;  // relative to bottom menu position
	  SelectText->XSelectStart = SelectText->XPosition  + lXBottomMenu;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + mosaic.nx_RightPanel;
	  if ( ovl_root.npts != 1 )
	    { // 0 = mutual information
	      if ( (AlignNL.Adjusted.x || AlignNL.Adjusted.y || AlignNL.Adjusted.z)
		   && AlignNL.Lambda_reg != 0 )
		sprintf(SelectText->string,"MI = %5.3f/%5.3f",AlignAid.CF_Affine,AlignAid.CF_NonLinear);
	      else
		sprintf(SelectText->string,"MI = %5.3f",AlignAid.CF_Affine);
	    }
	  sprintf(SelectText->message1,"The current cost function is Mutual Information.");
	  sprintf(SelectText->message2,"No other cost functions are currently implemented.");
	  sprintf(SelectText->message3," ");
	}
      else if ( SelectText->FieldID == CURSOR_ON_AUTO_ALIGN )
	{
	  SelectText->visible = 1;
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + mosaic.nx_RightPanel;
	  sprintf(SelectText->message1,"Keyboard: a/A");
	  affine = FALSE;
	  for (j=0; j<NAFFINE; j++)
	    affine |= AlignAid.Adjust[j];
	  nonlinear = AlignNL.Adjust.x || AlignNL.Adjust.y || AlignNL.Adjust.z;
	  if ( affine && nonlinear )
	    {
	      if ( AlignAid.DisplaySet == 2 )
		affine = FALSE;
	      else
		nonlinear = FALSE;
	    }
	  if ( affine )
	    {
	      sprintf(SelectText->string,"  auto-align affine ");
	      sprintf(SelectText->message2,"Multi-Resolution (a) or Single Resolution (A)");
	      sprintf(SelectText->message3,"L button             or R button");
	    }
	  else
	    {
	      sprintf(SelectText->string,"  auto-align non-linear ");
	      sprintf(SelectText->message2,"Multi-Step (a) or Single Step (A)");
	      sprintf(SelectText->message3,"L button       or R button");
	    }
	}
      else if ( SelectText->FieldID == CURSOR_ON_DO_EVERYTHING )
	{
	  AdjustedNL = AlignNL.Adjusted.x || AlignNL.Adjusted.y || AlignNL.Adjusted.z;
	  if ( AdjustedNL )
	    SelectText->visible = 0;
	  else
	    SelectText->visible = 1;
	  SelectText->visible = 0;
	  SelectText->XPosition    = mosaic.ImageRegionPixels.x + mosaic.nx_ColorBar;
	  SelectText->XSelectStart = SelectText->XPosition;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + mosaic.nx_RightPanel;
	  sprintf(SelectText->message1,"Keyboard: e/E");
	  affine = !(AlignNL.Adjust.x || AlignNL.Adjust.y || AlignNL.Adjust.z);
	  sprintf(SelectText->string,"  Do everything  ");
	  sprintf(SelectText->message1,"After crude manual adjustment ...");
	  sprintf(SelectText->message2,"Apply rolloff/overlay filters");
	  sprintf(SelectText->message3,"Perform affine & non-linear alignments");
	}
      else if ( SelectText->FieldID == CURSOR_ON_RESET_RECALL )
	{
	  SelectText->visible = 1;
	  SelectText->XPosition    = iRightPanel;  // relative to bottom menu position
	  SelectText->XSelectStart = SelectText->XPosition  + lXBottomMenu;
	  SelectText->XSelectEnd   = SelectText->XSelectStart + mosaic.nx_RightPanel;
	  if ( AlignAid.DisplaySet == 0 )
	    {
	      sprintf(SelectText->string," recall saved ");
	      sprintf(SelectText->message1,"Load parameters from file 'align.com'");
	      sprintf(SelectText->message2,"Caution: this will overwrite any");
	      sprintf(SelectText->message3,"alignment during this program session");
	    }
	  else
	    {
	      NonZero    = FALSE;
	      SavedValue = FALSE;
	      if ( AlignAid.DisplaySet == 1 )
		{ // affine
		  for (j=0; j<6; j++)        // translations & rotations
		    {
		      NonZero    |= (Alignment.value[j]      != 0.);
		      SavedValue |= (AlignAid.saved_value[j] != 0.);
		    }
		  for (j=6; j<9; j++)        // inflations
		    {
		      NonZero    |= (Alignment.value[j]      != 1.);
		      SavedValue |= (AlignAid.saved_value[j] != 1.);
		    }
		  for (j=9; j<NAFFINE; j++)  // skews
		    {
		      NonZero    |= (Alignment.value[j]      != 0.);
		      SavedValue |= (AlignAid.saved_value[j] != 0.);
		    }
		}
	      else if ( AlignAid.DisplaySet == 2 )
		{ // non-linear
		  NonZero = FALSE;
		  SavedValue = FALSE;
		  for (j=0; j<distort_x.n3d; j++)
		    {
		      if ( distort_x.SpaceTime.fData[j] != 0. ) NonZero = TRUE;
		      if ( distort_y.SpaceTime.fData[j] != 0. ) NonZero = TRUE;
		      if ( distort_z.SpaceTime.fData[j] != 0. ) NonZero = TRUE;
		      if ( distort_x.SpaceTime.fData[j+distort_x.n3d] != 0. ) SavedValue = TRUE;
		      if ( distort_y.SpaceTime.fData[j+distort_y.n3d] != 0. ) SavedValue = TRUE;
		      if ( distort_z.SpaceTime.fData[j+distort_z.n3d] != 0. ) SavedValue = TRUE;
		    }
		}
	      if ( NonZero )
		{
		  sprintf(SelectText->string," reset ");
		  sprintf(SelectText->message1,"Reset values to starting values");
		  sprintf(SelectText->message2,"and store current values.");
		  sprintf(SelectText->message3,"Use to evaluate alignment changes.");
		}
	      else if ( SavedValue )
		{
		  sprintf(SelectText->string," recall ");
		  sprintf(SelectText->message1,"Recall previously saved values");
		  sprintf(SelectText->message2,"from this session.");
		  sprintf(SelectText->message3,"Use to evaluate alignment changes.");
		}
	      else
		SelectText->visible = 0;
	    }
	}
    }
}
void update_specific_graph_selectable_text()
{
}

void show_alignment_info()
{
  char string[LEN_STR];
  IPOINT2D iStart, iEnd, iPut;
  int NParamShown, iPar, which_color, iStep;
  int j;

  if ( !XWindow_image ) return;

  // Clear the right panel
  XPutImage( XDisplay, XPixmap_RightPanel, XGC_image, XImage_RightPanel, 0,0,0,0,
	     XImage_RightPanel->width,XImage_RightPanel->height );

  // Panel headers.
  if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    which_color = COLOR_RED;
  else
    which_color = COLOR_WHITE;
  if ( AlignAid.DisplaySet == 0 )
    sprintf(string,"  Setup  ");
  else if ( AlignAid.DisplaySet == 1 )
    {
      if ( Alignment.RigidBody )
	sprintf(string," Affine ");
      else
	sprintf(string,"deck-of-cards");
    }
  else if ( AlignAid.DisplaySet == 2 )
    sprintf(string,"Non-linear");
  // Draw a line.
  iStart.x = 2;             iEnd.x = mosaic.nx_RightPanel-2;
  iStart.y = INFO_HEIGHT+2; iEnd.y = iStart.y;
  DPRIMITIVE_DrawLine(Drawable_RightPanel, which_color, iStart, iEnd);
  // Reset the start and end values for writing
  iStart.x = 10;
  iStart.y = INFO_HEIGHT;
  iEnd.x   = mosaic.nx_RightPanel;
  iEnd.y   = mosaic.ImageRegionPixels.y;
  // Write the panel header
  iPut.x = iStart.x + 0.1 * (iEnd.x-iStart.x);
  iPut.y = iStart.y;
  DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);

  if ( AlignAid.DisplaySet == 0 )
    {
      if ( iEnd.y < 200 )
	NParamShown = 15; // designed to account for sub-headers and info text
      else
	NParamShown = 17; // designed to account for sub-headers and info text
    }
  else if ( AlignAid.DisplaySet == 1 )
    NParamShown = AlignAid.iLastParShown - AlignAid.iFirstParShown + 1;
  else if ( AlignAid.DisplaySet == 2 )
    NParamShown = 17;

  iStep = (iEnd.y-iStart.y)/NParamShown;
  if ( AlignAid.DisplaySet == 0 )
    { // Orientation parameters and information
      // Setup section header: orientation
      sprintf(string,"Orientation");
      iPut.y = iStart.y + 1.5*iStep;
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_CYAN, iPut, string);
      for ( j = 0; j < 4; j++ )
	{
	  iPut.y += iStep;
	  if ( j == 0 )      // rotation axis: x
	    sprintf(string,"Rot. x  %d",Alignment.OrientRotation.x*90);
	  else if ( j == 1 ) // rotation axis: y
	    sprintf(string,"Rot. y  %d",Alignment.OrientRotation.y*90);
	  else if ( j == 2 ) // rotation axis: z
	    sprintf(string,"Rot. z  %d",Alignment.OrientRotation.z*90);
	  else if ( j == 3 ) // swap z
	    {
	      if ( Alignment.OrientSwapZ )
		sprintf(string,"Swap z: Yes");
	      else
		sprintf(string,"Swap z: No ");
	    }
	  if ( j == AlignAid.iParameter )
	    which_color = COLOR_YELLOW;
	  else
	    which_color = COLOR_WHITE;
	  DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
	}
      // information text: parity
      if ( iEnd.y > 200 )
	{
	  iPut.y += iStep;
	  if ( Alignment.OrientSwapZ )
	    {
	      sprintf(string,"swaps parity");
	      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
	      iPut.y += INFO_HEIGHT;
	      sprintf(string,"RHC <--> LHC");
	      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
	    }
	  else
	    {
	      sprintf(string,"conserves parity");
	      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
	      iPut.y += INFO_HEIGHT;
	      sprintf(string,"RHC <--> RHC");
	      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
	    }
	}
      // Setup section header: filters
      iPut.y += 1.5*iStep;
      sprintf(string,"Filters:");
      if ( !AlignAid.LowResolution )
	DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_CYAN, iPut, string);
      iPut.y += iStep;
      sprintf(string,"rolloff %d",AlignAid.smooth_fov_fraction);
      if ( AlignAid.iParameter == 4 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      if ( !AlignAid.LowResolution )
	DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
      iPut.y += iStep;
      sprintf(string,"overlay");
      if ( AlignAid.iParameter == 5 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      if ( !AlignAid.LowResolution && ovl_root.npts != AlignAid.nmask )
	DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);

      // Setup section header: down-sampling
      iPut.y += 1.5*iStep;
      sprintf(string,"Down-sampling");
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_CYAN, iPut, string);
      for ( j = 6; j < 9; j++ )
	{
	  iPut.y += iStep;
	  if ( j == 6 )      // rotation axis: x
	    sprintf(string," x    %d",AlignAid.SubSampleSetting.x);
	  else if ( j == 7 ) // rotation axis: y
	    sprintf(string," y    %d",AlignAid.SubSampleSetting.y);
	  else if ( j == 8 ) // rotation axis: z
	    sprintf(string," z    %d",AlignAid.SubSampleSetting.z);
	  if ( j == AlignAid.iParameter )
	    which_color = COLOR_YELLOW;
	  else
	    which_color = COLOR_WHITE;
	  DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
	}
      // "change res." to different resolution
      if ( AlignAid.SubSampleSetting.x != AlignAid.SubSample.x ||
	   AlignAid.SubSampleSetting.y != AlignAid.SubSample.y ||
	   AlignAid.SubSampleSetting.z != AlignAid.SubSample.z )
	{
	  iPut.y += iStep;
	  sprintf(string,"change res.");
	  if ( AlignAid.iParameter == 9 )
	    which_color = COLOR_YELLOW;
	  else
	    which_color = COLOR_WHITE;
	  DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
	}
    }
  else if ( AlignAid.DisplaySet == 1 )
    { // Affine alignment parameters
      for ( iPar = AlignAid.iFirstParShown, j=0;
	    iPar <= AlignAid.iLastParShown;
	    iPar++, j++ )
	{
	  iPut.y = iStart.y + INFO_HEIGHT/2 + iStep*(j+0.5);
	  iPut.x -= 10;  // increment for parameter name and value
	  // Adjustment flag
	  if ( AlignAid.Adjust[iPar] )
	    strcpy(string,"x ");
	  else
	    strcpy(string,"  ");
	  DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_YELLOW, iPut, string);
	  iPut.x += 10;  // decrement back to starting position for adjustment flag
	  if ( iPar == AlignAid.iParameter )
	    which_color = COLOR_YELLOW;
	  else
	    which_color = COLOR_WHITE;
	  if ( iPar == AlignAid.iParameter && !AlignAid.ChangeValue )
	    // increment
	    sprintf(string,"increment %g",AlignAid.increment[iPar]);
	  else
	    // Name and increment
	    sprintf(string,"%s %6.2f",AlignAid.name[iPar],Alignment.value[iPar]);
	  DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
	}
    }
  else if ( AlignAid.DisplaySet == 2 )
    { // Non-linear alignment
      iPut.x -= 10;
      sprintf(string,"Nonlinear fields");
      iPut.y = iStart.y + 1.5*iStep;
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_CYAN, iPut, string);
      iPut.x += 10;
      for ( iPar=0; iPar < 3; iPar++ )
	{
	  iPut.y += iStep;
	  iPut.x -= 10;  // increment for parameter name and value
	  // Adjustment flag
	  if ( (iPar == 0 && AlignNL.Adjust.x) ||
	       (iPar == 1 && AlignNL.Adjust.y) ||
	       (iPar == 2 && AlignNL.Adjust.z) )
	    strcpy(string,"x ");
	  else
	    strcpy(string,"  ");
	  DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_YELLOW, iPut, string);
	  iPut.x += 10;  // decrement back to starting position for adjustment flag
	  if ( iPar == AlignAid.iParameter )
	    which_color = COLOR_YELLOW;
	  else
	    which_color = COLOR_WHITE;
	  // Name only
	  if ( iPar == 0 )
	    sprintf(string,"X Field");
	  else if ( iPar == 1 )
	    sprintf(string,"Y Field");
	  else if ( iPar == 2 )
	    sprintf(string,"Z Field");
	  DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
	}

      iPut.y += 2*iStep;
      sprintf(string," Resolution");
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_CYAN, iPut, string);
      iPut.y += iStep;
      if ( AlignAid.iParameter == 3 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      // Name
      sprintf(string,"down-sample %d",AlignNL.iGridDown);
      DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
      iPut.y += iStep;
      sprintf(string,"FWHM (mm)");
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
      iPut.y += iStep;
      sprintf(string,"%5.2g %5.2g %5.2g",
	      AlignNL.GridDown.x*template.resolution.x,
	      AlignNL.GridDown.y*template.resolution.y,
	      AlignNL.GridDown.z*template.resolution.z);
      iPut.x -= 20;
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
      iPut.x += 20;

      iPut.y += iStep;
      if ( AlignNL.ViewBasis )
	strcpy(string,"x ");
      else
	strcpy(string,"  ");
      iPut.x -= 10;
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_YELLOW, iPut, string);
      iPut.x += 10;
      sprintf(string," %d / %d ",AlignNL.iBasis,AlignNL.NBasis);
      if ( AlignAid.iParameter == 4 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
      // Regularization
      iPut.y += 2*iStep;
      sprintf(string,"Regularization");
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_CYAN, iPut, string);
      // Rigidity
      iPut.y += iStep;
      if ( AlignAid.iParameter == 5 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      sprintf(string," Rigidity  %d ",AlignNL.Lambda_reg);
      DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
      // Jacobian
      iPut.y += iStep;
      if ( AlignAid.iParameter == 6 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      sprintf(string," Jacobian  %d ",AlignNL.Jacob_limits);
      DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
      // Stiffness/smoothness
      iPut.y += iStep;
      sprintf(string,"smooth<->stiff");
      DPRIMITIVE_DrawText(Drawable_RightPanel, COLOR_WHITE, iPut, string);
      iPut.y += iStep;
      if ( AlignNL.stiffness == 1. )
	sprintf(string,"-  -  -  -  X");
      else if ( AlignNL.stiffness == 0.75 )
	sprintf(string,"-  -  -  X  -");
      else if ( AlignNL.stiffness == 0.50 )
	sprintf(string,"-  -  X  -  -");
      else if ( AlignNL.stiffness == 0.25 )
	sprintf(string,"-  X  -  -  -");
      else if ( AlignNL.stiffness == 0. )
	sprintf(string,"X  -  -  -  -");
      if ( AlignAid.iParameter == 7 )
	which_color = COLOR_YELLOW;
      else
	which_color = COLOR_WHITE;
      DPRIMITIVE_FillText(Drawable_RightPanel, which_color, iPut, string);
    }

  // Highlight the parameter area if it is selected.
  iStart.x = 2;  iEnd.x   -= 2;
  iStart.y = 2;  iEnd.y   -= 2;
  if ( mosaic.cursor_selection&CURSOR_ON_RIGHT_PANEL )
    DPRIMITIVE_DrawRectangle(Drawable_RightPanel, COLOR_RED,   iStart, iEnd);
  else
    DPRIMITIVE_DrawRectangle(Drawable_RightPanel, COLOR_WHITE, iStart, iEnd);

  refresh_display_image(2);
}

void show_alignment_help_text()
{
  char message1[LEN_STR], message2[LEN_STR], message3[LEN_STR];

  if ( AlignAid.DisplaySet == 0 )
    {
      if ( AlignAid.iParameter >= 0 && AlignAid.iParameter < 3 )  // orientation angles
	{
	  sprintf(message1,"Use these orientation angles for rough alignment,");
	  sprintf(message2,"E.g., to switch sagital to coronal, ...");
	  sprintf(message3,"Increments of +- 90 degrees.");
	}
      else if ( AlignAid.iParameter == 3 ) // swap-z
	{
	  sprintf(message1,"Use this option to reverse slices.");
	  sprintf(message2,"Swapping Z changes the coordinate handedness,");
	  sprintf(message3,"which will swap left/right identification. ");
	}
      else if ( AlignAid.iParameter == 4 ) // roll-off filter
	{
	  sprintf(message1,"Use the rolloff filter to reduce RF bias.");
	  sprintf(message2,"RF bias can contribute to poor registration.");
	  sprintf(message3,"The default value of 1/4 FOV generally works well.");
	}
      else if ( AlignAid.iParameter == 5 ) // overlay filter
	{
	  sprintf(message1,"Use an overlay to mask extracerebral signal.");
	  sprintf(message2,"Create & use a brain overlay with your template.");
	  sprintf(message3,"This is most important for linear registration.");
	}
      else if ( AlignAid.iParameter >= 6 && AlignAid.iParameter < 9 ) // sub-sampling
	{
	  sprintf(message1,"Sub-sampling boosts speed by reducing voxels.");
	  sprintf(message2,"Only use this for large grids with good");
	  sprintf(message3,"resolution and good image contrast.");
	}
      else if ( AlignAid.iParameter == 9 )  // change resolution
	{
	  sprintf(message1,"You can change resolution here manually.");
	  sprintf(message2,"Subsampling will be used automatically with 'a'");
	  sprintf(message3,"auto-alignment; each step reduces sub-sampling.");
	}
    }
  else if ( AlignAid.DisplaySet == 1 ) // 12 affine parameters
    {
      sprintf(message1,"Affine alignment parameters: adjust manually");
      sprintf(message2,"to produce very rough alignment, and then");
      sprintf(message3,"auto-align using 'a' or 'A'");
    }
  else if ( AlignAid.DisplaySet == 2 )
    {
      if ( AlignAid.iParameter >= 0 && AlignAid.iParameter < 3 )  // distortion fields
	{ // distortion fields
	  sprintf(message1,"Use non-linear alignment AFTER affine alignment.");
	  sprintf(message2,"Use 'x' or L/R arrows to select/de-select fields.");
	  sprintf(message3,"Auto-align using 'a' or 'A'");
	}
      else if ( AlignAid.iParameter == 3 )  // basis grid
	{ // basis grid
	  sprintf(message1,"The basis grid is chosen automatically");
	  sprintf(message2,"for auto-alignment ('a'). You can manually select");
	  sprintf(message3,"values for multi-stage custom alignment ('A').");
	}
      else if ( AlignAid.iParameter == 4 )  // viewing basis functions
	{
	  sprintf(message1,"This field if provided for viewing basis functions.");
	  sprintf(message2,"Choose a basis function (arrows), select with 'x',");
	  sprintf(message3,"select the basis function image, & re-window ('i')");
	}
      else if ( AlignAid.iParameter == 5 )  // rigidity
	{
	  sprintf(message1,"This is the strength of the warping penalty.");
	  sprintf(message2,"Larger values allow less distortions.");
	  sprintf(message3,"A value of 5-10 generally works well.");
	}
      else if ( AlignAid.iParameter == 6 )  // Jacobian
	{
	  sprintf(message1,"This limits compression/expansion explicitly.");
	  sprintf(message2,"Larger values enable more distortions.");
	  sprintf(message3,"Jacobians range from 1/n to n.");
	}
      else if ( AlignAid.iParameter == 7 )  // smooth <--> stiff
	{
	  sprintf(message1,"You can adjust the form of the penalty term.");
	  sprintf(message2,"Move the slider using L/R arrows to weight by");
	  sprintf(message3,"the 2nd deriv (smooth) or the 1st deriv (stiff).");
	}
    }
  show_on_info_line(0,1,message1,message2,message3);
}

void get_info_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 )
{
  // do nothing for the alignment package
}
