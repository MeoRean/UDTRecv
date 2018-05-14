package net.meoi.udtrecv;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends AppCompatActivity {
    int test;
    private EditText edtName,edtName2,edtName3,edtName4;
    private Button btnShow;
    private native static int RecvFileFromServer(String UDTServerAddress,String UDTServerPortStr,String RemoteFileName,String LocalFileName);
    static {
        System.loadLibrary("udt");
        System.loadLibrary("recvfile");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //test=MainActivity.GetReply();
        edtName= (EditText)this.findViewById(R.id.ed_name);
        edtName2= (EditText)this.findViewById(R.id.ed_name2);
        edtName3= (EditText)this.findViewById(R.id.ed_name3);
        edtName4= (EditText)this.findViewById(R.id.ed_name4);
        btnShow= (Button)this.findViewById(R.id.btn_show);
        btnShow.setOnClickListener(new Button.OnClickListener(){
            public void onClick(View arg0){
                MainActivity.RecvFileFromServer(edtName.getText().toString().trim(),edtName2.getText().toString().trim(),edtName3.getText().toString().trim(),edtName4.getText().toString().trim());
                //MainActivity.RecvFileFromServer("192.168.0.100","9000","file.txt","test.txt");
            }
        });
    }
}
