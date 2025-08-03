#ifndef __CLIENT_CONFIG_H__
#define __CLIENT_CONFIG_H__


// 服务器环境类型定义
#define SERVER_DEV          0         // 开发环境
#define SERVER_TEST         1         // 测试环境
#define SERVER_PROD         2         // 生产环境

// 目标服务器环境
#define TARGET_SERVER_ENV        SERVER_PROD

// 验证服务器地址和端口号
#if TARGET_SERVER_ENV == SERVER_PROD
#define AUTH_SERVER_ADDR            "auth.snowfight.gamen.io"
#define AUTH_SERVER_PORT            18401

#else
#define AUTH_SERVER_ADDR            "192.168.1.109"
#define AUTH_SERVER_PORT            18401

#endif // TARGET_SERVER_ENV

// 时间同步服务器地址和端口号
#if TARGET_SERVER_ENV == SERVER_PROD
#define NTS_SERVER_ADDR             "time.gamen.io"
#define NTS_SERVER_PORT             18123

#else
#define NTS_SERVER_ADDR             "192.168.1.109"
#define NTS_SERVER_PORT             18123

#endif // TARGET_SERVER_ENV

#endif // __CLIENT_CONFIG_H__
