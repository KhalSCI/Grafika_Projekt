#version 330

//Uniform variables
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos; // Light position in world space

//Attributes
in vec4 vertex; //Vertex coordinates in model space
in vec3 normal; //Vertex normal in model space  
in vec2 texcoord;
in vec4 color; // Vertex color attribute

//Varying variables
out vec4 l; // Light vector in eye space
out vec4 n; // Normal vector in eye space
out vec4 v; // View vector in eye space
out vec2 iTexCoord0; // Primary texture coordinates
out vec2 iTexCoord1; // Secondary texture coordinates (normal-based)
out vec4 vertexColor; // Pass vertex color to fragment shader

void main(void) {
    vec4 lp = vec4(lightPos, 1.0); // Light position in world space
    vec4 vertexEyeSpace = V * M * vertex; // Vertex in eye space
    vec4 lightEyeSpace = V * lp; // Transform light position to eye space
    
    l = normalize(lightEyeSpace - vertexEyeSpace); // Vector towards the light in eye space
    v = normalize(vec4(0, 0, 0, 1) - vertexEyeSpace); // Vector towards the viewer in eye space
    n = normalize(V * M * vec4(normal, 0.0)); // Normal vector in eye space

    iTexCoord0 = texcoord;
    iTexCoord1 = (n.xy + 1.0) / 2.0; // Normal-based texture coordinates
    vertexColor = color; // Pass vertex color to fragment shader

    gl_Position = P * V * M * vertex;
}
