package io.gamen.snowfight;

public final class FirebaseHelper {
    public static String getAdMobAppID() {
        AppActivity appActivity = (AppActivity) AppActivity.getContext();
        if (appActivity != null) {
            return appActivity.getResources().getString(R.string.admob_app_id);
        }
        return "";
    }
}
