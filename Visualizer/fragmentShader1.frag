precision mediump float;
varying vec4 fPosition;
varying vec4 fColor;
varying vec2 fTexCoord;
uniform float Factor1;

uniform sampler2D sampler2d;

const float offsetX = 0.0;
const float offsetY = 300.0;
const float frequency = 25.0;
const float amplitude = 50.0;
const float width = 10.0;
const float speed = 10.0;

void main()
{
	vec4 resultColor;
	vec4 texColor = texture2D(sampler2d, fTexCoord);
	
	float y1 = amplitude * sin((gl_FragCoord.x / frequency) - (offsetX +  speed * Factor1)) + offsetY - width;
	float y2 = amplitude * sin((gl_FragCoord.x / frequency) - (offsetX +  speed * Factor1)) + offsetY + width;
	
	// y = asin(b) + c;
	if(gl_FragCoord.y >= y1 && gl_FragCoord.y <= y2)
	{
		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
		mix(1.0, 2.0, 0.5);
	}
	else
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	
	resultColor = texColor * gl_FragColor;
}
