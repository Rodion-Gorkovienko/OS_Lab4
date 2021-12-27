#pragma once

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <vector>
#include <memory>

unsigned char getch_secure(unsigned char beg, unsigned char end)
{
	unsigned char in = _getch();
	if (in < beg || in > end)
	{
		system("cls");
		std::cout << "Value is incorrect.";
		in = 0;
		system("pause");
	}
	return in;
}

void menu_pause() {
	std::cout << "Press any key to return to the menu..." << std::endl;
	_getch();
}

std::string* gets_secure() {
	std::string* str = new std::string;
	do {
		system("cls");
		std::cout << "Enter the new value:" << std::endl;
		if (!(std::cin >> *str) || (*str)[0] == '\0' || (*str)[0] == EOF) {
			str->clear();
			std::cout << "Something wrong. Try again." << std::endl;
		}
		else if ((*str)[0] == '\n') {
			str->clear();
			std::cout << "String is empty. Try again." << std::endl;
		}
		else for (char& c : *str) {
			if (c == '\n')
				c = '\0';
		}
	} while (str->size() == 0);
	return str;
}

class Property{
public:
	Property(std::shared_ptr<std::string> type, std::shared_ptr<std::string> val, bool inherited = true) : type(type), val(val), inherited(inherited) {}
	std::string get_value() {
		return *val;
	}
	std::string get_type() {
		return *type;
	}
	void set_value(std::string* new_val) {
		*val = *new_val;
	}
	void set_value(std::shared_ptr<std::string> new_val) {
		*val = *new_val;
	}
	bool is_inherted() {
		return inherited;
	}
private:
	std::shared_ptr<std::string> type;
	std::shared_ptr<std::string> val;
	bool inherited;
};

typedef std::shared_ptr<std::vector<Property>> Property_vector;

class  Menu_item {
public:
	Menu_item(const std::string& name) :name(name) {
		std::vector<Property>* vec = new std::vector<Property>;
		property_list = std::make_shared<std::vector<Property>>(*vec);
	}
	virtual void execute() {};
	std::string get_name() { return name; }
	virtual void add_property(std::shared_ptr<std::string> type, std::shared_ptr<std::string> name, bool inherited = true);
	bool check_type(std::string);

protected:
	const std::string name;
	Property_vector property_list;
};

class Action : public Menu_item {
public:
	Action(const std::string& name, void (*func)(std::vector<std::string>*)) : Menu_item(name), func(func) {}
	void execute() override;
private:
	void (*func)(std::vector<std::string>*);
};

typedef std::shared_ptr<std::vector<std::shared_ptr<Menu_item>>> Item_vector;

class Menu : public Menu_item {
public:

	Menu(const std::string& name) : Menu_item(name) {
		items = std::make_shared<std::vector<std::shared_ptr<Menu_item>>>();
	}
	std::shared_ptr<Menu> add_item(const std::string& name);
	std::shared_ptr<Action> add_item(const std::string& name, void (*func)(std::vector<std::string>*));
	void execute() override;
	void print_items();
	virtual void add_property(std::shared_ptr<std::string> type, std::shared_ptr<std::string> name, bool inherited = true) override;
private:
	Item_vector items = nullptr;
};

void Menu::execute() {
	unsigned char choice = 0;
	do {
		do {
			print_items();
			choice = getch_secure('0', '0' + items->size() + property_list->size());
			choice -= '0';
		} while (choice < 0 || choice > items->size() + property_list->size() + 1);
		if (choice > 0 && choice <= items->size()) (*items)[choice - 1]->execute();
		if (choice > items->size() && choice <= items->size() + property_list->size() + 1) {
			(*property_list)[choice - items->size() - 1].set_value(gets_secure()); 
		}
	} while (choice != 0);
}

void Action::execute() {
	system("cls");
	std::vector<std::string>* attributes = new std::vector<std::string>;
	unsigned char i = 0;
	for (auto prop : *property_list) {
		attributes->push_back(prop.get_value());
		i++;
	}
	func(attributes);
	menu_pause();
}

void Menu::print_items() {
	system("cls");
	std::cout << name << std::endl;
	std::cout << "0. Exit" << std::endl << std::endl;
	int i = 1;

	for (auto& item : *items) {
		std::cout << i << ". " << item->get_name() << std::endl;
		i++;
	}

	if (!(property_list->empty())) {
		std::cout << std::endl << "Property (select to change): " << std::endl;
		for (auto& prop : *property_list) {
			std::cout << i << ". " << prop.get_type() << ": " << prop.get_value() << std::endl;
			i++;
		}
	}
	std::cout << ">  ";
}

void Menu_item::add_property(std::shared_ptr<std::string> type, std::shared_ptr<std::string>  val, bool inherited) {
	if (!check_type(*type)) {
		property_list->push_back(Property(type, val, inherited));
	}
}

void Menu::add_property(std::shared_ptr<std::string> type, std::shared_ptr<std::string>  val, bool inherited) {
	if (!check_type(*type)) {
		property_list->push_back(Property(type, val, inherited));
		if (inherited)
			for(auto& item : *items)
				item->add_property(type, val);
	}
}

bool Menu_item::check_type(std::string type) {
	bool exist = false;
	for(auto& prop : *property_list)
		if (prop.get_type() == type) exist = true;
	return exist;
}

std::shared_ptr<Menu> Menu::add_item(const std::string& name) {
	auto ptr = std::make_shared<Menu>(name);
	items->push_back(ptr);
	return ptr;
}

std::shared_ptr<Action> Menu::add_item(const std::string& name, void (*func)(std::vector<std::string>*)) {
	auto ptr = std::make_shared<Action>(name, func);
	items->push_back(ptr);
	return ptr;
}