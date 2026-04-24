#version 450

layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec2 inTexCoord;
layout(location = 2) in  vec3 inNormal;
layout(location = 3) in  vec2 inPara;
layout(location = 4) in  uint inMaterial;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat uint fragTexIndex;
layout(location = 3) out flat uint fragShadow;

layout(set = 2, binding = 0) readonly uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

struct ObjectData {
    uint index;
    mat4 model;
    bool shadow;
    vec4 color;
};

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} instance;

void main() {
    vec4 position = vec4(inPosition, 1.0);
    vec4 normal = vec4(inNormal, 0.0);
    mat4 worldTransform = (
        instance.objects[gl_InstanceIndex].model
    );

    gl_Position = (
        ubo.proj *
        ubo.view *
        worldTransform *
        position
    );

    fragColor = instance.objects[gl_InstanceIndex].color;
    fragTexCoord = inTexCoord;
    fragTexIndex = instance.objects[gl_InstanceIndex].index;
    fragShadow = instance.objects[gl_InstanceIndex].shadow ? 1 : 0;
}
