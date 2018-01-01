#pragma once

#include <vector>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <GL/glew.h>

#include "ConfigDefines.h"

struct Critter
{
	float x;
	float y;
	float vx;
	float vy;
};

class GL43Window
{
public:
	static GL43Window * get_instance();
	~GL43Window();
	void main_loop();

	static GL43Window *m_instance;

private:
	GL43Window();
	void cleanup();
	void initialize();
	void init_shaders();
	void init_critters();
	void init_buffers();
	void init_textures();
	void load_images();
	void draw_critters();
	void handle_mouse_click(unsigned int x, unsigned int y);

	SDL_Window *window_;
	SDL_Surface *critter_image_;
	SDL_Surface *portsmouth_image_;
	SDL_GLContext glcontext_;
	GLuint program_;
	GLuint shader_object_;
	GLuint ssbo_;				// shader storage buffer object
	GLuint texo_[2];
	GLint u_num_steps_loc_;
	GLint u_time_step_loc_;
	GLint u_num_critters_loc_;
	struct Critter critters_[NUM_CRITTERS];
};
