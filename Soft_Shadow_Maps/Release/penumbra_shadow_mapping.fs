#version 120

#extension GL_EXT_texture_array : enable

uniform sampler2D Texture;
uniform sampler2DArrayShadow ShadowMapCascade1;
uniform mat4x4 LightTextureMatricesCascade1[36];
varying vec3 LightDirection, LightDirectionReflected, CameraDirection, Normal;
varying float po;
varying vec4 vert;

void main()
{   float bias = 0.005;
	float x,y;
	float xPixelOffset1 = 1.0/1024;
	float yPixelOffset1 = 1.0/1024;
	float ombre ;


    vec4 ShadowTexCoord1[36];
    
	for(int i = 0; i < 36; i++){
	ShadowTexCoord1[i] = LightTextureMatricesCascade1[i] * vert;
	//ShadowTexCoord1[i] = ShadowTexCoord1[i]/ ShadowTexCoord1[i].w;
	}

	float ShadowCascade1=0.0 ;
	ombre = 0;
	for(int i = 0; i < 36; i++)
	{ for (y = -1.0 ; y <=1.0 ; y+=1.0)// 3*3
	  for (x = -1.0 ; x <=1.0 ; x+=1.0)// 3*3		

		ombre += shadow2DArray(ShadowMapCascade1, vec4(ShadowTexCoord1[i].xy / ShadowTexCoord1[i].w, i, ShadowTexCoord1[i].z / ShadowTexCoord1[i].w)+  vec4(x * xPixelOffset1 * ShadowTexCoord1[i].w, y * yPixelOffset1 * ShadowTexCoord1[i].w, 0.05, 0.0)).r;
	ombre /= 9.0  ; // 3*3
	ShadowCascade1 += ombre;
	}
	
	ShadowCascade1 /= 36.0;
	
    float NdotLD = (max(dot(normalize(LightDirection), Normal), 0.0)+0.2) * ShadowCascade1;
	//float Spec = pow(max(dot(normalize(LightDirectionReflected), normalize(CameraDirection)), 0.0), 2500.0) * ShadowCascade1;
	
	//gl_FragColor = vec4( (0.25 + NdotLD * 0.75 + Spec),(0.25 + NdotLD * 0.75 + Spec1),(0.25 + NdotLD * 0.75 + Spec), 1.0);
		//gl_FragColor = vec4(texture2D(Texture, gl_TexCoord[0].st).rgb * (0.25 + NdotLD * 0.75 + Spec), 1.0);
	gl_FragColor = vec4(texture2D(Texture, gl_TexCoord[0].st).rgb * (0.25 + NdotLD * 0.75 ), 1.0);

}
