#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
#include "token.hpp"
#include<iomanip>
#include<vector>
#include<map>

class Tokenizer{
public:
    Tokenizer(const std::string& input) : text(input) {}

    void tokenize(){
        bool hitDef=false;
        std::string buff="";
        while(current<text.length()){
            char currentChar = peek();
            char currentToken;
            switch (currentChar){
                case '"':
                    consume();
                    while(peek()!='"' && !isAtEnd()){
                        if(peek()=='\n'){
                            line++;
                        }
                        buff+=consume();
                    }
                    if(isAtEnd()){
                        std::cerr<<"[line "<<line<<"] Error: Unterminated string."<<std::endl;
                        hitDef=true;
                        break;
                    }
                    consume();
                    addToken(TokenType::STRING, buff);
                    std::cout<<"STRING \""<<buff<<"\" "<<buff<<std::endl;
                    buff="";
                    break;
                case '(':
                    currentToken = consume();
                    std::cout<<"LEFT_PAREN "<<currentToken<<" null"<<std::endl;
                    break;
                case ')':
                    currentToken = consume();
                    std::cout<<"RIGHT_PAREN "<<currentToken<<" null"<<std::endl;
                    break;
                case '{':
                    currentToken = consume();
                    std::cout<<"LEFT_BRACE "<<currentToken<<" null"<<std::endl;
                    break;
                case '}':
                    currentToken = consume();
                    std::cout<<"RIGHT_BRACE "<<currentToken<<" null"<<std::endl;
                    break;
                case ',':
                    currentToken = consume();
                    std::cout<<"COMMA "<<currentToken<<" null"<<std::endl;
                    break;
                case '.':
                    currentToken = consume();
                    std::cout<<"DOT "<<currentToken<<" null"<<std::endl;
                    break;
                case '+':
                    currentToken = consume();
                    std::cout<<"PLUS "<<currentToken<<" null"<<std::endl;
                    break;
                case '-':
                    currentToken = consume();
                    std::cout<<"MINUS "<<currentToken<<" null"<<std::endl;
                    break;
                case '*':
                    currentToken = consume();
                    std::cout<<"STAR "<<currentToken<<" null"<<std::endl;
                    break;
                case '/':
                    currentToken = consume();
                    if(peek()=='/'){
                        while(peek()!='\n' && current<text.length()) consume();
                    }else{
                        std::cout<<"SLASH "<<currentToken<<" null"<<std::endl;
                    }
                    break;
                case ';':
                    currentToken = consume();
                    std::cout<<"SEMICOLON "<<currentToken<<" null"<<std::endl;
                    break;
                case '!':
                    currentToken = consume();
                    if(peek()=='='){
                        buff="";
                        buff+=currentToken;
                        buff+=consume();
                        std::cout<<"BANG_EQUAL "<<buff<<" null"<<std::endl;
                    }else{
                        std::cout<<"BANG "<<currentToken<<" null"<<std::endl;
                    }
                    buff="";
                    break;
                case '=':
                    currentToken = consume();
                    if(peek()=='='){
                        buff="";
                        buff+=currentToken;
                        buff+=consume();
                        std::cout<<"EQUAL_EQUAL "<<buff<<" null"<<std::endl;
                    }else{
                        std::cout<<"EQUAL "<<currentToken<<" null"<<std::endl;
                    }
                    buff="";
                    break;
                case '<':
                    currentToken = consume();
                    if(peek()=='='){
                        buff="";
                        buff+=currentToken;
                        buff+=consume();
                        std::cout<<"LESS_EQUAL "<<buff<<" null"<<std::endl;
                    }else{
                        std::cout<<"LESS "<<currentToken<<" null"<<std::endl;
                    }
                    buff="";
                    break;
                case '>':
                    currentToken = consume();
                    if(peek()=='='){
                        buff="";
                        buff+=currentToken;
                        buff+=consume();
                        std::cout<<"GREATER_EQUAL "<<buff<<" null"<<std::endl;
                    }else{
                        std::cout<<"GREATER "<<currentToken<<" null"<<std::endl;
                    }
                    buff="";
                    break;
                case ' ':
                case '\r':
                case '\t':
                    currentToken = consume();
                    break;
                case '\n':
                    currentToken = consume();
                    line++;
                    break;
                default:
                    if (isDigit(currentChar)) {
                        number();
                    } else if (currentChar == '.' && isDigit(peek(1))) {
                        consume(); // consume the dot
                        std::cout << "DOT . null" << std::endl;
                        number(); // consume the rest as number
                    } else if(isalpha(currentChar) || currentChar == '_') {
                        identifier();
                    }else {
                        std::cerr << "[line " << line << "] Error: Unexpected character: " << currentChar  << std::endl;
                        hitDef = true;
                        consume(); // advance to avoid infinite loop
                    }
                    break;
            }
        }
        std::cout << "EOF  null" << std::endl;
        if(hitDef){
            exit(65);
        }
    }



private:

    char peek(int index=0){
        if(current + index >= text.length()){
            return '\0';
        }
        return text[current+index];
    }

    char consume(){
        return text[current++];
    }

    void addToken(TokenType type){
        addToken(type, literal());
    }

    bool isDigit(char c){
        return c>='0' && c<='9';
    }

    void addToken(TokenType type, literal lit){
        std::string lexeme = text.substr(start, current - start);
        tokens.push_back(Token(type,lexeme,lit,line));
    }

    std::string normalizeNumberLiteral(const std::string& numStr) {
        size_t dotPos = numStr.find('.');
        if (dotPos == std::string::npos) {
            // No decimal point, add ".0"
            return numStr + ".0";
        }

        // Start from the end and remove trailing zeros
        size_t endPos = numStr.size() - 1;
        while (endPos > dotPos && numStr[endPos] == '0') {
            endPos--;
        }

        // If the last char after trimming is '.', append '0'
        if (endPos == dotPos) {
            endPos++;
        }

        return numStr.substr(0, endPos + 1);
    }

    void identifier(){
        std::string buff="";
        while(isalnum(peek()) || peek()=='_'){
            buff+=consume();
        }
        TokenType type = TokenType::IDENTIFIER;
        if(keywords.find(buff)!=keywords.end()){
            type=keywords[buff];
        }
        addToken(type);
        std::cout << "IDENTIFIER " << buff << " null" << std::endl;
    }

    void number() {
        std::string buff = "";
        bool isFloat = false;

        while (isDigit(peek())) {
            buff += consume();
        }

        // Handle case like "123." (trailing dot but not float)
        if (peek() == '.' && !isDigit(peek(1))) {
            double value = std::stod(buff);
            std::cout << "NUMBER " << buff << " " << buff << ".0" << std::endl;
            std::cout << "DOT . null" << std::endl;
            consume();
            return;
        }

        // Handle float like "123.456"
        if (peek() == '.' && isDigit(peek(1))) {
            isFloat = true;
            buff += consume(); // consume the dot
            while (isDigit(peek())) {
                buff += consume();
            }
        }

        if (isFloat) {
            std::string normalized = normalizeNumberLiteral(buff);
            std::cout << "NUMBER " << buff << " " << normalized << std::endl;
        }else {
            std::string normalized = buff+".0";
            std::cout << "NUMBER " << buff << " " << normalized << std::endl;
        }
    }


    bool isAtEnd(){
        return current>text.length();
    }

    int start=0;
    int current=0;
    int line=1;
    std::string text;
    std::string current_token;
    std::vector<Token> tokens;
    std::map<std::string,TokenType> keywords = {
        {"and", TokenType::AND},
        {"class", TokenType::CLASS},
        {"else", TokenType::ELSE},
        {"false", TokenType::FALSE},
        {"fun", TokenType::FUN},
        {"for", TokenType::FOR},
        {"if", TokenType::IF},
        {"nil", TokenType::NIL},
        {"or", TokenType::OR},
        {"print", TokenType::PRINT},
        {"return", TokenType::RETURN},
        {"super", TokenType::SUPER},
        {"this", TokenType::THIS},
        {"true", TokenType::TRUE},
        {"var", TokenType::VAR},
        {"while", TokenType::WHILE}
    };
};