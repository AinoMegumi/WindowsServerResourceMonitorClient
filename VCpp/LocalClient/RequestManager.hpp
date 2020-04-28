#pragma once
#include "../Server/httplib.h"
#include "StringEncode.hpp"
#include <picojson/picojson.h>
#include <chrono>

class RequestManager {
protected:
	httplib::Client client;
	long long RequestInterval;
	std::chrono::milliseconds LastRequest;
	int ErrorCount;
	int MaxErrorCount;
	int LastStatus;
	static constexpr std::chrono::milliseconds GetCurrentClock() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	}
public:
	RequestManager(const std::string& Host, const int port, const long long Interval = 1000, const int ErrorMax = 5)
		: client(Host, port), RequestInterval(Interval), LastRequest(GetCurrentClock()), ErrorCount(), MaxErrorCount(ErrorMax), LastStatus(200) {}
	int GetAll(picojson::object& obj, const std::string& Path) {
		const std::chrono::microseconds Now = GetCurrentClock();
		if (const long long time = (Now - this->LastRequest).count(); time < this->RequestInterval) {
			return 1;
		}
		auto res = this->client.Get(Path.c_str());
		if (res == nullptr) return 1;
		this->LastStatus = res->status;
		if (res->status != 200) {
			if (this->LastStatus == 503) return 1; // 503はサービスが一時停止中にも来るのでエラーカウントしない
			this->ErrorCount++;
			if (this->MaxErrorCount == this->ErrorCount) 
				throw std::runtime_error("複数回にわたってエラーが発生しました。サーバーを確認して下さい。");
			return -1;
		}
		this->ErrorCount = 0;
		picojson::value val{};
		if (const std::string err = picojson::parse(val, UTF8_To_ShiftJIS(res->body)); !err.empty()) throw std::runtime_error(err);
		obj = val.get<picojson::object>();
		this->LastRequest = GetCurrentClock();
		return 0;
	}
	void Post(const std::string& Path, const std::string& Body = std::string(), const std::string& ContentType = std::string()) {
		this->client.Post(Path.c_str(), Body, ContentType.c_str());
	}
};
