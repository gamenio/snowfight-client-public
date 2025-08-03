package io.gamen.snowfight;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.telephony.TelephonyManager;

public final class ConnectivityHelper {
    private static final int NETWORK_CLASS_UNKNOWN = 0;
    private static final int NETWORK_CLASS_2G = 1;
    private static final int NETWORK_CLASS_3G = 2;
    private static final int NETWORK_CLASS_4G = 3;
    private static final int NETWORK_CLASS_5G = 4;
    private static final int NETWORK_CLASS_WIFI = 8;

    public static int getNetworkClass() {
        final AppActivity appActivity = (AppActivity)AppActivity.getContext();
        if(appActivity == null)
            return NETWORK_CLASS_UNKNOWN;

        int networkClass = NETWORK_CLASS_UNKNOWN;
        ConnectivityManager cm = (ConnectivityManager) appActivity.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo info = cm.getActiveNetworkInfo();
        if (info == null || !info.isConnected())
            networkClass = NETWORK_CLASS_UNKNOWN;
        else if (info.getType() == ConnectivityManager.TYPE_WIFI)
            networkClass = NETWORK_CLASS_WIFI;
        else if (info.getType() == ConnectivityManager.TYPE_MOBILE) {
            int networkType = info.getSubtype();
            switch (networkType) {
                case TelephonyManager.NETWORK_TYPE_GPRS:
                case TelephonyManager.NETWORK_TYPE_EDGE:
                case TelephonyManager.NETWORK_TYPE_CDMA:
                case TelephonyManager.NETWORK_TYPE_1xRTT:
                case TelephonyManager.NETWORK_TYPE_IDEN:
                case TelephonyManager.NETWORK_TYPE_GSM:
                    networkClass = NETWORK_CLASS_2G;
                    break;
                case TelephonyManager.NETWORK_TYPE_UMTS:
                case TelephonyManager.NETWORK_TYPE_EVDO_0:
                case TelephonyManager.NETWORK_TYPE_EVDO_A:
                case TelephonyManager.NETWORK_TYPE_HSDPA:
                case TelephonyManager.NETWORK_TYPE_HSUPA:
                case TelephonyManager.NETWORK_TYPE_HSPA:
                case TelephonyManager.NETWORK_TYPE_EVDO_B:
                case TelephonyManager.NETWORK_TYPE_EHRPD:
                case TelephonyManager.NETWORK_TYPE_HSPAP:
                case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                    networkClass = NETWORK_CLASS_3G;
                    break;
                case TelephonyManager.NETWORK_TYPE_LTE:
                case TelephonyManager.NETWORK_TYPE_IWLAN:
                case 19: // TelephonyManager.NETWORK_TYPE_LTE_CA
                    networkClass = NETWORK_CLASS_4G;
                    break;
                case 20: // TelephonyManager.NETWORK_TYPE_NR:
                    networkClass =  NETWORK_CLASS_5G;
                    break;
                default: // TelephonyManager.NETWORK_TYPE_UNKNOWN
                    networkClass = NETWORK_CLASS_UNKNOWN;
                    break;
            }
        }

        return networkClass;
    }
}
