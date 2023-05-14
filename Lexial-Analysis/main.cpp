#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <fstream>
#include <stdio.h>
#include <iomanip>

using namespace std;

/*
    |单词符号       |  种别码      | 助记符      |   内码值     |
    | ----------- | ----------- | ----------- | ----------- |
    | begin       | 01          | $BEGIN      |             |
    | end         | 02          | $END        |             |
    | integer     | 03          | $INTEGER    |             |
    | if          | 04          | $IF         |             |
    | then        | 05          | $THEN       |             |
    | else        | 06          | $ELSE       |             |
    | function    | 07          | $FUCTION    |             |
    | read        | 08          | $READ       |             |
    | write       | 09          | $WRITE      |             |
    | 标识符       | 10          | $ID         |             |
    | 常数         | 11          | $INT        |             |
    | =           | 12          | $EQ         |             |
    | <>          | 13          | $NE         |             |
    | <=          | 14          | $LE         |             |
    | <           | 15          | $LT         |             |
    | >=          | 16          | $GE         |             |
    | >           | 17          | $GT         |             |
    | -           | 18          | $SUB        |             |
    | *           | 19          | $MUL        |             |
    | :=          | 20          | $ASSIGN     |             |
    | (           | 21          | $LPAR       |             |
    | )           | 22          | $RPAR       |             |
    | ;           | 23          | $SEM        |             |
    |EOLN         | 24          | $EOLN       |             |
    |EOF          | 25          | $EOF        |             |

*/

// 全局变量
string keywords[9] = {"begin", "end", "integer", "if", "then", "else", "function", "read", "write"};
string operate[9] = {"=", "<>", "<=", "<", ">=", ">", "-", "*", ":="};
string op = "=<>-*:";
string delimiter[3] = {"(", ")", ";"};
enum
{
    $ID,
    $INT,
    $KEYWORDS,
    $OPERATE,
    $DELIMITER,
    $ERROR,
    $SYNAX_ERROR,  // 语法错误
    $COLON_ERROR,  // 冒号错误
    $LENGTH_ERROR, // 长度错误
    $EOF,
    $SPACE,
    $EOLN
};
map<string, int> mnemonicList;
string code_file = "./code_file";
string code_file_test = "./code_file_test";
string code_out = "./code_out.dyd";
string code_erro = "./code_erro.err";

string code;      // 去掉空白字符后的输入
string tempToken; // 存储当前的token
int line = 1;     // 行号

struct Token
{
    string mnemonic; // 助记符
    int value;       // 值
    // 构造函数
    Token(string mnemonic, int value)
    {
        this->mnemonic = mnemonic;
        this->value = value;
    }
};

vector<Token> tokenList;

// 指针，取字符
int pos = 0;

// 从文件中读取代码, 不要去除换行符
void readCode()
{
    // ifstream in(code_file);
    ifstream in(code_file_test);
    string line;
    while (getline(in, line))
    {
        code += line + '\n';
    }
    in.close();
}

// 初始化种别码
void init()
{
    mnemonicList["begin"] = 1;
    mnemonicList["end"] = 2;
    mnemonicList["integer"] = 3;
    mnemonicList["if"] = 4;
    mnemonicList["then"] = 5;
    mnemonicList["else"] = 6;
    mnemonicList["function"] = 7;
    mnemonicList["read"] = 8;
    mnemonicList["write"] = 9;
    mnemonicList["$ID"] = 10;
    mnemonicList["$INT"] = 11;
    mnemonicList["="] = 12;
    mnemonicList["<>"] = 13;
    mnemonicList["<="] = 14;
    mnemonicList["<"] = 15;
    mnemonicList[">="] = 16;
    mnemonicList[">"] = 17;
    mnemonicList["-"] = 18;
    mnemonicList["*"] = 19;
    mnemonicList[":="] = 20;
    mnemonicList["("] = 21;
    mnemonicList[")"] = 22;
    mnemonicList[";"] = 23;
    mnemonicList["$EOLN"] = 24;
    mnemonicList["$EOF"] = 25;
}

// 取下一个字符, 若存在则返回，不存在则返回'/0'
char peek()
{
    if (pos + 1 <= code.length())
    {
        return code[pos + 1];
    }
    else
        return '\0';
}

// 判断是否是数字
bool isDight(char ch)
{
    if (ch >= '0' && ch <= '9')
        return true;
    else
        return false;
}

// 判断是否是字母
bool isLetter(char ch)
{
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        return true;
    else
        return false;
}

// 查找关键字
bool isKeywords(string str)
{
    for (int i = 0; i < 9; i++)
    {
        if (str == keywords[i])
        {
            return true;
        }
    }

    return false;
}

// 查找界符
bool isDelimiter(char ch)
{
    for (int i = 0; i < 3; i++)
    {
        if (ch == delimiter[i][0])
        {
            return true;
        }
    }

    return false;
}

bool isOP(char ch)
{
    for (int i = 0; i < 7; i++)
    {
        if (ch == op[i])
        {
            return true;
        }
    }

    return false;
}

// 查找运算符
bool isOperate(string str)
{
    for (int i = 0; i < 9; i++)
    {
        if (str == operate[i])
        {
            return true;
        }
    }

    return false;
}

// 词法分析器
int LexAnalyze(char ch)
{
    // 空白字符
    if (ch == '\t' || ch == '\r' || ch == ' ')
    {
        return $SPACE;
    }

    // 换行符
    if (ch == '\n')
    {
        return $EOLN;
    }

    // 以数字开头
    if (isDight(ch))
    {
        tempToken = ch;
        while (isDight(peek()))
        {
            tempToken += peek();
            pos++;
        }
        return $INT;
    }

    // 以字母开头
    if (isLetter(ch))
    {
        tempToken = ch;
        while (isLetter(peek()) || isDight(peek()))
        {
            tempToken += peek();
            pos++;
        }
        if (isKeywords(tempToken))
        {
            return $KEYWORDS;
        }
        else
        {
            if (tempToken.length() > 16)
            {
                return $LENGTH_ERROR;
            }
            return $ID;
        }
    }

    // 以运算符开头
    if (isOP(ch))
    {
        tempToken = ch;
        // cout << tempToken << endl;
        char nextchar = peek();
        // cout << nextchar << endl;
        if (isOP(nextchar))
        {
            if (isOperate(tempToken + nextchar))
            {
                tempToken += nextchar;
                pos++;
                return $OPERATE;
            }
            // 判断tempToken是否为:
            else if (tempToken == ":")
            {
                return $COLON_ERROR;
            }
            else
            {
                return $OPERATE;
            }
        }
        else
        {
            if (isOperate(tempToken))
            {
                return $OPERATE;
            }
            else
            {
                return $COLON_ERROR;
            }
        }
    }

    // 以界符开头
    if (isDelimiter(ch))
    {
        tempToken = ch;
        return $DELIMITER;
    }

    return $SYNAX_ERROR;
}

int read_next()
{
    int type = LexAnalyze(code[pos]);

    // 如果是空白字符，则跳过
    while (type == $SPACE && pos < code.length())
    {
        ++pos;
        type = LexAnalyze(code[pos]);
    }

    // 如果到达文件末尾，则返回$EOF
    if (pos >= code.length())
    {
        tokenList.push_back(Token("EOF", mnemonicList["$EOF"]));
        return $EOF;
    }

    ++pos;

    // 如果是换行符
    if (type == $EOLN)
    {
        tokenList.push_back(Token("EOLN", mnemonicList["$EOLN"]));
        line++;
        return type;
    }

    // 如果是语法错误
    if (type == $SYNAX_ERROR)
    {

        ofstream fout(code_erro, ios::app);
        fout << "Line " << line << " U "
             << "Syntax error"
             << ": Grammatical rules are not satisfied near " << tempToken << endl;

        fout.close();
        return $ERROR;
    }

    // 如果是冒号错误
    if (type == $COLON_ERROR)
    {
        // cout << type << endl;
        ofstream fout(code_erro, ios::app);
        fout << "Line " << line << " U "
             << "Colon error"
             << ": The use of colons does not satisfy the rules" << endl;
        fout.close();
        return $ERROR;
    }

    // 如果是长度错误
    if (type == $LENGTH_ERROR)
    {
        ofstream fout(code_erro, ios::app);
        fout << "Line " << line << " U "
             << "Length error"
             << ": The length of the " << tempToken << " exceeds the limit" << endl;
        fout.close();
        return $ERROR;
    }

    // 如果是数字
    if (type == $INT)
    {
        tokenList.push_back(Token(tempToken, mnemonicList["$INT"]));
        return type;
    }

    // 如果是标识符
    if (type == $ID)
    {
        tokenList.push_back(Token(tempToken, mnemonicList["$ID"]));
        return type;
    }

    // 如果是关键字或界符或运算符
    if (type == $KEYWORDS || type == $OPERATE || type == $DELIMITER)
    {
        tokenList.push_back(Token(tempToken, mnemonicList[tempToken]));
        return type;
    }

    return $ERROR;
}

int main()
{
    // getnbc(test);
    readCode();

    init();
    while (pos <= code.length())
    {
        int flag = read_next();
        if (flag == $EOF)
            break;
        else if (flag == $ERROR)
        {
            tokenList.push_back(Token("ERROR", 26));
        }
    }

    for (auto t : tokenList)
    {

        // 将上述输出写入文件
        ofstream fout(code_out, ios::app);
        fout << left << setw(16) << t.mnemonic << " " << setw(2) << t.value << endl;
        // if (t.value == 24)
        //     fout << endl;
        // else
        //     fout << " ";
        fout.close();
    }
    return 0;
}
