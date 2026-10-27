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
#include <string.h>
#include <math.h>
#include "util.h"
#include "display.h"
#include "glm-prototypes.h"
#include "srtm-prototypes.h"


int write_ovl_list_into_roi_file(char * file_name)
{
  struct ovl_list *ovl_current, *ovl_last;

  if ( ! ovl_root.next )
    {
      printf("No pre-defined overlays!\n");
      return(1);
    }

  // Make the last overlay active.
  for ( ovl_current = ovl_root.next; ovl_current; ovl_current = ovl_current->next )
    {
      ovl_current->active = FALSE;
      ovl_last = ovl_current;
    }
  ovl_last->active = TRUE;

  // Write all overlays into the ROI file.
  printf("Write time series for overlays written to file %s:\n",file_name);
  for (ovl_current=ovl_root.next; ovl_current; ovl_current=ovl_current->next)
    {
      action_step_overlay_list(1,0);  // go to next, don't combine
      mosaic.update_regions |= UPDATE_IMAGES;
      mosaic.update_regions |= UPDATE_SELECT_TEXT_IMAGE;
      Update_Regions();
      fill_graph_vector(1);
      build_graph_display(1);
      show_on_info_line(0,0,ovl_current->name,"","");
      // Write the overlay.
      if ( write_ovl_into_roi_file(file_name) ) return(1);
      printf("overlay: %s\n",ovl_current->name);
    }

  return(0);
}

int get_number_rois_in_file( char *file_name )
{
  struct roi_file_list *file_list, *file_selected;

  // Select the file.
  file_selected = NULL;
  for (file_list=&roi_file_root; file_list; file_list=file_list->next)
    {
      if ( !strcmp(file_name, file_list->name) )
	file_selected = file_list;
    }
  if ( file_selected == NULL )
    return(0);
  else
    return(file_selected->number_rois);
}

int write_ovl_into_roi_file( char *file_name )
{
  FILE *file;
  struct roi_file_list *file_list, *file_selected;
  char whole_line[LEN_STR], new_text[LEN_STR];
  int *ibuffer;
  int ichar, nchar, iloc, nevents, jevent, j;
  float fit;

  // Find the existing (or new) ROI file.
  prep_roi_file_list(file_name);

  // Select the file.
  for (file_list=&roi_file_root; file_list; file_list=file_list->next)
    {
      if ( !strcmp(file_name, file_list->name) )
	file_selected = file_list;
    }

  // Open the file.
  if ( file_selected->number_rois == 0 )
    { // Create a new file.
      if ( !(file = fopen(file_selected->name,"w")) )
	{
	  printf("\nError: cannot open file %s for writing.\n",file_selected->name);
	  return(1);
	}
      // Write the ROI data.
      if ( graph.type == 3 && graph.time_model == 2 )
	{ // REM has error bars
	  fprintf(file,"i x %s %s_err\n",ovl_root.name,ovl_root.name);
	  for (j=0; j<graph.npoints; j+=2)
	    fprintf(file,"%d %g %g %g\n",j,graph.xVector[j],graph.yVector[j],graph.yVector[j+1]);
	}
      else
	{ // 1st order fMRI and SRTM GLM have no error bars
	  fprintf(file,"i x %s\n",ovl_root.name);
	  for (j=0; j<graph.npoints; j++)
	    fprintf(file,"%d %g %g\n",j,graph.xVector[j],graph.yVector[j]);
	}
      if ( graph.type == 3 && graph.time_model == 1 )
	{ // GLM (fMRI)
	  nevents = GLM_get_number_events();
	  if ( nevents > 0 )
	    {
	      // The complete fit to data.
	      fprintf(file,"new   # GLM fit to data\n");
	      for (j=0; j<graph.npoints; j++)
		fprintf(file,"%d %g %g\n",j,graph.xVector[j],graph.yFit[j]);
	      // The fit for each of the individual event types.
	      for (jevent=0; jevent<nevents; jevent++)
		{
		  fprintf(file,"new   # fit for event type %c\n",GLM_EventChar(GLM_get_event_number(jevent)));
		  for (j=0; j<graph.npoints; j++)
		    {
		      fit = graph.yFit[j];
		      if ( GLM_event_fit(glm_model.baseline, glm_model.residue_string, jevent, j, &fit) )
			return(1);
		      fprintf(file,"%d %g %g\n",j,graph.xVector[j],fit);
		    }
		}
	    }
	}
      else if ( graph.type == 3 && graph.time_model == 2 )
	{ // REM
	  if ( glm_model.baseline )
	    {
	      // The complete fit to data with global scaling.
	      fprintf(file,"new   # GLM fit to data with global scaling\n");
	      for (j=0; j<graph.npoints; j+=2)
		fprintf(file,"%d %g %g 0\n",j,graph.xVector[j],graph.yFit[j]);
	    }
	  // The complete fit to data without global scaling.
	  fprintf(file,"new   # GLM fit to data\n");
	  for (j=1; j<graph.npoints; j+=2)
	    fprintf(file,"%d %g %g 0\n",j,graph.xVector[j],graph.yFit[j]);
	}
      else if ( graph.type == 3 && graph.time_model == 3 )
	{ // SRTM GLM
	  // The complete fit to data.
	  fprintf(file,"new   # SRTM fit to data\n");
	  for (j=0; j<graph.npoints; j++)
	    fprintf(file,"%d %g %g\n",j,graph.xVector[j],graph.yFit[j]);
	  // The NS binding fit.
	  fprintf(file,"new   # SRTM fit of non-specific binding\n");
	  for (j=0; j<graph.npoints; j++)
	    {
	      if ( SRTM_binding_fit(0, j, &fit) ) return(1);
	      fprintf(file,"%d %g %g\n",j,graph.xVector[j],fit);
	    }
	  // The S binding fit.
	  fprintf(file,"new   # SRTM fit of specific binding\n");
	  for (j=0; j<graph.npoints; j++)
	    {
	      if ( SRTM_binding_fit(1, j, &fit) ) return(1);
	      fprintf(file,"%d %g %g\n",j,graph.xVector[j],fit);
	    }
	  // R1
	  fprintf(file,"new   # R1 value from SRTM fit\n");
	  for (j=0; j<graph.npoints; j++)
	    {
	      fit = SRTM_get_parameter_value(0,j);
	      fprintf(file,"%d %g %g\n",j,graph.xVector[j],fit);
	    }
	  // tau2'
	  fprintf(file,"new   # tau2' value from SRTM fit\n");
	  for (j=0; j<graph.npoints; j++)
	    {
	      fit = SRTM_get_parameter_value(1,j);
	      fprintf(file,"%d %g %g\n",j,graph.xVector[j],fit);
	    }
	  // BP
	  fprintf(file,"new   # BP value from SRTM fit\n");
	  for (j=0; j<graph.npoints; j++)
	    {
	      fit = SRTM_get_parameter_value(2,j);
	      fprintf(file,"%d %g %g\n",j,graph.xVector[j],fit);
	    }
	}
      file_selected->number_rois++;
      fclose(file);
      printf("Time series data for overlay %s written to file %s\n",ovl_root.name,file_selected->name);
      return(0);
    }

  // Open an existing file to append to the contents.
  // Open the file to count the characters in the file.
  if ( !(file = fopen(file_selected->name,"r")) )
    {
      printf("\nError: cannot open file %s for reading\n",file_selected->name);
      return(1);
    }
  nchar = 0;
  do
    {
      ichar = fgetc(file);
      nchar++;
    }
  while (ichar != EOF);
  fclose(file);
  // Allocate space for the file buffer.
  ibuffer = int_vector(nchar);
  // Open the file again to store contents.
  if ( !(file = fopen(file_selected->name,"r")) )
    {
      printf("\nError: cannot open file %s for reading\n",file_selected->name);
      return(1);
    }
  nchar = 0;
  do
    {
      ichar = fgetc(file);
      ibuffer[nchar] = ichar;
      nchar++;
    }
  while (ichar != EOF);
  fclose(file);

  // Open the file (3rd time) to overwrite contents.
  if ( !(file = fopen(file_selected->name,"w")) )
    {
      printf("\nError: cannot open file %s for writing\n",file_selected->name);
      return(1);
    }
  // Append the roi name to the header.
  if ( graph.type == 3 && graph.time_model == 2 )
    sprintf(new_text," %s %s_err\n",ovl_root.name,ovl_root.name);
  else
    sprintf(new_text," %s\n",ovl_root.name);
  iloc = 0;
  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
  fprintf(file,"%s",whole_line);
  // Write the ROI data.
  if ( graph.type == 3 && graph.time_model == 2 )
    { // REM
      for (j=0; j<graph.npoints; j+=2)
	{
	  sprintf(new_text," %g %g\n",graph.yVector[j],graph.yVector[j+1]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
    }
  else
    {
      for (j=0; j<graph.npoints; j++)
	{
	  sprintf(new_text," %g\n",graph.yVector[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
    }
  if ( graph.type == 3 && graph.time_model == 1 )
    { // GLM (fMRI)
      nevents = GLM_get_number_events();
      if ( nevents > 0 )
	{
	  // The complete fit to data.
	  sprintf(new_text," \n");
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line); // "new"
	  for (j=0; j<graph.npoints; j++)
	    {
	      sprintf(new_text," %g\n",graph.yFit[j]);
	      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	      fprintf(file,"%s",whole_line);
	    }
	  // The fit for each of the individual event types.
	  for (jevent=0; jevent<nevents; jevent++)
	    {
	      sprintf(new_text," \n");
	      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	      fprintf(file,"%s",whole_line); // "new"
	      for (j=0; j<graph.npoints; j++)
		{
		  fit = graph.yFit[j];
		  if ( GLM_event_fit(glm_model.baseline, glm_model.residue_string, jevent, j, &fit) )
		    return(1);
		  sprintf(new_text," %g\n",fit);
		  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
		  fprintf(file,"%s",whole_line);
		}
	    }
	}
    }
  else if ( graph.type == 3 && graph.time_model == 2 )
    {
      if ( glm_model.baseline )
	{
	  // The complete fit to data with global scaling.
	  sprintf(new_text," \n");
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line); // "new"
	  for (j=0; j<graph.npoints; j+=2)
	    {
	      sprintf(new_text," %g 0\n",graph.yFit[j]);
	      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	      fprintf(file,"%s",whole_line);
	    }
	}
      // The complete fit to data without global scaling.
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=1; j<graph.npoints; j+=2)
	{
	  sprintf(new_text," %g 0\n",graph.yFit[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
    }
  else if ( graph.type == 3 && graph.time_model == 3 )
    { // SRTM GLM
      // The complete fit to data.
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  sprintf(new_text," %g\n",graph.yFit[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // The NS binding fit.
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  if ( SRTM_binding_fit(0, j, &fit) ) return(1);
	  sprintf(new_text," %g\n",fit);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // The S binding fit.
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  if ( SRTM_binding_fit(1, j, &fit) ) return(1);
	  sprintf(new_text," %g\n",fit);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // R1
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  fit = SRTM_get_parameter_value(0,j);
	  sprintf(new_text," %g\n",fit);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // tau2'
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  fit = SRTM_get_parameter_value(1,j);
	  sprintf(new_text," %g\n",fit);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // BP
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  fit = SRTM_get_parameter_value(2,j);
	  sprintf(new_text," %g\n",fit);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
    }
  file_selected->number_rois++;
  fclose(file);
  free_int_vector(ibuffer);

  printf("Time series data for overlay %s written to file %s\n",ovl_root.name,file_selected->name);

  return(0);
}

int write_projection( char *file_name )
{
  FILE *file;
  struct roi_file_list *file_list, *file_selected;
  char whole_line[LEN_STR], new_text[LEN_STR];
  int *ibuffer;
  int ichar, nchar, iloc, j;

  // Find the existing (or new) ROI file.
  prep_roi_file_list(file_name);
  // Select the file from the file list.
  for (file_list=&roi_file_root; file_list; file_list=file_list->next)
    {
      if ( !strcmp(file_name, file_list->name) )
	file_selected = file_list;
    }
  // Open the file.
  if ( file_selected->number_rois == 0 )
    { // Create a new file.
      if ( !(file = fopen(file_selected->name,"w")) )
	{
	  printf("\nError: cannot open file %s for writing.\n",file_selected->name);
	  return(1);
	}
      if ( mosaic.activation.fSpaceMatrix != NULL )
	{
	  if ( graph.type == 0 )      // x
	    sprintf(new_text,"y%dz%dt%d y%dz%dt%d_act",
		    mosaic.icross.y,mosaic.icross.z,graph.iTime,
		    mosaic.icross.y,mosaic.icross.z,graph.iTime);
	  else if ( graph.type == 1 ) // y
	    sprintf(new_text,"x%dz%dt%d x%dz%dt%d_act",
		    mosaic.icross.x,mosaic.icross.z,graph.iTime,
		    mosaic.icross.x,mosaic.icross.z,graph.iTime);
	  else if ( graph.type == 2 ) // z
	    sprintf(new_text,"x%dy%dt%d x%dy%dt%d_act",
		    mosaic.icross.x,mosaic.icross.z,graph.iTime,
		    mosaic.icross.x,mosaic.icross.z,graph.iTime);
	  else if ( graph.type == 4 ) // hist
	    sprintf(new_text,"hist%d",file_selected->number_rois);
	  fprintf(file,"x %s\n",new_text);
	  // Write the projection.
	  for (j=0; j<graph.npoints; j++)
	    fprintf(file,"%d %g %g\n",j,graph.yVector1[j],graph.yVector[j]);
	  // Write the fit to data, but repeat the 1st vector to keep the same columns
	  fprintf(file,"new # fit to projection\n");
	  for (j=0; j<graph.npoints; j++)
	    fprintf(file,"%d %g %g\n",j,graph.yVector1[j],graph.yFit[j]);
	}
      else
	{
	  if ( graph.type == 0 )      // x
	    sprintf(new_text,"y%dz%dt%d",mosaic.icross.y,mosaic.icross.z,graph.iTime);
	  else if ( graph.type == 1 ) // y
	    sprintf(new_text,"x%dz%dt%d",mosaic.icross.x,mosaic.icross.z,graph.iTime);
	  else if ( graph.type == 2 ) // z
	    sprintf(new_text,"x%dy%dt%d",mosaic.icross.x,mosaic.icross.z,graph.iTime);
	  else if ( graph.type == 4 ) // hist
	    sprintf(new_text,"hist%d",file_selected->number_rois);
	  fprintf(file,"x %s\n",new_text);
	  // Write the projection.
	  for (j=0; j<graph.npoints; j++)
	    fprintf(file,"%d %g\n",j,graph.yVector[j]);
	  // Write the fit to data
	  fprintf(file,"new # fit to projection\n");
	  for (j=0; j<graph.npoints; j++)
	    fprintf(file,"%d %g\n",j,graph.yFit[j]);
	}
      file_selected->number_rois++;
      fclose(file);
      if ( graph.type != 4 )
	printf("Projection data written to file %s\n",file_selected->name);
      else
	printf("Histogram data written to file %s\n",file_selected->name);
      return(0);
    }

  // Open an existing file to append to the contents.
  // Open the file to count the characters in the file.
  if ( !(file = fopen(file_selected->name,"r")) )
    {
      printf("\nError: cannot open file %s for reading\n",file_selected->name);
      return(1);
    }
  nchar = 0;
  do
    {
      ichar = fgetc(file);
      nchar++;
    }
  while (ichar != EOF);
  fclose(file);
  // Allocate space for the file buffer.
  ibuffer = int_vector(nchar);
  // Open the file again to store contents.
  if ( !(file = fopen(file_selected->name,"r")) )
    {
      printf("\nError: cannot open file %s for reading\n",file_selected->name);
      return(1);
    }
  nchar = 0;
  do
    {
      ichar = fgetc(file);
      ibuffer[nchar] = ichar;
      nchar++;
    }
  while (ichar != EOF);
  fclose(file);

  // Open the file (3rd time) to overwrite contents.
  if ( !(file = fopen(file_selected->name,"w")) )
    {
      printf("\nError: cannot open file %s for writing\n",file_selected->name);
      return(1);
    }
  // Append the projection name to the header.
  if ( mosaic.activation.fSpaceMatrix != NULL )
    {
      if ( graph.type == 0 )      // x
	sprintf(new_text," y%dz%dt%d y%dz%dt%d_act\n",
		mosaic.icross.y,mosaic.icross.z,graph.iTime,
		mosaic.icross.y,mosaic.icross.z,graph.iTime);
      else if ( graph.type == 1 ) // y
	sprintf(new_text," x%dz%dt%d x%dz%dt%d_act\n",
		mosaic.icross.x,mosaic.icross.z,graph.iTime,
		mosaic.icross.x,mosaic.icross.z,graph.iTime);
      else if ( graph.type == 2 ) // z
	sprintf(new_text," x%dy%dt%d x%dy%dt%d_act\n",
		mosaic.icross.x,mosaic.icross.z,graph.iTime,
		mosaic.icross.x,mosaic.icross.z,graph.iTime);
      else if ( graph.type == 4 ) // hist
	sprintf(new_text," hist%d\n",file_selected->number_rois);
      iloc = 0;
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line);
      // Write the ROI data.
      for (j=0; j<graph.npoints; j++)
	{
	  sprintf(new_text," %g %g\n",graph.yVector1[j],graph.yVector[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // The fit to data.
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  sprintf(new_text," %g %g\n",graph.yVector1[j],graph.yFit[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
    }
  else
    {
      if ( graph.type == 0 )      // x
	sprintf(new_text," y%dz%dt%d\n",mosaic.icross.y,mosaic.icross.z,graph.iTime);
      else if ( graph.type == 1 ) // y
	sprintf(new_text," x%dz%dt%d\n",mosaic.icross.x,mosaic.icross.z,graph.iTime);
      else if ( graph.type == 2 ) // z
	sprintf(new_text," x%dy%dt%d\n",mosaic.icross.x,mosaic.icross.z,graph.iTime);
      else if ( graph.type == 4 ) // hist
	sprintf(new_text," hist%d\n",file_selected->number_rois);
      iloc = 0;
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line);
      // Write the ROI data.
      for (j=0; j<graph.npoints; j++)
	{
	  sprintf(new_text," %g\n",graph.yVector[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
      // The fit to data.
      sprintf(new_text," \n");
      create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
      fprintf(file,"%s",whole_line); // "new"
      for (j=0; j<graph.npoints; j++)
	{
	  sprintf(new_text," %g\n",graph.yFit[j]);
	  create_new_ROI_line(ibuffer, new_text, &iloc, whole_line);
	  fprintf(file,"%s",whole_line);
	}
    }
  file_selected->number_rois++;
  fclose(file);
  free_int_vector(ibuffer);
  if ( graph.type != 4 )
    printf("Projection data written to file %s\n",file_selected->name);
  else
    printf("Histogram data written to file %s\n",file_selected->name);
  return(0);
}

void create_new_ROI_line(int *ibuffer, char *new_text, int *iloc, char *whole_line)
{
  static char CNULL = '\0';
  int j;
  char *ptr_line;

  ptr_line = whole_line;
  for ( j=*iloc, ptr_line = whole_line;
	ibuffer[j] !=10 ;
	j++, ptr_line++)
    *ptr_line = ibuffer[j];
  *ptr_line = CNULL;
  *iloc = j + 1;
  strcat(whole_line,new_text);
}

void prep_roi_file_list( char *file_name )
{
  struct roi_file_list *file_list, *file_this, *file_last;
  BOOL found;

  // Determine whether the current file name contains ROIs.
  found = FALSE;
  for (file_list=&roi_file_root; file_list; file_list=file_list->next)
    {
      if ( !strcmp(file_name, file_list->name) )
	{
	  found = TRUE;
	  file_this = file_list;
	}
      file_last = file_list;
    }
  // Allocate a new member of the list, if the file was not found.
  if ( !found )
    {
      // Allocate a new member of the list;
      if ( !(file_this = (struct roi_file_list *)malloc(sizeof(struct roi_file_list))) )
	{
	  printf("Error: memory problem creating a new file structure in the ROI list.\n");
	  exit(1);
	}
      // Copy the file name into the file list.
      strcpy(file_this->name,file_name);
      file_this->number_rois = 0;
      file_this->contains_ovl_list = FALSE;
      file_this->next = NULL;
      file_last->next = file_this;
    }
  return;
}
