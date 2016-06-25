//
// Depth of field
//

$version$
$precision$


uniform sampler2D	uTex0;
uniform sampler2D	uTex1;

uniform float blurAmnt;
uniform float focusZ;

in vec2 TexCoord;

out vec4 oColor;

// SOURCE:https://github.com/Jam3/glsl-fast-gaussian-blur
vec4 blur9(sampler2D image, vec2 uv, vec2 direction) {
  vec4 color = vec4(0.0);

  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  
  color += texture(image, uv) * 0.2270270270;
  color += texture(image, uv + off1) * 0.3162162162;
  color += texture(image, uv - off1) * 0.3162162162;
  color += texture(image, uv + off2) * 0.0702702703;
  color += texture(image, uv - off2) * 0.0702702703;

  return color;
}

void main()
{
  // SOURCE:https://github.com/emahub/gaussianBlurFilter
  vec4 color = vec4(0);

  float depth = texture(uTex1, TexCoord).x;
  float blur = abs(depth - focusZ) * blurAmnt;
  oColor = blur9(uTex0, TexCoord, vec2(blur, 0.0));
}
