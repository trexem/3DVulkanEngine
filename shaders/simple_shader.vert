#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outPosWorld;
layout(location = 3) out vec3 outNormalWorld;

struct PointLight {
    vec4 position;
    vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 inverseView;
    vec4 ambientLightColor;
    PointLight pointLights[10];
    int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform TextureUbo {
	int texIndex;
} textureUbo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);

	gl_Position = ubo.projection * ubo.view * positionWorld;
	outPosWorld = positionWorld.xyz;
	fragColor = inColor;
	fragTexCoord = uv;
	
	outNormalWorld = normalize(mat3(push.normalMatrix) * normal);
}
