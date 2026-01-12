#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include "data_type.hpp"

class LexerException : public std::runtime_error {
 public:
    LexerException(int line, int col) : std::runtime_error(
        " at line " +
        std::to_string(line) +
        ", column " + std::to_string(col)) {}
};

class ParserException : public std::runtime_error {
 public:
    ParserException(int line, int col) : std::runtime_error(
        " at line " +
        std::to_string(line) +
        ", column " + std::to_string(col)) {}
};

struct SemanticErrorContext {
    std::string identifier;
    DataType expectedType;
    DataType actualType;
    std::string functionName;
    std::vector<DataType> signatureExpected;
    std::vector<DataType> signatureActual;
    int expectedArgs;
    int actualArgs;
    std::string op;

    static SemanticErrorContext Identifier(const std::string& id) {
        SemanticErrorContext c;
        c.identifier = id;
        return c;
    }

    static SemanticErrorContext ActualType(const DataType& got) {
        SemanticErrorContext c;
        c.actualType = got;
        return c;
    }

    static SemanticErrorContext InvalidOperationBetweenTypes(
        const std::string& op, const DataType& t1, const DataType& t2) {
        SemanticErrorContext c;
        c.op = op;
        c.expectedType = t1;
        c.actualType = t2;
        return c;
    }

    static SemanticErrorContext IdentifierTypeMismatch(const std::string& id,
                                    const DataType& exp, const DataType& got) {
        SemanticErrorContext c;
        c.identifier = id;
        c.expectedType = exp;
        c.actualType = got;
        return c;
    }

    static SemanticErrorContext ReturnTypeMismatch(const std::string& funcName,
                                               const DataType& exp,
                                               const DataType& got) {
        SemanticErrorContext c;
        c.functionName = funcName;
        c.expectedType = exp;
        c.actualType = got;
        return c;
    }

    static SemanticErrorContext Function(const std::string& name) {
        SemanticErrorContext c;
        c.functionName = name;
        return c;
    }

    static SemanticErrorContext Signature(const std::string& name,
                                        const std::vector<DataType>& exp,
                                        const std::vector<DataType>& got) {
        SemanticErrorContext c;
        c.functionName = name;
        c.signatureExpected = exp;
        c.signatureActual = got;
        return c;
    }

    static SemanticErrorContext ArgCount(const std::string& name,
                                                int exp, int got) {
        SemanticErrorContext c;
        c.functionName = name;
        c.expectedArgs = exp;
        c.actualArgs = got;
        return c;
    }
};


enum class SemanticErrorType {
    IOTA = 0,
    REDECLARED_IDENTIFIER = 1,
    UNDECLARED_IDENTIFIER = 2,
    VAR_DECL_TYPE_MISMATCH = 3,
    VAR_ASSIGN_TYPE_MISMATCH = 4,
    VAR_ASSIGN_TO_CONSTANT = 5,
    UNDECLARED_FUNCTION = 6,
    REDECLARED_FUNCTION = 7,
    NOT_A_FUNCTION = 8,
    INVALID_SIGNATURE = 9,
    RETURN_TYPE_MISMATCH = 10,
    RETURN_OUTSIDE_FUNCTION = 11,
    CONDITION_NOT_BOOL = 12,
    INVALID_UNARY_OPERATION = 13,
    INVALID_BINARY_OPERATION = 14,
    FUNCTION_USED_AS_VARIABLE = 15,
    WRONG_NUMBER_OF_ARGUMENTS = 16,
    UNREACHABLE_CODE = 17,
    DEAD_CODE = 18,
    MISSING_RETURN = 19,
    INFINITE_LOOP_DETECTED = 20,
};

class SemanticException : public std::runtime_error {
 public:
    SemanticErrorType errorType;
    SemanticErrorContext context;

    SemanticException(SemanticErrorType type,
                        const SemanticErrorContext& ctx = {})
        : std::runtime_error(buildErrorMessage(type, ctx)),
          errorType(type), context(ctx) {}

 private:
    static std::string buildErrorMessage(SemanticErrorType type,
                                const SemanticErrorContext& ctx) {
        std::ostringstream oss;
        switch (type) {
            case SemanticErrorType::REDECLARED_IDENTIFIER:
                oss << "Redeclaration of identifier '" << ctx.identifier << "'";
                break;
            case SemanticErrorType::REDECLARED_FUNCTION:
                oss << "Redeclaration of function '" << ctx.functionName << "'";
                break;
            case SemanticErrorType::UNDECLARED_IDENTIFIER:
                oss << "Use of undeclared identifier '"
                << ctx.identifier << "'";
                break;
            case SemanticErrorType::UNDECLARED_FUNCTION:
                oss << "Call to undeclared function '"
                << ctx.functionName << "'";
                break;
            case SemanticErrorType::NOT_A_FUNCTION:
                oss << "Identifier '" << ctx.identifier
                << "' is not a function";
                break;
            case SemanticErrorType::VAR_DECL_TYPE_MISMATCH:
                oss << "Type mismatch during variable declaration for '"
                << ctx.identifier << "': expected '" << ctx.expectedType
                << "', got '" << ctx.actualType << "'";
                break;
            case SemanticErrorType::VAR_ASSIGN_TYPE_MISMATCH:
                oss << "Type mismatch during variable assignment for '"
                << ctx.identifier << "': expected '" << ctx.expectedType
                << "', got '" << ctx.actualType << "'";
                break;
            case SemanticErrorType::VAR_ASSIGN_TO_CONSTANT:
                oss << "Attempt to assign to constant variable '"
                << ctx.identifier << "'";
                break;
            case SemanticErrorType::RETURN_TYPE_MISMATCH:
                oss << "Return type mismatch for function '" << ctx.functionName
                << "': expected '" << ctx.expectedType << "', got '"
                << ctx.actualType << "'";
                break;
            case SemanticErrorType::WRONG_NUMBER_OF_ARGUMENTS:
                oss << "Wrong number of arguments in call to function '"
                << ctx.functionName << "': expected " << ctx.expectedArgs
                << ", got " << ctx.actualArgs;
                break;
            case SemanticErrorType::INVALID_SIGNATURE:
                oss << "Invalid signature for function '" << ctx.functionName
                    << "' — expected (";
                for (size_t i = 0; i < ctx.signatureExpected.size(); ++i) {
                    oss << ctx.signatureExpected[i];
                    if (i != ctx.signatureExpected.size() - 1) oss << ", ";
                }
                oss << "), got (";
                for (size_t i = 0; i < ctx.signatureActual.size(); ++i) {
                    oss << ctx.signatureActual[i];
                    if (i != ctx.signatureActual.size() - 1) oss << ", ";
                }
                oss << ")";
                break;
            case SemanticErrorType::RETURN_OUTSIDE_FUNCTION:
                oss << "Return statement used outside of a function";
                break;
            case SemanticErrorType::CONDITION_NOT_BOOL:
                oss << "Condition expression does not evaluate to bool";
                break;
            case SemanticErrorType::INVALID_UNARY_OPERATION:
                oss << "Invalid unary operation on type '"
                << ctx.actualType << "'";
                break;
            case SemanticErrorType::INVALID_BINARY_OPERATION:
                oss << "Invalid binary operation '" << ctx.op << "'"
                    << " between types '" << ctx.expectedType << "' and '"
                    << ctx.actualType << "'";
                break;
            case SemanticErrorType::FUNCTION_USED_AS_VARIABLE:
                oss << "Function '" << ctx.functionName
                << "' used as a variable";
                break;
            case SemanticErrorType::UNREACHABLE_CODE:
                oss << "Unreachable code detected";
                break;
            case SemanticErrorType::DEAD_CODE:
                oss << "Dead code detected";
                break;
            case SemanticErrorType::MISSING_RETURN:
                oss << "Missing return statement in function '"
                << ctx.functionName << "'";
                break;
            case SemanticErrorType::INFINITE_LOOP_DETECTED:
                oss << "Infinite loop detected in function '"
                << ctx.functionName << "'";
                break;
            default:
                oss << "Unknown semantic error";
                break;
        }
        return oss.str();
    }
};

#endif /* EXCEPTION_HPP */
