attribute vec4 vPosition;
attribute vec4 vColor; //Get from CPU
attribute vec2 vTexCoord; //Get from CPU

varying vec4 fPosition; //Pass to fragment shaders
varying vec4 fColor; //Pass to fragment shaders
varying vec2 fTexCoord; //Pass to fragment shaders

uniform mat4 uMvpMatrix;

void main()
{
	fPosition = vPosition;
	fColor = vColor;
	fTexCoord = vTexCoord;
	
	gl_Position = uMvpMatrix * vPosition; //Do not do vPosition * uMvpMatrix, it's a matrix
}
