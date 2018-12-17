precision mediump float;

// varying time Variable for Animation
uniform float Factor1;
uniform float Factor2;
uniform float Factor3;

// Spectrum Audio Data for FMOD
uniform float Spectrum1;
uniform float Spectrum2;
uniform float Spectrum3;

// WeirdCirclePattern Variable
vec3 colorWeirdCircle = vec3( 1.0,0.9, 0.4 );
float radiusWeirdCircle = 1.0 * Spectrum3;
float radiusWeirdCircle2 = 1.0 * Spectrum3;
float pulseSpeedWeirdCircle = 1.5 * Spectrum3;
float intensityWeirdCircle = 0.1 * Spectrum3;
float frequencyWeirdCircle = 80.0 * Spectrum3;

// resolution of .xy
vec2 resolution = vec2(800.0, 600.0);

// rotate function for NervePattern()
vec2 rotate( vec2 matrix, float angle ) 
{
	return vec2( matrix.x * cos(radians(angle)), matrix.x * sin(radians(angle))) + vec2(matrix.y * -sin(radians(angle)), matrix.y * cos(radians(angle)));
}

// To generate Noise Function and Pattern
float hash( float n ) 
{ 
	return fract(sin(n) * 753.5453123); 
}

// Noise Function
float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = p.x + p.y*157.0;
    return mix(mix(hash(n + 0.0), hash(n + 1.0), f.x), mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y);
}

// Related to Noise Function
float fbm(vec2 p, vec3 a)
{
     float v = 0.0;
     v += noise(p * a.x) * 0.5;
     v += noise(p * a.y) * 0.25;
     v += noise(p * a.z) * 0.125;
     return v;
}

// Calculation for WeirdCirclePattern
vec3 WeirdCircle(vec2 uv, float radius, float thickness, float intensity, vec3 fbmOffset, float t2, vec3 color)
{
   float distanceWCircle = length(uv);
	
    float aTan = atan(uv.y / uv.x); 
    float t1 = intensity * sin(aTan * frequencyWeirdCircle + Factor3 * t2) * 0.5 + 0.5 + fbm(uv,fbmOffset) * 0.15;
    float warp = mix(0.0, 0.5, t1); 
	 
	 // Getting the effect
    float effector = thickness*abs(0.003 / ((distanceWCircle) - (radius + warp)));
	
    return color * effector;
}

// Main Function of WeirdCirclePattern + Noise Function
void WeirdCirclePattern()
{
	vec2 uv = ( gl_FragCoord.xy / resolution.xy ) * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    vec3 finalColor = vec3(0.0);

    finalColor += WeirdCircle(uv, radiusWeirdCircle * Spectrum3, 1.0, intensityWeirdCircle  * Spectrum3, vec3(200, 100, 50), -10.0, vec3(0.8,0.5, 0.0));
    finalColor += WeirdCircle(uv, radiusWeirdCircle * Spectrum3, 1.0, intensityWeirdCircle * Spectrum3, vec3(90, 15, 1),1.0, vec3(0.3, 0.5, 2.5));
    finalColor += WeirdCircle(uv, radiusWeirdCircle2 * Spectrum3, 1.0, intensityWeirdCircle * Spectrum3, vec3(30, 6, 5),5.0, vec3(0.12, 0.15, 2.1));
    finalColor += WeirdCircle(uv, radiusWeirdCircle2 - 0.04 * Spectrum3, 1.0, intensityWeirdCircle * 2.0 * Spectrum3, vec3(100,100,100), 4.0, vec3(2.3, 0.5, 0.5));
    finalColor += WeirdCircle(uv, radiusWeirdCircle2  - 0.1, 0.1 * Spectrum3, intensityWeirdCircle * Spectrum3, vec3(200,200,200),4.0, vec3(0.1, 0.1, 0.2));
    
	gl_FragColor += vec4(finalColor, 1.0);
}

// Circle Pattern main()
void CirclePattern()
{
	vec2 position = ((gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0) * vec2(resolution.x / resolution.y, 1.0);
	
	float distanceCPattern = abs(Spectrum3 + 0.1 + length(position) - 1.0 * abs(cos(sin(Factor2)))) * 5.0;

	gl_FragColor += vec4(sin(Factor1) / 4.0 / distanceCPattern, 0.1 / distanceCPattern, cos(Factor1) / 4.0 / distanceCPattern, 1.0);
}

// Wave Pattern main()
void WavePattern()
{
	vec2 position = ( gl_FragCoord.xy / resolution.xy ) ;
	
	float redColour;
	float blueColour;
	float greenColour;
	
	float yOrigin = 0.2 + 0.1 * sin(position.x * Spectrum2 * 2.0 * Factor2);
	
	float distanceWPattern = (20.0*abs(yOrigin - position.y));
	
	redColour = (Spectrum1  + 0.1 * sin(Factor1)) / distanceWPattern;
	greenColour = (Spectrum2  + 0.1 * sin(Factor2)) / distanceWPattern;
	blueColour = (Spectrum3  + 0.1 * sin(Factor3)) / distanceWPattern;

	gl_FragColor += vec4(redColour, greenColour, blueColour, 1.0 );
}

// Pulse Pattern main()
void PulsePattern()
{
	vec2 postionPPattern = 2.0 * ( gl_FragCoord.xy / resolution.xy ) - 1.0;
	postionPPattern.x *= (resolution.x / resolution.y)*2.0;
	vec3 color = vec3(0.0);
	
	
	for (int i = 0; i < 40; i++) 
	{
		float a = Factor1 + float(i); 
		vec2 c = vec2(sin(a * 0.2) * cos(0.1 * Factor1), cos(a * 0.5) * sin(Factor1 * 0.01)); 
		float distancePPatern = 0.18 / (0.000 + 40.0 * abs(length(postionPPattern.xy) - 1.0 / (1.0 - mod(Factor1 * Spectrum1, 4.0) + 0.1 * float(i)))); 
		color += vec3(0.8, 0.3, 0.0) * distancePPatern;
		color *= sin(Factor1);
	}
	gl_FragColor += vec4(color, 1.0);
}

// More Circle Pattern main()
void MoreCirclePattern()
{
	vec2 positionMCirclePattern = (gl_FragCoord.xy * 2.0 - resolution) / min(resolution.x, resolution.y);
    vec3 dimColorSet = vec3(0.5, 0.5, 1.0);
    float f = 0.01;
    for(float i = 0.0; i < 6.0; i++)
	{
        float s = sin(Factor1 + i * 1.04719);
        float c = cos(Factor1 + i * 1.04719);
        f += Spectrum2 / abs(length(positionMCirclePattern + vec2(c, s)) - 0.25 * abs(sin(Factor2)));
	}
	
    gl_FragColor += vec4(vec3(dimColorSet * f), 1.0);
}

// Nerve Pattern main()
void NervePattern()
{
	vec3 redColor = vec3(0.7, 0.1, 0.3) * ((sin(Factor1 * 50.0) * 0.5 + 3.14) * 0.066);
	vec3 greenColor = vec3(0.0, 0.5, 0.5);
	vec3 blueColor = vec3(0.7, 0.5, 0.1) * ((cos(Factor1) + 1.125) * 2.0);

	vec2 position = ( gl_FragCoord.xy / resolution.xy ) / 4.0;
	position = gl_FragCoord.xy * 2.0 - resolution;
	position /= min(resolution.x, resolution.y);
	position = rotate(position, Factor1);
	
	//position -= mod(position, 0.0333); // make it pixelise!!
	
	float a = sin(position.y * 1.3 - Factor1 * 0.1) / 1.0;
	float b = cos(position.y * 1.4 - Factor1 * 0.2) / 1.0;
	float c = sin(position.x * 1.5 - Factor1 * 0.2 + 3.14) / 2.0;
	float d = cos(position.y * 1.6 - Factor1 * 0.5 + 3.14) / 2.0;
	
	float e = Spectrum1 / abs(position.x + a);
	float f = Spectrum2 / abs(position.x + b);
	float g = Spectrum3 / abs(position.y + c);
	float h = Spectrum3 / abs(position.x + d);
	
	vec3 color = redColor * e * greenColor * f + blueColor * g * 0.0 * h * f * Spectrum3 * 0.5;

	
	gl_FragColor += vec4(color, 1.0);
}

// Main() Function
void main()
{
	MoreCirclePattern();
	PulsePattern();
	WavePattern();
	CirclePattern();
	NervePattern();
	WeirdCirclePattern();
}