#version 450

layout(location = 0) in  vec3 inPosition;
layout(location = 1) in  vec3 inNormals;
layout(location = 2) in  vec2 inTexCoords;
layout(location = 3) in  vec3 inColor;
layout(location = 4) in  vec4 inWeight;
layout(location = 5) in  vec4 inJoint;
layout(location = 6) in  vec3 inMorphPos1;
layout(location = 7) in  vec3 inMorphPos2;
layout(location = 8) in  vec3 inMorphPos3;
layout(location = 9) in  vec3 inMorphPos4;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoords;
layout(location = 2) out flat uint fragTexIndex;
layout(location = 3) out flat uint shadow;

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

struct NodeData {
    mat4 animation;
    float weight1;
    float weight2;
    float weight3;
    float weight4;

    int jointToNodeID;
    int a;
    int b;
    int c;
};

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer{
	ObjectData objects[];
} instance;

layout(std140, set = 0, binding = 1) readonly buffer NodeBuffer1{
    mat4 transform[];
} node1;

layout(std140, set = 0, binding = 2) readonly buffer NodeBuffer2{
    NodeData data[];
} node2;

layout(push_constant) uniform constants {
	int nodeID;
    int materialID;
} PushConstants;

vec3 applyMorph(vec3 pos) {
    vec3 result = pos;

    result += node2.data[PushConstants.nodeID].weight1 * inMorphPos1;
    result += node2.data[PushConstants.nodeID].weight2 * inMorphPos2;
    result += node2.data[PushConstants.nodeID].weight3 * inMorphPos3;
    result += node2.data[PushConstants.nodeID].weight4 * inMorphPos4;

    return result;
}

int getNodeID(float ID) {
    return node2.data[int(ID)].jointToNodeID;
}

mat4 applyBoneTransform() {
    mat4 result = mat4(0.0);

    if (all(equal(inWeight, vec4(0.0)))) {
        result = mat4(1.0);
    }
    else for (int i = 0; i < 4; i += 1) {
        result += node2.data[getNodeID(inJoint[i])].animation * inWeight[i];
    }

    return result;
}

void main() {
    vec4 position = vec4(applyMorph(inPosition), 1.0);
    mat4 worldTransform = (
        instance.objects[gl_InstanceIndex].model *
        node2.data[PushConstants.nodeID].animation *
        node1.transform[PushConstants.nodeID]
    );

    vec3 fragVertex = (
        worldTransform *
        applyBoneTransform() *
        position
    ).xyz;

    gl_Position = (
        ubo.proj *
        ubo.view *
        vec4(fragVertex, 1.0)
    );

    fragColor = instance.objects[gl_InstanceIndex].color;
    fragTexCoords = inTexCoords;
    fragTexIndex = instance.objects[gl_InstanceIndex].index;
    shadow = instance.objects[gl_InstanceIndex].shadow ? 1 : 0;
}
