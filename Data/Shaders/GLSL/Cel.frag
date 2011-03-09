uniform sampler2D Texture0;

varying vec2 vCoord;
varying vec3 vNor;

const vec4 shadowColor = vec4(0.6, 0.75, 0.9, 1.0);

void main() {
	vec3 nor, light;
	nor = normalize(vNor);
	
	vec4 diffuse, specular, color, lightSource;
	float ambient = length(gl_FrontMaterial.ambient.rgb);
	lightSource = gl_LightSource[0].position;
	light = normalize(gl_ModelViewMatrix * lightSource).xyz;
	
	color = texture2D(Texture0, vCoord.st);
	float intensity = max(dot(light,nor), 0.0);
	if (intensity < 0.5) {
		intensity *= 0.5;
		color *= shadowColor;
	}
	intensity = min(clamp(intensity, ambient, 1.0) + 0.25, 1.0);
	
	gl_FragColor = color * intensity;
	
	// Don't allow transparency. Bad Things will happen.
	gl_FragColor.a = 1.0;
}
