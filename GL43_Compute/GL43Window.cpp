#include "GL43Window.h"

#include <cstdlib>
#include <ctime>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include <GL/glew.h>

#include <string>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::string;

#include "ConfigDefines.h"
#include "GL43WindowInitException.h"
#include "util.h"

GL43Window * GL43Window::get_instance()
{
	if (GL43Window::m_instance == nullptr)
		GL43Window::m_instance = new GL43Window();

	return GL43Window::m_instance;
}

GL43Window::GL43Window()
{
	try
	{
		this->initialize();
		this->init_shaders();
	}
	catch (GL43WindowInitException &e)
	{
		this->cleanup();
		throw e;
	}

	this->load_images();
	this->init_critters();
	// this->init_buffers();
	this->init_textures();
}

GL43Window::~GL43Window()
{
	this->cleanup();
}

void GL43Window::initialize()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
		throw GL43WindowInitException("Error initializing SDL");

	atexit(SDL_Quit);

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
		throw GL43WindowInitException("Error initializing SDL Image");

	atexit(IMG_Quit);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	window_ = SDL_CreateWindow(
		"GL43Window",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_OPENGL
		);

	if (window_ == NULL)
		throw GL43WindowInitException(SDL_GetError());

	glcontext_ = SDL_GL_CreateContext(window_);
	if (glcontext_ == NULL)
	{
		// store message since SDL_DestroyWindow could generate a new one
		const char *error_message = SDL_GetError();
		SDL_DestroyWindow(window_);
		throw GL43WindowInitException(error_message);
	}

	glewExperimental = true;
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		const char *error_message = (const char *)glewGetErrorString(error);
		SDL_GL_DeleteContext(glcontext_);
		SDL_DestroyWindow(window_);
		throw GL43WindowInitException(error_message);
	}

	glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
}

void GL43Window::cleanup()
{
	glDeleteBuffers(1, &ssbo_);
	glDeleteTextures(2, texo_);
	glDeleteShader(shader_object_);
	glDeleteProgram(program_);
	SDL_GL_DeleteContext(glcontext_);
	SDL_DestroyWindow(window_);
}

void GL43Window::init_shaders()
{
	program_ = glCreateProgram();
	shader_object_ = glCreateShader(GL_COMPUTE_SHADER);

	char *shader_src = NULL;
	readFile("shader.comp", &shader_src);

	glShaderSource(shader_object_, 1, &shader_src, NULL);
	glCompileShader(shader_object_);

	GLint success = GL_TRUE;
	glGetShaderiv(shader_object_, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logsize = 0;
		glGetShaderiv(shader_object_, GL_INFO_LOG_LENGTH, &logsize);
		
		char *buffer = new char[logsize];
		glGetShaderInfoLog(shader_object_, logsize, NULL, buffer);

		GL43WindowInitException e(buffer);
		delete[] buffer;
		throw e;
	}


	glAttachShader(program_, shader_object_);
	glLinkProgram(program_);

	glGetProgramiv(program_, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logsize = 0;
		glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &logsize);

		char *buffer = new char[logsize];
		glGetProgramInfoLog(program_, logsize, NULL, buffer);

		GL43WindowInitException e(buffer);
		delete[] buffer;
		throw e;
	}
}

void GL43Window::init_critters()
{
	srand((unsigned int)time(NULL));

	for (unsigned int i = 0; i < NUM_CRITTERS; ++i)
	{
		critters_[i].x = (float)(rand() % (WINDOW_WIDTH - 31));
		if (critters_[i].x >= 128 && critters_[i].x <= WINDOW_WIDTH - 160)
			critters_[i].y = (float)(rand() % (WINDOW_HEIGHT - 31));
		else
			critters_[i].y = (float)(rand() % (WINDOW_HEIGHT - 159 - 128) + 128);

		critters_[i].vx = 640.0f / 1000.0f;
		critters_[i].vy = 0.0f;
	}
}

void GL43Window::init_buffers()
{
	glGenBuffers(1, &ssbo_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_);

	glBufferData(
		GL_SHADER_STORAGE_BUFFER,
		sizeof(struct Critter) * NUM_CRITTERS,
		critters_,
		GL_DYNAMIC_COPY
	);
}

void GL43Window::init_textures()
{
	glGenTextures(2, texo_);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texo_[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA32F,
		portsmouth_image_->w, portsmouth_image_->h,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		portsmouth_image_->pixels);

	glBindImageTexture(0, texo_[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	glBindTexture(GL_TEXTURE_2D, texo_[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA32F,
		portsmouth_image_->w, portsmouth_image_->h,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL);

	glBindImageTexture(1, texo_[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void GL43Window::load_images()
{
	critter_image_ = IMG_Load("lolo.png");
	portsmouth_image_ = IMG_Load("portsmouth.png");
}

void GL43Window::draw_critters()
{
	SDL_Rect pos;
	for (unsigned int i = 0; i < NUM_CRITTERS; ++i)
	{
		pos.x = (unsigned int)critters_[i].x;
		pos.y = (unsigned int)critters_[i].y;
		SDL_BlitSurface(critter_image_, NULL, SDL_GetWindowSurface(window_), &pos);
	}
}

void GL43Window::main_loop()
{
	SDL_Surface *window_surface = SDL_GetWindowSurface(window_);

	unsigned int start_time = SDL_GetTicks();
	unsigned int delta_time = 0;

	SDL_Event event;
	bool running = true;
	while (running)
	{
		unsigned int stop_time = SDL_GetTicks();
		unsigned int frame_time = stop_time - start_time;
		delta_time += frame_time;
		start_time = stop_time;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				this->handle_mouse_click(event.motion.x, event.motion.y);
			}
		}

		glUseProgram(program_);
		
		// update
		unsigned int num_steps = delta_time / TIME_STEP;
		delta_time -= num_steps * TIME_STEP;

		// clear window to blue
		SDL_FillRect(
			SDL_GetWindowSurface(window_),
			NULL,
			SDL_MapRGB(window_surface->format, 0, 255, 255)
			);
		
		glDispatchCompute(portsmouth_image_->w / 8 + 1, portsmouth_image_->h / 8 + 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		// bind output texture for reading using glGetTexImage
		glBindTexture(GL_TEXTURE_2D, texo_[1]);

		unsigned char *pixels = (unsigned char *)malloc(portsmouth_image_->w * portsmouth_image_->h * 4);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		SDL_Surface *out_img = SDL_CreateRGBSurfaceFrom(
			pixels,
			portsmouth_image_->w, portsmouth_image_->h,
			32,
			portsmouth_image_->w * 4,
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
		);

		SDL_SaveBMP(out_img, "output_image.bmp");
		SDL_FreeSurface(out_img);
		free(pixels);

		//this->draw_critters();
		SDL_UpdateWindowSurface(window_);
	}

	SDL_GL_DeleteContext(glcontext_);
	SDL_DestroyWindow(window_);
}

void GL43Window::handle_mouse_click(unsigned int mouse_x, unsigned int mouse_y)
{
}

GL43Window * GL43Window::m_instance = nullptr;
