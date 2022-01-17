#include "rule.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

// std::vector<std::string> split(const std::string &str) {
//     std::vector<std::string> result;
//     size_t pos = 0, last_pos = 0;
//     while ((pos = str.find_first_of('/', last_pos)) != std::string::npos) {
//         result.push_back(str.substr(last_pos, pos - last_pos));
//         last_pos = pos + 1;
//     }
//     result.push_back(str.substr(last_pos));
//     return result;
// }

class path_filter {
    std::vector<rule> rules;

  public:
    path_filter() = default;
    path_filter(const char *path) {
        std::ifstream ifs{path};
        if (!ifs) {
            std::cout << "failed to open file: " << path << std::endl;
            exit(1);
        }
        std::regex re("(-|\\+)\\s+(.*)");

        for (int i = 1; !ifs.eof(); i++) {
            std::string line;
            std::getline(ifs, line);
            std::smatch m;

            if (std::regex_match(line, m, re)) {
                rule::rule_type rule_type;
                std::string rule_str = m.str(1),
                            pattern = m.str(2);
                if (rule_str == "-") {
                    rule_type = rule::RULE_TYPE_EXCLUDE;
                } else if (rule_str == "+") {
                    rule_type = rule::RULE_TYPE_INCLUDE;
                } else {
                    std::cout << "Unsupported filter rule" << std::endl;
                    exit(1);
                }

                rules.push_back(rule{pattern, rule_type});
            }
        }
    }

    bool contains(const char *const path) {
        // printf("%s\n", path);
        for (auto &&_rule : rules) {
            if (_rule.match(path)) {
                switch (_rule.type) {
                case rule::RULE_TYPE_EXCLUDE:
                    return false;
                case rule::RULE_TYPE_INCLUDE:
                    return true;
                default:
                    break;
                }
            }
        }
        return true;
    }
};
