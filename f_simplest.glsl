#version 330

/*
 * EYE-SPACE LIGHTING FRAGMENT SHADER WITH TEXTURE MIXING
 * 
 * This shader implements:
 * 1. Eye-space Phong lighting model (ambient + diffuse + specular)
 * 2. Multi-texture support with material flags
 * 3. Advanced texture mixing using dual coordinate systems
 * 4. Special handling for emissive materials (sun)
 * 
 * Key features:
 * - Same texture sampled twice with different coordinates for natural variation
 * - Material-specific mixing ratios for optimal visual results
 * - Efficient single-pass lighting calculation
 */

out vec4 pixelColor; //Output variable. Final pixel color sent to framebuffer.

// Texture samplers - each bound to a different texture unit
uniform sampler2D textureMap0; // Bark texture (tree branches/trunk)
uniform sampler2D textureMap1; // Leaf texture (tree foliage)
uniform sampler2D textureMap2; // Grass texture (ground surface)
uniform sampler2D textureMap3; // Sun texture (bright yellow emissive)

// Material selection flags - only one should be 1 per fragment
uniform int useBarkTex;   // 1 = use bark texture, 0 = don't use
uniform int useLeafTex;   // 1 = use leaf texture, 0 = don't use
uniform int useGroundTex; // 1 = use ground texture, 0 = don't use
uniform int useSunTex;    // 1 = use sun texture (emissive, no lighting), 0 = don't use

// Varying variables from vertex shader (interpolated across triangle surface)
in vec4 n; // Normal in eye space (surface orientation)
in vec4 l; // Light vector in eye space (vertex -> light direction)
in vec4 v; // View vector in eye space (vertex -> camera direction)
in vec2 iTexCoord0; // Primary texture coordinates (standard UV mapping)
in vec2 iTexCoord1; // Secondary texture coordinates (normal-based, for mixing)

void main(void) {
	/*
	 * STEP 1: NORMALIZE INTERPOLATED VECTORS
	 * Vectors from vertex shader are interpolated across triangle surface,
	 * so they need to be re-normalized for accurate lighting calculations
	 */
	vec4 ml = normalize(l); // Light vector (vertex -> light)
	vec4 mn = normalize(n); // Normal vector (surface orientation)
	vec4 mv = normalize(v); // View vector (vertex -> camera)
	
	/*
	 * STEP 2: CALCULATE REFLECTION VECTOR
	 * Used for specular highlights (shiny reflections)
	 * reflect(-ml, mn) = direction of perfect reflection of light off surface
	 */
	vec4 mr = reflect(-ml, mn);

	/*
	 * STEP 3: MATERIAL/TEXTURE SELECTION WITH ADVANCED MIXING
	 * Choose appropriate texture and apply mixing for surface detail variation
	 */
	vec4 kd; // Diffuse color (main surface color from texture)
	
	if (useSunTex == 1) {
		/*
		 * SUN MATERIAL: Pure emissive (no lighting calculations)
		 * Sun should appear bright regardless of lighting conditions
		 */
		pixelColor = texture(textureMap3, iTexCoord0);
		return; // Early exit - no lighting needed for emissive materials
		
	} else if (useGroundTex == 1) {
		/*
		 * GROUND MATERIAL: Grass texture with subtle variation
		 * Mix ratio 0.2 = 20% normal-based, 80% UV-based
		 * Lower mixing for more uniform ground appearance
		 */
		kd = mix(texture(textureMap2, iTexCoord0), texture(textureMap2, iTexCoord1), 0.2);
		
	} else if (useLeafTex == 1) {
		/*
		 * LEAF MATERIAL: Higher mixing for organic variation
		 * Mix ratio 0.3 = 30% normal-based, 70% UV-based
		 * More variation makes leaves look natural and less repetitive
		 */
		kd = mix(texture(textureMap1, iTexCoord0), texture(textureMap1, iTexCoord1), 0.3);
		
	} else if (useBarkTex == 1) {
		/*
		 * BARK MATERIAL: Moderate mixing for surface detail
		 * Mix ratio 0.25 = 25% normal-based, 75% UV-based
		 * Adds bark surface texture while preserving main pattern
		 */
		kd = mix(texture(textureMap0, iTexCoord0), texture(textureMap0, iTexCoord1), 0.25);
		
	} else {
		/*
		 * FALLBACK MATERIAL: Neutral gray when no texture is specified
		 */
		kd = vec4(0.5, 0.5, 0.5, 1.0);
	}
	
	/*
	 * STEP 4: LIGHTING MATERIAL PROPERTIES
	 */
	vec4 ks = vec4(0.3, 0.3, 0.3, 1.0); // Specular color (reflection intensity)

	/*
	 * STEP 5: PHONG LIGHTING MODEL CALCULATIONS
	 */
	
	// Diffuse lighting: surface brightness based on angle to light
	// Lambert's law: brightness = cos(angle between normal and light)
	float nl = clamp(dot(mn, ml), 0.0, 1.0);
	
	// Specular lighting: shiny highlights based on reflection angle
	// Phong model: intensity = cos^n(angle between reflection and view)
	// Higher exponent (25.0) = smaller, sharper highlights
	float rv = pow(clamp(dot(mr, mv), 0.0, 1.0), 25.0);
	
	/*
	 * STEP 6: COMBINE LIGHTING COMPONENTS
	 * Final color = ambient + diffuse + specular
	 */
	
	// Ambient: base illumination (simulates indirect/scattered light)
	vec3 ambient = 0.05 * kd.rgb; // 5% of diffuse color
	
	// Diffuse: main surface color affected by light angle
	vec3 diffuse = kd.rgb * nl;
	
	// Specular: bright highlights on shiny surfaces
	vec3 specular = ks.rgb * rv;
	
	// Combine all lighting components
	pixelColor = vec4(ambient + diffuse + specular, kd.a);
}
