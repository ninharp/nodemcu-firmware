/*
* Copyright (c) 2016, Michael Sauer
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/ 
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
