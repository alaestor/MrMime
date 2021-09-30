#include <iostream>
#include <filesystem> // path, directory_iterator
#include <fstream> // ifstream

#include "mister_mime.hpp"

namespace fs = std::filesystem;

using std::cout, std::endl;

using
	MrMime::FileType,
	MrMime::header_data_buffer_t,
	MrMime::deduceFileType,
	MrMime::hydrus_compatible_filetype;

void example_scan_directory(const fs::path folder_path)
{
	header_data_buffer_t buffer{};
	char* const buffer_ptr{ reinterpret_cast<char*>(buffer.data()) };

	try
	{
		for (const auto& dir_entry : fs::directory_iterator(folder_path))
		{
			if (dir_entry.is_regular_file())
			{
				const auto filepath{ dir_entry.path() };

				auto ifs{ std::ifstream(filepath, std::ios::binary) };

				// could use exceptions instead:
				//ifs.exceptions(ifs.badbit | ifs.failbit | ifs.eofbit);

				if (!ifs.is_open()) [[unlikely]]
				{
					std::cerr << "failed to open file: " << filepath << endl;
					continue;
				}

				if (!ifs.seekg(ifs.beg)) [[unlikely]]
				{
					std::cerr << "failed to seekg file: " << filepath << endl;
					continue;
				}

				if (!ifs.read(buffer_ptr, buffer.size())) [[unlikely]]
				{
					std::cerr << "failed to read file: " << filepath << endl;
					continue;
				}

				const FileType filetype{ deduceFileType(buffer) };

				cout
					<< fileType_to_string(filetype)
					<< " (" << hydrus_compatible_filetype(filetype) << "): "
					<< filepath.filename() << endl;
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << endl;
	}
}

int main()
{
	example_scan_directory("test_dump");

	// bellow here is just a print-out of some internal Mr.Mime statistics

	using
		MrMime::internal::signatures,
		MrMime::internal::Signatures_t,
		MrMime::internal::signature_count,
		MrMime::internal::size_of_largest_signature;

	constexpr std::size_t size_including_skipped_bytes{
		std::apply(
			[]<typename ... SIGS>(const SIGS& ... sigs)
				constexpr -> std::size_t { return (sigs.size() + ...); },
			signatures
		)
	};

	cout
		<< "\n\t" << signature_count << " signatures defined, "
		<< " consuming " << sizeof(Signatures_t) << " bytes "
		<< "(matches to " << size_including_skipped_bytes << " bytes)\n"
		<< "\tThe largest signature (and the header_data_buffer_t) is "
		<< size_of_largest_signature << " bytes\n"
		<< endl;

	return 0;
}
