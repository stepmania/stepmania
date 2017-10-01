//We need this shader to pass the vertex color to the frag shader

varying vec4 vertexColor;
varying vec2 texCoords;

void main(void) {
	vertexColor = gl_Color;
	texCoords = gl_MultiTexCoord0.st;
	gl_Position = gl_Vertex * gl_ModelViewProjectionMatrix;
}
