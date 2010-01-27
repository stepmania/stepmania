varying vec3 normal, viewVector;
varying vec4 lightVector, lightColor;
varying vec2 texCoords;

void main(void)
{
	texCoords = gl_MultiTexCoord0.st;
	
	vec4 objectPos = gl_ModelViewMatrix * gl_Vertex;

	normal = gl_NormalMatrix * gl_Normal;
	lightVector = normalize(gl_LightSource[0].position);
	lightColor = gl_LightSource[0].diffuse;
	viewVector  = normalize(vec3(0) - objectPos.xyz);
	gl_Position = ftransform();
}