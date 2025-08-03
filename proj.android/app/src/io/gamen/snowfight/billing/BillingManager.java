package io.gamen.snowfight.billing;

import android.app.Activity;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryPurchasesParams;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

public class BillingManager implements PurchasesUpdatedListener {
    private static final String TAG = "BillingManager";

    private BillingClient mBillingClient;

    private final Handler mUiThreadHandler = new Handler(Looper.getMainLooper());

    private final Activity mActivity;

    private final HashMap<String, ProductDetails> mProductDetailsMap = new HashMap<>();

    private interface BillingServiceRequest {
        void onServiceSetupFinished(BillingResult billingResult);
    }

    private LinkedList<BillingServiceRequest> mServiceRequestQueue = new LinkedList<>();

    private native void onQueryPurchasesFinished(int responseCode, String debugMessage, Purchase[] purchases);
    private native void onPurchasesUpdated(int responseCode, String debugMessage, Purchase[] purchases);
    private native void onProductDetailsResponse(int responseCode, String debugMessage, ProductDetails[] productDetailsList);
    private native void onAcknowledgePurchaseResponse(int responseCode, String debugMessage, String productId, String transactionId, String purchaseToken);
    private native void onConsumeResponse(int responseCode, String debugMessage, String productId, String transactionId, String purchaseToken);

    public void validateProductIds(final String productType, Object[] productIds) {
        Log.d(TAG, "validateProductIds length:" + productIds.length);

        final ArrayList<String> productIdList = new ArrayList<>();
        for (Object idStr : productIds) {
            productIdList.add((String) idStr);
        }

        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                queryProductDetailsAsync(productType, productIdList, new ProductDetailsResponseListener() {
                    @Override
                    public void onProductDetailsResponse(BillingResult billingResult, List<ProductDetails> productDetailsList) {
                        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                            mProductDetailsMap.clear();
                            for(ProductDetails details: productDetailsList) {
                                mProductDetailsMap.put(details.getProductId(), details);
                            }
                        }
                        else
                        {
                            if(productDetailsList == null)
                                productDetailsList = new ArrayList<>();
                        }
                        Log.d(TAG, "onProductDetailsResponse code:" + billingResult.getResponseCode() + " list:" + productDetailsList.size());
                        BillingManager.this.onProductDetailsResponse(billingResult.getResponseCode(), billingResult.getDebugMessage(), productDetailsList.toArray(new ProductDetails[0]));
                    }
                });
            }
        };
        mUiThreadHandler.post(runnable);
    }

    public void acknowledgePurchase(final String productId, final String transactionId, final String purchaseToken, final String developerPayload) {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                acknowledgePurchase(purchaseToken, developerPayload, new AcknowledgePurchaseResponseListener() {
                    @Override
                    public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                        BillingManager.this.onAcknowledgePurchaseResponse(billingResult.getResponseCode(), billingResult.getDebugMessage(), productId, transactionId, purchaseToken);
                    }
                });
            }
        };
        mUiThreadHandler.post(runnable);
    }

    public void consumeAsync(final String productId, final String transactionId, final String purchaseToken, final String developerPayload) {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                consumeAsync(purchaseToken, developerPayload, new ConsumeResponseListener() {
                    @Override
                    public void onConsumeResponse(BillingResult billingResult, String purchaseToken) {
                        BillingManager.this.onConsumeResponse(billingResult.getResponseCode(), billingResult.getDebugMessage(), productId, transactionId, purchaseToken);
                    }
                });
            }
        };
        mUiThreadHandler.post(runnable);
    }

    public void queryCachedPurchases(final String productType) {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                queryPurchases(productType);
            }
        };
        mUiThreadHandler.post(runnable);
    }


    public void initiatePurchaseFlow(final String productId) {
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                launchBillingFlow(productId);
            }
        };
        mUiThreadHandler.post(runnable);

    }

    public BillingManager(Activity activity) {
        Log.d(TAG, "Creating Billing client.");
        mActivity = activity;
        mBillingClient = BillingClient.newBuilder(mActivity)
                .enablePendingPurchases(PendingPurchasesParams.newBuilder().enableOneTimeProducts().build())
                .setListener(this)
                .build();

    }

    @Override
    public void onPurchasesUpdated(BillingResult billingResult, List<Purchase> purchases) {
        if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK) {
            Log.w(TAG, "onPurchasesUpdated() got failed resultCode: " + billingResult.getResponseCode());
            if(purchases == null)
                purchases = new ArrayList<>();
        }
        this.onPurchasesUpdated(billingResult.getResponseCode(), billingResult.getDebugMessage(), purchases.toArray(new Purchase[0]));
    }

    public void launchBillingFlow(final String productId) {
        BillingServiceRequest purchaseFlowRequest = new BillingServiceRequest() {
            @Override
            public void onServiceSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    ProductDetails productDetails = mProductDetailsMap.get(productId);
                    if (productDetails != null) {
                        List<BillingFlowParams.ProductDetailsParams> productDetailsParamsList = new ArrayList<>();
                        BillingFlowParams.ProductDetailsParams params = BillingFlowParams.ProductDetailsParams.newBuilder()
                                .setProductDetails(productDetails)
                                .build();
                        productDetailsParamsList.add(params);

                        BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                                .setProductDetailsParamsList(productDetailsParamsList)
                                .build();
                        billingResult = mBillingClient.launchBillingFlow(mActivity, billingFlowParams);
                        if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                            BillingManager.this.onPurchasesUpdated(billingResult, null);
                    } else {
                        billingResult = BillingResult.newBuilder()
                                .setResponseCode(BillingClient.BillingResponseCode.DEVELOPER_ERROR)
                                .setDebugMessage("No ProductDetails found")
                                .build();
                        BillingManager.this.onPurchasesUpdated(billingResult, null);
                    }

                } else {
                    BillingManager.this.onPurchasesUpdated(billingResult, null);
                }
            }
        };

        executeServiceRequest(purchaseFlowRequest);
    }

    public Context getContext() {
        return mActivity;
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
    }

    public void destroy() {
        Log.d(TAG, "Destroying the manager.");

        if (mBillingClient != null && mBillingClient.isReady()) {
            mBillingClient.endConnection();
            mBillingClient = null;
        }
    }

    public void queryProductDetailsAsync(final String productType, final List<String> productIdList,
                                         final ProductDetailsResponseListener listener) {
        // Creating a runnable from the request to use it inside our connection retry policy below
        BillingServiceRequest queryRequest = new BillingServiceRequest() {
            @Override
            public void onServiceSetupFinished(BillingResult billingResult) {
                if(billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    List<QueryProductDetailsParams.Product> productList = new ArrayList<>();
                    for(String productId: productIdList)
                    {
                        QueryProductDetailsParams.Product product = QueryProductDetailsParams.Product.newBuilder()
                                .setProductId(productId)
                                .setProductType(productType)
                                .build();
                        productList.add(product);
                    }

                    QueryProductDetailsParams params = QueryProductDetailsParams.newBuilder()
                            .setProductList(productList)
                            .build();

                    mBillingClient.queryProductDetailsAsync(params, listener);
                } else {
                    listener.onProductDetailsResponse(billingResult, new ArrayList<ProductDetails>());
                }
            }
        };

        executeServiceRequest(queryRequest);
    }

    private void acknowledgePurchase(final String purchaseToken, final String developerPayload, final AcknowledgePurchaseResponseListener listener) {
        BillingServiceRequest acknowledgeRequest = new BillingServiceRequest() {
            @Override
            public void onServiceSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    AcknowledgePurchaseParams acknowledgePurchaseParams =
                            AcknowledgePurchaseParams.newBuilder()
                                    .setPurchaseToken(purchaseToken)
                                    .build();
                    mBillingClient.acknowledgePurchase(acknowledgePurchaseParams, listener);
                } else {
                    listener.onAcknowledgePurchaseResponse(billingResult);
                }
            }
        };

        executeServiceRequest(acknowledgeRequest);
    }

    private void consumeAsync(final String purchaseToken, final String developerPayload, final ConsumeResponseListener listener)  {
        BillingServiceRequest consumeRequest = new BillingServiceRequest() {
            @Override
            public void onServiceSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    ConsumeParams consumeParams =
                            ConsumeParams.newBuilder()
                                    .setPurchaseToken(purchaseToken)
                                    .build();
                    mBillingClient.consumeAsync(consumeParams, listener);
                } else {
                    listener.onConsumeResponse(billingResult, purchaseToken);
                }
            }
        };

        executeServiceRequest(consumeRequest);
    }

    public void queryPurchases(final String productType) {
        BillingServiceRequest queryToExecute = new BillingServiceRequest() {
            @Override
            public void onServiceSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    final long time = System.currentTimeMillis();
                    QueryPurchasesParams params = QueryPurchasesParams.newBuilder()
                            .setProductType(productType)
                            .build();
                    mBillingClient.queryPurchasesAsync(params, new PurchasesResponseListener() {
                        @Override
                        public void onQueryPurchasesResponse(BillingResult billingResult, List<Purchase> purchases) {
                            Log.i(TAG, "Querying purchases elapsed time: " + (System.currentTimeMillis() - time) + "ms");
                            if (purchases == null)
                                purchases = new ArrayList<>();
                            BillingManager.this.onQueryPurchasesFinished(billingResult.getResponseCode(), billingResult.getDebugMessage(), purchases.toArray(new Purchase[0]));
                        }
                    });

                } else {
                    BillingManager.this.onQueryPurchasesFinished(billingResult.getResponseCode(), billingResult.getDebugMessage(), new Purchase[0]);
                }
            }
        };

        executeServiceRequest(queryToExecute);
    }

    public void startServiceConnection() {
        if(mBillingClient.getConnectionState() == BillingClient.ConnectionState.CONNECTING ||  mBillingClient.getConnectionState() == BillingClient.ConnectionState.CONNECTED)
            return;

        mBillingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
                Log.d(TAG, "Setup finished. Response code: " + billingResult.getResponseCode());

                while (!mServiceRequestQueue.isEmpty())
                    mServiceRequestQueue.poll().onServiceSetupFinished(billingResult);
            }

            @Override
            public void onBillingServiceDisconnected() {
                while (!mServiceRequestQueue.isEmpty())
                {
                    BillingResult result = BillingResult.newBuilder()
                            .setResponseCode(BillingClient.BillingResponseCode.SERVICE_DISCONNECTED)
                            .build();
                    mServiceRequestQueue.poll().onServiceSetupFinished(result);
                }

            }
        });
    }

    private void executeServiceRequest(BillingServiceRequest requset) {
        if (mBillingClient.getConnectionState() == BillingClient.ConnectionState.CONNECTED) {
            BillingResult result = BillingResult.newBuilder()
                    .setResponseCode(BillingClient.BillingResponseCode.OK)
                    .build();
            requset.onServiceSetupFinished(result);
        } else {
            mServiceRequestQueue.add(requset);
            // If billing service was disconnected, we try to reconnect 1 time.
            // (feel free to introduce your retry policy here).
            startServiceConnection();
        }
    }
}

