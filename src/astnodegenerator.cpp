#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <optional>
#include "token.hpp"

// Utility function to trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    return (first == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

void defineType(std::ofstream& outputFile, const std::string& baseName, const std::string& className, const std::string& fields) {
    // Parse fields
    std::vector<std::pair<std::string, std::string>> fieldList;
    std::istringstream iss(fields);
    std::string field;

    while (std::getline(iss, field, ',')) {
        std::istringstream fieldIss(trim(field));
        std::string type, name;
        fieldIss >> type >> name;
        fieldList.emplace_back(type, name);
    }

    // Class definition
    outputFile << "class " << className << " : public " << baseName << " {\n";
    outputFile << "public:\n";

    // Constructor
    outputFile << "    " << className << "(";
    for (size_t i = 0; i < fieldList.size(); ++i) {
        const auto& [type, name] = fieldList[i];
        if (type == "Expr")
            outputFile << "std::shared_ptr<" << baseName << "> " << name;
        else
            outputFile << type << " " << name;

        if (i + 1 < fieldList.size()) outputFile << ", ";
    }
    outputFile << ") : ";
    for (size_t i = 0; i < fieldList.size(); ++i) {
        const auto& [_, name] = fieldList[i];
        outputFile << name << "(" << name << ")";
        if (i + 1 < fieldList.size()) outputFile << ", ";
    }
    outputFile << " {}\n";

    // accept() method
    outputFile << "    void accept(const ExprVisitorBase& visitor) const override { visitor.visit(*this); }\n";

    // Fields
    for (const auto& [type, name] : fieldList) {
        if (type == "Expr")
            outputFile << "    std::shared_ptr<" << baseName << "> " << name << ";\n";
        else
            outputFile << "    " << type << " " << name << ";\n";
    }

    outputFile << "};\n\n";
}

void defineAST(const std::string& outputDir, const std::string& baseName, const std::vector<std::string>& types) {
    std::string path = outputDir + "/" + baseName + ".hpp";
    std::ofstream outputFile(path);
    if (!outputFile) {
        std::cerr << "Error creating file: " << path << std::endl;
        return;
    }

    // Header
    outputFile << "#pragma once\n";
    outputFile << "#include <string>\n";
    outputFile << "#include <optional>\n";
    outputFile << "#include <vector>\n";
    outputFile << "#include <memory>\n";
    outputFile << "#include \"token.hpp\"\n\n";

    outputFile << "class " << baseName << ";\n";
    outputFile << "class ExprVisitorBase;\n\n";

    // Base class
    outputFile << "class " << baseName << " {\n";
    outputFile << "public:\n";
    outputFile << "    virtual ~" << baseName << "() = default;\n";
    outputFile << "    virtual void accept(const ExprVisitorBase& visitor) const = 0;\n";
    outputFile << "};\n\n";

    // Forward declarations for all classes
    outputFile << "// Forward declarations\n";
    for (const std::string& type : types) {
        std::istringstream iss(type);
        std::string className;
        std::getline(iss, className, ':');
        outputFile << "struct " << trim(className) << ";\n";
    }
    outputFile << "\n";

    // Visitor base interface (non-template)
    outputFile << "class ExprVisitorBase {\n";
    outputFile << "public:\n";
    for (const std::string& type : types) {
        std::istringstream iss(type);
        std::string className;
        std::getline(iss, className, ':');
        className = trim(className);
        outputFile << "    virtual void visit(const " << className << "& expr) const = 0;\n";
    }
    outputFile << "    virtual ~ExprVisitorBase() = default;\n";
    outputFile << "};\n\n";

    // Generate classes
    for (const std::string& type : types) {
        std::istringstream iss(type);
        std::string className, fields;
        std::getline(iss, className, ':');
        std::getline(iss, fields);
        defineType(outputFile, baseName, trim(className), fields);
    }

    // Aliases
    outputFile << "using ExprPtr = std::shared_ptr<" << baseName << ">;\n";
    outputFile << "using ExprList = std::vector<ExprPtr>;\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: generate_ast <output directory>" << std::endl;
        return 64;
    }
    std::string outputDir = argv[1];

    defineAST(outputDir, "Expr", {
        "Binary : Expr left, Token op, Expr right",
        "Grouping : Expr expression",
        "Literal : std::optional<std::string> value",
        "Unary : Token op, Expr right"
    });

    return 0;
}
