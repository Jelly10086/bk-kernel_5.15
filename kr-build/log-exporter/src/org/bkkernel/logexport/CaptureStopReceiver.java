package org.bkkernel.logexport;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.widget.Toast;

public final class CaptureStopReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        String name = intent.getStringExtra("name");
        if (!CaptureStore.valid(name)) return;
        CaptureStore store = new CaptureStore(context, name);
        if (!name.equals(store.preferences().getString("active", null)) || !store.file(".request").isFile()) return;
        // 兼容升级前已经显示的通知
        store.write(".stop", "stop");
        try {
            context.startForegroundService(new Intent(context, CaptureService.class)
                    .setAction(CaptureService.STOP).putExtra("name", name));
        } catch (RuntimeException error) {
            Log.e("BkkCapture", "Cannot resume notification stop", error);
            Toast.makeText(context, "停止请求已发送，请回模块查看保存结果", Toast.LENGTH_LONG).show();
        }
    }
}
