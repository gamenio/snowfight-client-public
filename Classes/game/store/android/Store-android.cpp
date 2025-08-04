//
//  Store-android.cpp
//  snowfight
//
//  Created by Luthier on 2019/5/8.
//

#include "Store-android.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "Security.h"

NS_BEGIN

static const std::string billingManagerClassName = "io/gamen/snowfight/billing/BillingManager";
static StoreImpl* s_instance = nullptr;

#define  LOG_TAG    "StoreImpl"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

bool checkJniMethodID(JNIEnv* env, jmethodID methodID, const char* name)
{
    if(!methodID)
    {
        if(env->ExceptionCheck())
            env->ExceptionClear();

        LOGE("JNI: Failed to find method id of %s()", name);
        return false;
    }
    return true;
}

bool checkJniClass(JNIEnv* env, jclass cls, const char* className)
{
    if(!cls)
    {
        if(env->ExceptionCheck())
            env->ExceptionClear();

        LOGE("JNI: Failed to find class %s", className);
        return false;
    }
    return true;
}

#define CHECK_JNI_METHODID_RETURN(env, methodID, name) if(!checkJniMethodID(env, methodID, name)) return;
#define CHECK_JNI_METHODID_RETURN_VAL(env, methodID, name, returnValue) if(!checkJniMethodID(env, methodID, name)) return returnValue;

#define CHECK_JNI_CLASS_RETURN(env, cls, className) if(!checkJniClass(env, cls, className)) return;
#define CHECK_JNI_CLASS_RETURN_VAL(env, cls, className, returnValue) if(!checkJniClass(env, cls, className)) return returnValue;

// https://developer.android.com/reference/com/android/billingclient/api/BillingClient.ProductType
/** A Product type for Android apps in-app products. */
#define INAPP "inapp"
/** A Product type for Android apps subscriptions. */
#define SUBS "subs"

// https://developer.android.com/reference/com/android/billingclient/api/BillingClient.BillingResponseCode
enum BillingResponseCode {
    /**
     * This field is deprecated.
     * See SERVICE_UNAVAILABLE which will be used instead of this code.
     */
    SERVICE_TIMEOUT = -3,
    /** The requested feature is not supported by the Play Store on the current device. */
    FEATURE_NOT_SUPPORTED = -2,
    /** The app is not connected to the Play Store service via the Google Play Billing Library. */
    SERVICE_DISCONNECTED = -1,
    /** Success */
    OK = 0,
    /** Transaction was canceled by the user. */
    USER_CANCELED = 1,
    /** The service is currently unavailable. */
    SERVICE_UNAVAILABLE = 2,
    /** A user billing error occurred during processing. */
    BILLING_UNAVAILABLE = 3,
    /** The requested product is not available for purchase. */
    ITEM_UNAVAILABLE = 4,
    /** Error resulting from incorrect usage of the API. */
    DEVELOPER_ERROR = 5,
    /** Fatal error during the API action. */
    ERROR = 6,
    /** The purchase failed because the item is already owned. */
    ITEM_ALREADY_OWNED = 7,
    /** Requested action on the item failed since it is not owned by the user. */
    ITEM_NOT_OWNED = 8,
    /** A network error occurred during the operation. */
    NETWORK_ERROR = 12,
};

// https://developer.android.com/reference/com/android/billingclient/api/Purchase.PurchaseState
enum PurchaseState {
    // Purchase with unknown state.
    UNSPECIFIED_STATE = 0,
    // Purchase is completed.
    PURCHASED = 1,
    // Purchase is waiting for payment completion.
    PENDING = 2,
};

StoreError getStoreErrorWithBillingResponseCode(int responseCode, std::string description)
{
    StoreError error;
    error.description = description;
    switch(responseCode)
    {
        case OK:
            error.code = STORE_ERROR_NONE;
            break;
        case SERVICE_DISCONNECTED:
            error.code = STORE_ERROR_SERVICE_DISCONNECTED;
            break;
        case SERVICE_TIMEOUT:
        case SERVICE_UNAVAILABLE:
            error.code = STORE_ERROR_SERVICE_UNAVAILABLE;
            break;
        case FEATURE_NOT_SUPPORTED:
            error.code = STORE_ERROR_FEATURE_NOT_SUPPORTED;
            break;
        case USER_CANCELED:
            error.code = STORE_ERROR_CANCELLED;
            break;
        case BILLING_UNAVAILABLE:
            error.code = STORE_ERROR_CLIENT_INVALID;
            break;
        case ITEM_UNAVAILABLE:
            error.code = STORE_ERROR_STORE_PRODUCT_NOT_AVAILABLE;
            break;
        case ITEM_ALREADY_OWNED:
            error.code = STORE_ERROR_ITEM_ALREADY_OWNED;
            break;
        case ITEM_NOT_OWNED:
            error.code = STORE_ERROR_ITEM_NOT_OWNED;
            break;
        case NETWORK_ERROR:
            error.code = STORE_ERROR_NETWORK_ERROR;
            break;
        default:
            error.code = STORE_ERROR_UNKNOWN;
            break;
    }
    return error;
}

std::vector<GoogleBillingTransaction> getTransactionListWithPurchases(JNIEnv* env, jobjectArray purchases)
{
    std::vector<GoogleBillingTransaction> transactions;
    jsize len = env->GetArrayLength(purchases);
    if(len > 0)
    {
        jclass listCls = env->FindClass("java/util/List");
        CHECK_JNI_CLASS_RETURN_VAL(env, listCls, "java/util/List", transactions);
        jmethodID listGetMid = env->GetMethodID(listCls, "get", "(I)Ljava/lang/Object;");
        CHECK_JNI_METHODID_RETURN_VAL(env, listGetMid, "get", transactions);
        jmethodID listSizeMid = env->GetMethodID(listCls, "size", "()I");
        CHECK_JNI_METHODID_RETURN_VAL(env, listSizeMid, "size", transactions);
        env->DeleteLocalRef(listCls);

        jclass purchaseCls = env->FindClass("com/android/billingclient/api/Purchase");
        CHECK_JNI_CLASS_RETURN_VAL(env, purchaseCls, "com/android/billingclient/api/Purchase", transactions);
        jmethodID orderIdMid = env->GetMethodID(purchaseCls, "getOrderId", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, orderIdMid, "getOrderId", transactions);
        jmethodID productsMid = env->GetMethodID(purchaseCls, "getProducts", "()Ljava/util/List;");
        CHECK_JNI_METHODID_RETURN_VAL(env, productsMid, "getProducts", transactions);
        jmethodID purchaseTimeMid = env->GetMethodID(purchaseCls, "getPurchaseTime", "()J");
        CHECK_JNI_METHODID_RETURN_VAL(env, purchaseTimeMid, "getPurchaseTime", transactions);
        jmethodID purchaseStateMid = env->GetMethodID(purchaseCls, "getPurchaseState", "()I");
        CHECK_JNI_METHODID_RETURN_VAL(env, purchaseStateMid, "getPurchaseState", transactions);
        jmethodID isAcknowledgedMid = env->GetMethodID(purchaseCls, "isAcknowledged", "()Z");
        CHECK_JNI_METHODID_RETURN_VAL(env, isAcknowledgedMid, "isAcknowledged", transactions);
        jmethodID purchaseTokenMid = env->GetMethodID(purchaseCls, "getPurchaseToken", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, purchaseTokenMid, "getPurchaseToken", transactions);
        jmethodID packageNameMid = env->GetMethodID(purchaseCls, "getPackageName", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, packageNameMid, "getPackageName", transactions);
        jmethodID signatureMid = env->GetMethodID(purchaseCls, "getSignature", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, signatureMid, "getSignature", transactions);
        jmethodID originalJsonMid = env->GetMethodID(purchaseCls, "getOriginalJson", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, originalJsonMid, "getOriginalJson", transactions);
        jmethodID developerPayloadMid = env->GetMethodID(purchaseCls, "getDeveloperPayload", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, developerPayloadMid, "getDeveloperPayload", transactions);

        env->DeleteLocalRef(purchaseCls);

        for(jsize i = 0; i < len; ++i)
        {
            GoogleBillingTransaction trans;
            jobject purchaseObj = env->GetObjectArrayElement(purchases, i);

            jstring orderId = (jstring)env->CallObjectMethod(purchaseObj, orderIdMid);
            trans.transactionId = StringUtils::getStringUTFCharsJNI(env, orderId);
            env->DeleteLocalRef(orderId);

            jobject productListObj = (jobject)env->CallObjectMethod(purchaseObj, productsMid);
            jint productListSize = env->CallIntMethod(productListObj, listSizeMid);
            for (int32 i = 0; i < productListSize; i++)
            {
                jstring productId = (jstring)env->CallObjectMethod(productListObj, listGetMid, i);
                trans.productId = StringUtils::getStringUTFCharsJNI(env, productId);
                env->DeleteLocalRef(productId);
            }
            env->DeleteLocalRef(productListObj);

            jlong purchaseTime = env->CallLongMethod(purchaseObj, purchaseTimeMid);
            trans.transactionTime = static_cast<uint32>(purchaseTime / 1000);

            jboolean isAcknowledged = env->CallBooleanMethod(purchaseObj, isAcknowledgedMid);
            trans.isAcknowledged = isAcknowledged;

            jstring purchaseToken = (jstring)env->CallObjectMethod(purchaseObj, purchaseTokenMid);
            trans.purchaseToken = StringUtils::getStringUTFCharsJNI(env, purchaseToken);
            env->DeleteLocalRef(purchaseToken);

            jstring packageName = (jstring)env->CallObjectMethod(purchaseObj, packageNameMid);
            trans.packageName = StringUtils::getStringUTFCharsJNI(env, packageName);
            env->DeleteLocalRef(packageName);

            jstring signature = (jstring)env->CallObjectMethod(purchaseObj, signatureMid);
            trans.signature = StringUtils::getStringUTFCharsJNI(env, signature);
            env->DeleteLocalRef(signature);

            jstring originalJson = (jstring)env->CallObjectMethod(purchaseObj, originalJsonMid);
            trans.originalJson = StringUtils::getStringUTFCharsJNI(env, originalJson);
            env->DeleteLocalRef(originalJson);

            jstring developerPayload = (jstring)env->CallObjectMethod(purchaseObj, developerPayloadMid);
            trans.developerPayload = StringUtils::getStringUTFCharsJNI(env, developerPayload);
            env->DeleteLocalRef(developerPayload);

            jint purchaseState = env->CallIntMethod(purchaseObj,purchaseStateMid);
            switch(purchaseState)
            {
                case PurchaseState::PURCHASED:
                    trans.state = PaymentState::PAYMENT_STATE_PURCHASED;
                    break;
                case PurchaseState::PENDING:
                    trans.state = PaymentState::PAYMENT_STATE_DEFERRED;
                    break;
                default: // PurchaseState::UNSPECIFIED_STATE
                    trans.state = PaymentState::PAYMENT_STATE_UNSPECIFIED;
                    break;
            }

            env->DeleteLocalRef(purchaseObj);

            transactions.push_back(trans);

        }
    }

    return transactions;
}

std::vector<ProductInfo*> getProductListWithProductDetailsList(JNIEnv* env, jobjectArray productDetailsList)
{
    std::vector<ProductInfo*> products;
    jsize len = env->GetArrayLength(productDetailsList);
    if (len > 0) {
        jclass productDetailsCls = env->FindClass("com/android/billingclient/api/ProductDetails");
        CHECK_JNI_CLASS_RETURN_VAL(env, productDetailsCls, "com/android/billingclient/api/ProductDetails", products);
        jmethodID productIdMid = env->GetMethodID(productDetailsCls, "getProductId", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, productIdMid, "getProductId", products);
        jmethodID typeMid = env->GetMethodID(productDetailsCls, "getProductType", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, typeMid, "getProductType", products);
        jmethodID titleMid = env->GetMethodID(productDetailsCls, "getTitle", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, titleMid, "getTitle", products);
        jmethodID descriptionMid = env->GetMethodID(productDetailsCls, "getDescription", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, descriptionMid, "getDescription", products);
        jmethodID nameMid = env->GetMethodID(productDetailsCls, "getName", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, nameMid, "getName", products);
        jmethodID oneTimePurchaseOfferDetailsMid = env->GetMethodID(productDetailsCls, "getOneTimePurchaseOfferDetails", "()Lcom/android/billingclient/api/ProductDetails$OneTimePurchaseOfferDetails;");
        CHECK_JNI_METHODID_RETURN_VAL(env, oneTimePurchaseOfferDetailsMid, "getOneTimePurchaseOfferDetails", products);

        jclass oneTimePurchaseOfferDetailsCls = env->FindClass("com/android/billingclient/api/ProductDetails$OneTimePurchaseOfferDetails");
        CHECK_JNI_CLASS_RETURN_VAL(env, oneTimePurchaseOfferDetailsCls, "com/android/billingclient/api/ProductDetails$OneTimePurchaseOfferDetails", products);
        jmethodID formattedPriceMid = env->GetMethodID(oneTimePurchaseOfferDetailsCls, "getFormattedPrice", "()Ljava/lang/String;");
        CHECK_JNI_METHODID_RETURN_VAL(env, formattedPriceMid, "getFormattedPrice", products);
        env->DeleteLocalRef(oneTimePurchaseOfferDetailsCls);

        env->DeleteLocalRef(productDetailsCls);

        for(jsize i = 0; i < len; ++i)
        {
            GoogleBillingProduct* product = new GoogleBillingProduct();
            jobject productDetailsObj = env->GetObjectArrayElement(productDetailsList, i);

            jstring productId = (jstring)env->CallObjectMethod(productDetailsObj, productIdMid);
            product->productId = StringUtils::getStringUTFCharsJNI(env, productId);
            env->DeleteLocalRef(productId);

            jobject oneTimePurchaseOfferDetailsObj = (jobject)env->CallObjectMethod(productDetailsObj, oneTimePurchaseOfferDetailsMid);

            jstring formattedPrice = (jstring)env->CallObjectMethod(oneTimePurchaseOfferDetailsObj, formattedPriceMid);
            product->localizedPrice = StringUtils::getStringUTFCharsJNI(env, formattedPrice);
            env->DeleteLocalRef(formattedPrice);

            env->DeleteLocalRef(oneTimePurchaseOfferDetailsObj);

            jstring name = (jstring)env->CallObjectMethod(productDetailsObj, nameMid);
            product->localizedTitle = StringUtils::getStringUTFCharsJNI(env, name);
            env->DeleteLocalRef(name);

            jstring description = (jstring)env->CallObjectMethod(productDetailsObj, descriptionMid);
            product->localizedDescription = StringUtils::getStringUTFCharsJNI(env, description);
            env->DeleteLocalRef(description);

            jstring type = (jstring)env->CallObjectMethod(productDetailsObj, typeMid);
            product->type = StringUtils::getStringUTFCharsJNI(env, type);
            env->DeleteLocalRef(type);

            env->DeleteLocalRef(productDetailsObj);

            products.push_back(product);
        }
    }

    return products;
}

extern "C" {
    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_billing_BillingManager_onQueryPurchasesFinished(JNIEnv *env, jobject thiz, jint responseCode, jstring debugMessage, jobjectArray purchases)
    {
        std::vector<GoogleBillingTransaction> transactions;
        if(responseCode == BillingResponseCode::OK)
            transactions = getTransactionListWithPurchases(env, purchases);

        std::string description = StringUtils::getStringUTFCharsJNI(env, debugMessage);
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([transactions, responseCode, description]() mutable {
            if(s_instance)
            {
                if(responseCode == BillingResponseCode::OK)
                    s_instance->onQueryPurchasesSuccess(transactions);
                else
                    s_instance->onQueryPurchasesFailed(getStoreErrorWithBillingResponseCode(responseCode, description));
            }
        });
    }

    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_billing_BillingManager_onPurchasesUpdated(JNIEnv* env, jobject thiz, jint responseCode, jstring debugMessage, jobjectArray purchases) {
        std::vector<GoogleBillingTransaction> transactions;
        if(responseCode == BillingResponseCode::OK)
            transactions = getTransactionListWithPurchases(env, purchases);

        std::string description = StringUtils::getStringUTFCharsJNI(env, debugMessage);
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([transactions, responseCode, description]() mutable {
            if(s_instance)
            {
                if(responseCode == BillingResponseCode::OK)
                    s_instance->onPurchasesUpdated(transactions);
                else
                    s_instance->onPurchasesFailed(
                            getStoreErrorWithBillingResponseCode(responseCode, description));
            }
        });
    }

    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_billing_BillingManager_onProductDetailsResponse(JNIEnv* env, jobject thiz, jint responseCode, jstring debugMessage, jobjectArray productDetailsList) {
        std::vector<ProductInfo*> products;
        if(responseCode == BillingResponseCode::OK)
            products = getProductListWithProductDetailsList(env, productDetailsList);

        std::string description = StringUtils::getStringUTFCharsJNI(env, debugMessage);
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([products, responseCode, description]() {
            if(s_instance)
            {
                if(responseCode == BillingResponseCode::OK)
                    s_instance->onProductDetailsRequestSuccess(products);
                else
                    s_instance->onProductDetailsRequestFailed(getStoreErrorWithBillingResponseCode(responseCode, description));
            }

        });
    }

    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_billing_BillingManager_onAcknowledgePurchaseResponse(JNIEnv* env, jobject thiz, jint responseCode, jstring debugMessage, jstring productIdJs, jstring transactionIdJs, jstring purchaseTokenJs) {
        std::string description = StringUtils::getStringUTFCharsJNI(env, debugMessage);
        std::string productId = StringUtils::getStringUTFCharsJNI(env, productIdJs);
        std::string transactionId = StringUtils::getStringUTFCharsJNI(env, transactionIdJs);
        std::string purchaseToken = StringUtils::getStringUTFCharsJNI(env, purchaseTokenJs);
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([productId, transactionId, purchaseToken, responseCode, description]() mutable {
            if(s_instance)
            {
                if(responseCode == BillingResponseCode::OK)
                    s_instance->onAcknowledgePurchaseSuccess(productId, transactionId, purchaseToken);
                else
                    s_instance->onAcknowledgePurchaseFailed(productId, transactionId,
                                                            getStoreErrorWithBillingResponseCode(responseCode, description));
            }
        });
    }

    JNIEXPORT void
    JNICALL Java_io_gamen_snowfight_billing_BillingManager_onConsumeResponse(JNIEnv* env, jobject thiz, jint responseCode, jstring debugMessage, jstring productIdJs, jstring transactionIdJs, jstring purchaseTokenJs) {
        std::string description = StringUtils::getStringUTFCharsJNI(env, debugMessage);
        std::string productId = StringUtils::getStringUTFCharsJNI(env, productIdJs);
        std::string transactionId = StringUtils::getStringUTFCharsJNI(env, transactionIdJs);
        std::string purchaseToken = StringUtils::getStringUTFCharsJNI(env, purchaseTokenJs);
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([productId, transactionId, purchaseToken, responseCode, description]() mutable {
            if(s_instance)
            {
                if(responseCode == BillingResponseCode::OK)
                    s_instance->onConsumeSuccess(productId, transactionId, purchaseToken);
                else
                    s_instance->onConsumeFailed(productId, transactionId,
                                                getStoreErrorWithBillingResponseCode(responseCode, description));
            }
        });
    }

} // extern "C"

const char* BASE_64_ENCODED_PUBLIC_KEY = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlwusEB8kwkmZHrx4Pfg/COfmBIM5FwZJxnns5+jCvIqr3TsSG1eqLcuRu5VUuFSyBYHmdhIVDH30SejaoUMsmHaDMrdSJngO7U8P0kraEzQBUGbua0CaklnNEZ2NoMNS50sAECZb9OqUceUKKCnoqPuAL+mozbnO6rlrxpyoj7T9Mybb0bE5QzHJsNKuXns2wcCyUZae1hXUtBw/lVr3OOMiFuGB2/HM1nQsDPwPEtA4YHxw+DRhrNqQSVA82Ai13XgCVJ+8CiaMJgEozc1dI/ZztNi0jbacyAjvUeoatvT9Buj7o2X3DqQ1empfBGtVIXnU9lRj//hNe8FZEBlsUwIDAQAB";

StoreImpl::StoreImpl() :
        m_billingMgrJObj(NULL),
        m_isValidatingProductIds(false),
        m_isProductIdsValidated(false),
        m_isPurchasing(false),
        m_isTransactionObserverRegistered(false)
{
    s_instance = this;

    this->initBillingManager();
}

StoreImpl::~StoreImpl()
{
    s_instance = nullptr;
    for(auto it = m_products.begin(); it != m_products.end();)
    {
        ProductInfo* product = *it;
        it = m_products.erase(it);
        CC_SAFE_DELETE(product);
    }

    this->destroyBillingManager();
}

void StoreImpl::initBillingManager()
{
    JNIEnv* env = NULL;
    jclass billingMgrCls = NULL;
    jobject billingMgrObj = NULL;
    do{
        env = JniHelper::getEnv();
        if (!env) {
            LOGE("JNI: Failed to get JNIEnv");
            break;
        }

        billingMgrCls = env->FindClass(billingManagerClassName.c_str());
        if (!billingMgrCls) {
            LOGE("JNI:Failed to find class %s", billingManagerClassName.c_str());
            break;
        }

        jmethodID constructorMid = env->GetMethodID(billingMgrCls, "<init>", "(Landroid/app/Activity;)V");
        if (!constructorMid) {
            LOGE("JNI:Failed to get constructor method id of class %s", billingManagerClassName.c_str());
            break;
        }

        billingMgrObj = env->NewObject(billingMgrCls, constructorMid, JniHelper::getActivity());
        if(!billingMgrObj)
        {
            LOGE("JNI:Failed to new object %s", billingManagerClassName.c_str());
            break;
        }
    } while(false);

    if(env)
    {
        if(!env->ExceptionCheck())
            m_billingMgrJObj = env->NewGlobalRef(billingMgrObj);
        else
            env->ExceptionClear();
    }

    env->DeleteLocalRef(billingMgrCls);
    env->DeleteLocalRef(billingMgrObj);
}

void StoreImpl::destroyBillingManager() {
    if(m_billingMgrJObj)
    {
        JniMethodInfo t;
        std::string methodName = "destroy";
        std::string signature = "()V";
        if(JniHelper::getMethodInfo(t, billingManagerClassName.c_str(),methodName.c_str(), signature.c_str()))
        {
            t.env->CallVoidMethod(m_billingMgrJObj, t.methodID);;
            t.env->DeleteLocalRef(t.classID);
        }
        else
            LOGE("Failed to find java method. Class name: %s, method name: %s, signature: %s ",  billingManagerClassName.c_str(), methodName.c_str(), signature.c_str());

        JNIEnv* env = JniHelper::getEnv();
        if(env)
            env->DeleteGlobalRef(m_billingMgrJObj);
        m_billingMgrJObj = NULL;
    }
}

void StoreImpl::validateProductIds(std::vector<std::string> const& productIds)
{
    cocos2d::JniMethodInfo t;
    std::string methodName = "validateProductIds";
    std::string signature = "(Ljava/lang/String;[Ljava/lang/Object;)V";
    if (cocos2d::JniHelper::getMethodInfo(t, billingManagerClassName.c_str(), methodName.c_str(), signature.c_str())) {
        jstring productTypeJs = StringUtils::newStringUTFJNI(t.env, INAPP);
        jclass stringCls = t.env->FindClass("java/lang/String");
        jobjectArray productIdArray = t.env->NewObjectArray(static_cast<jsize>(productIds.size()), stringCls, NULL);
        bool noerror = false;
        if(productIdArray)
        {
            for(size_t i = 0; i < productIds.size(); ++i)
            {
                std::string idStr = productIds[i];
                jstring idJs = StringUtils::newStringUTFJNI(t.env, idStr.c_str());
                t.env->SetObjectArrayElement(productIdArray, i, idJs);
                t.env->DeleteLocalRef(idJs);
            }
            noerror = !t.env->ExceptionCheck();
        }

        if(noerror)
            t.env->CallVoidMethod(m_billingMgrJObj, t.methodID, productTypeJs, productIdArray);
        else
        {
            t.env->ExceptionClear();
            LOGE("JNI: Failed to call method. method name: %s", methodName.c_str());
        }

        t.env->DeleteLocalRef(productIdArray);
        t.env->DeleteLocalRef(productTypeJs);
        t.env->DeleteLocalRef(stringCls);
        t.env->DeleteLocalRef(t.classID);

        m_isValidatingProductIds = true;
        m_isProductIdsValidated = false;
    }
    else {
        LOGE("Failed to find java method. Class name: %s, method name: %s, signature: %s ",  billingManagerClassName.c_str(), methodName.c_str(), signature.c_str());
    }
}

ProductInfo const* StoreImpl::getProductById(std::string const& productId) const {
    auto it = std::find_if(m_products.begin(), m_products.end(), [&productId](ProductInfo const* p){
        return p->productId == productId;
    });
    if(it != std::end(m_products))
        return (*it);
    return nullptr;
}

bool StoreImpl::requestPayment(std::string const& productId, int32 quantity)
{
    GoogleBillingProduct const* product = static_cast<GoogleBillingProduct const*>(this->getProductById(productId));
    if(!product)
        return false;

    cocos2d::JniMethodInfo t;
    std::string methodName = "initiatePurchaseFlow";
    std::string signature = "(Ljava/lang/String;)V";
    if (cocos2d::JniHelper::getMethodInfo(t, billingManagerClassName.c_str(), methodName.c_str(), signature.c_str())) {
        jstring productIdJs = StringUtils::newStringUTFJNI(t.env, product->productId);

        m_isPurchasing = true;
        t.env->CallVoidMethod(m_billingMgrJObj, t.methodID, productIdJs);

        t.env->DeleteLocalRef(productIdJs);
        t.env->DeleteLocalRef(t.classID);
        return true;
    }
    else {
        LOGE("JNI: Failed to find java method. Class name: %s, method name: %s, signature: %s ",  billingManagerClassName.c_str(), methodName.c_str(), signature.c_str());
        return false;
    }
}

bool StoreImpl::hasDeferredTransaction(std::string const& productId)
{
    return false;
}

void StoreImpl::addListener(StoreListener* listener)
{
    auto it = std::find(m_listeners.begin(), m_listeners.end(), listener);
    if(it == std::end(m_listeners))
        m_listeners.push_back(listener);
}

void StoreImpl::removeListener(StoreListener* listener)
{
    m_listeners.erase(std::remove(m_listeners.begin(), m_listeners.end(), listener), m_listeners.end());
}

void StoreImpl::registerTransactionObserver()
{
    if(m_isTransactionObserverRegistered)
        return;

    this->queryCachedPurchases(INAPP);
    m_isTransactionObserverRegistered = true;
}

void StoreImpl::unregisterTransactionObserver()
{
    m_isTransactionObserverRegistered = false;
}

void StoreImpl::acknowledgeTransaction(PaymentTransaction const& transaction)
{
    auto it = m_acknowledgingTransactions.find(transaction.transactionId);
    if(it != m_acknowledgingTransactions.end())
        return;

    GoogleBillingTransaction const& gbTrans = static_cast<GoogleBillingTransaction const&>(transaction);

    cocos2d::JniMethodInfo t;
    std::string methodName = "acknowledgePurchase";
    std::string signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
    if (cocos2d::JniHelper::getMethodInfo(t, billingManagerClassName.c_str(), methodName.c_str(), signature.c_str())) {
        m_acknowledgingTransactions.emplace(transaction.transactionId, gbTrans);

        jstring productIdJs = StringUtils::newStringUTFJNI(t.env, gbTrans.productId.c_str());
        jstring transactionIdJs = StringUtils::newStringUTFJNI(t.env, gbTrans.transactionId.c_str());
        jstring purchaseTokenJs = StringUtils::newStringUTFJNI(t.env, gbTrans.purchaseToken.c_str());
        jstring developerPayloadJs = StringUtils::newStringUTFJNI(t.env, gbTrans.developerPayload.c_str());
        t.env->CallVoidMethod(m_billingMgrJObj, t.methodID, productIdJs, transactionIdJs, purchaseTokenJs, developerPayloadJs);

        t.env->DeleteLocalRef(productIdJs);
        t.env->DeleteLocalRef(transactionIdJs);
        t.env->DeleteLocalRef(purchaseTokenJs);
        t.env->DeleteLocalRef(developerPayloadJs);
        t.env->DeleteLocalRef(t.classID);
    }
    else {
        LOGE("JNI: Failed to find java method. Class name: %s, method name: %s, signature: %s ",  billingManagerClassName.c_str(), methodName.c_str(), signature.c_str());
    }
}

void StoreImpl::consume(PaymentTransaction const& transaction)
{
    auto it = m_acknowledgingTransactions.find(transaction.transactionId);
    if(it != m_acknowledgingTransactions.end())
        return;

    GoogleBillingTransaction const& gbTrans = static_cast<GoogleBillingTransaction const&>(transaction);

    cocos2d::JniMethodInfo t;
    std::string methodName = "consumeAsync";
    std::string signature = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
    if (cocos2d::JniHelper::getMethodInfo(t, billingManagerClassName.c_str(), methodName.c_str(), signature.c_str())) {
        m_acknowledgingTransactions.emplace(transaction.transactionId, gbTrans);

        jstring productIdJs = StringUtils::newStringUTFJNI(t.env, gbTrans.productId.c_str());
        jstring transactionIdJs = StringUtils::newStringUTFJNI(t.env, gbTrans.transactionId.c_str());
        jstring purchaseTokenJs = StringUtils::newStringUTFJNI(t.env, gbTrans.purchaseToken.c_str());
        jstring developerPayloadJs = StringUtils::newStringUTFJNI(t.env, gbTrans.developerPayload.c_str());
        t.env->CallVoidMethod(m_billingMgrJObj, t.methodID, productIdJs, transactionIdJs, purchaseTokenJs, developerPayloadJs);

        t.env->DeleteLocalRef(productIdJs);
        t.env->DeleteLocalRef(transactionIdJs);
        t.env->DeleteLocalRef(purchaseTokenJs);
        t.env->DeleteLocalRef(developerPayloadJs);
        t.env->DeleteLocalRef(t.classID);
    }
    else {
        LOGE("JNI: Failed to find java method. Class name: %s, method name: %s, signature: %s ",  billingManagerClassName.c_str(), methodName.c_str(), signature.c_str());
    }
}

void StoreImpl::onProductDetailsRequestSuccess(std::vector<ProductInfo*> const& products)
{
    m_isProductIdsValidated = true;
    m_isValidatingProductIds = false;
    m_products = products;

    for(StoreListener* listen: m_listeners)
        listen->onProductsRequestFinished();
}
void StoreImpl::onProductDetailsRequestFailed(StoreError const& error)
{
    m_isValidatingProductIds = false;
    m_isProductIdsValidated = false;
    for(StoreListener* listen: m_listeners)
        listen->onProductsRequestFailed(error);
}

void StoreImpl::queryCachedPurchases(std::string const& productType)
{
    cocos2d::JniMethodInfo t;
    std::string methodName = "queryCachedPurchases";
    std::string signature = "(Ljava/lang/String;)V";
    if (cocos2d::JniHelper::getMethodInfo(t, billingManagerClassName.c_str(), methodName.c_str(), signature.c_str())) {
        jstring productTypeJs = StringUtils::newStringUTFJNI(t.env, productType.c_str());
        t.env->CallVoidMethod(m_billingMgrJObj, t.methodID, productTypeJs);

        t.env->DeleteLocalRef(productTypeJs);
        t.env->DeleteLocalRef(t.classID);
    }
    else {
        LOGE("JNI: Failed to find java method. Class name: %s, method name: %s, signature: %s ",  billingManagerClassName.c_str(), methodName.c_str(), signature.c_str());
    }
}

bool StoreImpl::verifyTransaction(GoogleBillingTransaction const &transaction)
{
	// Verify purchase data
#if NS_DEBUG
    bool isValid = true;
    if(transaction.productId.find("android.test") == std::string::npos)
        isValid = verifyPurchase(BASE_64_ENCODED_PUBLIC_KEY, transaction.originalJson, transaction.signature);
#else
    bool isValid = verifyPurchase(BASE_64_ENCODED_PUBLIC_KEY, transaction.originalJson, transaction.signature);
#endif
    return isValid;
}

void StoreImpl::onPurchasesUpdated(std::vector<GoogleBillingTransaction>& transactions)
{
    m_isPurchasing = false;

    for(auto& transaction: transactions)
    {
        switch(transaction.state)
        {
            case PAYMENT_STATE_PURCHASED:
            {
                bool verified = this->verifyTransaction(transaction);
                if(!verified)
                {
                    transaction.state = PAYMENT_STATE_FAILED;
                    transaction.error.description = "Purchase details validation failed.";
                    transaction.error.code = STORE_ERROR_VALIDATION_FAILED;
                }

                for(StoreListener* listen: m_listeners)
                    listen->onPaymentTransactionUpdated(transaction);

                break;
            }
            default:
            {
                for(StoreListener* listen: m_listeners)
                    listen->onPaymentTransactionUpdated(transaction);
                break;
            }
        }
    }
}

void StoreImpl::onPurchasesFailed(StoreError const& error)
{
    m_isPurchasing = false;

    for(StoreListener* listen: m_listeners)
        listen->onPurchasesFailed(error);

}

void StoreImpl::onAcknowledgePurchaseSuccess(std::string const& productId, std::string const& transactionId, std::string const& purchaseToken)
{
    auto it = m_acknowledgingTransactions.find(transactionId);
    if(it != m_acknowledgingTransactions.end())
    {
        GoogleBillingTransaction const& transaction = (*it).second;
        for(StoreListener* listen: m_listeners)
            listen->onAcknowledgePurchaseFinished(transaction);
        m_acknowledgingTransactions.erase(it);
    }
    else
    {
        StoreError error;
        error.code = STORE_ERROR_UNKNOWN;
        this->onAcknowledgePurchaseFailed(productId, transactionId, error);
    }
}
void StoreImpl::onAcknowledgePurchaseFailed(std::string const& productId, std::string const& transactionId, StoreError const& error)
{
    for(StoreListener* listen: m_listeners)
        listen->onAcknowledgePurchaseFailed(error);

    auto it = m_acknowledgingTransactions.find(transactionId);
    if(it != m_acknowledgingTransactions.end())
        m_acknowledgingTransactions.erase(it);
}

void StoreImpl::onConsumeSuccess(std::string const& productId, std::string const& transactionId, std::string const& purchaseToken)
{
    auto it = m_acknowledgingTransactions.find(transactionId);
    if(it != m_acknowledgingTransactions.end())
    {
        GoogleBillingTransaction const& transaction = (*it).second;
        for(StoreListener* listen: m_listeners)
            listen->onConsumeFinished(transaction);
        m_acknowledgingTransactions.erase(it);
    }
    else
    {
        StoreError error;
        error.code = STORE_ERROR_UNKNOWN;
        this->onConsumeFailed(productId, transactionId, error);
    }
}

void StoreImpl::onConsumeFailed(std::string const& productId, std::string const& transactionId, StoreError const& error)
{
    for(StoreListener* listen: m_listeners)
        listen->onConsumeFailed(error);

    auto it = m_acknowledgingTransactions.find(transactionId);
    if(it != m_acknowledgingTransactions.end())
        m_acknowledgingTransactions.erase(it);
}

void StoreImpl::onQueryPurchasesSuccess(std::vector<GoogleBillingTransaction>& transactions)
{
    for(auto& transaction: transactions)
    {
        switch(transaction.state)
        {
            case PAYMENT_STATE_PURCHASED:
            {
                bool verified = this->verifyTransaction(transaction);
                if(verified)
                {
                    // Cached purchase transaction is viewed as a restored purchase
                    transaction.state = PAYMENT_STATE_RESTORED;

                    for(StoreListener* listen: m_listeners)
                        listen->onPaymentTransactionUpdated(transaction);

                }
                break;
            }
            default:
                break;
        }
    }
}

void StoreImpl::onQueryPurchasesFailed(StoreError const& error)
{

}

void StoreImpl::onEnterForeground()
{
    if(!m_isPurchasing)
        this->queryCachedPurchases(INAPP);
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
