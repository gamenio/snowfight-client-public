package io.gamen.snowfight;

import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;

public final class StoreReviewHelper {
    public static boolean openURL(String url) {
        final AppActivity appActivity = (AppActivity) AppActivity.getContext();
        if (appActivity == null)
            return false;

        try {
            Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            int flags = Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_MULTIPLE_TASK;
            if (Build.VERSION.SDK_INT >= 21) {
                flags |= Intent.FLAG_ACTIVITY_NEW_DOCUMENT;
            } else {
                //noinspection deprecation
                flags |= Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET;
            }
            intent.addFlags(flags);
            appActivity.startActivity(intent);

            return true;

        } catch (Exception e) {
        }
        return false;
    }
}
