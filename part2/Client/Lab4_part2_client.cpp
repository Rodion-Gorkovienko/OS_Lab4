#include "Menu.h"
#include "ClientFunctions.h"


auto form_menu() {
	using namespace std;
	auto main_menu = std::make_shared<Menu>("Client");
	main_menu->add_item("Conect", client_pipe_create);
	main_menu->add_item("Read", client_pipe_write);
	main_menu->add_item("Disconect", client_pipe_disconect);

	return main_menu;
}

int main()
{
	form_menu()->execute();
}
