varying vec2 texCoords;
varying vec3 normal, viewVector;
varying vec4 lightVector, lightColor;

uniform sampler2D Texture1;

void main(void)
{
	float intensity = dot(normal, lightVector.xyz);
	float fresnel = pow(1.0 - dot(viewVector, normal), 5.0);
	fresnel = fresnel < 0.5 ? 0.0 : 0.4;
	
	float shade = 1.0;
	if( intensity > 0.35 )
		shade = clamp(1.0 * intensity + 0.4, 0.0, 1.0);
	else
		shade = clamp(0.85 * intensity + 0.25, 0.4, 0.6);
	shade = clamp(shade - fresnel, 0.3, 1.0);
	
	vec4 vcol = gl_FrontMaterial.diffuse;
	vec4 tex = texture2D(Texture1, texCoords);
	
	gl_FragColor = lightColor * vec4(tex.rgb * shade, 1.0);
}