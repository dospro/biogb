//
// Created by Ruben Gutierrez on 2019-04-02.
//

#ifndef BIOGB_EXCEPTIONS_H
#define BIOGB_EXCEPTIONS_H


#include <stdexcept>

class BioGBError : public std::runtime_error {
public:
    BioGBError(const std::string &string) : runtime_error(string) {}
};

class SoundSystemError : public BioGBError {
public:
    SoundSystemError(const std::string &string) : BioGBError(string) {}
};


#endif //BIOGB_EXCEPTIONS_H
