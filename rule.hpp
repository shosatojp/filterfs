#include "segment.hpp"
#include <stdexcept>
#include <string>
#include <vector>

class rule {
  public:
    enum rule_path_type {
        RULE_PATH_TYPE_ABSOLUTE,
        RULE_PATH_TYPE_RELATIVE,
    };

    enum rule_type {
        RULE_TYPE_INCLUDE,
        RULE_TYPE_EXCLUDE,
    };

    //   private:
    rule_path_type path_type;
    rule_type type;
    std::vector<segment> segments;

  public:
    rule(const std::string &_str, rule_type type) : type(type) {
        if (_str.length() == 0) {
            throw std::runtime_error("rule not found");
        }

        std::string str;
        if (_str[0] == '/') {
            path_type = RULE_PATH_TYPE_ABSOLUTE;
            // remove first /
            str = _str.substr(1);
        } else {
            path_type = RULE_PATH_TYPE_RELATIVE;
            str = _str;
        }
        // remove last /
        if (str[str.length() - 1] == '/') {
            str = str.substr(0, str.length() - 1);
        }

        size_t pos = 0, last_pos = 0;
        while ((pos = str.find_first_of('/', last_pos)) != std::string::npos) {
            auto seg = str.substr(last_pos, pos - last_pos);
            segments.push_back(segment{seg});

            last_pos = pos + 1;
        }
        auto seg = str.substr(last_pos);
        segments.push_back(segment{seg});
    }

    bool match(const char *path) const {
        switch (path_type) {
        case RULE_PATH_TYPE_ABSOLUTE:
            return match_rule_absolute(path);
        case RULE_PATH_TYPE_RELATIVE:
            return match_rule_relative(path);
        default:
            throw std::runtime_error("Unsupported rule type.");
        }
    }

  private:
    inline bool match_rule_absolute(const char *const _path) const {
        if (_path[0] == '\0') {
            std::cerr << "path is zero length" << std::endl;
            return false;
        }

        const char *path;
        // remove first /
        if (_path[0] == '/') {
            path = _path + 1;
        } else {
            path = _path;
        }

        bool matched = false;
        size_t pos = 0, last_pos = 0,
               rule_segment_index = 0,
               path_segment_index = 0;
        for (const char *c = path;; pos++, c++) {
            if (*c == '/' || (*c == '\0' && *(c - 1) != '/')) {
                if (path_segment_index > segments.size() - 1) {
                    // len(path) > len(rule)
                    return false;
                }

                if (rule_segment_index < segments.size()) {
                    segment::segment_match_type match = segments[rule_segment_index].match(pos - last_pos, path + last_pos);
                    switch (match) {
                    case segment::SEGMENT_MATCH_TRUE:
                        matched = true;
                        break;
                    case segment::SEGMENT_MATCH_FALSE:
                        matched = false;
                        return false;
                    case segment::SEGMENT_MATCH_WIDE_MATCH:
                        return true;
                    default:
                        break;
                    }
                    rule_segment_index++;
                }

                last_pos = pos + 1;
                path_segment_index++;
            }
            if (*c == '\0')
                break;
        }

        if (path_segment_index - 1 < segments.size() - 1) {
            // len(path) < len(rule)
            return false;
        } else {
            return matched;
        }
    }

    // TODO: 1個だけに対応
    inline bool match_rule_relative(const char *const _path) const {
        if (_path[0] == '\0') {
            std::cerr << "path is zero length" << std::endl;
            return false;
        }

        const char *path;
        // remove first /
        if (_path[0] == '/') {
            path = _path + 1;
        } else {
            path = _path;
        }

        size_t pos = 0, last_pos = 0;
        for (const char *c = path;; pos++, c++) {
            if (*c == '/' || *c == '\0') {
                segment::segment_match_type match = segments[0].match(pos - last_pos, path + last_pos);
                switch (match) {
                case segment::SEGMENT_MATCH_TRUE:
                    return true;
                }

                last_pos = pos + 1;

                if (*c == '\0')
                    break;
            }
        }

        return false;
    }
};
