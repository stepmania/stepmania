void main() {
	vec4 position = vec4(gl_Vertex.xyz + gl_Normal * 0.125, 1.0);
	gl_Position = gl_ModelViewProjectionMatrix * position;
}
