#include "RequestManager.hpp"
#include "ResponseProcessingManager.hpp"
#include <thread>
#include <mutex>
std::mutex mutex;
picojson::object res;
bool Updated = false;

namespace Config {
	constexpr const TCHAR* WindowTitle = _T("リソースマネージャー");
	constexpr int WindowWidth = 1280;
	constexpr int WindowHeight = 720;
	constexpr int StringSize = 16;
}

inline void InitDxLib() {
	if (-1 == DxLib::SetMultiThreadFlag(TRUE)) throw std::runtime_error("Error in SetMultiThreadFlag function");
	if (-1 == DxLib::SetMainWindowText(Config::WindowTitle)) throw std::runtime_error("Error in SetMainWindowText function");
	if (-1 == DxLib::SetOutApplicationLogValidFlag(FALSE)) throw std::runtime_error("Error in SetMainWindowText function");
	if (-1 == DxLib::SetDoubleStartValidFlag(TRUE)) throw std::runtime_error("Error in DoubleStartValidFlag function");
	if (-1 == DxLib::SetGraphMode(Config::WindowWidth, Config::WindowHeight, 32)) throw std::runtime_error("Error in SetGraphMode function");
	if (-1 == DxLib::ChangeWindowMode(true)) throw std::runtime_error("Error in ChangeWindowMode function");
	if (-1 == DxLib::SetBackgroundColor(255, 255, 255)) throw std::runtime_error("Error in SetBackgroundColor function");
	if (-1 == DxLib::SetAlwaysRunFlag(TRUE)) throw std::runtime_error("Error in SetAlwaysRunFlag function");
	if (-1 == DxLib::DxLib_Init()) throw std::runtime_error("Failed to initialize.");
	if (-1 == DxLib::SetTransColor(255, 255, 255)) throw std::runtime_error("Error in SetTransColor function");
	if (-1 == DxLib::SetDrawScreen(DX_SCREEN_BACK)) throw std::runtime_error("Error in SetDrawScreen function");
}

void GetResourceInformation(std::exception_ptr& eptr) {
	try {
		StringManager string = StringManager("Font", Config::StringSize, Color("#000000"));
		ResponseProcessingManager resmgr(string);
		auto valid = [&resmgr](const picojson::object& obj) {
			try {
				resmgr.Update(obj, false);
				return true;
			}
			catch (std::exception) {
				return false;
			}
		};
		RequestManager request("localhost", 32768, 1000, 100);
		picojson::object resVal{};
		while (1) {
			if (const int Result = request.GetAll(resVal, "/v1/"); Result == 0) {
				if (!valid(resVal)) continue;
				std::lock_guard<std::mutex> lock(mutex);
				res = std::move(resVal);
				Updated = true;
			}
			if (ProcessMessage() == -1) break;
		}
	}
	catch (...) {
		eptr = std::current_exception();
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	try {
		InitDxLib();
		StringManager string = StringManager("Font", Config::StringSize, Color("#000000"));
		ResponseProcessingManager resmgr(string);
		std::exception_ptr eptr{};
		std::thread th(GetResourceInformation, std::ref(eptr));
		th.detach();
		while (res.size() == 0 && ProcessMessage() != -1) {}
		size_t arrSize = 0;
		while (ProcessMessage() != -1) {
			if (eptr) std::rethrow_exception(eptr);
			ClearDrawScreen();
			resmgr.Draw();
			ScreenFlip();
			resmgr.ApplyViewParameter();
			arrSize = res.size();
			if (arrSize != 0 && Updated) {
				std::lock_guard<std::mutex> lock(mutex);
				Updated = false;
				resmgr.Update(res);
			}
		}
	}
	catch (const std::exception& er) {
		MessageBoxA(NULL, er.what(), "エラー", MB_ICONERROR | MB_OK);
	}
	DxLib_End();
	return 0;
}

