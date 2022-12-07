#extension GL_OES_EGL_image_external

precision mediump float;

uniform samplerExternalOES u_texture0;

varying vec2 TexCoord;

void main()
{
	gl_FragColor = texture2D(u_texture0, TexCoord);
}
