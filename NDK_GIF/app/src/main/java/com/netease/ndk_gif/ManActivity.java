package com.netease.ndk_gif;

import android.Manifest;
import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v4.app.ActivityCompat;
import android.widget.ImageView;

import java.io.File;

public class ManActivity extends Activity {

    private Bitmap bitmap;
    private ImageView imageView;
    private GifHandler gifHandler;

    private Handler mHandler = new Handler(new Handler.Callback() {
        @Override
        public boolean handleMessage(Message msg) {
            int mNextFrame = gifHandler.updateFrame(bitmap);
            mHandler.sendEmptyMessageDelayed(1,mNextFrame);
            imageView.setImageBitmap(bitmap);
            return false;
        }
    });

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = findViewById(R.id.sample_text);

        new Thread(){
           @Override
           public void run() {
                File file = new File(Environment.getExternalStorageDirectory(),"demo.gif");
                gifHandler = new GifHandler(file.getAbsolutePath());
                int height = gifHandler.getHeight();
                int width = gifHandler.getWidth();
                bitmap = Bitmap.createBitmap(width,height, Bitmap.Config.ARGB_8888);
                int mNextFrame = gifHandler.updateFrame(bitmap);
                mHandler.sendEmptyMessageDelayed(1,mNextFrame);
            }
    }.start();

    }


    @Override
    protected void onResume() {
        super.onResume();
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0123);
    }
}
