#pragma once 
#include "GaugeValue.hpp"
#include <chrono>

template<typename T, std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, std::nullptr_t> = nullptr>
class GaugeValueManager : public GaugeValue<T> {
private:
	using sysclock = std::chrono::system_clock;
	T NotApplyDamageToGraph;
	T ApplySpeed;
	std::chrono::system_clock::time_point LastChangeTime;
public:
	GaugeValueManager() : GaugeValueManager({ 0, std::numeric_limits<T>::max() }, { 0, std::numeric_limits<T>::max() }) {}
	//GaugeValueManager(const PossibleChangeStatusArrange<T> StartParam) : GaugeValueManager(StartParam, StartParam) {}
	GaugeValueManager(const PossibleChangeStatusArrange<T> Real, const PossibleChangeStatusArrange<T> Graph)
		: GaugeValue<T>(Real, Graph), NotApplyDamageToGraph(*Graph - *Real), ApplySpeed(0), LastChangeTime() {}
	GaugeValueManager(const GaugeValue<T>& Param)
		: GaugeValueManager(Param.RealParameter, Param.GraphParameter) {}
	bool IsMin() const noexcept { return this->RealParameter.IsMin(); }
	bool IsMax() const noexcept { return this->RealParameter.IsMax(); }
	void Update(const T NewPoint) {
		this->RealParameter = NewPoint;
		this->LastChangeTime = sysclock::now();
	}
	GaugeValueManager& operator += (const T AddPoint) {
		this->RealParameter += AddPoint;
		this->LastChangeTime = sysclock::now();
		return *this;
	}
	GaugeValueManager& operator -= (const T SubtractPoint) {
		this->RealParameter -= SubtractPoint;
		this->LastChangeTime = sysclock::now();
		return *this;
	}
	void Apply() {
		this->NotApplyDamageToGraph = this->GraphParameter.Get() - this->RealParameter.Get();
		if (this->NotApplyDamageToGraph == 0) return;
		else if (this->NotApplyDamageToGraph > 0) this->ApplySpeed = std::max(std::max(1, this->NotApplyDamageToGraph / 20), this->ApplySpeed);
		else this->ApplySpeed = std::min(std::min(-1, this->NotApplyDamageToGraph / 20), this->ApplySpeed);
		this->GraphParameter -= this->ApplySpeed;
		this->NotApplyDamageToGraph -= this->ApplySpeed;
		this->ApplySpeed = 0;
	}
};
