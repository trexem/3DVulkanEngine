#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 inPosWorld;
layout(location = 3) in vec3 inNormalWorld;

layout (location = 0) out vec4 outColor;

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
	vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(inNormalWorld);

	vec3 cameraPosWorld = ubo.inverseView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - inPosWorld);

	for (int i = 0; i < ubo.numLights; i++) {
		PointLight light = ubo.pointLights[i];
		vec3 lightDirection = light.position.xyz - inPosWorld;
		float attenuation = 1.0 / dot(lightDirection, lightDirection);
		lightDirection = normalize(lightDirection);
		float cosAngIncidence = max(dot(surfaceNormal, lightDirection),0);
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngIncidence;

		//specular light
		vec3 halfAngle = normalize(lightDirection + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 32.0);
		specularLight += intensity * blinnTerm;
	}
    vec4 texColor = texture(texSampler, fragTexCoord);

	if (textureUbo.texIndex == 0) {
        outColor = vec4((diffuseLight + specularLight) * texColor.rgb, 1.0);
    } else if (textureUbo.texIndex == 1) {
		if(fragTexCoord.x > 0.5) {
			discard;
		} else {
			outColor = vec4((diffuseLight + specularLight) * texColor.rgb, 1.0);
		}
    } else {
        outColor = vec4((diffuseLight + specularLight) * texColor.rgb, 1.0);
    }
}
