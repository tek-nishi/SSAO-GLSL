//
// AO
//
$version$

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec2 TexCoord;

void main(void) {
  gl_Position	= ciPosition;
  TexCoord    = ciPosition.xy * 0.5 + 0.5;
}
