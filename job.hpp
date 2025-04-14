#pragma once

#include <functional>
#include <string>
#include <chrono>

namespace benjamin {
	template<typename T>
	struct job {
		std::string label;
		std::function<T()> func;
		int execution_time = 0;
		inline job(std::string label, std::function<T()> f) : label(label), func(f) {}
	};
}
