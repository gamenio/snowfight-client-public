package io.gamen.snowfight;

import android.app.Dialog;

import com.google.android.gms.common.ConnectionResult;
import com.google.android.gms.common.GoogleApiAvailability;

public final class GoogleServicesHelper {
    public static int checkGooglePlayServices() {
        AppActivity appActivity = (AppActivity) AppActivity.getContext();
        int errorCode = ConnectionResult.SUCCESS;
        if (appActivity != null) {
            errorCode = GoogleApiAvailability.getInstance().isGooglePlayServicesAvailable(appActivity);
        } else {
            errorCode = ConnectionResult.INTERNAL_ERROR;
        }

        return errorCode;
    }

    public static void showErrorDialog(final int errorCode) {
        final AppActivity appActivity = (AppActivity) AppActivity.getContext();
        if (appActivity != null) {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    Dialog dialog = GoogleApiAvailability.getInstance().getErrorDialog(appActivity, errorCode, 0);
                    dialog.show();
                }
            };
            appActivity.runOnUiThread(runnable);
        }
    }
}
