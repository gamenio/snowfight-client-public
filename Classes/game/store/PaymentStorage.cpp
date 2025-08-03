//
//  PaymentStorage.cpp
//  snowfight
//
//  Created by Luthier on 2019/5/23.
//

#include "PaymentStorage.h"

#include "external/xxtea/xxtea.h"

#include "../GameConfig.h"

NS_BEGIN

#define DATA_FILE         "payment.data"


PaymentStorage::PaymentStorage()
{
}

PaymentStorage::~PaymentStorage()
{
}

void PaymentStorage::addTransaction(PaymentTransaction const& transaction)
{
    m_transactions[transaction.transactionId] = transaction;
}

bool PaymentStorage::getTransaction(std::string const& transactionId, PaymentTransaction& transaction)
{
    auto it = m_transactions.find(transactionId);
    if(it != m_transactions.end())
    {
        transaction = (*it).second;
        return true;
    }
    return false;
}

bool PaymentStorage::findTransaction(std::string const& transactionId)
{
    auto it = m_transactions.find(transactionId);
    return it != m_transactions.end();
}

void PaymentStorage::removeTransaction(std::string const& transactionId)
{
    m_transactions.erase(transactionId);
}

bool PaymentStorage::loadData()
{
    bool ret = false;
    
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string dataFile = writablePath + DATA_FILE;
    if(FileUtils::getInstance()->isFileExist(dataFile))
    {
        PaymentData paymentData;
        ret = this->parseDataFile(dataFile, paymentData);
        if(ret)
        {
            this->updateValues(paymentData);
        }
    }
    
    return ret;
}

bool PaymentStorage::saveData()
{
    bool ret = false;
    
    PaymentData paymentData;
    paymentData.set_id("PaymentStorage");
    for(auto const& transaction: m_transactions)
    {
        Transaction* trans = paymentData.add_transactions();
        trans->set_transaction_id(transaction.second.transactionId);
        trans->set_product_id(transaction.second.productId);
        trans->set_state(static_cast<int32>(transaction.second.state));
        trans->set_transaction_time(transaction.second.transactionTime);
    }
    
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string dataFile = writablePath + DATA_FILE;
    ret = this->serializeToDataFile(dataFile, paymentData);
    return ret;
}

void PaymentStorage::deleteData()
{
    std::string writablePath = FileUtils::getInstance()->getWritablePath();
    std::string dataFile = writablePath + DATA_FILE;
    if(FileUtils::getInstance()->isFileExist(dataFile))
        FileUtils::getInstance()->removeFile(dataFile);
}

bool PaymentStorage::parseDataFile(std::string const& dataFile, PaymentData& paymentData)
{
    bool ret = false;
    Data data = FileUtils::getInstance()->getDataFromFile(dataFile);
    // 解密数据
    xxtea_long retLen;
    uint8* decryptData = xxtea_decrypt(data.getBytes(), (xxtea_long)data.getSize(), PAYMENT_DATA_KEY, PAYMENT_DATA_KEY_LENGTH, &retLen);
    if(decryptData)
        ret = paymentData.ParseFromArray(decryptData, retLen);
    else
        ret = false;
    
    return ret;
}

bool PaymentStorage::serializeToDataFile(std::string const& dataFile, PaymentData& paymentData)
{
    bool ret = false;
    int32 byteSize = (int32)paymentData.ByteSizeLong();
    uint8* bytes = new uint8[byteSize];
    ret = paymentData.SerializeToArray(bytes, byteSize);
    if(ret)
    {
        // 加密数据
        xxtea_long retLen;
        uint8* encryptData = xxtea_encrypt(bytes, byteSize, PAYMENT_DATA_KEY, PAYMENT_DATA_KEY_LENGTH, &retLen);
        
        if (encryptData)
        {
            Data data;
            data.fastSet(encryptData, retLen);
            ret = FileUtils::getInstance()->writeDataToFile(data, dataFile);
        }
        else
            ret = false;
    }
    CC_SAFE_DELETE_ARRAY(bytes);
    
    return ret;
}


void PaymentStorage::updateValues(PaymentData const& paymentData)
{
    for(int32 i = 0; i < paymentData.transactions_size(); ++i)
    {
        auto const& transaction = paymentData.transactions(i);
        auto& trans = m_transactions[transaction.transaction_id()];
        trans.transactionId = transaction.transaction_id();
        trans.productId = transaction.product_id();
        trans.state = static_cast<PaymentState>(transaction.state());
        trans.transactionTime = transaction.transaction_time();
    }
}



NS_END
