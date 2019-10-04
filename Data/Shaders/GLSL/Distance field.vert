//We need this shader to pass the vertex color to the frag shader

varying vec4 vertexColor;
varying vec4 texCoords;

void main(void) {
	vec4 position = vec4(gl_Vertex.xyz + gl_Normal * 0.125, 1.0);
	gl_Position = gl_ModelViewProjectionMatrix * position;
	texCoords = gl_MultiTexCoord0;
	vertexColor = gl_Color;
}
