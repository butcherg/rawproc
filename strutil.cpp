

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream> 

#include <stdio.h>
#include <stdarg.h> 


std::string tostr(double t)
{ 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 



std::vector<std::string> split(std::string s, std::string delim)
{
	std::vector<std::string> v;
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == s.npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}


std::map<std::string, std::string> parseparams(std::string params)
{
	std::map<std::string, std::string> p;
	std::vector<std::string> l = split(params,";");
	for (std::vector<std::string>::iterator it=l.begin(); it!=l.end(); ++it) {
		std::string name, val;
		std::vector<std::string> nameval = split(*it,"=");
		if (nameval.size() == 2)
			p[nameval[0]] = nameval[1];
		else
			p[nameval[0]] = "1";
	}
	return p;
}


std::string string_format(const std::string fmt, ...) 
{
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}




