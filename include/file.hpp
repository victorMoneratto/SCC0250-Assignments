#pragma once

#include <common.hpp>

#include <fstream>
#include <sstream>

inline std::string ReadFile(std::string FileName) {

	std::ifstream File{ FileName };
	Assert(File.is_open());

	std::string Result;

#if MSVC
	struct stat Stat;
	if (stat(FileName.c_str(), &Stat) != -1) {
		Result.reserve(Stat.st_size);
	}

	char Buffer[1024];
	while (File.get(Buffer, SizeOf(Buffer), EOF)) {
		Result.append(Buffer);
	}
#else
	std::stringstream Buffer;
	Buffer << File.rdbuf();
	Result = Buffer.str();
#endif

	return Result;
}
