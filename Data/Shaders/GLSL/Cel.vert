varying vec2 vCoord;
varying vec3 vNor;

void main() {
	vCoord = gl_MultiTexCoord0.st;
	vNor = gl_Normal.xyz;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
