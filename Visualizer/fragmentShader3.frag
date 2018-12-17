precision mediump float;
varying vec4 fPosition;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Time;
uniform float Spectrum;

const vec2 resolution = vec2(800.0, 800.0);

const float offsetX = 400.0;
const float offsetY = 300.0;
const float freq = 50.0;
const float amp = 50.0;
const float width = 40.0;
const float speed = 15.0;

//Heart
const float minSize = 50.0;
const float maxSize = 100.0;
const float curvature = 3.0;

//Fractals
const int octaves = 5;

//Linearly interpolate between two values
float lerp(float v0, float v1, float t)
{
	return ((1.0 - t) * v0) + (t * v1);
}

float invLerp(float v0, float v1, float v)
{
	return (v - v0) / (v1 - v0);
}

float randomFract(in vec2 _st)
{
	return fract(sin(dot(_st.xy, vec2(12.9898,78.233)))* 3758.5453123); //Used Fixed Random Seed (Pass in random values from CPU?)
}

float noise(in vec2 _st)
{
	vec2 i = floor(_st);
	vec2 f = fract(_st);
	
	//Four corners in a 2D tile
	float a = randomFract(i);
	float b = randomFract(i + vec2(1.0, 0.0));
	float c = randomFract(i + vec2(0.0, 1.0));
	float d	= randomFract(i + vec2(1.0, 1.0));
	
	vec2 u = f * f * (3.0 - 2.0 * f);

    return	mix(a, b, u.x) +
			(c - a)* u.y * (1.0 - u.x) +
			(d - b) * u.x * u.y;
}

//Fractal Brownian Motion (Fractal Noise)
float fbm(in vec2 _st)
{
	//Initial values
	float value = 0.0;
    float amp = 0.5;
	
    vec2 shift = vec2(100.0);
	
    // Rotate to reduce axial bias
    mat2 rot = mat2
	(
		cos(0.5), sin(0.5),
        -sin(0.5), cos(0.5)
	);
	
    for (int i = 0; i < octaves; ++i) //Add in noises on top of the others one by one
	{
        value += amp * noise(_st);
        _st = rot * _st * 2.0 + shift;
        //_st *= 2.0;
        amp *= 0.5;
    }
    return value;
}

void main()
{
	vec2 st = gl_FragCoord.xy / resolution.xy * 10.0;
	// st += st * abs(sin(u_time*0.1)*3.0); //Zooming
	vec3 fractMix = vec3(0.0);
	vec4 fractColor;
	
	vec2 st2 = gl_FragCoord.xy / resolution.xy * 10.0;
    //st2.x *= resolution.x / resolution.y;
    vec3 fractMix2 = vec3(0.0);
    fractMix2 += fbm(st2);
	
	vec2 q = vec2(0.0);
    q.x = fbm( st + 0.00 * Time);
    q.y = fbm( st + vec2(1.0));
	vec2 r = vec2(0.0);;
    r.x = fbm( st + 1.0 * q + vec2(1.7,9.2)+ 0.15 * Time );
    r.y = fbm( st + 1.0 * q + vec2(8.3,2.8)+ 0.126 * Time);
	
	float f = fbm(st + r);
	
	fractMix = mix
	(
		vec3(0.101961 * Spectrum * 10.0,0.619608 * Spectrum * 10.0,0.666667 * Spectrum * 10.0),
        vec3(0.666667 * Spectrum * 10.0,0.666667 * Spectrum * 10.0,0.498039 * Spectrum * 10.0),
        clamp((f*f)*4.0,0.0,1.0)
	);

    fractMix = mix
	(
		fractMix,
        vec3(0,0,0.164706 * Spectrum * 10.0),
        clamp(length(q),0.0,1.0)
	);

    fractMix = mix
	(
		fractMix,
        vec3(0.666667 * Spectrum * 2.5,1.0 * Spectrum * 2.5,1.0 * Spectrum * 2.5),
        clamp(length(r),0.0,1.0)
	);

	//fractColor = vec4(fractMix2, 1.0);
    fractColor = vec4((f*f*f + 0.6*f*f + 0.5*f) * fractMix, 1.0);
	
	vec4 texColor = texture2D(sampler2d, fTexCoord);
	vec4 resultColor;
	vec4 resultColor2;
	resultColor.a = 1.0;
	
	float equation = amp * Spectrum * 2.5 * sin((gl_FragCoord.x / freq) - (offsetX + Time * speed)) + offsetY;
	
	//float dSize = (maxSize - minSize) * abs(sin(Time * speed)) + minSize;
	float dSize = (maxSize - minSize) * sin(Spectrum * 10.0) + minSize;
	float hEq = pow((gl_FragCoord.x - offsetX) / dSize, 2.0) + pow(((gl_FragCoord.y - offsetY) / dSize) - sqrt(abs((gl_FragCoord.x - offsetX) / dSize)), 2.0);
	
	if(gl_FragCoord.y >= equation - width * Spectrum * 5.0 && gl_FragCoord.y <= equation + width * Spectrum * 5.0) //Sine Wave
	{
		resultColor2.r = (1.0 - abs((invLerp(equation - width * Spectrum * 5.0, equation + width * Spectrum * 5.0, gl_FragCoord.y) - 0.5) * 2.0)) * Spectrum * 10.0;
		resultColor2.g = (1.0 - abs((invLerp(equation - width * Spectrum * 5.0, equation + width * Spectrum * 5.0, gl_FragCoord.y) - 0.5) * 2.0)) * Spectrum * 10.0;
		resultColor2.b = (1.0 - abs((invLerp(equation - width * Spectrum * 5.0, equation + width * Spectrum * 5.0, gl_FragCoord.y) - 0.5) * 2.0)) * Spectrum * 10.0;
	}
	else
	{
		resultColor2.r = 0.0;
		resultColor2.g = 0.0;
		resultColor2.b = 0.0;
	}
	
	if(hEq <= curvature && hEq >= curvature / 2.0) //Heart Equation
	{
		resultColor.r = (1.0 - abs((invLerp(curvature / 2.0, curvature, hEq)))) * 0.8;
		resultColor.g = (1.0 - abs((invLerp(curvature / 2.0, curvature, hEq)))) * 0.8;
		resultColor.b = (1.0 - abs((invLerp(curvature / 2.0, curvature, hEq)))) * 0.8;
	}
	else if(hEq < curvature / 2.0)
	{
		resultColor.r = 1.0 * 0.85;
		resultColor.g = 1.0 * 0.85;
		resultColor.b = 1.0 * 0.85;
	}
	else
	{
		resultColor.r = 0.0;
		resultColor.g = 0.0;
		resultColor.b = 0.0;
	}
	
	/*
	vec4 gradientColor;
	
	gradientColor.r = 0.5 * sin(gl_FragCoord.x / 125.0 + 0.0) + 0.5;
	gradientColor.g = 0.5 * sin(gl_FragCoord.x / 125.0 + 2.0) + 0.5;
	gradientColor.b = 0.5 * sin(gl_FragCoord.x / 125.0 + 4.0) + 0.5;
	gradientColor.a = 1.0;
	*/
		
	gl_FragColor = texColor + resultColor + resultColor2;
}

/*
float amplitude = 1.;
float frequency = 1.;
float s = 0.672 + 1.5;
float s1 = s * 1.656;
float s2 = s * -1.085;
y = sin(x * frequency)*s;
float t = 0.01*(-u_time*130.0);
y += sin(x*frequency*2.1*s + t*s2)*4.5*(s-1.0)*(s-1.5);
y += sin(x*frequency*1.72*s1 + t*1.121*s2)*4.0*(s-1.0)*(s-1.5);
y += sin(x*frequency*2.221*s1 + t*0.437)*5.0*(s-1.0)*(s-1.5);
y += sin(x*frequency*3.832+ t*4.269*s2)*3.572*(s-1.0)*(s-1.5);
y *= amplitude*0.06;
*/