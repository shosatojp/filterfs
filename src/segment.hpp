#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class segment {
  public:
    enum segment_type {
        SEGMENT_TYPE_UNDEFINED = 0,
        SEGMENT_TYPE_WILDCARD = 1,
        SEGMENT_TYPE_NO_WILDCARD = 2,
        SEGMENT_TYPE_WIDE_MATCH = 3,
    };

    enum segment_match_type {
        SEGMENT_MATCH_FALSE = 0,
        SEGMENT_MATCH_TRUE = 1,
        SEGMENT_MATCH_WIDE_MATCH = 2,
    };

  public:
    segment_type type = SEGMENT_TYPE_UNDEFINED;
    std::string str;
    size_t wildcard_pos = -1;

  public:
    segment(const std::string_view str) : str(str) {
        if (str == "**") {
            type = SEGMENT_TYPE_WIDE_MATCH;
            return;
        }

        for (size_t i = 0, len = str.length(); i < len; i++) {
            if (str[i] == '*') {
                if (type == SEGMENT_TYPE_WILDCARD) {
                    throw std::runtime_error("ParseError: Two wildcards in a segment is not allowed.");
                } else {
                    type = SEGMENT_TYPE_WILDCARD;
                    wildcard_pos = i;
                }
            }
        }

        if (type == SEGMENT_TYPE_UNDEFINED) {
            type = SEGMENT_TYPE_NO_WILDCARD;
        }
    }

    segment_match_type match(size_t seg_size, const char *seg) const {
        switch (this->type) {
        case SEGMENT_TYPE_NO_WILDCARD:
            return (segment_match_type)match_segment_no_wildcard(seg_size, seg);
        case SEGMENT_TYPE_WILDCARD:
            return (segment_match_type)match_segment_wildcard(seg_size, seg);
        case SEGMENT_TYPE_WIDE_MATCH:
            return SEGMENT_MATCH_WIDE_MATCH;
        default:
            throw std::runtime_error("Unsupported segment type.");
        }
    }

  private:
    inline int match_segment_wildcard(const size_t seg_size, const char *seg) const {
        const size_t last_part_size = str.length() - wildcard_pos - 1;
        const char *seg_last_part = seg + (seg_size - last_part_size);

        return !std::strncmp(str.c_str(), seg, wildcard_pos) &&
               !std::strncmp(str.substr(wildcard_pos + 1).c_str(), seg_last_part, last_part_size);
    }

    inline int match_segment_no_wildcard(const size_t seg_size, const char *seg) const {
        if (seg_size != str.length()) {
            return false;
        }
        return !std::strncmp(str.c_str(), seg, seg_size);
    }
};
