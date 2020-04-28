#pragma once
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "GaugeValueManager.hpp"
#include "StringController.hpp"
#include "StringManager.hpp"
#include "Color.hpp"
#include <picojson/picojson.h>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <sstream>

class ResponseProcessingManager {
private:
	static constexpr size_t CharBufferSize = 1024;
	class Base {
	public:
		class GraphicInformation {
		private:
			GraphicHandle handle;
		public:
			int Radius;
		private:
			Color Background;
			int GaugeWidth;
			double DrawStartPos;
			double NoUseArea;
			static constexpr int MaxVertexOfTriangle = 120;
			static constexpr int digit(const double val) { return static_cast<int>(val + 0.5); }
			static constexpr double ToRadian(const double Vertex) { return Vertex * M_PI / 180; }
			static inline double GetSinVal(const double Vertex) { return std::sin(ToRadian(Vertex)); }
			static inline double GetCosVal(const double Vertex) { return std::cos(ToRadian(Vertex)); }
			double CalcDrawPercent(const double Percent) {
				return this->DrawStartPos - (Percent * (100.0 - this->NoUseArea) / 100.0);
			}
		public:
			GraphicInformation(const std::string& FilePath, const std::string& BackgroundColor = "#ffffff", const int GaugeWidth = 10, const double DrawStartPos = -25.0, const double NoUseArea = 50.0)
				: handle(DxLib::LoadGraph(CharsetManager::AlignCmdLineStrType(FilePath).c_str())), Background(BackgroundColor), GaugeWidth(GaugeWidth), DrawStartPos(DrawStartPos), NoUseArea(NoUseArea) {
				if (this->handle == -1) throw std::runtime_error("Failed to load graph image\nPath : " + FilePath);
				int X = 0, Y = 0;
				DxLib::GetGraphSize(this->handle, &X, &Y);
				this->Radius = X / 2;
			}
			void Draw(const int X, const int Y, const double Percent) const noexcept {
				static const Color Black = Color("#000000");
				// Draw関数の引数が左上座標なので半径を足すことで場所を調整

				// 黒いエリアのサイズを調整するために-3を加えている
				DrawCircle(X + this->Radius, Y + this->Radius, this->Radius - 3, Black.GetColorCode());
				DrawCircleGauge(X + this->Radius, Y + this->Radius, Percent, this->handle, this->DrawStartPos);
				DrawCircle(X + this->Radius, Y + this->Radius, this->Radius - this->GaugeWidth, this->Background.GetColorCode());
			}
		};
		class ResponsePercentDataProcessor {
		protected:
			GaugeValueManager<int> Val;
			GraphicInformation GraphInfo;
			std::reference_wrapper<StringManager> string;
			virtual std::string GetViewTextOnGraph() const = 0;
			virtual std::string GetViewTextInGraph() const = 0;
			virtual std::string GetViewTextUnderGraph() const = 0;
			virtual void UpdateResourceInfo(const picojson::object& obj) = 0;
		public:
			ResponsePercentDataProcessor(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff", const int GaugeWidth = 10, const double DrawStartPos = -25.0, const double NoUseArea = 50.0)
				: string(string), Val({ 0, 100 }), GraphInfo(FilePath, BackgroundColor, GaugeWidth, DrawStartPos, NoUseArea) {}
		private:
			void DrawImpl(const int X, const int Y) const {
				this->GraphInfo.Draw(X, Y + this->string.get().StringSize, this->Val.GraphParameter.Get<double>());
				if (const std::string Text = GetViewTextOnGraph(); !Text.empty()) 
					this->string.get().Draw(X, Y, Text);
				if (const std::string Text = GetViewTextInGraph(); !Text.empty()) 
					this->string.get().Draw(X + this->GraphInfo.Radius - this->string.get().GetLength(Text.c_str()) / 2, Y + this->GraphInfo.Radius + (this->string.get().StringSize / 2), Text);
				if (const std::string Text = GetViewTextUnderGraph(); !Text.empty())
					this->string.get().Draw(X, Y + this->GraphInfo.Radius * 2 + this->string.get().StringSize, Text);
			}
		public:
			void Draw(const int X, const int Y) const { return this->DrawImpl(X + 1, Y + 1); }
			void UpdateVal(const double New) {
				this->Val.Update(static_cast<int>(New));
			}
			void ApplyViewParameter() {
				this->Val.Apply();
			}
			int GetRadius() const noexcept { return this->GraphInfo.Radius; }
			void Update(const picojson::object& obj) {
				try {
					this->UpdateResourceInfo(obj);
				}
				catch(...) {}
			}
		};
		class TransferPercentManager {
		private:
			double Max;
			double Current;
		public:
			TransferPercentManager() = default;
			double Calc(const double& Transfer) {
				this->Current = Transfer;
				this->Max = std::max(this->Max, this->Current);
				return Transfer / this->Max;
			}
			double GetCurrent() const noexcept { return this->Current; }
		};
	};
	class Processor : public Base::ResponsePercentDataProcessor {
	private:
		std::string ProcessorName;
		int ProcessNum;
		std::string GetViewTextOnGraph() const override {
			return "CPU";
		}
		std::string GetViewTextInGraph() const override {
			char Buffer[CharBufferSize];
			sprintf_s(Buffer, CharBufferSize, "Use: %d%% / Process: %d", Base::ResponsePercentDataProcessor::Val.RealParameter.Get(), this->ProcessNum);
			return std::string(Buffer);
		}
		std::string GetViewTextUnderGraph() const override {
			return "";
		}
  	public:
		Processor(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), ProcessorName(), ProcessNum() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::string.get().Draw(X + 1, Y + 1, "CPU");
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
	private:
		void UpdateResourceInfo(const picojson::object& data) override {
			if (this->ProcessorName.empty()) this->ProcessorName = data.at("name").get<std::string>();
			const double val = data.at("usage").get<double>();
			Base::ResponsePercentDataProcessor::UpdateVal(val);
			this->ProcessNum = static_cast<int>(data.at("process").get<double>());
		}
	public:
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class Memory : public Base::ResponsePercentDataProcessor {
	private:
		double TotalMemory;
		double MemoryUsed;
		std::string GetViewTextOnGraph() const override {
			return "Memory";
		}
		std::string GetViewTextInGraph() const override {
			char Buffer[CharBufferSize];
			sprintf_s(Buffer, CharBufferSize, "%.2lf / %.2lf MB", this->MemoryUsed, this->TotalMemory);
			return std::string(Buffer);
		}
		std::string GetViewTextUnderGraph() const override {
			return "";
		}
	public:
		Memory(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), TotalMemory(), MemoryUsed() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
	private:
		void UpdateResourceInfo(const picojson::object& data) override {
			Base::ResponsePercentDataProcessor::UpdateVal(data.at("usedper").get<double>());
			this->MemoryUsed = data.at("used").get<double>();
			this->TotalMemory = data.at("total").get<double>(); // 仮想メモリ全体の容量はシステムの状態によって変化することがあるから変更可能にしておく必要あり
		}
	public:
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class DiskUsage : public Base::ResponsePercentDataProcessor {
	private:
		std::string Drive;
		std::pair<double, std::string> DiskUsedVal;
		std::pair<double, std::string> DiskTotal;
		std::string GetViewTextOnGraph() const override {
			return "Disk(" + this->Drive + ")";
		}
		std::string GetViewTextInGraph() const override {
			char Buffer[CharBufferSize];
			sprintf_s(
				Buffer, CharBufferSize, "%.2lf%s / %.2lf %s",
				this->DiskUsedVal.first, this->DiskUsedVal.second.c_str(),
				this->DiskTotal.first, this->DiskTotal.second.c_str()
			);
			return std::string(Buffer);
		}
		std::string GetViewTextUnderGraph() const override {
			return "";
		}
	public:
		DiskUsage(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), DiskTotal(), DiskUsedVal() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::Draw(X, Y);
		}
	private:
		void UpdateImpl(const picojson::object& diskused, const picojson::object& disktotal) {
			Base::ResponsePercentDataProcessor::UpdateVal(diskused.at("per").get<double>());
			this->DiskUsedVal = std::make_pair(diskused.at("capacity").get<double>(), diskused.at("unit").get<std::string>());
			this->DiskTotal = std::make_pair(disktotal.at("capacity").get<double>(), diskused.at("unit").get<std::string>());
		}
		void UpdateResourceInfo(const picojson::object& DiskInfo) override {
			if (this->Drive.empty()) this->Drive = DiskInfo.at("drive").get<std::string>();
			this->UpdateImpl(DiskInfo.at("used").get<picojson::object>(), DiskInfo.at("total").get<picojson::object>());
		}
	public:
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class DiskRead : public Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
		std::string GetViewTextOnGraph() const override {

		}
		std::string GetViewTextInGraph() const override {

		}
		std::string GetViewTextUnderGraph() const override {

		}
	public:
		DiskRead(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
	private:
		void UpdateResourceInfo(const picojson::object& DiskInfo) override {
			Base::ResponsePercentDataProcessor::UpdateVal(this->Transfer.Calc(DiskInfo.at("read").get<double>()));
		}
	public:
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class DiskWrite : public Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
		std::string GetViewTextOnGraph() const override {

		}
		std::string GetViewTextInGraph() const override {

		}
		std::string GetViewTextUnderGraph() const override {

		}
	public:
		DiskWrite(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
	private:
		void UpdateResourceInfo(const picojson::object& DiskInfo) override {
			Base::ResponsePercentDataProcessor::UpdateVal(this->Transfer.Calc(DiskInfo.at("write").get<double>()));
		}
	public:
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class NetworkReceive : public Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
		std::string GetViewTextOnGraph() const override {

		}
		std::string GetViewTextInGraph() const override {

		}
		std::string GetViewTextUnderGraph() const override {

		}
	public:
		NetworkReceive(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
	private:
		void UpdateResourceInfo(const picojson::object& NetworkInfo) override {
			Base::ResponsePercentDataProcessor::UpdateVal(this->Transfer.Calc(NetworkInfo.at("read").get<double>()));
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	class NetworkSend : public Base::ResponsePercentDataProcessor {
	private:
		Base::TransferPercentManager Transfer;
		std::string GetViewTextOnGraph() const override {

		}
		std::string GetViewTextInGraph() const override {

		}
		std::string GetViewTextUnderGraph() const override {

		}
	public:
		NetworkSend(StringManager& string, const std::string& FilePath, const std::string& BackgroundColor = "#ffffff")
			: Base::ResponsePercentDataProcessor(string, FilePath, BackgroundColor, 10, 2.0 / 3.0, 1.0 / 3.0), Transfer() {}
		void Draw(const int X, const int Y) const {
			Base::ResponsePercentDataProcessor::Draw(X, Y);

		}
	private:
		void UpdateResourceInfo(const picojson::object& NetworkInfo) override {
			Base::ResponsePercentDataProcessor::UpdateVal(this->Transfer.Calc(NetworkInfo.at("write").get<double>()));
		}
		void ApplyViewParameter() {
			Base::ResponsePercentDataProcessor::ApplyViewParameter();
		}
	};
	Processor processor;
	Memory memory;
	DiskUsage diskUsed;
	//DiskRead diskRead;
	//DiskWrite diskWrite;
	//NetworkReceive netReceive;
	//NetworkSend netSend;
	static constexpr int GraphSpaceWidth = 10;
public:
	ResponseProcessingManager(StringManager& string) :
		processor(string, ".\\Graph\\Processor.png"),
		memory(string, ".\\Graph\\Memory.png"),
		diskUsed(string, ".\\Graph\\DiskUsed.png") {}
	void Draw() const {
		this->processor.Draw(0, 0);
		this->memory.Draw(GraphSpaceWidth + this->processor.GetRadius() * 2, 0);
		this->diskUsed.Draw((GraphSpaceWidth + this->processor.GetRadius() + this->memory.GetRadius()) * 2, 0);
	}
	void Update(const picojson::object& obj, const bool WriteErrorJson = true) {
		try {
			this->processor.Update(obj.at("cpu").get<picojson::object>());
			this->memory.Update(obj.at("memory").get<picojson::object>().at("physical").get<picojson::object>());
			this->diskUsed.Update(obj.at("disk").get<picojson::array>()[0].get<picojson::object>());
		}
		catch (const std::exception& er) {
			if (!WriteErrorJson) throw er;
			std::ofstream ofs("errorjson.log", std::ios::out | std::ios::app);
			ofs << picojson::value(obj) << std::endl;
		}
	}
	void ApplyViewParameter() {
		this->processor.ApplyViewParameter();
		this->memory.ApplyViewParameter();
		this->diskUsed.ApplyViewParameter();
	}
};
