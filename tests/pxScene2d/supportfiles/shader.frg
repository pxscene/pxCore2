// FRAGMENT SHADER:  Invert Colors
//
varying vec2       v_uv;
uniform vec2       u_resolution;
uniform sampler2D  s_texture;
void main()
{
  vec4 pix = texture2D(s_texture, v_uv);
  vec3 inv = vec3(1.0) - pix.rgb; // INVERT color
  inv *= pix.a;
  gl_FragColor = vec4(inv, pix.a);
}
