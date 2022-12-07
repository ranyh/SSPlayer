#version 330 core

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;

in vec2 TexCoord;
out vec4 FragColor;

void main (void)
{
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture(u_texture0, TexCoord).r;//rgba
    yuv.y = texture(u_texture1, TexCoord).r - 0.5;
    yuv.z = texture(u_texture1, TexCoord).a - 0.5;
    
    rgb = mat3( 1,        1,          1,
                0,        -0.39465,   2.03211,
                1.13983,  -0.58060,   0) * yuv;
    FragColor = vec4(rgb, 1);
}
