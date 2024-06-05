#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    const vec2 positions[4] = vec2[]
    (
        vec2(-1, -1),
        vec2(+1, -1),
        vec2(-1, +1),
        vec2(+1, +1)
    );

    const vec2 coords[4] = vec2[]
    (
        vec2(0, 0),
        vec2(1, 0),
        vec2(0, 1),
        vec2(1, 1)
    );

    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);//vec4(aPos.x, aPos.y, 0.0, 1.0); 
    TexCoords = coords[gl_VertexID];
}