package org.rockbox.Helper;

import android.util.Log;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiInfo;
import android.content.Context;
import android.os.Build;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import java.io.BufferedReader;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import org.json.JSONObject;
import org.json.JSONException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

public class Connectivity{   
    // Context for WiFi operations
    private static Context context = null;
    
    public static void setContext(Context ctx) {
        context = ctx;
    }

    public static boolean lastFm(boolean listenbrainz, String apiUrl, String username, String password, String apiKey, String sharedSecret, String artist, String track, String album, int timestamp, long length){
        // make sure all parameters are encoded correctly
        String username_enc;
        String password_enc;
        String artist_enc;
        String track_enc;
        String album_enc;
        try {
            username_enc = URLEncoder.encode(username, "UTF-8");
            password_enc = URLEncoder.encode(password, "UTF-8");

            artist_enc = URLEncoder.encode(artist, "UTF-8");
            track_enc = URLEncoder.encode(track, "UTF-8");
            album_enc = URLEncoder.encode(album, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            Log.d("RockboxService", "Last.fm: Error URL encoding: " + e.getMessage());
            return false;
        }

        String command;
        String response;
        if (!listenbrainz) {
            // auth
            String method = "auth.getMobileSession";
            String signBase = "api_key" + apiKey +
                        "method" + method + 
                        "password" + password +
                        "username" + username + sharedSecret;
            String apiSig = md5sum(signBase);

            command = "/data/data/gocurl --cacert /data/data/cacert.pem -s " +
                                "-X POST " +
                                "--url '" + apiUrl +"' " +
                                "-d " +
                                "'method="+method+
                                "&api_key="+apiKey+
                                "&username="+username_enc+
                                "&password="+password_enc+
                                "&api_sig="+apiSig+
                                "&format=json'";
            response = execShell(command);
            String sessionKey = "";
            try {
                JSONObject jsonObject = new JSONObject(response);
                if (jsonObject.has("session")) {
                    JSONObject session = jsonObject.getJSONObject("session");
                    if (session.has("key")) {
                        sessionKey = session.getString("key");
                    }
                }
            } catch (JSONException e) {
                Log.e("RockboxService", "Last.fm: json error: " + e.getMessage());
                return false;
            }

            // actual scrobble request
            method="track.scrobble";
            signBase =  "album"+album+
                        "api_key"+apiKey+
                        "artist"+artist+
                        "method"+method+
                        "sk"+sessionKey+
                        "timestamp"+String.valueOf(timestamp)+
                        "track"+track+sharedSecret;
            apiSig = md5sum(signBase);
            command = "/data/data/gocurl --cacert /data/data/cacert.pem -s " +
                        "-X POST " +
                        "--url '" + apiUrl +"' " +
                        "-d " +
                        "'method="+method+
                        "&api_key="+apiKey+
                        "&sk="+sessionKey+
                        "&artist="+artist_enc+
                        "&track="+track_enc+
                        "&album="+album_enc+
                        "&timestamp="+String.valueOf(timestamp)+
                        "&api_sig="+apiSig+
                        "&format=json'";
        } else {
            track_enc = track.replace("'", "'\\''");
            artist_enc = artist.replace("'", "'\\''");
            album_enc = album.replace("'", "'\\''");
            String jsonPayload = "{"
                + "\"listen_type\": \"single\","
                + "\"payload\": ["
                +   "{"
                +   "\"listened_at\":" + String.valueOf(timestamp) + ","
                +   "\"track_metadata\": {"
                +       "\"track_name\": \""+ track_enc +"\","
                +       "\"artist_name\": \""+ artist_enc +"\","
                +       "\"release_name\": \""+ album_enc +"\","
                +       "\"additional_info\": {"
                +           "\"duration_ms\":"+ String.valueOf(length)
                +          "}"
                +       "}"
                +    "}"
                + "]"
                + "}";

            command = "/data/data/gocurl --cacert /data/data/cacert.pem -s " +
                  "-X POST " +
                  "--url '" + apiUrl + "/submit-listens" + "' " +
                  "-H 'Content-Type: application/json' " +
                  "-H 'Authorization: Token " + apiKey + "' " +
                  "-d '" + jsonPayload + "'";
        }
        response = execShell(command);

        // check if we got a valid response
        try {
            JSONObject jsonObject = new JSONObject(response);
            if (jsonObject.has("scrobbles")) {
                return true;
            } else {
                return false;
            }
        } catch (JSONException e) {
            Log.e("RockboxService", "Last.fm: json error: " + e.getMessage());
            return false;
        }
    }

    public static String md5sum(String input){
        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            md.update(input.getBytes());
            byte[] digest = md.digest();
            StringBuilder output = new StringBuilder();
            for (byte b : digest) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    output.append('0');
                }
                output.append(hex);
            }
            return output.toString();
        } catch (NoSuchAlgorithmException e) {
            Log.e("RockboxService", "md5sum error: " + e.getMessage());
            return "";
        }
    }

    public static String execShell(String command) {
        StringBuilder output = new StringBuilder();
        StringBuilder errorOutput = new StringBuilder();
        java.lang.Process process = null;
        BufferedReader reader = null;
        BufferedReader errorReader = null;

        try {
            process = Runtime.getRuntime().exec("su -u root -c " + command);

            // stdout
            reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line;
            while ((line = reader.readLine()) != null) {
                output.append(line).append("\n");
            }

            // stderr
            errorReader = new BufferedReader(new InputStreamReader(process.getErrorStream()));
            String errorLine;
            while ((errorLine = errorReader.readLine()) != null) {
                errorOutput.append(errorLine).append("\n");
            }

            process.waitFor();
        } catch (IOException | InterruptedException e) {
            Log.e("RockboxService", "exec shell error: " + e.toString());
            return "Error executing command: " + e.getMessage();
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e) {
                    Log.e("RockboxService", "exec shell error: " + e.toString());
                }
            }
            if (errorReader != null) {
                try {
                    errorReader.close();
                } catch (IOException e) {
                    Log.e("RockboxService", "exec shell error: " + e.toString());
                }
            }
            if (process != null) {
                process.destroy();
            }
        }

        // Combine stdout + stderr
        if (errorOutput.length() > 0) {
            return output.toString() + "\nErrors:\n" + errorOutput.toString();
        } else {
            return output.toString();
        }
    }

    public static boolean downloadPodcast(String podcastUrl, String outputPath, int episode) {
        String command = "/data/data/poddl -ca-cert /data/data/cacert.pem"+
                                         " -write-tag"+ 
                                         " -add-episode-to-title"+
                                         " -add-episode-to-filename"+
                                         " -dest \""+ outputPath + "\"" +
                                         " -num " + String.valueOf(episode) + 
                                         " " + podcastUrl;
        String ret;
        ret = execShell(command);
        if (ret.contains("Failed reading")){
            return false;
        } else {
            return true;
        }
    }

    public static String getPodcastNames(ArrayList<String[]> podcasts) {
        String ret = "";
        for (int i = 0; i < podcasts.size(); i++) {
            ret = ret + podcasts.get(i)[0] + "\n";
        }
        return ret;
    }

    public static String getPodcastUrls(ArrayList<String[]> podcasts){
        String ret = "";
        for (int i = 0; i < podcasts.size(); i++) {
            ret = ret + podcasts.get(i)[1] + "\n";
        }
        return ret;
    }

    public static void startPodcastDownload(ArrayList<String[]> podcasts, int podcastNum, int episode, String podcastFolder) {
        String podcastUrl=podcasts.get(podcastNum)[1];
        String outputPath=podcastFolder;
        if (outputPath.endsWith("/")){
            outputPath = outputPath + podcasts.get(podcastNum)[0];
        } else {
            outputPath = outputPath + "/" + podcasts.get(podcastNum)[0];
        }
        boolean ret;
        String ret_str;

        ret = downloadPodcast(podcastUrl, outputPath, episode);
    }

    public static int getNewEpisodes(String podcastUrl, String podcastFolder) {
        String outputPath = podcastFolder;
        /* currently not using this method but we would need to either pass podcastNum or calculate it
        if (outputPath.endsWith("/")){
            outputPath = outputPath + podcasts.get(podcastNum)[0];
        } else {
            outputPath = outputPath + "/" + podcasts.get(podcastNum)[0];
        }
        */
        String command="/data/data/poddl " + "-dest \"" + outputPath + "\" -ca-cert /data/data/cacert.pem -check " + podcastUrl;
        String ret;
        int newEpisodes;

        ret = execShell(command);
        try {
            newEpisodes = Integer.parseInt(ret.trim());
        } catch (NumberFormatException e) {
            newEpisodes = -1;
            Log.d("RockboxService", "getNewEpisodes: " + e.getMessage());
        }

        return newEpisodes;
    }

    public static String getEpisodeList(ArrayList<String[]> podcasts, int podcastNum, String podcastFolder) {
        String podcastUrl=podcasts.get(podcastNum)[1];
        String outputPath=podcastFolder;
        if (outputPath.endsWith("/")){
            outputPath = outputPath + podcasts.get(podcastNum)[0];
        } else {
            outputPath = outputPath + "/" + podcasts.get(podcastNum)[0];
        }
        String command="/data/data/poddl " + "-dest \"" + outputPath + "\" -ca-cert /data/data/cacert.pem -get-episodes " + podcastUrl;
        String ret;
        ret = execShell(command);

        if (ret != null) {
            return ret;
        } else {
            return "ERROR";
        }
    }

    public static String getEpisodePath(ArrayList<String[]> podcasts, int podcastNum, int num, String podcastFolder) {
        String podcastUrl=podcasts.get(podcastNum)[1];
        String outputPath=podcastFolder;
        if (outputPath.endsWith("/")){
            outputPath = outputPath + podcasts.get(podcastNum)[0];
        } else {
            outputPath = outputPath + "/" + podcasts.get(podcastNum)[0];
        }
        String command = "/data/data/poddl -ca-cert /data/data/cacert.pem"+
                                " -write-tag"+ 
                                " -add-episode-to-title"+
                                " -add-episode-to-filename"+
                                " -dest \""+ outputPath + "\"" +
                                " -num " + String.valueOf(num) + 
                                " -get-location" +
                                " " + podcastUrl;

        String ret;
        ret = execShell(command);

        if (ret != null) {
            return ret;
        } else {
            return "ERROR";
        }
    }

    public static void deleteEpisode(ArrayList<String[]> podcasts, int podcastNum, int episode, String podcastFolder) {
        String podcastUrl=podcasts.get(podcastNum)[1];
        String outputPath=podcastFolder;
        if (outputPath.endsWith("/")){
            outputPath = outputPath + podcasts.get(podcastNum)[0];
        } else {
            outputPath = outputPath + "/" + podcasts.get(podcastNum)[0];
        }

        String command = "/data/data/poddl -ca-cert /data/data/cacert.pem"+
                                        " -write-tag"+ 
                                        " -add-episode-to-title"+
                                        " -add-episode-to-filename"+
                                        " -dest \""+ outputPath + "\"" +
                                        " -delete " + String.valueOf(episode) + 
                                        " " + podcastUrl;
        String ret;
        ret = execShell(command);
    }
    
    public static String connectWifi(String ssid, String password) {
        int i;
        int threshold;

        if (context == null) {
            Log.d("RockboxService", "wifi error: context = null");
            return "Failed";
        }
        
        if (ssid == null || ssid.isEmpty()) {
            Log.d("RockboxService", "wifi error: no ssid configured");
            return "Not configured";
        }
        
        try {
            WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            
            if (wifiManager == null) {
                Log.d("RockboxService", "wifi error: wifiManager = null");
                return "Failed";
            }
            
            // enable wifi
            if (!wifiManager.isWifiEnabled()) {
                wifiManager.setWifiEnabled(true);
            }
            
            List<WifiConfiguration> configuredNetworks = wifiManager.getConfiguredNetworks();
            int networkId = -1;
            
            if (configuredNetworks != null) {
                for (WifiConfiguration config : configuredNetworks) {
                    if (config.SSID != null && config.SSID.equals("\"" + ssid + "\"")) {
                        networkId = config.networkId;
                        break;
                    }
                }
            }
            
            // If network is not configured, add it
            if (networkId == -1) {
                WifiConfiguration config = new WifiConfiguration();
                config.SSID = "\"" + ssid + "\"";
                config.preSharedKey = "\"" + password + "\"";
                config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
                config.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
                config.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
                config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
                config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
                config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
                config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
                
                // try to add network for up to 10s, the wifi stack takes some time to get ready
                threshold = 100;
                i = 1;
                while (networkId == -1 && i < threshold){
                    networkId = wifiManager.addNetwork(config);
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                    }
                    i++;
                }
                
                if (networkId == -1) {
                    Log.d("RockboxService", "wifi error: networkid = -1");
                    return "Failed";
                }
            }
            
            wifiManager.enableNetwork(networkId, true);
            wifiManager.saveConfiguration();
            
            wifiManager.reconnect();
            Log.d("RockboxService", "wifi connect: success");

            i = 1;
            threshold = 50;
            while (i < threshold){
                try {
                    if (isInternetAvailable()){
                        Log.d("RockboxService", "internet connection: success");
                        return "Success";
                    }
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }
                i++;
            }
            Log.e("RockboxService", "Failed to connect to internet");
            return "Failed";
        } catch (Exception e) {
            Log.e("RockboxService", "Failed to connect to wifi: " + e.getMessage());
            return "Failed";
        }
    }

    private static boolean isInternetAvailable() {
        try {
            Process p = Runtime.getRuntime().exec("ping -c 1 1.1.1.1");
            int returnVal = p.waitFor();
            return (returnVal == 0);
        } catch (Exception e) {
            return false;
        }
    }
    
    public static String disconnectWifi() {
        if (context == null) {
            Log.d("RockboxService", "wifi error, disconnect: context = null");
            return "Failed";
        }
        
        try {
            WifiManager wifiManager = (WifiManager) context.getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            
            if (wifiManager == null) {
                Log.d("RockboxService", "wifi error, disconnect: wifiManager = null");
                return "Failed";
            }
            
            // disconnect network, deactivate wifi
            wifiManager.disconnect();
            wifiManager.setWifiEnabled(false);

            Log.d("RockboxService", "wifi: disconnect success");
            return "Success";
        } catch (Exception e) {
            Log.e("RockboxService", "Failed to disconnect from WiFi: " + e.getMessage());
            return "Failed";
        }
    }
}
