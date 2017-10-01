uniform sampler2D Texture1;

varying vec2 texCoords;
varying vec4 vertexColor;

//this should be different maybe?
const float smoothingFactor = 1.0/16.0;

void main(void)
{
	float dfValue = texture2D( Texture1, texCoords ).r;
	dfValue = smoothstep(0.5 - smoothingFactor, 0.5 + smoothingFactor, dfValue);
	gl_FragColor = vec4(vertexColor.rgb, vertexColor.a * dfValue);
}
