package io.gamen.snowfight;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public final class GameCenterHelper {

    private static native void nativeOnActivityCreate(Activity activity, Bundle savedInstanceState);
    private static native void nativeOnActivityResult(Activity activity, int requestCode, int resultCode, Intent data);


    static void onActivityCreate(Activity activity, Bundle savedInstanceState)
    {
        nativeOnActivityCreate(activity, savedInstanceState);
    }

    static void onActivityResult(Activity activity, int requestCode, int resultCode, Intent data)
    {
        nativeOnActivityResult(activity, requestCode, resultCode, data);
    }
}
