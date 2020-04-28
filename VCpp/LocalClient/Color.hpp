#pragma once
#include <DxLib.h>
#include <string>
#include <stdexcept>

class Color {
private:
	int Red, Green, Blue;
public:
	Color() = default;
	Color::Color(const int Red, const int Green, const int Blue)
		: Red(Red), Green(Green), Blue(Blue) {}
	Color::Color(const std::string& ColorCode) {
		if (ColorCode.size() != 7 || '#' != ColorCode.at(0)) throw std::runtime_error("色コードが不正です。");
		auto Convert = [&ColorCode](const size_t Start) { return std::stoi(ColorCode.substr(Start, 2), nullptr, 16); };
		this->Red = Convert(1);
		this->Green = Convert(3);
		this->Blue = Convert(5);
	}
	unsigned int GetColorCode() const { return DxLib::GetColor(this->Red, this->Green, this->Blue); }
};
