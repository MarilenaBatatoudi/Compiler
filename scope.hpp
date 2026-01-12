#ifndef SCOPE_HPP
#define SCOPE_HPP

#include <map>
#include <memory>
#include <string>
#include "data_type.hpp"

enum class SymbolKind {
    Variable,
    Constant,
    Function
};

class SymbolInfo {
 public:
    std::string name;
    SymbolKind kind;
    DataType type;
    std::vector<DataType> paramTypes;
    
    SymbolInfo(const std::string& n, SymbolKind k, DataType t) 
        : name(n), kind(k), type(t) {}
};

// scope class to manage symbol tables
class Scope {
 private:
    std::map<std::string, std::unique_ptr<SymbolInfo>> symbolTable;
    
 public:
    std::shared_ptr<Scope> parent;
    
    explicit Scope(std::shared_ptr<Scope> p = nullptr) : parent(p) {}
    
    // adding a symbol to this scope
    void addSymbol(const std::string& name, SymbolKind kind, DataType type) {
        symbolTable[name] = std::make_unique<SymbolInfo>(name, kind, type);
    }
    
    // adding a function symbol with parameter types
    void addFunction(const std::string& name, DataType returnType, const std::vector<DataType>& params) {
        auto symbol = std::make_unique<SymbolInfo>(name, SymbolKind::Function, returnType);
        symbol->paramTypes = params;
        symbolTable[name] = std::move(symbol);
    }
    
    // Check if symbol exists in current scope only
    bool existsInCurrentScope(const std::string& name) const {
        return symbolTable.find(name) != symbolTable.end();
    }
    
    // Look up symbol in current scope and parent scopes
    SymbolInfo* lookup(const std::string& name) {
        auto it = symbolTable.find(name);
        if (it != symbolTable.end()) {
            return it->second.get();
        }
        if (parent) {
            return parent->lookup(name);
        }
        return nullptr;
    }
};

#endif // SCOPE_HPP