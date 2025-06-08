#version 330

out vec4 pixelColor; //Output variable. Almost final pixel color.

uniform sampler2D textureMap0; // Bark texture
uniform sampler2D textureMap1; // Leaf texture  
uniform sampler2D textureMap2; // Grass texture

uniform int useBarkTex;
uniform int useLeafTex;
uniform int useGroundTex;

//Varying variables from vertex shader
in vec4 n; // Normal in eye space
in vec4 l; // Light vector in eye space  
in vec4 v; // View vector in eye space
in vec2 iTexCoord0; // Primary texture coordinates
in vec2 iTexCoord1; // Secondary texture coordinates (normal-based)

void main(void) {
	//Normalized, interpolated vectors
	vec4 ml = normalize(l);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);
	//Reflected vector
	vec4 mr = reflect(-ml, mn);

	//Surface parameters - choose texture based on uniforms
	vec4 kd;
	if (useGroundTex == 1) {
		// For ground, mix grass texture with normal-based coordinates
		kd = mix(texture(textureMap2, iTexCoord0), texture(textureMap2, iTexCoord1), 0.2);
	} else if (useLeafTex == 1) {
		// For leaves, mix leaf texture with normal-based coordinates for variation
		kd = mix(texture(textureMap1, iTexCoord0), texture(textureMap1, iTexCoord1), 0.3);
	} else if (useBarkTex == 1) {
		// For bark, mix bark texture with normal-based coordinates for detail
		kd = mix(texture(textureMap0, iTexCoord0), texture(textureMap0, iTexCoord1), 0.25);
	} else {
		// Default color for sun or other objects
		kd = vec4(1.0, 0.9, 0.6, 1.0); // Warm sun color
	}
	
	vec4 ks = vec4(0.3, 0.3, 0.3, 1.0); // Specular color

	//Lighting model computation
	float nl = clamp(dot(mn, ml), 0.0, 1.0); // Diffuse factor
	float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 25.0); // Specular factor (lower shininess)
	
	// Combine diffuse and specular with some ambient
	vec3 ambient = 0.15 * kd.rgb;
	vec3 diffuse = kd.rgb * nl;
	vec3 specular = ks.rgb * rv;
	
	pixelColor = vec4(ambient + diffuse + specular, kd.a);
}
