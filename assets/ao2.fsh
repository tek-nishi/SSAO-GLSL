//
// AO
// TYPE:Alchemy Ambient Obscurance
//
$version$
$precision$

float PI = 3.14159;

uniform sampler2D	uTex0;
uniform sampler2D	uTex1;
uniform sampler2D	uTex2;
uniform sampler2D	matrix_texture;
uniform vec2 sampOffset[6];

in vec2 TexCoord;

out vec4 oColor;


void main(void) {
  vec3 pos = vec3(TexCoord, texture(uTex2, TexCoord).x);
  vec3 normal = vec3(texture(uTex1, TexCoord));

  // vec4 pallet = vec4(texture(matrix_texture, TexCoord * 128.0));
  // mat2 m = mat2(pallet.x, pallet.y, pallet.z, pallet.w);

  float d = 0.0;
  for (int i = 0; i < 6; ++i) {
    vec2 t = sampOffset[i];
    vec3 p = vec3(TexCoord + t, texture(uTex2, TexCoord + t).x); 

    vec3 v = normalize(p - pos);
    d += min(dot(v, normal), 0.0);
  }
  d = 1.0 + d / 6.0;
  
  // vec4 t = texture(uTex0, TexCoord);
  // oColor = vec4(t.xyz * d, t.w);
  // vec4 v = texture(uTex1, TexCoord);
  // oColor = vec4(v.w, v.w, v.w, 1.0);
  // vec4 v = texture(uTex0, TexCoord);
  // oColor = vec4(v.xyz * d, 1.0);
  oColor = vec4(d, d, d, 1.0);
}
