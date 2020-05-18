#pragma once

#include <math.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

class Shader {
public:
	Shader();
	~Shader();
	Shader(const char*vsFile, const char *fsFile);
	void init(const char *vsFile, const char *fsFile);
	void bind();
	void unbind();
	GLuint id();

private:
	GLuint shader_id;
	GLuint shader_vp;
	GLuint shader_fp;
};