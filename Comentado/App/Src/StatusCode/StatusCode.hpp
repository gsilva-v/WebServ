#ifndef __STATUSCODE_H__
#define __STATUSCODE_H__

#include <map>
#include "../Boost/String.hpp"

#define INSERT_CODE code.insert(std::pair<boost::string, boost::string>

typedef struct StatusCode {

    std::map<boost::string, boost::string> code;

    StatusCode() {
                INSERT_CODE("100", " Continue"));
                INSERT_CODE("200", " Ok"));
                INSERT_CODE("201", " Created"));
                INSERT_CODE("202", " Accepted"));
                INSERT_CODE("301", " Moved Permanently"));
                INSERT_CODE("400", " Bad Request"));
                INSERT_CODE("401", " Unauthorized"));
                INSERT_CODE("403", " Forbidden"));
                INSERT_CODE("404", " Page not found"));
                INSERT_CODE("405", " Method Not Allowed"));
                INSERT_CODE("500", " Internal Server Error"));
                INSERT_CODE("502", " Bad Gateway"));
                INSERT_CODE("503", " Service Unavailable"));
                INSERT_CODE("504", " Gateway Timeout"));
                INSERT_CODE("505", " HTTP Version Not Supported"));
    }
} StatusCode;

#endif // __STATUSCODE_H__