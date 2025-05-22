#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
#include<unordered_map>
#include<map>
#include "token.hpp"
#include<vector>

class Tokenizer{
public:
    Tokenizer(const std::string& input) : text(input) {}

    void tokenize(){
        bool hitDef=false;
        std::string combineToken="";
        while(current<text.length()){
            char currentChar = peek();
            char currentToken;
            switch (currentChar){
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
                    hitDef=true;
                    currentToken = consume();
                    std::cerr<<"[line "<<line<<"] Error: Unexpected character: "<<currentToken<<std::endl;
                    break;
            }
        }
        std::cout << "EOF  null" << std::endl;
        if(hitDef){
            exit(65);
        }
    }



private:

    char peek(){
        if(current >= text.length()){
            return '\0';
        }
        return text[current];
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

    bool isAtEnd(){
        return current>=text.length();
    }

    int start=0;
    int current=0;
    int line=1;
    std::string text;
    std::string current_token;
    std::vector<Token> tokens;
};