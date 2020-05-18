#include "shader.h"

Shader::Shader() {

}

Shader::~Shader() {
	glDetachShader(shader_id, shader_fp);
	glDetachShader(shader_id, shader_vp);

	glDeleteShader(shader_fp);
	glDeleteShader(shader_vp);
	glDeleteProgram(shader_id);
}

Shader::Shader(const char *vsFile, const char *fsFile) {
	init(vsFile, fsFile);
}

static char* readTextFile(const char *fileName) {
	char* text = nullptr;

	if (fileName != nullptr) {
		FILE *file = nullptr;
		fopen_s(&file, fileName, "r+");

		if (file != nullptr) {
			fseek(file, 0, SEEK_END);
			int count = ftell(file);
			rewind(file);

			if (count > 0) {
				text = (char*)malloc(sizeof(char)*(count + 1));
				count = fread(text, sizeof(char), count, file);
				text[count] = '\0';
			}
			fclose(file);
		}
	}
	return text;
}

static void validateShader(GLuint shader, const char* file = 0) {
	const unsigned int BUFFER_SIZE = 2048;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;

	glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
	if (length > 0) {
		std::cerr << "Shader " << shader << "(" << (file ? file : "") << ") compile error :" << buffer << std::endl;
	}
}

static void validateProgram(GLuint program) {
	const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;

	memset(buffer, 0, BUFFER_SIZE);
	glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
	if (length > 0)
		std::cerr << "Program " << program << " link error: " << buffer << std::endl;

	glValidateProgram(program);
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE)
		std::cerr << "Error validating shader " << program << std::endl;
}

void Shader::init(const char *vsFile, const char *fsFile) {
	shader_vp = glCreateShader(GL_VERTEX_SHADER);
	shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vsText = readTextFile(vsFile);
	const char* fsText = readTextFile(fsFile);

	if (vsText == nullptr && fsText == nullptr) {
		std::cerr << "Both vertex and fragment shader not found. At least one must be present." << std::endl;
		return;
	}

	shader_id = glCreateProgram();

	if (vsText != nullptr) {
		glShaderSource(shader_vp, 1, &vsText, NULL);

		glCompileShader(shader_vp);
		validateShader(shader_vp, vsFile);

		glAttachShader(shader_id, shader_vp);
	}
	if (fsText != nullptr) {
		glShaderSource(shader_fp, 1, &fsText, NULL);

		glCompileShader(shader_fp);
		validateShader(shader_fp, fsFile);

		glAttachShader(shader_id, shader_fp);
	}

	glLinkProgram(shader_id);
	validateProgram(shader_id);
}

GLuint Shader::id() {
	return shader_id;
}

void Shader::bind() {
	glUseProgram(shader_id);
}

void Shader::unbind() {
	glUseProgram(0);
}