# Lox Interpreter in C++

A complete implementation of the Lox programming language interpreter in C++, following the "Crafting Interpreters" book by Robert Nystrom. This implementation supports all language features including classes, inheritance, closures, and advanced scoping semantics.

[Link to Deep Wiki Page](https://deepwiki.com/Quantumgoku/CC_Interpreter-Compiler)

## Features

### Core Language Features
- **Variables and Data Types**: Numbers, strings, booleans, and nil
- **Expressions**: Arithmetic, comparison, logical, and assignment operations
- **Control Flow**: If statements, while loops, and for loops
- **Functions**: First-class functions with lexical scoping and closures
- **Classes**: Object-oriented programming with inheritance
- **Built-in Functions**: Clock function for timing

### Advanced Features
- **Lexical Scoping**: Proper variable resolution with nested scopes
- **Closures**: Functions that capture their lexical environment
- **Method Binding**: Automatic binding of `this` in method calls
- **Inheritance**: Single inheritance with `super` keyword support
- **Super in Closures**: Advanced case where closures inside methods use `super`

## Architecture

The interpreter follows a classic tree-walking interpreter design with these phases:

1. **Lexical Analysis** (`Tokenizer`): Converts source code into tokens
2. **Parsing** (`Parser`): Builds an Abstract Syntax Tree (AST)
3. **Resolution** (`Resolver`): Analyzes variable scoping and binding
4. **Interpretation** (`Interpreter`): Executes the AST

### Key Components

```
src/
├── main.cpp              # Entry point and command handling
├── tokenizer.hpp         # Lexical analysis
├── parser.hpp            # Syntax analysis and AST construction
├── Resolver.hpp          # Variable scope resolution
├── interpreter.hpp       # AST evaluation and execution
├── Environment.hpp       # Variable environment management
├── Expr.hpp              # Expression AST nodes
├── Stmt.hpp              # Statement AST nodes
├── LoxCallable.hpp       # Interface for callable objects
├── LoxFunction.hpp/cpp   # Function and method implementation
├── LoxClass.hpp/cpp      # Class implementation
├── LoxInstance.hpp/cpp   # Object instance implementation
└── literal.hpp           # Value representation
```

## Lox Grammar

The complete grammar for the Lox language:

```
program        → declaration* EOF ;

declaration    → classDecl
               | funDecl
               | varDecl
               | statement ;

classDecl      → "class" IDENTIFIER ( "<" IDENTIFIER )?
                 "{" function* "}" ;
funDecl        → "fun" function ;
varDecl        → "var" IDENTIFIER ( "=" expression )? ";" ;

statement      → exprStmt
               | forStmt
               | ifStmt
               | printStmt
               | returnStmt
               | whileStmt
               | block ;

exprStmt       → expression ";" ;
forStmt        → "for" "(" ( varDecl | exprStmt | ";" )
                           expression? ";"
                           expression? ")" statement ;
ifStmt         → "if" "(" expression ")" statement
                 ( "else" statement )? ;
printStmt      → "print" expression ";" ;
returnStmt     → "return" expression? ";" ;
whileStmt      → "while" "(" expression ")" statement ;
block          → "{" declaration* "}" ;

function       → IDENTIFIER "(" parameters? ")" block ;
parameters     → IDENTIFIER ( "," IDENTIFIER )* ;

expression     → assignment ;
assignment     → ( call "." )? IDENTIFIER "=" assignment
               | logic_or ;
logic_or       → logic_and ( "or" logic_and )* ;
logic_and      → equality ( "and" equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;

unary          → ( "!" | "-" ) unary | call ;
call           → primary ( "(" arguments? ")" | "." IDENTIFIER )* ;
primary        → "true" | "false" | "nil" | "this"
               | NUMBER | STRING | IDENTIFIER | "(" expression ")"
               | "super" "." IDENTIFIER ;

arguments      → expression ( "," expression )* ;
```

## Language Examples

### Basic Variables and Functions
```lox
var message = "Hello, World!";
print message;

fun greet(name) {
    return "Hello, " + name + "!";
}

print greet("Alice");
```

### Classes and Inheritance
```lox
class Animal {
    init(name) {
        this.name = name;
    }
    
    speak() {
        print this.name + " makes a sound";
    }
}

class Dog < Animal {
    speak() {
        print this.name + " barks";
    }
    
    wagTail() {
        print this.name + " wags tail";
    }
}

var dog = Dog("Buddy");
dog.speak();    // "Buddy barks"
dog.wagTail();  // "Buddy wags tail"
```

### Super Calls
```lox
class A {
    method() {
        print "A method";
    }
}

class B < A {
    method() {
        print "B method";
        super.method();
    }
}

B().method();
// Output:
// B method
// A method
```

### Closures with Super
```lox
class A {
    say() {
        print "A";
    }
}

class B < A {
    getClosure() {
        fun closure() {
            super.say();  // Captures the correct super binding
        }
        return closure;
    }
    
    say() {
        print "B";
    }
}

class C < B {
    say() {
        print "C";
    }
}

// Even though called on C instance, super refers to A
C().getClosure()();  // Prints: "A"
```

## Building and Running

### Prerequisites
- C++17 compatible compiler
- CMake 3.10 or higher

### Build Instructions
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Debug
```

### Usage
The interpreter supports multiple modes:

```bash
# Tokenize source code
./interpreter tokenize file.lox

# Parse and print AST
./interpreter parse file.lox

# Evaluate single expression
./interpreter evaluate file.lox

# Run complete program
./interpreter run file.lox
```

## Error Handling

The interpreter provides detailed error messages with line numbers for:
- **Lexical errors**: Invalid characters or tokens
- **Syntax errors**: Malformed expressions or statements
- **Resolution errors**: Invalid variable usage (compile-time)
- **Runtime errors**: Type mismatches, undefined variables, etc.

### Error Codes
- `0`: Success
- `1`: Usage error
- `65`: Compilation error (syntax or resolution)
- `70`: Runtime error

## Technical Implementation Details

### Environment Chain
Variables are resolved using a chain of environments:
- Each scope creates a new environment
- Environments link to their enclosing scope
- Variable lookup traverses the chain

### Method Binding
Methods are bound to instances using these steps:
1. Create a new function with the instance stored
2. During execution, inject `this` into the execution environment
3. For returned closures, automatically bind to the same instance

### Super Resolution
The `super` keyword is handled specially:
1. Resolver tracks class context and assigns distances
2. Interpreter looks up `super` at the resolved distance
3. For closures, `this` is found in the execution environment (distance 0)

### Closure Implementation
Closures capture their lexical environment:
1. Functions store a reference to their declaration environment
2. When called, create execution environment with closure as parent
3. Returned closures from methods are automatically bound to the method's instance

## Testing

The implementation includes comprehensive tests for:
- Basic language features
- Class inheritance
- Method overriding
- Super calls in various contexts
- Closures with captured variables
- Edge cases with super in closures


## Contributing

This implementation follows the Crafting Interpreters book closely while adding modern C++ features and comprehensive error handling. The code is structured for clarity and maintainability.

Key design principles:
- Use of smart pointers for memory management
- Visitor pattern for AST traversal
- Exception-based error handling
- Clear separation of concerns between phases

## License

This implementation is provided for educational purposes, following the concepts from "Crafting Interpreters" by Robert Nystrom.

---
