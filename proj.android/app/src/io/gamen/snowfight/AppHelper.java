package io.gamen.snowfight;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.provider.Settings;
import android.view.DisplayCutout;
import android.view.WindowInsets;

import java.util.Date;
import java.util.Locale;
import java.util.TimeZone;
import java.util.UUID;

public final class AppHelper {
    private static Activity sActivity = null;
    private static Vibrator sVibrateService = null;
    private static boolean sIsInited = false;

    public static void init(final Activity activity)
    {
        sActivity = activity;
        if(!sIsInited)
        {
            sVibrateService = (Vibrator)activity.getSystemService(Context.VIBRATOR_SERVICE);
            sIsInited = true;
        }
    }

    public static String getCountryCode() {
        String country = Locale.getDefault().getCountry();
        return country;
    }

    public static String getScriptCode() {
        String scriptCode = "";
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
            scriptCode = Locale.getDefault().getScript();
        }
        return scriptCode;
    }

    public static String getOSVersion() {
        String os = "Android " + Build.VERSION.RELEASE;
        return os;
    }

    public static String getModel() {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;

        return manufacturer + " " + model;
    }

    public static String getDeviceGUID() {
        String uuid = UUID.randomUUID().toString();
        return uuid;
    }

    public static int getVersionCode() {
        int versionCode = BuildConfig.VERSION_CODE;
        return versionCode;
    }

    public static int getTimeZone() {
        TimeZone timezone = TimeZone.getDefault();
        Date now = new Date();
        int seconds = timezone.getOffset(now.getTime()) / 1000;
        return seconds;
    }

    public static boolean isSupportDisplayCutout()
    {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            DisplayCutout cutout = getDisplayCutout();
            return cutout != null && !cutout.getBoundingRects().isEmpty();
        }
        return false;
    }

    private static DisplayCutout getDisplayCutout() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            AppActivity appActivity = (AppActivity) AppActivity.getContext();
            if (appActivity != null) {
                WindowInsets insets = appActivity.getWindow().getDecorView().getRootWindowInsets();
                if(insets != null) {
                    DisplayCutout displayCutout = insets.getDisplayCutout();
                    return displayCutout;
                }
            }
        }
        return null;
    }

    public static int getSafeInsetLeft() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            DisplayCutout cutout = getDisplayCutout();
            if (cutout != null)
                return cutout.getSafeInsetLeft();
        }
        return 0;
    }

    public static int getSafeInsetRight() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            DisplayCutout cutout = getDisplayCutout();
            if (cutout != null)
                return cutout.getSafeInsetRight();
        }
        return 0;
    }

    public static int getSafeInsetTop() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            DisplayCutout cutout = getDisplayCutout();
            if (cutout != null)
                return cutout.getSafeInsetTop();
        }
        return 0;
    }

    public static int getSafeInsetBottom() {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            DisplayCutout cutout = getDisplayCutout();
            if (cutout != null)
                return cutout.getSafeInsetBottom();
        }
        return 0;
    }

    public static void setMultipleTouchEnabled(boolean enabled) {
        AppActivity appActivity = (AppActivity) AppActivity.getContext();
        if (appActivity != null) {
            appActivity.getGLSurfaceView().setMultipleTouchEnabled(enabled);
        }
    }

    public static boolean isAppInstalled(String packageName) {
        AppActivity appActivity = (AppActivity) AppActivity.getContext();
        if(appActivity != null)
        {
            PackageManager pm = appActivity.getPackageManager();
            try {
                pm.getPackageInfo(packageName, PackageManager.GET_ACTIVITIES);
                return true;
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
            }
        }
        return false;
    }

    public static void vibrate(float duration, int amplitude) {
        if(sVibrateService != null) {
            if(sVibrateService.hasVibrator()) {
                ContentResolver contentResolver = sActivity.getContentResolver();
                int hapticFeedbackEnabled = Settings.System.getInt(contentResolver, Settings.System.HAPTIC_FEEDBACK_ENABLED, 0);
                if(hapticFeedbackEnabled != 0) {
                    long milliseconds = (long)(duration * 1000);
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                        if(!sVibrateService.hasAmplitudeControl())
                            amplitude = VibrationEffect.DEFAULT_AMPLITUDE;
                        sVibrateService.vibrate(VibrationEffect.createOneShot(milliseconds, amplitude));
                    } else {
                        sVibrateService.vibrate(milliseconds);
                    }
                }
            }
        }
    }
}
