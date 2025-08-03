//
//  PaymentStorage.h
//  snowfight
//
//  Created by Luthier on 2019/5/23.
//

#ifndef __PAYMENT_STORAGE_H__
#define __PAYMENT_STORAGE_H__

#include "PaymentData.pb.h"

#include "common/Common.h"
#include "StoreProtocol.h"

USING_NS_CC;

NS_BEGIN

class PaymentStorage
{
public:
    PaymentStorage();
    virtual ~PaymentStorage();
    
    // 增加一笔交易。如果交易ID相同则新的交易将覆盖旧的交易
    void addTransaction(PaymentTransaction const& transaction);
    // 获取指定交易ID的PaymentTransaction对象。如果交易存在则返回true，否则返回false。
    bool getTransaction(std::string const& transactionId, PaymentTransaction& transaction);
    bool findTransaction(std::string const& transactionId);
    void removeTransaction(std::string const& transactionId);
    std::unordered_map<std::string, PaymentTransaction> const& getTransactions() const { return m_transactions; }
    
    // 加载交易数据。如果成功则返回true，否则返回false
    bool loadData();
    bool saveData();
    
    void deleteData();
    
private:
    bool parseDataFile(std::string const& dataFile, PaymentData& paymentData);
    bool serializeToDataFile(std::string const& dataFile, PaymentData& paymentData);
    void updateValues(PaymentData const& paymentData);
    
    std::unordered_map<std::string/* TransactionID */, PaymentTransaction> m_transactions;
};

NS_END

#endif // __PAYMENT_STORAGE_H__
