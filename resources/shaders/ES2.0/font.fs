precision mediump float;

uniform sampler2D tex;
uniform vec4 color;

varying vec2 TexCoord;

void main()
{
	float a = texture2D(tex, TexCoord).r;
	a = smoothstep(0.08, 1.0, a);
	if (a == 0.0)
		discard;

	vec4 sampled = vec4(color.xyz, a);
	gl_FragColor = sampled;
}
