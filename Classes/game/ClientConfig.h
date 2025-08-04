#ifndef __CLIENT_CONFIG_H__
#define __CLIENT_CONFIG_H__


// Server environment type definition
#define SERVER_DEV          0         // Development
#define SERVER_TEST         1         // Test
#define SERVER_PROD         2         // Production

// Target server environment
#define TARGET_SERVER_ENV        SERVER_PROD

// Authentication server address and port number
#if TARGET_SERVER_ENV == SERVER_PROD
#define AUTH_SERVER_ADDR            "auth.snowfight.gamen.io"
#define AUTH_SERVER_PORT            18401

#else
#define AUTH_SERVER_ADDR            "192.168.1.109"
#define AUTH_SERVER_PORT            18401

#endif // TARGET_SERVER_ENV

// Time synchronization server address and port number
#if TARGET_SERVER_ENV == SERVER_PROD
#define NTS_SERVER_ADDR             "time.gamen.io"
#define NTS_SERVER_PORT             18123

#else
#define NTS_SERVER_ADDR             "192.168.1.109"
#define NTS_SERVER_PORT             18123

#endif // TARGET_SERVER_ENV

#endif // __CLIENT_CONFIG_H__
