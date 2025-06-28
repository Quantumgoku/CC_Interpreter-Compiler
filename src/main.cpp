#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include "tokenizer.hpp"
#include "parser.hpp"
#include "astprinter.hpp"
#include "interpreter.hpp"
#include "RuntimeError.hpp"
#include "literal.hpp"
#include "Resolver.hpp"

std::string read_file_contents(const std::string& filename);

/**
 * Execute a Lox program by resolving variable scopes and then interpreting.
 */
void interpret(std::vector<std::shared_ptr<Stmt>>& statements){
    Interpreter interpreter;
    Resolver resolver(interpreter);
    resolver.resolve(statements);
    for(const auto& statement : statements){
        interpreter.execute(*statement);
    }
}

/**
 * Lox Interpreter Main Entry Point
 * Supports multiple modes: tokenize, parse, evaluate, and run
 */
int main(int argc, char *argv[]) {
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    try {
        if (argc < 3) {
            std::cerr << "Usage: ./your_program <command> <filename>" << std::endl;
            std::cerr << "Commands: tokenize, parse, evaluate, run" << std::endl;
            return 1;
        }

        const std::string command = argv[1];
        std::string file_contents = read_file_contents(argv[2]);

        if (command == "tokenize") {
            // Tokenize the source and print tokens
            Tokenizer tokenizer(file_contents, true);
            tokenizer.tokenize();
        } else if(command == "parse"){
            // Parse an expression and print the AST
            Tokenizer Tokenizer(file_contents, false);
            std::vector<Token> tokens = Tokenizer.tokenize();
            Parser parser(tokens);
            std::shared_ptr<Expr> expr = parser.parseExpr();
            if(expr){
                ASTPrinter printer;
                std::string output = printer.print(*expr);
                std::cout << output << std::endl;
            } else {
                std::cerr << "Parsing failed." << std::endl;
                return 65;
            }
        } else if(command == "evaluate"){
            // Evaluate a single expression and print the result
            Tokenizer tokenizer(file_contents, false);
            std::vector<Token> tokens = tokenizer.tokenize();
            Parser parser(tokens);
            std::shared_ptr<Expr> expr = parser.parseExpr();
            if (expr) {
                try{
                    Interpreter interpreter;
                    lox_literal output = interpreter.evaluate(*expr);
                    std::cout << literal_to_string(output) << std::endl;
                }catch(const RuntimeError& e){
                    std::cerr << e.what() << "\n";
                    std::cerr << e.token.getLine() << std::endl;
                    return 70;
                }
            } else {
                std::cerr << "Evaluating failed." << std::endl;
                return 65;
            }
        } else if(command == "run"){
            // Run a complete Lox program
            try{
                Tokenizer tokenizer(file_contents, false);
                std::vector<Token> tokens = tokenizer.tokenize();
                Parser parser(tokens);
                std::vector<std::shared_ptr<Stmt>> statements = parser.parse();
                if (!statements.empty()) {
                    try {
                        Interpreter interpreter;
                        Resolver resolver(interpreter);
                        
                        // Phase 1: Resolve variable scopes (compile-time)
                        try {
                            resolver.resolve(statements);
                        } catch(const RuntimeError& e) {
                            std::cerr << e.what() << "\n";
                            std::cerr << e.token.getLine() << std::endl;
                            return 65;
                        }
                        
                        // Phase 2: Execute the program (runtime)
                        for(const auto& statement : statements){
                            interpreter.execute(*statement);
                        }
                    } catch(const RuntimeError& e) {
                        std::cerr << e.what() << "\n";
                        std::cerr << e.token.getLine() << std::endl;
                        return 70;
                    }
                } else {
                    std::cerr << "Executing failed." << std::endl;
                    return 65;
                }
            }catch(const Parser::ParseError& e){
                std::cerr << e.what() << std::endl;
                return 65;
            }catch(const std::exception& e){
                std::cerr << e.what() << std::endl;
                return 65;
            }catch(...){
                std::cerr << "An unknown error occurred." << std::endl;
                return 65;
            }
        } else {
            std::cerr << "Unknown command: " << command << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        std::exit(70);
    } catch (...) {
        std::cerr << "Unknown fatal error." << std::endl;
        std::exit(70);
    }
    std::exit(0);
}

/**
 * Read the entire contents of a file into a string.
 */
std::string read_file_contents(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error reading file: " << filename << std::endl;
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}
