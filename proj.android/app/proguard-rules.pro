# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in E:\developSoftware\Android\SDK/tools/proguard/proguard-android.txt
# You can edit the include path and order by changing the proguardFiles
# directive in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# Add any project specific keep options here:

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Proguard Cocos2d-x
-keep public class org.cocos2dx.** { *; }
-dontnote org.cocos2dx.**
-dontwarn org.cocos2dx.**
-keep public class com.chukong.** { *; }
-dontnote com.chukong.**
-dontwarn com.chukong.**
-keep public class com.huawei.android.** { *; }
-dontnote com.huawei.android.**
-dontwarn com.huawei.android.**

# Proguard Apache HTTP
-keep class org.apache.http.** { *; }
-dontnote org.apache.http.**
-dontwarn org.apache.http.**

# Proguard SnowFight
-keep,includedescriptorclasses public class io.gamen.snowfight.** { *; }

# Proguard Google Mobile Services
-keep class com.google.android.gms.** { *; }
-dontnote com.google.android.gms.**
-dontwarn com.google.android.gms.**

# Proguard Google Billing
-keep public class com.android.billingclient.** { public *; }
-dontnote com.android.billingclient.**
-dontwarn com.android.billingclient.**

# Proguard Firebase
-keep,includedescriptorclasses class com.google.firebase.** { *; }
-keep,includedescriptorclasses class com.google.ads.** { *; }
-dontnote com.google.firebase.**
-dontwarn com.google.firebase.**

# Proguard Android Webivew. uncomment if you are using a webview in cocos2d-x
#-keep public class android.net.http.SslError
#-keep public class android.webkit.WebViewClient

#-dontwarn android.webkit.WebView
#-dontwarn android.net.http.SslError
#-dontwarn android.webkit.WebViewClient

# Fix: Missing classes detected while running R8
-dontwarn android.os.ServiceManager
-dontwarn android.util.Slog
