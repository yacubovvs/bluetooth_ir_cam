package ru.cubos.ir_cam_android;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class IRView extends AppCompatActivity {
    private static final String TAG = "bluetooth2";

    Button connectBtn;
    Handler messageHandler;

    private static final int REQUEST_ENABLE_BT = 1;
    final int RECIEVE_MESSAGE = 1;        // Статус для Handler
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private StringBuilder sb = new StringBuilder();

    private ConnectedThread mConnectedThread;
    public static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    public static String address = "8D:A1:05:0E:45:54";

    public TextView maxTemp;
    public TextView minTemp;
    public TextView fps;
    public TextView central_temp;

    ImageView irImage;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.ir_view);

        ((TextView)findViewById(R.id.ir_cam_add)).setText("IR_CAM address: " + address + " - change in code");
        connectBtn = findViewById(R.id.connectBtn);
        irImage = findViewById(R.id.irImage);
        fps = findViewById(R.id.fps);
        central_temp = findViewById(R.id.central_temp);

        maxTemp = findViewById(R.id.maxTemp);
        minTemp = findViewById(R.id.minTemp);

        messageHandler = new Handler() {
            public void handleMessage(android.os.Message msg) {
                switch (msg.what) {
                    case RECIEVE_MESSAGE:                                                   // если приняли сообщение в Handler
                        byte[] readBuf = (byte[]) msg.obj;
                        String strIncom = new String(readBuf, 0, msg.arg1);
                        sb.append(strIncom);                                                // формируем строку
                        int endOfLineIndex = sb.indexOf("\r\n");                            // определяем символы конца строки
                        if (endOfLineIndex > 0) {                                            // если встречаем конец строки,
                            String sbprint = sb.substring(0, endOfLineIndex);               // то извлекаем строку
                            sb.delete(0, sb.length());                                      // и очищаем sb

                            String data[] = sbprint.split(";")[0].split(",");
                            try {
                                if (data.length == 64) {
                                    float data_f[] = new float[64];
                                    for (int i = 0; i < 64; i++) {
                                        data_f[i] = Float.parseFloat(data[i]);
                                    }

                                    drawImage(data_f);
                                }
                            }catch(Exception e){}
                        }
                        //Log.d(TAG, "...Строка:"+ sb.toString() +  "Байт:" + msg.arg1 + "...");
                        break;
                }
            };
        };

        btAdapter = BluetoothAdapter.getDefaultAdapter();       // получаем локальный Bluetooth адаптер
        checkBTState();


        connectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //btAdapter = BluetoothAdapter.getDefaultAdapter();       // получаем локальный Bluetooth адаптер
                //checkBTState();
                IRView.this.connect();
            }
        });

        clearImage();
    }

    public void clearImage(){
        Bitmap bitmap = Bitmap.createBitmap(8, 8,
                Bitmap.Config.RGB_565);

        bitmap.eraseColor(Color.BLUE);

        irImage.setImageBitmap(bitmap);
    }

    double current_time = 0;
    double fps_count = 0;

    public void updateFPS_onData(){

        double time = System.currentTimeMillis()/1000;
        if(time != current_time){
            fps.setText("FPS: " + (int)fps_count);
            fps_count = 1;
            current_time = time;
        }else{
            fps_count ++;
        }


    }

    public void drawImage(float data[]){
        updateFPS_onData();

        float max = -1000;
        float min = 1000;

        for (int i=0; i<64; i++){
            if(max<data[i]) max = data[i];
            if(min>data[i]) min = data[i];
        }

        Bitmap bitmap = Bitmap.createBitmap(8, 8,
                Bitmap.Config.ARGB_8888);

        bitmap.eraseColor(Color.BLUE);

        minTemp.setText("" + min);
        maxTemp.setText("" + max);

        //Color pixelColor = new Color();
        //pixelColor.red(255);
        //pixelColor.green(255);
        //pixelColor.blue(0);

        float central_max_temp = -1000;

        for (int x=0; x<8; x++){
            for (int y=0; y<8; y++){
                int position = x*8 + y;

                int alpha = 255;
                int green = 0;

                if(x==3 || x ==4 || y==3 || y==4){
                    if(central_max_temp<data[position]) central_max_temp = data[position];
                }

                float length_temp = max - min;
                float d_value = (data[position] - min)/length_temp*255;
                int red = (int)d_value;
                int blue = (int)(255 - d_value);
                //int blue = 0;


                int pixelColor = (alpha << 24) | (red << 16) | (green << 8) | blue;
                //bitmap.setPixel(0,0, Color.RED);
                bitmap.setPixel(7-x,y, pixelColor);
            }
        }

        central_temp.setText("" + central_max_temp + "°C");
        irImage.setImageBitmap(bitmap);
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    public void connect(){
        BluetoothDevice device = btAdapter.getRemoteDevice(address);

        try {
            btSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
        } catch (IOException e) {
        }

        btAdapter.cancelDiscovery();

        try {
            btSocket.connect();
        } catch (IOException e) {
            try {
                btSocket.close();
            } catch (IOException ee) {
            }
        }

        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();
    }

    @Override
    public void onPause() {
        super.onPause();
        try     {
            btSocket.close();
        } catch (IOException e) {
        }
    }

    private void checkBTState() {

        if(btAdapter==null) {
        } else {
            if (btAdapter.isEnabled()) {
            } else {
                Intent enableBtIntent = new Intent(btAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
        }
    }

    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) { }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[512];  // buffer store for the stream
            int bytes; // bytes returned from read()

            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer);        // Получаем кол-во байт и само собщение в байтовый массив "buffer"
                    messageHandler.obtainMessage(RECIEVE_MESSAGE, bytes, -1, buffer).sendToTarget();     // Отправляем в очередь сообщений Handler
                } catch (IOException e) {
                    break;
                }
            }
        }

    }
}