#pragma once

#include <string>
#include <fmt/format.h>

namespace EQEmu
{
	enum LogLevel
	{
		LogCritical,
		LogDebug,
		LogError,
		LogInfo,
		LogTrace,
		LogWarning,
		LogMax
	};

	class ILogger
	{
	public:
		ILogger() { }
		virtual ~ILogger() { }

		virtual void Enable(LogLevel level) = 0;
		virtual void Disable(LogLevel level) = 0;
		virtual bool IsEnabled(LogLevel level) = 0;
		virtual void RawLog(LogLevel level, const std::string &msg) = 0;

		template <typename... Args>
		void Log(LogLevel level, const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(level, log_str);
		}

		template <typename... Args>
		void LogCritical(const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(LogLevel::LogCritical, log_str);
		}

		template <typename... Args>
		void LogDebug(const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(LogLevel::LogDebug, log_str);
		}

		template <typename... Args>
		void LogError(const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(LogLevel::LogError, log_str);
		}

		template <typename... Args>
		void LogInfo(const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(LogLevel::LogInfo, log_str);
		}

		template <typename... Args>
		void LogTrace(const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(LogLevel::LogTrace, log_str);
		}

		template <typename... Args>
		void LogWarning(const std::string &str, const Args&... args) {
			std::string log_str = fmt::format(str, args...);
			RawLog(LogLevel::LogWarning, log_str);
		}
	};
}
