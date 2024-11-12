
#include <string>
#include <sstream>

namespace utils {
	template <typename T>
	bool setIfHasParams(int argc, char** argv, const char* name, T* param)
	{
		int i = 0;
		while (i < argc)
		{
			if (strcmp(argv[i], name) == 0 && i + 1 < argc)
			{
				std::stringstream stream{ std::string{argv[i + 1]} };
				stream >> (*param);
				return true;
			}
			++i;
		}
		return false;
	}
}