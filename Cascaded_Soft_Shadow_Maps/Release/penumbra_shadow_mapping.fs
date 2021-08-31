#version 120

#extension GL_EXT_texture_array : enable

uniform sampler2D Texture;
uniform sampler2DArrayShadow ShadowMapCascade1;
uniform sampler2DArrayShadow ShadowMapCascade2;
uniform sampler2DArrayShadow ShadowMapCascade3;
uniform mat4x4 LightTextureMatricesCascade3[4];
uniform mat4x4 LightTextureMatricesCascade2[9];
uniform mat4x4 LightTextureMatricesCascade1[16];
varying vec3 LightDirection, LightDirectionReflected, CameraDirection, Normal;
varying float po;
varying vec4 vert;

void main()
{   float bias = 0.005;
	float x,y;
	float xPixelOffset1 = 1.0/1024;
	float yPixelOffset1 = 1.0/1024;
	float xPixelOffset2 = 1.0/512;
	float yPixelOffset2 = 1.0/512;
	float xPixelOffset3 = 1.0/128;
	float yPixelOffset3 = 1.0/128;
	float ombre ;

    vec4 ShadowTexCoord3[4];vec4 ShadowTexCoord2[9];vec4 ShadowTexCoord1[16];
    
	float A=9,B=14,AA=6.0,BB=11.0;
	float ShadowCascade3=0.0;
	float ShadowCascade2=0.0 ;
	float ShadowCascade1=0.0 ;
	
	float NdotLD,NdotLD1,NdotLD2,NdotLD3,Spec,Spec1,Spec2,Spec3;
	NdotLD = max(dot(normalize(LightDirection), Normal), 0.0)+0.2;
	//Spec = pow(max(dot(normalize(LightDirectionReflected), normalize(CameraDirection)), 0.0), 2500.0);
	 vec4 CascadeIndicator = vec4(0.0, 0.0, 0.0, 0.0);
	if(po<=(A))
	{
	CascadeIndicator = vec4(0.0, 0.0, 0.2, 0.0);
		for(int i = 0; i < 16; i++){
	ShadowTexCoord1[i] = LightTextureMatricesCascade1[i] * vert;
	}
		ombre = 0;
		for(int i = 0; i < 16; i++)
		{
            for (y = -1.0 ; y <=1.0 ; y+=1.0)// 3*3
			for (x = -1.0 ; x <=1.0 ; x+=1.0)// 3*3		

		   ombre  += shadow2DArray(ShadowMapCascade1, vec4(ShadowTexCoord1[i].xy / ShadowTexCoord1[i].w, i, ShadowTexCoord1[i].z / ShadowTexCoord1[i].w)+  vec4(x * xPixelOffset1 * ShadowTexCoord1[i].w, y * yPixelOffset1 * ShadowTexCoord1[i].w, 0.05, 0.0)).r;
			ombre /= 9.0  ; // 3*3
			ShadowCascade1 += ombre;
		}
		ShadowCascade1 /= 16.0;
	
		NdotLD1 = NdotLD * ShadowCascade1;
		Spec1 = Spec * ShadowCascade1;
		//gl_FragColor = vec4( (0.25 + NdotLD1 * 0.75 + Spec1),(0.25 + NdotLD1 * 0.75 + Spec1),(0.25 + NdotLD1 * 0.75 + Spec1), 1.0);
		//gl_FragColor = vec4(texture2D(Texture, gl_TexCoord[0].st).rgb * (0.25 + NdotLD * 0.75 + Spec), 1.0);
	gl_FragColor = vec4(texture2D(Texture, gl_TexCoord[0].st).rgb * (0.25 + NdotLD1 * 0.75 ), 1.0);// + CascadeIndicator;
		//gl_FragColor = vec4( (0.25 + NdotLD1 * 0.75 ),(0.25 + NdotLD1 * 0.75 ),(0.25 + NdotLD1 * 0.75 ), 1.0) + CascadeIndicator;
	}
	
	
	
	if(po<=(B) && po>(A))
	{ CascadeIndicator = vec4(0.0, 0.2, 0.0, 0.0);
		for(int i = 0; i < 9; i++) {
		ShadowTexCoord2[i] = LightTextureMatricesCascade2[i] * vert;
		}
		ombre = 0;
		for(int i = 0; i < 9; i++)
		{ for (y = -1.0 ; y <=1.0 ; y+=1.0)// 3*3
		for (x = -1.0 ; x <=1.0 ; x+=1.0)// 3*3		

		ombre += shadow2DArray(ShadowMapCascade2, vec4(ShadowTexCoord2[i].xy / ShadowTexCoord2[i].w, i, ShadowTexCoord2[i].z / ShadowTexCoord2[i].w)+  vec4(x * xPixelOffset2 * ShadowTexCoord2[i].w, y * yPixelOffset2 * ShadowTexCoord2[i].w, 0.05, 0.0)).r;
		ombre /= 9.0  ; // 3*3
		ShadowCascade2 += ombre;
		}
		ShadowCascade2 /= 9.0;
	
		NdotLD2 = NdotLD * ShadowCascade2;
		Spec2 = Spec * ShadowCascade2;
		//gl_FragColor = vec4( (0.25 + NdotLD2 * 0.75 + Spec2),(0.25 + NdotLD2 * 0.75 + Spec2),(0.25 + NdotLD2 * 0.75 + Spec2), 1.0);
		gl_FragColor = vec4(texture2D(Texture, gl_TexCoord[0].st).rgb * (0.25 + NdotLD2 * 0.75 ), 1.0);//+ CascadeIndicator;
		//gl_FragColor = vec4( (0.25 + NdotLD2 * 0.75 ),(0.25 + NdotLD2 * 0.75 ),(0.25 + NdotLD2 * 0.75 ), 1.0) + CascadeIndicator;
	}
	
	
	
	if(po>(B))
	{ CascadeIndicator = vec4(0.2, 0.0, 0.0, 0.0);
		for(int i = 0; i < 4; i++) {
		ShadowTexCoord3[i] = LightTextureMatricesCascade3[i] * vert;
		}
	
		ombre = 0;
		for(int i = 0; i < 4; i++)
		{ for (y = -1.0 ; y <=1.0 ; y+=1.0)// 3*3
		for (x = -1.0 ; x <=1.0 ; x+=1.0)// 3*3		
		ombre += shadow2DArray(ShadowMapCascade3, vec4(ShadowTexCoord3[i].xy / ShadowTexCoord3[i].w, i, ShadowTexCoord3[i].z / ShadowTexCoord3[i].w)+  vec4(x * xPixelOffset3 * ShadowTexCoord3[i].w, y * yPixelOffset3 * ShadowTexCoord3[i].w, 0.05, 0.0)).r;
		ombre /= 9.0  ; // 3*3
		ShadowCascade3 += ombre;
		}
		ShadowCascade3 /= 4.0;
	
		NdotLD3 = NdotLD * ShadowCascade3;
		Spec3 = Spec * ShadowCascade3;
		//gl_FragColor = vec4( (0.25 + NdotLD3 * 0.75 + Spec3),(0.25 + NdotLD3 * 0.75 + Spec3),(0.25 + NdotLD3 * 0.75 + Spec3), 1.0);
		//gl_FragColor = vec4( (0.25 + NdotLD3 * 0.75 ),(0.25 + NdotLD3 * 0.75 ),(0.25 + NdotLD3 * 0.75 ), 1.0) + CascadeIndicator ;
		gl_FragColor = vec4(texture2D(Texture, gl_TexCoord[0].st).rgb * (0.25 + NdotLD3 * 0.75 ), 1.0);//+ CascadeIndicator;
		
	}
	

}
