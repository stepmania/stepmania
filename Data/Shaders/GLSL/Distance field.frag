uniform sampler2D Texture1;

varying vec4 texCoords;
varying vec4 vertexColor;

float aastep(float threshold, float dist) {
	float afwidth = 0.75 * length(vec2(dFdx(dist), dFdy(dist)));
	return smoothstep(threshold - afwidth, threshold + afwidth, dist);
}

float median(float r, float g, float b) {
	return max(min(r, g), min(max(r, g), b));
}

const float distanceThreshold = 0.5;

float sample(vec2 tc) {
	vec3 texel = texture2D(Texture1, tc).rgb;
	float dist = median(texel.r, texel.g, texel.b);
	return aastep(distanceThreshold, dist);
}

void main(void)
{
	float dfValue = sample(texCoords.st);
	gl_FragColor = vec4(vertexColor.rgb, vertexColor.a * dfValue);
}
