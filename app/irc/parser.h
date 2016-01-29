#ifndef IRC_PARSER_H
#define	IRC_PARSER_H
#include "c_types.h"
#include "c_stdio.h"
#include "c_stdlib.h"
#include "c_string.h"
#ifdef	__cplusplus
extern "C" {
#endif

#define IRC_MAX_MESSAGES	15

enum irc_msg_type {
	IRC_PRIVMSG = 0,
	IRC_NOTICE,
	IRC_PING,
	IRC_RPL_WELCOME,
	IRC_ERR_NICKNAMEINUSE,
	IRC_UNKNOWN_MSG
};

typedef struct irc_mask {
	char* nickname;
	char* username;
	char* host;
	char* raw;
} irc_mask;

typedef struct irc_message {
	char* part0;
	char* part1;
	char* message;
	irc_mask mask;
	char* command;
	char* param;
	enum irc_msg_type type;
} irc_message;

typedef struct irc_data {
	int count;
	char* raw;
	irc_message msgs[IRC_MAX_MESSAGES];
} irc_data;


void irc_parse_init(irc_data* data, int n);
void irc_parse_close(irc_data* data);
int irc_parse(irc_data*, char*);

#ifdef	__cplusplus
}
#endif

#endif	/* IRC_PARSER_H */
