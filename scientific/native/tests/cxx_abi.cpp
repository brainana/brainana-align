#include "jip_engine.h"
#include <iostream>
int main() {
    jip_options options{};
    jip_options_default(&options);
    if(options.grid_down!=8||options.affine_parameters[6]!=1)return 1;
    jip_config config{};jip_config_default(&config);
    if(config.struct_size!=sizeof(config)||config.stages!=JIP_STAGE_NONLINEAR||config.affine_model!=6)return 2;
    jip_transform transform{};
    if(jip_engine_get_transform(nullptr,&transform)!=JIP_INVALID||jip_engine_run(nullptr,JIP_STAGE_AFFINE)!=JIP_INVALID)return 3;
    std::cout << "CXX_ABI_RESULT " << JIP_ENGINE_VERSION << " passed\n";
}
