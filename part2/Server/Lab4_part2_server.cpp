#include "Menu.h"
#include "ServerFunctions.h"


auto form_menu() {
	using namespace std;
	auto main_menu = std::make_shared<Menu>("Server");
	main_menu->add_item("Create", server_pipe_create);
	main_menu->add_item("Write", server_pipe_write);
	main_menu->add_item("Disconect", server_pipe_disconect);

	main_menu->add_property(make_shared<string>("Data to write"), make_shared<string>("Cake is a lie"));
	return main_menu;
}

int main()
{
	form_menu()->execute();
}
