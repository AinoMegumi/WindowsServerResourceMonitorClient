#pragma once
#include "PossibleChangeStatusArrange.hpp"

template<typename T, std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, std::nullptr_t> = nullptr>
class GaugeValue {
public:
	// 実際のパラメーター
	PossibleChangeStatusArrange<T> RealParameter;
	// 画面表示パラメーター
	PossibleChangeStatusArrange<T> GraphParameter;
	GaugeValue() = default;
	//BodyParameter(const PossibleChangeStatusArrange<T> StartParam) : BodyParameter(StartParam, StartParam) {}
	GaugeValue(const PossibleChangeStatusArrange<T> Real, const PossibleChangeStatusArrange<T> Graph)
		: RealParameter(Real), GraphParameter(Graph) {}
	bool IsMin() const noexcept { return this->RealParameter.IsMin(); }
	bool IsMax() const noexcept { return this->RealParameter.IsMax(); }
	GaugeValue& operator += (const T AddPoint) {
		this->RealParameter += AddPoint;
		return *this;
	}
	GaugeValue& operator -= (const T SubtractPoint) {
		this->RealParameter -= SubtractPoint;
		return *this;
	}
};
