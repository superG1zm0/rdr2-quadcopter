#include "menu.h"
#include "nativesExtended.h"

Menu::Menu()
{
	submenu = 0;
	maxOptions = 8;
	optionPress = false;
	currentMenuMaxOptions = maxOptions;
	currentMenuMinOptions = 1;
	leftPress = false;
	rightPress = false;
}

void Menu::CloseMenu()
{
	submenu = Closed;
	submenuLevel = 0;
	currentOption = 1;
}

int Menu::getSubmenu()
{
	return submenu;
}

void Menu::changeSubmenu(int newSubmenu)
{
	lastSubmenu[submenuLevel] = submenu;
	lastOption[submenuLevel] = currentOption;
	lastSubmenuMinOptions[submenuLevel] = currentMenuMinOptions;
	lastSubmenuMaxOptions[submenuLevel] = currentMenuMaxOptions;
	currentOption = 1;
	currentMenuMinOptions = 1;
	currentMenuMaxOptions = maxOptions;
	submenu = newSubmenu;
	submenuLevel++;
	optionPress = false;
}

void Menu::addTitle(const char* title) {
	optionCount = 0;
	UI_X::DRAW_TEXT(title, menuX + 0.13f, 0.076f, 255, 255, 255, 255, true, 0.5f);
	GRAPHICS_X::DRAW_RECT(menuX, 0.058f, 0.260f, 0.104f, 0, 0, 0, 190);
	GRAPHICS_X::DRAW_SPRITE("generic_textures", "menu_header_1a", menuX, 0.058f, 0.260f, 0.074f, 0, 255, 255, 255, 255);
	GRAPHICS_X::DRAW_SPRITE("generic_textures", "hud_menu_4a", menuX, 0.131f + 0.027f, 0.260f, 0.002f, 0, 255, 255, 255, 255);
}

void Menu::addHeader(const char* header)
{
	UI_X::DRAW_TEXT(header, menuX + 0.13f, 0.076f + 0.0575f, 255, 255, 255, 255, true, 0.3f);
}

bool Menu::addOption(const char* option, const char* details) {
	optionCount++;
	if (currentOption <= currentMenuMaxOptions && optionCount <= currentMenuMaxOptions && currentOption >= currentMenuMinOptions && optionCount >= currentMenuMinOptions) {
		UI_X::DRAW_TEXT(option, menuX + 0.007f, 0.131f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 255, 255, 255, 255);
		GRAPHICS_X::DRAW_RECT(menuX, 0.124f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 0.260f, 0.038f, 0, 0, 0, 190);
		if (currentOption == optionCount) {
			strDetails = details;
			GRAPHICS_X::DRAW_SPRITE("generic_textures", "selection_box_bg_1d", menuX, 0.124f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 0.260f, 0.038f, 0, 255, 0, 0, 190);
			if (optionPress)
			{
				return true;
			}
		}
	}
	return false;
}

void Menu::addSubmenuOption(const char* option, int submenu, const char* details) {
	addOption(option);
	if (currentOption <= currentMenuMaxOptions && optionCount <= currentMenuMaxOptions && currentOption >= currentMenuMinOptions && optionCount >= currentMenuMinOptions) {
		GRAPHICS_X::DRAW_SPRITE("menu_textures", "selection_arrow_right", menuX + 0.235f, 0.132f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 0.01125f, 0.02f, 0, 255, 255, 255, 255);
		if (currentOption == optionCount)
		{
			strDetails = details;
			if (optionPress)
				changeSubmenu(submenu);
		}
	}
}

bool Menu::addBoolOption(const char* option, bool* var, const char* details) {
	addOption(option);
	if (currentOption <= currentMenuMaxOptions && optionCount <= currentMenuMaxOptions && currentOption >= currentMenuMinOptions && optionCount >= currentMenuMinOptions) {
		if (*var) {
			GRAPHICS_X::DRAW_SPRITE("generic_textures", "tick_box", menuX + 0.232f, 0.132f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 0.0140625f, 0.025f, 0, 255, 255, 255, 255);
			GRAPHICS_X::DRAW_SPRITE("generic_textures", "tick", menuX + 0.232f, 0.132f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 0.0140625f, 0.025f, 0, 255, 255, 255, 255);
		}
		else {
			GRAPHICS_X::DRAW_SPRITE("generic_textures", "tick_box", menuX + 0.232f, 0.132f + (0.038f * ((optionCount - currentMenuMinOptions) + 1)), 0.0140625f, 0.025f, 0, 255, 255, 255, 255);
		}
		if (currentOption == optionCount)
		{
			strDetails = details;
			if (optionPress)
			{
				*var = !*var;
				return true;
			}
		}
	}
	return false;
}

bool Menu::addIntOption(const char* option, int* var, int min, int max, int step, const char* details) {
	char buffer[64];
	snprintf(buffer, 64, "%s < %i >", option, *var);
	addOption(buffer);
	if (currentOption == optionCount) {
		strDetails = details;
		if (leftPress) {
			if (*var <= min)
				*var = max;
			else
				*var -= step;
			return true;
		}
		else if (rightPress) {
			if (*var >= max)
				*var = min;
			else
				*var += step;
			return true;
		}
	}
	return false;
}

bool Menu::addFloatOption(const char* option, float* var, float min, float max, float step, const char* details) {
	char buffer[64];
	snprintf(buffer, 64, "%s < %.03f >", option, *var);
	addOption(buffer);
	if (currentOption == optionCount) {
		strDetails = details;
		if (leftPress) {
			if (*var <= min)
				*var = max;
			else
				*var -= step;
			return true;
		}
		else if (rightPress) {
			if (*var >= max)
				*var = min;
			else
				*var += step;
			return true;
		}
	}
	return false;
}

bool Menu::addStringOption(const char* option, const char* var, int* intvar, int elementCount, const char* details) {
	char buffer[64];
	snprintf(buffer, 64, "%s < %s >", option, var);
	addOption(buffer);
	if (currentOption == optionCount) {
		strDetails = details;
		if (leftPress) {
			if (*intvar <= 0)
				*intvar = elementCount;
			else
				*intvar -= 1;
			return true;
		}
		else if (rightPress)
		{
			if (*intvar >= elementCount)
				*intvar = 0;
			else
				*intvar += 1;
			return true;
		}
	}
	return false;
}

void Menu::displayOptionIndex() {
	char buffer[32];
	snprintf(buffer, 32, "%i of %i", currentOption, optionCount);
	if (optionCount >= maxOptions) {
		UI_X::DRAW_TEXT(buffer, menuX + 0.13f, 0.131f + (0.038f * (maxOptions + 1)), 255, 255, 255, 255, true);
		GRAPHICS_X::DRAW_RECT(menuX, 0.124f + (0.038f * (maxOptions + 1)), 0.260f, 0.038f, 0, 0, 0, 190);
		GRAPHICS_X::DRAW_SPRITE("generic_textures", "hud_menu_4a", menuX, 0.126f + (0.038f * (maxOptions + 1)), 0.260f, 0.002f, 0, 255, 255, 255, 255);
	} else {
		UI_X::DRAW_TEXT(buffer, menuX + 0.13f, 0.131f + (0.038f * (optionCount + 1)), 255, 255, 255, 255, true);
		GRAPHICS_X::DRAW_RECT(menuX, 0.124f + (0.038f * (optionCount + 1)), 0.260f, 0.038f, 0, 0, 0, 190);
		GRAPHICS_X::DRAW_SPRITE("generic_textures", "hud_menu_4a", menuX, 0.126f + (0.038f * (optionCount + 1)), 0.260f, 0.002f, 0, 255, 255, 255, 255);
	}
}

void Menu::displayDetails() {

	if (strDetails)
	{
		int line;
		const char* cursor;

		for (line = 0, cursor = strDetails; *cursor != '\0'; cursor++)
			if (*cursor == '\n') line++;

		int nbOptions = (optionCount >= maxOptions) ? maxOptions : optionCount;

		UI_X::DRAW_TEXT(strDetails, menuX + 0.007f, 0.141f + (0.038f * (nbOptions + 2)), 255, 255, 255, 255);
		GRAPHICS_X::DRAW_RECT(menuX, 0.134f + (0.038f * (nbOptions + 2)), 0.260f, 0.021f * (line + 2), 0, 0, 0, 190);
	}
}

/*void Menu::displayDetails2() {

	if (strDetails)
	{
		int line = 0;
		size_t strDetailSize = strlen(strDetails);

		char* buffer = (char*)malloc(strDetailSize + 1);

		if (buffer == NULL) return;

		for (unsigned int i = 0, j = 0; *(strDetails + i) != NULL; i++)
		{
			if (*(strDetails + i) == ' ')
			{
				strncpy_s(buffer, strDetailSize + 1, strDetails, i);

				// Must be replace by a GET_STRING_WIDTH function
				if (UI::GET_LENGTH_OF_LITERAL_STRING(buffer) > 31)
				{
					if (j)
					{
						strncpy_s(buffer, strDetailSize + 1, strDetails, j);
						strDetails = strDetails + j + 1;
					}
					else
					{
						strncpy_s(buffer, strDetailSize + 1, strDetails, i);
						strDetails = strDetails + i + 1;
					}
					i = 0;

					if (optionCount >= maxOptions) {
						UI_X::DRAW_TEXT(buffer, menuX + 0.007f, 0.141f + (0.038f * (maxOptions + 2)) + 0.02f * line, 255, 255, 255, 255);
					}
					else {
						UI_X::DRAW_TEXT(buffer, menuX + 0.007f, 0.141f + (0.038f * (optionCount + 2)) + 0.02f * line, 255, 255, 255, 255);
					}

					line++;
				}
				j = i;
			}
		}
		if (optionCount >= maxOptions) {
			UI_X::DRAW_TEXT(strDetails, menuX + 0.007f, 0.141f + (0.038f * (maxOptions + 2)) + 0.02f * line, 255, 255, 255, 255);
			GRAPHICS_X::DRAW_RECT(menuX, 0.134f + (0.038f * (maxOptions + 2)), 0.260f, 0.02f * (line + 2), 0, 0, 0, 190);
		}
		else {
			UI_X::DRAW_TEXT(strDetails, menuX + 0.007f, 0.141f + (0.038f * (optionCount + 2)) + 0.02f * line, 255, 255, 255, 255);
			GRAPHICS_X::DRAW_RECT(menuX, 0.134f + (0.038f * (optionCount + 2)), 0.260f, 0.02f * (line + 2), 0, 0, 0, 190);
		}

		free(buffer);
	}
}*/

void Menu::resetVars()
{
	if (submenu != Closed) {
		displayOptionIndex();
		displayDetails();
	}
	optionPress = false;
	rightPress = false;
	leftPress = false;
}

void Menu::ButtonMonitoring()
{
	if (submenu == Closed)
	{
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_LT")) || CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_LT")))
		{
			submenu = Main_Menu;
			submenuLevel = 0;
			currentOption = 1;
			currentMenuMinOptions = 1;
			currentMenuMaxOptions = maxOptions;
		}
	}
	else
	{
		if (CONTROLS::IS_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_ACCEPT"))) { //Enter
			optionPress = true;
		}

		if (CONTROLS::IS_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_CANCEL"))) //Backspace
		{
			if (submenu == Main_Menu) {
				CloseMenu();
			}
			else
			{
				submenu = lastSubmenu[submenuLevel - 1];
				currentOption = lastOption[submenuLevel - 1];
				currentMenuMinOptions = lastSubmenuMinOptions[submenuLevel - 1];
				currentMenuMaxOptions = lastSubmenuMaxOptions[submenuLevel - 1];
				submenuLevel--;
			}
		}

		if (CONTROLS::IS_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_UP"))) //Scroll Up
		{
			if (currentOption == 1)
			{
				currentOption = optionCount;
				currentMenuMaxOptions = optionCount;
				if (optionCount > maxOptions)
					currentMenuMinOptions = optionCount - maxOptions + 1;
				else
					currentMenuMinOptions = 1;
			}
			else
			{
				currentOption--;
				if (currentOption < currentMenuMinOptions) {
					currentMenuMinOptions = currentOption;
					currentMenuMaxOptions = currentOption + maxOptions - 1;
				}
			}
		}

		if (CONTROLS::IS_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_DOWN"))) //Scroll Down
		{
			if (currentOption == optionCount)
			{
				currentOption = 1;
				currentMenuMinOptions = 1;
				currentMenuMaxOptions = maxOptions;
			}
			else
			{
				currentOption++;
				if (currentOption > currentMenuMaxOptions) {
					currentMenuMaxOptions = currentOption;
					currentMenuMinOptions = currentOption - maxOptions + 1;
				}
			}
		}

		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_LEFT"))) //Scroll Left
		{
			delay = GetTickCount64() + 500;
			leftPress = true;
		}
		if (CONTROLS::IS_DISABLED_CONTROL_JUST_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_RIGHT"))) //Scroll Right
		{
			delay = GetTickCount64() + 500;
			rightPress = true;
		}

		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_LEFT"))) { 
			if (GetTickCount64() > delay)
			{
				delay = GetTickCount64() + 50;
				leftPress = true;
			}
		}
		if (CONTROLS::IS_DISABLED_CONTROL_PRESSED(0, GAMEPLAY_X::JOAAT("INPUT_FRONTEND_RIGHT"))) {
			if (GetTickCount64() > delay)
			{
				delay = GetTickCount64() + 50;
				rightPress = true;
			}
		}
	}
}
