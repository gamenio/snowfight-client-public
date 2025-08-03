//
//  StoreUtil-ios.h
//  snowfight
//
//  Created by Luthier on 2019/5/16.
//

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#include "../StoreError.h"

USING_NS;
    
StoreError storeErrorWithNSError(NSError *error);
