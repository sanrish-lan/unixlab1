#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <openssl/sha.h>

using namespace std;

string compute_sha1_hash(const filesystem::path& file_path)
{
    ifstream file_stream(file_path, ios::binary);
    if (!file_stream.is_open())
    {
        throw runtime_error("Failed to open file");
    }

    SHA_CTX sha1_ctx;
    SHA1_Init(&sha1_ctx);

    char data_buffer[4096];
    while (file_stream.good())
    {
        file_stream.read(data_buffer, sizeof(data_buffer));
        int bytes_read = file_stream.gcount();
        if (bytes_read > 0)
        {
            SHA1_Update(&sha1_ctx, data_buffer, bytes_read);
        }
    }

    unsigned char hash_digest[SHA_DIGEST_LENGTH];
    SHA1_Final(hash_digest, &sha1_ctx);

    stringstream hex_stream;
    hex_stream << hex << setfill('0');
    for (int index = 0; index < SHA_DIGEST_LENGTH; index++)
    {
        hex_stream << setw(2) << static_cast<int>(hash_digest[index]);
    }

    return hex_stream.str();
}

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " <directory>" << endl;
        return 1;
    }

    filesystem::path root_directory = argv[1];

    if (!filesystem::exists(root_directory) || !filesystem::is_directory(root_directory))
    {
        cerr << "Error: The specified path does not exist or is not a directory." << endl;
        return 1;
    }

    unordered_map<string, filesystem::path> hash_list;

    int total_files_processed = 0;
    int duplicate_replacements = 0;

    cout << "Scanning directory: " << root_directory << endl;

    for (auto& dir_entry : filesystem::recursive_directory_iterator(root_directory))
    {
        if (!dir_entry.is_regular_file() || dir_entry.is_symlink())
        {
            continue;
        }

        filesystem::path current_file = dir_entry.path();
        total_files_processed++;

        try
        {
            string file_hash = compute_sha1_hash(current_file);
            auto hash_match = hash_list.find(file_hash);

            if (hash_match == hash_list.end())
            {
                hash_list[file_hash] = current_file;
            }
            else
            {
                filesystem::path original_file = hash_match->second;
                if (filesystem::equivalent(current_file, original_file))
                {
                    continue;
                }

                cout << "Duplicate " << current_file.filename() << " == " << original_file.filename() << endl;
                filesystem::remove(current_file);
                filesystem::create_hard_link(original_file, current_file);

                duplicate_replacements++;
            }
        }
        catch (const exception& exc)
        {
            cerr << "Error processing file " << current_file << ": " << exc.what() << endl;
        }
    }

    cout << "\nTotal files processed: " << total_files_processed << endl;
    cout << "Duplicates replaced: " << duplicate_replacements << endl;

    return 0;
}