/* Public ABI bridge. Numerical routines above retain their original order. */
static int image_valid(const jip_image *im) {
  uint64_t n=1;
  if(!im||!im->data)return 0;
  for(int a=0;a<3;a++) {
    if(im->dim[a]<2||!isfinite(im->spacing[a])||im->spacing[a]<=0)return 0;
    if((double)im->dim[a]*im->spacing[a]>FLT_MAX)return 0;
    if(n>(uint64_t)PTRDIFF_MAX/sizeof(float)/(uint32_t)im->dim[a])return 0;
    n*=(uint32_t)im->dim[a];
  }
  if(n!=im->count)return 0;
  for(int i=0;i<16;i++)if(!isfinite(im->native_affine[i]))return 0;
  if(im->native_affine[12]!=0||im->native_affine[13]!=0||im->native_affine[14]!=0||im->native_affine[15]!=1)return 0;
  const float *m=im->native_affine;
  double det=(double)m[0]*(m[5]*m[10]-m[6]*m[9])-(double)m[1]*(m[4]*m[10]-m[6]*m[8])+(double)m[2]*(m[4]*m[9]-m[5]*m[8]);
  if(!isfinite(det)||fabs(det)<1.e-12)return 0;
  float minimum=INFINITY,maximum=-INFINITY;
  for(uint64_t i=0;i<n;i++) {
    float v=im->data[i];if(!isfinite(v))return 0;
    if(v<minimum)minimum=v;if(v>maximum)maximum=v;
  }
  /* The legacy full-volume stopping metric does not define a histogram for
     uniform/nonfinite images. Reject those inputs at the new API boundary. */
  if(!(maximum>minimum))return 0;
  return 1;
}
static struct data_stack image_copy(const jip_image *im) {
  struct data_stack s={0};
  s.dim=(IPOINT3D){im->dim[0],im->dim[1],im->dim[2]};
  s.resolution=(POINT3D){im->spacing[0],im->spacing[1],im->spacing[2]};
  s.direction=(POINT3D){1,1,1};s.fov=(POINT3D){s.dim.x*s.resolution.x,s.dim.y*s.resolution.y,s.dim.z*s.resolution.z};
  s.n2d=(JIP_INDEX)s.dim.x*s.dim.y;s.n3d=s.n4d=s.npts_volume=(JIP_INDEX)im->count;s.nt=1;
  memcpy(s.ijk_to_xyz.m,im->native_affine,sizeof(s.ijk_to_xyz.m));s.xyz_to_ijk=invert_mat44(s.ijk_to_xyz);
  s.SpaceTime.fData=float_vector(s.n3d);memcpy(s.SpaceTime.fData,im->data,(size_t)s.n3d*sizeof(float));return s;
}
static jip_image image_view(struct data_stack *s) {
  jip_image im={{s->dim.x,s->dim.y,s->dim.z},{s->resolution.x,s->resolution.y,s->resolution.z},{0},s->SpaceTime.fData,(uint64_t)s->n3d};
  memcpy(im.native_affine,s->ijk_to_xyz.m,sizeof(im.native_affine));return im;
}
void jip_options_default(jip_options *o) {
  if(!o)return;memset(o,0,sizeof(*o));o->grid_down=8;o->regularization=5;o->jacobian_limit=2;
  o->affine_parameters[6]=o->affine_parameters[7]=o->affine_parameters[8]=1;
}
void jip_config_default(jip_config *config) {
  if(!config)return;memset(config,0,sizeof(*config));config->struct_size=sizeof(*config);
  config->stages=JIP_STAGE_NONLINEAR;config->affine_model=6;
  for(int a=0;a<3;a++)config->nonlinear_axes[a]=1;
}
static int stages_valid(uint32_t stages,const jip_config *config) {
  return stages&&!(stages&~(uint32_t)(JIP_STAGE_AFFINE|JIP_STAGE_NONLINEAR))&&
    (!(stages&JIP_STAGE_NONLINEAR)||config->nonlinear_axes[0]||config->nonlinear_axes[1]||config->nonlinear_axes[2]);
}
static int config_valid(const jip_config *config) {
  if(!config||config->struct_size!=sizeof(*config)||!stages_valid(config->stages,config))return 0;
  if(config->affine_model!=6&&config->affine_model!=9&&config->affine_model!=12)return 0;
  if(config->affine_metric!=JIP_AFFINE_MI&&config->affine_metric!=JIP_AFFINE_NMI)return 0;
  if(config->matching_volume<0||config->matching_volume>1)return 0;
  for(int a=0;a<3;a++)if(config->nonlinear_axes[a]<0||config->nonlinear_axes[a]>1||config->wrap_around[a]<0||config->wrap_around[a]>1)return 0;
  return 1;
}
static int options_valid(const jip_options *o) {
  if(!o||(o->metric!=0&&o->metric!=1)||(o->grid_down!=1&&o->grid_down!=2&&o->grid_down!=4&&o->grid_down!=8&&o->grid_down!=16))return 0;
  if(o->regularization<0||o->regularization>46340||!isfinite(o->jacobian_limit)||o->jacobian_limit<=1||!isfinite(o->stiffness)||o->stiffness<0||o->stiffness>1)return 0;
  for(int i=0;i<12;i++)if(!isfinite(o->affine_parameters[i]))return 0;
  for(int i=6;i<9;i++)if(o->affine_parameters[i]<=0)return 0;
  return 1;
}
enum {RUN_IDLE,RUN_ACTIVE,RUN_CANCELLED,RUN_COMMITTING};
_Static_assert(ATOMIC_INT_LOCK_FREE==2,"cancellation requires lock-free int atomics");
static void bind_stack(jip_engine *c) {c->mosaic_state.stack=&c->target_stack;c->mosaic_state.fSpaceMatrix=c->target_stack.SpaceTime.fData;}
static int enter_context(jip_engine *c,jip_engine **previous) {
  if(!c||atomic_flag_test_and_set(&c->busy))return 0;
  *previous=jip_active;jip_active=c;return 1;
}
static void leave_context(jip_engine *c,jip_engine *previous) {jip_active=previous;atomic_flag_clear(&c->busy);}
static jip_status record_error(jip_engine *c,jip_status code,const char *phase,const char *message) {
  c->last_error=(jip_error){0};c->last_error.code=code;
  snprintf(c->last_error.phase,sizeof(c->last_error.phase),"%s",phase);
  snprintf(c->last_error.message,sizeof(c->last_error.message),"%s",message);return code;
}
static jip_engine *create_body(const jip_image *fixed,const jip_image *moving,const jip_options *o,const jip_config *config,const jip_sampling *sampling,jip_operation *op) {
  if(!image_valid(fixed)||!image_valid(moving)||!options_valid(o)||!config_valid(config))jip_runtime_raise(JIP_INVALID,"invalid image geometry, intensities or registration options",0);
  for(int a=0;a<3;a++)if((double)fixed->spacing[a]/moving->spacing[a]>(INT_MAX-2)/2)
    jip_runtime_raise(JIP_INVALID,"interpolation geometry exceeds supported integer kernel range",0);
  jip_engine *c=jip_runtime_handle_new(sizeof(*c));op->handle=c;op->arena=&c->arena;jip_active=c;
  atomic_flag_clear(&c->busy);atomic_init(&c->run_state,RUN_IDLE);c->owner=c;c->options=*o;c->config=*config;
  c->saved_arithmetic=JIP_ARITHMETIC_ID;c->saved_architecture=JIP_ARCHITECTURE_ID;
  c->first_nonlinear=c->first_affine=c->first_affine_iteration=c->first_target=c->first_source=c->first_basis=TRUE;
  template=image_copy(fixed);source_unfiltered=image_copy(moving);mosaic.stack=&template;
  initialize_alignment();AlignAid.CostFunction=o->metric;AlignNL.Lambda_reg=o->regularization;AlignNL.Jacob_limits=o->jacobian_limit;AlignNL.stiffness=o->stiffness;
  AlignAid.matching_volume=config->matching_volume;
  for(int a=0;a<3;a++)AlignNL.Adjust.direction[a]=config->nonlinear_axes[a];
  Alignment.WrapAround=(IPOINT3D){config->wrap_around[0],config->wrap_around[1],config->wrap_around[2]};
  SetGridDown(o->grid_down);initialize_data_stacks();memcpy(Alignment.value,o->affine_parameters,sizeof(Alignment.value));
  define_affine_matrix();
  for(int a=0;a<4;a++)for(int b=0;b<4;b++)if(!isfinite(Alignment.Affine[a][b])||!isfinite(Alignment.AffineI[a][b]))
    jip_runtime_raise(JIP_INVALID,"affine initialization is not finite",0);
  if(sampling) {
    if(sampling->struct_size!=sizeof(*sampling)||(!sampling->fixed&&sampling->fixed_count)||(!sampling->moving&&sampling->moving_count)||
       (sampling->fixed&&sampling->fixed_count!=fixed->count)||(sampling->moving&&sampling->moving_count!=moving->count))
      jip_runtime_raise(JIP_INVALID,"invalid registration sampling geometry",0);
    const uint8_t *input[]={sampling->fixed,sampling->moving};uint8_t **dest[]={&c->fixed_selection,&c->moving_selection};uint64_t sizes[]={fixed->count,moving->count};
    for(int side=0;side<2;side++)if(input[side]){
      *dest[side]=malloc((size_t)sizes[side]);uint64_t included=0;
      for(uint64_t i=0;i<sizes[side];i++){if(input[side][i]>1)jip_runtime_raise(JIP_INVALID,"sampling mask must contain only 0 or 1",0);(*dest[side])[i]=input[side][i];included+=input[side][i];}
      if(!included)jip_runtime_raise(JIP_INVALID,"registration sampling mask is empty",0);
    }
    if(sampling->moving_window_count>3||(sampling->moving_window_count&&!sampling->moving_windows))jip_runtime_raise(JIP_INVALID,"invalid moving windows",0);
    c->moving_window_count=sampling->moving_window_count;
    for(uint32_t w=0;w<c->moving_window_count;w++){
      jip_window v=sampling->moving_windows[w];if(!v.axes||v.axes>7)jip_runtime_raise(JIP_INVALID,"invalid window axes",0);
      for(int a=0;a<3;a++)if(!isfinite(v.minimum[a])||!isfinite(v.maximum[a])||((v.axes&(1u<<a))&&v.minimum[a]>v.maximum[a]))jip_runtime_raise(JIP_INVALID,"invalid physical window bounds",0);
      c->moving_windows[w]=v;
    }
    if(c->fixed_selection||c->moving_selection||c->moving_window_count)c->sample_selection=malloc((size_t)fixed->count);
  }
  AlignAid.xyz_to_ijk=centered_xyz_to_ijk(source);transform_coordinates(1,0,1);
  c->result_objective=cost_function_nonlinear(0);return c;
}
static void discard_handle(jip_engine *c) {if(c){if(c->owner==c)jip_accel_destroy(c->accelerator);jip_arena_clear(&c->arena);jip_runtime_handle_free(c);}}
jip_status jip_engine_create_sampled(const jip_image *fixed,const jip_image *moving,const jip_options *o,const jip_config *config,const jip_sampling *sampling,jip_engine **out) {
  if(!out)return JIP_INVALID;*out=NULL;
  jip_engine *previous=jip_active;jip_operation *op=jip_runtime_begin("create");if(!op)return JIP_MEMORY;
  if(setjmp(op->recover)) {
    jip_status status=op->error.code;discard_handle(op->handle);jip_active=previous;jip_runtime_end(op);return status;
  }
  *out=create_body(fixed,moving,o,config,sampling,op);op->handle=NULL;jip_active=previous;jip_runtime_end(op);return JIP_OK;
}
jip_status jip_engine_create_configured(const jip_image *fixed,const jip_image *moving,const jip_options *o,const jip_config *config,jip_engine **out) {
  return jip_engine_create_sampled(fixed,moving,o,config,NULL,out);
}
jip_status jip_engine_create(const jip_image *fixed,const jip_image *moving,const jip_options *o,jip_engine **out) {
  jip_config config;jip_config_default(&config);return jip_engine_create_configured(fixed,moving,o,&config,out);
}
jip_status jip_engine_get_config(jip_engine *c,jip_config *config) {
  jip_engine *previous;if(!c||!config)return JIP_INVALID;if(!enter_context(c,&previous))return JIP_BUSY;
  *config=c->config;leave_context(c,previous);return JIP_OK;
}
jip_status jip_engine_get_transform(jip_engine *c,jip_transform *out) {
  jip_engine *previous;if(!c||!out)return JIP_INVALID;if(!enter_context(c,&previous))return JIP_BUSY;
  memset(out,0,sizeof(*out));out->completed_stages=c->completed_stages;
  memcpy(out->affine_parameters,Alignment.value,sizeof(out->affine_parameters));
  memcpy(out->centered_pull_affine,Alignment.Affine,sizeof(out->centered_pull_affine));
  memcpy(out->fixed_voxel_to_native_mm,template.ijk_to_xyz.m,sizeof(out->fixed_voxel_to_native_mm));
  memcpy(out->moving_voxel_to_native_mm,source_unfiltered.ijk_to_xyz.m,sizeof(out->moving_voxel_to_native_mm));
  memcpy(out->fixed_voxel_to_centered_mm,AlignAid.ijk_to_xyz.m,sizeof(out->fixed_voxel_to_centered_mm));
  memcpy(out->moving_centered_mm_to_voxel,AlignAid.xyz_to_ijk.m,sizeof(out->moving_centered_mm_to_voxel));
  out->displacement[0]=distort_x.SpaceTime.fData;out->displacement[1]=distort_y.SpaceTime.fData;out->displacement[2]=distort_z.SpaceTime.fData;
  out->count=(uint64_t)target.n3d;out->nonlinear_objective=c->result_objective;out->affine_objective=c->affine_objective;
  leave_context(c,previous);return JIP_OK;
}
jip_status jip_engine_release(jip_engine *c) {
  if(!c)return JIP_OK;if(atomic_flag_test_and_set(&c->busy))return JIP_BUSY;
  discard_handle(c);return JIP_OK;
}
void jip_engine_destroy(jip_engine *c) {(void)jip_engine_release(c);}
jip_status jip_engine_set_trace(jip_engine *c,jip_trace_callback cb,void *user) {
  jip_engine *p;if(!enter_context(c,&p))return c?JIP_BUSY:JIP_INVALID;
  c->trace=cb;c->trace_user=user;leave_context(c,p);return JIP_OK;
}
jip_status jip_engine_get_error(jip_engine *c,jip_error *error) {
  jip_engine *p;if(!c||!error)return JIP_INVALID;if(!enter_context(c,&p))return JIP_BUSY;
  *error=c->last_error;leave_context(c,p);return JIP_OK;
}
jip_status jip_engine_request_cancel(jip_engine *c) {
  if(!c)return JIP_INVALID;
  int state=atomic_load(&c->run_state);
  for(;;) {
    if(state==RUN_CANCELLED)return JIP_OK;
    if(state!=RUN_ACTIVE)return state==RUN_COMMITTING?JIP_BUSY:JIP_INVALID;
    if(atomic_compare_exchange_weak(&c->run_state,&state,RUN_CANCELLED))return JIP_OK;
  }
}
static int registration_cancelled(void *c) {return atomic_load(&((jip_engine*)c)->run_state)==RUN_CANCELLED;}
static void injected_legacy_failure(void) {(void)index4d(-1,0,0,0);}
static void *copy_buffer(const void *data,JIP_INDEX count,size_t width) {
  size_t bytes=jip_bytes(count,width);void *copy=malloc(bytes);memcpy(copy,data,bytes);return copy;
}
static float **copy_basis(float **old,JIP_INDEX rows,JIP_INDEX cols) {
  float **copy=float_matrix(rows,cols);for(JIP_INDEX i=0;i<rows;i++)memcpy(copy[i],old[i],jip_bytes(cols,sizeof(float)));return copy;
}
static jip_engine *clone_attempt(jip_engine *c,jip_operation *op) {
  jip_runtime_phase("clone accepted registration");
  jip_engine *trial=jip_runtime_handle_new(sizeof(*trial));op->handle=trial;op->arena=&trial->arena;
  /* Only the numerical prefix is copied. Atomics/control/error state are never
     memcpy'd while a cancellation request may be arriving on another thread. */
  memcpy(trial,c,offsetof(jip_engine,busy));
  atomic_flag_clear(&trial->busy);atomic_init(&trial->run_state,RUN_IDLE);
  trial->owner=c;trial->options=c->options;trial->config=c->config;trial->trace=c->trace;trial->trace_user=c->trace_user;
  trial->accelerator=c->accelerator;trial->profile=(jip_profile){0};
  trial->saved_arithmetic=c->saved_arithmetic;trial->saved_architecture=c->saved_architecture;
  struct data_stack *mutable[]={&trial->mimic_stack,&trial->bases_stack,&trial->dx,&trial->dy,&trial->dz,&trial->jac};
  for(size_t i=0;i<sizeof(mutable)/sizeof(*mutable);i++)mutable[i]->SpaceTime.fData=copy_buffer(mutable[i]->SpaceTime.fData,mutable[i]->n4d,sizeof(float));
  if(trial->sample_selection)trial->sample_selection=copy_buffer(trial->sample_selection,trial->target_stack.n3d,1);
  struct jip_align_nl *n=&trial->nl;JIP_INDEX count=n->NBasis;
#define COPY_MEMBER(member) n->member=copy_buffer(n->member,count,sizeof(*n->member))
  COPY_MEMBER(Index3DStart);COPY_MEMBER(Index3DEnd);COPY_MEMBER(iBasis1D);COPY_MEMBER(NeighborLow);COPY_MEMBER(NeighborHigh);
  COPY_MEMBER(value);COPY_MEMBER(Scale);COPY_MEMBER(Adjust3D);COPY_MEMBER(AdjustOrder);
#undef COPY_MEMBER
  n->BasisX=copy_basis(n->BasisX,n->NBasis1D.x,trial->basis_target_dim.x);
  n->BasisY=copy_basis(n->BasisY,n->NBasis1D.y,trial->basis_target_dim.y);
  n->BasisZ=copy_basis(n->BasisZ,n->NBasis1D.z,trial->basis_target_dim.z);
  /* Fixed, original moving, filtered moving and target voxel buffers remain
     read-only and borrowed from c, which stays locked/alive until completion. */
  bind_stack(trial);return trial;
}
static void release_mutable(jip_engine *c) {
  free(c->sample_selection);
  struct data_stack *mutable[]={&c->mimic_stack,&c->bases_stack,&c->dx,&c->dy,&c->dz,&c->jac};
  for(size_t i=0;i<sizeof(mutable)/sizeof(*mutable);i++)free(mutable[i]->SpaceTime.fData);
  struct jip_align_nl *n=&c->nl;
  free(n->Index3DStart);free(n->Index3DEnd);free(n->iBasis1D);free(n->NeighborLow);free(n->NeighborHigh);
  free(n->value);free(n->Scale);free(n->Adjust3D);free(n->AdjustOrder);
  free_float_matrix(n->BasisX,n->NBasis1D.x,c->basis_target_dim.x);
  free_float_matrix(n->BasisY,n->NBasis1D.y,c->basis_target_dim.y);
  free_float_matrix(n->BasisZ,n->NBasis1D.z,c->basis_target_dim.z);
}
static void verify_attempt_ownership(jip_engine *trial) {
  /* The supported nonlinear path retains exactly these mutable allocations.
     Catch forgotten temporaries before transferring ownership at commit.
     New persistent engine fields must update cloning, release and this check. */
  uint64_t bytes=0,blocks=0;
  if(trial->sample_selection){bytes+=(uint64_t)trial->target_stack.n3d;blocks++;}
  struct data_stack *stacks[]={&trial->mimic_stack,&trial->bases_stack,&trial->dx,&trial->dy,&trial->dz,&trial->jac};
  for(size_t i=0;i<sizeof(stacks)/sizeof(*stacks);i++){bytes+=jip_bytes(stacks[i]->n4d,sizeof(float));blocks++;}
  struct jip_align_nl *n=&trial->nl;
#define COUNT_MEMBER(member) do{bytes+=jip_bytes(n->NBasis,sizeof(*n->member));blocks++;}while(0)
  COUNT_MEMBER(Index3DStart);COUNT_MEMBER(Index3DEnd);COUNT_MEMBER(iBasis1D);COUNT_MEMBER(NeighborLow);COUNT_MEMBER(NeighborHigh);
  COUNT_MEMBER(value);COUNT_MEMBER(Scale);COUNT_MEMBER(Adjust3D);COUNT_MEMBER(AdjustOrder);
#undef COUNT_MEMBER
  int rows[]={n->NBasis1D.x,n->NBasis1D.y,n->NBasis1D.z};int cols[]={trial->basis_target_dim.x,trial->basis_target_dim.y,trial->basis_target_dim.z};
  for(int a=0;a<3;a++){bytes+=jip_bytes(rows[a],sizeof(float*));bytes+=jip_bytes(jip_product(rows[a],cols[a]),sizeof(float));blocks+=1+(uint64_t)rows[a];}
  if(trial->arena.bytes!=bytes||trial->arena.blocks!=blocks)jip_runtime_raise(JIP_INTERNAL,"unexpected temporary allocations remain at registration commit",0);
}
jip_status jip_engine_run(jip_engine *c,uint32_t stages) {
  jip_engine *previous;if(!enter_context(c,&previous))return c?JIP_BUSY:JIP_INVALID;
  if(!stages_valid(stages,&c->config)){leave_context(c,previous);return JIP_INVALID;}
  if(c->sample_selection){
    uint64_t usable=0;float low=INFINITY,high=-INFINITY;
    for(JIP_INDEX i=0;i<c->target_stack.n3d;i++)if(c->sample_selection[i]&&(c->config.matching_volume||c->target_stack.SpaceTime.fData[i]!=0)){
      float v=c->target_stack.SpaceTime.fData[i];if(v<low)low=v;if(v>high)high=v;usable++;
    }
    if(usable<64||!(high>low)){record_error(c,JIP_INVALID,"registration region","Too few usable or varying fixed-image samples inside the registration regions. Enlarge the regions or adjust exclusion masks.");leave_context(c,previous);return JIP_INVALID;}
  }
  int same_policy=(c->saved_arithmetic==JIP_ARITHMETIC_ID||(!c->saved_arithmetic&&JIP_ARITHMETIC_ID==1))&&
    (JIP_ARITHMETIC_ID==2||!c->saved_architecture||c->saved_architecture==JIP_ARCHITECTURE_ID);
  int new_nonlinear=stages==JIP_STAGE_NONLINEAR&&!(c->completed_stages&JIP_STAGE_NONLINEAR)&&c->first_nonlinear;
  if(!same_policy&&!new_nonlinear){record_error(c,JIP_INVALID,"arithmetic policy","Checkpoint arithmetic differs from this engine. Export the saved map, or start a new nonlinear fit from linear alignment.");leave_context(c,previous);return JIP_INVALID;}
  double profile_start=jip_monotonic_seconds();
  c->profile=(jip_profile){0};
  atomic_store(&c->run_state,RUN_ACTIVE);
  c->source_epoch++;
  jip_operation *op=jip_runtime_begin("registration setup");
  if(!op){record_error(c,JIP_MEMORY,"registration setup","cannot allocate operation guard");atomic_store(&c->run_state,RUN_IDLE);leave_context(c,previous);return JIP_MEMORY;}
  if(setjmp(op->recover)) {
    jip_status status=op->error.code;c->last_error=op->error;discard_handle(op->handle);
    atomic_store(&c->run_state,RUN_IDLE);jip_runtime_end(op);leave_context(c,previous);return status;
  }
  op->cancelled=registration_cancelled;op->cancel_user=c;
  op->injected_legacy_failure=injected_legacy_failure;
  jip_engine *trial=clone_attempt(c,op);jip_active=trial;jip_runtime_phase("nonlinear registration");
  if(stages&JIP_STAGE_AFFINE) {
    jip_runtime_phase("automatic affine initialization");
    AlignAid.CostFunction=trial->config.affine_metric;
    for(int i=0;i<NAFFINE;i++)AlignAid.Adjust[i]=i<trial->config.affine_model;
    /* Full-resolution legacy mode retains the original parameter hierarchy.
       Resolution changes are deliberately outside this borrowed-input trial. */
    action_auto_adjust_affine(2);
    for(int i=0;i<NAFFINE;i++)if(!isfinite(Alignment.value[i])||(i>=6&&i<9&&Alignment.value[i]<=0))
      jip_runtime_raise(JIP_INTERNAL,"affine initialization produced invalid or reflected scale",0);
    for(int a=0;a<4;a++)for(int b=0;b<4;b++)if(!isfinite(Alignment.Affine[a][b])||!isfinite(Alignment.AffineI[a][b]))
      jip_runtime_raise(JIP_INTERNAL,"affine initialization produced an invalid matrix",0);
    trial->affine_objective=cost_function();
    if(!isfinite(trial->affine_objective))jip_runtime_raise(JIP_INTERNAL,"affine initialization produced a nonfinite objective",0);
    trial->completed_stages|=JIP_STAGE_AFFINE;engine_trace(6);
    AlignAid.CostFunction=trial->options.metric;
  }
  if(stages&JIP_STAGE_NONLINEAR) {
    jip_runtime_phase("nonlinear registration");action_auto_adjust_nonlinear(1);
    trial->completed_stages|=JIP_STAGE_NONLINEAR;
  }
  engine_trace(4);trial->result_objective=cost_function_nonlinear(0);
  if(!isfinite(trial->result_objective))jip_runtime_raise(JIP_INTERNAL,"registration produced a nonfinite objective",0);
  verify_attempt_ownership(trial);
  jip_runtime_poll();int expected=RUN_ACTIVE;
  /* Linearization point: cancellation wins before this exchange, or the whole
     accepted state commits. No allocation/poll/callback occurs after it. */
  if(!atomic_compare_exchange_strong(&c->run_state,&expected,RUN_COMMITTING))jip_runtime_raise(JIP_CANCELLED,"registration cancelled before commit",0);
  release_mutable(c);memcpy(c,trial,offsetof(jip_engine,busy));bind_stack(c);jip_arena_move(&c->arena,&trial->arena);
  c->profile=trial->profile;c->profile.run_seconds=jip_monotonic_seconds()-profile_start;
  jip_runtime_handle_free(trial);op->handle=NULL;c->last_error=(jip_error){0};
  c->saved_arithmetic=JIP_ARITHMETIC_ID;c->saved_architecture=JIP_ARCHITECTURE_ID;
  atomic_store(&c->run_state,RUN_IDLE);jip_runtime_end(op);leave_context(c,previous);return JIP_OK;
}
jip_status jip_engine_register(jip_engine *c) {
  /* Configuration is immutable throughout handle lifetime. */
  if(!c)return JIP_INVALID;return jip_engine_run(c,c->config.stages);
}
jip_status jip_engine_result(jip_engine *c,jip_image *im,float *objective) {
  jip_engine *p;if(!c||!im)return JIP_INVALID;if(!enter_context(c,&p))return JIP_BUSY;
  *im=image_view(&mimic);if(objective)*objective=c->result_objective;
  leave_context(c,p);return JIP_OK;
}
const char *jip_engine_status_string(jip_status s) {
  switch(s){case JIP_OK:return "success";case JIP_INVALID:return "invalid input";case JIP_IO:return "file input/output failed";case JIP_BUSY:return "context is already active";case JIP_CANCELLED:return "cancelled; accepted result preserved";case JIP_MEMORY:return "insufficient memory";case JIP_INTERNAL:return "numerical engine error";default:return "unknown error";}
}
#include "engine_io.inc"
