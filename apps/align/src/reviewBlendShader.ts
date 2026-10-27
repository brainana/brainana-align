/** Two-volume review only. The control uniform weights the moving texture, never the fixed image. */
export const REVIEW_BLEND_SHADER=`#version 300 es
precision highp float;
precision highp int;
uniform highp sampler3D volume;
uniform highp sampler3D overlay;
uniform highp sampler3D drawing;
uniform highp sampler3D paqd;
uniform highp sampler2D colormap;
uniform vec4 paqdUniforms;
uniform float overlays;
uniform float opacity;
uniform float overlayAlphaShader;
uniform float overlayOutlineWidth;
uniform float drawOpacity;
uniform float drawRimOpacity;
uniform int axCorSag;
uniform bool isAlphaClipDark;
uniform int backgroundMasksOverlays;
in vec3 texPos;
out vec4 color;
void main() {
  vec4 fixedSample=texture(volume,texPos);
  vec4 movingSample=vec4(0.0);
  if(overlays>0.0) movingSample=texture(overlay,texPos);
  // Reserved endpoints: pure-image inspection must not reveal the other image.
  if(overlayAlphaShader<0.0){color=vec4(fixedSample.rgb,isAlphaClipDark && fixedSample.a==0.0 ? 0.0 : clamp(opacity,0.0,1.0));return;}
  if(overlayAlphaShader>1.0){color=movingSample.a>0.0 ? vec4(movingSample.rgb,1.0) : vec4(0.0,0.0,0.0,1.0);return;}
  float fixedAlpha=isAlphaClipDark && fixedSample.a==0.0 ? 0.0 : clamp(opacity,0.0,1.0);
  float movingAlpha=clamp(movingSample.a*overlayAlphaShader,0.0,1.0);
  if(backgroundMasksOverlays>0 && fixedSample.a==0.0) movingAlpha=0.0;
  float outAlpha=fixedAlpha+movingAlpha*(1.0-fixedAlpha);
  float movingWeight=outAlpha>0.0 ? movingAlpha/outAlpha : 0.0;
  color=vec4(mix(fixedSample.rgb,movingSample.rgb,movingWeight),outAlpha);
}`
