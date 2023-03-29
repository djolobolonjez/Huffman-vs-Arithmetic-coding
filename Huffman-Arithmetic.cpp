#include <iostream>
#include <fstream>
#include <queue>
#include <map>
#include <cmath>
#include <vector>
#include <filesystem>
#include <sstream>

namespace HuffmanArithmetic {

    struct Symbol {
        std::string symb;
        double p;

        Symbol* left = nullptr, *right = nullptr;

        Symbol(std::string _symb, double _p) : symb(_symb), p(_p) { } 
        bool operator<(const Symbol& rhs) const { return p > rhs.p; }
    };

    struct Comparator {
        bool operator() (Symbol* lhs, Symbol* rhs) {
            return *lhs < *rhs;
        }
    };

    
    struct EncodingArgs {
        std::priority_queue<Symbol*, std::vector<Symbol*>, Comparator> pq;
        std::map<std::string, std::pair<int, double>> symbolMap;
        Symbol* treeRoot;
        double arithmeticCodeInterval = 1;
        int k = 0;
    };

    void huffmanRatio(EncodingArgs& args, Symbol* root, std::vector<int>& codes, int limit, int& sum) {
        if (!root) {
            return;
        }
        if (root->left) {
            codes[limit] = 0;
            huffmanRatio(args, root->left, codes, limit + 1, sum);
        }
        if (root->right) {
            codes[limit] = 1;
            huffmanRatio(args, root->right, codes, limit + 1, sum);
        }

        if (root->left == nullptr && root->right == nullptr) {
            sum += (limit * args.symbolMap[root->symb].first);
        }
    }

    void createTree(EncodingArgs& args) {
        if (args.pq.size() == 1) {
            args.treeRoot = args.pq.top();
        }
        else {
            while (args.pq.size() != 1) {
                Symbol* left = args.pq.top(); args.pq.pop();
                Symbol* right = args.pq.top(); args.pq.pop();
                Symbol* parent = new Symbol("#", left->p + right->p);
              
                parent->left = left;
                parent->right = right;
                args.pq.push(parent);
            }
            
            args.treeRoot = args.pq.top();
        }
    }
    
    void initHuffman(std::string sequence, EncodingArgs& args) {
        for (char c : sequence) {
            std::string s;
            s += c;
            if (args.symbolMap.find(s) != args.symbolMap.end()) {
                args.symbolMap[s].first++;
            } 
            else {
                args.symbolMap[s] = {1, 0.};
            }
        }
        
        for (auto iter = args.symbolMap.begin(); iter != args.symbolMap.end(); iter++) {
            double p = (double)iter->second.first / sequence.length();
            iter->second.second = p;
            args.pq.push(new Symbol(iter->first, p));
        }

        createTree(args);
    }   

    void calculateInterval(EncodingArgs& args) {
        for (auto iter = args.symbolMap.begin(); iter != args.symbolMap.end(); iter++) {
            args.arithmeticCodeInterval *= pow(iter->second.second, iter->second.first);
        }
    }

    void getNumberOfBits(EncodingArgs& args) {
        args.k = ceil(log2(1/args.arithmeticCodeInterval)) + 1;
    }

    int bitsPerSymbol(int n) {
        int lg = log2(n);
        if (pow(2, lg) >= n) {
            return lg;
        }

        return lg + 1;
    }

    std::string huffmanResults(std::string sequence, EncodingArgs& args) {
        std::vector<int> codes;
        codes.reserve(args.symbolMap.size() - 1);

        int statistical = 0;
        huffmanRatio(args, args.treeRoot, codes, 0, statistical);
        int nonStatistical = bitsPerSymbol(args.symbolMap.size()) * sequence.length();

        std::stringstream ssoutput;

        double ro = (double)nonStatistical / statistical;
        ssoutput << "HUFFMAN CODE COMPRESSION RATIO: " << ro << std::endl;
        return ssoutput.str();
    }

    std::string arithmeticResults(std::string sequence, EncodingArgs& args) {
        std::stringstream ssoutput;
        int nonStatistical = bitsPerSymbol(args.symbolMap.size()) * sequence.length();
        double ro = (double)nonStatistical / args.k;

        ssoutput << "ARITHMETIC CODE COMPRESSION RATIO: " << ro << std::endl;
        return ssoutput.str();
    }

    void readInput(std::filesystem::path dir, std::filesystem::path wdir) {
        
        int outNum = 1;
        std::filesystem::path outdir = wdir / "out";
        if (!std::filesystem::create_directory(outdir)) {
            std::cerr << "Error: Unable to create directory" << std::endl;
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (!entry.is_directory()) {
                std::filesystem::path filepath = entry.path();  
                std::ifstream file;

                file.open(filepath);
                if (file.is_open()) {
                    std::string sequence;
                    EncodingArgs args;

                    getline(file, sequence);    
                    initHuffman(sequence, args);
                    calculateInterval(args);
                    getNumberOfBits(args);

                    std::string ofname = "out_" + std::to_string(outNum++) + ".txt";
                    std::filesystem::path ofpath = outdir / ofname;
                    std::ofstream outfile(ofpath);
                    outfile << huffmanResults(sequence, args);
                    outfile << arithmeticResults(sequence, args);
                }
            }
        } 
    }
}

int main() {
    std::filesystem::path wd = std::filesystem::current_path();
    std::filesystem::path dir = wd.parent_path();
    std::string dirstring = "input";

    std::filesystem::path dirpath = dir / dirstring;
    HuffmanArithmetic::readInput(dirpath, dir);
    return 0;
}