#pragma once
#include "PossibleChangeStatus.hpp"

template<typename T, std::enable_if_t<std::is_integral<T>::value, std::nullptr_t> = nullptr>
class PossibleChangeStatusArrange : public PossibleChangeStatus<T> {
public:
	PossibleChangeStatusArrange() : PossibleChangeStatus<T>() {}
	constexpr PossibleChangeStatusArrange(const T Max) : PossibleChangeStatus<T>(Max) {}
	constexpr PossibleChangeStatusArrange(const T Current, const T MaxStatus, const T MinStatus = 0)
		: PossibleChangeStatus<T>(Current, MaxStatus, MinStatus) {}
	int GetRatioArrange() const noexcept {
		const int Ratio = static_cast<int>((PossibleChangeStatus<T>::GetRatio() + 0.005) * 100);
		return Ratio == 100 && !PossibleChangeStatus<T>::IsMax() ? 99 : (Ratio == 0 && !PossibleChangeStatus<T>::IsMin() ? 1 : Ratio);
	}
};
