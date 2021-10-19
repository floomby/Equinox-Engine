#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 normal;
    float highlight;
} ubo;

layout(binding = 3) uniform ViewProjPosNearFar {
    mat4 view;
    mat4 proj;
    vec3 pos;
    vec2 nearFar;
} lighting;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 vertNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out float highlight;
layout(location = 3) out vec3 normalInterp;
layout(location = 4) out vec3 vertPos;
layout(location = 5) out vec3 shadowCoord;

layout( push_constant ) uniform constants {
    mat4 view;
    mat4 projection;
    int index;
} pushConstants;

void main() {
    vec4 worldPos = ubo.model * vec4(inPosition, 1.0);
    vec4 vertPos4 = pushConstants.view * worldPos;
    gl_Position = pushConstants.projection * vertPos4;
    vertPos = vec3(vertPos4) / vertPos4.w;
    normalInterp = vec3(ubo.normal * vec4(vertNormal, 0.0));
    fragTexCoord = inTexCoord;
    highlight = ubo.highlight;
    // Leave this in for now for debugging stuff if we want
    fragColor = inColor;

    vec4 inLightingSpace = lighting.proj * lighting.view * worldPos;
    shadowCoord = inLightingSpace.xyz / inLightingSpace.w;
}
