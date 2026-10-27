void action_key_c();
void action_key_C();
void action_key_control_c( int button );
void action_key_d();
void action_key_D();
void action_key_F();
void action_key_i();
void action_key_control_i();
void action_key_m();
void action_key_M();
void action_key_n();
void action_key_N();
void action_key_r();
void action_key_R();
void action_key_T();

void action_key_s();
void action_key_S();
void action_key_r();
void action_key_R();
void action_key_w();
void action_key_W();
void action_key_z();
void action_key_Z();
void action_key_3();
void action_key_pound_sign();
void action_key_question_mark();
void action_key_return();

// prototypes for common display functions
void action_help(int mode, int button);
void action_image_button1( int press, int xbut, int ybut );
void action_image_button23( int press );
void action_image_button4();
void action_image_button5();
void action_image_motion_notify();
void action_image_arrow_up_down( int up );
void action_image_arrow_left_right( int left );
void action_image_arrow_in_out( int in );
void action_RightPanel_tab();

void move_overlay_in_one_direction( int direction, int increment );
void action_step_overlay_list( int next, int combine );
void action_clear_overlay();
void action_clear_wires();
void action_set_projection_location();
void action_image_text_selection( int button );
float get_activation_threshold( int low_or_high );
void set_activation_threshold( int low_or_high, float value );
void change_activation_threshold( int low_or_high, float change );
void change_grayscale_range( int low_or_high, float change );
void action_new_graph_type( int change );
void action_graph_motion_notify();
void action_rescale_image_from_graph();
void action_graph_button1( int ixvalue );
void action_graph_button23( int press, int ixvalue );
void action_graph_arrow_left_right( int left );
void action_graph_arrow_up_down( int up, float yvalue );
void action_ColorBar_cursor();
void action_ColorBar_arrow_up_down( int up );
void action_ColorBar_arrow_left_right( int left );
void action_ColorBar_button1();
void action_ColorBar_button23();
void action_RightPanel_cursor();
void action_RightPanel_arrow_up_down( int up );
void action_RightPanel_arrow_left_right( int left );
void action_RightPanel_button1();
void action_RightPanel_button23();
void action_set_projection_location();
void action_change_graph_units();
void action_graph_text_selection_specific( int button );
void action_image_text_selection_specific( int button );
int action_modify_time_model_button23();
void action_select_activated_point();
void action_select_activated_region();
BOOL voxel_touches_activated_neighbor(IPOINT3D voxel);
void action_graph_button23_TimeSeriesModel();


void define_mosaic_cursor_location(int x2d, int y2d);
void define_graph_cursor_location(int x2d, int y2d);
void reset_cursor_selections();
void CursorSelection_AboveImageWindow( int ix_pos, int iy_pos );
void CursorSelection_BelowImageWindow( int ix_pos, int iy_pos );
int CursorOnSelectableText_AboveImageWindow( int ix_pos, int iy_pos );
int CursorOnSelectableText_BelowImageWindow( int ix_pos, int iy_pos );
int CursorOnSelectableText_AboveGraphWindow( int ix_pos );
int CursorOnSelectableText_BelowGraphWindow( int ix_pos, int iy_pos );
void CursorSelection_OnColorBar( int iy );
void CursorSelection_OnImageWindow( int x2d, int y2d );
IPOINT2D Convert_Space_to_Pixel(int which_orientation, POINT3D Space, int CenterPixel);
IPOINT2D Convert_rVoxel_to_Pixel(int which_orientation, POINT3D DataIndex, int CenterPixel);
IPOINT2D Convert_3DMosaicIndex_to_Pixel(int which_orientation, POINT3D DataIndex, int CenterPixel);
JIP_INDEX index4d(int ix, int iy, int iz, int it);
float stack_data_4d(int ix, int iy, int iz, int it);
float mosaic_data_1d(JIP_INDEX index_1d);
float mosaic_data_3d(IPOINT3D voxel);
JIP_INDEX voxel3D_to_1D(IPOINT3D voxel);
void update_XWindow_image();
void build_graph_display(BOOL UpdateAll);
void build_graph_display_XYZT(BOOL UpdateAll);
void plot_DataPoints( int nbins, float *BinXVector, float *BinYVector, POINT2D *Scale );
void update_graph_data( BOOL UpdateGraphAll );
void set_graph_location(int iGraph);
void initialize_graph();
void plot_graph_projection_secondary_data();
void plot_TimeSeriesModel( POINT2D *Scale );
void create_neighborhoods();
void create_neighborhood( int which_slice );
void invert_neighborhood( int which_slice );
void interpolate_2d( int iCrossZ, int which_slice, IPOINT2D PixelIndex,
		     float *fData, float *fMap, float *fOvl, float *fCross, float *fProj, int *ovl_color );
unsigned long image_pixel(float fData, float fMap, float fOvl, float fCross, float fProj, int ovl_color);
unsigned long activation_color_scheme0( float value, unsigned long gray_pixel );
unsigned long activation_color_scheme1( float value, unsigned long gray_pixel );
unsigned long activation_color_scheme2( float value, unsigned long gray_pixel );
void update_color_scheme();
void draw_crosshairs( int itype );
void highlight_active_image_plane();
void update_borders_for_slices( int nz, int *display_slice);
void draw_wire_frames();
void show_information_text_image();
void flip_vertical_CursorIndex();
POINT3D flip_vertical_DataIndex( int which_panel, POINT3D Voxel );
void read_program_init_file( char *init_file_name, char *arguments[LEN_STR], int *myargc);
void add_image_file( struct file_list *file_root, char *file_name, int scale_log, int type );
void update_root_overlay( BOOL root_selected, BOOL update_text );
void update_overlay_map();
void fill_last_closed_wire( JIP_INDEX iLastPoint );
void fill_closed_wires();
void action_mirror_overlay();
void action_mirror_wires();
BOOL intersecting_2D_vectors(POINT2D v1_start, POINT2D v1_end, POINT2D v2_start, POINT2D v2_end);
POINT2D vector_difference_2D(POINT2D v1, POINT2D v2);
POINT2D unit_vector_2D(POINT2D v1);
float cross_product_2D(POINT2D v1, POINT2D v2);
float dot_product_2D(POINT2D v1, POINT2D v2);
void ComputeBezier( POINT3D* control_point, int npts, POINT3D* curve );
CONTROL_POINT *control_vector(JIP_INDEX length);
void free_control_vector(CONTROL_POINT *vec);
BOOL coordinates_on_slice( POINT3D Point );
void automatically_set_panels( int nslices, int *nx, int *ny);
void initialize_selectable_text();
void initialize_common_selectable_text();
void initialize_specific_selectable_text();
void update_image_selectable_text();
void update_graph_selectable_text();
void update_common_image_selectable_text();
void update_common_graph_selectable_text();
void update_specific_image_selectable_text();
void update_specific_graph_selectable_text();
void new_selectable_image_text(int yloc, int IDTag);
void new_selectable_graph_text(int yloc, int IDTag);
void draw_selectable_text_image();
void draw_selectable_text_graph();
int graph_x( float dx, float x );
int graph_y( float yscale, float y );
IPOINT2D Space2D_to_GraphPixels( POINT2D Space2D, POINT2D Scale );
void Update_Regions();
void show_information_graph(int move_cursor);
void show_position_graph( int move_cursor );
void create_colorbar();
void label_colorbar();
void update_right_panel();


void loop_X_events();
void initialize_XWindows();
void make_XWindow_image();
void make_XWindow_graph();
void compute_display_size( IPOINT2D *RoughDisplaySize, float pixel_factor );
void change_window_name();
void make_orthogonal_images();
void make_mosaic_images();
void make_image( int which_orientation, int iZ );
void label_planes_ortho();
void label_planes_mosaic();
void horizontal_ticks_on_axis( int which_orientation, int istep, int iZ );
void vertical_ticks_on_axis( int which_orientation, int istep, int iZ );
void DPRIMITIVE_DrawLine(int which_drawable, int which_color, IPOINT2D iDraw1, IPOINT2D iDraw2);
void DPRIMITIVE_DrawPoint(int which_drawable, int which_color, IPOINT2D iDraw);
void DPRIMITIVE_DrawRectangle(int which_drawable, int which_color, IPOINT2D iDraw1, IPOINT2D iDraw2);
void DPRIMITIVE_FillRectangle(int which_drawable, int which_color, IPOINT2D iDraw1, IPOINT2D iDraw2);
void DPRIMITIVE_DrawText(int which_drawable, int which_color, IPOINT2D iDraw, char *string);
void DPRIMITIVE_FillText(int which_drawable, int which_color, IPOINT2D iDraw, char *string);
void highlight_image_panel( int which_panel, int color );
void action_bring_up_graph();
void action_select_slices();
void action_quit();
void action_quit_specific();
void action_image_draw_overlay( int press );
void action_draw_wires();
void refresh_display_image( int which );
void show_on_info_line(int image_or_graph, int PromptForInput, char *string1, char *string2, char *string3);

int read_SpaceTime_data( BOOL Verbose, int common_dimensions, struct data_stack *stack1,
			 struct file_list *file_info, int *nfiles );
int read_xdisplay_file( struct data_stack *stack1, struct file_list *file_info );
int read_freesurfer_file( struct data_stack *stack1, struct file_list *file_info );
void QuaternionRotation(struct data_stack *stack1,   // pass dimensions
			struct file_list *file_info, // pass rotation values
			int it );                     // time point
int read_overlay_list( char *list_file_name );
int read_wire_list( char *list_file_name );
int read_overlay_file( char *file_name, struct ovl_list *ovl );
int write_overlay_file();
int read_wire_file( char *file_name, struct wire_list *wire_list, int new_color );
int write_wire_file();
int write_roi_stats( int itime );

void label_graph_window();
void change_window_name();
void show_information_colorbar();
void rescale_image(int compute_extrema );
void rescale_image_settings();
void fill_graph_vector(int mode);
void fill_graph_time_fit();
void increment_graph_location(BOOL change_time);
void decrement_graph_location(BOOL change_time);
void increment_graph_location_time();
void decrement_graph_location_time();
void action_increment_activation_map();
void action_decrement_activation_map();

int get_X11_string(int image_or_graph, char *prompt, char *options, char *default_string,
		   int Input_allowed, int CR_required, char *string);
void smooth_graph_vector( float *RawVector, float *SmoothVector);
void bin_graph_vector( int nbins, float *RawVector, float *BinYVector, float *BinXVector);


void get_info_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 );
void fill_TimeSeriesModel( IPOINT3D voxel );
void plot_TimeSeriesModel_DataPoints( int nbins, float *BinXVector, float *BinYVector, POINT2D *Scale );
