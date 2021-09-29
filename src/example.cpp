#include <iostream>
#include <filesystem> // path, directory_iterator
#include <fstream> // ifstream

#include "mister_mime.hpp"

namespace fs = std::filesystem;

using std::cout, std::endl;

using
	MrMime::FileType,
	MrMime::header_data_buffer_t,
	MrMime::compareSignaturesTo,
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

				const FileType filetype{ compareSignaturesTo(buffer) };

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

constexpr char size_byte_arrays[]{
	"\xff\xd8"
	"GIF87a"
	"GIF89a"
	"\x89PNG" "IHDR" "acTL"
	"\x89PNG"
	"WEBP"
	"II*\x00'"
	"MM\x00*"
	"BM"
	"\x00\x00\x01\x00"
	"\x00\x00\x02\x00"
	"CWS"
	"FWS"
	"ZWS"
	"FLV"
	"%PDF"
	"8BPS\x00\x01"
	"8BPS\x00\x02"
	"CSFCHUNK"
	"PK\x03\x04"
	"PK\x05\x06"
	"PK\x07\x08"
	"7z\xBC\xAF\x27\x1C"
	"\x52\x61\x72\x21\x1A\x07\x00"
	"\x52\x61\x72\x21\x1A\x07\x01\x00"
	"hydrus encrypted zip"
	"ftypmp4"
	"ftypisom"
	"ftypM4V"
	"ftypMSNV"
	"ftypavc1"
	"ftypFACE"
	"ftypdash"
	"ftypqt"
	"fLaC"
	"RIFF"
	"WAVE"
	"AVI"
	"\x30\x26\xB2\x75\x8E\x66\xCF\x11" "\xA6\xD9\x00\xAA\x00\x62\xCE\x6C"
};

constexpr std::size_t size_skip_bytes{ 8+8+8+21+4+4+4+4+4+4+4+4+8 };

constexpr std::size_t raw_size{ sizeof(size_byte_arrays) + size_skip_bytes };

int main()
{
	example_scan_directory("test_dump");

	using
		MrMime::internal::Signatures_t,
		MrMime::internal::signature_count,
		MrMime::internal::size_of_largest_signature;

	cout
		<< "\n\t" << signature_count << " signatures defined, "
		<< " consuming " << sizeof(Signatures_t) << " bytes "
		<< "(raw: " << raw_size << " bytes)\n"
		<< "\tThe largest signature (and the file readbuffer) is "
		<< size_of_largest_signature << " bytes\n"
		<< endl;

	return 0;
}
