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
    
	// Add a transaction. If the transaction IDs are the same then the new transaction will overwrite the old one
    void addTransaction(PaymentTransaction const& transaction);
	// Gets the PaymentTransaction object for the specified transaction ID. Returns true if the transaction exists, false otherwise
    bool getTransaction(std::string const& transactionId, PaymentTransaction& transaction);
    bool findTransaction(std::string const& transactionId);
    void removeTransaction(std::string const& transactionId);
    std::unordered_map<std::string, PaymentTransaction> const& getTransactions() const { return m_transactions; }
    
	// Load transaction data. Returns true if successful, false otherwise
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
