package org.bkkernel.logexport;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.util.AtomicFile;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.channels.FileLock;
import java.nio.charset.StandardCharsets;

final class CaptureStore {
    final Context context;
    final String name;

    CaptureStore(Context context, String name) {
        if (!valid(name)) throw new IllegalArgumentException("Invalid capture name");
        this.context = context;
        this.name = name;
    }

    static boolean valid(String name) {
        return name != null && name.matches("bkk-performance-[0-9]{8}-[0-9]{6}-[A-Za-z0-9]{6}\\.tar\\.gz");
    }

    File file(String suffix) { return new File(context.getFilesDir(), name + suffix); }

    SharedPreferences preferences() {
        return context.getSharedPreferences("performance", Context.MODE_PRIVATE);
    }

    String read(String suffix) {
        try (FileInputStream input = new AtomicFile(file(suffix)).openRead()) {
            byte[] bytes = new byte[512];
            int count = input.read(bytes);
            return count < 0 ? "" : new String(bytes, 0, count, StandardCharsets.UTF_8).trim();
        } catch (IOException error) { return ""; }
    }

    boolean write(String suffix, String value) {
        AtomicFile target = new AtomicFile(file(suffix));
        FileOutputStream output = null;
        try {
            output = target.startWrite();
            output.write((value + "\n").getBytes(StandardCharsets.UTF_8));
            target.finishWrite(output);
            return true;
        } catch (IOException error) {
            if (output != null) target.failWrite(output);
            return false;
        }
    }

    boolean pending() { return file(".request").isFile() && !file(".stop").exists(); }

    Uri destination() {
        String value = preferences().getString(name, null);
        return value == null ? null : Uri.parse(value);
    }

    void remember(Uri uri, int flags) throws IOException {
        if ((flags & Intent.FLAG_GRANT_WRITE_URI_PERMISSION) == 0) {
            throw new SecurityException("No write grant");
        }
        context.getContentResolver().takePersistableUriPermission(uri, flags);
        if (!preferences().edit().putString(name, uri.toString())
                .putInt(name + ".flags", flags).commit()) {
            try { context.getContentResolver().releasePersistableUriPermission(uri, flags); }
            catch (SecurityException ignored) { }
            throw new IOException("Destination not saved");
        }
    }

    void release(boolean delete) {
        Uri uri = destination();
        if (uri != null) {
            if (delete) {
                try { DocumentsContract.deleteDocument(context.getContentResolver(), uri); }
                catch (Exception ignored) { }
            }
            try {
                context.getContentResolver().releasePersistableUriPermission(uri,
                        preferences().getInt(name + ".flags", 0));
            } catch (SecurityException ignored) { }
        }
        preferences().edit().remove(name).remove(name + ".flags").commit();
    }

    boolean save() {
        // ACK只在输出流关闭后写入，失败时保留源文件和URI授权供重试
        try (FileOutputStream lockFile = new FileOutputStream(file(".lock"));
             FileLock lock = lockFile.getChannel().lock()) {
            if ("saved".equals(read(".result"))) return true;
            Uri uri = destination();
            File source = file("");
            if (uri == null || !source.isFile() || !source.getCanonicalFile().getParentFile()
                    .equals(context.getFilesDir().getCanonicalFile())) throw new IOException("No capture archive");
            try (FileInputStream input = new FileInputStream(source);
                 OutputStream output = context.getContentResolver().openOutputStream(uri, "wt")) {
                if (output == null) throw new IOException("Destination unavailable");
                byte[] buffer = new byte[64 * 1024];
                int count;
                while ((count = input.read(buffer)) != -1) output.write(buffer, 0, count);
                output.flush();
            }
            if (!write(".result", "saved")) throw new IOException("ACK not saved");
            source.delete();
            release(false);
            return true;
        } catch (IOException | SecurityException error) {
            write(".result", "error");
            return false;
        }
    }
}
