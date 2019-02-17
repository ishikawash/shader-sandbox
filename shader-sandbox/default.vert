#version 120

uniform mat4 _LocalToCameraMatrix;
uniform mat4 _ProjectionMatrix;
attribute vec4 _VertexPosition;

void main(void)
{
    gl_Position = _ProjectionMatrix * _LocalToCameraMatrix * _VertexPosition;
}
