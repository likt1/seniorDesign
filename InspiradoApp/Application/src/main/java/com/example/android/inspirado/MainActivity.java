package com.example.android.inspirado;

import android.app.ActionBar;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.dropbox.core.v2.users.FullAccount;

import java.util.Locale;


public class MainActivity extends FragmentActivity implements ActionBar.TabListener{

    SectionsPagerAdapter mSectionsPagerAdapter;
    ViewPager mViewPager;
    StringBuilder sb = new StringBuilder();
    private boolean mLogShown;
    private final Handler handler = new Handler();

    private static final int IMAGE_REQUEST_CODE = 101;
    private String ACCESS_TOKEN;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Load the UI from res/layout/sample_main.xml
        setContentView(R.layout.sample_main);

        final ActionBar actionBar = getActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

        mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

        // Set up the ViewPager with the sections adapter.
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mViewPager.setAdapter(mSectionsPagerAdapter);

        mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener() {
            @Override
            public void onPageSelected(int position) {
                actionBar.setSelectedNavigationItem(position);
                SharedPreferences prefs = getSharedPreferences("com.example.android.inspirado", Context.MODE_PRIVATE);
                prefs.edit().putInt("currentPage",position).apply();
            }
        });

        for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++) {
            // Create a tab with text corresponding to the page title defined by the adapter. Also
            // specify this Activity object, which implements the TabListener interface, as the
            // callback (listener) for when this tab is selected.
            actionBar.addTab(
                    actionBar.newTab()
                            .setText(mSectionsPagerAdapter.getPageTitle(i))
                            .setTabListener(this));
        }

        if (!tokenExists()) {
            //No token
            //Back to LoginActivity
            Intent intent = new Intent(MainActivity.this, LoginActivity.class);
            startActivity(intent);
        }

        ACCESS_TOKEN = retrieveAccessToken();
        getUserAccount();

        android.support.v4.app.FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        BluetoothChatFragment fragment = new BluetoothChatFragment();
        transaction.replace(R.id.sample_content_fragment, fragment);
        transaction.commit();

    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem logToggle = menu.findItem(R.id.menu_toggle_log);
        //logToggle.setVisible(findViewById(R.id.sample_output) instanceof ViewAnimator);
        //logToggle.setTitle(mLogShown ? R.string.sample_hide_log : R.string.sample_show_log);

        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        /*
        switch(item.getItemId()) {
            case R.id.menu_toggle_log:
                mLogShown = !mLogShown;
                ViewAnimator output = (ViewAnimator) findViewById(R.id.sample_output);
                if (mLogShown) {
                    output.setDisplayedChild(1);
                } else {
                    output.setDisplayedChild(0);
                }
                supportInvalidateOptionsMenu();
                return true;
        }*/
        return super.onOptionsItemSelected(item);

    }

    /**
     * Update {@link ViewPager} after a tab has been selected in the ActionBar.
     *
     * @param tab Tab that was selected.
     * @param fragmentTransaction A {@link android.app.FragmentTransaction} for queuing fragment operations to
     *                            execute once this method returns. This FragmentTransaction does
     *                            not support being added to the back stack.
     */
    // BEGIN_INCLUDE (on_tab_selected)
    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
        // When the given tab is selected, tell the ViewPager to switch to the corresponding page.
        mViewPager.setCurrentItem(tab.getPosition());
    }
    // END_INCLUDE (on_tab_selected)

    /**
     * Unused. Required for {@link android.app.ActionBar.TabListener}.
     */
    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    /**
     * Unused. Required for {@link android.app.ActionBar.TabListener}.
     */
    @Override
    public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction) {
    }

    // BEGIN_INCLUDE (fragment_pager_adapter)
    /**
     * A {@link FragmentPagerAdapter} that returns a fragment corresponding to
     * one of the sections/tabs/pages. This provides the data for the {@link ViewPager}.
     */
    public class SectionsPagerAdapter extends FragmentPagerAdapter {
    // END_INCLUDE (fragment_pager_adapter)

        public SectionsPagerAdapter(FragmentManager fm) {
            super(fm);
        }

        // BEGIN_INCLUDE (fragment_pager_adapter_getitem)
        /**
         * Get fragment corresponding to a specific position. This will be used to populate the
         * contents of the {@link ViewPager}.
         *
         * @param position Position to fetch fragment for.
         * @return Fragment for specified position.
         */
        @Override
        public Fragment getItem(int position) {
            // getItem is called to instantiate the fragment for the given page.
            // Return a DummySectionFragment (defined as a static inner class
            // below) with the page number as its lone argument.
            switch(position) {
                case 0:
                    Fragment wifiFragment = new WifiFragment();
                    return wifiFragment;
                case 1:
                    Fragment dropboxFragment = new DropboxFragment();
                    return dropboxFragment;
                case 2:
                    Fragment controlFragment = new ControlFragment();
                    return controlFragment;
                default:
                    Fragment fragment = new DummySectionFragment();
                    Bundle args = new Bundle();
                    args.putInt(DummySectionFragment.ARG_SECTION_NUMBER, position + 1);
                    fragment.setArguments(args);
                    return fragment;
            }

        }
        // END_INCLUDE (fragment_pager_adapter_getitem)

        // BEGIN_INCLUDE (fragment_pager_adapter_getcount)
        /**
         * Get number of pages the {@link ViewPager} should render.
         *
         * @return Number of fragments to be rendered as pages.
         */
        @Override
        public int getCount() {
            // Show 3 total pages.
            return 3;
        }
        // END_INCLUDE (fragment_pager_adapter_getcount)

        // BEGIN_INCLUDE (fragment_pager_adapter_getpagetitle)
        /**
         * Get title for each of the pages. This will be displayed on each of the tabs.
         *
         * @param position Page to fetch title for.
         * @return Title for specified page.
         */
        @Override
        public CharSequence getPageTitle(int position) {
            Locale l = Locale.getDefault();
            switch (position) {
                case 0:
                    return getString(R.string.title_section1).toUpperCase(l);
                case 1:
                    return getString(R.string.title_section2).toUpperCase(l);
                case 2:
                    return getString(R.string.title_section3).toUpperCase(l);
            }
            return null;
        }
        // END_INCLUDE (fragment_pager_adapter_getpagetitle)
    }

    /**
     * A dummy fragment representing a section of the app, but that simply displays dummy text.
     * This would be replaced with your application's content.
     */
    public static class DummySectionFragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        public static final String ARG_SECTION_NUMBER = "section_number";

        public DummySectionFragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.fragment_main_dummy, container, false);
            TextView dummyTextView = (TextView) rootView.findViewById(R.id.section_label);
            dummyTextView.setText(Integer.toString(getArguments().getInt(ARG_SECTION_NUMBER)));
            return rootView;
        }
    }

    /**
     */
    public static class WifiFragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        public static final String ARG_SECTION_NUMBER = "section_number";

        public WifiFragment() {
        }

        Intent intent;

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.wifi, container, false);

            Button button = (Button) rootView.findViewById(R.id.button_send);
            return rootView;
        }

    }

    /**
     */
    public static class DropboxFragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        public static final String ARG_SECTION_NUMBER = "section_number";

        public DropboxFragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.dropbox, container, false);
            return rootView;
        }
    }

    /**
     */
    public static class ControlFragment extends Fragment {
        /**
         * The fragment argument representing the section number for this
         * fragment.
         */
        public static final String ARG_SECTION_NUMBER = "section_number";

        public ControlFragment() {
        }

        @Override
        public View onCreateView(LayoutInflater inflater, ViewGroup container,
                                 Bundle savedInstanceState) {
            View rootView = inflater.inflate(R.layout.control, container, false);
            return rootView;
        }
    }

    private boolean tokenExists() {
        SharedPreferences prefs = getSharedPreferences("com.example.android.inspirado", Context.MODE_PRIVATE);
        String accessToken = prefs.getString("access-token", null);
        return accessToken != null;
    }


    private String retrieveAccessToken() {
        //check if ACCESS_TOKEN is previously stored on previous app launches
        SharedPreferences prefs = getSharedPreferences("com.example.android.inspirado", Context.MODE_PRIVATE);
        String accessToken = prefs.getString("access-token", null);
        if (accessToken == null) {
            return null;
        } else {
            //accessToken already exists
            return accessToken;
        }
    }

    protected void getUserAccount() {
        if (ACCESS_TOKEN == null)return;
        new UserAccountTask(DropboxClient.getClient(ACCESS_TOKEN), new UserAccountTask.TaskDelegate() {
            @Override
            public void onAccountReceived(FullAccount account) {
                //Print account's info
                SharedPreferences prefs = getSharedPreferences("com.example.android.inspirado", Context.MODE_PRIVATE);
                prefs.edit().putString("dbxName",account.getName().getDisplayName()).apply();
                prefs.edit().putString("dbxEmail",account.getEmail()).apply();
            }
            @Override
            public void onError(Exception error) {

            }
        }).execute();
    }


}
