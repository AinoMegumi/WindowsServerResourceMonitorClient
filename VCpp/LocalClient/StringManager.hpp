#pragma once
#include "StringController.hpp"
#include "Color.hpp"
#include "DxLibHandle.hpp"

class StringManager {
public:
	int StringSize;
private:
	StringHandle handle;
	Color StringColor;
public:
	StringManager(const char* FontName, const int FontSize, const Color& StringColor)
		: StringManager(std::string(FontName), FontSize, StringColor) {}
	StringManager(const std::string& FontName, const int FontSize, const Color& StringColor) 
		: StringSize(FontSize), handle(CreateFontToHandle(CharsetManager::AlignCmdLineStrType(FontName).c_str(), FontSize, -1)), StringColor(StringColor) {}
	void Draw(const int X, const int Y, const std::string& str) const noexcept {
		DrawStringToHandle(X, Y, CharsetManager::AlignCmdLineStrType(str).c_str(), this->StringColor.GetColorCode(), this->handle);
	}
	int GetLength(const std::string& str) {
		return GetDrawStringWidth(CharsetManager::AlignCmdLineStrType(str).c_str(), static_cast<int>(std::strlen(str.c_str())));
	}
};
