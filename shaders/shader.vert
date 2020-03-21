#version 450
#extension GL_ARB_separate_shader_objects : enable

#define DS_SCENE 0
#define DS_FRAME 1
#define DS_DRAW 2

layout(binding = 0, set=DS_SCENE) uniform SceneUniformBuffer
{
	mat4 proj;
};

layout(binding = 0, set=DS_FRAME) uniform FrameUniformBuffer
{
	mat4 view;
};

layout(binding = 0, set=DS_DRAW) uniform DrawUniformBuffer
{
	mat4 model;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
	gl_Position = proj * view * model * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexCoord = inTexCoord;
}
