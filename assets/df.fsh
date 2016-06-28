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


// vec4 GaussianBlur(sampler2D tex0, vec2 centreUV, vec2 pixelOffset) {
//   vec4 colOut = vec4(0);

//   // Kernel width 35 x 35
//   const int stepCount = 9;
//   const float gWeights[stepCount] = float[](
//     0.10855,
//     0.13135,
//     0.10406,
//     0.07216,
//     0.04380,
//     0.02328,
//     0.01083,
//     0.00441,
//     0.00157
//   );
  
//   const float gOffsets[stepCount] = float[](
//     0.66293,
//     2.47904,
//     4.46232,
//     6.44568,
//     8.42917,
//     10.41281,
//     12.39664,
//     14.38070,
//     16.36501
//   );

//   for (int i = 0; i < stepCount; i++) {
//     vec2 texCoordOffset = gOffsets[i] * pixelOffset;
//     vec4 col = texture(tex0, centreUV + texCoordOffset) + texture(tex0, centreUV - texCoordOffset);
//     colOut += gWeights[i] * col;
//   }
    
//   return colOut;
// }


void main() {
  float depth = texture(uTex1, TexCoord).x;
  vec2  blur  = blurAmnt * (depth - focusZ);

  // SOURCE:https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
  // oColor = GaussianBlur(uTex0, TexCoord, blur);
  
  // SOURCE:https://github.com/mattdesl/lwjgl-basics/wiki/ShaderLesson5
  vec4  color = vec4(0);

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
