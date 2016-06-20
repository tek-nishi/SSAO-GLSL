//
// AO
//
$version$
$precision$

float PI = 3.14159;

uniform sampler2D	uTex0;
uniform sampler2D	uTex1;
                                                          
in vec2 TexCoord;

out vec4 oColor;


void main(void) {
  float d0 = texture(uTex1, TexCoord).w;

  float d = PI * 2 * 4;
  for (int i = 0; i < 4; ++i) {
    float r = (PI * 2) * 4 / i;
    mat2 m = mat2(cos(r), -sin(r), sin(r), cos(r));
    
    vec2  t1 = TexCoord + m * vec2(0.01, 0.0);
    float d1 = max(texture(uTex1, t1).w - d0, 0.0);

    vec2  t2 = TexCoord + m * vec2(-0.01, 0.0f);
    float d2 = max(texture(uTex1, t2).w - d1, 0.0);

    d -= atan(0.01, d1) + atan(0.01, d2);
  }
  d /= (PI * 2.0) * 4.0;

  // vec4 t = texture(uTex0, TexCoord);
  // oColor = vec4(t.xyz * d, t.w);
  // vec4 v = texture(uTex1, TexCoord);
  // oColor = vec4(v.w, v.w, v.w, 1.0);
  vec4 v = texture(uTex1, TexCoord);
  oColor = vec4(v.xyz, 1.0);
}
