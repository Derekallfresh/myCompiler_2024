#include"front/lexical.h"

#include<map>
#include<cassert>
#include<string>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

// 辅助函数
frontend::TokenType get_op_type(std::string s){     // 获取运算符类型
    if (s == "+")   return frontend::TokenType::PLUS;
    else if (s == "-")  return frontend::TokenType::MINU;
    else if (s == "*")  return frontend::TokenType::MULT;
    else if (s == "/")  return frontend::TokenType::DIV;
    else if (s == "%")  return frontend::TokenType::MOD;
    else if (s == "<")  return frontend::TokenType::LSS;
    else if (s == ">")  return frontend::TokenType::GTR;
    else if (s == ":")  return frontend::TokenType::COLON;
    else if (s == "=")  return frontend::TokenType::ASSIGN;
    else if (s == ";")  return frontend::TokenType::SEMICN;
    else if (s == ",")  return frontend::TokenType::COMMA;
    else if (s == "(")  return frontend::TokenType::LPARENT;
    else if (s == ")")  return frontend::TokenType::RPARENT;
    else if (s == "[")  return frontend::TokenType::LBRACK;
    else if (s == "]")  return frontend::TokenType::RBRACK;
    else if (s == "{")  return frontend::TokenType::LBRACE;
    else if (s == "}")  return frontend::TokenType::RBRACE;
    else if (s == "!")  return frontend::TokenType::NOT;
    else if (s == "<=") return frontend::TokenType::LEQ;
    else if (s == ">=") return frontend::TokenType::GEQ;
    else if (s == "==") return frontend::TokenType::EQL;
    else if (s == "!=") return frontend::TokenType::NEQ;
    else if (s == "&&") return frontend::TokenType::AND;
    else if (s == "||") return frontend::TokenType::OR;
    else    std::cout << "invalid Op:" << s << std::endl;
}

frontend::TokenType get_ident_type(std::string s){  // 获取关键字类型
    // TokenType中IDENFR、INTLTR、FLOATLTR都不是确定字符串，因此无法都在函数内判断
    if (s == "const")   return frontend::TokenType::CONSTTK;
    else if (s == "void")   return frontend::TokenType::VOIDTK;
    else if (s == "int")    return frontend::TokenType::INTTK;
    else if (s == "float")  return frontend::TokenType::FLOATTK;
    else if (s == "if")     return frontend::TokenType::IFTK;
    else if (s == "else")   return frontend::TokenType::ELSETK;
    else if (s == "while")  return frontend::TokenType::WHILETK;
    else if (s == "continue")   return frontend::TokenType::CONTINUETK;
    else if (s == "break")  return frontend::TokenType::BREAKTK;
    else if (s == "return") return frontend::TokenType::RETURNTK;
    else    return frontend::TokenType::IDENFR;     // 函数名或变量名
}

bool isCharOp(char c){
    if(c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='<'||c=='>'||c=='='||c==':'||c==';'||c=='('||c==')'||c=='['||c==']'||c=='{'||c=='}'||c=='!'||c=='&'||c=='|'||c==','){
        return true;
    }else{
        return false;
    }
}

bool isStringOp(std::string s){
    if(s=="+"||s=="-"||s=="*"||s=="/"||s=="%"||s=="<"||s==">"||s=="="||s==":"||s==";"||s=="("||s==")"||s=="["||s=="]"||s=="{"||s=="}"||s=="!"||s=="&"||s=="|"||s==","||s=="<="||s==">="||s=="=="||s=="!="||s=="&&"||s=="||"){
        return true;
    }else{
        return false;
    }
}

std::string frontend::toString(State s) {
    switch (s) {
    case State::Empty: return "Empty";
    case State::Ident: return "Ident";
    case State::IntLiteral: return "IntLiteral";
    case State::FloatLiteral: return "FloatLiteral";
    case State::op: return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}

std::set<std::string> frontend::keywords= {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"
};

std::string preScanner(std::ifstream &fin) {
    std::string result;
    std::string line;
    bool inMultiLineComment = false;
    bool inString = false;  // 是否在字符串内
    char stringDelimiter = 0; // 当前字符串的分隔符

    while (std::getline(fin, line)) {
        size_t pos = 0;

        if (inMultiLineComment) {
            pos = line.find("*/");
            if (pos != std::string::npos) {
                inMultiLineComment = false;
                line.erase(0, pos + 2);
            } else {
                continue;
            }
        }

        for (pos = 0; pos < line.length(); ++pos) {
            if (!inString && line[pos] == '\"' || line[pos] == '\'') {
                inString = true;
                stringDelimiter = line[pos];
            } else if (inString && line[pos] == stringDelimiter) {
                inString = false;
            } else if (!inString && line.compare(pos, 2, "//") == 0) {
                line.erase(pos);
                break;
            } else if (!inString && line.compare(pos, 2, "/*") == 0) {
                size_t endPos = line.find("*/", pos + 2);
                if (endPos != std::string::npos) {
                    line.erase(pos, endPos - pos + 2);
                } else {
                    line.erase(pos);
                    inMultiLineComment = true;
                }
                break;
            }
        }

        result += line + "\n";
    }

    return result;
}




// 待定义函数
frontend::DFA::DFA(): cur_state(frontend::State::Empty), cur_str() {}

frontend::DFA::~DFA() {}
void frontend::DFA::reset() {
    cur_state = State::Empty;
    cur_str = "";
}

bool frontend::DFA::next(char input, Token& buf) {
    // 打印当前状态
    #ifdef DEBUG_DFA
    #include<iostream>
        std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\t";
    #endif

    bool flag = false; // 标记是否生成新的 token

    switch (cur_state) {
        case frontend::State::Empty:
            processEmpty(input, buf, flag);
            break;
        case frontend::State::Ident:
            processIdent(input, buf, flag);
            break;
        case frontend::State::op:
            processOp(input, buf, flag);
            break;
        case frontend::State::IntLiteral:
            processIntLiteral(input, buf, flag);
            break;
        case frontend::State::FloatLiteral:
            processFloatLiteral(input, buf, flag);
            break;
        default:
            assert(0 && "invalid State");
    }

    // 打印下一个状态
    #ifdef DEBUG_DFA
        std::cout << "next state is [" << toString(cur_state) << "], next str = " << cur_str << ", ret = " << flag << std::endl;
    #endif

    return flag;
}

void frontend::DFA::processEmpty(char input, Token& buf, bool& flag) {
    if (isspace(input)) {
        reset();
    } else if (isalpha(input) || input == '_') {
        cur_state = frontend::State::Ident;
        cur_str += input;
    } else if (isdigit(input)) {
        cur_state = frontend::State::IntLiteral;
        cur_str += input;
    } else if (input == '.') {
        cur_state = frontend::State::FloatLiteral;
        cur_str += input;
    } else if (isCharOp(input)) {
        cur_state = frontend::State::op;
        cur_str += input;
    } else {
        assert(0 && "invalid next State");
    }
}

void frontend::DFA::processIdent(char input, Token& buf, bool& flag) {
    if (isspace(input)) {
        buf.type = get_ident_type(cur_str);
        buf.value = cur_str;
        reset();
        flag = true;
    } else if (isalpha(input) || isdigit(input) || input == '_') {
        cur_str += input;
    } else if (isCharOp(input)) {
        buf.type = get_ident_type(cur_str);
        buf.value = cur_str;
        cur_state = frontend::State::op;
        cur_str = input;
        flag = true;
    } else {
        assert(0 && "invalid next State");
    }
}

void frontend::DFA::processOp(char input, Token& buf, bool& flag) {
    if (isspace(input)) {
        buf.type = get_op_type(cur_str);
        buf.value = cur_str;
        reset();
        flag = true;
    } else if (isalpha(input) || input == '_') {
        buf.type = get_op_type(cur_str);
        buf.value = cur_str;
        cur_state = frontend::State::Ident;
        cur_str = input;
        flag = true;
    } else if (isdigit(input)) {
        buf.type = get_op_type(cur_str);
        buf.value = cur_str;
        cur_state = frontend::State::IntLiteral;
        cur_str = input;
        flag = true;
    } else if (input == '.') {
        buf.type = get_op_type(cur_str);
        buf.value = cur_str;
        cur_state = frontend::State::FloatLiteral;
        cur_str = input;
        flag = true;
    } else if (isCharOp(input)) {
        if (isStringOp(cur_str + input)) {
            cur_str += input;
        } else {
            buf.type = get_op_type(cur_str);
            buf.value = cur_str;
            cur_str = input;
            flag = true;
        }
    } else {
        assert(0 && "invalid next State");
    }
}

void frontend::DFA::processIntLiteral(char input, Token& buf, bool& flag) {
    if (isspace(input)) {
        buf.type = frontend::TokenType::INTLTR;
        buf.value = cur_str;
        reset();
        flag = true;
    } else if (isdigit(input) || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F') || input == 'x' || input == 'X') {
        cur_str += input;
    } else if (input == '.') {
        cur_state = frontend::State::FloatLiteral;
        cur_str += input;
    } else if (isCharOp(input)) {
        buf.type = frontend::TokenType::INTLTR;
        buf.value = cur_str;
        cur_state = frontend::State::op;
        cur_str = input;
        flag = true;
    } else {
        assert(0 && "invalid next State");
    }
}

void frontend::DFA::processFloatLiteral(char input, Token& buf, bool& flag) {
    if (isspace(input)) {
        buf.type = frontend::TokenType::FLOATLTR;
        buf.value = cur_str;
        reset();
        flag = true;
    } else if (isdigit(input)) {
        cur_str += input;
    } else if (isCharOp(input)) {
        buf.type = frontend::TokenType::FLOATLTR;
        buf.value = cur_str;
        cur_state = frontend::State::op;
        cur_str = input;
        flag = true;
    } else {
        assert(0 && "invalid next State");
    }
}

frontend::Scanner::Scanner(std::string filename): fin(filename) {
    if(!fin.is_open()) {
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

frontend::Scanner::~Scanner() {
    fin.close();
}

std::vector<frontend::Token> frontend::Scanner::run() { // 使用DFA接受字符串前需要对字符串进行预处理
    std::vector<Token> result;
    Token tk;
    DFA dfa;
    std::string s = preScanner(fin);    // 删除注释
    s += "\n";                          // s结尾加上换行符
    for (auto c : s){
        if (dfa.next(c, tk)){
            // 压入token
            result.push_back(tk);
            // 打印结果
            #ifdef DEBUG_SCANNER
            #include <iostream>
                std::cout << "token: " << toString(tk.type) << "\t" << tk.value << std::endl;
            #endif
        }
    }
    return result;
}