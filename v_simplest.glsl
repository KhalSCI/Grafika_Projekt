#version 330

/*
 * EYE-SPACE LIGHTING VERTEX SHADER
 * 
 * This shader performs eye-space lighting calculations, which means all lighting
 * vectors (light, normal, view) are computed in the camera's coordinate system.
 * This approach is commonly used because:
 * 1. The view vector calculation is simplified (camera is at origin in eye space)
 * 2. Lighting calculations remain consistent regardless of world transformations
 * 3. It's efficient for multiple lights and complex scenes
 */

//Uniform variables (constant for all vertices in a draw call)
uniform mat4 P; // Projection matrix (3D -> 2D screen projection)
uniform mat4 V; // View matrix (world space -> eye/camera space)
uniform mat4 M; // Model matrix (object space -> world space)
uniform vec3 lightPos; // Light position in world space (sun position)

//Attributes (input data per vertex)
in vec4 vertex; // Vertex coordinates in model/object space
in vec3 normal; // Vertex normal in model/object space  
in vec2 texcoord; // Texture coordinates (UV mapping)

//Varying variables (output to fragment shader, interpolated across triangle)
out vec4 l; // Light vector in eye space (vertex -> light direction)
out vec4 n; // Normal vector in eye space (surface orientation)
out vec4 v; // View vector in eye space (vertex -> camera direction)
out vec2 iTexCoord0; // Primary texture coordinates (standard UV mapping)
out vec2 iTexCoord1; // Secondary texture coordinates (generated from normals for mixing)


void main(void) {
    /*
     * STEP 1: COORDINATE SPACE TRANSFORMATIONS
     * Transform positions from model space to eye space for lighting calculations
     */
    vec4 lp = vec4(lightPos, 1.0); // Convert light position to homogeneous coordinates
    vec4 vertexEyeSpace = V * M * vertex; // Transform vertex: model -> world -> eye space
    vec4 lightEyeSpace = V * lp; // Transform light position: world -> eye space
    
    /*
     * STEP 2: LIGHTING VECTOR CALCULATIONS (in eye space)
     * All vectors point FROM the surface vertex TOWARDS their target
     */
    
    // Light vector: direction from vertex towards light source
    l = normalize(lightEyeSpace - vertexEyeSpace);
    
    // View vector: direction from vertex towards camera (camera is at origin in eye space)
    v = normalize(vec4(0, 0, 0, 1) - vertexEyeSpace);
    
    // Normal vector: surface orientation in eye space (w=0 because it's a direction, not position)
    n = normalize(V * M * vec4(normal, 0.0));

    /*
     * STEP 3: TEXTURE COORDINATE GENERATION
     * Generate two sets of texture coordinates for advanced mixing effects
     */
    
    // Primary texture coordinates: standard UV mapping from model
    iTexCoord0 = texcoord;
    
    // Secondary coordinates: generated from surface normal for procedural detail
    // Maps normal.xy from [-1,1] to [0,1] range for texture sampling
    // This creates texture coordinates that vary with surface orientation
    iTexCoord1 = (n.xy + 1.0) / 2.0;
    
    /*
     * STEP 4: VERTEX POSITION OUTPUT
     * Final transformation chain: model -> world -> eye -> screen/clip space
     */
    gl_Position = P * V * M * vertex;
}
