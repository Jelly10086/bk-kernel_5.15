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
import android.widget.Toast;

public final class CaptureService extends Service {
    private static final String CHANNEL = "performance";
    private static final int NOTIFICATION = 1;
    private final Handler handler = new Handler(Looper.getMainLooper());
    private CaptureStore store;
    private String progress = "";
    private String shownPhase = "";
    private long lastChange;
    private boolean copying;
    private boolean attempted;
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
        if (!CaptureStore.valid(name)) { stopSelf(); return START_NOT_STICKY; }
        if (store != null && !store.name.equals(name)) return START_NOT_STICKY;
        store = new CaptureStore(this, name);
        if (!store.file(".request").isFile() || store.destination() == null || !notificationsAllowed()) {
            store.write(store.read(".progress").startsWith("choosing ") ? ".choice" : ".result", "error");
            store.write(".stop", "stop");
            if (store.read(".progress").startsWith("choosing ")) store.release(true);
            finish();
            return START_NOT_STICKY;
        }
        try {
            startForeground(NOTIFICATION, notification("recording"));
            if (!store.preferences().edit().putString("active", name).commit()) throw new IllegalStateException("No session");
            store.write(".heartbeat", Long.toString(SystemClock.uptimeMillis()));
            if (store.read(".progress").startsWith("choosing ") && store.pending()) {
                if (!store.write(".choice", "selected")) throw new IllegalStateException("No ACK");
            }
        } catch (RuntimeException error) {
            store.write(store.read(".progress").startsWith("choosing ") ? ".choice" : ".result", "error");
            store.write(".stop", "stop");
            if (store.read(".progress").startsWith("choosing ")) store.release(true);
            finish();
            return START_NOT_STICKY;
        }
        lastChange = SystemClock.uptimeMillis();
        if (!copying) attempted = false;
        handler.removeCallbacks(poll);
        handler.post(poll);
        return START_STICKY;
    }

    private boolean notificationsAllowed() {
        NotificationManager manager = getSystemService(NotificationManager.class);
        NotificationChannel channel = manager.getNotificationChannel(CHANNEL);
        return manager.areNotificationsEnabled() && channel.getImportance() != NotificationManager.IMPORTANCE_NONE
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
            Intent stop = new Intent(this, CaptureStopReceiver.class).putExtra("name", store.name);
            PendingIntent action = PendingIntent.getBroadcast(this, 1, stop,
                    PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);
            builder.addAction(new Notification.Action.Builder(null, "停止并保存", action).build());
        }
        return builder.build();
    }

    private final Runnable poll = new Runnable() {
        @Override
        public void run() {
            if (store == null) return;
            if (heartbeat++ % 5 == 0) store.write(".heartbeat", Long.toString(SystemClock.uptimeMillis()));
            String current = store.read(".progress");
            if (!current.equals(progress)) {
                progress = current;
                lastChange = SystemClock.uptimeMillis();
            }
            String phase = current.split(" ", 2)[0];
            if (!store.file(".request").isFile() || "cancelled".equals(phase) || "ready".equals(phase)) {
                if (!copying) { store.release(true); finish(); }
                return;
            }
            if ("saved".equals(phase) || "error".equals(phase)) {
                if (!copying) finish();
                return;
            }
            if (!phase.equals(shownPhase)) {
                getSystemService(NotificationManager.class).notify(NOTIFICATION, notification(phase));
                shownPhase = phase;
            }
            if ("saving".equals(phase) && store.file("").isFile() && !attempted) {
                attempted = true;
                copying = true;
                new Thread(() -> {
                    boolean saved = store.save();
                    handler.post(() -> {
                        copying = false;
                        Toast.makeText(CaptureService.this, saved ? "性能日志已保存" : "保存失败，日志已保留，可回模块重试",
                                Toast.LENGTH_LONG).show();
                        finish();
                    });
                }, "bkk-performance-save").start();
            }
            // 使用uptime排除系统休眠，采样进程失联时不留下永久通知
            if (!copying && SystemClock.uptimeMillis() - lastChange > 180_000) {
                store.write(".stop", "stop");
                store.write(".result", "error");
                finish();
                return;
            }
            handler.postDelayed(this, 1000);
        }
    };

    private void finish() {
        handler.removeCallbacks(poll);
        store.preferences().edit().remove("active").commit();
        stopForeground(STOP_FOREGROUND_REMOVE);
        stopSelf();
    }

    @Override
    public void onDestroy() {
        handler.removeCallbacks(poll);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) { return null; }
}
