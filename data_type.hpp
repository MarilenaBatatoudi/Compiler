#ifndef DATA_TYPE_HPP
#define DATA_TYPE_HPP

#include <iostream>
#include <string>

enum class DataType {
    IOTA = 0,
    INT,
    FLOAT,
    BOOL,
};

inline std::ostream& operator<<(std::ostream& os, const DataType& dt) {
    switch (dt) {
        case DataType::INT:
            return os << "int";
        case DataType::FLOAT:
            return os << "float";
        case DataType::BOOL:
            return os << "bool";
        default:
            return os << "iota";
    }
}

inline DataType stringToDataType(const std::string& typeStr) {
    if (typeStr == "int") return DataType::INT;
    if (typeStr == "float") return DataType::FLOAT;
    if (typeStr == "bool") return DataType::BOOL;
    return DataType::IOTA;
}

#endif // DATA_TYPE_HPP
