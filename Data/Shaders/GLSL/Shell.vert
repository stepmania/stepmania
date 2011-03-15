void main() {
	vec4 position = gl_Vertex + vec4(gl_Normal, 1.0) * 0.125;
	// prevent skewing
	position.w = 1.0;
	
	gl_Position = gl_ModelViewProjectionMatrix * position;
}
