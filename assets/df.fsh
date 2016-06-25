//
// Depth of field
//

$version$
$precision$


uniform sampler2D	uTex0;
uniform sampler2D	uTex1;

uniform vec2 blurAmnt;
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
  vec2 blur = blurAmnt * abs(depth - focusZ);
  // oColor = blur9(uTex0, TexCoord, blur);

  color += texture(uTex0, TexCoord - 4.0 * blur) * 0.0162162162;
  color += texture(uTex0, TexCoord - 3.0 * blur) * 0.0540540541;
  color += texture(uTex0, TexCoord - 2.0 * blur) * 0.1216216216;
  color += texture(uTex0, TexCoord -       blur) * 0.1945945946;

  color += texture(uTex0, TexCoord) * 0.2270270270;

  color += texture(uTex0, TexCoord +       blur) * 0.1945945946;
  color += texture(uTex0, TexCoord + 2.0 * blur) * 0.1216216216;
  color += texture(uTex0, TexCoord + 3.0 * blur) * 0.0540540541;
  color += texture(uTex0, TexCoord + 4.0 * blur) * 0.0162162162;

  oColor = color;
}
