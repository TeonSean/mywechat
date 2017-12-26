#ifndef MESSAGEDEF_H
#define MESSAGEDEF_H

#define ACTION_LOGIN        0
#define ACTION_LOGOUT       1
#define ACTION_SEARCH       2
#define ACTION_ADD          3
#define ACTION_LIST         4
#define ACTION_SEND_MSG     5
#define ACTION_SEND_FILE    6
#define ACTION_RECV_MSG     7
#define ACTION_RECV_FILE    8
#define ACTION_PROFILE      9
#define ACTION_CHAT         10
#define ACTION_EXIT         11

#include<string>

struct unsent
{
    std::string sender;
    std::string msg;
};

#define SUCCESS             0
#define WRONG_PASSWORD      1
#define ACCOUNT_CREATED     2
#define ALREADY_ONLINE      3
#define ALREADY_FRIEND      1
#define USER_NON_EXIST      2
#define ADD_YOURSELF        3
#define NOT_YOUR_FRIEND     1
#define NOTHING_NEW         1
#define NEW_MSG             2
#define NEW_FILE            3

#endif // MESSAGEDEF_H
