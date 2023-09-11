//@	{
//@		"target":{"name": "application_log.o"},
//@		"compiler":{
//@			"config": {
//@				"cflags":[
//	NOTE: We need to change some compiler flags for Qt
//@					"-Wno-error=conversion",
//@					"-Wno-error=deprecated-enum-enum-conversion",
//@					"-fpic"
//@				]
//@			}
//@		}
//@	}

#include "./application_log.hpp"

size_t terraformer::write(application_log writer, std::span<std::byte const> buffer)
{
	writer.app.get().post_event([&console = writer.console.get(), buffer](){
		std::string str{};
		str.reserve(std::size(buffer));
		std::ranges::transform(buffer, std::back_inserter(str), [](auto src){
			return static_cast<char>(src);
		});
		console.moveCursor(QTextCursor::End);
		console.insertPlainText(QString::fromStdString(str));
		console.moveCursor(QTextCursor::End);
		return true;
	});
	return std::size(buffer);
}