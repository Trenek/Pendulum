#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inBezzier;
layout(location = 3) in uint inInOut;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragBezzier;
layout(location = 2) out flat uint fragInOut;
layout(location = 3) out flat uint shadow;

layout(set = 2, binding = 0) readonly uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

struct ObjectData {
    uint index;
    mat4 model;
    bool shadow;
};

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} instance;

layout(std140, set = 0, binding = 1) readonly buffer MeshBuffer{
	mat4 localModel[];
} mesh;

layout(push_constant) uniform constants {
	int meshID;
} PushConstants;

void main() {
    //gl_Position = mesh.localModel[PushConstants.meshID] * vec4(inPosition, 1.0);
    gl_Position = vec4((
        ubo.proj *
        instance.objects[gl_InstanceIndex].model * 
        mesh.localModel[PushConstants.meshID] * 
        vec4(inPosition.xy, 0.0, 1.0)
    ).xy, 0.0, 1.0);

    fragColor = inColor;
    fragBezzier = inBezzier;
    fragInOut = inInOut;
    shadow = instance.objects[gl_InstanceIndex].shadow ? 1 : 0;
}
