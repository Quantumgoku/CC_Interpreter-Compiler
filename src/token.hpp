#include<string>
#include<variant>
#include<optional>

using literal = std::variant<std::monostate, std::string, double, bool>;

enum class TokenType{
    // Single-character tokens.
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SEMICOLON,
    SLASH,
    STAR,

    // One or two character tokens.
    BANG,
    BANG_EQUAL,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // Literals.
    IDENTIFIER,
    STRING,
    NUMBER,

    // Keywords.
    AND,
    CLASS,
    ELSE,
    FALSE,
    FUN,
    FOR,
    IF,
    NIL,
    OR,
    PRINT,
    RETURN,
    SUPER,
    THIS,
    TRUE,
    VAR,
    WHILE,

    END_OF_FILE,
};


class Token{
public:
    Token(TokenType type,std::string lexme, literal lit,int line){
        this->type=type;
        this->lexme=lexme;
        this->lit=lit;
        this->line=line;
    }

    TokenType getType() const {
        return type;
    }
    std::string getLexme() const {
        return lexme;
    }
    literal getLit() const {
        return lit;
    }
    int getLine() const {
        return line;
    }

private:
    TokenType type;
    std::string lexme;
    literal lit;
    int line;
};