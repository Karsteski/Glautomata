#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;

out vec3 outVertexColour;

void main()
{
gl_Position = vec4(position, 1.0);
outVertexColour = colour;
};


#shader fragment
#version 330 core

in vec3 outVertexColour;
out vec4 fragmentColour;

void main()
{
    fragmentColour = vec4(outVertexColour, 1.0);
};