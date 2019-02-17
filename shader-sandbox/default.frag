#version 120

uniform float _Time;
uniform vec2 _Mouse;
uniform vec2 _Resolution;

void main( void )
{
    vec2 p = gl_FragCoord.xy / _Resolution;
    p *= (2.0*_Mouse - 1.0);
    float l = sin(p.y * sin(_Time * 1.3) + sin(p.x * 4.0 + _Time) * sin(_Time));
    vec3 rgb = vec3(sin(l * 6.0), sin(l * 7.0), sin(l * 10.0));
    gl_FragColor = vec4(rgb*0.5 + 0.5, 1.0);
}
