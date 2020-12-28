#ifndef BIOGB_CUSTOM_EXCEPTIONS_H
#define BIOGB_CUSTOM_EXCEPTIONS_H

#include <stdexcept>

class BioGBError : public std::runtime_error {
public:
    BioGBError(const std::string &string) : runtime_error(string) {}
};

class SoundSystemError : public BioGBError {
public:
    SoundSystemError(const std::string &string) : BioGBError(string) {}
};


#endif  // BIOGB_CUSTOM_EXCEPTIONS_H
