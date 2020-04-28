#pragma once
#include <DxLib.h>

namespace SingleArg {
	typedef int (*DxLibCloseFunc)(int);

	template<DxLibCloseFunc Close>
	class DxLibHandle {
	private:
		int handle;
	public:
		DxLibHandle(const int Handle = 0) : handle(Handle) {}
		~DxLibHandle() {
			if (this->handle > 0) {
				Close(this->handle);
				this->handle = 0;
			}
		}
		DxLibHandle(const DxLibHandle&) = delete;
		DxLibHandle(DxLibHandle&& h) : handle(h.handle) { h.handle = 0; }
		DxLibHandle& operator = (const DxLibHandle&) = delete;
		DxLibHandle& operator = (DxLibHandle&& h) {
			this->handle = h.handle;
			h.handle = 0;
			return *this;
		}
		operator const int& () const noexcept { return this->handle; }
		operator int& () noexcept { return this->handle; }
	};
}

namespace DoubleArg {
	typedef int (*DxLibCloseFunc)(int, int);

	template<DxLibCloseFunc Close>
	class DxLibHandle {
	private:
		int handle;
	public:
		DxLibHandle(const int Handle = 0) : handle(Handle) {}
		~DxLibHandle() {
			if (this->handle > 0) {
				Close(this->handle, 0);
				this->handle = 0;
			}
		}
		DxLibHandle(const DxLibHandle&) = delete;
		DxLibHandle(DxLibHandle&& h) : handle(h.handle) { h.handle = 0; }
		DxLibHandle& operator = (const DxLibHandle&) = delete;
		DxLibHandle& operator = (DxLibHandle&& h) {
			this->handle = h.handle;
			h.handle = 0;
			return *this;
		}
		operator const int& () const noexcept { return this->handle; }
		operator int& () noexcept { return this->handle; }
	};
}
using GraphicHandle = DoubleArg::DxLibHandle<DxLib::DeleteGraph>;
using SoundHandle = DoubleArg::DxLibHandle<DxLib::DeleteSoundMem>;
using StringHandle = SingleArg::DxLibHandle<DxLib::DeleteFontToHandle>;
