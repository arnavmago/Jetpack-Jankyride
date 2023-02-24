#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
  
in vec2 TexCoord;
uniform float opaque;
uniform vec3 blur;

uniform sampler2D Texture;
uniform vec2 transback;

void main()
{
    FragColor = texture(Texture, TexCoord - transback);
    // if(blur.r >= 0.9 && length(FragColor.rgb) <= 0.05) FragColor.a = 0.f;
    BrightColor = vec4(FragColor.rgb * blur, 1.0f);
    BrightColor.a = FragColor.a;
}