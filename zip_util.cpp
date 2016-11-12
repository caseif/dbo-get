#include "zip_util.h"

#include <fstream>
#include <zip.h>

#include "util.h"

std::vector<std::string>* unzip(std::string file, std::string dest) {
    zip_file* file_in_zip;
    int ierr;
    int files_total;
    int r;
    char buffer[4096];

    makePath(dest);

    zip* zip_file = zip_open(file.c_str(), 0, &ierr);
    if (!zip_file) {
        err("Cannot read downloaded file " + file + ".");
        return NULL;
    }

    files_total = zip_get_num_files(zip_file);

    std::vector<std::string>* files = new std::vector<std::string>(files_total);
    for (zip_uint64_t i = 0; i < files_total; i++) {
        file_in_zip = zip_fopen_index(zip_file, i, 0);
        if (!file_in_zip) {
            err("Cannot open entry at index " + std::to_string(i) + " in zip.");
            continue;
        }
        std::string entryName = std::string(zip_get_name(zip_file, i, 0));
        (*files)[i] = entryName;
        print("    Extracting file " + entryName + "...");
        std::ofstream out(dest + "/" + entryName);
        while ((r = zip_fread(file_in_zip, buffer, sizeof(buffer))) > 0) {
            out.write(buffer, r);
        }
        zip_fclose(file_in_zip);
        out.close();
    }

    zip_close(zip_file);
    return files;
}
