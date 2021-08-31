#version 120

uniform vec3 LightPosition, CameraPosition;
varying vec3 LightDirection, LightDirectionReflected, CameraDirection, Normal;
varying float po;
varying vec4 vert;

void main()
{
	
	LightDirection = LightPosition - gl_Vertex.xyz;
	
	LightDirectionReflected = reflect(-LightDirection, gl_Normal);
	
	CameraDirection = CameraPosition - gl_Vertex.xyz;
	
	Normal = gl_Normal;
	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	vert=gl_Vertex;
	
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	
	po=gl_Position.z;
}
