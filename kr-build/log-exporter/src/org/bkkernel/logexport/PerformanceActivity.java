package org.bkkernel.logexport;

import android.Manifest;
import android.app.Activity;
import android.app.NotificationManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.widget.Toast;

import java.io.IOException;

public final class PerformanceActivity extends Activity {
    private static final int CHOOSE = 1;
    private static final int NOTIFICATIONS = 2;
    private CaptureStore store;
    private String operation;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        String name = state == null ? getIntent().getStringExtra("name") : state.getString("name");
        operation = state == null ? getIntent().getStringExtra("operation") : state.getString("operation");
        if (!CaptureStore.valid(name)) { finish(); return; }
        store = new CaptureStore(this, name);
        if (!store.file(".request").isFile()) {
            fail("采样请求已失效");
            return;
        }
        if (!"choose".equals(operation) && !"resume".equals(operation)) { finish(); return; }
        if (state != null) return;
        if (Build.VERSION.SDK_INT >= 33 && checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS)
                != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.POST_NOTIFICATIONS}, NOTIFICATIONS);
        } else {
            choose();
        }
    }

    @Override
    protected void onSaveInstanceState(Bundle state) {
        super.onSaveInstanceState(state);
        if (store != null) state.putString("name", store.name);
        state.putString("operation", operation);
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        super.onRequestPermissionsResult(requestCode, permissions, results);
        if (requestCode != NOTIFICATIONS || store == null) return;
        if (results.length > 0 && results[0] == PackageManager.PERMISSION_GRANTED) choose();
        else fail("请允许采样通知后重试");
    }

    private void choose() {
        if (!getSystemService(NotificationManager.class).areNotificationsEnabled()) {
            fail("请先允许bkk-control通知");
            return;
        }
        if (!liveRequest()) { fail("采样请求已取消"); return; }
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT)
                .addCategory(Intent.CATEGORY_OPENABLE).setType("application/gzip")
                .putExtra(Intent.EXTRA_TITLE, store.name).putExtra(Intent.EXTRA_LOCAL_ONLY, true)
                .addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION
                        | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        try { startActivityForResult(intent, CHOOSE); }
        catch (RuntimeException error) { fail("无法打开文件管理器"); }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != CHOOSE || store == null) return;
        Uri uri = resultCode == RESULT_OK && data != null ? data.getData() : null;
        if (uri == null) {
            store.write("resume".equals(operation) ? ".result" : ".choice",
                    "resume".equals(operation) ? "error" : "cancelled");
            finish();
            return;
        }
        int flags = data.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
        try {
            store.release(false);
            store.remember(uri, flags);
            if (!liveRequest()) {
                store.release(true);
                fail("采样请求已取消，请重新开始");
                return;
            }
            startCapture();
        } catch (IOException | SecurityException error) {
            store.release(true);
            fail("无法保留保存授权，请重新选择位置");
        }
    }

    private boolean liveRequest() {
        return "resume".equals(operation)
                ? store.file(".request").isFile() && store.read(".progress").startsWith("saving ")
                : store.pending();
    }

    private void startCapture() {
        try {
            if (store.destination() == null) throw new IllegalStateException("No destination");
            startForegroundService(new Intent(this, CaptureService.class).putExtra("name", store.name));
            finish();
        } catch (RuntimeException error) { fail("无法启动性能采样通知"); }
    }

    private void fail(String message) {
        if (store != null) {
            store.write("choose".equals(operation) ? ".choice" : ".result", "error");
            if ("choose".equals(operation)) store.release(true);
        }
        Toast.makeText(this, message, Toast.LENGTH_LONG).show();
        finish();
    }
}
