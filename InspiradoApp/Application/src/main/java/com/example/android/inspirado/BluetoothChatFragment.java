/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.android.inspirado;

import android.app.ActionBar;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.design.widget.FloatingActionButton;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.view.ViewPager;
import android.util.Base64;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import java.nio.charset.CharacterCodingException;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import static android.view.View.GONE;
import static android.view.View.VISIBLE;


//import com.example.android.common.logger.Log;

/**
 * This fragment controls Bluetooth communication
 */
public class BluetoothChatFragment extends Fragment {

    private static final String TAG = "BluetoothChatFragment";

    // Intent request codes
    private static final int REQUEST_CONNECT_DEVICE_SECURE = 1;
    private static final int REQUEST_CONNECT_DEVICE_INSECURE = 2;
    private static final int REQUEST_ENABLE_BT = 3;

    // Layout Views
    private ListView mConversationView;
    private EditText mOutEditText;
    private Button mSendButton;
    private EditText mEditEmail, mEditDisplayName;

    /**
     * Name of the connected device
     */
    private String mConnectedDeviceName = null;

    /**
     * Array adapter for the conversation thread
     */
    private ArrayAdapter<String> mConversationArrayAdapter;

    /**
     * String buffer for outgoing messages
     */
    private StringBuffer mOutStringBuffer;

    /**
     * Local Bluetooth adapter
     */
    private BluetoothAdapter mBluetoothAdapter = null;

    /**
     * Member object for the chat services
     */
    private BluetoothChatService mChatService = null;

    /*
    * ArrayAdapter to populate available networks
    * */
    ArrayList<String> networks = new ArrayList<String>(){{add("Securewireless");}};
    ArrayList<Network> networksTest = new ArrayList<Network>(){{add(new Network(0,"Securewireless","peap"));}};
    private ArrayAdapter<Network> mAdapterNetwork = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
        // Get local Bluetooth adapter
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        // If the adapter is null, then Bluetooth is not supported
        if (mBluetoothAdapter == null) {
            FragmentActivity activity = getActivity();
            Toast.makeText(activity, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            activity.finish();
        }

    }


    @Override
    public void onStart() {
        super.onStart();
        // If BT is not on, request that it be enabled.
        // setupChat() will then be called during onActivityResult
        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, REQUEST_ENABLE_BT);
            // Otherwise, setup the chat session
        } else if (mChatService == null) {
            setupChat();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mChatService != null) {
            mChatService.stop();
        }

    }

    @Override
    public void onResume() {
        super.onResume();

        // Performing this check in onResume() covers the case in which BT was
        // not enabled during onStart(), so we were paused to enable it...
        // onResume() will be called when ACTION_REQUEST_ENABLE activity returns.
        if (mChatService != null) {
            // Only if the state is STATE_NONE, do we know that we haven't started already
            if (mChatService.getState() == BluetoothChatService.STATE_NONE) {
                // Start the Bluetooth chat services
                mChatService.start();
            }
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {

        final View rootView = inflater.inflate(R.layout.fragment_bluetooth_chat, container, false);
        return rootView;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        mConversationView = (ListView) view.findViewById(R.id.in);
        mOutEditText = (EditText) view.findViewById(R.id.edit_text_out);
        mSendButton = (Button) view.findViewById(R.id.button_send);

        final Spinner mSpinner = (Spinner) getView().findViewById(R.id.spinNetwork);
        final EditText mEditPassword = (EditText) getView().findViewById(R.id.editPassword);

        final SharedPreferences prefs = getActivity().getSharedPreferences("com.example.android.inspirado", Context.MODE_PRIVATE);

        final SharedPreferences.OnSharedPreferenceChangeListener listener = new SharedPreferences.OnSharedPreferenceChangeListener() {
            public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
                // Implementation
                Integer value = prefs.getInt(key, 0);
                //Toast.makeText(getActivity(), value.toString(), Toast.LENGTH_LONG).show();
                LinearLayout layoutWifi = (LinearLayout) getActivity().findViewById(R.id.layoutWifi);
                LinearLayout layoutDropbox = (LinearLayout) getActivity().findViewById(R.id.layoutDropbox);
                LinearLayout layoutControl = (LinearLayout) getActivity().findViewById(R.id.layoutControl);
                switch(value) {
                    case 0:
                        layoutWifi.setVisibility(VISIBLE);
                        layoutDropbox.setVisibility(GONE);
                        layoutControl.setVisibility(GONE);
                        break;
                    case 1:
                        layoutWifi.setVisibility(GONE);
                        layoutDropbox.setVisibility(VISIBLE);
                        layoutControl.setVisibility(GONE);
                        break;
                    case 2:
                        layoutWifi.setVisibility(GONE);
                        layoutDropbox.setVisibility(GONE);
                        layoutControl.setVisibility(VISIBLE);
                        break;
                }
            }
        };

        prefs.registerOnSharedPreferenceChangeListener(listener);

        mSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

            public void onItemSelected(AdapterView<?> parent, View view,
                                       int pos, long id) {
                Network selectedNetwork = (Network) parent.getItemAtPosition(pos);
                //Toast.makeText(getActivity(), selectedNetwork.getSecurity(),Toast.LENGTH_SHORT).show();

                if(selectedNetwork.getSecurity().equals("peap")){
                    getView().findViewById(R.id.editUsername).setVisibility(VISIBLE);
                    getView().findViewById(R.id.tvUsername).setVisibility(VISIBLE);
                }else{
                    getView().findViewById(R.id.editUsername).setVisibility(GONE);
                    getView().findViewById(R.id.tvUsername).setVisibility(GONE);
                }

            }

            public void onNothingSelected(AdapterView<?> parent) {
                // Do nothing, just another required interface callback
            }

        }); // (optional)

        final Button mTestButton = (Button) getView().findViewById(R.id.button_test);
        mTestButton.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                Gson gson = new Gson();
                JsonObject jsonObject = new JsonObject();
                jsonObject.addProperty("type","listNetworks");
                jsonObject.addProperty("parameters","");
                String json = gson.toJson(jsonObject);
                sendMessage(json);
            }
        });

        final Button mButtonConnect = (Button) getView().findViewById(R.id.button_connect);
        mButtonConnect.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v){
                View view = getView();
                if(null != view) {
                    String ssid = mSpinner.getSelectedItem().toString();
                    String password = mEditPassword.getText().toString();
                    String passwordEncrypted = encrypt(password);
                    Base64.encode(password.getBytes(), Base64.NO_PADDING);
                    int id = networksTest.indexOf(mSpinner.getSelectedItem());
                    String status = networksTest.get(id).getStatus();
                    String security = networksTest.get(id).getSecurity();
                    String serviceKey = networksTest.get(id).getServiceKey();

                    EditText teUser = (EditText) view.findViewById(R.id.editUsername);
                    String username = teUser.getText().toString();

                    Gson gson = new Gson();
                    JsonObject jsonObject = new JsonObject();
                    jsonObject.addProperty("type","connectToNetwork");

                    Toast.makeText(getActivity(), username, Toast.LENGTH_LONG).show();

                    if(username != "")
                        jsonObject.add("parameters", gson.toJsonTree(new Network(id,ssid,serviceKey,status,security,password,username)));
                    else
                        jsonObject.add("parameters", gson.toJsonTree(new Network(id,ssid,serviceKey,status,security,password)));
                    String json = gson.toJson(jsonObject);

                    sendMessage(json);
                }
            }
        });

        mAdapterNetwork = new ArrayAdapter<Network>(getActivity(),android.R.layout.simple_spinner_dropdown_item, networksTest);
        mAdapterNetwork.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(mAdapterNetwork);


        Spinner spinner = (Spinner) getActivity().findViewById(R.id.spinFormat);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(getActivity(),R.array.file_formats, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        mEditDisplayName = (EditText) getActivity().findViewById(R.id.editDisplayName);
        mEditEmail = (EditText) getActivity().findViewById(R.id.editEmail);

        mEditDisplayName.setText(prefs.getString("dbxName",""));
        mEditEmail.setText(prefs.getString("dbxEmail",""));

        SeekBar mSeekControl = (SeekBar) getActivity().findViewById(R.id.seekControl);
        mSeekControl.setMax(3);
        mSeekControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

                //sendMessage("TEST");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });


    }


    String encrypt(String text){
        //TODO Fix this garbage function
        String iv = "WHYDOINEEDIVHELP";
        String SecretKey = "WHYDOINEEDKEYPLZ";

        IvParameterSpec ivspec = new IvParameterSpec(iv.getBytes());
        SecretKeySpec keyspec = new SecretKeySpec(SecretKey.getBytes(), "AES");
        Cipher c = null;

        try {
            c = Cipher.getInstance("AES/CBC/NoPadding");
        }catch(NoSuchAlgorithmException e){

        }catch(NoSuchPaddingException e){

        }

        byte[] encrypted = null;

        try{
            c.init(c.ENCRYPT_MODE, keyspec);
            encrypted = c.doFinal(padString(text).getBytes());
        }catch(Exception e){

        }
        String encrpytedString =  byteArraytoHexString(encrypted);
        return encrpytedString;

    }

    static String byteArraytoHexString(byte[] array) {
        StringBuffer hexString = new StringBuffer();
        for (byte b : array) {
            int intVal = b & 0xff;
            if (intVal < 0x10)
                hexString.append("0");
            hexString.append(Integer.toHexString(intVal));
        }
        return hexString.toString();
    }

    static String padString(String source) {
        char paddingChar = 0;
        int size = 16;
        int x = source.length() % size;
        int padLength = size - x;
        for (int i = 0; i < padLength; i++) {
            source += paddingChar;
        }
        return source;
    }

    class Network{
        private int id;
        private String ssid;
        private String security;
        private String status;
        private String serviceKey;
        private String password;
        private String  username;

        public Network(){}

        public Network(int id, String ssid, String security){
            this.id = id;
            this.ssid = ssid;
            this.security = security;
        }

        public Network(int id, String ssid, String password, int dummy){
            this.id = id;
            this.ssid = ssid;
            this.password = password;
        }

        public Network(int id, String ssid, String serviceKey, String status, String security){
            this.id = id;
            this.ssid = ssid;
            this.serviceKey = serviceKey;
            this.status = status;
            this.security = security;
        }

        public Network(int id, String ssid, String serviceKey, String status, String security, String password){
            this.id = id;
            this.ssid = ssid;
            this.serviceKey = serviceKey;
            this.status = status;
            this.security = security;
            this.password = password;
        }

        public Network(int id, String ssid, String serviceKey, String status, String security, String password, String username){
            this.id = id;
            this.ssid = ssid;
            this.serviceKey = serviceKey;
            this.status = status;
            this.security = security;
            this.password = password;
            this.username = username;
        }

        public Network(int id, String ssid, String securityType, String connectionStatus){
            this.id = id;
            this.ssid = ssid;
            this.security = securityType;
            this.status = connectionStatus;
        }

        public int getId(){
            return this.id;
        }

        public String getSsid(){
            return this.ssid;
        }

        public String getStatus(){
            return this.status;
        }

        public String getSecurity(){
            return this.security;
        }

        public String getServiceKey(){
            return this.serviceKey;
        }

        @Override public String toString(){
            return ssid;
        }

    }

    class Command{
        private String type;
        //TODO Expand on this
    }

    /**
     * Set up the UI and background operations for chat.
     */
    private void setupChat() {
        //Log.d(TAG, "setupChat()");

        // Initialize the array adapter for the conversation thread
        mConversationArrayAdapter = new ArrayAdapter<String>(getActivity(), R.layout.message);

        mConversationView.setAdapter(mConversationArrayAdapter);

        // Initialize the compose field with a listener for the return key
        mOutEditText.setOnEditorActionListener(mWriteListener);

        // Initialize the send button with a listener that for click events
        mSendButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                // Send a message using content of the edit text widget
                View view = getView();
                if (null != view) {
                    TextView textView = (TextView) view.findViewById(R.id.edit_text_out);
                    String message = textView.getText().toString();
                    sendMessage(message);
                }
            }
        });

        // Initialize the BluetoothChatService to perform bluetooth connections
        mChatService = new BluetoothChatService(getActivity(), mHandler);

        // Initialize the buffer for outgoing messages
        mOutStringBuffer = new StringBuffer("");
    }

    /**
     * Makes this device discoverable.
     */
    private void ensureDiscoverable() {
        if (mBluetoothAdapter.getScanMode() !=
                BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE) {
            Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
            discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 300);
            startActivity(discoverableIntent);
        }
    }

    /**
     * Sends a message.
     *
     * @param message A string of text to send.
     */
    public void sendMessage(String message) {
        // Check that we're actually connected before trying anything
        if (mChatService.getState() != BluetoothChatService.STATE_CONNECTED) {
            Toast.makeText(getActivity(), R.string.not_connected, Toast.LENGTH_SHORT).show();
            return;
        }

        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the message bytes and tell the BluetoothChatService to write
            byte[] send = message.getBytes();
            mChatService.write(send);

            // Reset out string buffer to zero and clear the edit text field
            mOutStringBuffer.setLength(0);
            mOutEditText.setText(mOutStringBuffer);
        }
    }

    /**
     * The action listener for the EditText widget, to listen for the return key
     */
    private TextView.OnEditorActionListener mWriteListener
            = new TextView.OnEditorActionListener() {
        public boolean onEditorAction(TextView view, int actionId, KeyEvent event) {
            // If the action is a key-up event on the return key, send the message
            if (actionId == EditorInfo.IME_NULL && event.getAction() == KeyEvent.ACTION_UP) {
                String message = view.getText().toString();
                sendMessage(message);
            }
            return true;
        }
    };

    /**
     * Updates the status on the action bar.
     *
     * @param resId a string resource ID
     */
    private void setStatus(int resId) {
        FragmentActivity activity = getActivity();
        if (null == activity) {
            return;
        }
        final ActionBar actionBar = activity.getActionBar();
        if (null == actionBar) {
            return;
        }
        actionBar.setSubtitle(resId);
    }

    /**
     * Updates the status on the action bar.
     *
     * @param subTitle status
     */
    private void setStatus(CharSequence subTitle) {
        FragmentActivity activity = getActivity();
        if (null == activity) {
            return;
        }
        final ActionBar actionBar = activity.getActionBar();
        if (null == actionBar) {
            return;
        }
        actionBar.setSubtitle(subTitle);
    }

    /**
     * The Handler that gets information back from the BluetoothChatService
     */
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            FragmentActivity activity = getActivity();
            switch (msg.what) {
                case Constants.MESSAGE_STATE_CHANGE:
                    switch (msg.arg1) {
                        case BluetoothChatService.STATE_CONNECTED:
                            setStatus(getString(R.string.title_connected_to, mConnectedDeviceName));
                            mConversationArrayAdapter.clear();
                            break;
                        case BluetoothChatService.STATE_CONNECTING:
                            setStatus(R.string.title_connecting);
                            break;
                        case BluetoothChatService.STATE_LISTEN:
                        case BluetoothChatService.STATE_NONE:
                            setStatus(R.string.title_not_connected);
                            break;
                    }
                    break;
                case Constants.MESSAGE_WRITE:
                    byte[] writeBuf = (byte[]) msg.obj;
                    // construct a string from the buffer
                    String writeMessage = new String(writeBuf);
                    mConversationArrayAdapter.add("Me:  " + writeMessage);
                    break;
                case Constants.MESSAGE_READ:
                    byte[] readBuf = (byte[]) msg.obj;
                    // construct a string from the valid bytes in the buffer
                    String readMessage = new String(readBuf, 0, msg.arg1);

                    //Parse message
                    try{
                        JsonParser parser = new JsonParser();
                        JsonObject o = parser.parse(readMessage).getAsJsonObject();
                        String type = o.get("type").getAsString();

                        /*Context context = getActivity().getApplicationContext();
                        int duration = Toast.LENGTH_SHORT;
                        Toast toast = Toast.makeText(context, type, duration);
                        toast.show();*/

                        switch(type){
                            case "listNetworks":
                                JsonArray array = o.get("networks").getAsJsonArray();
                                networksTest.clear();

                                for(JsonElement net : array){
                                    networksTest.add(new Network(
                                            net.getAsJsonObject().get("id").getAsInt(),
                                            net.getAsJsonObject().get("ssid").getAsString(),
                                            net.getAsJsonObject().get("serviceKey").getAsString(),
                                            net.getAsJsonObject().get("status").getAsString(),
                                            net.getAsJsonObject().get("security").getAsString()
                                    ));
                                }

                                //Update the spinner
                                mAdapterNetwork.notifyDataSetChanged();

                                break;
                            case "getAccessKey":
                                //Toast.makeText(getActivity().getApplicationContext(), "I JUST GOT A MESSAGE THAT I SHOULD SEND AN ACCESS KEY", Toast.LENGTH_LONG).show();
                                SharedPreferences prefs = getActivity().getSharedPreferences("com.example.android.inspirado", Context.MODE_PRIVATE);
                                String accessToken = prefs.getString("access-token", null);

                                if (accessToken == null) {
                                    //
                                } else {
                                    //accessToken already exists
                                    Gson gson = new Gson();
                                    JsonObject jsonObject = new JsonObject();
                                    jsonObject.addProperty("type","accessKeyToServer");
                                    JsonObject jsonObjectTemp = new JsonObject();
                                    jsonObjectTemp.addProperty("access_key",accessToken);
                                    jsonObject.add("parameters",jsonObjectTemp);
                                    String json = gson.toJson(jsonObject);
                                    sendMessageDummy(json,"Dummy");
                                }

                                break;
                            default:
                                break;
                        }

                    }catch(Exception e){
                        //Not valid JSON
                    }

                    mConversationArrayAdapter.add(mConnectedDeviceName + ":  " + readMessage);
                    break;
                case Constants.MESSAGE_DEVICE_NAME:
                    // save the connected device's name
                    mConnectedDeviceName = msg.getData().getString(Constants.DEVICE_NAME);
                    if (null != activity) {
                        Toast.makeText(activity, "Connected to "
                                + mConnectedDeviceName, Toast.LENGTH_SHORT).show();
                    }
                    break;
                case Constants.MESSAGE_TOAST:
                    if (null != activity) {
                        Toast.makeText(activity, msg.getData().getString(Constants.TOAST),
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
                default:
                    if (null != activity) {
                        Toast.makeText(activity, "Something Happened",
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
            }
        }
    };


    /**
     * Sends a message. Changed name, moved down, and Overloaded because I was having trouble sending from within handlecommand.
     *
     * @param message A string of text to send.
     */
    public void sendMessageDummy(String message, String dummy) {
        // Check that we're actually connected before trying anything
        if (mChatService.getState() != BluetoothChatService.STATE_CONNECTED) {
            Toast.makeText(getActivity(), R.string.not_connected, Toast.LENGTH_SHORT).show();
            return;
        }

        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the message bytes and tell the BluetoothChatService to write
            byte[] send = message.getBytes();
            mChatService.write(send);

            // Reset out string buffer to zero and clear the edit text field
            mOutStringBuffer.setLength(0);
            mOutEditText.setText(mOutStringBuffer);
        }
    }


    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_CONNECT_DEVICE_SECURE:
                // When DeviceListActivity returns with a device to connect
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data, true);
                }
                break;
            case REQUEST_CONNECT_DEVICE_INSECURE:
                // When DeviceListActivity returns with a device to connect
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data, false);
                }
                break;
            case REQUEST_ENABLE_BT:
                // When the request to enable Bluetooth returns
                if (resultCode == Activity.RESULT_OK) {
                    // Bluetooth is now enabled, so set up a chat session
                    setupChat();
                } else {
                    // User did not enable Bluetooth or an error occurred
                    //Log.d(TAG, "BT not enabled");
                    Toast.makeText(getActivity(), R.string.bt_not_enabled_leaving,
                            Toast.LENGTH_SHORT).show();
                    getActivity().finish();
                }
        }
    }

    /**
     * Establish connection with other divice
     *
     * @param data   An {@link Intent} with {@link DeviceListActivity#EXTRA_DEVICE_ADDRESS} extra.
     * @param secure Socket Security type - Secure (true) , Insecure (false)
     */
    private void connectDevice(Intent data, boolean secure) {
        // Get the device MAC address
        String address = data.getExtras()
                .getString(DeviceListActivity.EXTRA_DEVICE_ADDRESS);
        // Get the BluetoothDevice object
        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice(address);
        // Attempt to connect to the device
        mChatService.connect(device, secure);
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        inflater.inflate(R.menu.bluetooth_chat, menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.secure_connect_scan: {
                // Launch the DeviceListActivity to see devices and do scan
                Intent serverIntent = new Intent(getActivity(), DeviceListActivity.class);
                startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE_SECURE);
                return true;
            }
            case R.id.insecure_connect_scan: {
                // Launch the DeviceListActivity to see devices and do scan
                Intent serverIntent = new Intent(getActivity(), DeviceListActivity.class);
                startActivityForResult(serverIntent, REQUEST_CONNECT_DEVICE_INSECURE);
                return true;
            }
            case R.id.discoverable: {
                // Ensure this device is discoverable by others
                ensureDiscoverable();
                return true;
            }
            case R.id.about: {
                //TODO: Add an about screen
                return true;
            }
        }
        return false;
    }

}
