precision mediump float;

uniform float Factor1;
uniform float Factor2;
uniform float Factor3;

uniform float Spectrum1;
uniform float Spectrum2;
uniform float Spectrum3;

vec2 resolution = vec2(800.0, 600.0);

vec2 rotate( vec2 matrix, float angle ) 
{
	return vec2( matrix.x*cos(radians(angle)), matrix.x*sin(radians(angle)) ) + vec2( matrix.y*-sin(radians(angle)), matrix.y*cos(radians(angle)) );
}

void CirclePattern()
{
	/*vec2 position = ((gl_FragCoord.xy / resolution.xy) * 2.0 - 1.0) * vec2(resolution.x / resolution.y, 1.0);
	
	float d = abs(Spectrum3 + 0.1 + length(position) - 1.0 * abs(cos(sin(Factor2)))) * 5.0;

	gl_FragColor += vec4(sin(Factor1) /4.0/d, 0.1 / d, cos(Factor1) /4.0 / d, 1.0);*/
	
	vec2 p = (gl_FragCoord.xy * 2.0 - resolution) / min(resolution.x, resolution.y);
    vec3 destColor = vec3(0.5, 0.5, 1.0);
    float f = 0.01;
    for(float i = 0.0; i < 6.0; i++)
	{
        float s = sin(Factor1 + i * 1.04719);
        float c = cos(Factor1 + i * 1.04719);
        f += 0.025 / abs(length(p + vec2(c, s)) -0.25 * abs(sin (Factor1) ));
	}
	
    gl_FragColor = vec4(vec3(destColor * f), 1.0);
}

void WavePattern()
{
	vec2 position = ( gl_FragCoord.xy / resolution.xy ) ;
	
	float r;
	float b;
	float g;
	
	float yorigin = 0.2 + 0.1 * sin(position.x * 30.0 + 0.8 * Factor2);
	
	float dist = (20.0*abs(yorigin - position.y));
	
	r = (Spectrum1  + 0.1 * sin(Factor1))/dist;
	b = (Spectrum2  + 0.1 * sin(Factor2))/dist;
	g = (Spectrum3  + 0.1 * sin(Factor3))/dist;

	gl_FragColor = vec4( r, g, b, 1.0 );
}

void NervesPattern()
{
	vec2 uv = ( gl_FragCoord.xy / resolution.xy ) * 2.0 - 1.0;

	vec3 finalColor = vec3 ( 1.0, 0.3, 0.5 );
	
	finalColor *= abs( Spectrum3 / (sin( uv.x + sin(uv.y+Factor1)* 0.10 ) * 40.0) );

	gl_FragColor = vec4( finalColor, 1.0 );
}

void main()
{
	WavePattern();
	NervesPattern();
	//CirclePattern();
}