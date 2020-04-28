#pragma once
#include "../Common/CommandLineManager.h"

namespace CommandLineManager = CommandLineManagerA;
#ifdef UNICODE
namespace CharsetManager = CommandLineManagerW;
#else
namespace CharsetManager = CommandLineManagerA;
#endif
