//
// Created by Luthier on 2019/9/4.
//

#ifndef __ANDROID_SECURITY_H__
#define __ANDROID_SECURITY_H__

#include <openssl/sha.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#include "common/Common.h"

NS_BEGIN

bool verifyPurchase(std::string const& base64PublicKey, std::string const& signedData, std::string const& signature);

RSA* generatePublicKey(std::string const& encodedPublicKey);
bool verity(RSA* rsa, std::string const& signedData, std::string const& signature);

NS_END

#endif // __ANDROID_SECURITY_H__
