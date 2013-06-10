attribute vec4 TextureMatrixScale;

void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec4 multiplied_tex_coord = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_TexCoord[0] = (multiplied_tex_coord * TextureMatrixScale) +
			(gl_MultiTexCoord0 * (vec4(1)-TextureMatrixScale));
	gl_FrontColor = gl_Color;
}

