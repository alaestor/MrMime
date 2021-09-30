#include <iostream>
#include <filesystem> // path, directory_iterator
#include <fstream> // ifstream

#include "mister_mime.hpp"

using
	MrMime::FileType,
	MrMime::header_data_buffer_t,
	MrMime::deduceFileType,
	MrMime::hydrus_compatible_filetype;

void example_scan_directory(const std::filesystem::path folder_path)
{
	namespace fs = std::filesystem;

	const auto read_fileheader_to_buffer{
		[](const fs::path& filepath, header_data_buffer_t& buffer)
		{
			auto ifs{ std::ifstream(filepath, std::ios::binary) };

			// could use exceptions instead:
			//ifs.exceptions(ifs.badbit | ifs.failbit | ifs.eofbit);

			if (!ifs.is_open()) [[unlikely]]
			{
				std::cerr << "failed to open file: " << filepath << std::endl;
				return false;
			}

			if (!ifs.seekg(ifs.beg)) [[unlikely]]
			{
				std::cerr << "failed to seekg file: " << filepath << std::endl;
				return false;
			}

			if (!ifs.read(
					reinterpret_cast<char*>(buffer.data()),
					static_cast<std::streamsize>(buffer.size()))) [[unlikely]]
			{
				std::cerr << "failed to read file: " << filepath << std::endl;
				return false;
			}

			return true;
		}
	};

	for (header_data_buffer_t buffer{};
		const auto& dir_entry : fs::directory_iterator(folder_path))
	{
		if (dir_entry.is_regular_file())
		{
			if (const auto filepath{ dir_entry.path() };
				read_fileheader_to_buffer(filepath, buffer)) [[likely]]
			{
				const FileType filetype{ deduceFileType(buffer) };

				std::cout
					<< fileType_to_string(filetype)
					<< " (" << hydrus_compatible_filetype(filetype) << "): "
					<< filepath.filename()
					<< std::endl;
			}
		}
	}
}

int main()
{
	try
	{
		example_scan_directory("test_dump");
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

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

	std::cout
		<< "\n\t" << signature_count << " signatures defined, "
		<< " consuming " << sizeof(Signatures_t) << " bytes "
		<< "(matches to " << size_including_skipped_bytes << " bytes)\n"
		<< "\tThe largest signature (and the header_data_buffer_t) is "
		<< size_of_largest_signature << " bytes\n"
		<< std::endl;

	return 0;
}
