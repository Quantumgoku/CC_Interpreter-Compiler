#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
#include "token.hpp"
#include<vector>

class Tokenizer{
public:
    Tokenizer(const std::string& input) : text(input) {}

    void tokenize(){
        bool hitDef=false;
        std::string combineToken="";
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
                        combineToken+=currentToken;
                        combineToken+=consume();
                        std::cout<<"BANG_EQUAL "<<combineToken<<" null"<<std::endl;
                    }else{
                        std::cout<<"BANG "<<currentToken<<" null"<<std::endl;
                    }
                    combineToken="";
                    break;
                case '=':
                    currentToken = consume();
                    if(peek()=='='){
                        combineToken+=currentToken;
                        combineToken+=consume();
                        std::cout<<"EQUAL_EQUAL "<<combineToken<<" null"<<std::endl;
                    }else{
                        std::cout<<"EQUAL "<<currentToken<<" null"<<std::endl;
                    }
                    combineToken="";
                    break;
                case '<':
                    currentToken = consume();
                    if(peek()=='='){
                        combineToken+=currentToken;
                        combineToken+=consume();
                        std::cout<<"LESS_EQUAL "<<combineToken<<" null"<<std::endl;
                    }else{
                        std::cout<<"LESS "<<currentToken<<" null"<<std::endl;
                    }
                    combineToken="";
                    break;
                case '>':
                    currentToken = consume();
                    if(peek()=='='){
                        combineToken+=currentToken;
                        combineToken+=consume();
                        std::cout<<"GREATER_EQUAL "<<combineToken<<" null"<<std::endl;
                    }else{
                        std::cout<<"GREATER "<<currentToken<<" null"<<std::endl;
                    }
                    combineToken="";
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
                    if(currentChar>='0' && currentChar<='9'){
                        current_token=consume();
                        number();
                    }else{
                        char consumed=consume();
                        hitDef=true;
                        std::cerr<<"[line "<<line<<"] Error: Unexpected character."<<consumed<<std::endl;
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

    void addToken(TokenType type, literal lit){
        std::string lexeme = text.substr(start, current - start);
        tokens.push_back(Token(type,lexeme,lit,line));
    }

    void number(){
        std::string buff = "";
        buff += current_token;
        while(isdigit(peek())){
            buff += consume();
        }
        // Check for fractional part
        if(peek() == '.' && isdigit(peek(1))){
            buff += consume();
            while(isdigit(peek())){
                buff += consume();
            }
        }
        double value = std::stod(buff);
        addToken(TokenType::NUMBER, literal(value));
        // Print .0 for integers, print as-is for floats
        if (buff.find('.') == std::string::npos) {
            std::cout << "NUMBER " << buff << " " << buff << ".0" << std::endl;
        } else {
            std::cout << "NUMBER " << buff << " " << buff << std::endl;
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
};