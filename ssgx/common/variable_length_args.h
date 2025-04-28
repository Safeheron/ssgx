#ifndef SAFEHERON_SGX_LIBRARY_VARIABLE_LENGTH_ARGUMENTS_H
#define SAFEHERON_SGX_LIBRARY_VARIABLE_LENGTH_ARGUMENTS_H

#include <vector>

template <typename T>
std::vector<std::string> MakeArgs(T first) {
    std::vector<std::string> vec;
    vec.reserve(6);
    vec.push_back(first);
    return vec;
}

template <typename T, typename... Types>
std::vector<std::string> MakeArgs(T first, Types... args) {
    std::vector<std::string> vec = std::move(MakeArgs(args...));
    vec.push_back(first);
    return vec;
}

#endif // SAFEHERON_SGX_LIBRARY_VARIABLE_LENGTH_ARGUMENTS_H
