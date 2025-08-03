package io.gamen.snowfight;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import static android.app.Activity.RESULT_CANCELED;
import static android.app.Activity.RESULT_OK;

public final class ShareHelper {
    public static final int REQUEST_CODE_SHARE = 1;
    private static final String MIME_TEXT_TYPE = "text/plain";
    enum ErrorCode
    {
        ERROR_NO_APP,
        ERROR_UNKNOWN,
    }

    private static native void onSharePresented();
    private static native void onShareCompleted();
    private static native void onShareCanceled();
    private static native void onShareFailed(int errorCode);

    public static void share(final String title, final String text, final String image, final String link) {
        final AppActivity appActivity = (AppActivity)AppActivity.getContext();
        appActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Intent sendIntent = new Intent();
                sendIntent.setAction(Intent.ACTION_SEND);
                sendIntent.setType(MIME_TEXT_TYPE);

                String content = "";
                if (!text.isEmpty())
                    content += text;
                if (!link.isEmpty())
                    content += "\n" + link;
                sendIntent.putExtra(Intent.EXTRA_TEXT, content);
                sendIntent.putExtra(Intent.EXTRA_SUBJECT, title);

                Intent shareIntent = Intent.createChooser(sendIntent, appActivity.getString(R.string.share_chooser_title));
                appActivity.startActivityForResult(shareIntent, REQUEST_CODE_SHARE);
                appActivity.runOnGLThread(new Runnable() {
                    @Override
                    public void run() {
                        ShareHelper.onSharePresented();
                    }
                });
            }
        });
    }

    public static void onActivityResult(Activity activity, int requestCode, final int resultCode, Intent data)
    {
        if(requestCode != REQUEST_CODE_SHARE)
            return;

        AppActivity appActivity = (AppActivity) activity;
        appActivity.runOnGLThread(new Runnable() {
            @Override
            public void run() {
                switch (resultCode) {
                    case RESULT_OK:
                        ShareHelper.onShareCompleted();
                        break;
                    case RESULT_CANCELED:
                        ShareHelper.onShareCanceled();
                        break;
                }
            }
        });
    }
}
