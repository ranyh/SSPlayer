#version 330 core

uniform sampler2D tex;
uniform vec4 color;

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
	float a = texture(tex, TexCoord).r;
	a = smoothstep(0.08, 1.0, a);
	if (a == 0.0)
		discard;

	vec4 sampled = vec4(color.xyz, a);
	FragColor = sampled;
}
