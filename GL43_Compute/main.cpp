#include <cstdlib>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "GL43Window.h"
#include "GL43WindowInitException.h"

int main(int argc, char **argv)
{
	try
	{
		GL43Window *window = GL43Window::get_instance();
		window->main_loop();
		delete window;
	}
	catch (GL43WindowInitException &e)
	{
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}
