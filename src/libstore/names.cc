#include "names.hh"
#include "util.hh"

#include <regex>


namespace nix {


struct Regex
{
    std::regex regex;
};


DrvName::DrvName()
{
    name = "";
}


/* Parse a derivation name.  The `name' part of a derivation name is
   everything up to but not including the first dash *not* followed by
   a letter.  The `version' part is the rest (excluding the separating
   dash).  E.g., `apache-httpd-2.0.48' is parsed to (`apache-httpd',
   '2.0.48'). */
DrvName::DrvName(std::string_view s) : hits(0)
{
    name = fullName = std::string(s);
    for (unsigned int i = 0; i < s.size(); ++i) {
        /* !!! isalpha/isdigit are affected by the locale. */
        if (s[i] == '-' && i + 1 < s.size() && !isalpha(s[i + 1])) {
            name = s.substr(0, i);
            version = s.substr(i + 1);
            break;
        }
    }
}


DrvName::~DrvName()
{ }


bool DrvName::matches(const DrvName & n)
{
    if (name != "*") {
        if (!regex) {
            regex = std::make_unique<Regex>();
            regex->regex = std::regex(name, std::regex::extended);
        }
        if (!std::regex_match(n.name, regex->regex)) return false;
    }
    if (version != "" && version != n.version) return false;
    return true;
}


string nextComponent(string::const_iterator & p,
    const string::const_iterator end)
{
    /* Skip any dots and dashes (component separators). */
    while (p != end && (*p == '.' || *p == '-')) ++p;

    if (p == end) return "";

    /* If the first character is a digit, consume the longest sequence
       of digits.  Otherwise, consume the longest sequence of
       non-digit, non-separator characters. */
    string s;
    if (isdigit(*p))
        while (p != end && isdigit(*p)) s += *p++;
    else
        while (p != end && (!isdigit(*p) && *p != '.' && *p != '-'))
            s += *p++;

    return s;
}


static bool componentsLT(const string & c1, const string & c2)
{
    auto n1 = string2Int<int>(c1);
    auto n2 = string2Int<int>(c2);

    if (n1 && n2) return *n1 < *n2;
    else if (c1 == "" && n2) return true;
    else if (c1 == "pre" && c2 != "pre") return true;
    else if (c2 == "pre") return false;
    /* Assume that `2.3a' < `2.3.1'. */
    else if (n2) return true;
    else if (n1) return false;
    else return c1 < c2;
}


int compareVersions(const string & v1, const string & v2)
{
    string::const_iterator p1 = v1.begin();
    string::const_iterator p2 = v2.begin();

    while (p1 != v1.end() || p2 != v2.end()) {
        string c1 = nextComponent(p1, v1.end());
        string c2 = nextComponent(p2, v2.end());
        if (componentsLT(c1, c2)) return -1;
        else if (componentsLT(c2, c1)) return 1;
    }

    return 0;
}


DrvNames drvNamesFromArgs(const Strings & opArgs)
{
    DrvNames result;
    for (auto & i : opArgs)
        result.emplace_back(i);
    return result;
}


}
