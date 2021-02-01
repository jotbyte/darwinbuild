//
//  stdlib.c
//  darwintrace
//
//  Created by John Othwolo on 1/30/21.
//  Copyright © 2021 The DarwinBuild Project. All rights reserved.
//

#include <cstdio>
#include <cerrno>
#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <experimental/filesystem>
#include <regex>

#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "darwinbuild.h"

namespace fs = std::filesystem;

int checkDir(std::string dir){
    fs::path dpath(dir);

    if (fs::exists(dpath)){
        if (fs::is_directory(dpath)) {
            return 0;
        } else {
            std::cerr << dir << " is not a directory\n" << std::endl;
            return ENOENT;
        }
    } else {
        std::cerr << "Directory \'" << dir << "\' doesn't exist" << strerror(errno) << std::endl;
        return errno;
    }
}

int checkFile(std::string file){
    fs::path fpath(file);

    if (fs::exists(fpath)){
        if (fs::is_regular_file(fpath)) {
            return 0;
        } else {
            std::cerr << "The file '" << file << "' is not supported\n" << std::endl;
            return ENOENT;
        }
    } else {
        std::cerr << "unable to open file \'" << file << "\' " << strerror(errno) << std::endl;
        return errno;
    }
}

int readFile(std::string path, char *out){
    std::ifstream file;
    size_t len;
    char *data;
    int ret;
    
    if((ret = checkFile(path)) != 0)
        return -1;
    
    file.open(path, std::ios::in | std::ios::ate);
    
    len = file.tellg();
    file.seekg (0, std::ios::beg);
    len = (len - file.tellg());
    
    data = new char[1000];
    file.read(data, 1000);
    file.close();
    
    data[len] = 0;
    if (data[len-1] == '\n')
        data[len-1] = 0;
    
    out = data;
    return (int)len;
}

int copyfile(std::string src, std::string dest, bool replace){
//    fs::copy_options opts;
//    opts
//    fs::copy(<#const path &__from#>, <#const path &__to#>, <#copy_options __opt#>, <#error_code &__ec#>)
    return 0;
}

bool isURL(std::string str){
    std::string pt = "https?:\\/\\/(www\\.)?[-a-zA-Z0-9@:%._\\+~#=]{2,256}\\.[a-z]{2,4}\b([-a-zA-Z0-9@:%_\\+.~#?&//=]*)";
    if (std::regex_match (str, std::regex(pt) )) {
        return true;
    } else {
        return false;
    }
}

bool isSSH(std::string str){
    std::string pt = "/^[A-Za-z][A-Za-z0-9_]*\\@[A-Za-z][A-Za-z0-9_\\.]*\\:(\\/[A-Za-z][A-Za-z0-9_]*)*$/";
    if (std::regex_match (str, std::regex(pt) )) {
        return true;
    } else {
        return false;
    }
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream){
    std::ofstream *fstream = static_cast<std::ofstream*>(stream);
    fstream->write((char*)ptr, size*nmemb);
    return size*nmemb;
}

int DownloadFile(std::string url, std::string filename){
    CURL *curl;
    std::ofstream file;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        file.open(filename);
        file.seekp(0, std::ios::beg);
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (&file));
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        file.close();
    }
    return 0;
}

int newDir(std::string dir){
    
    fs::create_directory(<#const path &__p#>, <#const path &__attrs#>, <#error_code &__ec#>)
    return 0;
}
