// prototypes
void action_reduce_rolloff( BOOL PromptForScale );
void action_filter_overlay();
void define_brain_overlay( char *ovl_file_name );
void initialize_data_stacks();
void initialize_mosaic( int number_panels );
void initialize_alignment();
void SetGridDown( int iGridDown );
void convert_activation_scale( struct file_list *this_file );
void rescale_activation();
void point_mosaic_to_stack_data();
void action_increment_image();
void action_decrement_image();
void action_change_display_type(int direction);
void action_change_resolution();
void update_alignment( BOOL most_accurate );
void transform_coordinates( BOOL all_voxels, BOOL most_accurate, BOOL IgnoreOutsideTemplate );
void align_mimic_target_centroids();
void show_alignment_help_text();
int recall_alignment_parameters();
void reset_or_recall_parameters();
void print_parameter_set( int mode, int ilevel );
void rescale_images(int find_extrema);
void action_do_full_alignment();
void action_auto_adjust_affine(int button);
void auto_adjust_affine_iterations( int ilevel_max, int ilevel_min );
BOOL adjust_affine_1pass( int ilevel );
void line_scan_1d( float cost_initial, float *cost_final, float *increment_final );
int ParabolicInterpolation(float *X3Point, float *Y3Point, float *XMax, float *YMax);
void calculate_volume_averages( float *av_mimic, float *av_template );
void action_set_increments( int level );
void get_starting_increments( int level, float *increment );
float cost_function();
void update_display_set();
void fill_TimeSeriesModel( IPOINT3D voxel );
void plot_TimeSeriesModel( POINT2D *Scale );
void increment_graph_location_time();
void decrement_graph_location_time();
int set_adjustment_flags(char *string);
int action_modify_time_model_button23();
void increment_activation_map();
void decrement_activation_map();
void action_image_text_selection_specific( int button );
void action_graph_text_selection_specific( int button );
void action_graph_button23_TimeSeriesModel();

int MyDisplayWidth();
void smooth_or_sharpen_isotropic( int sharpen, POINT3D fwhm, struct data_stack *stack );
int ft_1d( int idim, int inverse, struct data_stack *stack );
JIP_INDEX index4d_stack(int ix, int iy, int iz, int it, struct data_stack *stack);
int save_alignment_parameters();
int recall_alignment_parameters();
void switch_affine_format();


void calculate_orientation_rotation();
void define_cosine_sine( int iAngle, float *cs, float *sn);
void define_affine_matrix();


void action_auto_adjust_nonlinear( int button );
void auto_adjust_nonlinear_iterations( int ilevel );
void adjust_nonlinear_1pass( int ilevel, int ipass, float cost_start );
void adjust_1basis( int iPass );
void line_scan_1d_nonlinear( int iPass, int iDir, float cost_initial,
			     float *value_increment, float *cost_increment );
void get_NL_increment( int ipass, int iDirection, float *inc_low, float *inc_high );
void Set_AlignNL_Value(int iDir, float value_new);
float cost_function_nonlinear( int InitProbeSet );
float cost_deformation( int InitProbeSet, BOOL normalize );
int number_NL_basis_functions( int nvoxels, int iDown );
void calc_NL_basis_function_locations( int which, int *Peak, int *Start, int *End);
void define_basis_functions( int which );
void compute_mixing_matrix( int nBasis_l, int nBasis_h, float **mix_lh );
void update_nonlinear_alignment_parameters();
void create_basis_function_image();
float NL_Basis_function(JIP_INDEX iBasis, IPOINT3D voxel);
void adjust_the_adjust_flags();
float mutual_information(JIP_INDEX number_voxels, double *prob_I, double *prob_T, double *prob_IT);
void calculate_volume_averages_nonlinear( int all_bases, float *av_mimic, float *av_template,
					  float *min, float *max);
float jacobian_determinant(POINT3D d0, POINT3D d1, POINT3D res);
float bending_penalty( BOOL valid_plus, BOOL valid_minus, float delta_plus, float delta_minus, float res2 );
JIP_INDEX number_invalid_distortions( int all_voxels, int ilevel, int ipass );
void upsample_stack( struct data_stack *target, IPOINT3D DimensionHigh );
void initialize_adjustment_order( int ipass );
void determine_adjustment_order();
void my_mergesort(float array[],JIP_INDEX indices[],JIP_INDEX low,JIP_INDEX high);

int write_distortion_file( char *file_name );
int open_output_file( int ntime, int format, char *file_name_root, FILE **file );
int write_nifti_header( int ntime, char *file_name_root, FILE **file );
int read_distortion_file( char *file_name, struct XD_HEADER header );

void flatten_volume( int which_volume, struct data_stack *InputStack );
float SpatialPolyDesign( IPOINT3D iVoxelDim, IPOINT3D iPolyPower, IPOINT3D iVoxel);
float Power_int(float rVoxel, int iPoly);
void select_target( int iTime, IPOINT3D TargetDim );
void select_source( int iTime);
