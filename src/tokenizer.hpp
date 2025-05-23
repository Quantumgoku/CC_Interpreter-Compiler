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
                    if(isDigit(currentChar)){
                        number();
                    }else{
                        std::cerr<<"[line "<<line<<"] Error: Unexpected character."<<std::endl;
                        hitDef=true;
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

    void number(){
        while(isDigit(peek())){
            consume();
        }
        if(peek() == '.' && isDigit(peek(1))){
            consume();
            while(isDigit(peek())){
                consume();
            }
        }
        std::string num = text.substr(start, current - start);
        double value = std::stod(num);
        addToken(TokenType::NUMBER, value);
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