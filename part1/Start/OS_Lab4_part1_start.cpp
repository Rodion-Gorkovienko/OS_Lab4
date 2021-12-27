#include "Menu.h"
#include "StartFunction.h"

auto form_menu() {
	using namespace std;
	auto main_menu = std::make_shared<Menu>("Start of I/O processes");
	main_menu->add_item("Start", winapi_start);

	main_menu->add_property(make_shared<string>("Number of writers"), make_shared<string>("5"));
	main_menu->add_property(make_shared<string>("Number of readers"), make_shared<string>("5"));
	main_menu->add_property(make_shared<string>("Number of pages"), make_shared<string>("17"));
	return main_menu;
}

int main()
{
	form_menu()->execute();
}
