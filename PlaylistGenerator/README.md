Generates playlists for Vauxhall Astra J (navi 650)

# Music Folder Structure

Music
	Artist
		Album
			Song
			.
			.
			.
		.
	.
	.

# Use

1. Run AlbumList to generate Albums.txt for each artist
2. Change order of albums to suit
3. Run Playlist to generate playlists

# Convert FLAC to MP3

flactomp3.bat (requires FFmpeg, tested with 4.2.1) will take the Music folder and convert all FLAC files to MP3 and put them into a folder called MusicMP3

# Gotchas

- Removes duplicate songs from playlist (caused by greatest hits type compilations)
- Playlist files may need deleted manually before running generator to get alphabetical order
- Only sorts case correctly for a-z and ö characters