JIP_INDEX index2d(int ix, int iy, struct volume_list *volume);
JIP_INDEX index3d(int ix, int iy, int iz, struct volume_list *volume);
JIP_INDEX index4d(int ix, int iy, int iz, int it, struct data_stack *stack);
float volume_amp( struct volume_list *volume, JIP_INDEX index );
float volume_phase( struct volume_list *volume, JIP_INDEX index );
float stack_amp( struct data_stack *stack, JIP_INDEX index );
void polar_from_complex(float real, float imag, float *amp, float *phase);
void delete_stacks();
void show_stacks();
void convert_complex();
void convert_complex_stack( struct data_stack *stack );
void convert_to_real();
int copy_volumes( char *new_stack_name );
int inconsistent_3d_dimensions( IPOINT3D *dim, int *nt );

int fft2d_volumes( int inverse );
int fft2d_volume( struct volume_list *volume, int inverse );
int fft3d_stack( int inverse );
int fft_volumes( int idim, int inverse );
void fft_volume( struct volume_list *volume, int idim, int inverse );
void fft_volume_old( struct volume_list *volume, int idim, int inverse );
int smooth_in_kspace( float fwhm, int idim );
int sharpen_in_kspace( float fwhm, int idim );
void flatten_volumes( float fov_fraction );
int ft_1d( int idim, int inverse );
int ft_1d_volume( int idim, int inverse, struct volume_list *volume );
int ft_time();
void apply_2d_phase_roll( struct volume_list *volume, int dir, float slope, float offset );
void hamming_window( int mode );
void exponential_filter_1d( float tau );
void edge_filter();
int analyze_lectin();
void filter_small_islands( struct volume_list *volume, float AreaThreshold);
BOOL voxel_touches_2d_neighbor(struct volume_list  *volume, IPOINT2D voxel, short *mask_voxels);


void clip_stacks( int idim );
int clip_stacks_to_n( int idim, int nclip);
void clip_stack( struct data_stack *stack, int smaller_nx, int smaller_ny, int smaller_nz );
void pad_stacks( int idim );
void pad_stack( struct data_stack *stack, int larger_nx, int larger_ny, int larger_nz, int partial_ky );
void pad_2n_stacks();
void pad_stacks_to_n( int idim, int npad);
int square2d_stacks_up();
int square2d_stacks_down();
int shuffle_stacks( char *new_stack_name );
int slice_volumes( char *new_stack_name, int nslice );
void reverse_slice_order();
int reorder_time_and_slices();
int reorder_slice_time();
int reorder_slices();
int reorder_segments();
void set_slice_order( int norder, int *order );
void reorder_slice_and_phase();
void swap_quadrants_in_volume_list( int mirror, int iswap, int idiv_x, int idiv_y );
void swap_quadrants( struct volume_list *volume, int mirror, int iswap, int idiv_x, int idiv_y );
int add_slices( int nz_add );

int arithmetic_function_constant( int itype, float constant );
int arithmetic_function( int itype );
int arithmetic_function_on_stacks( int itype );
int create_fCBV( int mode, char *new_stack_name );
int absolute_value();

void baseline_correct();
void baseline_correct_volumes();
int magnitude_window();
int magnitude_window_together();
int scale_volumes(float target_max);
int normalize_volumes(float target_average);

int initialize_read_loop( int file_type, char *file_name, char *stack_name, char *list_name );
int store_read_loop_command( char *command_line );
int flush_read_loop_commands();
int create_read_command(int nset, int *set, int number_points_per_interation, int *ndone, char *command);
int macro_reorder_interleaved_slices( int istart );
int macro_read_glm_control_file( char * argument );

int read_xdisplay_file( char *file_name, char *stack_name, char *list_name, float time_step );
int write_output_file( int nifti_format, char *file_name );
int write_slices( char *file_name );
struct volume_file *install_file( char *output_file_name );
int xdisplay_format( char *file_name );

int read_ovl_file( char *file_name );
int write_ovl( char *file_name_in );
void delete_overlay();
int create_bitmap_from_overlay( char *new_stack_name );
int create_overlay_from_bitmap();
int mirror_overlay_in_x( int iDimensionX );

int read_bruker_file( char *file_name, char *stack_name, char *list_name );
int read_Bruker_parameters(char *file_name, int *lNx, int *lNy, int *lNz, int *lRepetitions, int *byte_order);
int separate_parameter_and_value(char *sWholeLine, char *sParameter, char *sValue);

int average_volumes( char *new_stack_name );
int average_stacks( char *new_stack_name );
int average_by_n(char *new_stack_name, int nbin);
int average_epochs(char *new_stack_name, int nt_per_epoch);
int smooth_volumes( float fwhm, int through_plane );
int average_volumes_by_s2( char *new_stack_name );
int average_spectra_by_s2( int istart, int iend, char *new_stack_name );
int weighted_average( char *new_stack_name );
int create_variances( char *new_stack_name );

int ishift_pixels( int ix_shift, int iy_shift, int iz_shift );
void register_volume( BOOL downsample );
void inverse_register_volume( BOOL downsample );
int register_histology( BOOL downsample );
void downsample_stacks();

int do_command(int imacro, char *command_line);
int strip_com_arguments(char *string);
int replace_passed_arguments(char *command_line, int argc, char *argv[]);
int interpret_command( int ilist, char *command );
int act_on_command( int icommand, char *argument );
int act_on_macro( int imacro, char *argument );
void set_variable( int ivariable, char *argument );
void show_variable( int ivariable );
void initialize_global_variables();
void initialize_volume_list();
int read_slice_order( char *argument );
int decode_volume_list( int which_list, char *argument );
int decode_stack_subset( char *stack_and_volumes, struct data_stack **stack, int *nset, int *iset );
int separate_name_and_list( char *name_and_volumes, char *name, char *list_name );
int decode_subset( char *list_name, int ntotal, int *nset, int *iset );
int decode_comparison_list( char *argument );
int decode_string_and_comparison_list(char *argument,char *string );
int decode_int_and_comparison_list(char *argument, int *number);
int decode_read_argument( char *argument, char *file_name, int *file_type,
			  char *stack_name, char *list_name, int *nread );
int decode_read_special( char *argument, char *file_name, char *stack_name );
int decode_string_and_list( int istring, char *argument, char *string );
int decode_int_and_list( char *argument, int *number );
int decode_int_string_and_list( int istring, char *argument, int *number, char *string );
int decode_2strings_and_list( int istring, char *argument, char *string1, char *string2 );
int decode_real_string_and_list( char *argument, float *number, char *string );
int decode_real_string_and_comparison_list( char *argument, float *number, char *string );
int decode_real_and_list( char *argument, float *real_number );
int decode_euler_and_list( char *argument, float *euler );
int decode_2reals_and_list( char *argument, float *real_number1, float *real_number2 );
int decode_4reals_string_and_list( char *argument, float *real_number1, float *real_number2,
				   float *real_number3, float *real_number4, char *string );
int decode_3reals_and_list( char *argument, float *real_number1, float *real_number2, float *real_number3 );
int decode_3reals_string_and_list( char *argument, float *real_number1, float *real_number2,
				   float *real_number3, char *string );
int decode_2ints_string_and_list( char *argument, int *number1, int *number2, char *string );
int read_volume_order( char *argument );
int read_map_array( char *argument );
struct data_stack *install_stack( char *stack_name, struct data_stack *stack_template,
				    IPOINT3D dim, int nt );
void free_stack( struct data_stack *free_me );
void jip_text(int type, char *text);
void print_help_page();

int glm_remove_or_correct_baseline( int mode );
int glm_histogram_unbiased();
int glm_histogram_unbiased_residue();
int glm_create_residue( char *residue_string, char *new_stack_name );
int glm_create_residual_variance( char *new_stack_name );
int glm_create_AR_maps( char *new_stack_name );
int glm_fit_events( char *new_stack_name );
int glm_maximal_effect( char *condition_string, char *new_stack_name );
int glm_assess_global_F_variance( JIP_INDEX n3d, int test_F_old, int event_number,
				  FCOMPLEX *fcData_F_new, FCOMPLEX *fcData_F_old );
int glm_vary_time_constant( int event_number, char *new_stack_name );
int glm_vary_time_constants( int ntime_constants, char *new_stack_name );
int T_or_F_test(int itest, char *new_stack_name);
int fill_histogram();
int fill_weighted_histogram();

int machine_byte_order();

int map_t2( int t2_map, char *new_stack_name );
int map_snr( char *new_stack_name );
int map_t1_from_flip_angles( int tr, char *new_stack_name );
int map_t1_from_multiple_tr( int flip, char *new_stack_name );
void fit_line( int npoints, float *x, float *y, float *slope, float *offset, float *sigma );
void t1_func( float ti, float param[], float *mag, float dydp[], int nparam );
int scatter_plot( char *plot_file_name );
int calculate_overlay_volume_overlap( float smooth_fwhm_mm );
int map_t1_Barral_MRM2010( char *new_stack_name );
void grid_search_t1( int mode, int nt, FCOMPLEX *signal_array, FCOMPLEX *A, FCOMPLEX *B, float *T1_opt);
FCOMPLEX Barral_gamma( int nt, FCOMPLEX *signal_array, double T1 );
double Barral_psi( int nt, FCOMPLEX *signal_array, double T1 );
FCOMPLEX Barral_A( int nt, FCOMPLEX *signal_array, double T1, FCOMPLEX B );


POINT3D Convert_Voxel_to_Space(IPOINT3D DataIndex, struct data_stack *stack);
POINT3D Convert_Space_to_rVoxel(POINT3D Space, struct data_stack *stack);
