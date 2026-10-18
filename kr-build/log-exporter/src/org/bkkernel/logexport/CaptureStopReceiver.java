package org.bkkernel.logexport;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public final class CaptureStopReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        String name = intent.getStringExtra("name");
        if (!CaptureStore.valid(name)) return;
        CaptureStore store = new CaptureStore(context, name);
        String active = store.preferences().getString("active", null);
        if (name.equals(active) && store.file(".request").isFile()) store.write(".stop", "stop");
    }
}
