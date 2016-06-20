//
// AO
//
$version$
$precision$

float PI = 3.14159;

uniform sampler2D	uTex0;
uniform sampler2D	uTex1;
uniform sampler2D	uTex2;

uniform float rotate[32];
uniform vec2 sampOffset[6];

in vec2 TexCoord;

out vec4 oColor;


float cross(vec2 a, vec2 b) {
  return a.x * b.y - a.y * b.x;
}

float tangent(vec3 p, vec3 s) {  
  return (p.z - s.z) / length(s.xy - p.xy);  
}  

void main(void) {
  vec3 pos = vec3(TexCoord, texture(uTex2, TexCoord).x);

  // float r = PI * 2 * mod(TexCoord.x + TexCoord.y, 1.0);
  float r = PI * 0.5;
  mat2 m = mat2(cos(r), -sin(r), sin(r), cos(r));

  float d = 0.0;
  for (int i = 0; i < 6; ++i) {
    vec2 t = sampOffset[i] * m;

    vec3 pl = vec3(TexCoord + t, texture(uTex2, TexCoord + t).x); 
    vec3 pr = vec3(TexCoord - t, texture(uTex2, TexCoord - t).x);

    float tl = atan(tangent(pos, pl)); 
    float tr = atan(tangent(pos, pr));
    d += clamp((tl + tr) / PI, 0.0, 1.0);
  }
  d = 1.0 - clamp(d / 6.0, 0.0, 1.0);
  
  // vec4 t = texture(uTex0, TexCoord);
  // oColor = vec4(t.xyz * d, t.w);
  // vec4 v = texture(uTex1, TexCoord);
  // oColor = vec4(v.w, v.w, v.w, 1.0);
  vec4 v = texture(uTex0, TexCoord);
  // oColor = vec4(v.xyz * d, 1.0);
  oColor = vec4(d, d, d, 1.0);
}
