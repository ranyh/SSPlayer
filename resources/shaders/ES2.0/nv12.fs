precision highp float;

varying vec2 TexCoord;
uniform sampler2D u_texture0;
uniform sampler2D u_texture1;

void main (void)
{
    vec3 yuv;
    vec3 rgb;
    yuv.x = texture2D(u_texture0, TexCoord).r;//rgba
    yuv.y = texture2D(u_texture1, TexCoord).r - 0.5;
    yuv.z = texture2D(u_texture1, TexCoord).a - 0.5;
    
    rgb = mat3( 1,        1,          1,
                0,        -0.39465,   2.03211,
                1.13983,  -0.58060,   0) * yuv;
    gl_FragColor = vec4(rgb, 1);
}
