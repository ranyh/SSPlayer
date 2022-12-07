precision mediump float;

uniform sampler2D u_texture0;
uniform sampler2D u_texture1;
uniform sampler2D u_texture2;
uniform float u_chroma_div_w;
uniform float u_chroma_div_h;

varying vec2 TexCoord;

vec4 yuvToRGBA(sampler2D u_texture0,
        sampler2D u_texture1,
        sampler2D u_texture2,
        vec2 texture_coordinate,
        float u_chroma_div_w,
        float u_chroma_div_h)
{
    vec3 yuv;

    yuv.r = texture2D(u_texture0, texture_coordinate).r - 0.0625;
    yuv.g = texture2D(u_texture1, vec2(texture_coordinate.x / u_chroma_div_w, texture_coordinate.y / u_chroma_div_h)).r - 0.5;
    yuv.b = texture2D(u_texture2, vec2(texture_coordinate.x / u_chroma_div_w, texture_coordinate.y / u_chroma_div_h)).r - 0.5;

    return clamp(vec4(mat3(1.1643,  1.16430, 1.1643,
                                    0.0,    -0.39173, 2.0170,
                                    1.5958, -0.81290, 0.0) * yuv, 1.0), 0.0, 1.0);
}

void main()
{
    gl_FragColor = yuvToRGBA(u_texture0, u_texture1, u_texture2,
            TexCoord, u_chroma_div_w, u_chroma_div_h);

}
