//
//  ValidateReceipt.h
//
//  Created by Ruotger Skupin on 23.10.10.
//  Copyright 2010-2011 Matthew Stevens, Ruotger Skupin, Apple, Alessandro Segala. All rights reserved.
//

#import <Foundation/Foundation.h>


extern NSString *kReceiptBundleIdentifer;
extern NSString *kReceiptBundleIdentiferData;
extern NSString *kReceiptVersion;
extern NSString *kReceiptOpaqueValue;
extern NSString *kReceiptHash;
extern NSString *kReceiptInApp;

extern NSString *kReceiptInAppQuantity;
extern NSString *kReceiptInAppProductIdentifier;
extern NSString *kReceiptInAppTransactionIdentifier;
extern NSString *kReceiptInAppPurchaseDate;
extern NSString *kReceiptInAppOriginalTransactionIdentifier;
extern NSString *kReceiptInAppOriginalPurchaseDate;

#if defined __cplusplus
extern "C" {
#endif
    
NSData* deviceGUID(void);
void parseInAppPurchasesData(NSData * inappData, NSMutableArray *resultArray);
NSDictionary * dictionaryWithAppStoreReceipt(NSData * receiptData);

NSArray *obtainInAppPurchasesWithData(NSData *receiptData);
NSArray *obtainInAppPurchasesWithReceipt(NSDictionary *receipt);
NSDictionary * validateReceiptWithData(NSData *data);


#if defined __cplusplus
};
#endif
