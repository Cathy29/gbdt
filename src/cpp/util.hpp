// Author: qiyiping@gmail.com (Yiping Qi)

#ifndef _UTIL_H_
#define _UTIL_H_

#include <string>
#include <vector>

namespace gbdt {

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)


std::string JoinString(
    const std::vector<std::string>& parts,
    const std::string& separator);

size_t SplitString(const std::string& str,
                   const std::string& delimiters,
                   std::vector<std::string>* tokens);
}

#endif /* _UTIL_H_ */
