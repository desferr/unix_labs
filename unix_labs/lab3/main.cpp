#include <iostream>
#include <filesystem>
#include <vector>
#include <openssl/sha.h>
#include <fstream>
#include <string>

using namespace std;

string get_hash(const filesystem::path& file_path) {
	ifstream file(file_path.string(), ios::binary);
	SHA_CTX context;
	SHA1_Init(&context);
	char buffer[4096];
	while (file.read(buffer, sizeof(buffer)) || file.gcount()) {
		SHA1_Update(&context, buffer, file.gcount());
	}
	unsigned char hash[SHA_DIGEST_LENGTH];
	SHA1_Final(hash, &context);
	string str;
	str.append(reinterpret_cast<const char*>(hash), sizeof(hash));
	return str;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "Передайте в виде аргумента путь до директории!" << endl;
		exit(1);
	}
	filesystem::path dir_path = argv[1];
	if (is_directory(dir_path) == 0) {
		cout << "Аргумент должен быть дерикторией!" << endl;
		exit(1);
	}
	vector<pair<filesystem::path, string>> vec_files;
	for (auto& entry : filesystem::recursive_directory_iterator(dir_path)) {
		if (filesystem::is_regular_file(entry) == 1) {
			pair<filesystem::path, string> temp_pair(entry.path(), get_hash(entry.path()));
			vec_files.push_back(temp_pair);
		}
	}
	for (int i = 0; i < vec_files.size(); i++) {
		filesystem::path first_file = vec_files[i].first;
		string first_hash = vec_files[i].second;
		for (int j = i + 1; j < vec_files.size(); j++) {
			filesystem::path second_file = vec_files[j].first;
			string second_hash = vec_files[j].second;
			if (first_hash == second_hash) {
				remove(second_file);
				create_hard_link(first_file, second_file);
				vec_files.erase(vec_files.begin() + j);
				j -= 1;
				continue;
			}
		}
	}
	return 0;
}
