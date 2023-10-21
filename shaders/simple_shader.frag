#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inPosWorld;
layout(location = 2) in vec3 inNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionView;
    vec4 ambientLightColor;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec3 lightDirection = ubo.lightPosition - inPosWorld;
	float attenuation = 1.0 / dot(lightDirection, lightDirection);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseLight = lightColor * max(dot(normalize(inNormalWorld), normalize(lightDirection)),0);

	outColor = vec4((diffuseLight + ambientLight) * inColor, 1.0);
}