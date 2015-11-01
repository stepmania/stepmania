uniform sampler2D Texture1;
uniform sampler2D color_mask;
uniform vec3 playercolor;

void main(void)
{
    vec4 ret = texture2D(Texture1, gl_TexCoord[0].st);
		vec4 mask = texture2D(color_mask, gl_TexCoord[0].st);
    ret.rgb = mix(playercolor, ret.rgb, 1.0 - mask.a);
    gl_FragColor = ret;
}
