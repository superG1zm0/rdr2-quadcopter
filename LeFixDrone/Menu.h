#pragma once
#include "nativesExtended.h"
#include <iomanip>

class Menu
{
public:
	Menu();
	void CloseMenu();
	int  getSubmenu();
	void addTitle(const char* title);
	void addHeader(const char* header);
	bool addOption(const char* option, const char* details = NULL);
	void addSubmenuOption(const char* option, int submenu, const char* details = NULL);
	bool addBoolOption(const char* option, bool* var, const char* details = NULL);
	bool addIntOption(const char* option, int* var, int min, int max, int step = 1, const char* details = NULL);
	bool addFloatOption(const char* option, float* var, float min, float max, float step = 1.0f, int precision = 3, const char* details = NULL);
	bool addStringOption(const char* option, const char* var, int* intvar, int elementCount, const char* details = NULL);
	void resetVars();
	void ButtonMonitoring();
	
private:
	const float menuX = 0.052f;
	
	int submenu;
	int submenuLevel;
	int lastSubmenu[20];
	int lastOption[20];
	int lastSubmenuMinOptions[20];
	int lastSubmenuMaxOptions[20];
	int currentOption;
	int optionCount;
	int maxOptions;
	bool optionPress;
	int currentMenuMaxOptions;
	int currentMenuMinOptions;
	bool leftPress;
	bool rightPress;
	ULONGLONG delay;
	const char* strDetails;
	
	void changeSubmenu(int newSubmenu);
	void displayOptionIndex();
	void displayDetails();
};

enum Submenus {
	Closed,
	Main_Menu,
	cameramenu,
	controlmenu,
	dronemenu,
	gamepadmenu,
	physxmenu,
	visualmenu,
	exitmenu,
};
