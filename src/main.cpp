#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::string read_file_contents(const std::string& filename);

class Tokenizer{
public:
    Tokenizer(const std::string& input) : index(0), text(input) {}

    void tokenize(){
        while(index<text.length()){
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
                case ';':
                    currentToken = consume();
                    std::cout<<"SEMICOLON "<<currentToken<<" null"<<std::endl;
                    break;
                default:
                    currentToken = consume();
                    std::cerr<<"[line 1] Error: Unexpected character: "<<currentToken<<std::endl;
                    break;
            }
        }
        std::cout << "EOF  null" << std::endl;
    }

private:

    char peek(){
        if(index >= text.length()){
            return '\0';
        }
        return text[index];
    }

    char consume(){
        return text[index++];
    }

    int index;
    std::string text;
    std::string current_token;
};

int main(int argc, char *argv[]) {
    // Disable output buffering
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here!" << std::endl;

    if (argc < 3) {
        std::cerr << "Usage: ./your_program tokenize <filename>" << std::endl;
        return 1;
    }

    const std::string command = argv[1];

    if (command == "tokenize") {
        std::string file_contents = read_file_contents(argv[2]);
        
        // Uncomment this block to pass the first stage
        // 
        // if (!file_contents.empty()) {
        //     std::cerr << "Scanner not implemented" << std::endl;
        //     return 1;
        // }
        Tokenizer tokenizer(file_contents);
        tokenizer.tokenize();
        
    } else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    return 0;
}

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
