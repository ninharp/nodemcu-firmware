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
* 
* Used MQTT Module as Template for this module
* 
* TODO:
* 	* Memory optimization
* 	* Replace the main parser to a more reliable one
*
*/
#include "parser.h"

#define IRC_BUF_SIZE 1024

void irc_parse_init(irc_data* data, int n)
{
	NODE_DBG("enter irc_parse_init.\n");
	//:irc1.ninirc.ga 001 testbot :Welcome to the ninIRC IRC Network testbot!testbot@dslc-082-082-157-046.pools.arcor-ip.net

	data->msgs[n].part0 = (char*)c_zalloc(64);
	data->msgs[n].part1 = (char*)c_zalloc(64);
	data->msgs[n].message = (char*)c_zalloc(128);
	data->msgs[n].command = (char*)c_zalloc(64);
	data->msgs[n].param = (char*)c_zalloc(64);

	data->msgs[n].mask.raw = (char*)c_zalloc(64);
	data->msgs[n].mask.nickname = (char*)c_zalloc(32);
	data->msgs[n].mask.username = (char*)c_zalloc(32);
	data->msgs[n].mask.host = (char*)c_zalloc(64);
	
	c_memset(data->msgs[n].part0, 0, 64);
	c_memset(data->msgs[n].part1, 0, 64);
	c_memset(data->msgs[n].message, 0, 128);
	c_memset(data->msgs[n].command, 0, 64);
	c_memset(data->msgs[n].param, 0, 64);
	
	c_memset(data->msgs[n].mask.raw, 0, 64);
	c_memset(data->msgs[n].mask.nickname, 0, 32);
	c_memset(data->msgs[n].mask.username, 0, 32);
	c_memset(data->msgs[n].mask.host, 0, 64);
	NODE_DBG("leave irc_parse_init.\n");
}

void irc_parse_close(irc_data* data)
{
	NODE_DBG("enter irc_parse_close.\n");
	int n;
	for (n = 0; n <= data->count; n++) {
		c_free(data->msgs[n].part0);
		c_free(data->msgs[n].part1);
		c_free(data->msgs[n].message);
		c_free(data->msgs[n].command);
		c_free(data->msgs[n].param);
		
		c_free(data->msgs[n].mask.raw);
		c_free(data->msgs[n].mask.nickname);
		c_free(data->msgs[n].mask.username);
		c_free(data->msgs[n].mask.host);
	}
	NODE_DBG("leave irc_parse_close.\n");
}

// TODO: check length of valid size
int irc_parse(irc_data* data, char* raw)
{
	NODE_DBG("enter irc_parse.\n");
	int len = c_strlen(raw);
	data->raw = raw;
	int num = 0;
	int state = 0;
	int i = 0;
	int c0 = 0, c1 = 0,c2 = 0;
	NODE_DBG("irc_parse raw[c%d]: %s\n", len, raw);
	irc_parse_init(data, num);
	NODE_DBG("irc_parse %d. readline %d\n", num+1, i);
	while (i < len) {
		if (raw[i] == '\r' && raw[i+1] == '\n') {
			NODE_DBG("irc_parse [c%d] found end of line %d\r\n", i, num+1);
			data->msgs[num].part0[c0-1] = 0x00;
			data->msgs[num].part1[c1-1] = 0x00;
			data->msgs[num].message[c2] = 0x00;
			//NODE_DBG("irc_parse raw part0: >%s<\n", data->msgs[num].part0);
			//NODE_DBG("irc_parse raw part1: >%s<\n", data->msgs[num].part1);
			//NODE_DBG("irc_parse raw message: >%s<\n", data->msgs[num].message);
			state = 0;
			c0 = 0, c1 = 0,c2 = 0;
			if ((len - i) > 2) {
				if (num < IRC_MAX_MESSAGES) {
					num++;
				} else {
					NODE_DBG("irc_parse Maximum Messages reached! Skip to 0\n");
					num = 0;
				}
				//NODE_DBG("irc_parse Maybe more lines to come!\n");
				irc_parse_init(data, num);
			} else {
				//NODE_DBG("Remaining chars cannot be one line! Breaking up!\n");
				break;
			}
		} else {
			if (raw[i] == ':') {
				state++;
			} else {
				if (state == 0)
					data->msgs[num].part0[c0++] = raw[i];
				else if (state == 1)
					data->msgs[num].part1[c1++] = raw[i];
				else if (state == 2)
					data->msgs[num].message[c2++] = raw[i];
			}
		}
		i++;
	}
	
	//NODE_DBG("Part0: %s\nPart1: %s\nMessage: %s\n", data->msgs[num].part0[c0-1], data->msgs[num].part1[c1-1], data->msgs[num].message[c2]);
	c1 = 0;
	for (i = 0; i < len; i++) {
		if (data->msgs[num].part1[i] == ' ' && state != 2) {
			if (state == 0) {
				data->msgs[num].mask.raw[c1] = 0x00;
			} else if (state == 1) {
				data->msgs[num].command[c1] = 0x00;
			} else if (state == 2) {
				data->msgs[num].param[c1] = 0x00;
			}
			state++;
			c1 = 0;
		} else {
			if (state == 0)
				data->msgs[num].mask.raw[c1++] = data->msgs[num].part1[i];
			else if (state == 1)
				data->msgs[num].command[c1++] = data->msgs[num].part1[i];
			else if (state == 2)
				data->msgs[num].param[c1++] = data->msgs[num].part1[i];
		}
	}
	
	state = 0;
	c1 = 0;
	len = c_strlen(data->msgs[num].mask.raw);
	for (i = 0; i < len; i++) {
		if (data->msgs[num].mask.raw[i] == '!' && state == 0) {
			state = 1;
			data->msgs[num].mask.nickname[c1] = 0x00;
			c1 = 0;
		} else if (data->msgs[num].mask.raw[i] == '@' && state == 1) {
			state = 2;
			data->msgs[num].mask.username[c1] = 0x00;
			c1 = 0;
		} else {
			if (state == 0)
				data->msgs[num].mask.nickname[c1++] = data->msgs[num].mask.raw[i];
			else if (state == 1)
				data->msgs[num].mask.username[c1++] = data->msgs[num].mask.raw[i];
			else if (state == 2)
				data->msgs[num].mask.host[c1++] = data->msgs[num].mask.raw[i];
		}
	}
	
	if (state == 0) {
		data->msgs[num].mask.host = data->msgs[num].mask.raw;
	}
	
	if (c_strcmp(data->msgs[num].part0, "PING") == 0) {
		data->msgs[num].type = IRC_PING;
	} else if (c_strcmp(data->msgs[num].command, "PRIVMSG") == 0) {
		data->msgs[num].type = IRC_PRIVMSG;
	} else if (c_strcmp(data->msgs[num].command, "NOTICE") == 0) {
		data->msgs[num].type = IRC_NOTICE;
	} else if (c_strcmp(data->msgs[num].command, "001") == 0) {
		data->msgs[num].type = IRC_RPL_WELCOME;
	} else if (c_strcmp(data->msgs[num].command, "433") == 0) {
		data->msgs[num].type = IRC_ERR_NICKNAMEINUSE;
	} else {
		data->msgs[num].type = IRC_UNKNOWN_MSG;
	}
	
	NODE_DBG("leave irc_parse.\n");
	if (num > 0)
		return num+1;
	return 0;
}
