#version 330

in vec2 vTexcoord;
in vec3 vNormal;
in vec3 vFragPos;
out vec4 outColor;

uniform sampler2D barkTex;
uniform sampler2D leafTex;
uniform int useBarkTex;
uniform int useLeafTex;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    vec3 color;
    if (useLeafTex == 1) {
        color = texture(leafTex, vTexcoord).rgb;
    } else if (useBarkTex == 1) {
        color = texture(barkTex, vTexcoord).rgb;
    } else {
        color = vec3(0.2, 0.7, 0.2);
    }
    // Simple Lambertian diffuse lighting
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(lightPos - vFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    float ambientStrength = 0.18; // Slightly higher ambient
    float diffuseStrength = 0.7;  // Lower diffuse
    vec3 ambient = ambientStrength * lightColor;
    vec3 diffuse = diffuseStrength * diff * lightColor;
    vec3 result = (ambient + diffuse) * color;
    outColor = vec4(result, 1.0);
}
