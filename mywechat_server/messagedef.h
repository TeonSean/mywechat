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
#define ACTION_SYNC         10

struct login_packet
{
    char namelen;
    char name[31];
    char codelen;
    char code[31];
};
#define SUCCESS             0
#define WRONG_PASSWORD      1
#define ACCOUNT_CREATED     2
#define ALREADY_ONLINE      3
#define ALREADY_FRIEND      4
#define USER_NON_EXIST      5
#define ADD_YOURSELF        6

#endif // MESSAGEDEF_H
