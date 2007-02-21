uniform sampler2D Texture1;
uniform sampler2D Texture2;

vec4 ApplyDodge( vec4 over, vec4 under, float fill )
{
	vec3 NeutralColor = vec3(0,0,0);
	over.rgb = mix( NeutralColor, over.rgb, fill );

	vec4 ret;
	ret.rgb = under.rgb;

	ret.rgb /= max(1.0-over.rgb, 0.000001);
	ret.rgb = min(ret.rgb, 1.0);
	ret.rgb *= over.a*under.a;
//	ret = clamp( ret, 0.0, 1.0 );

	ret.a = over.a * under.a;

	return ret;
}

void main(void)
{
	vec4 under = texture2DProj( Texture1, gl_TexCoord[0] );
	vec4 over = texture2DProj( Texture2, gl_TexCoord[0] );

	vec4 ret = ApplyDodge( over, under, gl_Color.a );

	ret.rgb += (1.0 - over.a) * under.rgb * under.a;
	ret.a += (1.0 - over.a) * under.a;

	over.a *= gl_Color.a;
	ret.rgb += (1.0 - under.a) * over.rgb * over.a;
	ret.a += (1.0 - under.a) * over.a;

	ret.rgb /= ret.a;
	
	gl_FragColor = ret;
	return;
}

/*
 * Copyright (c) 2007 Glenn Maynard
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

