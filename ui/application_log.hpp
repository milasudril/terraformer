//@	{
//@		"dependencies":[{"ref":"Qt5Widgets", "origin":"pkg-config"}],
//@		"dependencies_extra":[{"ref":"./application_log.o","rel":"implementation"}]
//@	}

#ifndef TERRAFORMER_APPLICATION_LOG_HPP
#define TERRAFORMER_APPLICATION_LOG_HPP

#include "./application.hpp"

#include <span>

#include <QTextEdit>

namespace terraformer
{
	struct application_log
	{
		std::reference_wrapper<terraformer::application> app;
		std::reference_wrapper<QTextEdit> console;
	};

	size_t write(application_log writer, std::span<std::byte const> buffer);
}

#endif