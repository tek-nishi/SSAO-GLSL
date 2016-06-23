//
// テクスチャ + ライティング
//

$version$
$precision$

uniform sampler2D	uTex0;

in vec2 TexCoord0;
in vec4 Color;
in vec4 Specular;
in vec3 Normal;

layout (location = 0) out vec4 oFragColor0;
layout (location = 1) out vec4 oFragColor1;


void main(void) {
  oFragColor0 = texture(uTex0, TexCoord0) * Color + Specular;
  // oFragColor1 = vec4(Normal, 1.0);
}
