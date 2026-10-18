package org.bkkernel.logexport;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.SystemClock;
import android.util.Log;
import android.widget.Toast;

public final class CaptureService extends Service {
    static final String STOP = "org.bkkernel.logexport.STOP";
    private static final String CHANNEL = "performance";
    private static final int NOTIFICATION = 1;
    private static final int RESULT_NOTIFICATION = 2;
    private final Handler handler = new Handler(Looper.getMainLooper());
    private CaptureStore store;
    private String progress = "";
    private String shownPhase = "";
    private long lastChange;
    private boolean copying;
    private boolean destroyed;
    private int heartbeat;

    @Override
    public void onCreate() {
        super.onCreate();
        NotificationChannel channel = new NotificationChannel(CHANNEL, "性能采集", NotificationManager.IMPORTANCE_LOW);
        channel.setSound(null, null);
        channel.setShowBadge(false);
        getSystemService(NotificationManager.class).createNotificationChannel(channel);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String name = intent == null ? getSharedPreferences("performance", MODE_PRIVATE).getString("active", null)
                : intent.getStringExtra("name");
        if (!CaptureStore.valid(name)) { stopSelf(startId); return START_NOT_STICKY; }
        if (store != null && !store.name.equals(name) && copying) {
            new CaptureStore(this, name).write(".choice", "error");
            Toast.makeText(this, "上次日志仍在写入，请稍后重试", Toast.LENGTH_LONG).show();
            return START_STICKY;
        }
        if (store == null || !store.name.equals(name)) {
            store = new CaptureStore(this, name);
            progress = "";
            shownPhase = "";
            heartbeat = 0;
        }
        String phase = store.read(".progress").split(" ", 2)[0];
        try {
            getSystemService(NotificationManager.class).cancel(RESULT_NOTIFICATION);
            startForeground(NOTIFICATION, notification(phase));
            if (intent != null && STOP.equals(intent.getAction()) && !store.write(".stop", "stop")) {
                throw new IllegalStateException("Stop request not saved");
            }
            if ("saved".equals(store.read(".result"))) {
                complete(true);
                return START_NOT_STICKY;
            }
            if (!store.file(".request").isFile() || store.destination() == null || !notificationsAllowed()) {
                throw new IllegalStateException("Capture request, destination or notification unavailable");
            }
            if (!store.preferences().edit().putString("active", name).commit()) throw new IllegalStateException("No session");
            if (!store.write(".heartbeat", Long.toString(SystemClock.uptimeMillis()))) {
                throw new IllegalStateException("Heartbeat not saved");
            }
            if ("choosing".equals(phase) && store.pending() && !store.write(".choice", "selected")) {
                throw new IllegalStateException("No ACK");
            }
        } catch (RuntimeException error) {
            Log.e("BkkCapture", "Cannot start or resume " + name, error);
            store.write(".failure", error.getClass().getSimpleName() + ": " + String.valueOf(error.getMessage()));
            store.write("choosing".equals(phase) ? ".choice" : ".result", "error");
            store.write(".stop", "stop");
            if ("choosing".equals(phase)) store.release(true);
            complete(false);
            return START_NOT_STICKY;
        }
        lastChange = SystemClock.uptimeMillis();
        handler.removeCallbacks(poll);
        handler.post(poll);
        return START_STICKY;
    }

    private boolean notificationsAllowed() {
        NotificationManager manager = getSystemService(NotificationManager.class);
        NotificationChannel channel = manager.getNotificationChannel(CHANNEL);
        return manager.areNotificationsEnabled() && channel != null
                && channel.getImportance() != NotificationManager.IMPORTANCE_NONE
                && (Build.VERSION.SDK_INT < 33 || checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                == PackageManager.PERMISSION_GRANTED);
    }

    private Notification notification(String phase) {
        boolean recording = "recording".equals(phase) || "choosing".equals(phase);
        Notification.Builder builder = new Notification.Builder(this, CHANNEL)
                .setSmallIcon(android.R.drawable.ic_menu_info_details)
                .setContentTitle(recording ? "正在进行性能采集" : "正在保存性能日志")
                .setContentText(recording ? "长按以停止采集并保存" : "压缩并写入已选择的位置")
                .setCategory(Notification.CATEGORY_SERVICE).setOngoing(true).setOnlyAlertOnce(true)
                .setShowWhen(false);
        if (Build.VERSION.SDK_INT >= 31) builder.setForegroundServiceBehavior(Notification.FOREGROUND_SERVICE_IMMEDIATE);
        if (recording) {
            Intent stop = new Intent(this, CaptureService.class).setAction(STOP).putExtra("name", store.name);
            // PendingIntent直接进入前台服务，进程重建后也能接收停止请求
            PendingIntent action = PendingIntent.getForegroundService(this, 1, stop,
                    PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
            builder.addAction(new Notification.Action.Builder(null, "停止并保存", action).build());
        }
        return builder.build();
    }

    private final Runnable poll = new Runnable() {
        @Override
        public void run() {
            if (store == null || destroyed) return;
            if (heartbeat++ % 5 == 0) store.write(".heartbeat", Long.toString(SystemClock.uptimeMillis()));
            String current = store.read(".progress");
            if (!current.equals(progress)) {
                progress = current;
                lastChange = SystemClock.uptimeMillis();
            }
            String phase = current.split(" ", 2)[0];
            String result = store.read(".result");
            if (!copying && ("saved".equals(result) || "saved".equals(phase))) { complete(true); return; }
            if (!copying && ("error".equals(result) || "error".equals(phase))) { complete(false); return; }
            if (!store.file(".request").isFile() || "cancelled".equals(phase) || "ready".equals(phase)) {
                if (!copying) {
                    store.release(true);
                    finish();
                }
                return;
            }
            if (!phase.equals(shownPhase)) {
                getSystemService(NotificationManager.class).notify(NOTIFICATION, notification(phase));
                shownPhase = phase;
            }
            if ("saving".equals(phase) && store.file("").isFile() && !copying) {
                copying = true;
                final CaptureStore session = store;
                new Thread(() -> {
                    boolean saved = session.save();
                    handler.post(() -> {
                        if (store != session) return;
                        copying = false;
                        if (!destroyed) complete(saved);
                    });
                }, "bkk-performance-save").start();
            }
            // 使用uptime排除系统休眠，采样进程失联时不留下永久通知
            if (!copying && SystemClock.uptimeMillis() - lastChange > 180_000) {
                store.write(".stop", "stop");
                store.write(".result", "error");
                complete(false);
                return;
            }
            handler.postDelayed(this, 1000);
        }
    };

    private void complete(boolean saved) {
        String message = saved ? "性能日志已保存" : "保存失败，日志已保留，可重试或重新采集";
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
        stopForeground(STOP_FOREGROUND_REMOVE);
        getSystemService(NotificationManager.class).notify(RESULT_NOTIFICATION,
                new Notification.Builder(this, CHANNEL)
                        .setSmallIcon(android.R.drawable.ic_menu_info_details)
                        .setContentTitle(message).setContentText(store.name)
                        .setStyle(new Notification.BigTextStyle().bigText(saved ? store.name
                                : "回到模块，点停止重试保存，或点开始进行新采集。未保存的日志仍保留在本机。"))
                        .setAutoCancel(true).setShowWhen(true).build());
        Log.i("BkkCapture", store.name + (saved ? " - Saved" : " - Save failed"));
        finish(false);
    }

    private void finish() { finish(true); }

    private void finish(boolean removeNotification) {
        handler.removeCallbacks(poll);
        if (store.name.equals(store.preferences().getString("active", null))) {
            store.preferences().edit().remove("active").commit();
        }
        if (removeNotification) stopForeground(STOP_FOREGROUND_REMOVE);
        stopSelf();
    }

    @Override
    public void onDestroy() {
        destroyed = true;
        handler.removeCallbacks(poll);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) { return null; }
}
