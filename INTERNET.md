# Disclaimer
The Innioasis Y1 runs an outdated version of Android. Going online with it has inherent risks. The connections this plugin makes *should* be save, as they use up-to-date certificates and only make secure connections to official Last.fm API servers. With such old software you never know though. Please make your own judgement calls if you are fine with this risk.

# Podcasts
## Adding podcasts
1. Find your feed URL: https://rss.com/tools/find-my-feed/
2. Download the wifi config [here](https://github.com/rockbox-y1/rockbox/blob/y1/android/scripts/wifi.cfg)
3. Follow the instructions in the `wifi.cfg` and add 
4. Also make sure to fill your wifi credentials
5. Copy `wifi.cfg` to `.rockbox/wifi.cfg` on the SD card
6. Go to Plugins > Applications > podcast_downloader
7. Controls:
- Clicking podcast: Loads episodes
- Long-press on podcast: Context menu - Download all episodes (takes a long time and shows no progress bar)
- Clicking Episode: Downloads episode
- Long-press on podcast: Context menu - Play, Re-download, Delete

# Scrobbling: Last.fm, ListenBrainz, Maloja,...
### Last.fm
1. Login to Last.fm
2. Create an application [here](https://www.last.fm/api/account/create) (you only need to enter an application name, e.g. `rockbox`)
3. Save the API key and the Shared secret that is displayed after creating the application
4. Download the wifi config [here](https://github.com/rockbox-y1/rockbox/blob/y1/android/scripts/wifi.cfg)
5. Follow the instructions in `wifi.cfg` and fill all the values necessary
6. Also make sure to fill your wifi credentials
7. Keep this file safe, don't share it online
8. Copy `wifi.cfg` to `.rockbox/wifi.cfg` on the SD card

### ListenBrainz
1. Login to ListenBrainz
2. Copy the user token from your settings page
3. Download the wifi config [here](https://github.com/rockbox-y1/rockbox/blob/y1/android/scripts/wifi.cfg)
4. Follow the instructions in `wifi.cfg` and fill all the values necessary
5. Also make sure to fill your wifi credentials
6. Keep this file safe, don't share it online
7. Copy `wifi.cfg` to `.rockbox/wifi.cfg` on the SD card

**Hint:** This works for ListenBrainz compatible servers too, just change the apiUrl.

### Scrobble Songs
1. Go to Plugins > Applications > lastfm_scrobbler
2. Follow general setup instructions
3. Upload Scrobbles > Yes
4. (Listen to some music)
5. Press Export

**Hint:** Set the timezone setting to 0 if it was configured to something else before. The time and time zone is setup automatically once you connect to the internet if you didn't change any time/date settings in the Android System Menus.