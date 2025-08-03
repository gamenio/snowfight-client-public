//
// Created by Luthier on 2019/9/4.
//

#include "Security.h"


NS_BEGIN

bool verifyPurchase(std::string const& base64PublicKey, std::string const& signedData, std::string const& signature)
{
    if (signedData.empty() || base64PublicKey.empty()
        || signature.empty()) {
        CCLOG("Purchase verification failed: missing data.");
        return false;
    }

    RSA* rsa = generatePublicKey(base64PublicKey);
    if(!rsa)
        return false;

    return verity(rsa, signedData, signature);
}

RSA* generatePublicKey(std::string const& encodedPublicKey)
{
    unsigned char* decodedKey = nullptr;
    int decodedKeyLen = cocos2d::base64Decode((unsigned char*)encodedPublicKey.c_str(), (unsigned int)encodedPublicKey.length(), &decodedKey);
    if(decodedKeyLen <= 0)
    {
        CCLOG("Base64 PublicKey decoding failed.");
        return NULL;
    }

    BIO* memBIO = BIO_new(BIO_s_mem());
    if(!memBIO)
        return NULL;

    int dataLen = BIO_write(memBIO, decodedKey, decodedKeyLen);
    if(decodedKey)
        free(decodedKey);

    if(dataLen <= 0)
        return NULL;

    RSA* rsa = d2i_RSA_PUBKEY_bio(memBIO, NULL);
    BIO_free(memBIO);

    return rsa;
}

bool verity(RSA* rsa, std::string const& signedData, std::string const& signature)
{
    unsigned char* decodedSig = nullptr;
    int decodedSigLen = cocos2d::base64Decode((unsigned char*)signature.c_str(), (unsigned int)signature.length(), &decodedSig);
    if(decodedSigLen <= 0)
    {
        CCLOG("Base64 Signature decoding failed.");
        return false;
    }

    unsigned char sha1Digest[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)signedData.c_str(), signedData.length(), sha1Digest);

    int verifyResult = RSA_verify(NID_sha1, sha1Digest, SHA_DIGEST_LENGTH, (unsigned char*)decodedSig, decodedSigLen, rsa);
    if(decodedSig)
        free(decodedSig);

    if(verifyResult)
        return true;
    else
    {
        unsigned long err = ERR_get_error();
        char buff[256];
        ERR_error_string(err, buff);
        CCLOG("Signature verification failed. msg: %s", buff);
    }

    return false;
}

NS_END


