/*

fslang Compiler v1.0.0
Copyright (c) Alex Baldwin 2019.

fslang is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License,
version 2 exclusively, as published by the Free Software Foundation.

fslang is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with fslang. If not, see:
https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt/

*/

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <cstring>
#include <string>
#include <fstream>
#include <vector>

//drl = data resource locator

int indexOfChar(char cdet,std::string str) {
    int fstioc = 0;
    char cmp;
    for (int i=0;i<str.length();i++) {
        cmp = str.at(i);
        if (cmp == cdet) {
            return fstioc;
        } else {
            fstioc++;
        }
    }
    return -1;
}

int lastIndexOfChar(char cdet,std::string str) {
    int fstioc = 0;
    int lastfstioc = 0;
    char cmp;
    for (int i=0;i<str.length();i++) {
        cmp = str.at(i);
        if (cmp == cdet) {
            lastfstioc = fstioc;
            fstioc++;
        } else {
            fstioc++;
        }
    }
    return lastfstioc;
}

int indexOfCharNotEscaped(char cdet,std::string str) {
    int fstioc = 0;
    char cmp;
    for (int i=0;i<str.length();i++) {
        cmp = str.at(i);
        if (cmp == cdet) {
            if (i > 0) {
                if (str.at(i-1) == 0x5C) {
                    //character was escaped
                    fstioc++;
                    continue;
                }
            }
            return fstioc;
        } else {
            fstioc++;
        }
    }
    return -1;
}

int indexOfFuncCharNotEscaped(std::string str) {
    int fstioc = 0;
    char cmp;
    for (int i=0;i<str.length();i++) {
        cmp = str.at(i);
        if ((cmp == 0x21) || (cmp == 0x44) || (cmp == 0x2B) || (cmp == 0x2D) || (cmp == 0x28) || (cmp == 0x3D) || (cmp == 0x3C) || (cmp == 0x3E) || (cmp == 0x3B)) {
            if (i > 0) {
                if (str.at(i-1) == 0x5C) {
                    //character was escaped
                    fstioc++;
                    continue;
                }
            }
            return fstioc;
        } else {
            fstioc++;
        }
    }
    return -1;
}

std::vector<std::string> drlMapIn;
std::vector<std::string> drlMapOut;

void addDRLtoMap(std::string drl) {
    bool drlAlreadyExists = false;
    for (int i = 0; i < drlMapIn.size(); i++) {
        if (drlMapIn[i] == drl) {
            drlAlreadyExists = true;
            break;
        }
    }
    if (!drlAlreadyExists) {
        drlMapIn.push_back(drl);
        drlMapOut.push_back("variableN" + std::to_string(drlMapIn.size()));
    }
}

std::string getOutputNameFromDRL(std::string drl) {
    for (int i = 0; i < drlMapIn.size(); i++) {
        if (drlMapIn[i] == drl) {
            return drlMapOut[i];
        }
    }
    return "null";
}

std::string fslangTypeToCppType(std::string typin) {
    if(typin == "string")    {return "std::string";}
    if(typin == "int")       {return "int";}
    if(typin == "float")     {return "float";}
    if(typin == "double")    {return "double";}
    if(typin == "bool")      {return "bool";}
    if(typin == "void")      {return "void";}
    return "auto";
}

std::string filePrependVariables = "";

std::string translateSegment(std::string src,std::string currentScope,int offsetLine,int offsetPosition) {
    int lineCounter = offsetLine;
    int positionCounter = offsetPosition;
    std::string outputString;
    std::cout << "INPUT: " << std::endl << src << std::endl << std::endl;
    while (src.length() > 0){
        try {

            std::cout << "v Processing line " << std::to_string(lineCounter) << std::endl;
            std::cout << src.substr(0,indexOfChar(0x0A,src)) << std::endl;
            int fstnsp = 0;
            for (int i=0;i<src.length();i++) {
                char cmp = src.at(i);
                if ((cmp >= 0x00) && (cmp <= 0x20)) {
                    fstnsp++;
                    if (cmp == 0x20) {
                        positionCounter++;
                    }
                    if (cmp == 0x09) {
                        positionCounter+=4;
                    }
                    if (cmp == 0x0A) {
                        lineCounter++;
                        positionCounter=1;
                    }
                } else {
                    break;
                }
            }
            src = src.substr(fstnsp);
            if (src.substr(0, 2) == "--") {
                outputString = outputString + "//" + src.substr(2,indexOfChar(0x0A,src)-1);
                src = src.substr(indexOfChar(0x0A,src)+1);
                lineCounter++;
                positionCounter=1;
                continue;
            }
            if ((src.substr(0, 1) == "/") || src.substr(0, 2) == "./") {
                int endMrk = indexOfFuncCharNotEscaped(src);
                bool localVariable = false;
                std::string drl = src.substr(0,endMrk);
                if (src.substr(0, 1) == ".") {
                    drl = currentScope + drl.substr(2);
                    localVariable = true;
                }
                src = src.substr(endMrk);
                while (src.substr(0, 1) == " ") {
                    src = src.substr(1);
                    positionCounter++;
                }
                addDRLtoMap(drl);
                std::string drlFunc = src.substr(0,indexOfChar(0x0A,src));
                std::string drlType = drl.substr(lastIndexOfChar(0x2E,drl)+1);
                while (drlType.substr(drlType.length()-1, drlType.length()) == " ") {
                    drlType = drlType.substr(0, drlType.length()-1);
                }

                std::cout << "DRL addr " << drl << std::endl;
                std::cout << "DRL type " << drl.substr(lastIndexOfChar(0x2E,drl)+1) << std::endl;
                std::cout << "DRL func " << drlFunc << std::endl;
                if (drlFunc == ";") {
                    std::cout << "DRL definition @ ln " << std::to_string(lineCounter) << " cl " << std::to_string(positionCounter) << " : " << drl << std::endl;
                    if (localVariable) {
                        outputString = outputString + fslangTypeToCppType(drlType) + " " + getOutputNameFromDRL(drl) + ";\n";
                    } else {
                        filePrependVariables = filePrependVariables + fslangTypeToCppType(drlType) + " " + getOutputNameFromDRL(drl) + ";\n";
                    }
                    src = src.substr(1);
                    lineCounter++;
                    positionCounter=1;
                    continue;
                }
                if (drlFunc.substr(0, 1) == "=") {
                    std::cout << "DRL assignment @ ln " << std::to_string(lineCounter) << " cl " << std::to_string(positionCounter) << " : " << drl << std::endl;
                    drlFunc = drlFunc.substr(1);
                    positionCounter++;
                    for (int i=0;i<drlFunc.length();i++) {
                        char cmp = drlFunc.at(i);
                        if (cmp == 0x20) {
                            drlFunc = drlFunc.substr(1);
                            positionCounter++;
                        } else {
                            break;
                        }
                    }
                    std::cout << "> " << drlFunc << std::endl;
                    if (drlFunc.substr(0,8) == "function") {
                        std::cout << "Function" << std::endl;
                        int openBrackets = 0;
                        int closeBrackets = 0;
                        int endOfArgs = 0;
                        for (int i=0;i<src.length();i++) {
                            char cmp = src.at(i);
                            if (cmp == 0x3B) { // ;
                                std::cerr << "ERROR unexpexted end of line @ ln " << std::to_string(lineCounter) << " cl " << std::to_string(positionCounter+i) << " : " << drl << std::endl;
                            }
                            if (cmp == 0x28) { // (
                                std::cout << "OPB" << std::endl;
                            }
                            if (cmp == 0x2C) { // (
                                std::cout << "CMA" << std::endl;
                            }
                            if (cmp == 0x29) { // (
                                std::cout << "CLB" << std::endl;
                            }
                            if (openBrackets == closeBrackets) {
                                endOfArgs = i;
                                break;
                            }
                        }
                        continue;
                    } else {
                        std::cout << "Non Function" << std::endl;
                        if (localVariable) {
                            outputString = outputString + fslangTypeToCppType(drlType) + " " + getOutputNameFromDRL(drl) + " = " + drlFunc + "\n";
                        } else {
                            filePrependVariables = filePrependVariables + fslangTypeToCppType(drlType) + " " + getOutputNameFromDRL(drl) + " = " + drlFunc + "\n";
                        }
                        continue;
                    }
                }
                if (drlFunc.substr(0, 1) == "(") {
                    std::cout << "DRL function call @ ln " << std::to_string(lineCounter) << " cl " << std::to_string(positionCounter) << " : " << drl << std::endl;
                    int openBrackets = 0;
                    int closeBrackets = 0;
                    int endOfArgs = 0;
                    for (int i=0;i<src.length();i++) {
                        char cmp = src.at(i);
                        if (cmp == 0x28) {
                            std::cout << "OPB" << std::endl;
                            openBrackets++;
                        }
                        if (cmp == 0x29) {
                            std::cout << "CLB" << std::endl;
                            closeBrackets++;
                        }
                        if (openBrackets == closeBrackets) {
                            //std::cout << "EOA: " << std::to_string(endOfArgs) << std::endl;
                            endOfArgs = i;
                            break;
                        }
                    }
                    std::cout << "Evaluating args" << std::endl;
                    positionCounter+=drl.length()+1;
                    std::string fnargs = translateSegment(src.substr(1,endOfArgs-1),currentScope,lineCounter,positionCounter);
                    std::cout << "Args = " << fnargs << std::endl;
                    continue;
                }
                std::cerr << "DRL string found but no operation performed @ ln " << std::to_string(lineCounter) << " cl " << std::to_string(positionCounter) << " : " << drl << std::endl;
                continue;
            }

            if (indexOfChar(0x3B,src) >= 0) {
                //std::cout << indexOfChar(0x3B,src) << std::endl;
                src = src.substr(indexOfChar(0x3B,src)+1);
                lineCounter++;
                positionCounter=1;
            } else {
                src = "";
                break;
            }
        } catch (const std::exception& e) {
            std::cout << "Internal compiler exception\n";
            std::cerr << e.what() << std::endl;
        }
    }
    return outputString;
}

int fslangTranslate(std::string pathIn,std::string pathOut) {
    std::ofstream imf;
    std::ifstream srcf;
    imf.open(pathOut);
    srcf.open(pathIn);
    std::stringstream buffer;
    buffer << srcf.rdbuf();
    std::string src = buffer.str();
    std::string currentScope = "/";

    std::cout << "Adding c++ imports\n";
    imf << "#include <iostream>\n";
    imf << "#include <cstdlib>\n";
    imf << "#include <sstream>\n";
    imf << "#include <cstring>\n";
    imf << "#include <string>\n";
    imf << "#include <fstream>\n";
    std::cout << "Adding helper functions\n";
    std::cout << "Translating main code body\n";
    std::string codeBody = translateSegment(src,currentScope,1,1);
    std::cout << filePrependVariables;
    imf << filePrependVariables;
    imf << codeBody;
    imf.close();
    return 0;
}

int gppFinalCompile(std::string pathIn,std::string pathOut) {
    std::stringstream stream;
    stream << "g++ " << "-o " << pathOut << pathIn;
    return system(stream.str().c_str());
}

int main(int argc, char** argv) {
    std::cout << "fslang Compiler v1.0.0\nCopyright (c) Alex Baldwin 2019.\n\n";

    //std::cout << "You have entered " << argc << " arguments:" << "\n";
    std::string inputFile = "";
    std::string outputFile = "";

	for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];
		if (arg == "-o") {
            i++;
            std::string arg2 = argv[i];
            outputFile = arg2;
            continue;
        }
        inputFile = arg;
    }

    std::cout << inputFile << " --> ";
    std::cout << outputFile << std::endl;

    std::cout << "Translating...\n";
    fslangTranslate(inputFile,inputFile + ".cpp");
    std::cout << "Finished translate step\n";

    return 0;
}
