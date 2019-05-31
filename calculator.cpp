/*
This is the modification of the example code from Chapter 6.7 "Principles and Practice using C++" by Bjarne Stroustrup

n33 (2019.5.19)
*/

#define _USE_MATH_DEFINES  // to use M_PI, M_E
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <chrono>  // to set random number seed 
using namespace std;

//------------------------------------------------------------------------------

// This section contains some operational-use functions

/* Display error */
void error(string message)
{
    throw runtime_error(message);
}

/* Create separation line in terminal window */
string display_line(int n) {
    string s;
    for (int i = 0; i < n ; i++) {
        s += "-";  // the separation line consists of multiple '-' symbols
    }
    s += '\n';
    return s;
}

//------------------------------------------------------------------------------

// This section contains some preliminary declarations
unordered_map<string, double> dict;  // stores variables
unordered_set<string> ban;  // stores prohibited names for variables

const char print = '\n';  // press Enter to calculate
const char quit = '$';  // use $ because it cannot be a name
const char number = 'n';
const char caled = 'c';  // means calculated, to prevent last result keep being displayed 
const char define = '@';  
const char special = '#';  // special operation
const char comma = 'a';  // saw a comma, so there are both lower bound and upper bound

//------------------------------------------------------------------------------

// This section contains the calculator Token class 

/* Token is a user-defined type for whatever the user inputs for calculation */
class Token 
{
    public:
        char key;  
        double value; 
        string name;
        Token(): key('d'), value(1) {}  // default Token constructor, 1 because *1, /1 = 1
        Token(char ch): key(ch), value(0) {}  // make a Token from a symbol   
        Token(char ch, double val): key(ch), value(val) {}  // make a Token from a number
        Token(char ch, string s): key(ch), name(s) {}  // make a Token from a variable
};

/* Stores Tokens in a way similar to iostream */
class Token_stream 
{
    private:
        bool full {false}; // is there a Token in the buffer?
        Token buffer;      // here is where we keep a Token put back using putback()
    public: 
        Token_stream(): full(false), buffer(0) {};   // make a Token_stream that reads from cin
        Token get_Token();      // get a Token 
        void putback(Token t);    // put a Token back
        void reset();  // reset if error occurs
};

/* Read the next input and convert it to a Token */
Token Token_stream::get_Token()
{
    if (full) {  // if already has a Token in stream buffer
        full = false;
        return buffer;
    } 

    char ch; cin.get(ch);  // to read '\n'
    while (true) {
        switch (ch) {
            case ' ': case '\t': {  // ignore whitespaces
                cin.get(ch); break;
            }
            case print: case quit:
            case '(': case ')': case '+': case '-': case '*': case '/': case '%': case '^': case '!': case ',': {
                return Token(ch);
            } 
            case '.':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9': {
                cin.putback(ch);
                double val; cin >> val;  // read the whole number
                return Token(number, val);
            }
            default:
                if (isalpha(ch)) {
                    string v_name; v_name += ch;
                    while (cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) v_name += ch;  // variable name ends at eof or '='
                    while (ch == ' ' || ch == '\t') cin.get(ch);
                    if (ch == '=') {  // if define a variable
                        if (ban.find(v_name) != ban.end()) {
                            error("\"" + v_name + "\" is a built-in name.");
                        }
                        else {  // if variable name is OK to use
                            cin >> ch; cin.putback(ch);  // outside below if(s) because the first if will read ch then the second if won't read the same element
                            if (isdigit(ch) || isalpha(ch)) {  // define variable using numbers or other variables
                                return Token(define, v_name);
                            }
                            else {
                                error("Wrong way to define variables!");
                            }
                        }
                    }
                    else {  // if use a variable
                        cin.putback(ch);
                        if (ban.find(v_name) != ban.end()) {  // if a special operation
                            return Token(special, v_name);
                        }
                        else if (v_name.substr(0, 3) == "log") {  // if a logarithm
                            return Token(special, v_name);
                        }                        
                        else {
                            if (dict.find(v_name) != dict.end()) return Token(number, dict[v_name]);
                            else error("No such variable \"" + v_name + "\"");
                        }
                    }
                }
                error("Bad token");
                return 1; // because this is not a void function
        }
    }
}

/* Put Token back into the Token_stream's buffer */
void Token_stream::putback(Token t) {
    if (full) error("Token stream already full, cannot put back");
    buffer = t;       // copy t to buffer
    full = true;      // buffer is now full
}

/* Ignore what's left in Token stream */
void Token_stream::reset() {
    full = false;
    if (buffer.key != print) {
        char ch;
        while (cin.get(ch)) {
            if (ch == print) break;
        }
    }
    cin.clear();  // seems without this above while loop won't be executed
    cout << "\nErrors have been cleared, you can run the calculator as usual or press '$' to exit.\n";
    cout << display_line(100);
}

//------------------------------------------------------------------------------

// This section defines some special operations

Token statement();  // below special operations nee to know what is statement

/* Initialize constant names and values */
void init_const() {
    ban.insert("pi");
    ban.insert("e");
    ban.insert("inf");
    ban.insert("sqrt");
    ban.insert("log");
    ban.insert("sin");
    ban.insert("cos");
    ban.insert("tan");
    ban.insert("cot");
    ban.insert("sind");
    ban.insert("cosd");
    ban.insert("tand");
    ban.insert("cotd");
    ban.insert("rand");

    dict["pi"] = M_PI; 
    dict["e"] = M_E;
}

/* Factorial */
double factorial(int n) {
    int sum {1};
    if (n != 0) {
        for (int i = n; i > 0; i--) {
            sum *= i;
        }
    }
    return sum;
}

/* Square root */
double square_root() {
    Token spe = statement();
    return sqrt(spe.value);
}

/* Logarithm */
double logarithm(string s) {
    double base;
    try {
        Token spe = statement();
        if (s == "loge" || s == "log") {
            return log(spe.value) / log(dict["e"]);  // log base change
        }
        else {
            string subs = s.substr(3, s.size() - 3);
            base = stod(subs);
            return log(spe.value) / log(base);
        }
    }
    catch (invalid_argument) {
        error("No such variable \"" + s + "\""); return 1;
    }
}

/* Trigonometry */
double trigonometry(string s) {
    Token spe = statement();
    if (s == "sin") {
        double angle = spe.value * 180.0 / dict["pi"];
        if (angle - 180.0 * int(angle / 180.0) == 0) return 0;  // otherwise STL returns a very small number instead of 0
        else return sin(spe.value);
    }
    else if (s == "cos") {
        double angle = spe.value * 180.0 / dict["pi"];
        if (abs(angle - 180.0 * int(angle / 180.0)) == 90.0) return 0;
        else return cos(spe.value);
    }
    else if (s == "tan") {
        double angle = spe.value * 180.0 / dict["pi"];
        if (angle - 180.0 * int(angle / 180.0) == 0) return 0;
        else if (abs(angle - 180.0 * int(angle / 180.0)) == 90.0) error("Inf");
        else return sin(spe.value) / cos(spe.value);
    } 
    else if (s == "cot") {
        double angle = spe.value * 180.0 / dict["pi"];
        if (angle - 180.0 * int(angle / 180.0) == 0) error("Inf");
        else if (abs(angle - 180.0 * int(angle / 180.0)) == 90.0) return 0;
        else return cos(spe.value) / sin(spe.value);
    }
    else if (s == "sind") {
        if (spe.value - 180.0 * int(spe.value / 180.0) == 0) return 0;
        else return sin(spe.value * dict["pi"] / 180.0);
    }
    else if (s == "cosd") {
        if (abs(spe.value - 180.0 * int(spe.value / 180.0)) == 90.0) return 0;
        else return cos(spe.value * dict["pi"] / 180.0);
    }
    else if (s == "tand") {
        if (spe.value - 180.0 * int(spe.value / 180.0) == 0) return 0;
        else if (abs(spe.value - 180.0 * int(spe.value / 180.0)) == 90.0) error("Inf");
        else return sin(spe.value * dict["pi"] / 180) / cos(spe.value * dict["pi"] / 180);
    }
    else if (s == "cotd") {
        if (spe.value - 180.0 * int(spe.value / 180.0) == 0) error("Inf");
        else if (abs(spe.value - 180.0 * int(spe.value / 180.0)) == 90.0) return 0;
        else return cos(spe.value * dict["pi"] / 180) / sin(spe.value * dict["pi"] / 180);
    }
    else error("Wrong trignometry"); return 1;
}

/* Generate a random number */
double random_generator() {
    random_device rd; 
    mt19937 gen(rd()); 
    gen.seed(chrono::high_resolution_clock::now().time_since_epoch().count()); 

    Token spe = statement();
    double lb = spe.value;
    if (spe.key == comma) {
        spe = statement();
        double ub = spe.value; 
        uniform_real_distribution <double> urd(lb, ub); 
        return urd(gen);
    }
    else {
        uniform_real_distribution <double> urd(0, lb); 
        return urd(gen);
    } 
}

/* Switch special operations */
double switch_operation(string s) {
    if (s == "pi" || s == "e") return dict[s];
    else if (s == "sqrt") return square_root();
    else if (s.find("log") != string::npos) return logarithm(s);
    else if (s == "sin" || s == "cos" || s == "tan" || s == "cot" || s == "sind" || s == "cosd" || s == "tand" || s == "cotd") return trigonometry(s);
    else if (s == "rand") return random_generator();
    else error("Cannot find this special operation"); return 1;
}

//------------------------------------------------------------------------------

// This section defines specific functions to deal with Tokens

Token_stream ts;  // declare a Token_stream for function usage
Token expression();  // primary() needs to know what is expression

/* Deal with numbers, () and unary signs */
double primary() {
    Token t = ts.get_Token();
    switch (t.key) {
        case '(': {  // what's after '(' must be a number  
            Token temp = statement();
            double d = temp.value;  // read the number
            t = ts.get_Token(); 
            if (t.key != ')' && t.key != ',') error("')' expected");
            else if (t.key == ',') ts.putback(t);  // put back so that expression() knows when to return the comma flag
            return d;
        }
        case ',': {
            Token temp = statement();
            double d = temp.value;
            t = ts.get_Token();
            if (t.key != ')') error("')' expected");
            return d;
        }
        case number: { 
            return t.value;
        }
        case '+': {  // unary plus
            t = ts.get_Token();
            if (t.key == '+' || t.key == '-') {
                error("more than 1 consecutive '+' or '-' makes no sense");
            }
            else {
                ts.putback(t);
                return primary();
            }
        }
        case '-': {  // unary minus    
            t = ts.get_Token();
            if (t.key == '+' || t.key == '-') {
                error("more than 1 consecutive '+' or '-' makes no sense");
            }
            else {
                ts.putback(t);
                return -primary();
            }
        }
        case special: {
            double val = switch_operation(t.name);
            return val;
        }
        default: {  // inputs must begin with a primary
            ts.putback(t);
            error("primary expected");
            return 0;
        }
    }
}

/* deal with *, / and % */
double term() {
    double left = primary();  // left = number waiting to be operated
    Token t = ts.get_Token(); 
    Token temp;  // used for power operation
    Token temp_rem;  // used for remainder with power operation

    while(true) {
        switch (t.key) {
            case '*': {
                double d = primary(); // always do primary before term
                left *= d;  
                temp = {t.key, d};
                t = ts.get_Token();
                break;  // see what's next operator
            }
            case '/': {
                double d = primary();
                if (d == 0) error("Inf");
                left /= d;
                temp = {t.key, d};
                t = ts.get_Token();
                break;
            }
            case '%': {
                double d = primary();
                if (d == 0) error("Inf");
                temp_rem = {t.key, left};
                left = left - d * int(left / d);
                temp = {t.key, d};
                t = ts.get_Token();
                break;
            }
            case '^': {
                double d = primary();  // exponential
                switch (temp.key) {
                    case '*': {
                        left /= temp.value;  // undo * 
                        left *= pow(temp.value, d);  // do power first then *
                        break;
                    }
                    case '/': {
                        left *= temp.value;
                        left /= pow(temp.value, d);
                        break;
                    }
                    case '%': {
                        left = temp_rem.value;
                        left = left - pow(temp.value, d) * int(left / pow(temp.value, d));
                        break;
                    }
                    default: {
                        left = pow(left, d);
                        break;
                    }
                }
                t = ts.get_Token();
                break;
            }
            case '!': {
                if ((temp.key == '*' || temp.key == '/' || temp.key == '%') && (temp.value - int(temp.value) != 0)) error("Only integers have factorial");  // a*b!, b is not integer
                else if (temp.key != '*' && temp.key != '/' && temp.key != '%' && left - int(left) != 0) error("Only integers have factorial");  // c!, c is not integer

                switch (temp.key) {
                    case '*': {
                        left /= temp.value;  // undo * 
                        left *= factorial(temp.value);  // do ! first then *
                        break;
                    }
                    case '/': {
                        left *= temp.value;
                        left /= factorial(temp.value);
                        break;
                    }
                    case '%': {
                        left = temp_rem.value;
                        left = left - factorial(temp.value) * int(left / factorial(temp.value));
                        break;
                    }
                    default: {
                        left = factorial(left);
                        break;
                    }
                }
                t = ts.get_Token();
                break;
            }
            default: {
                ts.putback(t); // do term first, then do expression
                return left;
            }
        }
    }
}

/* deal with + and - */
Token expression() {
    double left = term();  
    Token t = ts.get_Token(); 

    while(true) {    
        switch(t.key) {
            case '+': {
                left += term();  // always do term before expression
                t = ts.get_Token();
                break;
            }
            case '-': {
                left -= term(); 
                t = ts.get_Token();
                break;
            }
            case ',': { 
                ts.putback(t);
                return Token(comma, left);  // return a Token with the "comma" flag
            }
            default: {
                ts.putback(t);  
                return Token(caled, left);  // update result.key
            }
        }
    }
}

/* Deal with variables */
Token statement() {
    Token t = ts.get_Token();

    switch (t.key) {
        case define: {  // if define a new variable
            Token temp = statement();
            dict[t.name] = temp.value;
            cout << "Variable \"" << t.name << "\" is defined\n";
            cout << display_line(100);
            return 0; break;
        }
        default: {
            ts.putback(t);
            return expression();
        }
    }
}

/* Read inputs, calculates then print */
void calculate() {
    Token result;
    while (cin) {
        try {
            Token t = ts.get_Token();
            if (t.key == quit) break;
            if (t.key == print && result.key == caled) {  // print calculation result
                dict["ans"] = result.value;  // so that ans can be used like MATLAB
                cout << result.value << '\n';  
                cout << display_line(100);
                result.key = 'd';
            }
            else if (t.key == print && result.key != caled) {}  // if user keeps pressing Enter for fun or habit
            else {
                ts.putback(t);
                result = statement();
            }
        }
        catch (exception &e) {
            cerr << "Error: " << e.what() << '\n';
            ts.reset();
        }
    }
}

//------------------------------------------------------------------------------

int main() try
{
    cout << "Welcome to Stroustrup-n33 calculator (version 1.0), the syntaxes should be intuition-friendly and MATLAB-alike." << endl;
    cout << "1. Available operators are +, -, /, *, %, ^, !, sqrt." << endl;
    cout << "2. Define variable format: s = 1 or s = d, space can be ignored." << endl;
    cout << "3. Logarithm syntax: log3(10), where 3 is the base and 10 is exponent; To calculate natural log, use log or loge." << endl;
    cout << "4. Trigonometry syntax: same as MATLAB." << endl;
    cout << "5. Random number format: rand(10) uniformly from 0-10, rand(1, 4) uniformaly from 1-4." << endl;
    cout << "6. Add parentheses when combining special operations with ^ or !." << endl;
    cout << "7. Complex numbers are returned as nan." << endl;
    cout << "8. Currently support 2 constants: pi, e." << endl;
    cout << display_line(100) << display_line(100);

    cout.precision(7);
    init_const();

    while (true) {
        calculate();
        return 0;
    }
}
catch (exception &e)
{
    cerr << "Error: " << e.what() << '\n';
    cin.ignore(numeric_limits<streamsize>::max());  // keep window open
    return 1;
}
catch (...)
{
    cerr << "Oops: unknown exception!\n";
    cin.ignore(numeric_limits<streamsize>::max());
    return 2;
}