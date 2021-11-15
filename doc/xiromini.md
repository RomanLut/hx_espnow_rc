

c=IN IP4 192.168.4.2
m=video 7078 RTP/AVP 96 
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1;profile-level-id=42001E  

ffmpeg -i rtsp://@192.168.241.1:62156 -acodec copy -vcodec copy c:/abc.mp4

ffmpeg -loglevel debug -rtsp_transport tcp -i "rtsp://admin:admin@198.175.207.61:554/live" -c copy -map 0 foo.mp4

ffmpeg -i rtsp://@192.168.241.1:62156 -acodec copy -vcodec copy c:/abc.mp4



ESP32 IP forwarding, bridging, routing
https://esp32.com/viewtopic.php?t=9