void action_key_l();
void action_key_L();
void action_display_last_time_point();
void action_write_all_ROIs();
void action_write_1_ROI();

// prototypes
void initialize_mosaic_ovl( int number_panels );
void convert_activation_scale( struct file_list *this_file );
void rescale_activation();
void action_fit_psf( int itype );
int action_define_table_event( int event_number );
int action_define_square_function( int event_type );
int action_fit_gamma( int event_type );
int action_compute_p_value();
int action_write_glm_files();


void action_animate();
void action_ignore_out_of_range();
int MyDisplayWidth();
int write_ovl_list_into_roi_file(char * file_name);
int write_ovl_into_roi_file(char *file_name);
int write_projection( char *file_name );
void prep_roi_file_list( char *file_names );
void create_new_ROI_line(int *ibuffer, char *new_text, int *istep, char *whole_line);
int get_number_rois_in_file( char *file_name );
void split_graphX_into_file_and_location(int xvalue, int *ifile, int *itime);
void update_borders_for_slices( int nz, int *display_slice);
void update_XWindow_image();
void draw_wire_frames();
void highlight_active_image_plane();
void highlight_image_panel( int which_panel, int color );

void initialize_TimeSeriesModel( int which_model );
int read_TimeSeriesModel( char *file_name );
int TimeSeriesModel_is_initialized();
int define_TimeSeriesModel( int nflags, int *flags );
void update_SelectText_TimeSeriesModel( struct SelectableText *SelectText );
void action_graph_text_selection_TimeSeriesModel( int button );
int graph_event_handler_TimeSeriesModel( char key );
void label_graph_time_window_TimeSeriesModel( char *FileName );

void initialize_GLM_TimeSeriesModel();
int read_GLM_TimeSeriesModel( char *file_name );
int TimeSeriesModel_is_initialized();
int define_GLM_TimeSeriesModel( int nflags, int *flags );
void plot_GLM_TimeSeriesModel( POINT2D *Scale );
void fill_GLM_TimeSeriesModel();
void get_info_GLM_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 );
void update_SelectText_GLM_TimeSeriesModel( struct SelectableText *SelectText );
void action_graph_text_selection_GLM_TimeSeriesModel( int button );
void action_graph_button23_GLM_TimeSeriesModel();
int graph_event_handler_GLM_TimeSeriesModel( char key );
void label_graph_time_window_GLM_TimeSeriesModel( char *FileName );
void FT_graph_data( int iDirection );

void initialize_REM_TimeSeriesModel();
int read_REM_TimeSeriesModel( char *file_name );
int TimeSeriesModel_is_initialized();
int define_REM_TimeSeriesModel( int nflags, int *flags );
void plot_REM_TimeSeriesModel( POINT2D *Scale );
void fill_REM_TimeSeriesModel( IPOINT3D voxel );
void get_info_REM_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 );
void update_SelectText_REM_TimeSeriesModel( struct SelectableText *SelectText );
void action_graph_text_selection_REM_TimeSeriesModel( int button );
void action_graph_button23_REM_TimeSeriesModel();
int graph_event_handler_REM_TimeSeriesModel( char key );
void label_graph_time_window_REM_TimeSeriesModel( char *FileName );
void plot_REM_TimeSeriesModel_DataPoints( int nbins, float *BinXVector, float *BinYVector, POINT2D *Scale );

void initialize_SRTM_TimeSeriesModel();
int read_SRTM_TimeSeriesModel( char *file_name );
int TimeSeriesModel_is_initialized();
int define_SRTM_TimeSeriesModel( int nflags, int *flags );
void plot_SRTM_TimeSeriesModel( POINT2D *Scale );
void fill_SRTM_TimeSeriesModel();
void get_info_SRTM_TimeSeriesModel( BOOL move_cursor, char *string1, char *string2, char *string3 );
void update_SelectText_SRTM_TimeSeriesModel( struct SelectableText *SelectText );
void action_graph_text_selection_SRTM_TimeSeriesModel( int button );
void action_graph_button23_SRTM_TimeSeriesModel();
int graph_event_handler_SRTM_TimeSeriesModel( char key );
void label_graph_time_window_SRTM_TimeSeriesModel( char *FileName );
int define_parameter_scales();


double func_gamma(double x, double *p);
int fit_gamma(int npoints, double *xarray, double *yarray, float *amplitude, float *rate);
