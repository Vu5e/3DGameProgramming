precision mediump float;
varying vec4 fPosition;
varying vec4 fColor;
varying vec2 fTexCoord;

uniform sampler2D sampler2d;
uniform float Factor1;

float r = 60.0;

void main()                                 
{
	vec4 texColor = texture2D(sampler2d, fTexCoord);
	vec4 combinedColor;
	
	combinedColor = fColor * texColor;
	
	vec4 resultColor;
	
	float ax = pow((gl_FragCoord.x - 400.0), 2.0);
	float ay = pow((gl_FragCoord.y - 300.0), 2.0);
	float a = ax + ay;
	float b = r * r;
	
	if(a <= b)
	{
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	//Animate
	resultColor = gl_FragColor;
}