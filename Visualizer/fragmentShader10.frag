precision mediump float;

// varying time Variable for Animation
uniform float Factor1;
uniform float Factor2;
uniform float Factor3;

// Spectrum Audio Data for FMOD
uniform float Spectrum1;
uniform float Spectrum2;
uniform float Spectrum3;

// resolution of .xy
vec2 resolution = vec2(800.0, 600.0);

float PI =  3.14159265359;

// using glsl function to get the matrices.  mat2 (2x2)
mat2 rotate2d(float _angle)
{
    return mat2(cos(_angle),-sin(_angle), sin(_angle),cos(_angle));
}

// mat2 (2x2)
mat2 scale(vec2 _scale)
{
    return mat2(_scale.x,0.0, 0.0, _scale.y);
}

float box(in vec2 _st, in vec2 _size)
{
    _size = vec2(0.5) - _size*0.5;
    vec2 uv = smoothstep(_size, _size+vec2(0.001), _st);
    uv *= smoothstep(_size, _size+vec2(0.001), vec2(1.0)-_st);
    return uv.x*uv.y;
}

float cross(in vec2 _st, float _size)
{
    return  box(_st, vec2(_size,_size/4.0)) + box(_st, vec2(_size/4.0 ,_size));
}

void TranslatePattern()
{
	vec2 st = gl_FragCoord.xy / resolution.xy;
    vec3 color = vec3(0.0);

    // To move the cross we move the space
    vec2 translate = vec2(cos(Factor1),sin(Factor1));
    st += translate * 0.35;

    // Show the coordinates of the space on the background
    // color = vec3(st.x, st.y, 0.0);

    // Add the shape on the foreground
    color += vec3(cross(st, 0.25));

    gl_FragColor += vec4(color,1.0);
}

void RotationPattern()
{
	// Get the center position of the Space
	vec2 st = gl_FragCoord.xy / resolution.xy;
	// Set the colours
    vec3 color = vec3(0.0);

    // move space from the center to the vec2(0.0)
    st -= vec2(0.5);
    // rotate the space
    st = rotate2d( sin(Factor1)*PI ) * st;
    // move it back to the original place
    st += vec2(0.5);

    // Show the coordinates of the space on the background
    // color = vec3(st.x,st.y,0.0);

    // Add the shape on the foreground
    color += vec3(cross(st,0.4));

    gl_FragColor = vec4(color,1.0);
}

void ScalingPattern()
{
	vec2 st = gl_FragCoord.xy / resolution.xy;
    vec3 color = vec3(0.0);

    st -= vec2(0.5);
    st = scale( vec2(sin(Factor1)+1.0) ) * st;
    st += vec2(0.5);

    // Show the coordinates of the space on the background
    // color = vec3(st.x,st.y,0.0);

    // Add the shape on the foreground
    color += vec3(cross(st,0.2));

    gl_FragColor += vec4(color,1.0);
}

void main()
{
	//TranslatePattern();
	//RotationPattern();
	//ScalingPattern();
}