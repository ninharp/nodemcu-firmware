// Module for irc

#include "module.h"
#include "lauxlib.h"
#include "platform.h"

#include "c_string.h"
#include "c_stdlib.h"
#include "c_stdio.h"

#include "c_types.h"
#include "mem.h"
#include "lwip/ip_addr.h"
#include "espconn.h"

#include "parser.h"

#define IRC_CONNECT_TIMEOUT 5
#define IRC_SEND_TIMEOUT 5
#define IRC_BUF_SIZE 1024

typedef enum {
  IRC_INIT,
  IRC_CONNECT_SENT,
  IRC_CONNECT_SENDING,
  IRC_DATA
} tConnState;

typedef struct irc_connection {
  int auto_reconnect;    
  tConnState state;
  char* nickname;
  char* realname;
  char* username;
  char* password;
  int port;
} irc_connection;

typedef struct irc_socketdata
{
  lua_State *L;
  struct espconn *pesp_conn;
  int self_ref;
  int cb_connect_ref;
  int cb_disconnect_ref;
  int cb_message_ref;
  int cb_suback_ref;
  int cb_puback_ref;
  bool connected; 
  irc_connection connection;
  bool registered;
  irc_data irc_msg;
#ifdef CLIENT_SSL_ENABLE
  uint8_t secure;
#endif
} irc_socketdata;

static void socket_connect(struct espconn *pesp_conn);
static void irc_socket_reconnected(void *arg, sint8_t err);
static void irc_socket_connected(void *arg);

static ip_addr_t host_ip; // for dns
static dns_reconn_count = 0;

static int irc_socket_quit(lua_State* L)
{
  NODE_DBG("enter irc_socket_quit.\n");
  /*
  struct espconn *pesp_conn = NULL;
  irc_socketdata *sd;
  size_t l, unl = 0;
  uint8_t stack = 1;
  const char *message = NULL;
  
  sd = (irc_socketdata *)luaL_checkudata(L, stack, "irc.socket");
  luaL_argcheck(L, sd, stack, "irc.socket expected");
  stack++;
  if(sd==NULL){
    NODE_DBG("userdata is nil.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(sd->pesp_conn == NULL){
    NODE_DBG("sd->pesp_conn is NULL.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(!sd->connected){
    luaL_error( L, "not connected" );
    lua_pushboolean(L, 0);
    return 1;
  }
  
  // Get message parameter
  if(lua_isstring( L, stack )){
    message = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(message == NULL)
    unl = 0;
  NODE_DBG("length message: %d (%s)\r\n", unl, message);
  
  char *out_buffer;
  out_buffer = (char*) c_zalloc(128 * sizeof(char));
  c_sprintf(out_buffer, "QUIT %s\r\n", message);
  espconn_sent(sd->pesp_conn, out_buffer, c_strlen(out_buffer));
  c_free(out_buffer);
  */
  NODE_DBG("leave irc_socket_quit.\n");
}

static int irc_socket_say_channel(lua_State* L)
{
  NODE_DBG("enter irc_socket_say_channel.\n");
  struct espconn *pesp_conn = NULL;
  irc_socketdata *sd;
  size_t l, unl = 0;
  uint8_t stack = 1;
  const char *channel = NULL;
  const char* message = NULL;
  
  sd = (irc_socketdata *)luaL_checkudata(L, stack, "irc.socket");
  luaL_argcheck(L, sd, stack, "irc.socket expected");
  stack++;
  if(sd==NULL){
    NODE_DBG("userdata is nil.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(sd->pesp_conn == NULL){
    NODE_DBG("sd->pesp_conn is NULL.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(!sd->connected){
    luaL_error( L, "not connected" );
    lua_pushboolean(L, 0);
    return 1;
  }
  
  // Get channel parameter
  if(lua_isstring( L, stack )){
    channel = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(channel == NULL)
    unl = 0;
  NODE_DBG("length channel: %d (%s)\r\n", unl, channel);

  // Get message parameter
  if(lua_isstring( L, stack )){
    message = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(message == NULL)
    unl = 0;
  NODE_DBG("length message: %d (%s)\r\n", unl, message);

  char *out_buffer;
  out_buffer = (char*) c_zalloc(128 * sizeof(char));
  c_sprintf(out_buffer, "PRIVMSG %s :%s\r\n", channel, message);
  espconn_sent(sd->pesp_conn, out_buffer, c_strlen(out_buffer));
  c_free(out_buffer);
  NODE_DBG("leave irc_socket_say_channel.\n");
}

static int irc_socket_join_channel(lua_State* L)
{
  NODE_DBG("enter irc_socket_join_channel.\n");
  struct espconn *pesp_conn = NULL;
  irc_socketdata *sd;
  size_t l, unl = 0;
  uint8_t stack = 1;
  const char *channel = NULL;
  
  sd = (irc_socketdata *)luaL_checkudata(L, stack, "irc.socket");
  luaL_argcheck(L, sd, stack, "irc.socket expected");
  stack++;
  if(sd==NULL){
    NODE_DBG("userdata is nil.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(sd->pesp_conn == NULL){
    NODE_DBG("sd->pesp_conn is NULL.\n");
    lua_pushboolean(L, 0);
    return 1;
  }

  if(!sd->connected){
    luaL_error( L, "not connected" );
    lua_pushboolean(L, 0);
    return 1;
  }
  
  // Get channel parameter
  if(lua_isstring( L, stack )){
    channel = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(channel == NULL)
    unl = 0;
  NODE_DBG("length channel: %d (%s)\r\n", unl, channel);
  
  char *out_buffer;
  out_buffer = (char*) c_zalloc(128 * sizeof(char));
  c_sprintf(out_buffer, "JOIN %s\r\n", channel);
  espconn_sent(sd->pesp_conn, out_buffer, c_strlen(out_buffer));
  c_free(out_buffer);
  NODE_DBG("leave irc_socket_join_channel.\n");
}

static int irc_socket_client( lua_State* L )
{
  NODE_DBG("enter irc_socket_client.\n");
  
  irc_socketdata *sd;
  char tempid[20] = {0};
  c_sprintf(tempid, "%s%x", "NodeMCU_", system_get_chip_id() );
  NODE_DBG(tempid);
  NODE_DBG("\n");

  const char *username = NULL;
  const char *nickname = NULL;
  const char *realname = NULL;

  size_t idl = c_strlen(tempid);
  size_t unl = 0, pwl = 0;
  int stack = 1;
  int clean_session = 1;
  int top = lua_gettop(L);
  
  // create a object
  sd = (irc_socketdata *)lua_newuserdata(L, sizeof(irc_socketdata));
  // pre-initialize it, in case of errors
  sd->L = NULL;
  sd->self_ref = LUA_NOREF;
  sd->cb_connect_ref = LUA_NOREF;
  sd->cb_disconnect_ref = LUA_NOREF;

  sd->cb_message_ref = LUA_NOREF;
  sd->cb_suback_ref = LUA_NOREF;
  sd->cb_puback_ref = LUA_NOREF;
  sd->pesp_conn = NULL;
#ifdef CLIENT_SSL_ENABLE
  sd->secure = 0;
#endif

  sd->connection.state = IRC_INIT;
  sd->connected = false;
  sd->registered = false;

  //irc_parse_init(&sd->irc_msg);

  // set its metatable
  luaL_getmetatable(L, "irc.socket");
  lua_setmetatable(L, -2);

  sd->L = L;   // L for irc module.
  
  // Get nickname parameter
  if(lua_isstring( L, stack )){
    nickname = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(nickname == NULL)
    unl = 0;
  NODE_DBG("length nickname: %d (%s)\r\n", unl, nickname);
  sd->connection.nickname = (char*)nickname;
  
  // Get username parameter
  if(lua_isstring( L, stack )){
    username = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(username == NULL)
    unl = 0;
  NODE_DBG("length username: %d (%s)\r\n", unl, username);
  sd->connection.username = (char*)username;
  
  // Get realname parameter
  if(lua_isstring( L, stack )){
    realname = luaL_checklstring( L, stack, &unl );
    stack++;
  }
  if(realname == NULL)
    unl = 0;
  NODE_DBG("length realname: %d (%s)\r\n", unl, realname);
  sd->connection.realname = (char*)realname;
  NODE_DBG("leave irc_socket_client.\n");
  return 1;
}

static void irc_socket_received(void *arg, char *pdata, unsigned short len)
{
  NODE_DBG("enter irc_socket_received.\n");
  //espconn_recv_hold(arg);
  system_soft_wdt_restart();
  uint8_t *in_buffer = (uint8_t *)pdata;
  int length = (int)len;
  
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL)
    return;
  irc_socketdata *sd = (irc_socketdata *)pesp_conn->reverse;
  if(sd == NULL)
    return;

  if(length > IRC_BUF_SIZE || length <= 0)
	  return;
    
  sd->irc_msg.count = irc_parse(&sd->irc_msg, in_buffer);
  
  if (&sd->irc_msg.count <= 0)
    return;
    
  int count = 0;
  for (count = 0; count <= sd->irc_msg.count; count++) {
    //NODE_DBG("Raw[%d]: %s\nString: %s\n", count, in_buffer, sd->irc_msg.raw);
    //NODE_DBG("Part0[%d]: >%s<\n", count, sd->irc_msg.msgs[count].part0);
    //NODE_DBG("Line[%d]: >%s<\n", count, sd->irc_msg.msgs[count].part1);
    //NODE_DBG("Type[%d]: %d\n", count, sd->irc_msg.msgs[count].type);
    if (sd->irc_msg.msgs[count].type == IRC_PING) {
      NODE_DBG("irc: irc ping received! (%s)\n", sd->irc_msg.msgs[count].message);
      in_buffer[1] = 'O'; 
      NODE_DBG("irc: Raw Pong: %s\n", in_buffer);
      espconn_sent(sd->pesp_conn, in_buffer, c_strlen(in_buffer)); // send back pong
    } else if (sd->irc_msg.msgs[count].type == IRC_ERR_NICKNAMEINUSE) {
      char *out_buffer = "NICK testbot_\r\n";
      espconn_sent(sd->pesp_conn, out_buffer, c_strlen(out_buffer));
      c_free(out_buffer);
    } else if (sd->irc_msg.msgs[count].type == IRC_RPL_WELCOME) {
      //char *out_buffer = "JOIN #lobby\r\n";
      //espconn_sent(sd->pesp_conn, out_buffer, c_strlen(out_buffer));
      //c_free(out_buffer);
    } else if (sd->irc_msg.msgs[count].type == IRC_PRIVMSG) {
      NODE_DBG("irc: <%s> %s\n", sd->irc_msg.msgs[count].mask.nickname, sd->irc_msg.msgs[count].message);
    } else if (sd->irc_msg.msgs[count].type == IRC_NOTICE) {
      NODE_DBG("irc: irc notice found!\n");
      if (c_strcmp(sd->irc_msg.msgs[count].param, "*") == 0) {
        if (!sd->registered) {
          NODE_DBG("irc: First contact from IRC Server received!\n");
        //char out_buffer[128] = "USER testbot 0 0 :testbot\r\nNICK testbot\r\n";
        //c_sprintf(out_buffer, "USER %s 0 0 :%s\r\nNICK %s\r\n", sd->connection.username, sd->connection.realname, sd->connection.nickname);
        //out_buffer = (char*)c_zalloc(128);
          char *out_buffer = "USER testbot 0 0 :testbot\r\nNICK testbot\r\n";
          espconn_sent(sd->pesp_conn, out_buffer, c_strlen(out_buffer));
          c_free(out_buffer);
          sd->registered = true;
        } else {
          NODE_DBG("irc: Server Notice: %s\n", sd->irc_msg.msgs[count].message);
        }
      } 
    }
  }
  //irc_parse_close(&sd->irc_msg);
  //espconn_recv_unhold(pesp_conn);
  NODE_DBG("leave irc_socket_received.\n");
  return;
}

static void irc_socket_sent(void *arg)
{
  NODE_DBG("enter irc_socket_sent.\n");
  
  NODE_DBG("leave irc_socket_sent.\n");
}

static void irc_socket_disconnected(void *arg)    // tcp only
{
  NODE_DBG("enter irc_socket_disconnected.\n");
  struct espconn *pesp_conn = arg;
  bool call_back = false;
  if(pesp_conn == NULL)
    return;
  irc_socketdata *sd = (irc_socketdata *)pesp_conn->reverse;
  if(sd == NULL)
    return;
  
  if(sd->connected){     // call back only called when socket is from connection to disconnection.
    sd->connected = false;
    sd->registered = false;
    if((sd->L != NULL) && (sd->cb_disconnect_ref != LUA_NOREF) && (sd->self_ref != LUA_NOREF)) {
      lua_rawgeti(sd->L, LUA_REGISTRYINDEX, sd->cb_disconnect_ref);
      lua_rawgeti(sd->L, LUA_REGISTRYINDEX, sd->self_ref);  // pass the userdata(client) to callback func in lua
      call_back = true;
    }
  }
  
  if(sd->connection.auto_reconnect){
    sd->pesp_conn->reverse = sd;
    sd->pesp_conn->type = ESPCONN_TCP;
    sd->pesp_conn->state = ESPCONN_NONE;
    sd->connected = false;
    sd->pesp_conn->proto.tcp->remote_port = sd->connection.port;
    sd->pesp_conn->proto.tcp->local_port = espconn_port();
    espconn_regist_connectcb(sd->pesp_conn, irc_socket_connected);
    espconn_regist_reconcb(sd->pesp_conn, irc_socket_reconnected);
    socket_connect(pesp_conn);
  } else {
    if(sd->pesp_conn){
      sd->pesp_conn->reverse = NULL;
      if(sd->pesp_conn->proto.tcp)
        c_free(sd->pesp_conn->proto.tcp);
      sd->pesp_conn->proto.tcp = NULL;
      c_free(sd->pesp_conn);
      sd->pesp_conn = NULL;
    }

    if(sd->L == NULL)
      return;
    lua_gc(sd->L, LUA_GCSTOP, 0);
    if(sd->self_ref != LUA_NOREF){   // TODO: should we unref the client and delete it?
      luaL_unref(sd->L, LUA_REGISTRYINDEX, sd->self_ref);
      sd->self_ref = LUA_NOREF; // unref this, and the irc.socket userdata will delete it self
    }
    lua_gc(sd->L, LUA_GCRESTART, 0);
  }

  if((sd->L != NULL) && call_back){
    lua_call(sd->L, 1, 0);
  }

  NODE_DBG("leave irc_socket_disconnected.\n");
}

static void irc_socket_reconnected(void *arg, sint8_t err)
{
  NODE_DBG("enter irc_socket_reconnected.\n");

  NODE_DBG("leave irc_socket_reconnected.\n");
}

static void irc_socket_connected(void *arg)
{
  NODE_DBG("enter irc_socket_connected.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL)
    return;
  irc_socketdata *sd = (irc_socketdata *)pesp_conn->reverse;
  if(sd == NULL)
    return;
  sd->connected = true;
  espconn_regist_recvcb(pesp_conn, irc_socket_received);
  espconn_regist_sentcb(pesp_conn, irc_socket_sent);
  espconn_regist_disconcb(pesp_conn, irc_socket_disconnected);
  
  NODE_DBG("leave irc_socket_connected.\n");
  return;
}


static void socket_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
  NODE_DBG("enter socket_dns_found.\n");
  struct espconn *pesp_conn = arg;
  if(pesp_conn == NULL){
    NODE_DBG("pesp_conn null.\n");
    return;
  }

  if(ipaddr == NULL)
  {
    dns_reconn_count++;
    if( dns_reconn_count >= 5 ){
      NODE_ERR( "DNS Fail!\n" );
      // Note: should delete the pesp_conn or unref self_ref here.
      irc_socket_disconnected(arg);   // although not connected, but fire disconnect callback to release every thing.
      return;
    }
    NODE_ERR( "DNS retry %d!\n", dns_reconn_count );
    host_ip.addr = 0;
    espconn_gethostbyname(pesp_conn, name, &host_ip, socket_dns_found);
    return;
  }

  // ipaddr->addr is a uint32_t ip
  if(ipaddr->addr != 0)
  {
    dns_reconn_count = 0;
    c_memcpy(pesp_conn->proto.tcp->remote_ip, &(ipaddr->addr), 4);
    NODE_DBG("TCP ip is set: ");
    NODE_DBG(IPSTR, IP2STR(&(ipaddr->addr)));
    NODE_DBG("\n");
    socket_connect(pesp_conn);
  }
  NODE_DBG("leave socket_dns_found.\n");
}

static int irc_socket_close( lua_State* L )
{
  NODE_DBG("enter irc_socket_close.\n");
  int i = 0;
  irc_socketdata *sd = NULL;

  sd = (irc_socketdata *)luaL_checkudata(L, 1, "irc.socket");
  luaL_argcheck(L, sd, 1, "irc.socket expected");
  if(sd == NULL)
    return 0;

  if(sd->pesp_conn == NULL)
    return 0;

  irc_parse_close(&sd->irc_msg);
  // Send disconnect message
/*
#ifdef CLIENT_SSL_ENABLE
  if(sd->secure)
    espconn_secure_sent(sd->pesp_conn, temp_msg->data, temp_msg->length);
  else
#endif
    espconn_sent(sd->pesp_conn, temp_msg->data, temp_msg->length);
*/

  sd->connection.auto_reconnect = 0;   // stop auto reconnect.

#ifdef CLIENT_SSL_ENABLE
  if(sd->secure){
    if(sd->pesp_conn->proto.tcp->remote_port || sd->pesp_conn->proto.tcp->local_port)
      espconn_secure_disconnect(sd->pesp_conn);
  }
  else
#endif
  {
    if(sd->pesp_conn->proto.tcp->remote_port || sd->pesp_conn->proto.tcp->local_port)
      espconn_disconnect(sd->pesp_conn);
  }
  NODE_DBG("leave irc_socket_close.\n");
  return 0;
}


// Lua: irc:connect( host, port, secure, auto_reconnect, function(client) )
static int irc_socket_connect( lua_State* L )
{
  NODE_DBG("enter irc_socket_connect.\n");
  irc_socketdata *sd = NULL;
  unsigned port = 6670;
  size_t il;
  ip_addr_t ipaddr;
  const char *domain;
  const char *password = NULL;
  int stack = 1;
  unsigned secure = 0, auto_reconnect = 0;
  int top = lua_gettop(L);

  // Check for current irc.socket (create with irc.client(...))
  sd = (irc_socketdata *)luaL_checkudata(L, stack, "irc.socket");
  luaL_argcheck(L, sd, stack, "irc.socket expected");
  stack++;
  if(sd == NULL)
    return 0;

  if(sd->connected){
    return luaL_error(L, "already connected");
  }

  if(sd->pesp_conn){   //TODO: should I free tcp struct directly or ask user to call close()???
    sd->pesp_conn->reverse = NULL;
    if(sd->pesp_conn->proto.tcp)
      c_free(sd->pesp_conn->proto.tcp);
    sd->pesp_conn->proto.tcp = NULL;
    c_free(sd->pesp_conn);
    sd->pesp_conn = NULL;
  }

  struct espconn *pesp_conn = NULL;
	pesp_conn = sd->pesp_conn = (struct espconn *)c_zalloc(sizeof(struct espconn));
	if(!pesp_conn)
		return luaL_error(L, "not enough memory");

	pesp_conn->proto.udp = NULL;
	pesp_conn->proto.tcp = (esp_tcp *)c_zalloc(sizeof(esp_tcp));
	if(!pesp_conn->proto.tcp){
		c_free(pesp_conn);
		pesp_conn = sd->pesp_conn = NULL;
		return luaL_error(L, "not enough memory");
	}
	// reverse is for the callback function
	pesp_conn->reverse = sd;
	pesp_conn->type = ESPCONN_TCP;
	pesp_conn->state = ESPCONN_NONE;
  sd->connected = false;

  // Get hostname parameter
  if( (stack<=top) && lua_isstring(L,stack) )   // deal with the domain string
  {
    domain = luaL_checklstring( L, stack, &il );

    stack++;
    if (domain == NULL)
    {
      domain = "127.0.0.1";
    }
    ipaddr.addr = ipaddr_addr(domain);
    c_memcpy(pesp_conn->proto.tcp->remote_ip, &ipaddr.addr, 4);
    NODE_DBG("TCP ip is set: ");
    NODE_DBG(IPSTR, IP2STR(&ipaddr.addr));
    NODE_DBG("\n");
  }

  // Get port parameter
  if ( (stack<=top) && lua_isnumber(L, stack) )
  {
    port = lua_tointeger(L, stack);
    stack++;
    NODE_DBG("TCP port is set: %d.\n", port);
  }
  pesp_conn->proto.tcp->remote_port = port;
  pesp_conn->proto.tcp->local_port = espconn_port();
  sd->connection.port = port;
  
  // Get password parameter
  if( (stack<=top) && lua_isstring(L,stack) )
  {
	password = luaL_checklstring( L, stack, &il );
    stack++;
  }

  if ( (stack<=top) && lua_isnumber(L, stack) )
  {
    secure = lua_tointeger(L, stack);
    stack++;
    if ( secure != 0 && secure != 1 ){
      secure = 0; // default to 0
    }
  } else {
    secure = 0; // default to 0
  }
#ifdef CLIENT_SSL_ENABLE
  sd->secure = secure; // save
#else
  if ( secure )
  {
    return luaL_error(L, "ssl not available");
  }
#endif

  // Get autoreconnect parameter
  if ( (stack<=top) && lua_isnumber(L, stack) )
  {
    auto_reconnect = lua_tointeger(L, stack);
    stack++;
    if ( auto_reconnect != 0 && auto_reconnect != 1 ){
      auto_reconnect = 0; // default to 0
    }
  } else {
    auto_reconnect = 0; // default to 0
  }
  sd->connection.auto_reconnect = auto_reconnect;

  // Get 'connected' function parameter
  // call back function when a connection is obtained, tcp only
  if ((stack<=top) && (lua_type(L, stack) == LUA_TFUNCTION || lua_type(L, stack) == LUA_TLIGHTFUNCTION)){
    lua_pushvalue(L, stack);  // copy argument (func) to the top of stack
    if(sd->cb_connect_ref != LUA_NOREF)
      luaL_unref(L, LUA_REGISTRYINDEX, sd->cb_connect_ref);
    sd->cb_connect_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    stack++;
  }

  lua_pushvalue(L, 1);  // copy userdata to the top of stack
  if(sd->self_ref != LUA_NOREF)
    luaL_unref(L, LUA_REGISTRYINDEX, sd->self_ref);
  sd->self_ref = luaL_ref(L, LUA_REGISTRYINDEX);

  espconn_regist_connectcb(pesp_conn, irc_socket_connected);
  espconn_regist_reconcb(pesp_conn, irc_socket_reconnected);

  if((ipaddr.addr == IPADDR_NONE) && (c_memcmp(domain,"255.255.255.255",16) != 0))
  {
    host_ip.addr = 0;
    dns_reconn_count = 0;
    if(ESPCONN_OK == espconn_gethostbyname(pesp_conn, domain, &host_ip, socket_dns_found)){
      socket_dns_found(domain, &host_ip, pesp_conn);  // ip is returned in host_ip.
    }
  }
  else
  {
    socket_connect(pesp_conn);
  }

  NODE_DBG("leave irc_socket_connect.\n");
  return 0;
}

static void socket_connect(struct espconn *pesp_conn)
{
  NODE_DBG("enter socket_connect.\n");
  if(pesp_conn == NULL)
    return;
  irc_socketdata *sd = (irc_socketdata *)pesp_conn->reverse;
  if(sd == NULL)
    return;

  sd->connection.state = IRC_INIT;
#ifdef CLIENT_SSL_ENABLE
  if(sd->secure)
  {
    espconn_secure_connect(pesp_conn);
  }
  else
#endif
  {
    espconn_connect(pesp_conn);
  }

  NODE_DBG("leave socket_connect.\n");
}

// Module function map
static const LUA_REG_TYPE irc_socket_map[] = {
  { LSTRKEY( "connect" ),   LFUNCVAL( irc_socket_connect ) },
  { LSTRKEY( "say" ),      LFUNCVAL( irc_socket_say_channel ) },
  { LSTRKEY( "join" ),      LFUNCVAL( irc_socket_join_channel ) },
  { LSTRKEY( "quit" ),      LFUNCVAL( irc_socket_quit ) },
  { LSTRKEY( "close" ),     LFUNCVAL( irc_socket_close ) },
  { LSTRKEY( "__index" ),   LROVAL( irc_socket_map ) },
  { LNILKEY, LNILVAL }
};

static const LUA_REG_TYPE irc_map[] = {
  { LSTRKEY( "Client" ),      LFUNCVAL( irc_socket_client ) },
  { LSTRKEY( "__metatable" ), LROVAL( irc_map ) },
  { LNILKEY, LNILVAL }
};

int luaopen_irc( lua_State *L )
{
  // create metatable for irc.socket
  luaL_rometatable(L, "irc.socket", (void *)irc_socket_map);  
  return 0;
}

NODEMCU_MODULE(IRC, "irc", irc_map, luaopen_irc);
